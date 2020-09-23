/* MIT License
 * 
 * Copyright (c) [2020] [Ryan Wendland]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _USB64_CONF_h
#define _USB64_CONF_h

/* DEBUGGING OUTPUT - WARNING SOME OF THESE MAY BREAK TIMING AND CAUSE ISSUES
   USE ONLY FOR DEBUGGING */
#define serial_port Serial1
#define DEBUG_STATUS 1  //General information
#define DEBUG_N64 0     //For debugging N64 low level info
#define DEBUG_TPAK 0    //For debugging N64 TPAK low level info. It's complex so has its own flag
#define DEBUG_USBHOST 0 //For debugging the USB Host Stack
#define DEBUG_FATFS 0   //For debugging the FATFS io
#define DEBUG_MEMORY 0  //For debugging the memory allocator in external RAM.
#define DEBUG_ERROR 1   //For showing critical errors

/* USB HOST STACK */
#define ENABLE_USB_HUB 1

/* N64 LIB */
#define MAX_CONTROLLERS 4 //Max is 4
#define MAX_MICE 4 //0 to disable N64 mouse support, Must be <= MAX_CONTROLLERS
#define MAX_GBROMS 10 //ROMS over this will just get ignored
#define ENABLE_I2C_CONTROLLERS 0 //Received button presses over I2C, useful for integrating with a rasp pi etc.
#define ENABLE_HARDWIRED_CONTROLLER 0 //Ability to hardware a N64 controller into the usb64.


/* PIN MAPPING */
#define N64_CONSOLE_SENSE 37 //High when usb64 is connected to the n64 console
#define N64_CONSOLE_SENSE_DELAY 50 //ms to wait at power up for n64 console to power on
#define N64_CONTROLLER_1_PIN 36
#define N64_CONTROLLER_2_PIN 35
#define N64_CONTROLLER_3_PIN 34
#define N64_CONTROLLER_4_PIN 33
#define USER_LED_PIN 13

/* FILESYSTEM */
#define MAX_FILENAME_LEN 256
#define SETTINGS_FILENAME "SETTINGS.DAT"

/* FIRMWARE DEFAULTS (CONFIGURABLE DURING USE) */
#define DEFAULT_SENSITIVITY 2  //0 to 4 (0 = low sensitivity, 4 = max)
#define DEFAULT_DEADZONE 2     //0 to 4 (0 = no deadzone correction, 4 = max (40%))
#define DEFAULT_SNAP 1         //0 or 1 (0 = will output raw analog stick angle, 1 will snap to 45deg angles)
#define SNAP_RANGE 10          //+/- what angle range will snap. 10 will be +/-10 degrees from a 45 degree angle
#define MOUSE_SENSITIVITY 2.0f //Not configurable from the N64 console.
#define MAG_AT_45DEG 1.1f      //Octagonal shape has a larger magnitude at the 45degree points. 1.1 times larger seems about right

/* DEBUG PRINTERS */
#include "printf.h"
#define debug_print_status(fmt, ...)     do { if (DEBUG_STATUS)  printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_n64(fmt, ...)        do { if (DEBUG_N64)     printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_tpak(fmt, ...)       do { if (DEBUG_TPAK)    printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_usbhost(fmt, ...)    do { if (DEBUG_USBHOST) printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_fatfs(fmt, ...)      do { if (DEBUG_FATFS)   printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_memory(fmt, ...)     do { if (DEBUG_MEMORY)  printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_error(fmt, ...)      do { if (DEBUG_ERROR)   printf(fmt, ##__VA_ARGS__); } while (0)
#endif
