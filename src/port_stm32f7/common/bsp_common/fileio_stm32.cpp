// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "common.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

static FATFS fs;
static char SDPath[4] = {0};
extern const Diskio_drvTypeDef SD_Driver;

bool fileio_dev_init()
{
    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        return false;
    }

    int retries = 3;

    while (retries)
    {
        if (f_mount(&fs, (TCHAR const *)SDPath, 1) != FR_OK)
        {
            retries--;
            if (retries == 0)
                return false;
            HAL_Delay(100);
        }
        else
        {
            break;
        }
    }

    return f_chdir((TCHAR const *)SDPath) == FR_OK;
}

int fileio_dev_open_dir(const char *dir)
{
    DIR *dp = (DIR *)malloc(sizeof(DIR));
    if (dp == NULL)
    {
        return 0;
    }
    n64hal_disable_interrupts();
    FRESULT res = f_opendir(dp, (const TCHAR *)dir);
    n64hal_enable_interrupts();

    if (res != FR_OK)
    {
        free(dp);
        return 0;
    }

    return (uint32_t)dp;
}

const char *fileio_dev_get_next_filename(int handle)
{
    static FILINFO fno;
    FRESULT res;
    DIR *dp = (DIR *)handle;
    n64hal_disable_interrupts();
    res = f_readdir(dp, &fno);
    n64hal_enable_interrupts();
    if (dp && res == FR_OK && fno.fname[0] != 0)
    {
        return fno.fname;
    }
    return NULL;
}

void fileio_dev_close_dir(int handle)
{
    if (handle == 0)
    {
        return;
    }

    DIR *dp = (DIR *)handle;
    f_closedir(dp);
    free(dp);
}

int fileio_dev_read(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len)
{
    FIL fil;
    FRESULT res;
    UINT br;
    n64hal_disable_interrupts();
    res = f_open(&fil, filename, FA_READ);
    if (res != FR_OK)
    {
        n64hal_enable_interrupts();
        return -1;
    }

    f_lseek(&fil, file_offset);
    res = f_read(&fil, data, len, &br);
    if (res != FR_OK || br != len)
    {
        f_close(&fil);
        n64hal_enable_interrupts();
        return -2;
    }

    f_close(&fil);
    n64hal_enable_interrupts();
    return 0;
}

int fileio_dev_write(char *filename, uint8_t *data, uint32_t len)
{
    FIL fil;
    FRESULT res;
    UINT bw;
    n64hal_disable_interrupts();
    res = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK)
    {
        n64hal_enable_interrupts();
        return -1;
    }

    res = f_write(&fil, data, len, &bw);
    if (res != FR_OK || bw != len)
    {
        f_close(&fil);
        n64hal_enable_interrupts();
        return -2;
    }

    f_close(&fil);
    n64hal_enable_interrupts();
    return 0;
}
