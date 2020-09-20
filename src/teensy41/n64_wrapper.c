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

/*
 * N64 library wrapper function. The n64 library is intended to be as portable as possible within reason.
 * The hardware layer functions in this file are provided here to hopefully make it easier to port to other microcontrollers
 * This file is provided for a Teensy4.1 using Teensyduino, an external Flash chip (access via FATFS file system) and
 * using the Teensy's generous internal RAM
 * 
 * Supported microcontrollers need >256kb RAM recommended, >256kb internal flash and >100Mhz clock speed or so
 * with high speed external flash (>4Mb)
 * 
 * TODO: Option to disable mempack/tpak features to remove the requirement for external flash or lots of ram
 */

#include <Arduino.h>
#include "n64_controller.h"
#include "n64_wrapper.h"
#include "usb64_conf.h"

/*
 * Function: Reads a hardware realtime clock and populates day,h,m,s
 * ----------------------------
 *   Returns void
 *
 *   day: Pointer to a day value (0-6)
 *   h: Pointer to an hour value (0-23)
 *   m: Pointer to a minute value (0-59)
 *   s: Pointer to a second value (0-59)
 *   dst: Returns 1 if day light savings is active
 */
void n64hal_rtc_read(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst)
{
}

/*
 * Function: Writes to a hardware realtime clock with day,h,m,s,dst
 * ----------------------------
 *   Returns void
 *
 *   day: Pointer to a day value (0-6)
 *   h: Pointer to an hour value (0-23)
 *   m: Pointer to a minute value (0-59)
 *   s: Pointer to a second value (0-59)
 *   dst: Write 1 to enable day light savings
 */
void n64hal_rtc_write(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst)
{
}

/*
 * Function: Enable a high speed clock (>20Mhz or so) which is used for low level accurate timings.
 * Timer range should be atleast 32-bit.
 * Not speed critical
 * ----------------------------
 *   Returns: Void
 */
void n64hal_hs_tick_init()
{
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
}

/*
 * Function: Returns to clock rate of the high speed clock in Hz.
 * Speed critical!
 * ----------------------------
 *   Returns: The rate of the high speed clock in Hz
 */
uint32_t n64hal_hs_tick_get_speed()
{
    return F_CPU;
}

/*
 * Function: Resets the value of the high speed clock.
 * Calling n64hal_hs_tick_get() instantanously (hypothetically) after this would return 0.
 * Speed critical!
 * ----------------------------
 *   Returns: Void
 */
static uint32_t clock_count = 0;
void n64hal_hs_tick_reset()
{
    clock_count = ARM_DWT_CYCCNT;
}

/*
 * Function: Get the current value of the high speed clock.
 * Speed critical!
 * ----------------------------
 *   Returns: Number of clock ticks since n64hal_hs_tick_reset()
 */
uint32_t n64hal_hs_tick_get()
{
    return ARM_DWT_CYCCNT - clock_count;
}

/*
 * Function: Flips the gpio pin direction from an output (driven low) to an input (pulled up)
 *           for the controller passed by controller.
 * Speed critical!
 * ----------------------------
 *   Returns: void
 *
 *   controller: Pointer to the n64 controller struct which contains the gpio mapping
 *   val: N64_OUTPUT or N64_INPUT
 */
void n64hal_input_swap(n64_controller *controller, uint8_t val)
{
    switch (val)
    {
    case N64_OUTPUT:
        pinMode(controller->gpio_pin, OUTPUT);
        break;
    case N64_INPUT:
    default:
        pinMode(controller->gpio_pin, INPUT_PULLUP);
        break;
    }
}

/*
 * Function: Returns the data line level for the n64 controller passed to this function.
 * Speed critical!
 * ----------------------------
 *   Returns: 1 of the line if high, or 0 if the line is low.
 *
 *   controller: Pointer to the n64 controller struct which contains the gpio mapping
 */
uint8_t n64hal_input_read(n64_controller *controller)
{
    return digitalRead(controller->gpio_pin);
}
