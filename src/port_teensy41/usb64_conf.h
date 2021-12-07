// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "printf.h"
#undef printf
#undef sprintf
#undef snprinf
#undef vsnprintf 
#undef vprintf

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
#define MAX_CONTROLLERS 4             //Max is 4
#define MAX_MICE 4                    //0 to disable N64 mouse support. Must be <= MAX_CONTROLLERS
#define MAX_KB 4                      //0 to disable N64 randnet keyboard support. Must be <= MAX_CONTROLLERS
#define MAX_GBROMS 10                 //ROMS over this will just get ignored
#define ENABLE_I2C_CONTROLLERS 0      //Received button presses over I2C, useful for integrating with a rasp pi etc.
#define ENABLE_HARDWIRED_CONTROLLER 1 //Ability to hardware a N64 controller into the usb64.
#define PERI_CHANGE_TIME 750          //Milliseconds to simulate a peripheral changing time. Needed for some games.

//Not used for Arduino backend
typedef struct
{
   int pin;
} dev_gpio_t;

/* PIN MAPPING - Teensy uses an Arduino Backend, we just assign the enum to the Arduino Pin number
   USB64_PIN_MAX must be the largest pin number in the list add one*/
typedef enum {
   N64_CONSOLE_SENSE_PIN = 37,
   N64_CONTROLLER_1_PIN = 36,
   N64_CONTROLLER_2_PIN = 35,
   N64_CONTROLLER_3_PIN = 34,
   N64_CONTROLLER_4_PIN = 33,
   USER_LED_PIN = 13,
   N64_FRAME_PIN = 23,
   HW_A = 2,
   HW_B = 3,
   HW_CU = 4,
   HW_CD = 5,
   HW_CL = 6,
   HW_CR = 7,
   HW_DU = 8,
   HW_DD = 9,
   HW_DL = 10,
   HW_DR = 11,
   HW_START = 12,
   HW_Z = 28,
   HW_R = 29,
   HW_L = 30,
   HW_RUMBLE = 31, //Output, 1 when should be rumbling
   HW_EN = 32, //Active low, pulled high
   HW_X = 24, //Analog input, 0V to VCC. VCC/2 centre
   HW_Y = 25, //Analog input, 0V to VCC. VCC/2 centre
   TFT_DC = 40,
   TFT_CS = 41,
   TFT_MOSI = 26,
   TFT_SCK = 27,
   TFT_MISO = 39,
   TFT_RST = -1,
   USB64_PIN_MAX = 42,
} usb64_pin_t;

/* FILESYSTEM */
#define MAX_FILENAME_LEN 256
#define SETTINGS_FILENAME "SETTINGS.DAT"
#define GAMEBOY_SAVE_EXT ".SAV" //ROMFILENAME.SAV
#define MEMPAK_SAVE_EXT ".MPK" //MEMPAKXX.MPK

/* FIRMWARE DEFAULTS (CONFIGURABLE DURING USE) */
#define DEFAULT_SENSITIVITY 2  //0 to 4 (0 = low sensitivity, 4 = max)
#define DEFAULT_DEADZONE 2     //0 to 4 (0 = no deadzone correction, 4 = max (40%))
#define DEFAULT_SNAP 1         //0 or 1 (0 = will output raw analog stick angle, 1 will snap to 45deg angles)
#define DEFAULT_OCTA_CORRECT 1 //0 or 1 (Will correct the circular analog stuck shape to N64 octagonal)

/* FIRMWARE DEFAULTS (NOT CONFIGURABLE DURING USE) */
#define SNAP_RANGE 5           //+/- what angle range will snap. 5 will snap to 45 degree if between 40 and 50 degrees.
#define MOUSE_SENSITIVITY 2.0f //Just what felt right to me with my mouse.
#define MAG_AT_45DEG 1.1f      //Octagonal shape has a larger magnitude at the 45degree points. 1.1 times larger seems about right

/* TFT DISPLAY */
#define ENABLE_TFT_DISPLAY 1
#define TFT_ROTATION 1 //0-3
#define TFT_WIDTH 320
#define TFT_HEIGHT 240
#define TFT_PIXEL_SIZE 2
#define TFT_USE_FRAMEBUFFER 1

/* Define for variables to store in flash only */
#ifndef PROGMEM
#define PROGMEM
#endif
/* Define for function to store in flash only */
#ifndef FLASHMEM
#define FLASHMEM
#endif


/* DEBUG PRINTERS */
#define debug_print_status(fmt, ...)     do { if (DEBUG_STATUS)  usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_n64(fmt, ...)        do { if (DEBUG_N64)     usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_tpak(fmt, ...)       do { if (DEBUG_TPAK)    usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_usbhost(fmt, ...)    do { if (DEBUG_USBHOST) usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_fatfs(fmt, ...)      do { if (DEBUG_FATFS)   usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_memory(fmt, ...)     do { if (DEBUG_MEMORY)  usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_error(fmt, ...)      do { if (DEBUG_ERROR)   usb64_printf(fmt, ##__VA_ARGS__); } while (0)

#endif