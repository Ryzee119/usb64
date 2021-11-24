// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _FILEIO_H
#define _FILEIO_H

#include "usb64_conf.h"

//File access API
void fileio_init(void);
void fileio_write_to_file(char *filename, uint8_t *data, uint32_t len);
void fileio_read_from_file(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len);
uint32_t fileio_list_directory(char **list, uint32_t max);
bool fileio_detected();

//Device specific API. Implement for porting.
bool fileio_dev_init();
int fileio_dev_open_dir(const char* dir);
void fileio_dev_close_dir(int handle);
const char* fileio_dev_get_next_filename(int handle);
int fileio_dev_read(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len);
int fileio_dev_write(char *filename, uint8_t *data, uint32_t len);

#endif
