// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef N64_N64_SETTINGS_H_
#define N64_N64_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef struct
{
    uint8_t start;
    char default_tpak_rom[MAX_CONTROLLERS][MAX_FILENAME_LEN]; //Filename
    uint8_t sensitivity[MAX_CONTROLLERS];        //0 to 4
    uint8_t deadzone[MAX_CONTROLLERS];           //0 to 4
    uint8_t snap_axis[MAX_CONTROLLERS];          //0 or 1
    uint8_t octa_correct[MAX_CONTROLLERS];       //0 or 1
    uint8_t checksum;
} n64_settings;

void n64_settings_init(n64_settings* settings);
void n64_settings_update_checksum(n64_settings *settings);
n64_settings *n64_settings_get();

#ifdef __cplusplus
}
#endif

#endif /* N64_N64_SETTINGS_H_ */
