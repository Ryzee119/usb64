# Usage
* [Mempaks](#mempaks)
* [Rumblepaks](#rumblepaks)
* [Transferpaks](#transferpaks)
* [Virtualpak](#virtualpak)
* [Dual Stick Mode](#dual-stick-mode)
* [N64 Mouse](#n64-mouse)
* [TFT LCD Display](#tft-lcd-display)
* [Debug](#debug)

## Mempaks
* usb64 can simulate four Mempaks simultaneously. To select a Mempak press `BACK+D-PAD` direction to select the respective bank.
* Two controllers cannot have the same bank selected. The second controller will revert to a Rumblepak.
* Mempak saves are managed in RAM, so you need to be flushed to the SD card. You can do this by holding BACK then pressing A. usb64 will also sense when the n64 is powered off and automatically flush the RAM to the SD card.
* Inserting the SD card into your PC will show Mempaks as `MEMPAKXX.MPK` where XX is the bank number. You can back these up to your PC.

## Rumblepaks
* usb64 can simulate four Rumblepaks simultaneously. Rumblepaks are the default peripheral on power up. To select a Rumblepak press `BACK+LB`.
* The usb controller must support force feedback.

## Transferpaks
* usb64 can simulate four transferpaks simulateneously. The select a Transferpak press `BACK+RB`. The transferpak will attempt to load the previously set Gameboy or Gameboy Colour ROM from the SD Card.
* To select the ROM to load, you must first use the [*VirtualPak*](#virtualpak). If a ROM isn't selected, or fails to load, it will revert to a Rumblepak.
* Avoid having two controllers access the same ROM at once.
* Gameboy ROMS that save the SRAM, can be backed up to the SD Card by pressing `BACK+A`. Note: Don't do this mid game as it causes the controller to disconnect briefly and the game might error out.
* Gameboy saves can be copied over to the SD Card for use with the Transferpak. The file name must match the ROM save with a `.SAV` extension.
* You can simulate four transferpaks, with four difference ROMS, with four different save files! <p align="center"><img src="./images/tpak_6.png" alt="tpak_6" width="35%"/>  <img src="./images/tpak_7.png" alt="tpak_7" width="35%"/></p> <p align="center"><img src="./images/silver.gif" alt="silver" width="35%"/>  <img src="./images/tpak_1.png" alt="tpak_1" width="35%"/></p> <p align="center"><img src="./images/tpak_5.png" alt="tpak_5" width="35%"/>  <img src="./images/tpak_8.png" alt="tpak_8" width="35%"/></p> 

## Virtualpak
* The Virtualpak is one of my favourite features. It's like a Mempak, but is not used for save files. It exploits the Mempak managers built into some N64 games to configure the usb64 device!
* To select the Virtualpak press `BACK+START`.
* To use the Virtualpak, boot into a game that has a Mempak manager. Some games will work better than others. `Army Men: Air Combat` is a good one. `Perfect Dark` works well too. Hold START whilst the game is booting to access the Mempak manager. The follow screens show `Army Men: Air Combat` and `Perfect Dark` as an example. <p align="center"><img src="./images/vp_main.png" alt="vp_main" width="35%"/>  <img src="./images/vp_perfectdark.png" alt="vp_perfectdark" width="35%"/></p>
* To select an item, you actually delete that note from the Mempak. usb64 detects what row you selected as if navigating a menu!
* **TPAK SETTINGS** is used to configure what ROM to load into the Transferpak. This will scan the SD card for files with `.gb` and `.gbc` extensions. A `*` will print next to the currently set ROM. You can have up to ten ROMs on the SD card. After this they will just get ignored. <p align="center"><img src="./images/vp_tpak.png" alt="vp_tpak" width="35%"/></p>
* **CONT SETTINGS** is used to configure the controller. You can change deadzone, Sensitivity, toggle on/off snapping to 45deg angles and toggle on/off a octagonal N64 stick correction. The set values is shown as a number next to the row. Each controller can be configured individually. Note: Some controllers will have deadzones or 45 degree angle snapping built in. For these, usb64 can't disable it. <p align="center"><img src="./images/vp_cont.png" alt="vp_cont" width="35%"/></p>
* **USB64 INFO1** shows what controller is connected to that port. <p align="center"><img src="./images/vp_info1.png" alt="vp_info1" width="35%"/></p>
* **USB64 INFO2** is currently a placeholder.

## Dual Stick Mode
* Dual stick mode exploits a feature present in Perfect Dark and GoldenEye 007 to use two controllers at once for true dual analog stick input.
* To use, the usb64 must be connected to controller port one and two as a minimum. It works by simulating two controllers with one and injecting the 2nd analog stick into port two. If a 2nd controller is connected to the usb64 when using this mode, it will get pushed to slot three.
* To enable press `BACK+B`.
* Set the game controller input for dual stick mode. It has been designed to work best with `Layout 2.4`. <p align="center"><img src="./images/dual_goldeneye.png" alt="dual_goldeneye" width="35%"/>  <img src="./images/dual_perfectdark.png" alt="dual_perfectdark" width="35%"/></p>

## N64 Mouse
* usb64 can simulate four N64 Mouse peripherals simulateneously!
* Just plug in a USB mouse and the usb64 will auto detect it and emulate a N64 Mouse.
* The middle mouse button is mapped to START. <p align="center"><img src="./images/mouse_2.png" alt="mouse_2" width="35%"/> <img src="./images/mouse_1.png" alt="mouse_1" width="35%"/></p>

## TFT LCD Display
* usb64 supports an optional TFT LCD display based on the low cost and extremely common ILI9341 display controller.
* The display will automatically work once connected.
* To cycle through different screens press `L+R`. Currently the default screen shows an overview of the current controller status. The other screen shows some useful debug info. <p align="center"><img src="./images/tft_1.png" alt="tft_1" width="35%"/> <img src="./images/tft_2.png" alt="tft_2" width="35%"/></p>

## Debug
* There's alot going, and currently it may not be clear what the usb64 is doing. Until something better is implemented, you can connect the usb64 to your PC via a MicroUSB cable. This will enumerate as a serial comport. Connect to it with your favourite terminal to get some feedback. The code can be recompiled with [additional debug flags](./src/usb64_conf.h). <p align="center"><img src="./images/debug.png" alt="debug" width="65%"/></p>
