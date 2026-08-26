#!/usr/bin/env bash
# Regenerates .pio_vendor/promicro_nrf52_framework/ - a small symlink shim
# [env:promicro] in platformio.ini needs. Same shape/reasoning as
# setup_heltec_nrf52_pio.sh (see that script's own comment and
# [env:heltec_t096]'s in platformio.ini), but for a DIFFERENT third-party
# core (promicro:nrf52, github.com/pdcook/nRFMicro-Arduino-Core - the
# nice!nano v2 board this firmware's "promicro" target actually builds
# for), since PlatformIO's own nordicnrf52 platform doesn't know either
# vendor core.
#
# Unlike Heltec's core, this one already has a proper root package.json
# (name "nrfmicro-arduino") - no poison-pill-manifest problem here, so in
# principle a direct symlink:// straight at the arduino-cli-installed
# directory would resolve fine. Vendored through the same small-shim
# pattern anyway for consistency with the Heltec envs (own package.json
# named "framework-arduinoadafruitnrf52" - the name PlatformIO's
# platform_packages override actually keys on, not the source's own name)
# and so a future core update that ever reintroduces a stray nested
# package.json can't silently break this the way it did for Heltec.
#
# Requires arduino-cli's promicro:nrf52 board package already installed:
#   arduino-cli core install promicro:nrf52
set -euo pipefail

PROMICRO_CORE="$HOME/.arduino15/packages/promicro/hardware/nrf52/1.0.2"
VENDOR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.pio_vendor/promicro_nrf52_framework"

if [ ! -d "$PROMICRO_CORE" ]; then
  echo "error: $PROMICRO_CORE not found - run 'arduino-cli core install promicro:nrf52' first (and check the installed version matches this script's PROMICRO_CORE path if it's been updated since)" >&2
  exit 1
fi

mkdir -p "$VENDOR_DIR"
ln -sfn "$PROMICRO_CORE/cores" "$VENDOR_DIR/cores"
ln -sfn "$PROMICRO_CORE/variants" "$VENDOR_DIR/variants"
ln -sfn "$PROMICRO_CORE/libraries" "$VENDOR_DIR/libraries"
ln -sfn "$PROMICRO_CORE/boards.txt" "$VENDOR_DIR/boards.txt"
ln -sfn "$PROMICRO_CORE/platform.txt" "$VENDOR_DIR/platform.txt"

cat > "$VENDOR_DIR/package.json" <<'EOF'
{
  "name": "framework-arduinoadafruitnrf52",
  "version": "1.0.2",
  "description": "promicro:nrf52 core (pdcook/nRFMicro-Arduino-Core), vendored via symlinks from the arduino-cli-installed promicro board package for PlatformIO's [env:promicro] (nice!nano v2). See platformio.ini's own comment on [env:promicro] and setup_promicro_nrf52_pio.sh.",
  "keywords": ["framework", "arduino", "nordic semiconductor", "nrf52"],
  "url": "https://github.com/pdcook/nRFMicro-Arduino-Core"
}
EOF

echo "Done: $VENDOR_DIR"
