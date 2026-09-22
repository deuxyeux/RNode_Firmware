#!/usr/bin/env bash
# Wraps `pio run -e <env>` for boards whose combined "arduino, espidf" build
# needs sdkconfig.defaults values that differ from the project-wide defaults
# committed there (meshadventurer_s3/meshpoe_s3's real Octal PSRAM chip).
#
# Why this exists: sdkconfig.defaults is a single shared file with no
# per-env scoping under this platform version (espidf.py disables the
# custom_sdkconfig mechanism whenever "espidf" is in an env's framework
# list). heltec32v4pa_urns_ble's real PSRAM chip is Quad/2MB, not Octal, and
# boots into a crash loop under the Octal settings meshadventurer_s3/
# meshpoe_s3 need. This used to require a manual edit-build-revert dance
# (see sdkconfig.defaults's own PSRAM comment and project memory
# "Heltec32-V4 BLE-keyboard PSRAM crash") - skipping the revert step is what
# breaks the real boards' PSRAM silently, and skipping the swap step is what
# breaks heltec32v4pa_urns_ble silently (both have happened). This script
# makes both directions automatic so a mass-build loop can call `pio run`
# for every env uniformly without knowing about this quirk.
#
# Usage: scripts/pio_build.sh <env> [extra pio args, e.g. -t upload]
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "usage: $0 <env> [pio args...]" >&2
  exit 1
fi

ENV_NAME="$1"; shift

cd "$(git rev-parse --show-toplevel)"

SDKCONFIG_DEFAULTS="sdkconfig.defaults"

# Refuse to run over pre-existing local edits to sdkconfig.defaults - this
# script's cleanup does a hard `git checkout --`, which would silently
# discard real in-progress work on that file otherwise.
if ! git diff --quiet -- "$SDKCONFIG_DEFAULTS" || ! git diff --cached --quiet -- "$SDKCONFIG_DEFAULTS"; then
  echo "error: $SDKCONFIG_DEFAULTS has uncommitted changes - commit/stash them first" >&2
  echo "(this script needs to know the file's committed state is the correct one to restore)" >&2
  exit 1
fi

needs_quad_psram() {
  case "$1" in
    heltec32v4pa_urns_ble) return 0 ;;
    *) return 1 ;;
  esac
}

cleanup() {
  git checkout -- "$SDKCONFIG_DEFAULTS"
  rm -f "sdkconfig.${ENV_NAME}"
}
trap cleanup EXIT

if needs_quad_psram "$ENV_NAME"; then
  # Stale per-env sdkconfig cache files only fill in *unset* Kconfig options
  # on top of whatever they already have cached - if this exists from a
  # prior build, our sed edit below would silently be ignored.
  rm -f "sdkconfig.${ENV_NAME}"
  sed -i \
    -e 's/^CONFIG_SPIRAM_MODE_OCT=y$/CONFIG_SPIRAM_MODE_QUAD=y/' \
    -e 's/^CONFIG_SPIRAM_TYPE_AUTO=y$/CONFIG_SPIRAM_TYPE_ESPPSRAM16=y/' \
    "$SDKCONFIG_DEFAULTS"
  if ! grep -q '^CONFIG_SPIRAM_MODE_QUAD=y$' "$SDKCONFIG_DEFAULTS"; then
    echo "error: expected sed substitution on $SDKCONFIG_DEFAULTS didn't match - file layout changed, fix this script" >&2
    exit 1
  fi
fi

pio run -e "$ENV_NAME" "$@"
