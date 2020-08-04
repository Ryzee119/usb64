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
#ifndef _N64_CONF_h
#define _N64_CONF_h

#define MAX_CONTROLLERS 4 //Max is 4
#define MAX_GBROMS 10 //ROMS over this will just get ignored

/* PIN MAPPING */
#define N64_CONTROLLER_1_PIN 41
#define N64_CONTROLLER_2_PIN 27
#define N64_CONTROLLER_3_PIN 3
#define N64_CONTROLLER_4_PIN 4
#define USER_BUTTON_PIN 40
#define USER_LED_PIN 13

#define MAX_FILENAME_LEN 256
#define SETTINGS_FILENAME "SETTINGS.DAT"

#endif