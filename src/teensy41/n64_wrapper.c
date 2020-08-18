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
 * TODO: Remove dependenceny on USBHOST_t36 and use TinyUSB instead (its more portable)
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <Arduino.h>
#include "ff.h"
#include "qspi.h"
#include "n64_mempak.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_controller.h"
#include "n64_wrapper.h"
#include "usb64_conf.h"

FATFS fs;

/* clmt_clust() and clst2sect() have been copied from the FATFs ff.c file to be used in this wrapper.
   Unfortunately they are not visible outside of ff.c and I needed them :( */
static DWORD clmt_clust(DWORD *clmt, FSIZE_t ofs)
{
    DWORD cl, ncl, *tbl;
    tbl = clmt + 1;
    cl = (DWORD)(ofs / FF_MAX_SS / 1);
    for (;;)
    {
        ncl = *tbl++;
        if (ncl == 0) return 0;
        if (cl < ncl) break;
        cl -= ncl;
        tbl++;
    }
    return cl + *tbl;
}
static LBA_t clst2sect(FATFS *fs, DWORD clst)
{
    clst -= 2;
    if (clst >= fs->n_fatent - 2) return 0;
    return fs->database + (LBA_t)fs->csize * clst;
}

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
 * Function: Reads a packet of data from src and places it in rxdata of length len.
 * Speed critical!
 * ----------------------------
 *   Returns void
 *
 *   rxdata: Pointer to a destination data array.
 *   src: Pointer to the src data.
 *   address: The address offset from the src array origin
 *   len: Length of data to read in bytes.
 */
void n64hal_sram_read(uint8_t *rxdata, uint8_t *src, uint16_t address, uint16_t len)
{
    if (src != NULL)
    {
        memcpy(rxdata, src + address, len);
    }
}

/*
 * Function: Writes a packet of data from txdata to dest of length len.
 * Speed critical!
 * ----------------------------
 *   Returns void
 *
 *   txdata: Pointer to a data array to write
 *   dest: Pointer to the destination data array.
 *   address: The address offset from the dest array origin
 *   len: Length of data to write in bytes.
 */
void n64hal_sram_write(uint8_t *txdata, uint8_t *dest, uint16_t address, uint16_t len)
{
    if (dest != NULL)
    {
        memcpy(dest + address, txdata, len);
    }
}

/*
 * Function: Reads a packet of data from a gameboy cart ROM indicated by gb_cart.
 * Speed critical!
 *
 * ----------------------------
 *   Returns void
 *
 *   gb_cart: Pointer to the gb_cart object to read.
 *   address: The address offset of the gb rom. 0 being the start of the ROM.
 *   data: Pointer to the destination array.
 *   len: Length of data to read in bytes.
 */
uint8_t n64hal_rom_fastread(gameboycart *gb_cart, uint32_t address, uint8_t *data, uint32_t len)
{
    //Due to FATFS caching and overhead, pure f_open/f_seek/f_read calls were too slow for the n64 console.
    //Therefore, I use the FATFS system to open the file and create the cluster link map table for the file only once.
    //Then from that point on I use that table to perform raw flash reads at the correct flash sectors to get the data.
    //Arrays are used to manage multiple tpaks simulatenously (multiple roms being accessed).

    //Store a list of open filenames
    static uint8_t open_files[MAX_GBROMS][MAX_FILENAME_LEN];

    //Store an array of the cluster link map table for each file.
    //FIXME, is 32 ok or should I dynamically alloc the correct length?
    static DWORD clmt[MAX_GBROMS][32];

    FRESULT res;
    FIL fil;
    uint32_t sector_size;
    qspi_get_flash_properties(&sector_size, NULL);

    //If FATFS isn't mounted, mount it now
    if (fs.fs_type == 0)
    {
        debug_print_status("Mounting fs\n");
        f_mount(&fs, "", 1);
    }

    //Sanity check the inputs
    if (gb_cart == NULL || gb_cart->filename[0] == '\0' || data == NULL)
    {
        return 0;
    }

    //Find if the file is already open
    int i = 0;
    for (i = 0; i < MAX_GBROMS; i++)
    {
        if (strcmp((const char *)open_files[i], (char *)gb_cart->filename) == 0)
        {
            //debug_print_status("Found %s at slot %u\n", (char *)gb_cart->filename, i);
            break;
        }
    }

    //File not already open, open it and allocate it to a free spot
    if (i == MAX_GBROMS)
    {
        for (i = 0; i < MAX_GBROMS; i++)
        {
            const char *filename = (char *)gb_cart->filename;
            if (open_files[i][0] == '\0')
            {
                debug_print_status("%s allocated at slot %u\n", filename, i);
                strcpy((char *)open_files[i], (char *)filename);
                break;
            }
        }
        if (i == MAX_GBROMS)
        {
            debug_print_error("ERROR: File not opened. Too many files already open\n");
            return 0;
        }
    }

    //If the file is newly opened, build cluster link map table for fast seek
    if (clmt[i][0] == 0)
    {
        const char *filename = (char *)gb_cart->filename;
        debug_print_status("Building cluster link map table for %s\n", filename);
        res = f_open(&fil, (const TCHAR *)filename, FA_READ);
        if (res != FR_OK)
        {
            open_files[i][0] = '\0';
            return 0;
        }
        fil.cltbl = clmt[i];
        clmt[i][0] = sizeof(clmt);
        res = f_lseek(&fil, CREATE_LINKMAP);
        f_close(&fil);
    }

    //For speed, skip the fatfs f_read/f_seek and read with the backend QSPI at the address
    DWORD sector = clst2sect(&fs, clmt_clust(clmt[i], address));
    qspi_read(sector * sector_size + (address % sector_size), len, data);
    return 1;
}

/*
 * Function: Returns are array of strings for each gameboy rom available to the system up to max.
 * Not speed critical
 * ----------------------------
 *   Returns: Number of gb roms available.
 *
 *   array: array of char pointers of length greater than max.
 *   max: Max number of gb roms to find. Function exits with max is reached.
 */
uint8_t n64hal_scan_for_gbroms(char **array, int max)
{
    FRESULT res;
    DIR dir;
    UINT file_count = 0;
    static FILINFO fno;
    res = f_opendir(&dir, "");
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0 || file_count >= max)
                break;

            if (strstr(fno.fname, ".GB\0") != NULL || strstr(fno.fname, ".GBC\0") != NULL ||
                strstr(fno.fname, ".gb\0") != NULL || strstr(fno.fname, ".gbc\0") != NULL)
            {
                array[file_count] = (char *)malloc(strlen(fno.fname) + 1);
                strcpy(array[file_count], fno.fname);
                file_count++;
            }
        }
        f_closedir(&dir);
    }
    return file_count;
}

/*
 * Function: Backup a SRAM file to non-volatile storage.
 * ----------------------------
 *   Returns: Void
 *
 *   filename: The filename of the saved file
 *   data: Pointer to the array of data to be saved
 *   len: Number of bytes to save.
 */
void n64hal_sram_backup_to_file(uint8_t *filename, uint8_t *data, uint32_t len)
{
    //This function will overwrite the file if it already exists.

    if (fs.fs_type == 0)
    {
        debug_print_status("Mounting fs\n");
        f_mount(&fs, "", 1);
    }
    //Trying open the file
    FRESULT res;
    UINT br;
    FIL fil;
    res = f_open(&fil, (const TCHAR *)filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK)
    {
        debug_print_error("ERROR: Could not open %s for WRITE\n", filename);
        return;
    }
    //Cool, try write to the file
    res = f_write(&fil, data, len, &br);
    if (res != FR_OK || br != len)
    {
        debug_print_error("ERROR: Could not write %s\n", filename);
    }
    else
    {
        debug_print_status("Writing %s ok!\n", filename);
    }
    f_close(&fil);
}

/*
 * Function: Restore a SRAM file from non-volatile storage. This will return 0x00's if the file does not exist.
 * Not speed critical
 * ----------------------------
 *   Returns: Void
 *
 *   filename: The filename of the saved file
 *   data: Pointer to the array of data to be restored to
 *   len: Number of bytes to restore.
 */
void n64hal_sram_restore_from_file(uint8_t *filename, uint8_t *data, uint32_t len)
{
    if (fs.fs_type == 0)
    {
        debug_print_status("Mounting fs\n");
        f_mount(&fs, "", 1);
    }
    //Trying open the file
    FRESULT res;
    UINT br;
    FIL fil;
    res = f_open(&fil, (const TCHAR *)filename, FA_READ);
    if (res != FR_OK)
    {
        debug_print_status("WARNING: Could not open %s for READ\n", filename);
        memset(data, 0x00, len);
        return;
    }
    //Cool, try read the file
    res = f_read(&fil, data, len, &br);
    if (res != FR_OK)
    {
        debug_print_error("ERROR: Could not read %s with error %i\n", filename, res);
        int attempts = 3;
        while (res != FR_OK && attempts-- > 0)
            res = f_read(&fil, data, len, &br);

        if(res != FR_OK)
        {
            debug_print_error("ERROR: Could not read %s with 3 attempts\n", filename);
            memset(data, 0x00, len);
        }
    }
    else
    {
        debug_print_status("Reading %s ok!\n", filename);
    }
    f_close(&fil);
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