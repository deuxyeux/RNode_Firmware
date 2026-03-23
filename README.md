# RNode Firmware

This is a fork of [RNode_Firmware](https://github.com/markqvist/RNode_Firmware) by Mark Qvist  

As Mark no longer accepts PRs and RNode_Firmware_CE is inactive, it was decided to make a private fork to fix issues and add additional hardware.  
The fork contains several DIY devices by me and other developers and has a few small tweaks to the firmware like interference thresholds.

---

## Added devices

- [Aethernode](https://github.com/ahedproductions/aethernode) by aetherlab LZ1SWE
- [MeshAdventurer-S3](https://git.rns.moscow/deuxyeux/MeshAdventurer-S3) by Nickie Deuxyeux
- [MeshAdventurer](https://github.com/chrismyers2000/MeshAdventurer) by Frequency Labs
- [DIY-V1](https://github.com/NanoVHF/Meshtastic-DIY/) by NanoVHF
- [Faketec (ProMicro)](https://github.com/gargomoma/fakeTec_pcb) by gargomoma, ShimonHoranek, lupusworax

## Improvements
- Heltec T114 LCD backlight powers off when screen blanks
- LNA-equipped boards have interference detection threshold increased from 11 to 14dB
