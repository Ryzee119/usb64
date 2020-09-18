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
        memcpy(rx_buff, mempack->data + address, 32);
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
