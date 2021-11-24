// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

/* usb64 memory is buffered into RAM to prevent SD card latency causing issues.
 * - Smaller ram blocks are allocated to internal RAM. If internal alloc failes, external RAM is used
 * - Larger RAM blocks, (Like gameboy ROMS) are directly allocted into external RAM.
 * - Memory blocks can be marked as read only, so they will never write back to the SD card
 * - Memory blocks that are read/write and marked dirty when memory is written to them, so tehy wont write back to SD card
 * if they havent been touched.
 * Downside of this is that if you dont flush back to SD card, you may lose save data. usb64 auto senses when the n64 is powered off
 * and automatically flushes for you atleast. You can also manual flush with a button combo.
 */

#include "usb64_conf.h"
#include "memory.h"
#include "n64_wrapper.h"
#include "usb64_conf.h"
#include "fileio.h"

static sram_storage sram[32] = {0};

void memory_init()
{
    if (!memory_dev_init())
    {
        debug_print_memory("[MEMORY] Warning: External memory not initialised\n");
    }
    else
    {
        debug_print_memory("[MEMORY] External memory initialised\n");
    }
}

//This function allocates and manages SRAM for mempak and gameboy roms (tpak) for the system.
//SRAM is malloced into slots. Each slot stores a pointer to the memory location, its size, and
//a string name to identify what that slot is used for.
uint8_t *memory_alloc_ram(const char *name, uint32_t alloc_len, uint32_t read_only)
{
    if (alloc_len == 0)
    {
        debug_print_memory("[MEMORY] WARNING: Passed 0 len\n");
        return NULL;
    }

    //Loop through to see if alloced memory already exists
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (strcmp(sram[i].name, name) == 0)
        {
            //Already malloced, check len is ok
            if (sram[i].len <= alloc_len)
            {
                debug_print_memory("[MEMORY] Memory already malloced for %s at 0x%08x, returning pointer to it\n", name, sram[i].data);
                return sram[i].data;
            }

            debug_print_error("[MEMORY] ERROR: SRAM malloced memory isnt right, resetting memory\n");
            //Allocated length isnt long enough. Reset it to be memory safe
            memory_dev_free(sram[i].data);
            sram[i].data = NULL;
            sram[i].len = 0;
        }
    }
    //If nothing exists, loop again to find a spot and allocate
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].len == 0)
        {
            sram[i].data = (uint8_t *)memory_dev_malloc(alloc_len);

            //If failed, no RAM left?
            if (sram[i].data == NULL)
                break;

            sram[i].len = alloc_len;
            strcpy(sram[i].name, name);

            fileio_read_from_file(sram[i].name, 0,
                                  sram[i].data,
                                  sram[i].len);

            sram[i].read_only = read_only;
            sram[i].dirty = 0;
            
            debug_print_memory("[MEMORY] Alloc'd %s, %u bytes at 0x%08x\n", sram[i].name, sram[i].len, sram[i].data);
            return sram[i].data;
        }
    }
    debug_print_error("[MEMORY] ERROR: No SRAM space or slots left. Flush RAM to Flash!\n");
    return NULL;
}

void memory_free_item(void *ptr)
{
    if (ptr == NULL)
        return; //Already free'd

    //Loop through to see if alloced memory already exists
    for (uint32_t i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].data == ptr)
        {
            debug_print_memory("[MEMORY] Freeing %s at 0x%08x\n", sram[i].name, sram[i].data);
            memory_dev_free(sram[i].data);
            sram[i].name[0] = '\0';
            sram[i].data = NULL;
            sram[i].len = 0;
            sram[i].dirty = 0;
            return;
        }
    }
    debug_print_memory("[MEMORY] WARNING: Did not free 0x%08x\n", ptr);
}

//Flush SRAM to flash memory if required
void memory_flush_all()
{
    n64hal_disable_interrupts();
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].len == 0 || sram[i].data == NULL || sram[i].read_only != 0 || sram[i].dirty == 0)
            continue;

        debug_print_status("[MEMORY] Writing %s with %u bytes\n", sram[i].name, sram[i].len);
        fileio_write_to_file(sram[i].name, sram[i].data, sram[i].len);  
        sram[i].dirty = 0;
    }
    n64hal_enable_interrupts();
}

void memory_mark_dirty(void *ptr)
{
    if (ptr == NULL)
        return;

    for (uint32_t i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].data == ptr)
        {
            debug_print_memory("[MEMORY] Marking %s as dirty\n", sram[i].name);
            sram[i].dirty = 1;
            return;
        }
    }
    debug_print_error("[MEMORY] ERROR: Could not find 0x%08x\n", ptr);
}
