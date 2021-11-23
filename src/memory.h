// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _MEMORY_H
#define _MEMORY_H

#include "usb64_conf.h"

#define MEMORY_READ_WRITE 0
#define MEMORY_READ_ONLY 1

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
uint8_t memory_get_ext_ram_size();

bool memory_dev_init();
void *memory_dev_malloc(uint32_t len);
void memory_dev_free(void *add);

#endif
