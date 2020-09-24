// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "input.h"
#include "ff.h"
#include "printf.h"
#include "n64_wrapper.h"
#include "usb64_conf.h"
#include "n64_controller.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "analog_stick.h"
#include "memory.h"
#include "fileio.h"

static void ring_buffer_init(void);
static void ring_buffer_flush();

n64_controller n64_c[MAX_CONTROLLERS];
n64_settings *settings;

#if (MAX_CONTROLLERS >= 1)
void n64_controller1_clock_edge()
{
    if(input_is_connected(0))
        n64_controller_hande_new_edge(&n64_c[0]);
}
#endif
#if (MAX_CONTROLLERS >= 2)
void n64_controller2_clock_edge()
{
    if(input_is_connected(1))
        n64_controller_hande_new_edge(&n64_c[1]);
}
#endif
#if (MAX_CONTROLLERS >= 3)
void n64_controller3_clock_edge()
{
    if(input_is_connected(2))
        n64_controller_hande_new_edge(&n64_c[2]);
}
#endif
#if (MAX_CONTROLLERS >= 4)
void n64_controller4_clock_edge()
{
    if(input_is_connected(3))
        n64_controller_hande_new_edge(&n64_c[3]);
}
#endif

void setup()
{
    //Init the serial port and ring buffer
    serial_port.begin(115200);
    ring_buffer_init();

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

    fileio_init();

    memory_init();

    input_init();

    n64_subsystem_init(n64_c);

    //Read in settings from flash
    settings = (n64_settings *)memory_alloc_ram(SETTINGS_FILENAME, sizeof(n64_settings), 0);
    n64_settings_init(settings);

    //Set up N64 sense pin. To determine is the N64 is turned on or off
    //Input is connected to the N64 3V3 line on the controller port.
    pinMode(N64_CONSOLE_SENSE, INPUT_PULLDOWN);

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

    ring_buffer_flush();

    //Scan for controller input changes
    input_update_input_devices();
    for (int c = 0; c < MAX_CONTROLLERS; c++)
    {
        n64_buttons[c] = 0x0000;
        if (input_is_connected(c))
        {
            int max_axis = sizeof(usb_axis[c]) / sizeof(usb_axis[c][0]);
            input_get_buttons(c, &usb_buttons[c], usb_axis[c], max_axis,                       //Raw usb output (if wanted)
                                 &n64_buttons[c], &n64_x_axis[c], &n64_y_axis[c], &n64_combo); //Mapped n64 output

            if (input_is_gamecontroller(c))
            {
                /* Apply analog stick options */
                n64_c[c].is_mouse = false;
                n64_settings *settings = n64_settings_get();
                float x, y, range;
                astick_apply_deadzone(&x, &y, n64_x_axis[c] / 100.0f,
                                              n64_y_axis[c] / 100.0f,
                                              settings->deadzone[c] / 10.0f, 0.05f);
                
                if(input_is_dualstick_mode(c) && (c % 2) == 0)
                {
                    //If in dual analog stick mode, force lowest sensitivity. Seems too sensitive otherwise
                    range = astick_apply_sensitivity(0, &x, &y);
                }
                else
                {
                    range = astick_apply_sensitivity(settings->sensitivity[c], &x, &y);
                    if (settings->snap_axis[c])
                    astick_apply_snap(range, &x, &y);
                    astick_apply_octa_correction(&x, &y);
                }

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
                input_apply_rumble(c, 0xFF);
            if (n64_c[c].rpak->state == RUMBLE_STOP)
                input_apply_rumble(c, 0x00);
            n64_c[c].rpak->state = RUMBLE_APPLIED;
        }

        //Handle dual stick mode toggling
        static uint32_t dual_stick_toggle[MAX_CONTROLLERS] = {0};
        if (n64_combo && (n64_buttons[c] & N64_B))
        {
            if (dual_stick_toggle[c] == 0)
            {
                input_is_dualstick_mode(c) ? input_disable_dualstick_mode(c) : input_enable_dualstick_mode(c);
                debug_print_status("Dual stick mode for %u is %u\n", c, input_is_dualstick_mode(c));
                dual_stick_toggle[c] = 1;
            }
        }
        else
        {
            dual_stick_toggle[c] = 0;
        }
 
        //Handle ram flushing. Auto flushes when the N64 is turned off :)
        static uint32_t flushing_toggle[MAX_CONTROLLERS] = {0};
        if ((n64_combo && (n64_buttons[c] & N64_A)) || digitalReadFast(N64_CONSOLE_SENSE) == 0)
        {
            if (flushing_toggle[c] == 0)
            {
                memory_flush_all();
                debug_print_status("Flushed RAM to SD card as required\n");
                flushing_toggle[c] = 1;
            }
        }
        else
        {
            flushing_toggle[c] = 0;
        }

        //Handle peripheral change combinations
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
            if (n64_c[c].mempack != NULL)
            {
                n64_c[c].mempack->data = NULL;
                n64_c[c].mempack->id = VIRTUAL_PAK;
            }

            if (n64_c[c].tpak != NULL)
            {
                tpak_reset(n64_c[c].tpak);
                if (n64_c[c].tpak->gbcart != NULL)
                {
                    memory_free_item(n64_c[c].tpak->gbcart->rom);
                    n64_c[c].tpak->gbcart->filename[0] = '\0';
                    n64_c[c].tpak->gbcart->romsize = 0;
                    n64_c[c].tpak->gbcart->ramsize = 0;
                    n64_c[c].tpak->gbcart->ram = NULL; //Not free'd intentionally
                    n64_c[c].tpak->gbcart->rom = NULL;
                }
            }

            if (n64_c[c].rpak != NULL)
            {
                if (n64_c[c].rpak->state != RUMBLE_APPLIED)
                {
                    n64_c[c].rpak->state = RUMBLE_APPLIED;
                    input_apply_rumble(c, 0x00);
                }
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

                strcpy(gb_cart->filename, settings->default_tpak_rom[c]);
                if (gb_cart->filename[0] != '\0')
                {
                    fileio_read_from_file(gb_cart->filename, 0x100, gb_header, sizeof(gb_header));
                    gb_init_cart(gb_cart, gb_header, settings->default_tpak_rom[c]);

                    if (gb_cart->romsize > 0)
                    {
                        gb_cart->rom = memory_alloc_ram(n64_c[c].tpak->gbcart->filename, gb_cart->romsize, 1);
                    }

                    if (gb_cart->ramsize > 0)
                    {
                        //Readback savefile from Flash, replace .gb or .gbc with .sav
                        char save_filename[MAX_FILENAME_LEN];
                        strcpy(save_filename, n64_c[c].tpak->gbcart->filename);
                        strcpy(strrchr(save_filename, '.'), ".sav");
                        gb_cart->ram = memory_alloc_ram(save_filename, gb_cart->ramsize, gb_has_battery(gb_cart->mbc) == 0);
                    }

                    if (gb_cart->rom == NULL || gb_cart->ram == NULL)
                    {
                        n64_c[c].next_peripheral = PERI_RUMBLE; //Error, just set to rumblepak
                        debug_print_error("ERROR: Could not allocate rom or ram buffer for %s\n", n64_c[c].tpak->gbcart->filename);
                        n64_c[c].tpak->gbcart->romsize = 0;
                        n64_c[c].tpak->gbcart->ramsize = 0;
                        n64_c[c].tpak->gbcart->ram = NULL;
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
                    n64_c[c].mempack->data = memory_alloc_ram(filename, MEMPAK_SIZE, 0);
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
                    debug_print_error("ERROR: Could not alloc RAM for %s, seting to rpak\n", filename);
                    n64_c[c].next_peripheral = PERI_RUMBLE; //Error, set to rumblepak
                }
            }
            timer_peri_change[c] = millis();
        }

        //Simulate a peripheral change time. The peripheral goes to NONE
        //for a short period. Some games need this.
        if (n64_c[c].current_peripheral == PERI_NONE && (millis() - timer_peri_change[c]) > 750)
        {
            n64_c[c].current_peripheral = n64_c[c].next_peripheral;
        }

        //Update the virtual pak if required
        if (n64_c[c].mempack->virtual_update_req == 1)
        {
            //For the USB64-INFO1 Page I write the controller info (PID,VID etc)
            char msg[256];
            n64_virtualpak_update(n64_c[c].mempack); //Update so we get the right page
            uint8_t c_page = n64_virtualpak_get_controller_page();
            sprintf(msg, "%u:0x%04x/0x%04x\n%.15s\n%.15s\n", c_page+1,
                        input_get_id_vendor(c_page),
                        input_get_id_product(c_page),
                        input_get_manufacturer_string(c_page),
                        input_get_product_string(c_page));
            n64_virtualpak_write_info_1(msg);

            //Normal update
            n64_virtualpak_update(n64_c[c].mempack);
        }

    } //END FOR LOOP
} // MAIN LOOP

/* PRINTF HANDLING */
static uint32_t ring_buffer_pos = 0;
static char ring_buffer[4096];
void _putchar(char character)
{
    ring_buffer[ring_buffer_pos] = character;
    ring_buffer_pos++;
    if (ring_buffer_pos >= sizeof(ring_buffer))
        ring_buffer_pos = 0;
}

static void ring_buffer_init()
{
    memset(ring_buffer, 0xFF, sizeof(ring_buffer));
}

static void ring_buffer_flush()
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
