# ES8311 hardware bring-up test (MeshAdventurer-S3)

Isolated, throwaway PlatformIO project to validate the ES8311 audio codec's
speaker output and microphone input on real MeshAdventurer-S3 hardware,
before any of this touches the main RNode firmware. Not wired into the main
repo's `platformio.ini` in any way - safe to build/flash/delete independently.

## Wiring assumed

| Signal | GPIO |
|---|---|
| I2S BCK | 12 |
| I2S WS | 13 |
| I2S DI (MCU -> codec) | 14 |
| I2S DO (codec -> MCU) | 38 |
| Codec enable (SD) | 46 (assumed active-HIGH - unverified, see `pins.h`) |
| I2C SDA | 41 |
| I2C SCL | 42 |
| MCLK | not connected - codec runs in MCLK-less mode, clocked from BCLK |

## Build & flash

```
cd tools/es8311_test
pio run -t upload -t monitor
```

(Does not use the repo's `flash-device` skill - that's scoped to the main
firmware's board environments. This is a standalone project with its own
build.)

## Serial commands

Once connected at 115200 baud:

| Key | Action |
|---|---|
| `?` | print help |
| `i` | re-probe I2C bus, print CHIPID |
| `r` | re-run ES8311 register init |
| `t` | toggle a continuous 1kHz tone through the speaker |
| `m` | toggle a live mic RMS level meter |
| `l` | toggle mic -> speaker loopback (full duplex) |
| `v<0-100>` | set DAC volume percent, e.g. `v90` |
| any other key | stop whatever mode is running |

## Expected boot sequence / troubleshooting

1. `[GPIO46] Codec enable pin driven HIGH...` then `[I2C] ES8311 ACKed at
   0x1_` then a CHIPID printout.
   - **No ACK at either 0x18 or 0x19**: try flipping `CODEC_EN_ACTIVE` to
     `LOW` in `src/pins.h` first (this is the one genuinely unverified
     assumption in the wiring) and re-flash. If still failing, check
     SDA/SCL continuity.
2. `[ES8311] init OK` - each register write succeeded.
   - Mid-sequence failure right after a successful probe: try increasing
     the settle delay in `main.cpp`'s `initCodecAndBus()` after driving
     GPIO46.
3. `[I2S] Full-duplex STD init OK`.
   - Failure here means a pin conflict or invalid config - double check
     `pins.h` against the real wiring.
4. Press `t`: expect an audible 1kHz tone from the speaker.
   - Silence with no errors: check speaker/line-out wiring, try `v90` to
     rule out volume, confirm step 2 actually reported OK.
5. Press `m`: expect the printed RMS to visibly rise when speaking/clapping
   near the mic and fall back to a low baseline when quiet.
   - Flat regardless of sound: check the physical mic wiring and that step 2
     reported OK (mic PGA gain is set as part of codec init).
6. Press `l`: expect an audible near-real-time echo of ambient sound through
   the speaker - the strongest end-to-end confirmation, since it exercises
   both directions and both clock domains concurrently.
   - Choppy audio despite `t`/`m` working individually on their own: the
     chunk size (`CHUNK_SAMPLES` in `main.cpp`) may need lowering.

## Notes on the ES8311 driver (`src/es8311_driver.*`)

Register addresses, the init sequence, and the MCLK-less clock-coefficient
values are ported from Espressif's `esp-bsp` `es8311` component
(<https://github.com/espressif/esp-bsp>, `components/es8311`, Apache-2.0),
fetched 2026-09-28, and specialized to the single 16-bit/MCLK-less
configuration this test needs. Sample rate defaults to 32000 Hz (see
`pins.h`) - only 22050/32000/44100/48000/64000/88200/96000 Hz have an exact
match in the coefficient table when MCLK is derived from BCLK; 16000 Hz does
not, despite being the more obvious "voice" default.

`ADC routing register 0x44` and the DAC source-select bit are left at their
chip reset defaults, matching upstream (which never touches them either) -
should be correct for a straightforward mono ADC-in/DAC-out path, but worth
checking first if loopback captures silence despite the mic clearly working
standalone.
