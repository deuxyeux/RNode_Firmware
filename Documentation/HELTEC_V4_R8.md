# Heltec WiFi LoRa 32 V4 R8 (OLED)

Use the `heltec32v4pa_r8` PlatformIO environment for a host-controlled RNode,
or `heltec32v4pa_r8_urns` for the onboard Reticulum node. The plain
`heltec32v4pa` environment targets the R2 board and uses quad PSRAM.
These R8 environments support the OLED, not the optional TFT display.

## Pin mapping

The R8 has 8 MB octal PSRAM and 16 MB flash. Its memory configuration is
`qio_opi`, but its peripheral pins also differ from the R2 board:

| Function | R8 configuration |
| --- | --- |
| Peripheral power / Vext | GPIO40, active low |
| OLED-board LED | GPIO46 |
| Optional expansion GNSS power | GPIO42, active low |
| GNSS UART | RX39, TX38 |
| Battery measurement | GPIO1, 2.5 dB attenuation, divider factor 4.9 × 1.035 |
| RF front end | KCT8103L, power GPIO7, enable GPIO2, CTX GPIO5 |

GPIO33–37 belong to the octal PSRAM interface. Earlier R8 builds reused
R2 assignments for GPS enable (34), LEDs (35), Vext (36), and battery
measurement enable (37). Driving those pins can disrupt memory access.
The R8 battery setup must not drive GPIO37. GPIO40 must not also be driven
as a GPS standby signal.

References: [Meshtastic R8 variant](https://github.com/meshtastic/firmware/tree/develop/variants/esp32s3/heltec_v4_r8),
[Meshtastic board configuration](https://github.com/meshtastic/firmware/blob/develop/boards/heltec_v4_r8.json),
[Espressif GPIO restrictions](https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s3/api-reference/peripherals/gpio.html).

## Upload and verify

In PlatformIO's Project Tasks, select **heltec32v4pa_r8 → General → Upload**,
or use:

```sh
pio run -e heltec32v4pa_r8 -t upload
```

After upload, release BOOT/PRG and reset the board. Close the PlatformIO
serial monitor before using rnodeconf, and use the serial port that appears
after the firmware boots:

```sh
rnodeconf /dev/ttyACM0 --info
```

Replace the example port with the actual device port. A successful response
with an invalid/unprovisioned EEPROM still establishes that firmware detection
works. If detection fails, capture the reset log at 115200 baud and record
the exact upload environment and rnodeconf command.

In rnodeconf, `-p`/`--bluetooth-pair` requests Bluetooth pairing.
`-r`/`--rom` bootstraps EEPROM without flashing firmware. Holding the user
button for between five and seven seconds also requests pairing in this
firmware. Pairing is not required for USB provisioning.

This fork identifies the R8 as board `0xF8`, product `0xCB`, model `0xC8`.
Older rnodeconf automatic installers may only list the R2 V4; selecting that
entry can install the wrong firmware. Confirm tool support and the device's
current EEPROM state before provisioning or using automatic installation.

For an unprovisioned board, create a signing key if needed (an existing key
is preserved), then provision the R8 identity:

```sh
rnodeconf --key
rnodeconf /dev/ttyACM0 --rom --product cb --model c8 --hwrev 1
```

Then register the firmware hash as described below. Older rnodeconf versions
may fail to display product `0xCB` with `--info` even after provisioning succeeds.

## Firmware hash after a PlatformIO upload

EEPROM identity provisioning (`--rom`) alone does not register the expected
firmware hash. A missing or stale hash causes the screen to report
"Firmware Corrupt" even when the uploaded image is intact.

Read the running image's hash with `rnodeconf PORT --get-firmware-hash`.
Compare it with the embedded SHA256 of the exact application binary uploaded.
For the standard R8 build, print and verify that embedded checksum with:

```sh
python3 - <<'PY'
from pathlib import Path
import hashlib
data = Path('.pio_build/heltec32v4pa_r8/firmware.bin').read_bytes()
digest = hashlib.sha256(data[:-32]).digest()
assert digest == data[-32:], 'Application image checksum mismatch'
print(digest.hex())
PY
```

Use the `heltec32v4pa_r8_urns` directory instead for the onboard-node build.
When the two hashes match, run `rnodeconf PORT --firmware-hash HASH`, replacing
PORT and HASH, then reset the board. Repeat after uploading a changed image.
Do not use the hash of the factory/merged binary or the whole application
file; the ESP32 application hash excludes its appended digest.

The pin corrections have been compile-checked for R8, R8 URNS, and R2.
User testing confirmed successful R8 operation after provisioning and firmware
hash registration. Battery calibration, radio performance, sleep/wake, and
optional expansion GNSS have not been independently measured.
