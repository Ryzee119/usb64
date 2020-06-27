/*
 * n64_wrapper.h
 *
 *  Created on: 21Jan.,2020
 *      Author: Ryan
 */

#ifndef N64_N64_WRAPPER_H_
#define N64_N64_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include "n64_controller.h"

#define OUTPUT_PP OUTPUT
#define INPUT_PUP INPUT_PULLUP

    void n64hal_flash_read_rom(uint8_t *filename, uint32_t address, uint8_t *data, uint32_t len);
    void n64hal_backup_sram_to_flash(uint8_t *filename, uint8_t *data, uint32_t len);
    void n64hal_read_sram_from_flash(uint8_t *filename, uint8_t *data, uint32_t len);
    void n64hal_sram_read(uint8_t *rxdata, uint8_t *src, uint16_t address, uint16_t len);
    void n64hal_sram_write(uint8_t *txdata, uint8_t *dest, uint16_t address, uint16_t len);
    void n64hal_read_rtc(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst);
    void n64hal_write_rtc(uint16_t *day, uint8_t *h, uint8_t *m, uint8_t *s, uint32_t *dst);
    void n64hal_scan_flash_gbroms(gameboycart *gb_cart);
    //Different clocks used for N64

    //Millisecond counter, counts millisecond ticks since power up.
    uint32_t n64hal_milli_tick_get();

//Microsecond counter used for precise timing within N64 library
#define n64hal_hs_rate n64hal_hs_tick_init
    void n64hal_micro_tick_init();
    void n64hal_micro_tick_reset();
    uint32_t n64hal_micro_tick_get();

    //High speed counter used for bit bang timing precision of N64 protocol output
    //This clock needs to be count at >20Mhz as a minimum probably.
    uint32_t n64hal_hs_tick_init();
    void n64hal_hs_tick_reset();
    uint32_t n64hal_hs_tick_get();

    void n64hal_input_swap(n64_controller *controller, uint8_t val);
    uint8_t n64hal_input_read(n64_controller *controller);

#ifdef __cplusplus
}
#endif
#endif /* N64_N64_WRAPPER_H_ */
