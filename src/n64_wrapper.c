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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <Arduino.h>
#include "ff.h"
#include "qspi.h"
#include "printf.h"
#include "n64_mempak.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_controller.h"
#include "n64_wrapper.h"

FATFS fs;

/*
 * Function: Reads a hardware realtime clock and populates day,h,m,s
 * ----------------------------
 *   Returns void
 *
 *   day: Pointer to a day value (0-6)
 *   h: Pointer to an hour value (0-23)
 *   m: Pointer to a minute value (0-59)
 *   s: Pointer to a second value (0-59)
 */
void n64hal_rtc_read(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst)
{
}

void n64hal_rtc_write(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst)
{
}

void n64hal_sram_read(uint8_t *rxdata, uint8_t *src, uint16_t address, uint16_t len)
{
    if (src != NULL)
    {
        memcpy(rxdata, src + address, len);
    }
}

void n64hal_sram_write(uint8_t *txdata, uint8_t *dest, uint16_t address, uint16_t len)
{
    if (dest != NULL)
    {
        memcpy(dest + address, txdata, len);
    }
}
//FIXME FOR 4 TPAKS
uint8_t n64hal_rom_read(gameboycart *gb_cart, uint32_t address, uint8_t *data, uint32_t len)
{
    static uint8_t open_file[MAX_FILENAME_LEN];
    static FIL fil;
    static DWORD clmt[256];
    FRESULT res;
    uint32_t sector_size;
    qspi_get_flash_properties(&sector_size, NULL);

    if (fs.fs_type == 0)
    {
        printf("Mounting fs\r\n");
        f_mount(&fs, "", 1);
    }

    //Reset this function
    if (gb_cart == NULL)
    {
        strcpy((char *)open_file, "");
        return 0;
    }

    if (gb_cart->filename[0] == '\0')
    {
        return 0;
    }

    //Has file changed, open new file and build cluster link map table for fast seek
    const char *filename = (char *)gb_cart->filename;
    if (strcmp((const char *)open_file, filename) != 0)
    {
        printf("Building cluster link map table for %s\n", filename);
        f_close(&fil);
        res = f_open(&fil, (const TCHAR *)filename, FA_READ);
        if (res != FR_OK)
        {
            strcpy((char *)open_file, "");
            return 0;
        }
        fil.cltbl = clmt;
        clmt[0] = sizeof(clmt);
        res = f_lseek(&fil, CREATE_LINKMAP);
        strcpy((char *)open_file, (char *)filename);
    }

    //Fast seek to get sector
    res = f_lseek(&fil, address % 4096 == 0 ? address + 1 : address);
    if (res != FR_OK)
    {
        printf("Seek error on %s at %08x\r\n", filename, address); //Not good!
        strcpy((char *)open_file, "");
        return 0;
    }

    //For speed, skip the fatfs f_read and read with the backend QSPI at the seeked address
    if (data != NULL)
        qspi_read(fil.sect * sector_size + (address % sector_size), len, data);
    return 1;
}

uint8_t n64hal_scan_flash_gbroms(char **array, int max)
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

void n64hal_sram_backup_to_file(uint8_t *filename, uint8_t *data, uint32_t len)
{
    if (fs.fs_type == 0)
    {
        printf("Mounting fs\r\n");
        f_mount(&fs, "", 1);
    }
    //Trying open the file
    FRESULT res;
    FIL fil;
    UINT br;
    res = f_open(&fil, (const TCHAR *)filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK)
    {
        printf("Error opening %s for WRITE\r\n", filename);
        return;
    }
    //Cool, try write to the file
    res = f_write(&fil, data, len, &br);
    if (res != FR_OK || br != len)
    {
        printf("Error writing %s\r\n", filename);
    }
    else
    {
        printf("Writing %s ok!\n", filename);
    }
    f_close(&fil);
}

void n64hal_sram_restore_from_file(uint8_t *filename, uint8_t *data, uint32_t len)
{
    if (fs.fs_type == 0)
    {
        printf("Mounting fs\r\n");
        f_mount(&fs, "", 1);
    }
    //Trying open the file
    FRESULT res;
    FIL fil;
    UINT br;
    res = f_open(&fil, (const TCHAR *)filename, FA_READ);
    if (res != FR_OK)
    {
        //printf("Error opening %s for READ, probably doesn't exist\r\n", filename);
        memset(data, 0x00, len);
        return;
    }
    //Cool, try read the file
    res = f_read(&fil, data, len, &br);
    if (res != FR_OK)
    {
        printf("Error reading %s with error %i\r\n", filename, res); //Not good!
        memset(data, 0x00, len);
    }
    else
    {
        printf("Reading %s ok!\n", filename);
    }
    f_close(&fil);
}

static uint32_t clock_count = 0;
void n64hal_hs_tick_init()
{
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
}

uint32_t n64hal_hs_tick_get_speed()
{
    return F_CPU;
}

void n64hal_hs_tick_reset()
{
    clock_count = ARM_DWT_CYCCNT;
}

uint32_t n64hal_hs_tick_get()
{
    return ARM_DWT_CYCCNT - clock_count;
}

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

uint8_t n64hal_input_read(n64_controller *controller)
{
    return digitalRead(controller->gpio_pin);
}
