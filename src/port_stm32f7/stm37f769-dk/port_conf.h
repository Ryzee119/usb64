// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT
#ifndef _USB64_CONF_h
#define _USB64_CONF_h

//Specific headers for this port
#include <stm32f7xx_hal.h>
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_sd.h"
#include "stm32f769i_discovery_sdram.h"
#include "main.h"

/* PIN MAPPING - Teensy uses an Arduino Backend, we just assign the enum to the Arduino Pin number
   USB64_PIN_MAX must be the largest pin number in the list add one*/
typedef enum {
   USER_LED_PIN = -1,
   N64_FRAME_PIN = -1,
   HW_RUMBLE = -1,
   N64_CONSOLE_SENSE_PIN,
   N64_CONTROLLER_1_PIN,
   N64_CONTROLLER_2_PIN,
   N64_CONTROLLER_3_PIN,
   N64_CONTROLLER_4_PIN,
/*
   N64_FRAME_PIN,
   USER_LED_PIN,
   HW_A,
   HW_B,
   HW_CU,
   HW_CD,
   HW_CL,
   HW_CR,
   HW_DU,
   HW_DD,
   HW_DL,
   HW_DR,
   HW_START,
   HW_Z,
   HW_R,
   HW_L,
   HW_RUMBLE, //Output, 1 when should be rumbling
   HW_EN, //Active low, pulled high
   HW_X, //Analog input, 0V to VCC. VCC/2 centre
   HW_Y, //Analog input, 0V to VCC. VCC/2 centre
   TFT_DC,
   TFT_CS,
   TFT_MOSI,
   TFT_SCK,
   TFT_MISO,
   TFT_RST,
*/
   USB64_PIN_MAX,
} usb64_pin_t;

/* TFT DISPLAY */
#define ENABLE_TFT_DISPLAY 1
#define TFT_WIDTH 800
#define TFT_HEIGHT 472
#define TFT_PIXEL_SIZE 4
#define TFT_USE_FRAMEBUFFER 0

/* Define for variables to store in flash only */
#ifndef PROGMEM
#define PROGMEM
#endif
/* Define for function to store in flash only */
#ifndef FLASHMEM
#define FLASHMEM
#endif

#endif
