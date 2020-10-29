// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "usb64_conf.h"
#include "ff.h"
#include "printf.h"
FATFS fs;

void fileio_init()
{
    if (fs.fs_type == 0)
    {
        debug_print_status("[FILEIO] Mounting fs\n");
        if (f_mount(&fs, "", 1) != FR_OK)
        {
            debug_print_error("[MAIN] ERROR: Could not mount FATFS, probably not formatted correctly. Formatting flash...\n");
            MKFS_PARM defopt = {FM_FAT, 1, 0, 0, 4096};
            BYTE *work = (BYTE *)malloc(4096);
            f_mkfs("", &defopt, work, 4096);
            free(work);
            f_mount(&fs, "", 1);
        }
    }
}
/*
 * Function: Returns are array of strings for file the root directory up to max.
 * WARNING: This allocates heap memory, and must be free'd by user.
 * Not speed critical
 * ----------------------------
 *   Returns: Number of files found.
 *
 *   array: array of char pointers of length greater than max.
 *   max: Max number of gb roms to find. Function exits with max is reached.
 */
uint32_t fileio_list_directory(char **list, uint32_t max)
{
    FRESULT res;
    DIR dir;
    UINT file_count = 0;
    static FILINFO fno;

    if (fs.fs_type == 0)
        return 0;

    res = f_opendir(&dir, "");
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0 || file_count >= max)
                break;

            list[file_count] = (char *)malloc(strlen(fno.fname) + 1);
            strcpy(list[file_count], fno.fname);
            file_count++;
        }
        f_closedir(&dir);
    }
    return file_count;
}

/*
 * Function: Backup a ram data to non-volatile storage.
 * ----------------------------
 *   Returns: Void
 *
 *   filename: The filename of the saved file
 *   data: Pointer to the array of data to be saved
 *   len: Number of bytes to save.
 */
void fileio_write_to_file(char *filename, uint8_t *data, uint32_t len)
{
    //Trying open the file
    FRESULT res; UINT br; FIL fil;

    if (fs.fs_type == 0)
        return;

    res = f_open(&fil, (const TCHAR *)filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK)
    {
        debug_print_error("[FILEIO] ERROR: Could not open %s for WRITE\n", filename);
        return;
    }
    //File opened ok. Write to it.
    //As the data might coming from memory mapped IO external RAM,
    //I first buffer to internal RAM.
    uint8_t buffer[512];
    uint32_t bytes_written = 0;
    while (len > 0)
    {
        uint32_t b = (len > sizeof(buffer)) ? sizeof(buffer) : len;
        memcpy(buffer, data, b);
        res = f_write(&fil, buffer, b, &br);
        if (res != FR_OK)
            break;
        data += b;
        len -= b;
        bytes_written += b;
    }

    f_close(&fil);
    if (res != FR_OK)
    {
        debug_print_error("[FILEIO] ERROR: Could not write %s with error %i\n", filename, res);
    }
    else
    {
        debug_print_status("[FILEIO] Writing %s for %u bytes ok!\n", filename, bytes_written);
    }
}

/*
 * Function: Restore a file from non-volatile storage into RAM. This will return 0x00's if the file does not exist.
 * Not speed critical
 * ----------------------------
 *   Returns: Void
 *
 *   filename: The filename of the saved file
 *   file_offset: Number bytes from beginning of file
 *   data: Pointer to the array of data to be restored to
 *   len: Number of bytes to restore.
 */
void fileio_read_from_file(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len)
{
    //Trying open the file
    FRESULT res; UINT br; FIL fil;

    if (fs.fs_type == 0)
        return;

    res = f_open(&fil, (const TCHAR *)filename, FA_READ);
    if (res != FR_OK)
    {
        debug_print_status("[FILEIO] WARNING: Could not open %s for READ\n", filename);
        memset(data, 0x00, len);
        return;
    }
    if (file_offset > 0)
        f_lseek(&fil, file_offset);

    //File opened ok. Read from it.
    //As the data might going to memory mapped IO external RAM,
    //I first buffer to internal RAM.
    uint8_t buffer[512];
    uint32_t bytes_read = 0;
    while (len > 0)
    {
        uint32_t b = (len > sizeof(buffer)) ? sizeof(buffer) : len;
        res = f_read(&fil, buffer, b, &br);
        if (res != FR_OK)
            break;
        memcpy(data, buffer, b);
        data += b;
        len -= b;
        bytes_read += b;
    }

    if (res != FR_OK)
    {
        debug_print_error("[FILEIO] ERROR: Could not read %s with error %i\n", filename, res);
    }
    else
    {
        debug_print_status("[FILEIO] Reading %s for %u bytes ok!\n", filename, bytes_read);
    }
}

static FIL fp;
int fileio_open_file_readonly(const char *filename)
{
    FRESULT res;
    f_close(&fp);

    res = f_open(&fp, filename, FA_READ);

    if (res != FR_OK)
    {
        debug_print_status("Could not open %s for read1\n", filename);
        return 0;
    }
    return 1;
}

void fileio_close_file()
{
    f_close(&fp);
}

int fileio_get_line(char *buffer, int max_len)
{
    FRESULT res;
    UINT br;
    int i = 0;
    char c = 0;

    if (f_eof(&fp))
        return 0;

    while (!f_eof(&fp) && c != '\n')
    {
        res = f_read(&fp, &c, 1, &br);
        buffer[i++] = c;
        if (c == '\n')
            buffer[i++] = '\0';
        if (res != FR_OK || i > (max_len - 1))
            return 0;
    }
    return 1;
}