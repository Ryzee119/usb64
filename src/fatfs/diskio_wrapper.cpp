// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

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
