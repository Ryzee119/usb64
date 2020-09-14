<img src="./images/usb64_logo.png" alt="basic" width="208px"/>  

![Build](https://github.com/Ryzee119/usb64/workflows/Build/badge.svg) ![badge](https://img.shields.io/badge/license-MIT-green)  
A project developed to use USB controllers on the Nintendo 64 console.  
Precompiled binaries can be downloaded from [Releases](https://github.com/Ryzee119/usb64/releases).  

- [x] N64 controller emulation.
- [x] Rumblepak emulation.
- [x] Mempak emulation with 4 selectable banks. Technically unlimited.
- [x] Transferpak emulation with 16MB internal storage for Gameboy ROMs.
- [x] N64 mouse emulation (Use a USB Mouse!).
- [x] Configurable deadzones and sensitivity from the N64 Console.
- [x] USB mass storage driver to backup mempaks and gameboy saves over USB.
- [x] Raspberry Pi interface or equivalent for all other USB controllers.
- [x] Hardwired controller interface.

## Todo
- [ ] True dual analog sticks with GoldenEye and Perfect Dark.
- [ ] More inbuilt USB controller drivers.
- [ ] Gameboy Camera Support.
- [ ] Touch LCD Support.

## Supported USB Controllers
- Bluetooth 8bitdo/compatible controllers via the [8BitDo Wireless USB Adapter](https://www.8bitdo.com/wireless-usb-adapter/)
- Xbox one Wired (Genuine Only)
- Xbox 360 Wired
- Xbox 360 Wireless (Via PC USB Receiver)
- Raspberry Pi interface or equivalent for all other USB controllers.

## Controls
- Back + D-Pad = Insert Mempak banks 1 to 4
- Back + LB = Insert Rumblepak
- Back + RB = Insert Transferpak
- Back + Start = Select *virtual pak* (Use in-game mempak managers to configure the device)
- Back + A = Backup SRAM to Flash **(DO THIS BEFORE POWER OFF!)**

## Needed Parts
| Part Description | Link |
|--|--|
| Teensy 4.1 | https://www.pjrc.com/store/teensy41.html |
| USB Host Cable | https://www.pjrc.com/store/cable_usb_host_t36.html |
| 3 x Pin Header | https://www.pjrc.com/store/header_24x1.html |
| 128Mbit SPI Flash W25Q128 SOIC-8| https://www.digikey.com.au/short/zb3v27 |
| N64 Controller Extensions | [AliExpress](https://www.aliexpress.com/wholesale?catId=0&SearchText=n64%20controller%20extension) |
| Case | To do |

## Compile and Program
See [COMPILE.md](./COMPILE.md).

## Install
See [INSTALL.md](./INSTALL.md).

## How to Use
See [USAGE.md](./USAGE.md).

## License and Attribution
usb64 is shared under the [MIT license](https://github.com/Ryzee119/usb64/blob/dev/LICENSE), however this project includes code by others. Refer to the list below.
* [mpaland](https://github.com/mpaland)/**[printf](https://github.com/mpaland/printf)** shared under the [MIT License](https://github.com/mpaland/printf/blob/d3b984684bb8a8bdc48cc7a1abecb93ce59bbe3e/LICENSE).
* [hathach](https://github.com/hathach)/**[tinyusb](https://github.com/hathach/tinyusb)** shared under the [MIT License](https://github.com/hathach/tinyusb/blob/master/LICENSE).
* [FatFs by ChaN](http://elm-chan.org/fsw/ff/00index_e.html) shared under a [BSD-style license](https://github.com/Ryzee119/usb64/blob/dev/src/fatfs/LICENSE.txt).
* [QSPI](src/teensy41/qspi) is from [teensy41_extram](https://github.com/PaulStoffregen/teensy41_extram) shared under an '[MIT or MIT-like license](https://forum.pjrc.com/threads/29382-open-source-license-issues-when-using-teensy-products?p=79667&viewfull=1#post79667)'.
* [USBHost_t36 fork](https://github.com/Ryzee119/USBHost_t36) shared under an '[MIT or MIT-like license](https://forum.pjrc.com/threads/29382-open-source-license-issues-when-using-teensy-products?p=79667&viewfull=1#post79667)'.
* [Teensy cores](https://github.com/PaulStoffregen/cores) shared under an '[MIT or MIT-like license](https://forum.pjrc.com/threads/29382-open-source-license-issues-when-using-teensy-products?p=79667&viewfull=1#post79667)'.
* [MBC emulation code](src/n64/n64_transferpak_gbcarts.c) is adapted from [Peanut-GB](https://github.com/deltabeard/Peanut-GB) shared under an MIT License.
