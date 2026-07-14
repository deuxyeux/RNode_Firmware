# RNode Firmware

This is a fork of [RNode_Firmware](https://github.com/markqvist/RNode_Firmware) by Mark Qvist  

As Mark no longer accepts PRs and RNode_Firmware_CE is inactive, it was decided to make a private fork to fix issues and add additional hardware.  
The fork contains several DIY devices by me and other developers and has a few small tweaks to the firmware like interference thresholds.

---

## Added devices

- [Aethernode](https://github.com/ahedproductions/aethernode) by aetherlab LZ1SWE
- [Aethernode-S3](https://github.com/ahedproductions/aethernodeS3) by aetherlab LZ1SWE
- [MeshAdventurer-S3](https://git.rns.moscow/deuxyeux/MeshAdventurer-S3) by Nickie Deuxyeux
- [MeshAdventurer](https://github.com/chrismyers2000/MeshAdventurer) by Frequency Labs
- [DIY-V1](https://github.com/NanoVHF/Meshtastic-DIY/) by NanoVHF
- [Faketec (ProMicro)](https://github.com/gargomoma/fakeTec_pcb) by gargomoma, ShimonHoranek, lupusworax
- [MeshPoE-S3](https://git.rns.moscow/deuxyeux/MeshPoE-S3) by Nickie Deuxyeux
- [Heltec Mesh Node T096](https://heltec.org/project/t096/)
- [LilyGo T-Beam Supreme V3](https://lilygo.cc/en-us/products/t-beam-supreme)

## RNode Settings Menu system

Boards with a display and a main button get an on-device settings menu, opened with a long-press (3s) of the main button:

- **Display Timeout** - how long before the screen blanks
- **Brightness** - OLED contrast/backlight level
- **Orientation** - screen rotation
- **Sound** - mutes the buzzer entirely (defaults off on boards where the buzzer is a DIY add-on most builds skip, e.g. ProMicro/FakeTec; on otherwise defaults on)
- **Encoder** - on boards with encoder support built in (MeshAdventurer-S3, ProMicro/FakeTec), tells the menu whether a physical rotary encoder is actually populated, defaulting off since it's either a PCB-provisioned-but-optional part (MeshAdventurer-S3) or a DIY add-on (ProMicro/FakeTec) rather than something guaranteed present. Changes the on-screen footer hint (turn/press vs tap/hold) and actually gates the encoder's turn/press input - off ignores it entirely, so a board with no encoder wired up can't misread floating-pin noise as phantom input
- **WiFi** - mode/SSID/PSK (ESP32 boards with WiFi only)
- **Hardware** - read-only info: CPU temp (ESP32-S3 and nRF52 boards), input voltage, battery voltage, WiFi IP/netmask/MAC, Bluetooth MAC, depending on what the board actually has. Some boards also add a battery-scale recalibration field here, for boards without a real VSENSE divider (e.g. ProMicro/FakeTec)
- **Hardware > GPIO** - on DIY boards where the builder solders peripherals to whichever spare pad they used (currently ProMicro/FakeTec), lets you reassign a peripheral - the buzzer, and the encoder's Up/Down/Press pins - to a different physical pin from a short list of genuinely free GPIOs. Refuses to assign the same pin to two peripherals at once. Takes effect on the next boot

Boards with encoder support (MeshAdventurer-S3, ProMicro/FakeTec) turn/click through the same menu as an alternate to the main button, once Encoder is turned on. The main button always works regardless: a short tap moves forward one step, a quick double-tap moves back, and a long press selects/confirms - the on-screen footer shows whichever scheme is currently active (button-only, or button+encoder once Encoder is on).

Changes are staged in RAM as you navigate and only written to EEPROM (with the matching device reboot where needed) when you commit and exit the menu, so browsing settings never triggers spurious reboots or writes. Navigation and button presses get short tick/click buzzer feedback (unless Sound is off).

## Improvements
- RNode Settings Menu system - see above
- Heltec T114 LCD backlight powers off when screen blanks
- LNA-equipped boards have interference detection threshold increased from 11 to 14dB
- W5500 wired ethernet support with DHCP hostname
- MeshPoE-S3 status display shows an ETH box reflecting the W5500 physical link state, plus a rotating Ethernet IP page
- MeshAdventurer-S3 status display shows resistor-divider supply voltage, alternating with CPU temperature
- Heltec T096 display redesigned for the full 80x160 panel with colour indicators and gamma-corrected backlight PWM dimming
- Buzzer melodies for boot, Bluetooth pairing/connect, and RNS link-state changes, with a mute toggle
- Updated to Arduino ESP32 core 3.3.8 (ESP-IDF v5.5.4) with NimBLE support
- Added model for high-power 33dBm E22 modules
- TXEN pin handling for SX126x modems
- Fallback TX power raised from 17dBm to 20dBm
