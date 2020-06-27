/*
 * n64_settings.h
 *
 *  Created on: 6Mar.,2020
 *      Author: wendl
 */

#ifndef N64_N64_SETTINGS_H_
#define N64_N64_SETTINGS_H_

typedef struct
{
    uint8_t default_tpak_rom;
    uint8_t octa[2];
    uint8_t sensitivity[2];
    uint8_t deadzone[2];
    uint8_t snap_axis[2];

} n64_settings;

void n64_settings_read(n64_settings *settings);
void n64_settings_write(n64_settings *settings);

#endif /* N64_N64_SETTINGS_H_ */
