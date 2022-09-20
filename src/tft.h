// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _TFT_H
#define _TFT_H

// TFT API
void tft_init();
uint8_t tft_change_page(uint8_t page);    // Change the active TFT page
void tft_update();                        // Update the tft display
void tft_flag_update(uint8_t controller); // Mark the framebuffer as dirty, draw at next update call
void tft_add_log(char c);                 // Add a character to the TFT log screen. Each line should be ended with '\n'

// TFT device specific functions.
void tft_dev_init();

#endif