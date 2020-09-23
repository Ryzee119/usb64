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
#include "usb64_conf.h"
#include "ff.h"
#include "printf.h"
FATFS fs;
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
 * Function: Backup a SRAM file to non-volatile storage.
 * ----------------------------
 *   Returns: Void
 *
 *   filename: The filename of the saved file
 *   data: Pointer to the array of data to be saved
 *   len: Number of bytes to save.
 */
void fileio_write_to_file(char *filename, uint8_t *data, uint32_t len)
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
    //File opened ok. Write to it.
    //As the data might coming from memory mapped IO external RAM,
    //I first buffer to internal RAM.
    uint8_t buffer[512];
    while (len > 0)
    {
        uint32_t b = (len > sizeof(buffer)) ? sizeof(buffer) : len;
        memcpy(buffer, data, b);
        res = f_write(&fil, buffer, b, &br);
        if (res != FR_OK)
            break;
        data += b;
        len -= b;
    }

    f_close(&fil);
    if (res != FR_OK)
    {
        debug_print_error("ERROR: Could not write %s\n", filename);
    }
    else
    {
        debug_print_status("Writing %s ok!\n", filename);
    }
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
void fileio_read_from_file(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len)
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
        debug_print_error("ERROR: Could not read %s with error %i\n", filename, res);
    }
    else
    {
        debug_print_status("Reading %s for %u bytes ok!\n", filename, bytes_read);
    }
}
