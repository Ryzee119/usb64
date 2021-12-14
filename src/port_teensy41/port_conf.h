// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT
#ifndef _USB64_CONF_h
#define _USB64_CONF_h

#define serial_port Serial1

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

#endif
