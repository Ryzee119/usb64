/*-----------------------------------------------------------------------/
/  Low level disk interface modlue include file   (C)ChaN, 2019          /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

/* Status of Disk Functions */
typedef BYTE DSTATUS;

/* Results of Disk Functions */
typedef enum
{
	RES_OK = 0, /* 0: Successful */
	RES_ERROR,  /* 1: R/W Error */
	RES_WRPRT,  /* 2: Write Protected */
	RES_NOTRDY, /* 3: Not Ready */
	RES_PARERR  /* 4: Invalid Parameter */
} DRESULT;

/*---------------------------------------*/
/* Prototypes for disk control functions */
DSTATUS disk_initialize(BYTE pdrv);
DSTATUS disk_status(BYTE pdrv);
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count);
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count);
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT 0x01	 /* Drive not initialized */
#define STA_NODISK 0x02	 /* No medium in the drive */
#define STA_PROTECT 0x04 /* Write protected */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC 0		   /* Complete pending write process (needed at FF_FS_READONLY == 0) */
#define GET_SECTOR_COUNT 1 /* Get media size (needed at FF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE 2  /* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
#define GET_BLOCK_SIZE 3   /* Get erase block size (needed at FF_USE_MKFS == 1) */
#define CTRL_TRIM 4		   /* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

#ifdef __cplusplus
}
#endif

#endif
