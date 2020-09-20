/* MIT License
 * 
 * Copyright (c) [2020] [Ryan Wendland]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _MEMORY_H
#define _MEMORY_H

#include <Arduino.h>
#include "usb64_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

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
