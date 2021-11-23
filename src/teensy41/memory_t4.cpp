// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "memory.h"
#include "printf.h"

static uint32_t internal_size = 32768; //Smaller than this will malloc to internal RAM instead
extern uint8_t external_psram_size; //in MB. Set in startup.c
EXTMEM uint8_t ext_ram[1]; //Just to get the start of EXTMEM

bool memory_dev_init()
{
    if (external_psram_size == 0)
        return false;

    debug_print_memory("[MEMORY] Detected %uMB\n", external_psram_size);
    debug_print_memory("[MEMORY] Heap start: %08x\n", (uint32_t)ext_ram);
    debug_print_memory("[MEMORY] Heap end: %08x\n", (uint32_t)ext_ram + external_psram_size * 1024 * 1024);
    return true;
}

void *memory_dev_malloc(uint32_t len)
{
    void *add = NULL;

    //Try internal RAM for smaller files for best performance
    if (len <= internal_size || external_psram_size == 0)
    {
        add = malloc(len);
    }

    //If failed to malloc to internal RAM, try external RAM
    if (add == NULL)
    {
        add = (uint8_t *)extmem_malloc(len);
    }
    
    return add;
}

void memory_dev_free(void *add)
{
    if (add == NULL)
    {
        return;
    }
    //If the address range is in the external RAM memory map, call the correct free function.
    if (add >= ext_ram)
    {
        extmem_free(add);
    }
    else
    {
        free(add);
    }
}

uint8_t memory_get_ext_ram_size()
{
    return external_psram_size;
}
