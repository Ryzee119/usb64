// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _INPUT_H
#define _INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>

void input_init();
void input_update_input_devices();
bool input_is_connected(int id);
bool input_is_mouse(int id);
bool input_is_gamecontroller(int id);
uint16_t input_get_id_product(int id);
uint16_t input_get_id_vendor(int id);
const char *input_get_manufacturer_string(int id);
const char *input_get_product_string(int id);
uint16_t input_get_buttons(uint8_t id, uint32_t *raw_buttons, int32_t *raw_axis, uint32_t max_axis,
                                       uint16_t *n64_buttons, int8_t *n64_x_axis, int8_t *n64_y_axis, bool *combo_pressed);
void input_apply_rumble(int id, uint8_t stength);

#ifdef __cplusplus
}
#endif

#endif
