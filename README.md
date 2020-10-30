<img src="./images/usb64_logo.png" alt="basic" width="208px"/>  

![Build](https://github.com/Ryzee119/usb64/workflows/Build/badge.svg) ![badge](https://img.shields.io/badge/license-MIT-green)  
A project developed to use USB controllers on the Nintendo 64 console.  
Precompiled binaries can be downloaded from [Releases](https://github.com/Ryzee119/usb64/releases).  
**NOTE: This project is still in development and bugs are certainly still present. I plan to play alot of N64 to test :smile:.**

- [x] N64 controller emulation (up to four controllers at once!).
- [x] Rumblepak emulation.
- [x] Mempak emulation with four selectable banks. Technically unlimited.
- [x] Transferpak emulation. Put Gameboy ROMS on an SD Card!
- [x] N64 mouse emulation. Use a USB Mouse!
- [x] Configurable deadzones and sensitivity from the N64 Console.
- [x] True dual analog sticks with GoldenEye and Perfect Dark.
- [x] SD card driver with FATFS support for storage/backup of Gameboy ROMS, mempaks etc.
- [x] A single hardwired controller interface for ultimate hacking.
- [x] Optional TFT LCD Support.

## Todo
- [ ] Raspberry Pi interface or equivalent for all other USB controllers.
- [ ] More inbuilt USB controller drivers.

## Supported Controllers
- Bluetooth 8bitdo/compatible controllers via the [8BitDo Wireless USB Adapter](https://www.8bitdo.com/wireless-usb-adapter/)
- Wired 8bitdo controllers when they are started in X-input mode.
- Xbox one Wired (Genuine Only)
- Xbox 360 Wired
- Xbox 360 Wireless (Via PC USB Receiver)
- PS4 Wired
- Raspberry Pi interface or equivalent for all other USB controllers.
- A hardwired controller, use your own buttons etc.

## Controls
- Back + D-Pad = Insert Mempak banks 1 to 4
- Back + LB = Insert Rumblepak
- Back + RB = Insert Transferpak
- Back + Start = Select *virtual pak* (Use in-game mempak managers to configure the device)
- Back + B = Switch to true dual-analog stick more for GoldenEye 007/Perfect Dark
- Back + A = Backup buffered memory to SD Card **(DO THIS BEFORE POWER OFF!)**

## Needed Parts
| Qty | Part Description | Link |
|--|--|--|
| 1 | Teensy 4.1 | https://www.pjrc.com/store/teensy41.html |
| 1 | USB Host Cable | https://www.pjrc.com/store/cable_usb_host_t36.html |
| 3 | 0.1" Pin Header | https://www.pjrc.com/store/header_24x1.html |
| 2 | 64Mbit PSRAM  SOIC-8 | https://www.pjrc.com/store/psram.html |
| 4 | N64 Controller Extensions | [AliExpress](https://www.aliexpress.com/wholesale?catId=0&SearchText=n64%20controller%20extension) |
| 1 | (Optional) ILI9341 TFT LCD Display (320x240) | [AliExpress](https://www.aliexpress.com/wholesale?catId=0&SearchText=ili9341%20tft) |
| 1 | MicroSD Card | - |
| 1 | Case | To do |
| 1 | PCB breakout board | [Kitspace](https://kitspace.org/boards/github.com/ryzee119/usb64/) |

Note: PSRAM model numbers are IPS6404L-SQ-SPN or ESP-PSRAM64H.

## Compile and Program
See [COMPILE.md](./COMPILE.md).

## Install
See [INSTALL.md](./INSTALL.md).

## How to Use
See [USAGE.md](./USAGE.md).

## License and Attribution
usb64 is shared under the [MIT license](https://github.com/Ryzee119/usb64/blob/dev/LICENSE), however this project includes code by others. Refer to the list below.
* [mpaland](https://github.com/mpaland)/**[printf](https://github.com/mpaland/printf)** shared under the [MIT License](https://github.com/mpaland/printf/blob/master/LICENSE).
* [thi-ng](https://github.com/thi-ng)/**[tinyalloc](https://github.com/thi-ng/tinyalloc)** shared under the [Apache-2.0 License](https://github.com/thi-ng/tinyalloc/blob/master/LICENSE).
* [FatFs by ChaN](http://elm-chan.org/fsw/ff/00index_e.html) shared under a [BSD-style license](https://github.com/Ryzee119/usb64/blob/dev/src/fatfs/LICENSE.txt).
* [USBHost_t36 fork](https://github.com/Ryzee119/USBHost_t36) shared under an '[MIT or MIT-like license](https://forum.pjrc.com/threads/29382-open-source-license-issues-when-using-teensy-products?p=79667&viewfull=1#post79667)'.
* [Teensy cores](https://github.com/PaulStoffregen/cores) shared under an '[MIT or MIT-like license](https://forum.pjrc.com/threads/29382-open-source-license-issues-when-using-teensy-products?p=79667&viewfull=1#post79667)'.
* [MBC emulation code](src/n64/n64_transferpak_gbcarts.c) is adapted from [Peanut-GB](https://github.com/deltabeard/Peanut-GB) shared under an MIT License.

<p align="center"><img src="./images/setup.jpg" alt="vp_cont" width="50%"/></p>
