---
name: mass-build-deploy
description: Use when asked to build RNode_Firmware for all (or most) board targets and deploy/refresh the results on the flasher site (flasher.rns.moscow). Triggers on "build all targets", "compile for all boards", "refresh the flasher", "update the firmware archive", or similar mass-build-and-publish requests.
---

# Mass-building and deploying RNode_Firmware to the flasher

This is a long-running (30-70 minute), multi-step batch job — not a single command. Plan for background execution and expect to hit real compile bugs along the way, not just build-environment flukes (see "Don't dismiss failures" below).

**Build tool: PlatformIO (`pio run -e <env>`), not arduino-cli/Makefile.** The `Makefile`'s `release-<target>` recipes (arduino-cli-based) are what this skill used before the project's PlatformIO migration, and are kept only for reference/comparison — arduino-cli has had real breakage on GNSS-touching boards on this codebase's active branch (Library Manager 403s fetching TinyGPSPlus), which PlatformIO's vendored-library builds don't hit. Every board in scope here has a matching `[env:<name>]` in `platformio.ini` with the exact same name as its `release-<target>`/deployed-filename board name (verified 1:1 as of this writing) — if a newly-added board doesn't, treat that the same as a missing recipe (step 1: ask, don't guess).

## 0. Version directory and build number

```
PROTO_VERSION="$(grep 'define MAJ_VERS' RNode_Firmware/Config.h | awk '{print strtonum($3)}').$(grep 'define MIN_VERS' RNode_Firmware/Config.h | awk '{print strtonum($3)}')"
BUILD_NUMBER="$(git rev-list --count HEAD)"
VERSION_DIR="${PROTO_VERSION}.${BUILD_NUMBER}"   # e.g. 1.86.994
```

`BUILD_NUMBER` is `git rev-list --count HEAD` — it only changes across commits, not uncommitted edits. If deploying from an uncommitted working tree (confirm with the user first — the live production binaries then won't trace back to any commit), `VERSION_DIR` will match whatever directory was last deployed at the same HEAD; that's fine, it just means refreshing that directory's contents in place rather than creating a new one.

## 1. Establish the target list from the deployed directory, not just the Makefile/platformio.ini

Before building anything, list what's actually deployed and treat it as the source of truth for what needs refreshing:

```
ssh srv1 "ls /var/www/flasher.rns.moscow/htdocs/firmware/classic/<version>/"
```

Match each filename to a `[env:<target>]` in `platformio.ini` (`grep -n '^\[env:' platformio.ini`). Not every deployed file has a corresponding buildable target — e.g. `rnode_firmware_aethernode_dio2.zip`, `rnode_firmware_meshadventurer_merged.zip`, or anything only present under an *older* version directory/only linked from `latest/` via a stale symlink, may have no matching recipe anywhere in current source (no toggle for it exists at all). **Don't guess a substitute build for a file you can't find a real recipe for — ask the user, and leave that file un-refreshed rather than risk shipping wrong config for real hardware.**

Separately identify the **OTA-capable boards** (currently `meshpoe_s3` and `meshadventurer_s3` — `HAS_OTA` in `Boards.h`) by checking for an `OTA/` subdirectory in the deployed version dir; these need extra artifacts beyond the standard `.zip` (see step 4).

## 2. Build every target sequentially, in the background

```
pio run -e <env>
```

Run one board at a time (never in parallel) purely for reliability/resource reasons — each env has its own isolated `.pio_build/<env>/` output directory (unlike the Makefile flow's shared `build/`), so parallel *would* be technically safe, but a batch this size isn't worth the risk of resource contention or interleaved log output obscuring a real failure. A single build takes ~10-70 seconds with PlatformIO's caching (much faster than arduino-cli's ~1-2 min/board, since libraries are already vendored); expect the full ~34-board set to take well under the old 30-40 minute estimate, but budget for it anyway. Run the whole batch as one background shell script (loop over `pio run -e X`, log each target's output to its own file, keep going past individual failures) rather than one foreground call per target.

## 3. Don't dismiss build failures as environment flukes — verify

A mass rebuild is exactly the kind of event that surfaces real, previously-undetected firmware bugs: read the actual compiler error for every failure before assuming it's a fluke, especially recurring errors across board groups sharing something in common (same modem driver, same MCU family, same GNSS chip). Confirmed classes of failure to watch for (carried over from the arduino-cli era, still applicable under PlatformIO):
- A modem driver `.cpp`/`.h` referencing an ESP-IDF header that got relocated/removed in a newer bundled core version.
- One modem class (`sx127x`/`sx126x`/`sx128x`) missing a public method the other two implement, called generically from `RNode_Firmware.ino` — compare the three headers' public methods when only boards using one specific modem fail.

Fix forward in the source, re-verify with a single standalone `pio run -e <board>` for one affected board, *then* re-run the batch for every target that failed for the same reason — don't work around a real bug with a build-script hack.

## 4. Per-target artifact assembly

PlatformIO's own build output differs by MCU family and needs different post-processing to reach the flasher's expected `.zip`/`.uf2` format — it does **not** replicate the Makefile's `zip --junk-paths` step automatically for ESP32.

**ESP32 boards** — `pio run -e <env>` leaves `firmware.bin`, `bootloader.bin`, and `partitions.bin` in `.pio_build/<env>/` (no zip). Assemble the flasher zip manually, matching the exact internal member names and file set the deployed zips already use (verify with `unzip -l` against one already-deployed zip before assuming the list below is still current):
```
rnode_firmware_<board>.bin          <- .pio_build/<env>/firmware.bin
rnode_firmware_<board>.bootloader   <- .pio_build/<env>/bootloader.bin
rnode_firmware_<board>.partitions   <- .pio_build/<env>/partitions.bin
rnode_firmware_<board>.boot_app0    <- ~/.arduino15/packages/esp32/hardware/esp32/<ver>/tools/partitions/boot_app0.bin
                                        (shared across all ESP32 boards, not board-specific;
                                        <ver> matches Makefile's ARDUINO_ESP_CORE_VER)
console_image.bin                   <- Release/console_image.bin (or the board-specific
                                        console_image_<board>.bin for meshpoe_s3/meshadventurer_s3
                                        - these already exist in the repo's Release/ dir, don't
                                        regenerate them)
esptool.py                          <- Release/esptool/esptool.py (already in repo, static)
version.txt                         <- literal "<PROTO_VERSION>.<BUILD_NUMBER>" (e.g. "1.86.994"),
                                        confirmed uniform across every ESP32 board's deployed zip
```
Zip these 7 files flat (no directory structure) into `Release/rnode_firmware_<board>.zip`.

**nRF52 boards** (`heltec_t096`, `heltec_t114`, `heltec_t1`, `promicro`, `rak4631`, `rak3401`, `techo`) — PlatformIO's nordicnrf52 platform *already* produces a DFU-ready `.zip` as part of the normal build (`.pio_build/<env>/firmware.zip`, via the platform's bundled `adafruit-nrfutil`) — no manual `genpkg` step needed, unlike the old arduino-cli flow. Just copy it to `Release/rnode_firmware_<board>.zip`. (`rak3401` reuses `rak4631`'s own vendored framework/variant - see `[env:rak3401]`'s own comment in platformio.ini - so it behaves identically here.)

It does **not** produce a `.uf2` though. Generate that manually from the `.hex` using the generic uf2conv.py bundled with PlatformIO's nRF52 Arduino framework package (this one tool works for all seven boards — no need to hunt down each board's own vendored/stripped-down `.pio_vendor/<board>_nrf52_framework` copy, which has its `tools/` directory stripped out):
```
python3 ~/.platformio/packages/framework-arduinoadafruitnrf52/tools/uf2conv/uf2conv.py \
  -f 0xADA52840 -c -o Release/rnode_firmware_<board>.uf2 \
  .pio_build/<env>/firmware.hex
```
`0xADA52840` (the standard Adafruit nRF52840-application UF2 family ID) is correct for all six boards — all are nRF52840-based.

**OTA-capable boards** (`meshpoe_s3`, `meshadventurer_s3` as of this writing) additionally need, alongside the standard zip:
```
Release/OTA/rnode_firmware_<board>.bin      <- .pio_build/<env>/firmware.bin (same raw app image, not zipped)
Release/OTA/rnode_firmware_<board>.version  <- literal BUILD_NUMBER only, e.g. "994" (no dots, no
                                                PROTO_VERSION prefix - confirmed against OTA.h's
                                                OTA_VERSION_URL/ota_check_available(), which does
                                                `atol()` on this file directly, and against the
                                                already-deployed file's content)
```
This is the exact URL path the on-device OTA client fetches from — `OTA_VERSION_URL`/`OTA_BIN_URL` in `OTA.h` resolve to `.../firmware/classic/latest/OTA/rnode_firmware_<board>.{version,bin}` — so this `OTA/` subdirectory (not a top-level `.bin`/`.version` pair, which is a stale/pre-reorg convention from an older deploy that the live OTA flow doesn't read) is what actually matters for the self-update flow to pick up the new build. Don't invent or "fix" the legacy top-level `.bin`/`.version`/symlinks unless separately asked — verify against `OTA.h`'s actual fetch URLs, not assumptions, before changing anything here.

## 5. Deploy

```
rsync -avz --chown=user:user Release/*.zip Release/*.uf2 Release/OTA/ \
  srv1:/var/www/flasher.rns.moscow/htdocs/firmware/classic/<version>/
```
(adjust the `Release/OTA/` destination to land in `<version>/OTA/` specifically, e.g. a second rsync call targeting `.../<version>/OTA/` if a single invocation doesn't route it there). `--chown=user:user` matches the existing ownership convention on srv1 (nginx runs as its own uid and only needs read access, already satisfied by default perms under `user:user`).

`latest/` is a separate directory of symlinks pointing into a specific version directory (some using different legacy aliases for the same board, e.g. `heltec32v2.zip -> ../<version>/rnode_firmware_heltec32_v2.zip`) — refreshing `<version>/`'s file *contents* automatically updates what `latest/` resolves to, since the symlinks aren't touched. Don't recreate or edit `latest/`'s symlinks as part of a routine refresh; only touch that if explicitly asked to repoint which version is "latest" or to fix a specific broken/stale link.

## 6. Verify completeness before declaring done

Diff the deployed directory's filenames against what you just pushed, explicitly accounting for any files intentionally left un-refreshed (step 1) and the `OTA/` subdirectory (step 4):

```
ssh srv1 "ls /var/www/flasher.rns.moscow/htdocs/firmware/classic/<version>/" | sort > /tmp/srv1_files.txt
ls Release/*.zip Release/*.uf2 | xargs -n1 basename | sort > /tmp/local_files.txt
diff /tmp/srv1_files.txt /tmp/local_files.txt

ssh srv1 "ls /var/www/flasher.rns.moscow/htdocs/firmware/classic/<version>/OTA/" | sort > /tmp/srv1_ota_files.txt
ls Release/OTA/ | sort > /tmp/local_ota_files.txt
diff /tmp/srv1_ota_files.txt /tmp/local_ota_files.txt
```

Any unexpected diff (missing board, unexplained new/removed file) means something in the batch silently didn't run — don't treat "the script finished" as "the job is done."
