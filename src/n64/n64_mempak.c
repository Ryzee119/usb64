/*
 * n64_mempak.c

 *
 *  Created on: 4Feb.,2019
 *      Author: Ryan
 */

#include <stdint.h>
#include <string.h>
#include "n64_mempak.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_controller.h"
#include "n64_wrapper.h"

void n64_mempack_read32(n64_mempack *mempack, uint16_t address, uint8_t *rx_buff)
{
    if (mempack->virtual_is_active)
    {
        n64_virtualpak_read32(address, rx_buff);
    }
    else
    {
        n64hal_sram_read(rx_buff, mempack->data, address, 32);
    }
}

void n64_mempack_write32(n64_mempack *mempack, uint16_t address, uint8_t *tx_buff)
{
    if (mempack->virtual_is_active)
    {
        n64_virtualpak_write32(address, tx_buff);
    }
    else
    {
        n64hal_sram_write(tx_buff, mempack->data, address, 32);
    }
}