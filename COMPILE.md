## Compile
### CLI (Requires python and python-pip)
```
git clone https://github.com/Ryzee119/usb64.git --recursive
python -m pip install --upgrade pip
pip install platformio
cd usb64
platformio run -e teensy41
```
### Visual Studio Code
* Download and install [Visual Studio Code](https://code.visualstudio.com/).
* Install the [PlatformIO IDE](https://platformio.org/platformio-ide) plugin.
* Clone this repo recursively `git clone https://github.com/Ryzee119/usb64.git --recursive`
* In Visual Studio Code `File > Open Folder... > usb64`
* Hit build on the Platform IO toolbar (`✓`).

## Program
### Teensy (using Teensy Loader)
* Connect the Teensy to your PC using a MicroUSB cable.
* Run the [Teensy Loader Application](https://www.pjrc.com/teensy/loader.html).

### Teensy (using Visual Studio Code)
* Setup Visual Studio Code as per the Compile instructions.
* Hit the program button on the Platform IO toolbar (`→`).