#!/usr/bin/env bash
# Regenerates .pio_vendor/heltec_nrf52_framework/ - a small symlink shim
# both [env:heltec_t096] and [env:heltec_t114] in platformio.ini need
# (same underlying Heltec_nRF52 core, different board variant within it),
# gitignored since the symlinks are machine-specific absolute paths. See
# platformio.ini's own comment on why this exists (PlatformIO's manifest
# auto-discovery mis-identifies Heltec's raw repo root - it finds an
# unrelated tools/midi_tests/package.json before any real one, and treats
# that whole subdirectory as the entire package - so a proper root
# package.json plus symlinks to just the pieces actually needed,
# cores/variants/libraries/boards.txt/platform.txt, sidesteps that instead
# of pointing PlatformIO at the raw Heltec_nRF52 checkout directly).
#
# Requires arduino-cli's Heltec_nRF52 board package already installed:
#   arduino-cli core install Heltec_nRF52:Heltec_nRF52
set -euo pipefail

HELTEC_CORE="$HOME/.arduino15/packages/Heltec_nRF52/hardware/Heltec_nRF52"
VENDOR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.pio_vendor/heltec_nrf52_framework"

if [ ! -d "$HELTEC_CORE" ]; then
  echo "error: $HELTEC_CORE not found - run 'arduino-cli core install Heltec_nRF52:Heltec_nRF52' first" >&2
  exit 1
fi

mkdir -p "$VENDOR_DIR"
ln -sfn "$HELTEC_CORE/cores" "$VENDOR_DIR/cores"
ln -sfn "$HELTEC_CORE/variants" "$VENDOR_DIR/variants"
ln -sfn "$HELTEC_CORE/libraries" "$VENDOR_DIR/libraries"
ln -sfn "$HELTEC_CORE/boards.txt" "$VENDOR_DIR/boards.txt"
ln -sfn "$HELTEC_CORE/platform.txt" "$VENDOR_DIR/platform.txt"

cat > "$VENDOR_DIR/package.json" <<'EOF'
{
  "name": "framework-arduinoadafruitnrf52",
  "version": "1.6.0",
  "description": "Heltec's fork of the Adafruit nRF52 Arduino core, vendored via symlinks from the arduino-cli-installed Heltec_nRF52 board package for PlatformIO's heltec_t096/heltec_t114 envs. See platformio.ini's own comment on [env:heltec_t096] and setup_heltec_nrf52_pio.sh.",
  "keywords": ["framework", "arduino", "nordic semiconductor", "nrf52"],
  "url": "https://github.com/HelTecAutomation/Heltec_nRF52"
}
EOF

echo "Done: $VENDOR_DIR"
