// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "n64_settings.h"
#include "n64_wrapper.h"
#include "printf.h"

n64_settings *_settings = NULL;

static uint8_t _calc_checksum(n64_settings *settings)
{
    uint8_t checksum = 0;
    for (int i = 0; i < sizeof(n64_settings) - 1; i++)
    {
        checksum += ((uint8_t *)settings)[i];
    }
    return checksum;
}

//*settings should be a globally available allocated block of memory
void n64_settings_init(n64_settings *settings)
{
    if (settings->start != 0x64 || settings->checksum != _calc_checksum(settings))
    {
        debug_print_error("[N64 SETTINGS] ERROR: %s not found or invalid, setting to default\n", SETTINGS_FILENAME);
        for (int i = 0; i < MAX_CONTROLLERS; i++)
        {
            memset(settings, 0x00, sizeof(n64_settings));
            settings->start = 0x64; //N64 :)
            strcpy(settings->default_tpak_rom[i], "");
            settings->deadzone[i]    = DEFAULT_DEADZONE;
            settings->sensitivity[i] = DEFAULT_SENSITIVITY;
            settings->snap_axis[i]   = DEFAULT_SNAP;
        }
        settings->checksum = _calc_checksum(settings);
    }
    _settings = settings;
}

void n64_settings_update_checksum(n64_settings *settings)
{
    uint8_t start = 0x64; 
    n64hal_buffered_write(&start, settings, 0, 1); //This marks the memory as dirty.
    settings->checksum = _calc_checksum(settings);
    debug_print_n64("[N64 SETTINGS] Settings updated, new checksum %02x\n", settings->checksum);
}

n64_settings *n64_settings_get()
{
    if (_settings == NULL)
    {
        debug_print_error("[N64 SETTINGS] ERROR: n64_settings_get returning null. Did you init?\n");
    }
    return _settings;
}
