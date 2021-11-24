// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _TFT_H
#define _TFT_H

//TFT API
void tft_init();
uint8_t tft_change_page(uint8_t page);
void tft_force_update(); //Will block until previous DMA complete.
void tft_try_update();   //Will return if DMA busy, or framebuffer hasnt changed
void tft_flag_update();  //Mark the framebuffer as dirty, draw at next update call
void tft_add_log(char c);

//TFT device specific functions.
void tft_dev_init();
void tft_dev_draw(bool force);
void *tft_dev_get_fb();
bool tft_dev_is_busy();

#endif