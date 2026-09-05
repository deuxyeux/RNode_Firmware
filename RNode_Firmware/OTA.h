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
// as RTC.h's rtc_sync_status_cb_t/rtc_sync_ntp(). Web callers pass nullptr
// (an HTTP client isn't watching a live screen); the menu passes
// menu_draw_popup so its screen stays live for the whole blocking transfer.
typedef void (*ota_status_cb_t)(const char *status);

WebServer ota_server(OTA_WEB_PORT);
bool ota_in_progress = false;

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

void ota_handle_root() {
  String page = "<!doctype html><html><head><title>RNode Firmware Update</title>";
  page += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>";
  page += "<h1>RNode Firmware Update</h1>";
  // BUILD_NUMBER==0 means this build didn't come through the Makefile/
  // platformio.ini's git-commit-count injection at all (see BUILD_NUMBER's
  // own fallback, Boards.h) - unknown, not a real build 0, so the line is
  // omitted rather than showing a misleading "Build: 0".
  page += "<p>RNode Firmware version: " + ota_current_version();
  if (BUILD_NUMBER != 0) page += "<br>Build: " + String(BUILD_NUMBER);
  page += "</p>";
  page += "<h2>Check server for an update</h2>";
  page += "<button onclick=\"check()\">Check for Update</button> ";
  page += "<button onclick=\"install()\">Install Latest</button>";
  page += "<pre id=\"status\"></pre>";
  page += "<h2>Or upload a .bin directly</h2>";
  page += "<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">";
  page += "<input type=\"file\" name=\"firmware\"> <input type=\"submit\" value=\"Upload &amp; Install\"></form>";
  page += "<script>";
  page += "var proto='"+ota_current_version()+"';";
  page += "function setStatus(t){document.getElementById('status').innerText=t;}";
  page += "function check(){return fetch('/check').then(r=>r.json()).then(d=>{"
          "if(d.error){setStatus(d.error);return d;}"
          "setStatus('Current: '+proto+'.'+d.current+'\\nLatest:  '+proto+'.'+d.latest+'\\n'+(d.newer?'Update available.':'Already up to date.'));"
          "return d;});}";
  page += "function doInstall(){setStatus('Installing...');fetch('/install',{method:'POST'}).then(r=>r.text()).then(setStatus);}";
  page += "function install(){check().then(d=>{"
          "if(!d||d.error)return;"
          "if(!d.newer){"
          "if(!confirm('Server build ('+proto+'.'+d.latest+') is not newer than current build ('+proto+'.'+d.current+'). Install anyway?')){setStatus('Cancelled.');return;}"
          "}"
          "doInstall();});}";
  page += "</script></body></html>";
  ota_server.send(200, "text/html", page);
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

  esp_https_ota_handle_t ota_handle = NULL;
  esp_err_t err = esp_https_ota_begin(&ota_config, &ota_handle);
  if (err != ESP_OK) {
    ota_dbg("esp_https_ota_begin failed");
    ota_in_progress = false; firmware_update_mode = false;
    if (status_cb) status_cb("DOWNLOAD FAILED");
    return NULL;
  }

  if (status_cb) status_cb("DOWNLOADING...");
  do {
    err = esp_https_ota_perform(ota_handle);
    // update_display() (Display.h) delegates to draw_settings_menu_disp()
    // whenever the menu is open, which redraws menu_state as-is - here
    // that's still MENU_STATE_FWUPD_CONFIRM (the static UPDATE/CANCEL
    // list), since nothing changes menu_state during the download itself.
    // Calling update_display() unconditionally would keep overwriting the
    // "DOWNLOADING..." popup (drawn directly to hardware by status_cb,
    // bypassing this path) with that stale list on every redraw. So: let
    // status_cb keep the popup alive by itself while the menu owns the
    // screen, and only fall back to update_display() (for the web-
    // triggered path's firmware_update_mode banner) when it doesn't.
    #if HAS_MENU == true
      if (menu_is_open()) { if (status_cb) status_cb("DOWNLOADING..."); }
      else if (disp_ready) { update_display(); }
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

  if (ok) return target;

  firmware_update_mode = false;
  ota_in_progress = false;
  if (status_cb) status_cb("UPDATE FAILED");
  return NULL;
}

void ota_handle_install() {
  const esp_partition_t *target = ota_do_pull_download(ota_capture_status);
  if (target != NULL) {
    if (ota_verify_and_set_boot(target)) {
      ota_server.send(200, "text/plain", "Update downloaded, installing and rebooting...");
      ota_reboot(); // never returns
    }
    ota_capture_status("HASH VERIFY FAILED");
    firmware_update_mode = false;
    ota_in_progress = false;
  }
  ota_server.send(500, "text/plain", String("Update failed: ")+ota_last_status);
}

void ota_handle_upload_chunk() {
  HTTPUpload &upload = ota_server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    ota_dbg("Upload starting: "+upload.filename);
    ota_in_progress = true;
    firmware_update_mode = true;
    ota_upload_ok = false;
    ota_upload_target = esp_ota_get_next_update_partition(NULL);
    if (ota_upload_target != NULL) {
      ota_upload_ok = (esp_ota_begin(ota_upload_target, OTA_SIZE_UNKNOWN, &ota_upload_handle) == ESP_OK);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (ota_upload_ok) {
      ota_upload_ok = (esp_ota_write(ota_upload_handle, upload.buf, upload.currentSize) == ESP_OK);
    }
    // Same reasoning as the pull path's esp_https_ota_perform() loop - each
    // chunk callback is itself what's blocking loop(), so without this the
    // firmware_update_mode banner (Display.h) never gets pushed to screen.
    if (disp_ready) update_display();
  } else if (upload.status == UPLOAD_FILE_END) {
    if (ota_upload_ok) {
      ota_upload_ok = (esp_ota_end(ota_upload_handle) == ESP_OK);
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (ota_upload_handle) { esp_ota_end(ota_upload_handle); }
    ota_upload_ok = false;
  }
}

void ota_handle_upload_complete() {
  if (ota_upload_ok && ota_upload_target != NULL && ota_verify_and_set_boot(ota_upload_target)) {
    ota_server.send(200, "text/plain", "Upload complete, installing and rebooting...");
    ota_reboot(); // never returns
  }
  ota_dbg("Upload failed");
  firmware_update_mode = false;
  ota_in_progress = false;
  ota_server.send(500, "text/plain", "Upload failed - firmware unchanged.");
}

void ota_server_init() {
  ota_server.on("/", HTTP_GET, ota_handle_root);
  ota_server.on("/check", HTTP_GET, ota_handle_check);
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
