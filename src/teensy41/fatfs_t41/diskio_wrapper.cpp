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

#include <SD.h>
#include <SPI.h>
#include "diskio_wrapper.h"

Sd2Card card;
SdVolume volume;

void _disk_init(void)
{
    SD.begin(BUILTIN_SDCARD);
    card.init(SPI_HALF_SPEED, BUILTIN_SDCARD);
    volume.init(card);
}

uint32_t _disk_volume_num_blocks()
{
    return volume.blocksPerCluster() * volume.clusterCount();
}

uint32_t _disk_volume_get_cluster_size()
{
    return volume.blocksPerCluster();
}

uint32_t _disk_volume_get_block_size()
{
    return 512;
}

void _read_sector(uint8_t *buff, uint32_t sector, uint32_t cnt)
{
    while (cnt > 0)
    {
        card.readBlock(sector, buff);
        buff += _disk_volume_get_block_size();
        cnt -= 1;
    }
}

void _write_sector(const uint8_t *buff, uint32_t sector, uint32_t cnt)
{
    while (cnt > 0)
    {
        card.writeBlock(sector, buff);
        buff += _disk_volume_get_block_size();
        cnt -= 1;
    }
}