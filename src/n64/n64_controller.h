// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _N64_CONTROLLER_h
#define _N64_CONTROLLER_h

#ifdef __cplusplus
extern "C" {
#endif

#include "usb64_conf.h"
#include "n64_mempak.h"
#include "n64_rumblepak.h"
#include "n64_transferpak_gbcarts.h"

typedef struct  __attribute__((packed))
{
    uint16_t dButtons;
    int8_t x_axis;
    int8_t y_axis;
} n64_buttonmap;

typedef struct __attribute__((packed))
{
    uint8_t led_state;
    uint16_t buttons[3];
    uint8_t flags;
} n64_randnet_kb;

typedef enum
{
    N64_CONTROLLER,
    N64_MOUSE,
    N64_RANDNET
} n64_input_type;

typedef enum
{
    PERI_NONE,
    PERI_RUMBLE,
    PERI_MEMPAK,
    PERI_TPAK
} n64_peri_type;

typedef struct
{
    uint32_t id;                      //Controller ID
    int32_t current_bit;              //The current bit to being received in
    uint32_t current_byte;            //The current byte being received in
    uint8_t data_buffer[50];          //Controller main tx and rx buffer
    uint32_t peri_access;             //Peripheral flag is set when a peripheral is being accessed
    uint32_t crc_error;               //Set if the 2 byte address has a CRC error.
    n64_input_type type;              //Store the type of input device. Controller, Mouse. Randnet etc.
    n64_buttonmap b_state;            //N64 controller button and analog stick map
    n64_randnet_kb kb_state;          //Randnet keyboard object
    n64_peri_type current_peripheral; //Peripheral flag, PERI_NONE, PERI_RUMBLE, PERI_MEMPAK, PERI_TPAK
    n64_peri_type next_peripheral;    //What Peripheral to change to next after timer
    n64_transferpak *tpak;            //Pointer to installed transferpak
    n64_rumblepak *rpak;              //Pointer to installed rumblepak
    n64_mempack *mempack;             //Pointer to installed mempack
                                      //
    uint32_t interrupt_attached;      //Flag is set when this controller is connected to an ext int.
    uint32_t bus_idle_timer_clks;     //Timer counter for bus idle timing
    usb64_pin_t  pin;                 //What pin is this controller connected to
} n64_input_dev_t;

//N64 JOYBUS
#define N64_IDENTIFY 0x00
#define N64_CONTROLLER_STATUS 0x01
#define N64_PERI_READ 0x02
#define N64_PERI_WRITE 0x03
#define N64_CONTROLLER_RESET 0xFF
#define N64_RANDNET_REQ 0x13
#define N64_COMMAND_POS 0
#define N64_ADDRESS_MSB_POS 1
#define N64_ADDRESS_LSB_POS 2
#define N64_DATA_POS 3
#define N64_CRC_POS 35
#define N64_MAX_POS 36

//N64 Controller
#define N64_A (1UL << 7)
#define N64_B (1UL << 6)
#define N64_Z (1UL << 5)
#define N64_ST (1UL << 4)
#define N64_DU (1UL << 3)
#define N64_DD (1UL << 2)
#define N64_DL (1UL << 1)
#define N64_DR (1UL << 0)
#define N64_RES (1UL << 15)
#define NOTUSED (1UL << 14)
#define N64_LB (1UL << 13)
#define N64_RB (1UL << 12)
#define N64_CU (1UL << 11)
#define N64_CD (1UL << 10)
#define N64_CL (1UL << 9)
#define N64_CR (1UL << 8)

//RANDNET
#define RANDNET_LED_POS     1
#define RANDNET_BTN_POS     2
#define RANDNET_MAX_BUTTONS 3
#define RANDNET_LED_NUMLOCK            (1 << 0)
#define RANDNET_LED_CAPSLOCK           (1 << 1)
#define RANDNET_LED_POWER              (1 << 2)
#define RANDNET_FLAG_HOME_KEY          (1 << 0)
#define RANDNET_FLAG_EXCESS_BUTTONS    (1 << 4)

//Mempak
#define MEMPAK_SIZE 32768
#define MAX_MEMPAKS MAX_CONTROLLERS
#define VIRTUAL_PAK MAX_MEMPAKS

void n64_subsystem_init(n64_input_dev_t *in_dev);
void n64_controller_hande_new_edge(n64_input_dev_t *cont);

#ifdef __cplusplus
}
#endif

#endif
