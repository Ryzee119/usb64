// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _N64_MEMPAK_h
#define _N64_MEMPAK_h

#ifdef __cplusplus
extern "C" {
#endif

#define MEMPAK_SIZE 32768

typedef struct
{
    uint32_t id;
    uint8_t *data;
    //Only if a 'virtual mempak'
    uint32_t virtual_is_active;
    uint32_t virtual_update_req;
    uint32_t virtual_selected_row;
} n64_mempack;

void n64_mempack_read32(n64_mempack *mempack, uint16_t address, uint8_t *rx_buff);
void n64_mempack_write32(n64_mempack *mempack, uint16_t address, uint8_t *tx_buff);

#ifdef __cplusplus
}
#endif

#endif
