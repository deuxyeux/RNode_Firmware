#!/usr/bin/env python3
# Generates manifest.json describing ESP32 flash offsets for one
# PlatformIO env, for bundling into that board's release firmware zip.
#
# This lets a web flasher (or anything using esptool/esptool-js) flash an
# arbitrary RNode firmware zip without a hardcoded per-board offset table -
# it just reads manifest.json out of the zip.
#
# Sources of truth, in priority order:
#   - flash_mode/flash_freq: NOT derived from PlatformIO board metadata -
#     that reflects a board's nominal capability, not what's actually safe
#     to flash with. The RNode Flasher web app has flashed every one of
#     these boards for years via esptool-js with flash_mode/flash_freq
#     hardcoded to 'dio'/'80m' (htdocs/index.html, esploader.writeFlash
#     call) regardless of chip or board default - including boards whose
#     PlatformIO board.json nominally defaults to qio (e.g.
#     esp32-s3-devkitc-1). Matching that field-proven pair here rather than
#     e.g. trusting a board's "qio" default, which for meshadventurer_s3
#     doesn't even match what that env actually compiles at (its combined
#     ESP-IDF build's own flasher_args.json says dio, sdkconfig.defaults
#     override) - two disagreeing "authoritative" sources, so don't guess;
#     use the one this project has actually shipped for years instead.
#   - flash_size: this DOES vary per board (physical flash chip size) and
#     PlatformIO's board default is not reliable for it either - several
#     boards here reuse the generic "esp32-s3-devkitc-1" PlatformIO board
#     entry (nominal 8MB) despite shipping with a smaller physical chip
#     (see the flash_size overrides added to platformio.ini alongside this
#     script, sourced from htdocs/index.html's already field-validated
#     per-board fc(...) sz values).
#   - bootloader offset: not part of any board manifest or partitions.csv -
#     it's a fixed per-chip ROM/2nd-stage-loader constant (0x1000 for the
#     original ESP32, 0x0 for every later chip with a USB-Serial/JTAG ROM
#     download path). Hardcoded below; this has been stable across every
#     Espressif chip released to date.
#   - partition table offset: always 0x8000 (CONFIG_PARTITION_TABLE_OFFSET
#     default; none of this project's partition CSVs override it).
#   - app offset and whether boot_app0.bin is needed: parsed directly from
#     the env's board_build.partitions CSV (first "app" row for the app
#     offset; presence of a "data"/"ota" row - conventionally named
#     "otadata" - means the OTA-select seed needs to be written there).

import configparser
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
INI_PATH = REPO_ROOT / "platformio.ini"

PARTITION_TABLE_OFFSET = 0x8000

BOOTLOADER_OFFSET = {
    "esp32": 0x1000,
    "esp32s2": 0x1000,
    "esp32s3": 0x0,
    "esp32c2": 0x0,
    "esp32c3": 0x0,
    "esp32c6": 0x0,
    "esp32h2": 0x0,
}


def load_env(env_name):
    cp = configparser.ConfigParser(interpolation=None)
    cp.read(INI_PATH)
    section = f"env:{env_name}"
    if section not in cp:
        raise SystemExit(f"gen_manifest: no [env:{env_name}] in {INI_PATH}")
    return cp[section]


def parse_partitions_csv(path):
    rows = []
    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        fields = [f.strip() for f in line.split(",")]
        if len(fields) < 5:
            continue
        name, type_, subtype, offset, size = fields[:5]
        rows.append(
            {
                "name": name,
                "type": type_,
                "subtype": subtype,
                "offset": int(offset, 0),
            }
        )
    return rows


def resolve_partitions_csv(value, framework_dir):
    candidates = [REPO_ROOT / value, framework_dir / "tools" / "partitions" / Path(value).name]
    for c in candidates:
        if c.exists():
            return c
    raise SystemExit(f"gen_manifest: can't resolve partitions CSV {value!r} (tried {candidates})")


# What RNode Flasher has actually shipped to every ESP32/ESP32-S3 board for
# years (htdocs/index.html, esploader.writeFlash call) - not derived from
# PlatformIO board metadata, see module comment above.
FLASH_MODE = "dio"
FLASH_FREQ = "80m"


def build_manifest(env_name, release_basename):
    env = load_env(env_name)
    board_id = env["board"]

    from platformio.platform.factory import PlatformFactory

    platform = PlatformFactory.new(env.get("platform"))
    board_config = platform.board_config(board_id)
    framework_dir = Path(platform.get_package_dir("framework-arduinoespressif32"))

    mcu = board_config.get("build", {}).get("mcu", "").lower()
    if mcu not in BOOTLOADER_OFFSET:
        raise SystemExit(f"gen_manifest: unknown chip mcu {mcu!r} for board {board_id!r} - add it to BOOTLOADER_OFFSET")

    flash_size = env.get("board_upload.flash_size", board_config.get("upload", {}).get("flash_size"))

    partitions_csv = resolve_partitions_csv(env["board_build.partitions"], framework_dir)
    rows = parse_partitions_csv(partitions_csv)

    app_row = next((r for r in rows if r["type"] == "app"), None)
    if app_row is None:
        raise SystemExit(f"gen_manifest: no app partition in {partitions_csv}")

    otadata_row = next((r for r in rows if r["type"] == "data" and r["subtype"] == "ota"), None)

    flash_files = {
        hex(BOOTLOADER_OFFSET[mcu]): f"{release_basename}.bootloader",
        hex(PARTITION_TABLE_OFFSET): f"{release_basename}.partitions",
        hex(app_row["offset"]): f"{release_basename}.bin",
    }
    if otadata_row is not None:
        flash_files[hex(otadata_row["offset"])] = f"{release_basename}.boot_app0"

    return {
        # Lets a flasher identify which catalog device/variant a zip is for
        # from the manifest itself, not the outer zip's filename (which a
        # browser download or a user rename can change).
        "variant": release_basename,
        "chip_family": mcu,
        "flash_size": flash_size,
        "flash_mode": FLASH_MODE,
        "flash_freq": FLASH_FREQ,
        "flash_files": flash_files,
    }


def main():
    if len(sys.argv) != 4:
        raise SystemExit(f"usage: {sys.argv[0]} <pio_env> <release_basename> <output_path>")
    env_name, release_basename, out_path = sys.argv[1:4]
    manifest = build_manifest(env_name, release_basename)
    out = Path(out_path)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(manifest, indent=2) + "\n")


if __name__ == "__main__":
    main()
