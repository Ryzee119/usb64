// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _MEMORY_H
#define _MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include "usb64_conf.h"

typedef struct
{
    char name[MAX_FILENAME_LEN];
    uint8_t *data;
    uint32_t len;
    uint32_t read_only; //If read only, it will never write back to storage
    uint32_t dirty;
} sram_storage;

void memory_init();
uint8_t *memory_alloc_ram(const char *name, uint32_t alloc_len, uint32_t read_only);
void memory_flush_all(void);
void memory_free_item(void *ptr);
void memory_mark_dirty(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
