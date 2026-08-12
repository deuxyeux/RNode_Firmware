// KISS-local + RNS-remote wiring for microReticulum's already-vendored
// RNS::Provisioning::Provisioner (lib/microReticulum/src/microReticulum/
// Provisioning/) - a typed/namespaced remote-configuration engine that
// shipped fully built but dormant until now. Ported from
// microReticulum_Firmware's "RNS Remote Provisioning Support" /
// "Expanded Provisioning System" changelog entries.
//
// Two entry points into the same Provisioner singleton:
//   - Local: CMD_PROVISION_REQ/RSP over the existing KISS/USB link
//     (on_provision_request(), below - called from serial_callback(),
//     RNode_Firmware.ino).
//   - Remote: over an RNS Link to the "remote.management" destination's
//     "/provision" request handler (Transport::remote_provision_handler(),
//     already implemented in the vendored library) - reachable once
//     RNS::Reticulum::remote_management_enabled(true) is set (URNS.h,
//     urns_init()) AND the caller's identity hash has been added to
//     Transport::remote_management_allowed() (empty/deny-all by default;
//     settable as a field in the builtin "General Config" namespace, so
//     an operator bootstraps it via local KISS provisioning first - same
//     chicken-and-egg resolution microReticulum_Firmware's own web
//     console uses).
//
// Depends on URNS.h having already run urns_init() successfully
// (urns_ready) - provisioning_init() (called right after messenger_init(),
// RNode_Firmware.ino) assumes the "urns" LittleFS partition is mounted and
// registered with RNS::Utilities::OS.

#ifndef PROVISIONING_H
  #define PROVISIONING_H

  #define PROVISIONING_STORAGE_PATH URNS_BASE_PATH "/provisioning"

  void provisioning_init() {
    RNS::Provisioning::Provisioner& provisioner = RNS::Provisioning::Provisioner::instance();
    // Active op: client sent an explicit Reboot wire command. Same reset
    // path the on-device Settings menu and CMD_RESET already use.
    provisioner.on_reboot([]() {
      DEBUG_LOG("[PROV] reboot requested via Provisioning\r\n");
      hard_reset();
    });
    provisioner.begin(PROVISIONING_STORAGE_PATH);
    DEBUG_LOG("[PROV] Provisioner started, schema_hash=0x%08lX\r\n", (unsigned long)provisioner.schema_hash());
  }

  // Frames a Provisioner response the same way every other multi-byte
  // KISS reply in this firmware does (see e.g. kiss_indicate_sensor(),
  // Utilities.h) - FEND, command byte, escaped payload, FEND.
  void kiss_indicate_provision_response(const RNS::Bytes& response) {
    #if HAS_ESPNOW == true
      kiss_select_interface(0);
    #endif
    serial_write(FEND);
    serial_write(CMD_PROVISION_RSP);
    const uint8_t* data = response.data();
    size_t len = response.size();
    for (size_t i = 0; i < len; i++) escaped_serial_write(data[i]);
    serial_write(FEND);
  }

  // Called from serial_callback() (RNode_Firmware.ino) once a complete
  // CMD_PROVISION_REQ frame has been unescaped into prov_req_buf.
  void on_provision_request(const uint8_t* buf, size_t len) {
    if (!urns_ready) return;
    RNS::Bytes request(buf, len);
    RNS::Bytes response = RNS::Provisioning::Provisioner::instance().handle_message(request);
    kiss_indicate_provision_response(response);
  }

#endif
