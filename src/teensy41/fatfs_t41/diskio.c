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

#include <stdio.h>
#include "ff.h"
#include "diskio.h"
#include "usb64_conf.h"
#include "diskio_wrapper.h"

//Read sector first then compare to see if sector has actually changed
//Can reduce flash wear and actually speed up larger writes as only small parts
//of the file might change.
#define READBEFOREWRITE

static uint32_t sector_size = 0;

DSTATUS disk_status(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    return RES_OK;
}

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive number to identify the drive */
)
{
    debug_print_fatfs("FATFS: Initialise disk\n");
    _disk_init();
    sector_size = _disk_volume_get_block_size();
    debug_print_fatfs("FATFS: Sector size %u\n", sector_size);
    debug_print_fatfs("FATFS: Num sectors %u\n", _disk_volume_num_blocks());
    debug_print_fatfs("FATFS: Sectors per cluster %u\n", _disk_volume_get_cluster_size());
    return RES_OK;
}

DRESULT disk_read(
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    LBA_t sector, /* Start sector in LBA */
    UINT count    /* Number of sectors to read */
)
{
    if (sector_size == 0)
        return RES_NOTRDY;

    debug_print_fatfs("FATFS: Read from sector %i for %i sector(s)\n", sector, count);
    _read_sector(buff, sector, count);
    return RES_OK;
}

#if FF_FS_READONLY == 0
DRESULT disk_write(
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    const BYTE *buff, /* Data to be written */
    LBA_t sector,     /* Start sector in LBA */
    UINT count        /* Number of sectors to write */
)
{
    if (sector_size == 0)
        return RES_NOTRDY;

    _write_sector(buff, sector, count);
    debug_print_fatfs("FATFS: Wrote to sector %i for %i sector(s)\n", sector, count);
    return RES_OK;
}
#endif

DRESULT disk_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0..) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive control data */
)
{
    if (sector_size == 0)
        return RES_NOTRDY;

    switch (cmd)
    {
    case GET_SECTOR_SIZE:
        debug_print_fatfs("FATFS: disk_ioctl: Sector size %i\n", _disk_volume_get_block_size());
        *(WORD *)buff = _disk_volume_get_block_size();
        ;
        break;
    case GET_BLOCK_SIZE:
        debug_print_fatfs("FATFS: disk_ioctl: Block size %i\n", _disk_volume_get_cluster_size());
        *(WORD *)buff = _disk_volume_get_cluster_size();
        break;
    case GET_SECTOR_COUNT:
        debug_print_fatfs("FATFS: disk_ioctl: Sector count %i\n", _disk_volume_num_blocks());
        *(WORD *)buff = _disk_volume_num_blocks();
        break;
    default:
        break;
    }
    return RES_OK;
}
