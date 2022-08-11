// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include "memory.h"

/*
 * Function: Initalise memory for device (External RAM etc)
 * ----------------------------
 *   Returns: true on success, false on error
 */
bool memory_dev_init()
{
    return false;
}

/*
 * Function: Allocate a block of memory. This could be large block if using TPAK emulation.
 * This function should use internal or external RAM as required. Freed with memory_dev_free()
 * ----------------------------
 *   Returns: A pointer to the alloced memory, or NULL on error.
 *
 *   len: How many bytes to allocate.
 */
void *memory_dev_malloc(uint32_t len)
{
    return NULL;
}

/*
 * Function: Free an allocated memory block allocated with memory_dev_malloc();
 * ----------------------------
 *   Returns: Void
 *
 *   add: Pointer to the address block to free
 */
void memory_dev_free(void *add)
{
}

/*
 * Function: Detect the amount of external RAM installed. This prints it to the LCD and a number > 0 is required for TPAK
 * emulation. If you device has loads of internal RAM suitable for TPAK emulation (>2MB or so) which you want to use, set this to a non-zero number.
 * ----------------------------
 *   Returns: How many MB of external RAM is installed.
 */
uint8_t memory_get_ext_ram_size()
{
    return 0;
}
