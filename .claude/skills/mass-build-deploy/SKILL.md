---
name: mass-build-deploy
description: Use when asked to build RNode_Firmware for all (or most) board targets and deploy/refresh the results on the flasher site (flasher.rns.moscow). Triggers on "build all targets", "compile for all boards", "refresh the flasher", "update the firmware archive", or similar mass-build-and-publish requests.
---

# Mass-building and deploying RNode_Firmware to the flasher

This is a long-running (30-40 minute), multi-step batch job — not a single command. Plan for background execution and expect to hit real compile bugs along the way, not just build-environment flukes (see "Don't dismiss failures" below).

## 1. Establish the target list from the deployed directory, not just the Makefile

Before building anything, list what's actually deployed and treat it as the source of truth for what needs refreshing:

```
ssh srv1 "ls /var/www/flasher.rns.moscow/htdocs/firmware/classic/<version>/"
```

Match each filename to a `release-<target>` recipe in `Makefile` (`grep -n '^release-[a-z_0-9]*:' Makefile`). Not every deployed file has a corresponding buildable target — e.g. `rnode_firmware_aethernode_dio2.zip`/`rnode_firmware_aethernode_txrx.zip` have no matching recipe anywhere in the current `Makefile`/`Boards.h` (no DIO2-vs-TXRX toggle exists in the source at all). **Don't guess a substitute build for a file you can't find a real recipe for — ask the user, and leave that file un-refreshed rather than risk shipping wrong RF-switch config for real hardware.**

`release-all`'s own dependency list is stale relative to newer boards (e.g. it's missing `meshpoe_s3`, `meshadventurer_s3`, `aethernode_s3`, `diy_v1`, `promicro` as of this writing) — don't rely on it alone. Build the exact list of individual `release-<target>` names matching the deployed directory instead.

## 2. Build ESP32 targets sequentially, in the background

Each ESP32 `release-<target>` recipe ends with `rm -r build`, and many boards share the *same* FQBN (e.g. `esp32:esp32:esp32` is used by tbeam, lora32v10, heltec32v2, ng20, meshadventurer, diy_v1, featheresp32, aethernode, generic — differentiated only by a `-DBOARD_MODEL=` flag), so targets **must** run one at a time, never in parallel. A single ESP32 build takes ~1-2 minutes; expect ~25-35 minutes for the full ESP32 set. Run the whole batch as one background shell script (loop over `make release-X`, log each target's output to its own file, keep going past individual failures) rather than one foreground `make` call per target.

## 3. Don't dismiss build failures as environment flukes — verify

A mass rebuild is exactly the kind of event that surfaces real, previously-undetected firmware bugs (this happened twice in the one run this skill was written from): read the actual compiler error for every failure before assuming it's a fluke, especially recurring errors across board groups sharing something in common (same modem driver, same MCU family). Confirmed classes of failure to watch for:
- A modem driver `.cpp`/`.h` referencing an ESP-IDF header that got relocated/removed in a newer bundled core version (check `find ~/.arduino15/packages/esp32/hardware/esp32/<ver>/tools/esp32-arduino-libs -iname "<header>"` — if it's genuinely gone and unused elsewhere in the file, delete the stray include rather than trying to route around it).
- One modem class (`sx127x`/`sx126x`/`sx128x`) missing a public method the other two implement, called generically from `RNode_Firmware.ino` (e.g. `LoRa->reset()`) — compare the three headers' public methods when only boards using one specific modem fail.

Fix forward in the source, re-verify with a single standalone `arduino-cli compile` for one affected board, *then* re-run the batch for every target that failed for the same reason - don't work around a real bug with a build-script hack.

## 4. nRF52 targets need special handling for `.uf2` alongside the DFU `.zip`

The 5 nRF52 boards (`heltec_t114`, `heltec_t096`, `rak4631`, `promicro`, `techo`) each build a `.hex`, packaged via `adafruit-nrfutil dfu genpkg` into the `.zip` the flasher's DFU-over-BLE/serial flow uses. None of their `release-<target>` recipes do `rm -r build` (unlike ESP32 ones), but **a later ESP32 target's `rm -r build` will still wipe the whole `build/` tree** if nRF52 builds ran earlier in the same session and haven't been converted to `.uf2` yet — do the UF2 conversion immediately after each nRF52 board's compile, not deferred to the end.

Only `Heltec_nRF52`'s `platform.txt` has its `recipe.objcopy.uf2.pattern` uncommented (auto-produces a `.uf2` during `arduino-cli compile`); `promicro`, `rakwireless`, and `adafruit` (techo) core packages have the identical recipe line present but **commented out**. Don't edit the global `~/.arduino15` package files to uncomment it (not reproducible, could get clobbered on a package update) — instead run each board's bundled `uf2conv.py` manually against the already-built `.hex`:

```
python3 ~/.arduino15/packages/<pkg>/hardware/nrf52/<ver>/tools/uf2conv/uf2conv.py \
  -f 0xADA52840 -c -o Release/rnode_firmware_<board>.uf2 \
  build/<fqbn-dir>/RNode_Firmware.ino.hex
```

`0xADA52840` (the standard Adafruit nRF52840-application UF2 family ID) is correct for **all five** boards — confirmed by checking each board's own `boards.txt`: `rakwireless`/`Heltec_nRF52` hardcode it in their (commented or active) recipe, and `adafruit`'s `pca10056`/`promicro`'s `nicenanov2` board entries set `build.uf2_family=0xADA52840` explicitly. All five are nRF52840-based, so there's no per-board variation to look up.

## 5. Deploy

```
rsync -avz --chown=user:user Release/*.zip Release/*.uf2 \
  srv1:/var/www/flasher.rns.moscow/htdocs/firmware/classic/<version>/
```

`--chown=user:user` matches the existing ownership convention on srv1 (nginx runs as its own uid and only needs read access, already satisfied by default perms under `user:user`).

## 6. Verify completeness before declaring done

Diff the deployed directory's filenames against what you just pushed, explicitly accounting for any files intentionally left un-refreshed (step 1):

```
ssh srv1 "ls /var/www/flasher.rns.moscow/htdocs/firmware/classic/<version>/" | sort > /tmp/srv1_files.txt
ls Release/*.zip Release/*.uf2 | xargs -n1 basename | sort > /tmp/local_files.txt
diff /tmp/srv1_files.txt /tmp/local_files.txt
```

Any unexpected diff (missing board, unexplained new/removed file) means something in the batch silently didn't run — don't treat "the script finished" as "the job is done."
