// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "memory.h"
#include "usb64_conf.h"
#include "tinyalloc.h"
#include "fileio.h"
#include "printf.h"

extern uint8_t external_psram_size; //in MB
EXTMEM uint8_t ext_ram[1024 * 1024 * 16];
sram_storage sram[32] = {0};

void memory_init()
{
    //Init external RAM and memory heap
    uint32_t psram_bytes = 1024 * 1024 * external_psram_size;
    ta_init((void *)(ext_ram),               //Base of heap
            (void *)(ext_ram + psram_bytes), //End of heap
            psram_bytes / 32768,             //Number of memory chunks (32k/per chunk)
            16,                              //Smaller chunks than this won't split
            32);                             //32 word size alignment

    debug_print_memory("MEMORY: External memory initialised\n");
    debug_print_memory("MEMORY: Detected %uMB\n", external_psram_size);
    debug_print_memory("MEMORY: Heap start: %08x\n", (void *)(ext_ram));
    debug_print_memory("MEMORY: Heap end: %08x\n", (void *)(ext_ram + psram_bytes));
    debug_print_memory("MEMORY: Number of memory chunks: %u\n", psram_bytes / 32768);
}

//This function allocates and manages SRAM for mempak and gameboy roms (tpak) for the system.
//SRAM is malloced into slots. Each slot stores a pointer to the memory location, its size, and
//a string name to identify what that slot is used for.
uint8_t *memory_alloc_ram(const char *name, uint32_t alloc_len, uint32_t read_only)
{
    if (alloc_len == 0)
    {
        debug_print_memory("MEMORY: WARNING: %s Passed 0 len\n", __FUNCTION__);
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
                debug_print_memory("MEMORY: Memory already malloced for %s, returning pointer to it\n", name);
                return sram[i].data;
            }
                

            debug_print_error("MEMORY: ERROR: SRAM malloced memory isnt right, resetting memory\n");
            //Allocated length isnt long enough. Reset it to be memory safe
            ta_free(sram[i].data);
            sram[i].data = NULL;
            sram[i].len = 0;
        }
    }
    //If nothing exists, loop again to find a spot and allocate
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].len == 0)
        {
            sram[i].data = (uint8_t *)ta_alloc(alloc_len);
            if (sram[i].data == NULL)
                break;
            sram[i].len = alloc_len;
            strcpy(sram[i].name, name);

            fileio_read_from_file(sram[i].name, 0,
                                  sram[i].data,
                                  sram[i].len);

            sram[i].read_only = read_only;
            sram[i].dirty = 0;
            
            debug_print_memory("MEMORY: Alloc'd %s, %u bytes at 0x%08x\n", sram[i].name, sram[i].len, sram[i].data);
            return sram[i].data;
        }
    }
    debug_print_error("ERROR: %s, No SRAM space or slots left. Flush RAM to Flash!\n", __FUNCTION__);
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
            debug_print_memory("MEMORY: Freeing %s at 0x%08x\n", sram[i].name, sram[i].data);
            ta_free(sram[i].data);
            sram[i].name[0] = '\0';
            sram[i].data = NULL;
            sram[i].len = 0;
            sram[i].dirty = 0;
            return;
        }
    }
    debug_print_memory("WARNING: %s: Did not free 0x%08x\n", __FUNCTION__, ptr);
}

//Flush SRAM to flash memory if required
void memory_flush_all()
{
    noInterrupts();
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].len == 0 || sram[i].data == NULL || sram[i].read_only != 0 || sram[i].dirty == 0)
            continue;

        debug_print_status("MEMORY: Writing %s with %u bytes\n", sram[i].name, sram[i].len);
        fileio_write_to_file(sram[i].name, sram[i].data, sram[i].len);  
        sram[i].dirty = 0;
    }
    interrupts();
}

void memory_mark_dirty(void *ptr)
{
    if (ptr == NULL)
        return;

    for (uint32_t i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].data == ptr)
        {
            debug_print_memory("MEMORY: Marking %s as dirty\n", sram[i].name);
            sram[i].dirty = 1;
            return;
        }
    }
    debug_print_error("ERROR: %s: Could not find 0x%08x\n", __FUNCTION__, ptr);
}
