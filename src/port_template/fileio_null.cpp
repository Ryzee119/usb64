// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <stdint.h>

/*
 * Function: Initliased the File access device (SD Card, EMMC, Flash ROM, etc)
 *           This is used to save/load mempaks, usb64 config settings, GB Roms for transfer pak etc.
 * ----------------------------
 *   Returns true on sucess.
 */
bool fileio_dev_init()
{
    return true;
}

/*
 * Function: Opens a directory and returns a handle to it.
 * Used in confunction with  fileio_dev_get_next_filename()and fileio_dev_close_dir();
 * ----------------------------
 *   dir: The directory path to open. "/" should be the root directory.
 *   Returns handle or 0 on error.
 */
int fileio_dev_open_dir(const char *dir)
{
    return 0;
}

/*
 * Function: Returns the next filename of a file in a directory handle from fileio_dev_open_dir.
 * Used in confunction with  fileio_dev_open_dir()and fileio_dev_close_dir();
 * ----------------------------
 *   handle: The directory handle to get the next file from.
 *   Returns filename or NULL on error/no more files.
 */
const char *fileio_dev_get_next_filename(int handle)
{
    return NULL;
}

/*
 * Function: Closes a directory handle opened with fileio_dev_open_dir
 * Used in confunction with  fileio_dev_open_dir() and fileio_dev_get_next_filename();
 * ----------------------------
 *  handle: The directory handle to close obtained with fileio_dev_open_dir
 *  Returns void.
 */
void fileio_dev_close_dir(int handle)
{
}

/*
 * Function: Reads a file from storage.
 * ----------------------------
 *  filename: The name of the file to read.
 *  file_offset: The offset of the file to read from. 0 is the beginning of the file.
 *  data: Return data buffer
 *  len: How many bytes to read from file_offset
 *  Returns -1: Cant open file, -2: Cant read file: 0: Success.
 */
int fileio_dev_read(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len)
{
    return -1;
}

/*
 * Function: Writes a file to storage. This should always create a new file or overwrite if the file exists.
 * ----------------------------
 *  filename: The name of the file to write.
 *  data: The data to write
 *  len: How many bytes to write
 *  Returns -1: Cant open file, -2: Cant read file: 0: Success.
 */
int fileio_dev_write(char *filename, uint8_t *data, uint32_t len)
{
    return -1;
}
