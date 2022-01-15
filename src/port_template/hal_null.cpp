// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include "common.h"
#include "memory.h"
#include "fileio.h"
#include "memory.h"

/*
 * Function: Initialse any device specifc aspects
 * ----------------------------
 *   Returns: void
 */
void n64hal_system_init()
{
}

/*
 * Function: Initialse the device debuffer (i.e UART etc)
 * ----------------------------
 *   Returns: void
 */
void n64hal_debug_init()
{
}

/*
 * Function: Initialse the device GPIO for all pins set in usb64_conf.h
 * Note, n64 controller pins must be input pullup wth falling edge interrupt enabled.
 * ----------------------------
 *   Returns: void
 */
void n64hal_gpio_init()
{
}

/*
 * Function: Write a character to a debug output (i.e UART etc)
 * ----------------------------
 *   Returns: void
 */
void n64hal_debug_write(char c)
{
}

/*
 * Function: Disable global interrupts for the device.
 * ----------------------------
 *   Returns: void
 */
void n64hal_disable_interrupts()
{
}

/*
 * Function: Enable global interrupts for the device.
 * ----------------------------
 *   Returns: void
 */
void n64hal_enable_interrupts()
{
}

/*
 * Function: Attach an interrupt from a pin (See usb64_conf.h for pin numbers)
 * ----------------------------
 *   Returns: void
 *
 *   pin: The pin the configure (See usb64_conf.h)
 *   handler: Interrupt handler function in the form `void my_int_handler(void)`
 *   mode: N64_INTMODE_FALLING or N64_INTMODE_CHANGE or N64_INTMODE_RISING
 */
void n64hal_attach_interrupt(usb64_pin_t pin, void (*handler)(void), int mode)
{
}

/*
 * Function: Disconnect an interrupt from a pin (See usb64_conf.h for pin numbers)
 * ----------------------------
 *   Returns: void
 *
 *   pin: The pin the configure (See usb64_conf.h)
 */
void n64hal_detach_interrupt(usb64_pin_t pin)
{
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
}

/*
 * Function: Returns to clock rate of the high speed clock in Hz.
 * Speed critical!
 * ----------------------------
 *   Returns: The rate of the high speed clock in Hz
 */
uint32_t n64hal_hs_tick_get_speed()
{
    return 0;
}

/*
 * Function: Get the current value of the high speed clock.
 * Speed critical!
 * ----------------------------
 *   Returns: A timer count that should run fairly fast (>20Mhz or so)
 */
uint32_t n64hal_hs_tick_get()
{
    return 0;
}

/*
 * Function: Get the number of milliseconds since power up.
 * ----------------------------
 *   Returns: 32 bit tick value at 1000Hz.
 */
uint32_t n64hal_millis()
{
    return 0;
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
void n64hal_input_swap(usb64_pin_t pin, uint8_t val)
{
}

/*
 * Function: Returns the data line level for the pin passed to this function.
 * Speed critical!
 * ----------------------------
 *   Returns: 1 of the line if high, or 0 if the line is low.
 *
 *   pin: The pin the configure (See usb64_conf.h)
 */
uint8_t n64hal_input_read(usb64_pin_t pin)
{
    return 0;
}

/*
 * Function: Sets an output GPI to a level
 * Speed critical!
 * ----------------------------
 *   Returns: void.
 *
 *   pin: The pin the configure (See usb64_conf.h)
 *   level: 1 or 0
 */
void n64hal_output_set(usb64_pin_t pin, uint8_t level)
{
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
    memcpy(rx_buff, (void *)((uintptr_t)src + offset), len);
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
    memcpy((void *)((uintptr_t)dst + offset), tx_buff, len);
    memory_mark_dirty(dst);
}

/*
 * Function: Allocates a block of memory
 * ----------------------------
 *   Returns: pointer to memory region
 *
 *   len: How many bytes to allocate
 */
void *n64hal_malloc(uint32_t len)
{
    return memory_dev_malloc(len);
}

/*
 * Function: Free a block of memory
 * ----------------------------
 *   addr: The address to free
 */
void n64hal_free(void *addr)
{
    memory_dev_free(addr);
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
