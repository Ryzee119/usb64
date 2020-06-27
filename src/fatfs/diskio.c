/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "qspi.h"
#include "printf.h"
#include <string.h>
#include <Arduino.h>

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

static uint32_t block_size = 0;
static uint32_t flash_size = 0;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	qspi_init(&block_size, &flash_size);
	//printf("disk_initialized block_size %u flash_size %u\r\n", block_size, flash_size);
	//eraseFlashChip();
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (block_size == 0)
		return RES_NOTRDY;

	//printf("disk_read sector %u, count: %u\r\n", sector, count);
	qspi_read(sector * block_size, count * block_size, buff);
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (block_size == 0)
		return RES_NOTRDY;

	qspi_erase(sector * block_size, count * block_size);
	qspi_write(sector * block_size, count * block_size, (uint8_t*)buff);
	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if (block_size == 0)
		return RES_NOTRDY;
	
	switch(cmd) {
		case CTRL_SYNC: return RES_OK;
		case GET_SECTOR_SIZE:  *(WORD*)buff = block_size;   return RES_OK;
		case GET_BLOCK_SIZE:   *(WORD*)buff = block_size;   return RES_OK;
		case GET_SECTOR_COUNT: *(WORD*)buff = flash_size /
		                                      block_size;   return RES_OK;
		default:                                            return RES_OK;
	}
	return RES_OK;
}

