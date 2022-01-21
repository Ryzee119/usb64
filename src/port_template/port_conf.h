// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT
#ifndef _USB64_CONF_h
#define _USB64_CONF_h

/* PIN MAPPING - Teensy uses an Arduino Backend, we just assign the enum to the Arduino Pin number
   USB64_PIN_MAX must be the largest pin number in the list add one*/
typedef enum {
   USER_LED_PIN,
   N64_FRAME_PIN,
   HW_RUMBLE,
   N64_CONSOLE_SENSE_PIN,
   N64_CONTROLLER_1_PIN,
   N64_CONTROLLER_2_PIN,
   N64_CONTROLLER_3_PIN,
   N64_CONTROLLER_4_PIN,
   USB64_PIN_MAX,
} usb64_pin_t;

/* TFT DISPLAY */
#define ENABLE_TFT_DISPLAY 1
//Draw to a framebuffer, which you can then send to a screen (via DMA etc)
#define TFT_USE_FRAMEBUFFER 0
#define TFT_WIDTH 320
#define TFT_HEIGHT 240
//The TFT image is drawn within the framebuffer size. You can have this differ from above
//if you dont want to draw into the entire screen space.
#define TFT_FRAMEBUFFER_WIDTH 320
#define TFT_FRAMEBUFFER_HEIGHT 240
#define TFT_PIXEL_SIZE 2 //16bpp = 2, 32bpp = 4

/* Optional, to save RAM, define for variables to store in flash only */
#ifndef PROGMEM
#define PROGMEM
#endif
/* Optional, to save RAM, define for function to store in flash only */
#ifndef FLASHMEM
#define FLASHMEM
#endif

#endif
