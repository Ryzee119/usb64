// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef N64_N64_WRAPPER_H_
#define N64_N64_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "n64_controller.h"

#define N64_OUTPUT 1
#define N64_INPUT 2
#define N64_INPUT_PULLUP 2
#define N64_INPUT_PULLDOWN 3

#define N64_INTMODE_FALLING 1
#define N64_INTMODE_CHANGE 2
#define N64_INTMODE_RISING 3

//System wrappers
void n64hal_system_init();
void n64hal_debug_init();
void n64hal_debug_write(char c);
void n64hal_attach_interrupt(uint8_t pin, void (*handler)(void), int mode);
void n64hal_detach_interrupt(uint8_t pin);
void n64hal_disable_interrupts();
void n64hal_enable_interrupts();

//RTC wrapper prototypes (For gameboy roms with RTC, i.e Pokemon games)
void n64hal_rtc_read(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s);
void n64hal_rtc_write(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s);
  
//Timer wrappers
uint32_t n64hal_hs_tick_get_speed();
void n64hal_hs_tick_init();
uint32_t n64hal_hs_tick_get();
  
//RAM access wrappers
void n64hal_read_extram(void *rx_buff, void *src, uint32_t offset, uint32_t len);
void n64hal_write_extram(void *tx_buff, void *dst, uint32_t offset, uint32_t len);

//GPIO wrappers
void n64hal_output_set(uint8_t pin, uint8_t level);
void n64hal_input_swap(n64_input_dev_t *controller, uint8_t val);
uint8_t n64hal_input_read(int pin);
void n64hal_pin_set_mode(int pin, uint8_t mode);
  
//FileIO wrappers
uint32_t n64hal_list_gb_roms(char **list, uint32_t max);
void n64hal_read_storage(char *name, uint32_t file_offset, uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif /* N64_N64_WRAPPER_H_ */
