#!/usr/bin/env bash
# Regenerates .pio_vendor/techo_nrf52_framework/ - a small symlink shim
# [env:techo] in platformio.ini needs. Same shape/reasoning as
# setup_heltec_nrf52_pio.sh (see that script's own comment and
# [env:heltec_t096]'s in platformio.ini): adafruit:nrf52's own raw core
# checkout has the exact same poison-pill problem (a stray
# tools/midi_tests/package.json found by PlatformIO's recursive manifest
# search before any real root package.json), so it's vendored through the
# same symlinks-only shim instead of pointing symlink:// straight at the
# arduino-cli-installed directory.
#
# The Makefile's own firmware-techo/release-techo targets build LilyGO
# T-Echo (Boards.h: BOARD_TECHO, 0x44) against the generic Nordic
# nRF52840-DK profile (adafruit:nrf52:pca10056) rather than a
# T-Echo-specific FQBN - no such FQBN exists in this core, T-Echo's
# hardware (nRF52840 + S140 softdevice, same GxEPD2 e-ink display class)
# is close enough to the DK's own that this has always been the build
# target used. Mirrored here as-is.
#
# Requires arduino-cli's adafruit:nrf52 board package already installed:
#   arduino-cli core install adafruit:nrf52
set -euo pipefail

ADAFRUIT_CORE="$HOME/.arduino15/packages/adafruit/hardware/nrf52/1.7.0"
VENDOR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.pio_vendor/techo_nrf52_framework"

if [ ! -d "$ADAFRUIT_CORE" ]; then
  echo "error: $ADAFRUIT_CORE not found - run 'arduino-cli core install adafruit:nrf52' first (and check the installed version matches this script's ADAFRUIT_CORE path if it's been updated since)" >&2
  exit 1
fi

mkdir -p "$VENDOR_DIR"
ln -sfn "$ADAFRUIT_CORE/cores" "$VENDOR_DIR/cores"
ln -sfn "$ADAFRUIT_CORE/variants" "$VENDOR_DIR/variants"
ln -sfn "$ADAFRUIT_CORE/libraries" "$VENDOR_DIR/libraries"
ln -sfn "$ADAFRUIT_CORE/boards.txt" "$VENDOR_DIR/boards.txt"
ln -sfn "$ADAFRUIT_CORE/platform.txt" "$VENDOR_DIR/platform.txt"

cat > "$VENDOR_DIR/package.json" <<'EOF'
{
  "name": "framework-arduinoadafruitnrf52",
  "version": "1.7.0",
  "description": "Adafruit's own nRF52 Arduino core, vendored via symlinks from the arduino-cli-installed adafruit:nrf52 board package for PlatformIO's [env:techo] (LilyGO T-Echo, built against the generic pca10056/nRF52840-DK profile). See platformio.ini's own comment on [env:techo] and setup_techo_nrf52_pio.sh.",
  "keywords": ["framework", "arduino", "nordic semiconductor", "nrf52"],
  "url": "https://github.com/adafruit/Adafruit_nRF52_Arduino"
}
EOF

echo "Done: $VENDOR_DIR"
