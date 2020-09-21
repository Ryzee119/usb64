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

#include <Arduino.h>
#include "n64_controller.h"
#include "n64_wrapper.h"
#include "memory.h"
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


/*
 * Function: Returns an array of data read from external ram.
 * ----------------------------
 *   Returns: void
 *
 *   rx_buff: The buffer the returned data will be stored
 *   src: Pointer to the base address of the source data.
 *   offset: Bytes from the base address the actual data is we need.
 *   len: How many bytes to read.
 */
void n64hal_ram_read(void *rx_buff, void *src, uint32_t offset, uint32_t len)
{
    memcpy(rx_buff + offset, src, len);
}

/*
 * Function: Writes an array of data read to external ram.
 * ----------------------------
 *   Returns: void
 *
 *   tx_buff: The buffer of data to write
 *   src: Pointer to the base address of the destination data.
 *   offset: Bytes from the base address where we need to write.
 *   len: How many bytes to write.
 */
void n64hal_ram_write(void *tx_buff, void *dst, uint32_t offset, uint32_t len)
{
    memcpy(dst + offset, tx_buff, len);
    memory_mark_dirty(dst);
}

/*
 * Function: Returns a list of gameboy roms located on nonvolatile storage
 * WARNING: This mallocs memory on the heap. It must be free'd by user.
 * ----------------------------
 *   Returns: Number of roms found
 *
 *   gb_list: A list of char pointers to populate
 *   max: Max number of roms to return
 */
uint32_t n64hal_list_gb_roms(char **gb_list, uint32_t max)
{
    //Retrieve full directory list
    char *file_list[256];
    uint32_t num_files = fileio_list_directory(file_list, 256);
    
    //Find only files with .gb or gbc extensions to populate rom list.
    uint32_t rom_count = 0;
    for (uint32_t i = 0; i < num_files; i++)
    {
        if (file_list[i] == NULL)
            continue;

        if (strstr(file_list[i], ".GB\0") != NULL || strstr(file_list[i], ".GBC\0") != NULL ||
            strstr(file_list[i], ".gb\0") != NULL || strstr(file_list[i], ".gbc\0") != NULL)
        {
            gb_list[rom_count] = (char *)malloc(strlen(file_list[i]) + 1);
            strcpy(gb_list[rom_count], file_list[i]);
            rom_count++;
        }
        //Free file list as we go
        free(file_list[i]);
    }

    return rom_count;
}
