// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef N64_N64_WRAPPER_H_
#define N64_N64_WRAPPER_H_

#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "printf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define N64_OUTPUT 1
#define N64_INPUT 2
#define N64_INPUT_PULLUP 2
#define N64_INPUT_PULLDOWN 3

#define N64_INTMODE_FALLING 1
#define N64_INTMODE_CHANGE 2
#define N64_INTMODE_RISING 3

/* DEBUG PRINTERS - See platformio.ini to enable */
#define debug_print_status(fmt, ...)     do { if (DEBUG_STATUS)  usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_n64(fmt, ...)        do { if (DEBUG_N64)     usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_tpak(fmt, ...)       do { if (DEBUG_TPAK)    usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_usbhost(fmt, ...)    do { if (DEBUG_USBHOST) usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_fatfs(fmt, ...)      do { if (DEBUG_FATFS)   usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_memory(fmt, ...)     do { if (DEBUG_MEMORY)  usb64_printf(fmt, ##__VA_ARGS__); } while (0)
#define debug_print_error(fmt, ...)      do { if (DEBUG_ERROR)   usb64_printf(fmt, ##__VA_ARGS__); } while (0)

/* N64 LIB */
#define MAX_CONTROLLERS 4             //Max is 4
#define MAX_MICE 4                    //0 to disable N64 mouse support. Must be <= MAX_CONTROLLERS
#define MAX_KB 4                      //0 to disable N64 randnet keyboard support. Must be <= MAX_CONTROLLERS
#define MAX_GBROMS 10                 //ROMS over this will just get ignored
#define ENABLE_I2C_CONTROLLERS 0      //Received button presses over I2C, useful for integrating with a rasp pi etc.
#define ENABLE_HARDWIRED_CONTROLLER 1 //Ability to hardware a N64 controller into the usb64.
#define PERI_CHANGE_TIME 750          //Milliseconds to simulate a peripheral changing time. Needed for some games.

/* FILESYSTEM */
#define MAX_FILENAME_LEN 256
#define SETTINGS_FILENAME "SETTINGS.DAT"
#define GAMEBOY_SAVE_EXT ".SAV" //ROMFILENAME.SAV
#define CPAK_SAVE_EXT ".MPK" //CONTROLLER_PAK_XX.MPK

/* FIRMWARE DEFAULTS (CONFIGURABLE DURING USE) */
#define DEFAULT_SENSITIVITY 2  //0 to 4 (0 = low sensitivity, 4 = max)
#define DEFAULT_DEADZONE 2     //0 to 4 (0 = no deadzone correction, 4 = max (40%))
#define DEFAULT_SNAP 1         //0 or 1 (0 = will output raw analog stick angle, 1 will snap to 45deg angles)
#define DEFAULT_OCTA_CORRECT 1 //0 or 1 (Will correct the circular analog stuck shape to N64 octagonal)

/* FIRMWARE DEFAULTS (NOT CONFIGURABLE DURING USE) */
#define SNAP_RANGE 5           //+/- what angle range will snap. 5 will snap to 45 degree if between 40 and 50 degrees.
#define MOUSE_SENSITIVITY 2.0f //Just what felt right to me with my mouse.
#define MAG_AT_45DEG 1.1f      //Octagonal shape has a larger magnitude at the 45degree points. 1.1 times larger seems about right

#include "port_conf.h"

//System wrappers
void n64hal_system_init();
void n64hal_debug_init();
void n64hal_debug_write(char c);
void n64hal_attach_interrupt(usb64_pin_t pin, void (*handler)(void), int mode);
void n64hal_detach_interrupt(usb64_pin_t pin);
void n64hal_disable_interrupts();
void n64hal_enable_interrupts();

//RTC wrapper prototypes (For gameboy roms with RTC, i.e Pokemon games)
void n64hal_rtc_read(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s);
void n64hal_rtc_write(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s);
  
//Timer wrappers
uint32_t n64hal_hs_tick_get_speed();
void n64hal_hs_tick_init();
uint32_t n64hal_hs_tick_get();
uint32_t n64hal_millis();

//RAM access wrappers
void n64hal_read_extram(void *rx_buff, void *src, uint32_t offset, uint32_t len);
void n64hal_write_extram(void *tx_buff, void *dst, uint32_t offset, uint32_t len);

//GPIO wrappers
void n64hal_gpio_init();
void n64hal_output_set(usb64_pin_t pin, uint8_t level);
void n64hal_input_swap(usb64_pin_t pin, uint8_t val);
uint8_t n64hal_input_read(usb64_pin_t pin);
  
//FileIO wrappers
uint32_t n64hal_list_gb_roms(char **list, uint32_t max);
void n64hal_read_storage(char *name, uint32_t file_offset, uint8_t *data, uint32_t len);

//Memory wrappers
void *n64hal_malloc(uint32_t len);
void n64hal_free(void *addr);

#ifdef __cplusplus
}
#endif
#endif /* N64_N64_WRAPPER_H_ */
