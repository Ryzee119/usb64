/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2020 Ryan Wendland (Modified for usb64)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board.h"
#include "qspi.h"
#include "tusb.h"
#include "usb64_conf.h"

#if CFG_TUD_MSC

enum
{
    DISK_BLOCK_NUM = 4096,
    DISK_BLOCK_SIZE = 4096
};

static int cached_block = -1;
static uint8_t block_cache[DISK_BLOCK_SIZE];

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    (void)lun;
    debug_print_tinyusb("TUSB: tud_msc_inquiry_cb\n");
    const char _vid[] = "usb64 By Ryzee119";
    const char _pid[] = "Mass Storage";
    const char _rev[] = "1.0";

    memcpy(vendor_id, _vid, strlen(_vid));
    memcpy(product_id, _pid, strlen(_pid));
    memcpy(product_rev, _rev, strlen(_rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    (void)lun;
    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    (void)lun;
    debug_print_tinyusb("TUSB: Block_count: %u block_size: %u\n", DISK_BLOCK_NUM, DISK_BLOCK_SIZE);
    *block_count = DISK_BLOCK_NUM;
    *block_size  = DISK_BLOCK_SIZE;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    (void)lun;
    (void)power_condition;

    if (load_eject)
    {
        if (start)
        {
            //Don't need to do anything here
        }
        else
        {
            debug_print_tinyusb("TUSB: Unload MSC disk storage\n");
            if (cached_block != -1)
            {
                qspi_erase(cached_block * DISK_BLOCK_SIZE, DISK_BLOCK_SIZE);
                qspi_write(cached_block * DISK_BLOCK_SIZE, DISK_BLOCK_SIZE, block_cache);
            }
            cached_block = -1;
        }
    }

    return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    (void)lun;
    debug_print_tinyusb("TUSB: Read from sector %i with offset %i bytes for %i bytes\n", lba, offset, bufsize);
    //If lba is the cached block, read from the cache instead of flash.
    if (lba == cached_block)
        memcpy(buffer, block_cache + offset, bufsize);
    else
        qspi_read(lba * DISK_BLOCK_SIZE + offset, bufsize, buffer);

    return bufsize;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    (void)lun;
    debug_print_tinyusb("TUSB: Wrote to sector %i with offset %i bytes for %i bytes\n", lba, offset, bufsize);
    //Addressing a new block, we need to flush the old block to flash.
    if (lba != cached_block)
    {
        if (cached_block != -1)
        {
            qspi_erase(cached_block * DISK_BLOCK_SIZE, DISK_BLOCK_SIZE);
            qspi_write(cached_block * DISK_BLOCK_SIZE, DISK_BLOCK_SIZE, block_cache);
        }
        //Read new sector into cache
        qspi_read(lba * DISK_BLOCK_SIZE, DISK_BLOCK_SIZE, block_cache);
        cached_block = lba;
    }
    memcpy(block_cache + offset, buffer, bufsize);
    return bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    // read10 & write10 has their own callback and MUST not be handled here
    void const *response = NULL;
    uint16_t resplen = 0;
    debug_print_tinyusb("TUSB: SCSI Command 0x%02x received\n", scsi_cmd[0]);

    // most scsi handled is input
    bool in_xfer = true;

    switch (scsi_cmd[0])
    {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        // Host is about to read/write etc ... better not to disconnect disk
        resplen = 0;
        break;

    default:
        // Set Sense = Invalid Command Operation
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

        // Negative means error -> tinyusb could stall and/or response with failed status
        resplen = -1;
        break;
    }

    // Return resplen must not larger than bufsize
    if (resplen > bufsize)
        resplen = bufsize;

    if (response && (resplen > 0))
    {
        if (in_xfer)
        {
            memcpy(buffer, response, resplen);
        }
        else
        {
            // SCSI output
        }
    }

    return resplen;
}

#endif