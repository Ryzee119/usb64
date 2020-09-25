// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef DISKIO_WRAPPER_H_
#define DISKIO_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>

void _disk_init(void);
uint32_t _disk_volume_num_blocks();
uint32_t _disk_volume_get_block_size();
uint32_t _disk_volume_get_cluster_size();
void _read_sector(uint8_t* buff, uint32_t sector, uint32_t cnt);
void _write_sector(const uint8_t* buff, uint32_t sector, uint32_t cnt);

#ifdef __cplusplus
}
#endif
#endif /* DISKIO_WRAPPER_H_ */