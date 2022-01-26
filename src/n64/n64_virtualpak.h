// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef N64_VIRTUALPAK_H_
#define N64_VIRTUALPAK_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MENU_LINE1 0
#define MENU_LINE2 1
#define MENU_LINE3 2
#define MENU_LINE4 3
#define MENU_LINE5 4
#define MENU_LINE6 5
#define MENU_LINE7 6
#define MENU_LINE8 7
#define MENU_LINE9 8
#define MENU_LINE10 9
#define MENU_LINE11 10
#define MENU_LINE12 11
#define MENU_LINE13 12
#define MENU_LINE14 13
#define MENU_LINE15 14
#define MENU_LINE16 15

#define MENU_NAME_FIELD 0
#define MENU_EXT_FIELD 1

void n64_virtualpak_init(n64_controllerpak *vpak);
void n64_virtualpak_update(n64_controllerpak *vpak);
void n64_virtualpak_read32(uint16_t address, uint8_t *rx_buff);
void n64_virtualpak_write32(uint16_t address, uint8_t *tx_buff);
void n64_virtualpak_write_info_1(char* msg);
void n64_virtualpak_write_info_2(char* msg);
uint8_t n64_virtualpak_get_controller_page(void);

#ifdef __cplusplus
}
#endif

#endif /* N64_VIRTUALPAK_H_ */
