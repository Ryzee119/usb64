/* MIT License
 * 
 * Copyright (c) [2020] [Ryan Wendland]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _USB_HOST_H
#define _USB_HOST_H

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

#endif
