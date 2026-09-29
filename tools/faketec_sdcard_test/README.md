# Faketec SD card SPI bring-up test

Isolated, throwaway PlatformIO project to validate an SPI SD card wired to
spare pads on a ProMicro/FakeTec (nice!nano v2) board, before any of this
touches the main RNode firmware. This is bring-up for a future URNS-on-nRF52
storage backend. Not wired into the main repo's `platformio.ini` in any way -
safe to build/flash/delete independently.

## Wiring assumed

| SD card signal | nice!nano pad | nRF52 pin | Note |
|---|---|---|---|
| CLK / SCK | D5 | P0.24 | labeled "GPS_EN" |
| MISO | D4 | P0.22 | labeled "GPS_TX" |
| MOSI | D3 | P0.20 | labeled "GPS_RX" |
| CS | D0 | P0.06 | |

GPS_TX/GPS_RX are swapped here vs the generic Meshtastic reference table
(`nrf52_promicro_diy_tcxo/variant.h`, which has GPS_TX=D3/P0.20,
GPS_RX=D4/P0.22) - confirmed against this board's own physical silkscreen,
which marks the GPS_RX pad D3. Real-hardware bring-up (the `h` hold-test
below) caught the mismatch: SCK/CS toggled fine at their pads, but the wire
on GPS_RX/MOSI sat on what the firmware was driving as MISO (an input it
never actively toggles), which read as floating until the swap.

None of these are the nice!nano core's native hardware-SPI pins (SPI's
built-in mapping is D2/D3/D4/D5 = SCK/MISO/MOSI/SS) - they're just free pads
on this particular DIY board (see `Boards.h`'s own `BOARD_PROMICRO` comment
on why D3/D4/D5 are free: no GPS is actually wired up). The nRF52840's SPIM
peripheral can mux SCK/MOSI/MISO to arbitrary GPIOs, so `main.cpp` calls
`SPIClass::setPins()` to remap the existing `SPI` object before `begin()`
instead of bit-banging a software SPI driver.

## Build & flash

```
cd tools/faketec_sdcard_test
pio run -t upload -t monitor
```

(Does not use the repo's `flash-device` skill - that's scoped to the main
firmware's board environments. This is a standalone project with its own
build, reusing the main repo's already-vendored `promicro:nrf52` core via a
relative symlink path - see `platformio.ini`'s own comments.)

## Serial commands

Once connected at 115200 baud:

| Key | Action |
|---|---|
| `?` | print help |
| `i` | init/re-init SD card, print card type/size/free space |
| `w` | write a 4KB pseudo-random test pattern to `/faketec_sdtest.bin` |
| `r` | read the test file back and verify it against the same pattern |
| `l` | list the root directory |
| `x` | delete the test file |
| `f` | full auto test: init -> write -> read/verify -> list -> cleanup, prints PASS/FAIL |
| `p` | raw bit-banged CMD0 probe - bypasses the SPI peripheral and SdFat entirely |
| `h` | hold each output pin (SCK/MOSI/CS) LOW then HIGH for 4s each, for multimeter probing |

Typical first run: connect the serial monitor, press `f`, read the result.
If it fails, `p` isolates protocol-level issues from wiring, and `h` lets you
confirm each firmware pin number actually reaches the physical pad you
think it does (probe live with a multimeter while it prints).
