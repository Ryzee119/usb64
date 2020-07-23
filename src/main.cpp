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

#include <Arduino.h>
#include <string.h>
#include "USBHost_t36.h"
#include "ff.h"
#include "qspi.h"
#include "printf.h"
#include "n64_wrapper.h"
#include "n64_conf.h"
#include "n64_controller.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "tusb.h"

typedef struct
{
    char name[256];
    uint8_t *data;
    int len;
    int non_volatile;
} sram_storage;

#define serial_port Serial1
n64_controller n64_c[MAX_CONTROLLERS];

//USB Host Interface
USBHost usbh;
JoystickController joy1(usbh);
JoystickController joy2(usbh);
JoystickController joy3(usbh);
JoystickController joy4(usbh);
JoystickController *gamecontroller[] = {&joy1, &joy2, &joy3, &joy4};

sram_storage sram[10] = {0};
static uint8_t *alloc_sram(const char *name, int alloc_len)
{
    if (alloc_len == 0)
        return NULL;

    //Loop through to see if alloced memory already exists
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (strcmp(sram[i].name, (const char *)name) == 0)
        {
            printf("SRAM already malloced at slot %u\n", i);
            //Already malloced, check len is ok
            if (sram[i].len <= alloc_len)
                return sram[i].data;

            printf("ERROR: SRAM malloced memory isnt right, resetting memory\n");
            //Allocated length isnt long enough. Reset it to be memory safe
            free(sram[i].data);
            sram[i].data = NULL;
            sram[i].len = 0;
        }
    }
    //If nothing exists, loop again to find a spot and allocate
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].len == 0)
        {
            printf("SRAM slot %u is free, allocating %u bytes\n", i, alloc_len);
            sram[i].data = (uint8_t *)malloc(alloc_len);
            sram[i].len = alloc_len;
            strcpy(sram[i].name, name);

            n64hal_sram_restore_from_file((uint8_t *)sram[i].name,
                                          sram[i].data,
                                          sram[i].len);
            return sram[i].data;
        }
    }
    //Not allocated, or no spots left. You need to flush RAM to Flash
    printf("No SRAM allocs left. Flush RAM to Flash!\n");
    return NULL;
}

static void flush_sram()
{
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if(sram[i].len == 0)
            continue;

        if(sram[i].data == NULL)
            continue;

        n64hal_sram_backup_to_file((uint8_t*)sram[i].name, sram[i].data, sram[i].len);
    }
}

static void tusbd_interrupt(void)
{
    tud_int_handler(0);
}

static uint8_t ring_buffer_pos = 0;
static char ring_buffer[4096];
void _putchar(char character)
{
    //ring_buffer[ring_buffer_pos++] = character;
    //if (ring_buffer_pos >= sizeof(ring_buffer))
    //    ring_buffer_pos = 0;
    serial_port.write(character);
}

void n64_controller1_clock_edge()
{
    n64_controller_hande_new_edge(&n64_c[0]);
}
void n64_controller2_clock_edge()
{
    n64_controller_hande_new_edge(&n64_c[1]);
}
void n64_controller3_clock_edge()
{
    n64_controller_hande_new_edge(&n64_c[2]);
}
void n64_controller4_clock_edge()
{
    n64_controller_hande_new_edge(&n64_c[3]);
}

void setup()
{
    serial_port.begin(9600);
    //Check that the flash chip is formatted for FAT access
    //If it's not, format it! Should only happen once
    extern FATFS fs; 
    qspi_init(NULL, NULL);
    if (f_mount(&fs, "", 1) != FR_OK)
    {
        printf("Error mounting, probably not formatted correctly. Formatting flash...\n");
        MKFS_PARM defopt = {FM_FAT, 1, 0, 0, 4096};
        BYTE work[256];
        f_mkfs("", &defopt, work, sizeof(work));
    }

    //attachInterruptVector(IRQ_USB1, &tusbd_interrupt);
    //tusb_init();
    usbh.begin();
    memset(ring_buffer,0xFF,sizeof(ring_buffer));

    n64_init_subsystem(n64_c);
    n64_settings_init();
    n64_c[0].gpio_pin = N64_CONTROLLER_1_PIN;
    n64_c[1].gpio_pin = N64_CONTROLLER_2_PIN;
    n64_c[2].gpio_pin = N64_CONTROLLER_3_PIN;
    n64_c[3].gpio_pin = N64_CONTROLLER_4_PIN;

    pinMode(N64_CONTROLLER_1_PIN, INPUT_PULLUP);
    pinMode(N64_CONTROLLER_2_PIN, INPUT_PULLUP);
    //pinMode(N64_CONTROLLER_3_PIN, INPUT_PULLUP); //TODO MAP
    //pinMode(N64_CONTROLLER_4_PIN, INPUT_PULLUP); //TODO MAP
    NVIC_SET_PRIORITY(IRQ_USB1, 1);
    NVIC_SET_PRIORITY(IRQ_GPIO6789, 3);
    NVIC_SET_PRIORITY(IRQ_FLEXSPI2, 2);
    attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_1_PIN), n64_controller1_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_2_PIN), n64_controller2_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_3_PIN), n64_controller3_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_4_PIN), n64_controller4_clock_edge, FALLING);
}

static bool n64_combo = false;
void loop()
{
    static uint32_t usb_buttons[MAX_CONTROLLERS] = {0};
    static uint16_t n64_buttons[MAX_CONTROLLERS] = {0};
    static int32_t axis[MAX_CONTROLLERS][6] = {0};

    //tud_task();
    static uint8_t ring_buffer_print_pos = 0;
    while(ring_buffer[ring_buffer_print_pos] !=0xFF)
    {
        serial_port.write(ring_buffer[ring_buffer_print_pos]);
        ring_buffer[ring_buffer_print_pos] = 0xFF;
        ring_buffer_print_pos++;
        if (ring_buffer_print_pos >= sizeof(ring_buffer))
            ring_buffer_print_pos = 0;
    }
    
    for (int c = 0; c < MAX_CONTROLLERS; c++)
    {
        //If a change is buttons or axis has been detected
        if (gamecontroller[c]->available())
        {
            for (uint8_t i = 0; i < (sizeof(axis[c]) / sizeof(axis[c][0])); i++)
            {
                axis[c][i] = gamecontroller[c]->getAxis(i);
            }
            usb_buttons[c] = gamecontroller[c]->getButtons();
            gamecontroller[c]->joystickDataClear();
        }

        //Map usb controllers to n64 controller
        n64_buttons[c] = 0x0000;
        switch (gamecontroller[c]->joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            //Digital usb_buttons
            if (usb_buttons[c] & (1 << 0))  n64_buttons[c] |= N64_DU;  //DUP
            if (usb_buttons[c] & (1 << 1))  n64_buttons[c] |= N64_DD;  //DDOWN
            if (usb_buttons[c] & (1 << 2))  n64_buttons[c] |= N64_DL;  //DLEFT
            if (usb_buttons[c] & (1 << 3))  n64_buttons[c] |= N64_DR;  //DRIGHT
            if (usb_buttons[c] & (1 << 4))  n64_buttons[c] |= N64_ST;  //START
            if (usb_buttons[c] & (1 << 5))  n64_buttons[c] |= 0;       //BACK
            if (usb_buttons[c] & (1 << 6))  n64_buttons[c] |= 0;       //LS
            if (usb_buttons[c] & (1 << 7))  n64_buttons[c] |= 0;       //RS
            if (usb_buttons[c] & (1 << 8))  n64_buttons[c] |= N64_LB;  //LB
            if (usb_buttons[c] & (1 << 9))  n64_buttons[c] |= N64_RB;  //RB
            if (usb_buttons[c] & (1 << 10)) n64_buttons[c] |= 0;       //XBOX BUTTON
            if (usb_buttons[c] & (1 << 11)) n64_buttons[c] |= 0;       //XBOX SYNC
            if (usb_buttons[c] & (1 << 12)) n64_buttons[c] |= N64_A;   //A
            if (usb_buttons[c] & (1 << 13)) n64_buttons[c] |= N64_B;   //B
            if (usb_buttons[c] & (1 << 14)) n64_buttons[c] |= N64_B;   //X
            if (usb_buttons[c] & (1 << 15)) n64_buttons[c] |= 0;       //Y
            if (usb_buttons[c] & (1 << 7))  n64_buttons[c] |= N64_CU | //RS triggers
                                                              N64_CD | //all C usb_buttons
                                                              N64_CL |
                                                              N64_CR;
            //Analog stick
            n64_c[c].bState.x_axis = axis[c][0] * 85 / 32768;
            n64_c[c].bState.y_axis = axis[c][1] * 85 / 32768;

            //Z button
            if (axis[c][4] > 10) n64_buttons[c] |= N64_Z; //LT
            if (axis[c][5] > 10) n64_buttons[c] |= N64_Z; //RT

            //C usb_buttons
            if (axis[c][2] > 16000)  n64_buttons[c] |= N64_CR;
            if (axis[c][2] < -16000) n64_buttons[c] |= N64_CL;
            if (axis[c][3] > 16000)  n64_buttons[c] |= N64_CU;
            if (axis[c][3] < -16000) n64_buttons[c] |= N64_CD;

            //Button to hold for 'combos'
            n64_combo = (usb_buttons[c] & (1 << 5)); //back
            break;
        //TODO: OTHER USB CONTROLLERS
        default:
            break;
        }

        //Apply digital buttons to n64 controller if combo button isnt pressed
        if (n64_combo == 0)
            n64_c[c].bState.dButtons = n64_buttons[c];

        //Apply rumble if required
        if (n64_c[c].rpak != NULL)
        {
            if (n64_c[c].rpak->state == RUMBLE_START)
                gamecontroller[c]->setRumble(0xFF, 0xFF, 20);
            if (n64_c[c].rpak->state == RUMBLE_STOP)
                gamecontroller[c]->setRumble(0x00, 0x00, 20);
            n64_c[c].rpak->state = RUMBLE_APPLIED;
        }

        //Handle button combinations
        static uint32_t timer_peri_change[MAX_CONTROLLERS] = {0};
        if (n64_combo && (n64_buttons[c] & N64_DU ||
                          n64_buttons[c] & N64_DD ||
                          n64_buttons[c] & N64_DL ||
                          n64_buttons[c] & N64_DR ||
                          n64_buttons[c] & N64_ST ||
                          n64_buttons[c] & N64_LB ||
                          n64_buttons[c] & N64_RB))
        {
            if (n64_c[c].current_peripheral == PERI_NONE)
                break;

            /* HANDLE CURRENT PERIPHERAL */
            //Changing peripheral from MEMPAK
            if (n64_c[c].current_peripheral == PERI_MEMPAK)
            {
                n64_c[c].mempack->data = NULL;
            }

            //Changing peripheral from TPAK
            if (n64_c[c].current_peripheral == PERI_TPAK)
            {
                n64_c[c].tpak->gbcart->romsize = 0;
                n64_c[c].tpak->gbcart->ramsize = 0;
                n64_c[c].tpak->gbcart->ram = NULL;
                n64hal_rom_read(NULL, 0, NULL, 0);
            }

            /* HANDLE NEXT PERIPHERAL */
            n64_c[c].current_peripheral = PERI_NONE;

            //Changing peripheral to RUMBLEPAK
            if (n64_buttons[c] & N64_LB)
            {
                n64_c[c].next_peripheral = PERI_RUMBLE;
                printf("Changing controller %u's peripheral to rumblepak\n", c);
            }

            //Changing peripheral to TPAK
            if (n64_buttons[c] & N64_RB)
            {
                n64_c[c].next_peripheral = PERI_TPAK;
                printf("Changing controller %u's peripheral to tpak\n", c);

                n64_settings *settings = n64_settings_get();
                gameboycart *gb_cart = n64_c[c].tpak->gbcart;
                uint8_t gb_header[0x100];
                gb_cart->ram = NULL;
                //Read the ROM header
                strcpy((char *)gb_cart->filename, settings->default_tpak_rom[c]);
                if (n64hal_rom_read(gb_cart, 0x100, gb_header, sizeof(gb_header)))
                {
                    //Init the gb_cart struct using header info
                    gb_initGameBoyCart(gb_cart,
                                       gb_header,
                                       settings->default_tpak_rom[c]);

                    char save_filename[256];
                    if (gb_cart->ramsize > 0)
                    {
                        //Readback savefile from Flash, replace .gb or .gbc with .sav
                        char *file_name = (char *)n64_c[c].tpak->gbcart->filename;
                        strcpy(save_filename, file_name);
                        strcpy(strrchr(save_filename, '.'), ".sav");
                        gb_cart->ram = alloc_sram(save_filename, gb_cart->ramsize);
                        if (gb_cart->ram == NULL)
                            printf("ERROR: Could not alloc memory for %s\n", save_filename);
                    }
                }
                else
                {
                    printf("ERROR: Could not read %s\n", gb_cart->filename);
                }
                tpak_reset(n64_c[c].tpak);
            }

            //Changing peripheral to MEMPAK
            if ((n64_buttons[c] & N64_DU || n64_buttons[c] & N64_DD ||
                 n64_buttons[c] & N64_DL || n64_buttons[c] & N64_DR ||
                 n64_buttons[c] & N64_ST))
            {
                n64_c[c].mempack->data = NULL;
                n64_c[c].next_peripheral = PERI_MEMPAK;

                //Allocate mempack based on combo if available
                int8_t mempak_bank = 0;
                uint16_t dpad = n64_buttons[c];
                (dpad & N64_DU) ? mempak_bank = 0 : (0);
                (dpad & N64_DR) ? mempak_bank = 1 : (0);
                (dpad & N64_DD) ? mempak_bank = 2 : (0);
                (dpad & N64_DL) ? mempak_bank = 3 : (0);
                (dpad & N64_ST) ? mempak_bank = VIRTUAL_PAK : (0);
                
                //Create the filename
                uint8_t filename[32];
                snprintf((char *)filename, sizeof(filename), "MEMPAK%02u.MPK", mempak_bank);

                //Scan controllers to see if mempack is in use
                for (int i = 0; i < MAX_CONTROLLERS; i++)
                {
                    if (n64_c[i].mempack->id == mempak_bank && mempak_bank != VIRTUAL_PAK)
                    {
                        printf("Mempak already in use by controller setting to rumble %u\n", i);
                        n64_c[c].next_peripheral = PERI_RUMBLE;
                    }
                }

                //Mempack wasn't in use
                if (n64_c[c].next_peripheral != PERI_RUMBLE && mempak_bank != VIRTUAL_PAK)
                {
                    n64_c[c].mempack->data = alloc_sram((const char *)filename, MEMPAK_SIZE);
                }

                if (n64_c[c].mempack->data != NULL)
                {
                    printf("Changing controller %u's peripheral to mempak %u\n", c, mempak_bank);
                    n64_c[c].mempack->virtual_is_active = 0;
                }

                if (mempak_bank == VIRTUAL_PAK)
                {
                    n64_virtualpak_init(n64_c[c].mempack);
                }
            }
            timer_peri_change[c] = millis();
        }

        //We simulate a peripheral change time. The peripheral goes to NONE
        //for a short period. Some games need this.
        if (n64_c[c].current_peripheral == PERI_NONE &&
            (millis() - timer_peri_change[c]) > 750)
        {
            n64_c[c].current_peripheral = n64_c[c].next_peripheral;
        }

        if (n64_c[c].mempack->virtual_update_req == 1)
        {
            n64_virtualpak_update(n64_c[c].mempack);
        }

        if (n64_combo && (n64_buttons[c] & (N64_A | N64_B)))
        {
            printf("Flushing SRAM to Flash!\n");
            flush_sram();
            delay(1000);
        }
    } //END FOR LOOP
} // MAIN LOOP
