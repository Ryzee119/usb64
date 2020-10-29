// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _FILEIO_H
#define _FILEIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include "usb64_conf.h"

void fileio_init(void);
void fileio_write_to_file(char *filename, uint8_t *data, uint32_t len);
void fileio_read_from_file(char *filename, uint32_t file_offset, uint8_t *data, uint32_t len);
uint32_t fileio_list_directory(char **list, uint32_t max);

int fileio_open_file_readonly(const char *filename);
void fileio_close_file();
int fileio_get_line(char* buffer, int max_len);

#ifdef __cplusplus
}
#endif

#endif
