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

#ifndef _ANALOG_STICK_H
#define _ANALOG_STICK_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void astick_apply_deadzone(float *out_x, float *out_y, float x, float y, float dz_low, float dz_high);
float astick_apply_sensitivity(int sensitivity, float *x, float *y);
void astick_apply_snap(float range, float *x, float *y);
void astick_apply_octa_correction(float *x, float *y);

#ifdef __cplusplus
}
#endif

#endif
