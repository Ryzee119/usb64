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

/* DEBUGGING OUTPUT */
#define serial_port Serial1

/* N64 LIB */
#define MAX_CONTROLLERS 4 //Max is 4
#define MAX_GBROMS 10 //ROMS over this will just get ignored

/* PIN MAPPING */
#define N64_CONTROLLER_1_PIN 41
#define N64_CONTROLLER_2_PIN 27
#define N64_CONTROLLER_3_PIN 3 //FIXME
#define N64_CONTROLLER_4_PIN 4 //FIXME
#define USER_BUTTON_PIN 40
#define USER_LED_PIN 13

/* FILESYSTEM */
#define MAX_FILENAME_LEN 256
#define SETTINGS_FILENAME "SETTINGS.DAT"

/* FIRMWARE DEFAULTS (CONFIGURABLE DURING USE) */
#define DEFAULT_SENSITIVITY 3 //0 to 6 (0 = low sensitivity, 6 = max)
#define DEFAULT_DEADZONE 2    //0 to 4 (0 = no deadzone correction, 4 = max (40%))
#define DEFAULT_SNAP 1        //0 or 1 (0 = will output raw analog stick angle, 1 will snap to 45deg angles)

#endif