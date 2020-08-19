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

#include "ff.h"
#include "diskio.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "qspi.h"
#include "usb64_conf.h"

static uint32_t sector_size = 0;
static uint32_t flash_size = 0;

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
	qspi_init(&sector_size, &flash_size);
	return RES_OK;
}

DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	if (sector_size == 0)
		return RES_NOTRDY;

	debug_print_fatfs("FATFS: Read from sector %i for %i sector(s)\n", sector, count);
	qspi_read(sector * sector_size, count * sector_size, buff);
	return RES_OK;
}

#if FF_FS_READONLY == 0
DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	if (sector_size == 0)
		return RES_NOTRDY;

	//I read the sector first and see if it has been changed before writing.
	//Because writing is quite slow, this should actually speed things up a bit on average.
	uint32_t blen = count * sector_size;
	uint32_t bAddress = sector * sector_size;

	debug_print_fatfs("FATFS: Wrote to sector %i for %i sector(s)\n", sector, count);
	qspi_erase(bAddress, blen);
	qspi_write(bAddress, blen, (uint8_t *)buff);

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
		debug_print_fatfs("FATFS: disk_ioctl: Sector size %i\n", sector_size);
		*(WORD *)buff = sector_size;
		break;
	case GET_BLOCK_SIZE:
		debug_print_fatfs("FATFS: disk_ioctl: Block size %i\n", 1);
		*(WORD *)buff = 1;
		break;
	case GET_SECTOR_COUNT:
		debug_print_fatfs("FATFS: disk_ioctl: Sector count %i\n", flash_size / sector_size);
		*(WORD *)buff = flash_size /
						sector_size;
		break;
	default:
		break;
	}
	return RES_OK;
}
