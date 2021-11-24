// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "usb64_conf.h"
#include "fileio.h"
#include "memory.h"

static bool fileio_ok = false;

void fileio_init()
{
    if (!fileio_dev_init())
    {
        debug_print_error("[FILEIO] ERROR: Could not open SD Card\n");
    }
    else
    {
        debug_print_fatfs("[FILEIO] Opened SD card OK!\n");
        fileio_ok = true;
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
    int root = fileio_dev_open_dir("/");

    if (root == 0)
    {
        debug_print_error("[FILEIO] ERROR: Could not read SD Card\n");
        return 0;
    }

    debug_print_fatfs("[FILEIO] fileio_list_directory %08x!\n", root);

    while (true)
    {
        const char* filename = fileio_dev_get_next_filename(root);

        if (filename == NULL)
        {
            break;
        }

        debug_print_fatfs("Found file: %s\n", filename);
        list[file_count] = (char *)memory_dev_malloc(strlen(filename) + 1);
        strcpy(list[file_count], filename);
        file_count++;
    }
    fileio_dev_close_dir(root);
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
    int ret = fileio_dev_write(filename, data, len);
    if (ret == -1)
    {
        debug_print_error("[FILEIO] ERROR: Could not open %s for WRITE\n", filename);
    }
    else if (ret == -2)
    {
        debug_print_error("[FILEIO] ERROR: Could not write %s\n", filename);
    }
    else if (ret == -3)
    {
        debug_print_status("[FILEIO] Writing %s for %lu bytes ok!\n", filename, len);
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
    int ret = fileio_dev_read(filename, file_offset, data, len);

    if (ret == -1)
    {
        debug_print_error("[FILEIO] ERROR: Could not open %s for READ\n", filename);
    }
    else if (ret == -2)
    {
        debug_print_error("[FILEIO] ERROR: Could not read %s\n", filename);
    }
    else if (ret == -3)
    {
        debug_print_status("[FILEIO] Reading %s for %lu bytes ok!\n", filename, len);
    }
}

bool fileio_detected()
{
    return fileio_ok;
}