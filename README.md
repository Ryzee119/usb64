# USB64
A project developed to use USB controllers on the Nintendo 64 console.  
**WARNING: Considered early BETA!**

- [x] 4 x Simulatenous N64 controller emulation.
- [x] 4 x Simulatenous N64 mouse emulation (Use a USB Mouse!).
- [x] 4 x Simulatenous Rumblepak emulation.
- [x] 1 x Mempak Emulation (Player 1) with 8 banks (selectable with controller combos)
- [x] 1 x Transferpak Emulation (Player 1) with storage of up to 10 Gameboy ROMs.
- [x] Configuration deadzones, sensitivity and analog stick shape.
- [x] Complete configuration of device straight from the N64 console.
- [x] Simple cross platform GUI to backup SRAM saves or load Transferpak Gameboy ROMs over USB.

## Todo
- [ ] Teensy 4.1 is overkill. Port to cheaper platform.
- [ ] SD Card instead of Flash chip?
- [ ] 4 x Simultaneous Mempaks
- [ ] 4 x Simultaneous Transferpaks
- [ ] More USB controller drivers
- [ ] True dual analog sticks with GoldenEye and Perfect Dark

## Supported USB Controllers
- Bluetooth 8bitdo/compatible controllers via the [8BitDo Wireless USB Adapter](https://www.8bitdo.com/wireless-usb-adapter/)
- Xbox one Wired (Genuine Only)
- Xbox 360 Wired
- Xbox 360 Wireless (Via PC USB Receiver)
- PS3 Controllers
- PS4 Controllers

## Controls
- Back + D-Pad = Insert mempak banks 1 to 4
- Back + A/B/X/Y = Insert mempak banks 5 to 8
- Back + LB = Insert Rumblepak
- Back + RB = Insert Transferpak
- Back + Start = Select *virtual pak* (Use in-game mempak managers to configure the device)

## Needed Parts
| Part Description | Link |
|--|--|
| Teensy 4.1 | https://www.pjrc.com/store/teensy41.html |
| USB Host Cable | https://www.pjrc.com/store/cable_usb_host_t36.html |
| 128Mbit SPI Flash W25Q128 SOIC-8| https://www.digikey.com.au/short/zb3v27 |
| N64 Controller Extensions | [AliExpress](https://www.aliexpress.com/wholesale?catId=0&SearchText=n64%20controller%20extension) |
| Case | To do |

## Compile
To do

## Program
To do

## Install
To do

## License and Attribution
USB64 is shared under the [MIT license](https://github.com/Ryzee119/usb64/blob/dev/LICENSE), however this project includes code by others. Refer to the list below.
* [mpaland](https://github.com/mpaland)/**[printf](https://github.com/mpaland/printf)** shared under the [MIT License](https://github.com/mpaland/printf/blob/d3b984684bb8a8bdc48cc7a1abecb93ce59bbe3e/LICENSE)
* [FatFs by ChaN](http://elm-chan.org/fsw/ff/00index_e.html) shared under a [BSD-style license](https://github.com/Ryzee119/usb64/blob/dev/src/fatfs/LICENSE.txt) 
* [QSPI](https://github.com/Ryzee119/usb64/tree/dev/src/qspi) is from [teensy41_extram](https://github.com/PaulStoffregen/teensy41_extram) shared under an '[MIT or MIT-like license](https://forum.pjrc.com/threads/29382-open-source-license-issues-when-using-teensy-products?p=79667&viewfull=1#post79667)'
* [USBHost_t36 fork](https://github.com/Ryzee119/USBHost_t36) shared under an '[MIT or MIT-like license](https://forum.pjrc.com/threads/29382-open-source-license-issues-when-using-teensy-products?p=79667&viewfull=1#post79667)'
* [Teensy cores](https://github.com/PaulStoffregen/cores) shared under an '[MIT or MIT-like license](https://forum.pjrc.com/threads/29382-open-source-license-issues-when-using-teensy-products?p=79667&viewfull=1#post79667)'
