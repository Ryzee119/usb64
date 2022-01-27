// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "n64_controller.h"

void n64_cpak_read32(n64_controllerpak *cpak, uint16_t address, uint8_t *rx_buff)
{
    if (cpak->virtual_is_active)
    {
        n64_virtualpak_read32(address, rx_buff);
    }
    else
    {
        n64hal_read_extram(rx_buff, cpak->data, address, 32);
    }
}

void n64_cpak_write32(n64_controllerpak *cpak, uint16_t address, uint8_t *tx_buff)
{
    if (cpak->virtual_is_active)
    {
        n64_virtualpak_write32(address, tx_buff);
    }
    else
    {
        n64hal_write_extram(tx_buff, cpak->data, address, 32);
    }
}
