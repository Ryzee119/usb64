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

#ifndef N64_N64_WRAPPER_H_
#define N64_N64_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include "n64_controller.h"

#define N64_OUTPUT 1
#define N64_INPUT 2

uint8_t n64hal_rom_fastread(gameboycart *gb_cart, uint32_t address, uint8_t *data, uint32_t len);
void n64hal_sram_backup_to_file(uint8_t *filename, uint8_t *data, uint32_t len);
void n64hal_sram_restore_from_file(uint8_t *filename, uint8_t *data, uint32_t len);
void n64hal_sram_read(uint8_t *rxdata, uint8_t *src, uint16_t address, uint16_t len);
void n64hal_sram_write(uint8_t *txdata, uint8_t *dest, uint16_t address, uint16_t len);
void n64hal_rtc_read(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst);
void n64hal_rtc_write(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst);
uint8_t n64hal_scan_for_gbroms(char** array, int max);

uint32_t n64hal_hs_tick_get_speed();
void n64hal_hs_tick_init();
void n64hal_hs_tick_reset();
uint32_t n64hal_hs_tick_get();

void n64hal_input_swap(n64_controller *controller, uint8_t val);
uint8_t n64hal_input_read(n64_controller *controller);

#ifdef __cplusplus
}
#endif
#endif /* N64_N64_WRAPPER_H_ */
