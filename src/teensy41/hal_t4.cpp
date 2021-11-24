// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "n64_controller.h"
#include "n64_wrapper.h"
#include "memory.h"
#include "usb64_conf.h"
#include "fileio.h"
#include "memory.h"

void n64hal_system_init()
{
    NVIC_SET_PRIORITY(IRQ_GPIO6789, 1);
}

void n64hal_debug_init()
{
    serial_port.begin(256000);
}

void n64hal_debug_write(char c)
{
    serial_port.write(c);
}

void n64hal_disable_interrupts()
{
    noInterrupts();
}

void n64hal_enable_interrupts()
{
    interrupts();
}

/*
 * Function: Reads a hardware realtime clock and populates day,h,m,s.
 * Used by Pokemon Gameboy games only with TPAK that have a RTC.
 * ----------------------------
 *   Returns void
 *
 *   day_high: Bit 0  Most significant bit of Day Counter (Bit 8)
 *             Bit 6  Halt (0=Active, 1=Stop Timer)
 *             Bit 7  Day Counter Carry Bit (1=Counter Overflow)
 *   day_low: Lower 8 bits of Day Counter (0-255)
 *   h: Pointer to an hour value (0-23)
 *   m: Pointer to a minute value (0-59)
 *   s: Pointer to a second value (0-59)
 */
void n64hal_rtc_read(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s)
{
    //Not implemented
}

/*
 * Function: Writes to a hardware realtime clock with day,h,m,s,dst
 * Used by Pokemon Gameboy games only with TPAK that have a RTC.
 * ----------------------------
 *   Returns void
 *
 *   day_high: Bit 0  Most significant bit of Day Counter (Bit 8)
 *             Bit 6  Halt (0=Active, 1=Stop Timer)
 *             Bit 7  Day Counter Carry Bit (1=Counter Overflow)
 *   day_low: Lower 8 bits of Day Counter (0-255)
 *   h: Pointer to an hour value (0-23)
 *   m: Pointer to a minute value (0-59)
 *   s: Pointer to a second value (0-59)
 */
void n64hal_rtc_write(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s)
{
    //Not implemented
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
 * Function: Get the current value of the high speed clock.
 * Speed critical!
 * ----------------------------
 *   Returns: A timer count that should run fairly fast (>20Mhz or so)
 */
uint32_t n64hal_hs_tick_get()
{
    return ARM_DWT_CYCCNT;
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
void n64hal_input_swap(n64_input_dev_t *controller, uint8_t val)
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
 * Function: Returns the data line level for the pin passed to this function.
 * Speed critical!
 * ----------------------------
 *   Returns: 1 of the line if high, or 0 if the line is low.
 *
 *   Pin number: (See usb64_conf.h)
 */
uint8_t n64hal_input_read(int pin)
{
    return digitalReadFast(pin);
}

/*
 * Function: Sets the GPIO mode of a pin
 * ----------------------------
 *   Returns: void
 *
 *   Pin number (See usb64_conf.h)
 *   val: N64_OUTPUT or N64_INPUT_PULLDOWN or N64_INPUT_PULLUP
 */
void n64hal_pin_set_mode(int pin, uint8_t mode)
{
    switch (mode)
    {
    case N64_OUTPUT:
        pinMode(pin, OUTPUT);
        break;
    case N64_INPUT_PULLDOWN:
        pinMode(pin, INPUT_PULLDOWN);
        break;
    case N64_INPUT_PULLUP:
    default:
        pinMode(pin, INPUT_PULLUP);
        break;
    }
}

/*
 * Function: Sets an output GPI to a level
 * Speed critical!
 * ----------------------------
 *   Returns: void.
 *
 *   pin: Arduino compatible pin number
 *   level: 1 or 0
 */
void n64hal_output_set(uint8_t pin, uint8_t level)
{
    digitalWriteFast(pin, level);
}

void n64hal_attach_interrupt(uint8_t pin, void (*handler)(void), int mode)
{
    int _mode = -1;
    switch (mode)
    {
        case N64_INTMODE_CHANGE: _mode = CHANGE; break;
        case N64_INTMODE_FALLING: _mode = FALLING; break;
        case N64_INTMODE_RISING: _mode = RISING; break;
    }
    if (_mode != -1)
    {
        attachInterrupt(digitalPinToInterrupt(pin), handler, _mode);
    }
}

void n64hal_detach_interrupt(uint8_t pin)
{
    detachInterrupt(digitalPinToInterrupt(pin));
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
void n64hal_read_extram(void *rx_buff, void *src, uint32_t offset, uint32_t len)
{
    memcpy(rx_buff, (void *)((uint32_t)src + offset), len);
}

/*
 * Function: Writes an array of data read to external ram.
 * ----------------------------
 *   Returns: void
 *
 *   tx_buff: The buffer of data to write
 *   dst: Pointer to the base address of the destination data.
 *   offset: Bytes from the base address where we need to write.
 *   len: How many bytes to write.
 */
void n64hal_write_extram(void *tx_buff, void *dst, uint32_t offset, uint32_t len)
{
    memcpy((void *)((uint32_t)dst + offset), tx_buff, len);
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
            if (rom_count < max)
            {
                gb_list[rom_count] = (char *)memory_dev_malloc(strlen(file_list[i]) + 1);
                strcpy(gb_list[rom_count], file_list[i]);
                rom_count++;
            }
        }
        //Free file list as we go
        memory_dev_free(file_list[i]);
    }
    return rom_count;
}

/*
 * Function: Reads data from unbuffered memory. (i.e SD card)
 * ----------------------------
 *   Returns: Number of roms found
 *
 *   name: Name of file
 *   file_offset: Located to start reading file
 *   data: buffer to put data in
 *   len: number of bytes t0 read.
 */
void n64hal_read_storage(char *name, uint32_t file_offset, uint8_t *data, uint32_t len)
{
    fileio_read_from_file(name, file_offset, data, len);
}
