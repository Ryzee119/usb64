// n64_controller.h

#ifndef _N64_CONTROLLER_h
#define _N64_CONTROLLER_h

#ifdef __cplusplus
extern "C" {
#endif

#include "n64_mempak.h"
#include "n64_rumblepak.h"
#include "n64_transferpak_gbcarts.h"

typedef struct
{
    uint16_t dButtons;
    int8_t x_axis;
    int8_t y_axis;
} n64_buttonmap;

typedef struct
{
    int id;                    //Controller ID
    int8_t current_bit;        //The current bit to being received in
    int8_t current_byte;       //The current byte being received in
    uint8_t data_buffer[50];   //Controller main tx and rx buffer
    int8_t current_peripheral; //Peripheral flag, PERI_NONE, PERI_RUMBLE, PERI_MEMPCK, PERI_TPAK
    int8_t next_peripheral;    //What Peripheral to change to next after timer
    n64_buttonmap bState;      //N64 controller button and analog stick map
    uint8_t crc_error;         //Set if the 2 byte address has a CRC error.
    n64_transferpak *tpak;     //Pointer to installed transferpak
    n64_rumblepak *rpak;       //Pointer to installed rumblepak
    n64_mempack *mempack;      //Pointer to installed mempack
    uint8_t isMouse;           //Set to emulate a n64 mouse

    uint32_t bus_idle_timer_us; //Timer counter for bus idle timing
    void *gpio_port;
    uint16_t gpio_pin;

    uint8_t locked;
} n64_controller;

//N64 RELATED
#define N64_IDENTIFY 0x00
#define N64_CONTROLLER_STATUS 0x01
#define N64_PERI_READ 0x02
#define N64_PERI_WRITE 0x03
#define N64_CONTROLLER_RESET 0xFF
#define N64_COMMAND_POS 0
#define N64_ADDRESS_MSB_POS 1
#define N64_ADDRESS_LSB_POS 2
#define N64_DATA_POS 3
#define N64_CRC_POS 35
#define N64_MAX_POS 36
#define PERI_RUMBLE 0
#define PERI_MEMPCK 1
#define PERI_NONE 2
#define PERI_TPAK 3

//N64 Buttons
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

//N64 Controller Flags

#define MEMPAK_SIZE 32768

#define MAX_GBROMS 8
#define MAX_MEMPAKS 4
#define MAX_TPAKS 1
#define VIRTUAL_PAK MAX_MEMPAKS

void n64_init_subsystem(n64_controller *controllers);
void n64_controller_hande_new_edge(n64_controller *cont);

#ifdef __cplusplus
}
#endif

#endif
