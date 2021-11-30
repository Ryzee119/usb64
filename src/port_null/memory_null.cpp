// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include "memory.h"

bool memory_dev_init()
{
    return false;
}

void *memory_dev_malloc(uint32_t len)
{
    return NULL;
}

void memory_dev_free(void *add)
{
}

uint8_t memory_get_ext_ram_size()
{
    return 0;
}
