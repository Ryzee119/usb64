# Hardware Setup
* Install W25Q126 SPI Flash Chip on the designated footprint.
* Install a 5 x 0.1" pin header on the USB OTG Header.  

<img src="./images/install_setup.svg" alt="setup" width="50%"/>

# Quick Start
* Connect the player data lines to the N64 Console.
* Connect a usb64 compatible USB controller.
* Connect a MicroUSB cable to supply 5V.  

<img src="./images/install_basic.svg" alt="basic" width="75%"/>

# Advanced Usage
* Hardwire a custom N64 controller to the designated IO (Each digital pin is internally pulled-up, Analog input respect to VCC, VCC/2 is central position).
* Hook up a Raspberrty Pi or similar via I2C and send button presses via I2C.  

<img src="./images/install_advanced.svg" alt="advanced" width="75%"/>

## License and Attribution
usb64 is shared under the [MIT license](https://github.com/Ryzee119/usb64/blob/dev/LICENSE), however this project includes code by others. Refer to the list below.
* [N64 Console artwork](https://icon-library.net/icon/nintendo-64-icon-23.html) shared under [CC0 Public Domain Licence](https://creativecommons.org/publicdomain/zero/1.0/).
