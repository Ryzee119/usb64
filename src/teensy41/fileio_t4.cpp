// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include <SD.h>

bool fileio_dev_init()
{
    return SD.sdfs.begin(SdioConfig(FIFO_SDIO));
}

int fileio_dev_open_dir(const char* dir)
{
    File *handle = new File();
    *handle = SD.open("/");
    if (*handle == false)
    {
        return 0;
    }
    else
    {
        return (int)handle;
    }
}

const char *fileio_dev_get_next_filename(int handle)
{
    File *_handle = (File *)handle;
    const char *filename = NULL;
    File entry = _handle->openNextFile();

    if (entry == false)
    {
        return NULL;
    }
    filename = entry.name();
    entry.close();
    return filename;
}

void fileio_dev_close_dir(int handle)
{
    File *_handle = (File *)handle;
    _handle->close();
    delete _handle;
}

int fileio_dev_read(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len)
{
    FsFile fil = SD.sdfs.open(filename, O_READ);
    if (fil == false)
    {
        return -1;
    }

    fil.seekSet(file_offset);

    if (fil.read(data, len) != (int)len)
    {
        return -2;
    }
    else
    {
        return -3;
    }
    fil.close();
}

int fileio_dev_write(char *filename, uint8_t *data, uint32_t len)
{
    FsFile fil = SD.sdfs.open(filename, O_WRITE | O_CREAT);
    if (fil == false)
    {
        return -1;
    }
    if (fil.write(data, len) != len)
    {
        return -2;
    }
    else
    {
        return -3;
    }
    fil.close();
}
