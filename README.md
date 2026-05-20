# RNode Firmware

This is a fork of [RNode_Firmware](https://github.com/markqvist/RNode_Firmware) by Mark Qvist  

As Mark no longer accepts PRs and RNode_Firmware_CE is inactive, it was decided to make a private fork to fix issues and add additional hardware.  
The fork contains several DIY devices by me and other developers and has a few small tweaks to the firmware like interference thresholds.

---

## Added devices

- [Aethernode](https://github.com/ahedproductions/aethernode) by aetherlab LZ1SWE
- Aethernode-S3 by aetherlab LZ1SWE
- [MeshAdventurer-S3](https://git.rns.moscow/deuxyeux/MeshAdventurer-S3) by Nickie Deuxyeux
- [MeshAdventurer](https://github.com/chrismyers2000/MeshAdventurer) by Frequency Labs
- [DIY-V1](https://github.com/NanoVHF/Meshtastic-DIY/) by NanoVHF
- [Faketec (ProMicro)](https://github.com/gargomoma/fakeTec_pcb) by gargomoma, ShimonHoranek, lupusworax
- MeshPoE-S3 by Nickie Deuxyeux
- Heltec T096
- LilyGo T-Beam Supreme V3

## Improvements
- Heltec T114 LCD backlight powers off when screen blanks
- LNA-equipped boards have interference detection threshold increased from 11 to 14dB
- W5500 wired ethernet support with DHCP hostname
- Updated to Arduino ESP32 core 3.3.8 (ESP-IDF v5.5.4) with NimBLE support
- Added model for high-power 33dBm E22 modules
- TXEN pin handling for SX126x modems
- Fallback TX power raised from 17dBm to 20dBm
