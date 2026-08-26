#!/usr/bin/env bash
# Regenerates .pio_vendor/rak4631_nrf52_framework/ - a small symlink shim
# [env:rak4631] in platformio.ini needs. Same shape/reasoning as
# setup_heltec_nrf52_pio.sh (see that script's own comment and
# [env:heltec_t096]'s in platformio.ini): rakwireless:nrf52's own raw core
# checkout has the exact same poison-pill problem (a stray
# tools/midi_tests/package.json found by PlatformIO's recursive manifest
# search before any real root package.json), so it's vendored through the
# same symlinks-only shim instead of pointing symlink:// straight at the
# arduino-cli-installed directory.
#
# Requires arduino-cli's rakwireless:nrf52 board package already installed:
#   arduino-cli core install rakwireless:nrf52
set -euo pipefail

RAK_CORE="$HOME/.arduino15/packages/rakwireless/hardware/nrf52/1.3.3"
VENDOR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.pio_vendor/rak4631_nrf52_framework"

if [ ! -d "$RAK_CORE" ]; then
  echo "error: $RAK_CORE not found - run 'arduino-cli core install rakwireless:nrf52' first (and check the installed version matches this script's RAK_CORE path if it's been updated since)" >&2
  exit 1
fi

mkdir -p "$VENDOR_DIR"
ln -sfn "$RAK_CORE/cores" "$VENDOR_DIR/cores"
ln -sfn "$RAK_CORE/variants" "$VENDOR_DIR/variants"
ln -sfn "$RAK_CORE/libraries" "$VENDOR_DIR/libraries"
ln -sfn "$RAK_CORE/boards.txt" "$VENDOR_DIR/boards.txt"
ln -sfn "$RAK_CORE/platform.txt" "$VENDOR_DIR/platform.txt"

cat > "$VENDOR_DIR/package.json" <<'EOF'
{
  "name": "framework-arduinoadafruitnrf52",
  "version": "1.3.3",
  "description": "RAKwireless's fork of the Adafruit nRF52 Arduino core, vendored via symlinks from the arduino-cli-installed rakwireless:nrf52 board package for PlatformIO's [env:rak4631] (WisBlock RAK4631). See platformio.ini's own comment on [env:rak4631] and setup_rak4631_nrf52_pio.sh.",
  "keywords": ["framework", "arduino", "nordic semiconductor", "nrf52"],
  "url": "https://github.com/RAKWireless/WisBlock"
}
EOF

echo "Done: $VENDOR_DIR"
