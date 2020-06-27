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

static FATFS fs;

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
void n64hal_read_rtc(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst)
{
}

void n64hal_write_rtc(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst)
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

void n64hal_flash_read_rom(uint8_t *filename, uint32_t address, uint8_t *data, uint32_t len)
{
    static uint8_t open_file[256];
    static FIL fil;
    DWORD clmt[256];
    FRESULT res;

    uint32_t blocksize;
    qspi_get_flash_properties(&blocksize, NULL);

    //Has file changed, open new file and build cluster link map table for fast seek 
    if (strcmp((char *)open_file, (char *)filename) != 0)
    {
        f_close(&fil);
        res = f_open(&fil, (const TCHAR *)filename, FA_READ);   /* Open a file */
        fil.cltbl = clmt;
        clmt[0] = sizeof(clmt); 
        res = f_lseek(&fil, CREATE_LINKMAP);
        strcpy((char *)open_file, (char *)filename);
    }
    res = f_lseek(&fil, address);
    if (res != FR_OK){
        printf("Seek error on %s at %08x\r\n", filename, address);
    }
    qspi_read(fil.sect * blocksize + (address % blocksize), len, data);
}

void n64hal_scan_flash_gbroms(gameboycart *gb_cart)
{
}

void n64hal_backup_sram_to_flash(uint8_t *filename, uint8_t *data, uint32_t len)
{
    
    if (fs.fs_type == 0)
    {
        printf("Mounting fs\r\n");
        f_mount(&fs, "", 1);
    }

    FRESULT res; FIL fil; UINT br;
    res = f_open(&fil, (const TCHAR *)filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK)
    {
        printf("Error opening %s for WRITE\r\n", filename);
        return;
    }
    res = f_write(&fil, data, len, &br);
    if (res != FR_OK || br != len)
    {
        printf("Error writing %s\r\n", filename);
    }
    f_close(&fil);
    
}

void n64hal_read_sram_from_flash(uint8_t *filename, uint8_t *data, uint32_t len)
{
    if (fs.fs_type == 0)
    {
        printf("Mounting fs\r\n");
        f_mount(&fs, "", 1);
    }

    FRESULT res; FIL fil; UINT br;
    res = f_open(&fil, (const TCHAR *)filename, FA_READ);
    if (res != FR_OK)
    {
        //printf("Error opening %s for READ, probably doesn't exist\r\n", filename);
        memset(data, 0x00, len);
        return;
    }
    res = f_read(&fil, data, len, &br);
    if (res != FR_OK)
    {
        printf("Error reading %s with error %i\r\n", filename, res); //Not good :/
        memset(data, 0x00, len);
    }
    f_close(&fil);
}

uint32_t n64hal_milli_tick_get()
{
    return millis();
}

static uint32_t micro_count = 0;
void n64hal_micro_tick_reset()
{
    micro_count = ARM_DWT_CYCCNT;
}

uint32_t n64hal_micro_tick_get()
{
    return (ARM_DWT_CYCCNT - micro_count) / (F_CPU / 1000000);
}

static uint32_t clock_count = 0;
uint32_t n64hal_hs_tick_init()
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
    pinMode(controller->gpio_pin, val);
}

uint8_t n64hal_input_read(n64_controller *controller)
{
    return digitalRead(controller->gpio_pin);
}
