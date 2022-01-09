// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "common.h"
#include "tinyalloc.h"
#include "memory.h"

static uint32_t extram_start = 0;
static uint32_t extram_end = 0;

bool memory_dev_init()
{
    extram_start = SDRAM_DEVICE_ADDR + 0x200000;
    extram_end = extram_start + SDRAM_DEVICE_SIZE - 0x200000;

    //There's 8MB usable SDRAM on the f750-k board located from 0xC0000000
    //The first 2MB are reversed for the LCD. extram_start starts just after this
    //I create a new heap at this location for allocating large files
    extern SDRAM_HandleTypeDef sdramHandle;
    if (sdramHandle.State != HAL_SDRAM_STATE_READY)
    {
        BSP_SDRAM_Init();
    }

    uint32_t extram_bytes = extram_end - extram_start;
    ta_init((void *)(extram_start), //Base of heap
            (void *)(extram_end),   //End of heap
            extram_bytes / 32768,   //Number of memory chunks (32k/per chunk)
            16,                     //Smaller chunks than this won't split
            4);                     //32 word size alignment

    debug_print_memory("[MEMORY] External memory initialised\n");
    debug_print_memory("[MEMORY] Detected %ukB\n", extram_bytes / 1024);
    debug_print_memory("[MEMORY] Heap start: %08x\n", (void *)(extram_start));
    debug_print_memory("[MEMORY] Heap end: %08x\n", (void *)(extram_end));
    debug_print_memory("[MEMORY] Number of memory chunks: %u\n", extram_bytes / 32768);
    return true;
}

void *memory_dev_malloc(uint32_t len)
{
    void *pt = ta_alloc(len);
    if (pt == NULL)
    {
        debug_print_error("[MEMORY] ERROR, could not allocate memory\n");
    }
    return pt;
}

void memory_dev_free(void *add)
{
    if ((uint32_t)add >= extram_start)
    {
        ta_free(add);
    }
    else
    {
        free(add);
    }
}

/*
 * Function: Detect the amount of external RAM installed. This prints it to the LCD and a number > 0 is required for TPAK
 * emulation. If you device has loads of internal RAM suitable for TPAK emulation (>2MB or so) which you want to use, set this to a non-zero number.
 * ----------------------------
 *   Returns: How many MB of external RAM is installed.
 */
uint8_t memory_get_ext_ram_size()
{
    return (extram_end - extram_start) / 1024 / 1024;
}
