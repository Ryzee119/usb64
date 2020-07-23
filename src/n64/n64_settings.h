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

#ifndef N64_N64_SETTINGS_H_
#define N64_N64_SETTINGS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "n64_conf.h"

typedef struct
{
    uint8_t magic;
    char default_tpak_rom[MAX_CONTROLLERS][MAX_FILENAME_LEN]; //Filename
    uint8_t sensitivity[MAX_CONTROLLERS];        //0 to 5
    uint8_t deadzone[MAX_CONTROLLERS];           //0 to 3
    uint8_t snap_axis[MAX_CONTROLLERS];          //0 or 1
} n64_settings;

void n64_settings_init(n64_settings* settings);
n64_settings *n64_settings_get();

#ifdef __cplusplus
}
#endif

#endif /* N64_N64_SETTINGS_H_ */
