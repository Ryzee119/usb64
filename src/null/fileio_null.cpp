// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <stdint.h>

bool fileio_dev_init()
{
    return true;
}

int fileio_dev_open_dir(const char *dir)
{
    return 0;
}

const char *fileio_dev_get_next_filename(int handle)
{
    return NULL;
}

void fileio_dev_close_dir(int handle)
{
}

//-1 Cant open
//-2 Cant read
//0 OK
int fileio_dev_read(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len)
{
    return -1;
}

//-1 Cant open
//-2 Cant read
//0 OK
int fileio_dev_write(char *filename, uint8_t *data, uint32_t len)
{
    return -1;
}
