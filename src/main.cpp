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
#include "indev.h"
#include "ff.h"
#include "printf.h"
#include "n64_wrapper.h"
#include "usb64_conf.h"
#include "n64_controller.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "analog_stick.h"

#include <SD.h>
#include "tinyalloc.h"

typedef struct
{
    char name[MAX_FILENAME_LEN];
    uint8_t *data;
    int len;
    int non_volatile;
} sram_storage;

static uint8_t *alloc_sram(const char *name, int alloc_len, int non_volatile);
static void flush_sram(void);
static void init_ring_buffer(void);
static void flush_ring_buffer();

n64_controller n64_c[MAX_CONTROLLERS];
n64_settings *settings;

#if (MAX_CONTROLLERS >= 1)
void n64_controller1_clock_edge()
{
    n64_controller_hande_new_edge(&n64_c[0]);
}
#endif
#if (MAX_CONTROLLERS >= 2)
void n64_controller2_clock_edge()
{
    n64_controller_hande_new_edge(&n64_c[1]);
}
#endif
#if (MAX_CONTROLLERS >= 3)
void n64_controller3_clock_edge()
{
    n64_controller_hande_new_edge(&n64_c[2]);
}
#endif
#if (MAX_CONTROLLERS >= 4)
void n64_controller4_clock_edge()
{
    n64_controller_hande_new_edge(&n64_c[3]);
}
#endif
EXTMEM char bigBugger[1000000];

void setup()
{
    //Init the serial port and ring buffer
    serial_port.begin(115200);
    init_ring_buffer();

    //Init Ext RAM
    extern uint8_t external_psram_size; //in MB
    uint32_t psram_bytes = 1024 * 1024 * external_psram_size;
    printf("Ext ram detected %u\n", external_psram_size);
    ta_init((const void*)(0x70000000),               //base of heap
            (const void*)(0x70000000 + psram_bytes), //end of heap
            psram_bytes / 32768,                     //Number of memory chunks (32k)
            16,                                      //Smaller chunks than this won't split
            32);                                     //32 word size alignment

    void* test = ta_alloc(1000);
    printf("test ptr = %08x\n", test);
    if (ta_check())
        printf("Structural validation ok!\n");

    //Init SD card
    SD.begin(BUILTIN_SDCARD);

    flush_ring_buffer();

    //Check that the flash chip is formatted for FAT access
    //If it's not, format it! Should only happen once
    extern FATFS fs;
    if (f_mount(&fs, "", 1) != FR_OK)
    {
        debug_print_error("ERROR: Could not mount FATFS, probably not formatted correctly. Formatting flash...\n");
        MKFS_PARM defopt = {FM_FAT, 1, 0, 0, 4096};
        BYTE *work = (BYTE *)malloc(4096);
        f_mkfs("", &defopt, work, 4096);
        free(work);
        f_mount(&fs, "", 1);
    }
    flush_ring_buffer();

    indev_init(); //USB Host controller input devices
    n64_init_subsystem(n64_c);

    //Read in settings from flash
    settings = (n64_settings *)alloc_sram(SETTINGS_FILENAME, sizeof(n64_settings), 1);
    n64_settings_init(settings);

#if (MAX_CONTROLLERS >= 1)
    n64_c[0].gpio_pin = N64_CONTROLLER_1_PIN;
    pinMode(N64_CONTROLLER_1_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_1_PIN), n64_controller1_clock_edge, FALLING);
#endif

#if (MAX_CONTROLLERS >= 2)
    n64_c[1].gpio_pin = N64_CONTROLLER_2_PIN;
    pinMode(N64_CONTROLLER_2_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_2_PIN), n64_controller2_clock_edge, FALLING);
#endif

#if (MAX_CONTROLLERS >= 3)
    n64_c[2].gpio_pin = N64_CONTROLLER_3_PIN;
    pinMode(N64_CONTROLLER_3_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_3_PIN), n64_controller3_clock_edge, FALLING);
#endif

#if (MAX_CONTROLLERS >= 4)
    n64_c[3].gpio_pin = N64_CONTROLLER_4_PIN;
    pinMode(N64_CONTROLLER_4_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_4_PIN), n64_controller4_clock_edge, FALLING);
#endif

    NVIC_SET_PRIORITY(IRQ_GPIO6789, 1);
}

static bool n64_combo = false;
void loop()
{
    static uint32_t usb_buttons[MAX_CONTROLLERS] = {0};
    static int32_t usb_axis[MAX_CONTROLLERS][6] = {0};
    static uint16_t n64_buttons[MAX_CONTROLLERS] = {0};
    static int8_t n64_x_axis[MAX_CONTROLLERS] = {0};
    static int8_t n64_y_axis[MAX_CONTROLLERS] = {0};

    flush_ring_buffer();

    //Scan for controller input changes
    indev_update_input_devices();
    for (int c = 0; c < MAX_CONTROLLERS; c++)
    {
        n64_buttons[c] = 0x0000;
        if (indev_is_connected(c))
        {
            int max_axis = sizeof(usb_axis[c]) / sizeof(usb_axis[c][0]);
            indev_get_buttons(c, &usb_buttons[c], usb_axis[c], max_axis,                    //Raw usb output (if wanted)
                              &n64_buttons[c], &n64_x_axis[c], &n64_y_axis[c], &n64_combo); //Mapped n64 output

            if (indev_is_gamecontroller(c))
            {
                /* Apply analog stick options */
                n64_c[c].is_mouse = false;
                n64_settings *settings = n64_settings_get();
                float x, y, range;
                apply_deadzone(&x, &y, n64_x_axis[c] / 100.0f, n64_y_axis[c] / 100.0f, settings->deadzone[c] / 10.0f, 0.05f);
                range = apply_sensitivity(settings->sensitivity[c], &x, &y);
                if (settings->snap_axis[c])
                    apply_snap(range, &x, &y);
                apply_octa_correction(&x, &y);
                n64_x_axis[c] = x * 100.0f;
                n64_y_axis[c] = y * 100.0f;
            }
#if (MAX_MICE >= 1)
            else
            {
                n64_c[c].is_mouse = true;
            }
#endif
        }

        //Apply digital buttons and axis to n64 controller if combo button isnt pressed
        if (n64_combo == 0)
        {
            n64_c[c].b_state.dButtons = n64_buttons[c];
            n64_c[c].b_state.x_axis = n64_x_axis[c];
            n64_c[c].b_state.y_axis = n64_y_axis[c];
        }

        //Apply rumble if required
        if (n64_c[c].rpak != NULL)
        {
            if (n64_c[c].rpak->state == RUMBLE_START)
                indev_apply_rumble(c, 0xFF);
            if (n64_c[c].rpak->state == RUMBLE_STOP)
                indev_apply_rumble(c, 0x00);
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

            /* CLEAR CURRENT PERIPHERALS */
            n64_c[c].mempack->data = NULL;
            n64_c[c].mempack->id = VIRTUAL_PAK;
            if (n64_c[c].tpak->gbcart != NULL)
            {
                n64_c[c].tpak->gbcart->romsize = 0;
                n64_c[c].tpak->gbcart->ramsize = 0;
                n64_c[c].tpak->gbcart->ram = NULL;
            }

            /* HANDLE NEXT PERIPHERAL */
            n64_c[c].current_peripheral = PERI_NONE;

            //Changing peripheral to RUMBLEPAK
            if (n64_buttons[c] & N64_LB)
            {
                n64_c[c].next_peripheral = PERI_RUMBLE;
                debug_print_status("C%u to rpak\n", c);
            }

            //Changing peripheral to TPAK
            if (n64_buttons[c] & N64_RB)
            {
                n64_c[c].next_peripheral = PERI_TPAK;
                debug_print_status("C%u to tpak\n", c);

                gameboycart *gb_cart = n64_c[c].tpak->gbcart;
                uint8_t gb_header[0x100];
                gb_cart->ram = NULL;
                //Read the ROM header
                strcpy(gb_cart->filename, settings->default_tpak_rom[c]);
                if (gb_cart->filename[0] != '\0' && n64hal_rom_fastread(gb_cart, 0x100, gb_header, sizeof(gb_header)))
                {
                    //Init the gb_cart struct using header info
                    gb_init_cart(gb_cart, gb_header, settings->default_tpak_rom[c]);

                    char save_filename[MAX_FILENAME_LEN];
                    if (gb_cart->ramsize > 0)
                    {
                        //Readback savefile from Flash, replace .gb or .gbc with .sav
                        char *file_name = n64_c[c].tpak->gbcart->filename;
                        strcpy(save_filename, file_name);
                        strcpy(strrchr(save_filename, '.'), ".sav");

                        uint32_t mbc = gb_cart->mbc;
                        uint32_t volatile_flag = 0;
                        //Only if the MBC has a battery, set the non volatile flag for the SRAM.
                        if (mbc == ROM_RAM_BAT || mbc == ROM_RAM_BAT ||
                            mbc == MBC1_RAM_BAT || mbc == MBC2_BAT ||
                            mbc == MBC3_RAM_BAT || mbc == MBC3_TIM_BAT ||
                            mbc == MBC3_TIM_RAM_BAT || mbc == MBC4_RAM_BAT ||
                            mbc == MBC5_RAM_BAT || mbc == MBC5_RUM_RAM_BAT)
                        {
                            volatile_flag = 1;
                        }

                        gb_cart->ram = alloc_sram(save_filename, gb_cart->ramsize, volatile_flag);
                        if (gb_cart->ram == NULL)
                        {
                            n64_c[c].next_peripheral = PERI_RUMBLE; //Error, just set to rumblepak
                            debug_print_error("ERROR: Could not allocate sram for %s\n", save_filename);
                        }
                    }
                    tpak_reset(n64_c[c].tpak);
                }
                else
                {
                    n64_c[c].next_peripheral = PERI_RUMBLE; //Error, just set to rumblepak
                    if (gb_cart->filename[0] == '\0')
                        debug_print_error("ERROR: No default TPAK set or no ROMs found\n");
                    else
                        debug_print_error("ERROR: Could not read %s\n", gb_cart->filename);
                }
            }

            //Changing peripheral to MEMPAK
            if ((n64_buttons[c] & N64_DU || n64_buttons[c] & N64_DD ||
                 n64_buttons[c] & N64_DL || n64_buttons[c] & N64_DR ||
                 n64_buttons[c] & N64_ST))
            {
                n64_c[c].mempack->data = NULL;
                n64_c[c].next_peripheral = PERI_MEMPAK;

                //Allocate mempack based on combo if available
                uint32_t mempak_bank = 0;
                uint16_t b = n64_buttons[c];
                (b & N64_DU) ? mempak_bank = 0 : (0);
                (b & N64_DR) ? mempak_bank = 1 : (0);
                (b & N64_DD) ? mempak_bank = 2 : (0);
                (b & N64_DL) ? mempak_bank = 3 : (0);
                (b & N64_ST) ? mempak_bank = VIRTUAL_PAK : (0);

                //Create the filename
                char filename[32];
                snprintf(filename, sizeof(filename), "MEMPAK%02u.MPK", mempak_bank);

                //Scan controllers to see if mempack is in use
                for (int i = 0; i < MAX_CONTROLLERS; i++)
                {
                    if (n64_c[i].mempack->id == mempak_bank && mempak_bank != VIRTUAL_PAK)
                    {
                        debug_print_status("WARNING: map in use by C%u. Setting to rpak\n", i);
                        n64_c[c].next_peripheral = PERI_RUMBLE;
                    }
                }

                //Mempack wasn't in use, so allocate it in ram
                if (n64_c[c].next_peripheral != PERI_RUMBLE && mempak_bank != VIRTUAL_PAK)
                {
                    n64_c[c].mempack->data = alloc_sram(filename, MEMPAK_SIZE, 1);
                }

                if (n64_c[c].mempack->data != NULL)
                {
                    debug_print_status("C%u to mpak %u\n", c, mempak_bank);
                    n64_c[c].mempack->virtual_is_active = 0;
                    n64_c[c].mempack->id = mempak_bank;
                }
                else if (mempak_bank == VIRTUAL_PAK)
                {
                    debug_print_status("C%u to virtual pak\n", c);
                    n64_virtualpak_init(n64_c[c].mempack);
                }
                else
                {
                    n64_c[c].next_peripheral = PERI_RUMBLE; //Error, set to rumblepak
                }
            }
            timer_peri_change[c] = millis();
        }

        //Simulate a peripheral change time. The peripheral goes to NONE
        //for a short period. Some games need this.
        if (n64_c[c].current_peripheral == PERI_NONE &&
            (millis() - timer_peri_change[c]) > 750)
        {
            n64_c[c].current_peripheral = n64_c[c].next_peripheral;
        }

        //Update the virtual pak if required
        if (n64_c[c].mempack->virtual_update_req == 1)
        {
            char msg[256];
            int offset = 0;
            for (int i = 0; i < MAX_CONTROLLERS; i++)
            {
                offset += sprintf(msg + offset, "1:0x%04x/0x%04x\n%.15s\n%.15s\n", indev_get_id_vendor(i),
                                  indev_get_id_product(i),
                                  indev_get_manufacturer_string(i),
                                  indev_get_product_string(i));
            }
            n64_virtualpak_write_info_1(msg);
            n64_virtualpak_update(n64_c[c].mempack);
        }

        //If you pressed the combo to flush sram to flash, handle it here
        static uint32_t flushing[MAX_CONTROLLERS] = {0};
        if (n64_combo && (n64_buttons[c] & (N64_A | N64_B)))
        {
            if (!flushing[c])
            {
                debug_print_status("Flushing SRAM to Flash!\n");
                flush_sram();
                flushing[c] = 1;
            }
        }
        else if (n64_combo)
        {
            flushing[c] = 0;
        }

    } //END FOR LOOP
} // MAIN LOOP

//Ring buffer is used to buffer printf outputs
static uint32_t ring_buffer_pos = 0;
static char ring_buffer[4096];
void _putchar(char character)
{
    ring_buffer[ring_buffer_pos] = character;
    ring_buffer_pos++;
    if (ring_buffer_pos >= sizeof(ring_buffer))
        ring_buffer_pos = 0;
}

static void init_ring_buffer()
{
    memset(ring_buffer, 0xFF, sizeof(ring_buffer));
}

static void flush_ring_buffer()
{
    static uint32_t ring_buffer_print_pos = 0;
    while (ring_buffer[ring_buffer_print_pos] != 0xFF)
    {
        serial_port.write(ring_buffer[ring_buffer_print_pos]);
        ring_buffer[ring_buffer_print_pos] = 0xFF;
        ring_buffer_print_pos++;
        if (ring_buffer_print_pos >= sizeof(ring_buffer))
            ring_buffer_print_pos = 0;
    }
}

//This function allocates and manages SRAM for mempak and gameboy roms (tpak) for the system.
//SRAM is malloced into slots. Each slot stores a pointer to the memory location, its size, and
//a string name to identify what that slot is used for.
//A flag is set to determine wether that memory is non volatile and should be backed up/restored from flash.
sram_storage sram[10] = {0};
static uint8_t *alloc_sram(const char *name, int alloc_len, int non_volatile)
{
    if (alloc_len == 0)
        return NULL;

    //Loop through to see if alloced memory already exists
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (strcmp(sram[i].name, name) == 0)
        {
            //Already malloced, check len is ok
            if (sram[i].len <= alloc_len)
                return sram[i].data;

            debug_print_error("ERROR: SRAM malloced memory isnt right, resetting memory\n");
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
            sram[i].data = (uint8_t *)malloc(alloc_len);
            if (sram[i].data == NULL)
                break;
            sram[i].len = alloc_len;
            strcpy(sram[i].name, name);
            if (non_volatile)
            {
                n64hal_sram_restore_from_file((uint8_t *)sram[i].name,
                                              sram[i].data,
                                              sram[i].len);
                sram[i].non_volatile = 1;
            }
            else
            {
                sram[i].non_volatile = 0;
                memset(sram[i].data, 0x00, sram[i].len);
            }

            return sram[i].data;
        }
    }
    debug_print_error("ERROR: No SRAM space or slots left. Flush RAM to Flash!\n");
    return NULL;
}

//Flush SRAM to flash memory if the non volatile flag is set
static void flush_sram()
{
    noInterrupts();
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if (sram[i].len == 0 || sram[i].data == NULL || sram[i].non_volatile == 0)
            continue;
        debug_print_status("Writing %s %u bytes\n", (uint8_t *)sram[i].name, sram[i].len);
        n64hal_sram_backup_to_file((uint8_t *)sram[i].name, sram[i].data, sram[i].len);
    }
    interrupts();
    flush_ring_buffer();
}
