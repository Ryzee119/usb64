// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _TFT_H
#define _TFT_H

#include <Arduino.h>
#include "ILI9341_t3n.h"

#define CENTER ILI9341_t3n::CENTER
#define BG_COLOUR 0x10A2

void tft_init();
uint8_t tft_change_page(uint8_t page);
void tft_force_update(); //Will block until previous DMA complete.
void tft_try_update();   //Will return if DMA busy, or framebuffer hasnt changed
void tft_flag_update();  //Mark the framebuffer as dirty, draw at next update call
void tft_add_log(char c);

#endif