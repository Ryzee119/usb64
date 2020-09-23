// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef N64_N64_WRAPPER_H_
#define N64_N64_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include "n64_controller.h"

#define N64_OUTPUT 1
#define N64_INPUT 2

//RTC wrapper prototypes (For gameboy roms with RTC, i.e Pokemon games)
void n64hal_rtc_read(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s);
void n64hal_rtc_write(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s);
  
//Timer wrappers
uint32_t n64hal_hs_tick_get_speed();
void n64hal_hs_tick_init();
void n64hal_hs_tick_reset();
uint32_t n64hal_hs_tick_get();
  
//RAM access wrappers
void n64hal_buffered_read(void *rx_buff, void *src, uint32_t offset, uint32_t len);
void n64hal_buffered_write(void *tx_buff, void *dst, uint32_t offset, uint32_t len);

//GPIO wrappers
void n64hal_input_swap(n64_controller *controller, uint8_t val);
uint8_t n64hal_input_read(n64_controller *controller);
  
//FileIO wrappers
uint32_t n64hal_list_gb_roms(char **list, uint32_t max);
void n64hal_unbuffered_read(char *name, uint32_t file_offset, uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif /* N64_N64_WRAPPER_H_ */
