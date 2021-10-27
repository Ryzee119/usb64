// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include <SD.h>
#include "usb64_conf.h"
#include "printf.h"

void fileio_init()
{
    if (!SD.sdfs.begin(SdioConfig(FIFO_SDIO)))
    {
        debug_print_error("[FILEIO] ERROR: Could not open SD Card\n");
    }
    else
    {
        debug_print_fatfs("[FILEIO] Opened SD card OK! Size: %lld MB\n", SD.totalSize()/1024/1024);
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
    int file_count = 0;
    File root = SD.open("/");

    if (root == false)
    {
        debug_print_error("[FILEIO] ERROR: Could not read SD Card\n");
        return 0;
    }

    while (true)
    {
        File entry = root.openNextFile();
        if (entry == false)
            break;

        if (!entry.isDirectory())
        {
            debug_print_fatfs("Found file: %s\n", entry.name());
            list[file_count] = (char *)malloc(strlen(entry.name()) + 1);
            strcpy(list[file_count], entry.name());
            file_count++;
        }
        entry.close();
    }
    root.close();
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
    FsFile fil = SD.sdfs.open(filename, O_WRITE | O_CREAT);
    if (fil == false)
    {
        debug_print_error("[FILEIO] ERROR: Could not open %s for WRITE\n", filename);
        return;
    }
    if (fil.write(data, len) != len)
    {
        debug_print_error("[FILEIO] ERROR: Could not write %s\n", filename);
    }
    else
    {
        debug_print_status("[FILEIO] Writing %s for %u bytes ok!\n", filename, len);
    }
    fil.close();
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
    FsFile fil = SD.sdfs.open(filename, O_READ);
    if (fil == false)
    {
        debug_print_error("[FILEIO] ERROR: Could not open %s for READ\n", filename);
        return;
    }

    fil.seekSet(file_offset);

    if (fil.read(data, len) != (int)len)
    {
        debug_print_error("[FILEIO] ERROR: Could not read %s\n", filename);
    }
    else
    {
        debug_print_status("[FILEIO] Reading %s for %u bytes ok!\n", filename, len);
    }
    fil.close();
}
