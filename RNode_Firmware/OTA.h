// Copyright (C) 2024, Mark Qvist

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Network OTA firmware updates - HAS_OTA boards only (Boards.h; currently
// just MeshAdventurer-S3 - MeshPoE-S3 originally had this too but it's
// disabled there, see HAS_CONSOLE's own comment on that board's block).
// Originally written for MeshPoE-S3 specifically (PoE/Ethernet-deployed,
// likely to end up mounted somewhere without easy physical access, hence a
// browser-reachable page rather than the on-device button menu) but the
// same reasoning applies to any HAS_OTA board.
//
// Two ways to install a new image, both writing into the *inactive* OTA
// partition (default_16MB.csv, app0/app1 - see Makefile) so a failed update
// can never touch the currently-running, known-good firmware:
//   - Pull: device fetches the .bin from flasher.rns.moscow over HTTPS,
//     using the vanilla ESP-IDF esp_https_ota component (not Arduino's
//     Update.h wrapper).
//   - Push: browser uploads a .bin directly; handled with the lower-level
//     esp_ota_ops calls (esp_ota_begin/write/end) so both paths stay at the
//     same "vanilla ESP-IDF" level instead of mixing in a second OTA API.
//
// Accepted scope decisions (see plan): the page is open, no auth - anyone
// who can reach the device's IP can trigger a flash. There's no bootloader-
// level auto-rollback available through arduino-cli's prebuilt bootloader,
// so ota_check_recovery_button() (called first thing in setup(), RNode_
// Firmware.ino) is the field-recovery path: hold pin_btn_usr1 at power-on to
// force-boot the other OTA slot.

#include <esp_http_client.h>
#include <esp_https_ota.h>
#include <esp_crt_bundle.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_heap_caps.h>
#include <esp_attr.h>
#include <WebServer.h>

#if HAS_MENU == true
  // Forward declaration - Menu.h (which defines this) is included after
  // OTA.h in Utilities.h's chain. Needed so ota_do_pull_download() can tell
  // whether the menu currently owns the screen (see its own comment).
  bool menu_is_open();
#endif

#define OTA_WEB_PORT 8080
// Board-specific (OTA_BOARD_NAME, Boards.h - every HAS_OTA board defines
// its own) - these used to be a single hardcoded "meshpoe_s3" name shared
// by every HAS_OTA board regardless of which one was actually compiling,
// so any other HAS_OTA board's update check/pull silently used MeshPoE-S3's
// version number and binary instead of its own. Points through latest/ (a
// stable alias re-pointed at whichever release is current, not a specific
// version), so a compiled-in URL keeps working across future releases
// without needing a reflash just to update the URL itself - see the OTA/
// subdirectory under each classic/<version>/ release directory.
#define OTA_VERSION_URL "https://flasher.rns.moscow/firmware/classic/latest/OTA/rnode_firmware_" OTA_BOARD_NAME ".version"
#define OTA_BIN_URL     "https://flasher.rns.moscow/firmware/classic/latest/OTA/rnode_firmware_" OTA_BOARD_NAME ".bin"
#define OTA_HTTP_TIMEOUT_MS 15000

// Build identity used for update comparisons - deliberately separate from
// MAJ_VERS/MIN_VERS (Config.h), which track the upstream RNode wire
// protocol version and shouldn't be bumped just to ship an OTA-testable
// build. BUILD_NUMBER is the repo's git commit count at compile time
// (Makefile: firmware-meshpoe_s3/release-meshpoe_s3, `git rev-list --count
// HEAD`), so it increases automatically with every commit with no manual
// version bookkeeping. The .version file published alongside the .bin
// (see plan) is just this plain integer. Not OTA-specific despite living
// in this file for now - see Boards.h.
//
// Defined (with a 0 fallback for non-Makefile builds) in Boards.h, not
// here - Display.h (VERSION banner, disp_page 2) also needs it and is
// included earlier than OTA.h in Utilities.h's chain.

// Optional progress hook, shared by the web (/install, /upload) and menu
// (Menu.h, F/W Update) trigger paths - same plain-string-callback pattern
// as RTC.h's rtc_sync_status_cb_t/rtc_sync_ntp(). Web callers pass
// ota_stream_progress() (streams each update into the still-open /install
// response, see its own comment); the menu passes menu_draw_popup so its
// screen stays live for the whole blocking transfer. Both now receive a
// "DOWNLOADING NN%" string once ota_dl_percent is known, not just the
// static "DOWNLOADING..." this used to be fixed at.
typedef void (*ota_status_cb_t)(const char *status);

WebServer ota_server(OTA_WEB_PORT);
bool ota_in_progress = false;

// Random per-boot session marker, set once in ota_server_init() and
// exposed via /check's JSON - lets the web page's verifyReboot() detect a
// genuine reboot after triggering an install, independent of which build
// ends up running. Build-number comparison alone isn't enough: Force
// Update's whole point is reinstalling the *same* (or an older) build, so
// "build number unchanged" is the expected success outcome there, not a
// failure signal - only a changed boot_id actually proves a reboot
// happened.
uint32_t ota_boot_id = 0;

// Percent-complete of the current pull download, updated once per
// esp_https_ota_perform() call in ota_do_pull_download()'s loop below from
// esp_https_ota_get_image_len_read()/get_image_size() - -1 whenever no
// download is running or the server didn't send a Content-Length (chunked
// responses report size -1; flasher.rns.moscow's static .bin serving
// always has one in practice, but nothing here assumes that). Global
// rather than threaded through status_cb's plain string because the menu
// popup and the web stream both just want the same current number.
int ota_dl_percent = -1;

// ota_status_cb_t sink used by the web handlers (which have no live screen
// to update) purely to capture the failure reason for the HTTP response
// text - the callback itself does nothing UI-related.
char ota_last_status[24] = {0};
void ota_capture_status(const char *status) {
  strncpy(ota_last_status, status, sizeof(ota_last_status)-1);
  ota_last_status[sizeof(ota_last_status)-1] = 0;
}

// Upload (push) session state - set in ota_handle_upload_chunk(), consumed
// by ota_handle_upload_complete() once the WebServer upload callback chain
// for a single POST /upload finishes.
esp_ota_handle_t ota_upload_handle = 0;
const esp_partition_t *ota_upload_target = NULL;
bool ota_upload_ok = false;

void ota_dbg(String msg) { Serial.print("[OTA] "); Serial.println(msg); }

bool ota_network_up() {
  #if HAS_WIFI == true
    if (wifi_is_connected()) return true;
  #endif
  #if HAS_ETHERNET == true
    if (eth_is_connected) return true;
  #endif
  return false;
}

String ota_current_version() {
  return String((int)MAJ_VERS) + "." + String((int)MIN_VERS);
}

// Single-line "1.86.1041" form used elsewhere (Makefile's version.txt,
// `$(PROTO_VERSION).$(git rev-list --count HEAD)`) instead of the web page's
// old two-line version/build display. Safe to always append BUILD_NUMBER
// here (no BUILD_NUMBER==0 guard needed like Display.h's) because Boards.h's
// global fallback already force-disables HAS_OTA - and this file with it -
// whenever BUILD_NUMBER is 0.
String ota_full_version_string() {
  return ota_current_version() + "." + String(BUILD_NUMBER);
}

// Reused by both the pull and push flows once a new image has been written
// successfully - mirrors the existing serial-triggered CMD_FW_HASH handler
// (RNode_Firmware.ino) so the "Firmware Corrupt" check (Device.h) is
// satisfied on next boot without needing an external host to push the hash.
//
// esp_partition_get_sha256() reads the SHA-256 that the Arduino-ESP32 build
// pipeline appends to every compiled .bin (see partition_hashes, repo root -
// it independently verifies the same appended hash from a local .bin file)
// and re-validates it against the image before returning - it does NOT hash
// the whole (mostly-empty) partition. Calling it here and again from
// device_validate_partitions() (Device.h) on the next boot, with nothing
// else able to touch either OTA partition in between, is what makes this
// self-consistent without needing a separately-verified reference hash.
//
// Returns false (and leaves the running partition untouched/unbooted) if
// that read-back itself fails - meaning the just-written image didn't
// verify despite esp_ota_end()/esp_https_ota_finish() already reporting
// success, so we must not commit to booting it. Split from the actual
// reboot (ota_reboot(), below) specifically so a caller can send its own
// "success" response (e.g. the web handlers) or status_cb update in
// between - nothing after ota_reboot() ever runs.
bool ota_verify_and_set_boot(const esp_partition_t *target) {
  if (esp_partition_get_sha256(target, dev_firmware_hash_target) != ESP_OK) {
    ota_dbg("Hash verification of written image failed, not switching boot partition");
    return false;
  }
  device_save_firmware_hash();
  esp_ota_set_boot_partition(target);
  return true;
}

void ota_reboot(ota_status_cb_t status_cb = nullptr) {
  if (status_cb) status_cb("REBOOTING...");
  ota_dbg("Update installed, rebooting...");
  delay(200);
  esp_restart();
}

// Small blocking GET into a caller-provided buffer, used for the plain-text
// version file - kept on the same esp_http_client/cert-bundle stack as the
// firmware pull below rather than pulling in a second (Arduino) HTTP client
// just for this.
bool ota_http_get_text(const char *url, char *out_buf, size_t out_buf_len) {
  esp_http_client_config_t config = {};
  config.url = url;
  config.crt_bundle_attach = esp_crt_bundle_attach;
  config.timeout_ms = OTA_HTTP_TIMEOUT_MS;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) { DEBUG_LOG("[OTA] esp_http_client_init failed\r\n"); return false; }

  bool ok = false;
  esp_err_t open_err = esp_http_client_open(client, 0);
  if (open_err != ESP_OK) {
    // Internal-SRAM-only heap (MALLOC_CAP_INTERNAL) is what mbedTLS's TLS
    // session setup needs and what PSRAM=opi (Makefile) exists to relieve -
    // logged only on failure, not worth the noise on every call once this
    // is working.
    DEBUG_LOG("[OTA] esp_http_client_open(%s) -> %s (0x%x), internal heap free=%u largest=%u\r\n",
      url, esp_err_to_name(open_err), open_err,
      (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL), (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
  }
  if (open_err == ESP_OK) {
    esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);
    if (status == 200) {
      int read_len = esp_http_client_read(client, out_buf, out_buf_len-1);
      if (read_len >= 0) { out_buf[read_len] = 0; ok = true; }
    }
    esp_http_client_close(client);
  }
  esp_http_client_cleanup(client);
  return ok;
}

// Shared by the web /check handler and Menu.h's F/W Update > Check action.
// Caller is expected to have already confirmed ota_network_up() - this just
// does the HTTPS GET + comparison, no network-state handling of its own.
bool ota_do_check(long *current_out, long *latest_out, bool *newer_out) {
  char version_buf[32];
  if (!ota_http_get_text(OTA_VERSION_URL, version_buf, sizeof(version_buf))) return false;
  long remote_build = atol(version_buf);
  *current_out = BUILD_NUMBER;
  *latest_out = remote_build;
  *newer_out = remote_build > BUILD_NUMBER;
  return true;
}

// Device ID matches the "RNode XXXX" convention used everywhere else (BLE
// name/bt_devname, boot splash, Menu.h's settings_title) - same bt_dh[14]/
// bt_dh[15] MD5-of-MAC bytes, populated at boot (Bluetooth.h) regardless of
// whether Bluetooth itself is enabled, well before OTA.h's include point.
void ota_handle_root() {
  char id_buf[5];
  sprintf(id_buf, "%02X%02X", (uint8_t)bt_dh[14], (uint8_t)bt_dh[15]);

  // rns_link_state (Config.h) is the same host-connected flag Display.h's
  // cable icon reads - it's fed by whichever transport (USB/BT/WS) is
  // actually active, so this doesn't need to know which one. Not the
  // onboard microReticulum/LXMF node's own status (see urns_ready below) -
  // this is "is some *external* Reticulum instance (e.g. rnsd on a
  // connected PC) actively using this RNode as its KISS TNC interface right
  // now", flipped CONNECTED the moment a CMD_DATA KISS frame arrives
  // (RNode_Firmware.ino) and back to DISCONNECTED on host_disconnected()
  // (Utilities.h) - a carrier-detect signal, not a config setting.
  bool radio_on = radio_online;
  bool host_connected = (rns_link_state == RNS_LINK_STATE_CONNECTED);
  // urns_ready (URNS.h) - set once the onboard microReticulum/LXMF node
  // (identity, Transport, LXMF router/Messenger) has finished its own
  // startup, independent of whether any external host is attached above.
  #if HAS_URNS == true
    bool urns_active = urns_ready;
  #else
    bool urns_active = false;
  #endif
  bool show_caution = radio_on || host_connected;
  String caution = "";
  if (radio_on) caution += "The radio is currently active. ";
  if (host_connected) caution += "A KISS host interface is connected. ";

  String page = "<!doctype html><html><head><meta charset=\"utf-8\">";
  page += "<title>RNode " + String(id_buf) + " Firmware Update</title>";
  page += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  page += "<style>"
    ":root{--bg:#0a0e0a;--card:#0c130c;--accent:#00cc44;--accent-bright:#39ff14;"
    "--accent-dim:rgba(0,204,68,.55);--accent-ghost:rgba(0,204,68,.08);"
    "--amber:#f5a623;--amber-dim:rgba(245,166,35,.15);--red:#ff6666;"
    "--text:#b8d4b8;--text-bright:#d4edd4;--text-dim:#506650;--text-muted:#3a4e3a;"
    "--border:rgba(0,204,68,.18);"
    "--font:ui-monospace,'SFMono-Regular',Menlo,Consolas,'Liberation Mono',monospace}"
    "*{box-sizing:border-box}"
    "body{margin:0;background:var(--bg);color:var(--text);font-family:var(--font);"
    "font-size:14px;line-height:1.6;padding:24px 16px}"
    ".card{max-width:480px;margin:0 auto;background:var(--card);border:1px solid var(--border);"
    "border-radius:6px;padding:20px}"
    "h1{font-size:1.1rem;color:var(--text-bright);margin:0 0 4px;letter-spacing:.02em;font-weight:700}"
    "h1 .id{color:var(--accent-bright)}"
    ".ver{font-size:.78rem;color:var(--text-dim);margin-bottom:16px}"
    ".status-row{display:flex;gap:8px;margin-bottom:14px;flex-wrap:wrap}"
    ".badge{font-size:.68rem;letter-spacing:.06em;text-transform:uppercase;padding:4px 9px;"
    "border-radius:4px;border:1px solid var(--border);color:var(--text-dim)}"
    ".badge.ok{color:var(--accent);border-color:var(--accent-dim);background:var(--accent-ghost)}"
    ".badge.warn{color:var(--amber);border-color:rgba(245,166,35,.35);background:var(--amber-dim)}"
    ".caution{font-size:.8rem;color:var(--amber);background:var(--amber-dim);"
    "border:1px solid rgba(245,166,35,.35);border-radius:4px;padding:10px 12px;"
    "margin-bottom:16px;line-height:1.5}"
    ".section{border-top:1px solid var(--border);padding-top:14px;margin-top:14px}"
    ".section-title{font-size:.7rem;letter-spacing:.1em;text-transform:uppercase;"
    "color:var(--text-dim);margin-bottom:8px}"
    ".row{display:flex;gap:8px;flex-wrap:wrap;align-items:center}"
    ".btn{font-family:var(--font);font-size:.8rem;padding:9px 14px;background:transparent;"
    "color:var(--text);border:1px solid var(--border);border-radius:4px;cursor:pointer}"
    ".btn:hover:not(:disabled){border-color:var(--accent-dim);color:var(--accent)}"
    ".btn:disabled{opacity:.4;cursor:not-allowed}"
    // Base .btn:hover:not(:disabled) above sets color:var(--accent) (green)
    // at equal specificity to these - without an explicit color here too,
    // it wins on source order and turns the button text green on hover
    // regardless of variant (invisible on this one, since it's already
    // dark green on green; glaring on btn-danger below, dark red on red).
    ".btn-primary{background:var(--accent);color:#050c05;border-color:var(--accent);font-weight:700}"
    ".btn-primary:hover:not(:disabled){background:var(--accent-bright);border-color:var(--accent-bright);color:#050c05}"
    ".btn-primary:disabled{background:rgba(0,204,68,.12);color:var(--text-muted);border-color:var(--border)}"
    ".btn-danger{background:var(--red);color:#2a0a0a;border-color:var(--red);font-weight:700}"
    ".btn-danger:hover:not(:disabled){background:#ff8a8a;border-color:#ff8a8a;color:#2a0a0a}"
    ".btn-danger:disabled{background:rgba(255,102,102,.12);color:var(--text-muted);border-color:var(--border)}"
    "input[type=file]{font-family:var(--font);font-size:.76rem;color:var(--text-dim);flex:1;min-width:0}"
    ".status-line{font-size:.78rem;margin-top:10px;color:var(--text-dim)}"
    ".status-line.ok{color:var(--accent)}"
    ".status-line.warn{color:var(--amber)}"
    ".status-line.err{color:var(--red)}"
    ".progress-wrap{display:flex;align-items:center;gap:10px;margin-top:10px}"
    ".progress-bar{flex:1;height:5px;background:rgba(0,204,68,.1);border-radius:3px;overflow:hidden}"
    ".progress-fill{height:100%;width:0;background:var(--accent);transition:width .2s}"
    ".progress-text{font-size:.7rem;color:var(--text-dim);min-width:34px;text-align:right}"
    "</style></head><body><div class=\"card\">";

  page += "<h1>RNode <span class=\"id\">" + String(id_buf) + "</span> Firmware Update</h1>";
  page += "<div class=\"ver\">v" + ota_full_version_string() + "</div>";

  page += "<div class=\"status-row\">";
  page += "<span class=\"badge "; page += radio_on ? "warn" : "ok";
  page += "\">RADIO "; page += radio_on ? "ON" : "OFF"; page += "</span>";
  page += "<span class=\"badge "; page += host_connected ? "warn" : "ok";
  page += "\">HOST "; page += host_connected ? "CONNECTED" : "IDLE"; page += "</span>";
  // Unlike RADIO/HOST above, ACTIVE is the expected steady state here
  // (this board's own onboard node), not a caution one - "warn" instead
  // flags the node having failed to come up, worth noticing.
  page += "<span class=\"badge "; page += urns_active ? "ok" : "warn";
  page += "\">URNS "; page += urns_active ? "ACTIVE" : "OFF"; page += "</span>";
  page += "</div>";

  if (show_caution) {
    page += "<div class=\"caution\">&#9888; " + caution + "Installing now will interrupt operation.</div>";
  }

  page += "<div class=\"section\"><div class=\"section-title\">Check for Update</div>"
    "<div class=\"row\">"
    "<button id=\"btnCheck\" class=\"btn\" onclick=\"doCheck()\">Check for Update</button>"
    "<button id=\"btnInstall\" class=\"btn btn-primary\" disabled onclick=\"doInstall(false)\">Install Latest</button>"
    // Only shown once a check comes back "already up to date" (doCheck()
    // below) - lets the page force a reinstall of the same (or an older)
    // build anyway, e.g. to recover a device that reports corrupt/mismatched
    // firmware without waiting for a newer release to exist.
    "<button id=\"btnForce\" class=\"btn btn-danger\" style=\"display:none\" onclick=\"doInstall(true)\">Force Update</button>"
    "</div>"
    "<div class=\"progress-wrap\" id=\"installProgressWrap\" style=\"display:none\">"
    "<div class=\"progress-bar\"><div class=\"progress-fill\" id=\"installProgressFill\"></div></div>"
    "<span class=\"progress-text\" id=\"installProgressText\">0%</span></div>"
    "<div id=\"checkStatus\" class=\"status-line\"></div></div>"

    "<div class=\"section\"><div class=\"section-title\">Manual Firmware Upload</div>"
    "<div class=\"row\">"
    "<input type=\"file\" id=\"fwFile\" accept=\".bin\">"
    "<button id=\"btnUpload\" class=\"btn\" onclick=\"doUpload()\">Upload &amp; Install</button>"
    "</div>"
    "<div class=\"progress-wrap\" id=\"progressWrap\" style=\"display:none\">"
    "<div class=\"progress-bar\"><div class=\"progress-fill\" id=\"progressFill\"></div></div>"
    "<span class=\"progress-text\" id=\"progressText\">0%</span></div>"
    "<div id=\"uploadStatus\" class=\"status-line\"></div></div>"

    "</div><script>";

  page += "var proto='" + ota_current_version() + "';";
  page += "var showCaution=" + String(show_caution ? "true" : "false") + ";";
  page += "var cautionMsg='" + caution + "';";

  page += "var btnCheck=document.getElementById('btnCheck');"
    "var btnInstall=document.getElementById('btnInstall');"
    "var btnForce=document.getElementById('btnForce');"
    "var btnUpload=document.getElementById('btnUpload');"
    "var checkStatus=document.getElementById('checkStatus');"
    "var uploadStatus=document.getElementById('uploadStatus');"
    "var progressWrap=document.getElementById('progressWrap');"
    "var progressFill=document.getElementById('progressFill');"
    "var progressText=document.getElementById('progressText');"
    "var installProgressWrap=document.getElementById('installProgressWrap');"
    "var installProgressFill=document.getElementById('installProgressFill');"
    "var installProgressText=document.getElementById('installProgressText');"
    "function setStatus(el,text,cls){el.textContent=text;el.className='status-line'+(cls?' '+cls:'');}"
    // /status (not /check - see OTA.h's ota_handle_status() comment) is
    // purely local, so it's safe to poll repeatedly without depending on
    // the update server being reachable.
    "function fetchStatus(cb){"
      "fetch('/status',{cache:'no-store'}).then(function(r){return r.json();}).then(function(d){cb(d);}).catch(function(){cb(null);});"
    "}"
    // Both /install and /upload report success via the same "...rebooting"
    // wording (OTA.h's ota_reboot()) - once either sees it, the device is
    // about to disappear and come back on the newly installed image.
    // Rather than just blindly reloading after a guessed delay, poll
    // /status until it responds again.
    //
    // Each attempt is raced against a plain timer via Promise.race()
    // rather than force-aborting a slow fetch() with an AbortController -
    // simpler, and doesn't need cancellation semantics at all here: an
    // abandoned probe is just left to resolve (or not) on its own, its
    // result discarded if nothing is listening for it anymore. A real
    // reboot only takes a few seconds, so the retry budget stays short.
    "function pollUntilBackOnline(cb){"
      "var attempts=0,maxAttempts=20;" // ~20 * 3s = 60s - a real reboot takes seconds, not minutes
      "function attempt(){"
        "attempts++;"
        "var timeout=new Promise(function(resolve){setTimeout(function(){resolve(null);},3000);});"
        "var probe=fetch('/status',{cache:'no-store'}).then(function(r){return r.json();}).catch(function(){return null;});"
        "Promise.race([probe,timeout]).then(function(d){if(d){cb(d);}else{retry();}});"
      "}"
      "function retry(){if(attempts>=maxAttempts){cb(null);return;}setTimeout(attempt,3000);}"
      "attempt();"
    "}"
    // prevStatus is /status's response captured right before the install/
    // upload request was sent (doInstall()/doUpload() below) - comparing
    // boot_id, not the build number, is what actually proves a reboot
    // happened: Force Update's whole point is reinstalling the *same*
    // build, so an unchanged build number there is the expected success
    // outcome, not a failure sign. boot_id is a fresh random value picked
    // once per boot (OTA.h), so it's the one thing guaranteed to differ
    // after a real reboot regardless of which build ends up running.
    "function verifyReboot(statusEl,prevStatus,onDone){"
      "setStatus(statusEl,'Rebooting - waiting for device to come back online...','');"
      "pollUntilBackOnline(function(d){"
        "onDone();"
        "if(!d){setStatus(statusEl,'Device did not come back online - check it manually.','err');return;}"
        "var rebooted=!prevStatus||String(d.boot_id)!==String(prevStatus.boot_id);"
        "if(rebooted){setStatus(statusEl,'Update successful - device rebooted and is now running build '+d.current+'.','ok');}"
        "else{setStatus(statusEl,'Device responded, but does not appear to have rebooted - the update may not have taken effect.','warn');}"
      "});"
    "}"
    "function doCheck(){"
      "btnCheck.disabled=true;btnInstall.disabled=true;btnForce.style.display='none';btnForce.disabled=false;"
      "setStatus(checkStatus,'Checking...','');"
      "fetch('/check').then(function(r){return r.json();}).then(function(d){"
        "btnCheck.disabled=false;"
        "if(d.error){setStatus(checkStatus,d.error,'err');return;}"
        "var curV=proto+'.'+d.current,latV=proto+'.'+d.latest;"
        "if(d.newer){setStatus(checkStatus,'Update available: '+latV+' (current '+curV+')','warn');btnInstall.disabled=false;}"
        // Same build (or older) is all that's on the update server right
        // now - Install Latest has nothing to do, so Force Update takes
        // over as the only way to still push that same image, e.g. to
        // recover a device that's reporting corrupt/mismatched firmware.
        "else{setStatus(checkStatus,'Already up to date ('+curV+')','ok');btnInstall.disabled=true;btnForce.style.display='';}"
      "}).catch(function(){btnCheck.disabled=false;setStatus(checkStatus,'Request failed','err');});"
    "}"
    // /install streams one line per progress update over a single chunked
    // response (OTA.h's ota_stream_progress()) rather than returning once
    // at the end - readyState 3 (LOADING)/onprogress fire as each new
    // chunk arrives, letting responseText be read incrementally instead of
    // waiting for onload. The HTTP status is always 200 by the time any of
    // this streams (committed before the outcome is known) - success/
    // failure is read from the line text itself, not xhr.status.
    "function doInstall(force){"
      "var q=force?'No newer version is available on the update server.\\n\\nForce-install it anyway?':'Install the update now anyway?';"
      "if(force){if(!confirm((showCaution?cautionMsg+'\\n\\n':'')+q))return;}"
      "else if(showCaution&&!confirm(cautionMsg+'\\n\\n'+q)){return;}"
      "btnInstall.disabled=true;btnCheck.disabled=true;btnForce.disabled=true;"
      "installProgressWrap.style.display='flex';installProgressFill.style.width='0%';installProgressText.textContent='0%';"
      "setStatus(checkStatus,'Connecting...','');"
      // Captured fresh right here (not reused from some earlier doCheck())
      // so verifyReboot() below has an accurate pre-install boot_id to
      // compare against, even if the page has been sitting open a while.
      "fetchStatus(function(prevStatus){"
      "var xhr=new XMLHttpRequest();var seen=0;"
      "function poll(){"
        "var chunk=xhr.responseText.substring(seen);seen=xhr.responseText.length;"
        "var lines=chunk.split('\\n');"
        "for(var i=0;i<lines.length;i++){"
          "var line=lines[i].trim();"
          "if(!line)continue;"
          // In-progress phases (CONNECTING/DOWNLOADING, with or without a
          // percent) all get the same neutral styling and wording,
          // regardless of the raw uppercase text OTA.h's status_cb sent -
          // only a genuinely terminal line (reboot or failure) gets color.
          // Previously the percent-match branch forced grey while every
          // other line (including the plain "CONNECTING..." one) fell
          // through to the same 'ok'/green styling as a real success line -
          // inconsistent for no reason, since none of these are outcomes.
          "var m=line.match(/^DOWNLOADING (\\d+)%$/);"
          "if(m){"
            "installProgressFill.style.width=m[1]+'%';installProgressText.textContent=m[1]+'%';"
            "setStatus(checkStatus,'Downloading... '+m[1]+'%','');"
          "}else if(/^CONNECTING/i.test(line)){"
            "setStatus(checkStatus,'Connecting...','');"
          "}else if(/^DOWNLOADING/i.test(line)){"
            "setStatus(checkStatus,'Downloading...','');"
          "}else if(/rebooting/i.test(line)){"
            "verifyReboot(checkStatus,prevStatus,function(){btnCheck.disabled=false;btnInstall.disabled=true;btnForce.style.display='none';});"
          "}else{"
            "setStatus(checkStatus,line,/fail/i.test(line)?'err':'');"
          "}"
        "}"
      "}"
      "xhr.onprogress=poll;"
      "xhr.onreadystatechange=function(){if(xhr.readyState===3)poll();};"
      "xhr.onload=function(){"
        "poll();btnCheck.disabled=false;"
        "if(/fail/i.test(xhr.responseText)){"
          // Re-enable whichever of the two was actually the active path -
          // btnForce.style.display is only non-'none' while it's the one
          // shown (doCheck() above), so that alone tells us which.
          "if(btnForce.style.display==='none')btnInstall.disabled=false;"
          "else btnForce.disabled=false;"
        "}"
      "};"
      "xhr.onerror=function(){setStatus(checkStatus,'Connection lost - device is likely rebooting...','warn');};"
      "xhr.open('POST','/install');xhr.send();"
      "});"
    "}"
    "function doUpload(){"
      "var f=fwFile.files[0];"
      "if(!f){setStatus(uploadStatus,'Choose a .bin file first','err');return;}"
      "if(showCaution&&!confirm(cautionMsg+'\\n\\nUpload and install now anyway?'))return;"
      "btnUpload.disabled=true;"
      "setStatus(uploadStatus,'Uploading...','');"
      "fetchStatus(function(prevStatus){"
      "var fd=new FormData();fd.append('firmware',f);"
      "var xhr=new XMLHttpRequest();"
      "progressWrap.style.display='flex';progressFill.style.width='0%';progressText.textContent='0%';"
      "xhr.upload.onprogress=function(e){"
        "if(e.lengthComputable){"
          "var pct=Math.round(e.loaded/e.total*100);"
          "progressFill.style.width=pct+'%';progressText.textContent=pct+'%';"
        "}"
      "};"
      "xhr.onload=function(){"
        "setStatus(uploadStatus,xhr.responseText,xhr.status===200?'ok':'err');"
        "if(xhr.status!==200){btnUpload.disabled=false;}"
        "else if(/rebooting/i.test(xhr.responseText)){verifyReboot(uploadStatus,prevStatus,function(){btnUpload.disabled=false;});}"
      "};"
      "xhr.onerror=function(){setStatus(uploadStatus,'Connection lost - device is likely rebooting...','warn');};"
      "xhr.open('POST','/upload');xhr.send(fd);"
      "});"
    "}";

  page += "</script></body></html>";
  ota_server.send(200, "text/html; charset=utf-8", page);
}

// JSON so the page's install() can decide whether to confirm before
// force-installing a non-newer build, without re-parsing prose.
void ota_handle_check() {
  if (!ota_network_up()) { ota_server.send(503, "application/json", "{\"error\":\"No network connection\"}"); return; }

  long current, latest; bool newer;
  if (!ota_do_check(&current, &latest, &newer)) {
    ota_server.send(502, "application/json", "{\"error\":\"Could not reach update server\"}");
    return;
  }

  String resp = "{\"current\":"+String(current)+",\"latest\":"+String(latest)+",\"newer\":"+(newer ? "true" : "false")+"}";
  ota_server.send(200, "application/json", resp);
}

// Purely local (no flasher.rns.moscow round-trip, unlike /check above) -
// the web page's verifyReboot() polls this after triggering an install to
// find out when the device has come back up, and /check would be the
// wrong choice for that: it requires reaching the *external* update
// server too, so a slow/unreachable internet connection would report the
// device itself as unreachable even seconds after a perfectly successful
// reboot. ota_boot_id is what actually answers "did a reboot happen" -
// see its own comment.
void ota_handle_status() {
  String resp = "{\"current\":"+String(BUILD_NUMBER)+",\"boot_id\":"+String(ota_boot_id)+"}";
  ota_server.send(200, "application/json", resp);
}

// Shared by the web /install handler and Menu.h's F/W Update > Update
// (post-confirm) action. Downloads the image into the inactive partition
// and returns it on success - does NOT verify/reboot itself (see
// ota_verify_and_set_boot()/ota_reboot() above), since only the caller
// knows whether it still needs to do something (e.g. send an HTTP
// response) before the point of no return. Returns NULL on any failure,
// having already reset ota_in_progress/firmware_update_mode so the caller
// doesn't have to.
//
// Calls update_display() on every esp_https_ota_perform() chunk - without
// this, the whole download blocks loop() from ever reaching its own
// update_display() call, so firmware_update_mode's "OTA UPDATE" banner
// (Display.h) would never actually get pushed to the screen despite being
// set, just freezing on whatever was showing before.
const esp_partition_t* ota_do_pull_download(ota_status_cb_t status_cb = nullptr) {
  if (!ota_network_up()) { if (status_cb) status_cb("NO NETWORK"); return NULL; }
  if (ota_in_progress)   { if (status_cb) status_cb("BUSY"); return NULL; }

  const esp_partition_t *target = esp_ota_get_next_update_partition(NULL);
  if (target == NULL) { if (status_cb) status_cb("NO OTA SLOT"); return NULL; }

  ota_in_progress = true;
  firmware_update_mode = true;
  ota_dbg("Pulling update from "+String(OTA_BIN_URL));
  if (status_cb) status_cb("CONNECTING...");

  esp_http_client_config_t http_config = {};
  http_config.url = OTA_BIN_URL;
  http_config.crt_bundle_attach = esp_crt_bundle_attach;
  http_config.timeout_ms = OTA_HTTP_TIMEOUT_MS;

  esp_https_ota_config_t ota_config = {};
  ota_config.http_config = &http_config;

  // Masked for the whole begin/download/finish sequence - esp_https_ota_
  // perform() below writes each downloaded chunk straight to the inactive
  // OTA partition, real flash I/O spread over many calls across the whole
  // (multi-second) download. Same hazard class as feedback_dio0_isr_vs_
  // flash_io_crash/Menu.h's MSNGR_DELETE_CONFIRM: a DIO0 RX interrupt
  // landing while ESP-IDF has flash cache disabled for a write crashes
  // outright, not just races. Unlike those brief single-shot flash ops,
  // this one is unavoidably long - but the OTA page's own caution text
  // ("the radio is active... installing will interrupt operation") already
  // told the user to expect exactly that, so a clean radio pause for the
  // duration is the intended tradeoff, not a new regression.
  LoRa->maskDio0();
  esp_https_ota_handle_t ota_handle = NULL;
  esp_err_t err = esp_https_ota_begin(&ota_config, &ota_handle);
  if (err != ESP_OK) {
    LoRa->unmaskDio0();
    ota_dbg("esp_https_ota_begin failed");
    ota_in_progress = false; firmware_update_mode = false;
    if (status_cb) status_cb("DOWNLOAD FAILED");
    return NULL;
  }

  ota_dl_percent = -1;
  if (status_cb) status_cb("DOWNLOADING...");
  do {
    err = esp_https_ota_perform(ota_handle);

    // Confirmed live (task_wdt backtrace landing here, reset reason
    // TASK_WDT, reproduced consistently around 5% into the download):
    // same class of bug as the TX-queue-flush case above (its own comment
    // explains the general pattern) - loopTask is registered with the
    // task watchdog (CONFIG_ESP_TASK_WDT_TIMEOUT_S=5) and nothing else in
    // this call chain ever resets it, so a handful of back-to-back
    // esp_https_ota_perform() calls - individually unremarkable, entirely
    // normal over WiFi - cumulatively blows the 5s budget long before the
    // download itself is anywhere near done. The DIO0 masking above fixed
    // a real, separate crash class; this is what was actually rebooting
    // the device on every attempt.
    #if MCU_VARIANT == MCU_ESP32
      esp_task_wdt_reset();
    #endif

    // esp_https_ota_get_image_size() returns -1 for a chunked response
    // (no Content-Length) - flasher.rns.moscow's static .bin serving
    // always has one in practice, but this falls back to the old plain
    // "DOWNLOADING..." text rather than assuming it.
    int len_read = esp_https_ota_get_image_len_read(ota_handle);
    int total    = esp_https_ota_get_image_size(ota_handle);
    ota_dl_percent = (len_read >= 0 && total > 0) ? (int)(((int64_t)len_read * 100) / total) : -1;

    char dl_status[24];
    if (ota_dl_percent >= 0) snprintf(dl_status, sizeof(dl_status), "DOWNLOADING %d%%", ota_dl_percent);
    else strcpy(dl_status, "DOWNLOADING...");
    // Called every iteration now regardless of menu_is_open() (previously
    // only while the menu was open) - the web path's callback
    // (ota_stream_progress(), below) needs one call per chunk to stream
    // percent updates into the still-open /install response; the menu's
    // (menu_draw_popup) already expected per-iteration calls, so this is
    // unchanged for it beyond the string now carrying a real percentage.
    if (status_cb) status_cb(dl_status);

    // update_display() (Display.h) delegates to draw_settings_menu_disp()
    // whenever the menu is open, which redraws menu_state as-is - here
    // that's still MENU_STATE_FWUPD_CONFIRM (the static UPDATE/CANCEL
    // list), since nothing changes menu_state during the download itself.
    // Calling update_display() unconditionally would keep overwriting the
    // popup (drawn directly to hardware by status_cb, bypassing this path)
    // with that stale list on every redraw. So: let status_cb keep the
    // popup alive by itself while the menu owns the screen, and only fall
    // back to update_display() (for the web-triggered path's
    // firmware_update_mode banner) when it doesn't.
    #if HAS_MENU == true
      if (!menu_is_open() && disp_ready) { update_display(); }
    #else
      if (disp_ready) update_display();
    #endif
  } while (err == ESP_ERR_HTTPS_OTA_IN_PROGRESS);

  bool ok = false;
  if (err == ESP_OK && esp_https_ota_is_complete_data_received(ota_handle)) {
    ok = (esp_https_ota_finish(ota_handle) == ESP_OK);
  } else {
    ota_dbg("Download incomplete/failed, aborting");
    esp_https_ota_abort(ota_handle);
  }
  LoRa->unmaskDio0();
  ota_dl_percent = -1;

  if (ok) return target;

  firmware_update_mode = false;
  ota_in_progress = false;
  if (status_cb) status_cb("UPDATE FAILED");
  return NULL;
}

// Web progress callback for /install - the request is one long blocking
// call (ota_server.handleClient() doesn't return, and so can't service any
// other request, until this handler itself returns), so a separate
// polling endpoint the page could fetch mid-download isn't reachable.
// Instead this streams each update as its own line into the *same*
// response via chunked transfer encoding (ota_handle_install() opens it
// with setContentLength(CONTENT_LENGTH_UNKNOWN) before calling
// ota_do_pull_download()) - the page reads the growing response body as
// it arrives (XHR readyState 3/onprogress) instead of waiting for it to
// finish. Trades away a meaningful HTTP status code on failure (the 200
// is already committed by the time success/failure is known) for that -
// the page instead reads the final line's text.
void ota_stream_progress(const char *status) {
  ota_capture_status(status);
  ota_server.sendContent(String(status) + "\n");
}

void ota_handle_install() {
  ota_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  ota_server.send(200, "text/plain", "");
  const esp_partition_t *target = ota_do_pull_download(ota_stream_progress);
  if (target != NULL) {
    // Same DIO0-vs-flash-I/O hazard as ota_do_pull_download() itself - the
    // SHA-256 readback and esp_ota_set_boot_partition()'s otadata write are
    // both real flash I/O, just not masked internally by ota_verify_and_
    // set_boot() (shared with ota_check_recovery_button(), which runs
    // before LoRa/radio init and can't safely touch it) - so callers that
    // *do* have a live radio mask around the call themselves.
    LoRa->maskDio0();
    bool verified = ota_verify_and_set_boot(target);
    LoRa->unmaskDio0();
    if (verified) {
      ota_server.sendContent("Update downloaded, installing and rebooting...\n");
      ota_reboot(); // never returns
    }
    ota_capture_status("HASH VERIFY FAILED");
    firmware_update_mode = false;
    ota_in_progress = false;
  }
  ota_server.sendContent(String("Update failed: ")+ota_last_status+"\n");
}

void ota_handle_upload_chunk() {
  HTTPUpload &upload = ota_server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    ota_dbg("Upload starting: "+upload.filename);
    ota_in_progress = true;
    firmware_update_mode = true;
    ota_upload_ok = false;
    // Masked for the whole START..END/ABORTED session, same reasoning as
    // ota_do_pull_download()'s own comment - esp_ota_write() below is real
    // flash I/O, called once per chunk across the whole upload, and a DIO0
    // RX interrupt landing mid-write hits the same hard crash. Unmasked on
    // every exit below (END/ABORTED), never left masked across requests.
    LoRa->maskDio0();
    ota_upload_target = esp_ota_get_next_update_partition(NULL);
    if (ota_upload_target != NULL) {
      ota_upload_ok = (esp_ota_begin(ota_upload_target, OTA_SIZE_UNKNOWN, &ota_upload_handle) == ESP_OK);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (ota_upload_ok) {
      ota_upload_ok = (esp_ota_write(ota_upload_handle, upload.buf, upload.currentSize) == ESP_OK);
    }
    // Same task-watchdog exposure as the pull path's esp_https_ota_perform()
    // loop (see its own comment) - this callback is itself what's blocking
    // loop() across the whole upload, one call per chunk, with nothing else
    // in the chain ever resetting the watchdog.
    #if MCU_VARIANT == MCU_ESP32
      esp_task_wdt_reset();
    #endif
    // Same reasoning as the pull path's esp_https_ota_perform() loop - each
    // chunk callback is itself what's blocking loop(), so without this the
    // firmware_update_mode banner (Display.h) never gets pushed to screen.
    if (disp_ready) update_display();
  } else if (upload.status == UPLOAD_FILE_END) {
    if (ota_upload_ok) {
      ota_upload_ok = (esp_ota_end(ota_upload_handle) == ESP_OK);
    }
    LoRa->unmaskDio0();
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (ota_upload_handle) { esp_ota_end(ota_upload_handle); }
    ota_upload_ok = false;
    LoRa->unmaskDio0();
  }
}

void ota_handle_upload_complete() {
  // See ota_handle_install()'s own comment - ota_verify_and_set_boot()
  // isn't masked internally (shared with the pre-radio-init recovery
  // path), so this caller masks around it itself.
  LoRa->maskDio0();
  bool verified = ota_upload_ok && ota_upload_target != NULL && ota_verify_and_set_boot(ota_upload_target);
  LoRa->unmaskDio0();
  if (verified) {
    ota_server.send(200, "text/plain", "Upload complete, installing and rebooting...");
    ota_reboot(); // never returns
  }
  ota_dbg("Upload failed");
  firmware_update_mode = false;
  ota_in_progress = false;
  ota_server.send(500, "text/plain", "Upload failed - firmware unchanged.");
}

void ota_server_init() {
  ota_boot_id = esp_random();
  ota_server.on("/", HTTP_GET, ota_handle_root);
  ota_server.on("/check", HTTP_GET, ota_handle_check);
  ota_server.on("/status", HTTP_GET, ota_handle_status);
  ota_server.on("/install", HTTP_POST, ota_handle_install);
  ota_server.on("/upload", HTTP_POST, ota_handle_upload_complete, ota_handle_upload_chunk);
  ota_server.begin();
  ota_dbg("Web update server listening on port "+String(OTA_WEB_PORT));
}

void ota_loop() {
  ota_server.handleClient();
}

// RTC slow memory - not zero-initialized by the C runtime, but retained
// across every reset that keeps VDD3P3_RTC powered (software reset, panic,
// task-watchdog, deep sleep), and left holding stale/effectively-random
// content after an actual power-on-reset or brownout (that domain loses
// power too). Used purely as a "did we already switch partitions this
// power cycle" latch below, via an exact-match magic value that a fresh
// power-on essentially never happens to already contain.
RTC_NOINIT_ATTR uint32_t recovery_switch_guard;
#define RECOVERY_SWITCH_MAGIC 0x5AFEB007

// Field recovery path: hold pin_btn_usr1 down through power-on (or through
// an ongoing crash-bootloop - this runs before any of the code that might
// be crashing) to force-boot the *other* OTA partition. Called as the very
// first thing in setup() (RNode_Firmware.ino), before radio/display/network
// init, so it still runs even if a bad update breaks something later in
// boot - the one failure mode this can't help with is an image that doesn't
// get far enough to reach this line at all (see plan: no bootloader-level
// rollback is available through arduino-cli's prebuilt bootloader).
//
// recovery_switch_guard exists because this has no other debounce - if the
// running image keeps crashing and rebooting on its own (the exact scenario
// this exists to recover from), every one of those automatic reboots re-
// runs this function too. Holding the button through more than one of them
// used to flip the boot partition on *every* pass - good, bad, good, bad -
// with whichever slot happened to be selected the instant the button was
// physically released coming down to raw timing luck (confirmed on real
// hardware: the display never even got a chance to turn on while held,
// since each flip restarts before display init ever runs). The guard makes
// this a one-shot action per power cycle instead: a single continuous hold,
// however long, produces exactly one switch, and it stops mattering exactly
// when it gets released.
void ota_check_recovery_button() {
  pinMode(pin_btn_usr1, INPUT_PULLUP);
  delay(20);
  if (digitalRead(pin_btn_usr1) == LOW && recovery_switch_guard != RECOVERY_SWITCH_MAGIC) {
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *other = esp_ota_get_next_update_partition(running);
    if (other != NULL) {
      recovery_switch_guard = RECOVERY_SWITCH_MAGIC;
      // Re-registers dev_firmware_hash_target against the partition we're
      // switching to - the same call the normal OTA install path uses
      // (ota_handle_upload_complete()/ota_do_pull_download() above) to
      // satisfy Device.h's firmware-corrupt check. Without this, an
      // emergency switch left dev_firmware_hash_target pointing at whatever
      // was last installed via OTA - so the perfectly good partition being
      // switched *to* would fail that check on its very next boot and read
      // as "firmware corrupt" despite being the known-good image. This also
      // refuses to switch onto a partition that fails its own hash
      // verification (e.g. genuinely blank/never-flashed) rather than
      // committing to boot something worse than what's already failing.
      if (ota_verify_and_set_boot(other)) { esp_restart(); }
    }
  }
}
