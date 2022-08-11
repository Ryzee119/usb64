// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "common.h"
#include "input.h"
#include "n64_controller.h"
#include "analog_stick.h"
#include "memory.h"
#include "fileio.h"
#include "tft.h"

void usbh_dev_init(void);
static void ring_buffer_init(void);
static void ring_buffer_flush();

n64_input_dev_t n64_in_dev[MAX_CONTROLLERS];
n64_settings *settings;
int n64_is_on = 0;

#if (MAX_CONTROLLERS >= 1)
void n64_controller1_clock_edge()
{
    n64_controller_hande_new_edge(&n64_in_dev[0]);
}
#endif
#if (MAX_CONTROLLERS >= 2)
void n64_controller2_clock_edge()
{
    n64_controller_hande_new_edge(&n64_in_dev[1]);
}
#endif
#if (MAX_CONTROLLERS >= 3)
void n64_controller3_clock_edge()
{
    n64_controller_hande_new_edge(&n64_in_dev[2]);
}
#endif
#if (MAX_CONTROLLERS >= 4)
void n64_controller4_clock_edge()
{
    n64_controller_hande_new_edge(&n64_in_dev[3]);
}
#endif

#ifdef CFG_TUSB_DEBUG_PRINTF
extern "C" int CFG_TUSB_DEBUG_PRINTF(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    usb64_vprintf(format, args);
    return 1;
}
#endif

#ifndef ARDUINO
void setup();
void loop();
int main(void)
{
    setup();
    while(1)
    {
        loop();
    }
}
#endif

void setup()
{
    n64hal_system_init();
    n64hal_debug_init();
    n64hal_gpio_init();
    ring_buffer_init();
    fileio_init();
    memory_init();
    usbh_dev_init();
    input_init();
    tft_init();
    n64_subsystem_init(n64_in_dev);

    //Read in settings from flash
    settings = (n64_settings *)memory_alloc_ram(SETTINGS_FILENAME, sizeof(n64_settings), MEMORY_READ_WRITE);
    n64_settings_init(settings);
}

static bool n64_combo = false;
void loop()
{
    static uint8_t n64_response[MAX_CONTROLLERS][32] = {0};

    ring_buffer_flush();

    input_update_input_devices();

    tft_try_update();

    for (uint32_t c = 0; c < MAX_CONTROLLERS; c++)
    {
        if (input_is_connected(c))
        {
            if (n64_is_on && !n64_in_dev[c].interrupt_attached)
            {
                switch (c)
                {
                    case 0: n64hal_attach_interrupt(n64_in_dev[c].pin, n64_controller1_clock_edge, N64_INTMODE_FALLING); break;
                    case 1: n64hal_attach_interrupt(n64_in_dev[c].pin, n64_controller2_clock_edge, N64_INTMODE_FALLING); break;
                    case 2: n64hal_attach_interrupt(n64_in_dev[c].pin, n64_controller3_clock_edge, N64_INTMODE_FALLING); break;
                    case 3: n64hal_attach_interrupt(n64_in_dev[c].pin, n64_controller4_clock_edge, N64_INTMODE_FALLING); break;
                }
                n64_in_dev[c].interrupt_attached = true;
                tft_flag_update();
            }
            if (input_is(c, INPUT_GAMECONTROLLER))
            {
                n64_buttonmap *new_state = (n64_buttonmap *)n64_response[c];
                input_get_state(c, new_state,  &n64_combo);

                if(n64_in_dev[c].type != N64_CONTROLLER)
                {
                    n64_in_dev[c].type = N64_CONTROLLER;
                    tft_flag_update();
                }
                n64_settings *settings = n64_settings_get();
                float x, y, range;
                astick_apply_deadzone(&x, &y, new_state->x_axis / 100.0f,
                                              new_state->y_axis / 100.0f,
                                              settings->deadzone[c] / 10.0f, 0.05f);
                
                if(input_is_dualstick_mode(c) && (c % 2) == 0 /*Controller 0 or 2 only*/)
                {
                    //If in dual analog stick mode, force lowest sensitivity. Seems too sensitive otherwise.
                    range = astick_apply_sensitivity(0, &x, &y);
                }
                else
                {
                    range = astick_apply_sensitivity(settings->sensitivity[c], &x, &y);
                    if (settings->snap_axis[c]) astick_apply_snap(range, &x, &y);
                    if (settings->octa_correct[c]) astick_apply_octa_correction(&x, &y);
                }

                new_state->x_axis = x * 100.0f;
                new_state->y_axis = y * 100.0f;

                //Apply digital buttons and axis to n64 controller if combo button isnt pressed
                if (n64_combo == 0)
                {
                    n64_in_dev[c].b_state.dButtons = new_state->dButtons;
                    n64_in_dev[c].b_state.x_axis = new_state->x_axis;
                    n64_in_dev[c].b_state.y_axis = new_state->y_axis;
                }
            }
#if (MAX_MICE >= 1)
            else if (input_is(c, INPUT_MOUSE))
            {
                n64_buttonmap *new_state = (n64_buttonmap *)n64_response[c];
                input_get_state(c, new_state,  &n64_combo);

                if(n64_in_dev[c].type != N64_MOUSE)
                {
                    n64_in_dev[c].type = N64_MOUSE;
                    tft_flag_update();
                }
                n64_in_dev[c].b_state.dButtons = new_state->dButtons;
                n64_in_dev[c].b_state.x_axis = new_state->x_axis;
                n64_in_dev[c].b_state.y_axis = new_state->y_axis;
            }
#endif
#if (MAX_KB >= 1)
            else if (input_is(c, INPUT_KEYBOARD))
            {
                n64_randnet_kb *new_state = (n64_randnet_kb *)n64_response[c];
                //Maintain the old led state
                new_state->led_state = n64_in_dev[c].kb_state.led_state;

                input_get_state(c, new_state,  &n64_combo);

                if(n64_in_dev[c].type != N64_RANDNET)
                {
                    n64_in_dev[c].type = N64_RANDNET;
                    tft_flag_update();
                }
                memcpy(&n64_in_dev[c].kb_state, new_state, sizeof(n64_randnet_kb));
            }
#endif
        }

        if ((!input_is_connected(c) || !n64_is_on) && n64_in_dev[c].interrupt_attached)
        {
            n64_in_dev[c].interrupt_attached = false;
            n64hal_detach_interrupt(n64_in_dev[c].pin);
            tft_flag_update();
        }

        //Get a copy of the latest n64 button presses to handle the below combos
        uint16_t n64_buttons = 0;
        if (input_is(c, INPUT_GAMECONTROLLER))
        {
            n64_buttonmap *new_state = (n64_buttonmap *)n64_response[c];
            n64_buttons = new_state->dButtons;
        }

        //Apply rumble if required
        if (n64_in_dev[c].rpak != NULL)
        {
            if (n64_in_dev[c].rpak->state == RUMBLE_START)
                input_apply_rumble(c, 0xFF);
            if (n64_in_dev[c].rpak->state == RUMBLE_STOP)
                input_apply_rumble(c, 0x00);
            if (n64_in_dev[c].rpak->state != RUMBLE_APPLIED && ENABLE_HARDWIRED_CONTROLLER)
                n64hal_output_set(HW_RUMBLE, n64_in_dev[c].rpak->state == RUMBLE_START);
            n64_in_dev[c].rpak->state = RUMBLE_APPLIED;
        }

        //Handle dual stick mode toggling
        static uint32_t dual_stick_toggle[MAX_CONTROLLERS] = {0};
        if (n64_combo && (n64_buttons & N64_B))
        {
            if (dual_stick_toggle[c] == 0)
            {
                input_is_dualstick_mode(c) ? input_disable_dualstick_mode(c) : input_enable_dualstick_mode(c);
                debug_print_status("[MAIN] Dual stick mode for %u is %u\n", c, input_is_dualstick_mode(c));
                dual_stick_toggle[c] = 1;
            }
        }
        else
        {
            dual_stick_toggle[c] = 0;
        }

        //Handle ram flushing. Auto flushes when the N64 is turned off :)
        static uint32_t flushing_toggle[MAX_CONTROLLERS] = {0};
        n64_is_on = n64hal_input_read(N64_CONSOLE_SENSE_PIN);
        if ((n64_combo && (n64_buttons & N64_A)) || (n64_is_on == 0))
        {
            if (flushing_toggle[c] == 0)
            {
                memory_flush_all();
                debug_print_status("[MAIN] Flushed RAM to SD card as required\n");
                flushing_toggle[c] = 1;
                tft_flag_update();
            }
        }
        else
        {
            if (flushing_toggle[c]) tft_flag_update();
            flushing_toggle[c] = 0;
        }

#if (ENABLE_TFT_DISPLAY >= 1)
        //Cycle TFT display
        static uint32_t tft_toggle[MAX_CONTROLLERS] = {0};
        if (n64_buttons & N64_LB && n64_buttons & N64_RB)
        {
            static uint8_t tft_page = 0;
            if (tft_toggle[c] == 0)
            {
                tft_page = tft_change_page(++tft_page);
                tft_toggle[c] = 1;
                tft_flag_update();
            }
        }
        else
        {
            tft_toggle[c] = 0;
        }
#endif

        //Handle peripheral change combinations
        static uint32_t timer_peri_change[MAX_CONTROLLERS] = {0};
        if (n64_combo && (n64_buttons & N64_DU ||
                          n64_buttons & N64_DD ||
                          n64_buttons & N64_DL ||
                          n64_buttons & N64_DR ||
                          n64_buttons & N64_ST ||
                          n64_buttons & N64_LB ||
                          n64_buttons & N64_RB))
        {
            if (n64_in_dev[c].current_peripheral == PERI_NONE)
                break; //Already changing peripheral

            timer_peri_change[c] = n64hal_millis();

            /* CLEAR CURRENT PERIPHERALS */
            if (n64_in_dev[c].cpak != NULL)
            {
                n64_in_dev[c].cpak->data = NULL;
                n64_in_dev[c].cpak->id = VIRTUAL_PAK;
            }

            if (n64_in_dev[c].tpak != NULL)
            {
                tpak_reset(n64_in_dev[c].tpak);
                if (n64_in_dev[c].tpak->gbcart != NULL)
                {
                    memory_free_item(n64_in_dev[c].tpak->gbcart->rom);
                    n64_in_dev[c].tpak->gbcart->filename[0] = '\0';
                    n64_in_dev[c].tpak->gbcart->romsize = 0;
                    n64_in_dev[c].tpak->gbcart->ramsize = 0;
                    n64_in_dev[c].tpak->gbcart->ram = NULL; //RAM not free'd intentionally
                    n64_in_dev[c].tpak->gbcart->rom = NULL;
                }
            }

            if (n64_in_dev[c].rpak != NULL)
            {
                if (n64_in_dev[c].rpak->state != RUMBLE_APPLIED)
                {
                    input_apply_rumble(c, 0x00);
                    n64_in_dev[c].rpak->state = RUMBLE_APPLIED;
                }
            }

            /* HANDLE NEXT PERIPHERAL */
            n64_in_dev[c].current_peripheral = PERI_NONE; //Go to none whilst changing
            tft_force_update();

            //Changing peripheral to RUMBLEPAK
            if (n64_buttons & N64_LB)
            {
                n64_in_dev[c].next_peripheral = PERI_RPAK;
                debug_print_status("[MAIN] C%u to rpak\n", c);
            }

            //Changing peripheral to TPAK
            if (n64_buttons & N64_RB)
            {
                n64_in_dev[c].next_peripheral = PERI_TPAK;
                debug_print_status("[MAIN] C%u to tpak\n", c);

                gameboycart *gb_cart = n64_in_dev[c].tpak->gbcart;
                uint8_t gb_header[0x100];

                strcpy(gb_cart->filename, settings->default_tpak_rom[c]);
                if (gb_cart->filename[0] != '\0' && memory_get_ext_ram_size() > 0)
                {
                    fileio_read_from_file(gb_cart->filename, 0x100, gb_header, sizeof(gb_header));
                    gb_init_cart(gb_cart, gb_header, settings->default_tpak_rom[c]);

                    if (gb_cart->romsize > 0)
                    {
                        gb_cart->rom = memory_alloc_ram(n64_in_dev[c].tpak->gbcart->filename, gb_cart->romsize, MEMORY_READ_ONLY);
                    }

                    if (gb_cart->ramsize > 0)
                    {
                        //Readback savefile from Flash, replace .gb or .gbc with save file extension
                        char save_filename[MAX_FILENAME_LEN];
                        strcpy(save_filename, n64_in_dev[c].tpak->gbcart->filename);
                        strcpy(strrchr(save_filename, '.'), GAMEBOY_SAVE_EXT);
                                                                                         /*WRITE ONLY IF CART HAS BATTERY*/
                        gb_cart->ram = memory_alloc_ram(save_filename, gb_cart->ramsize, gb_has_battery(gb_cart->mbc) == 0);
                    }

                    if (gb_cart->rom == NULL || (gb_cart->ram == NULL && gb_cart->ramsize > 0))
                    {
                        n64_in_dev[c].next_peripheral = PERI_RPAK; //Error, just set to rumblepak
                        debug_print_error("[MAIN] ERROR: Could not allocate rom or ram buffer for %s\n", n64_in_dev[c].tpak->gbcart->filename);
                        n64_in_dev[c].tpak->gbcart->romsize = 0;
                        n64_in_dev[c].tpak->gbcart->ramsize = 0;
                        n64_in_dev[c].tpak->gbcart->ram = NULL;
                        if (gb_cart->rom !=NULL) memory_free_item(gb_cart->rom);
                    }
                }
                else
                {
                    n64_in_dev[c].next_peripheral = PERI_RPAK; //Error, just set to rumblepak
                    if (gb_cart->filename[0] == '\0')
                        debug_print_error("[MAIN] ERROR: No default TPAK ROM set or no ROMs found\n");
                    else if (memory_get_ext_ram_size() == 0)
                        debug_print_error("[MAIN] ERROR: No external RAM installed. TPAK disabled\n");
                    else
                        debug_print_error("[MAIN] ERROR: Could not read %s\n", gb_cart->filename);
                }
            }

            //Changing peripheral to CPAK
            if ((n64_buttons & N64_DU || n64_buttons & N64_DD ||
                 n64_buttons & N64_DL || n64_buttons & N64_DR ||
                 n64_buttons & N64_ST))
            {
                n64_in_dev[c].next_peripheral = PERI_CPAK;

                //Allocate cpak based on combo if available
                uint32_t cpak_bank_num = 0;
                uint16_t b = n64_buttons;
                (b & N64_DU) ? cpak_bank_num = 0 : (0);
                (b & N64_DR) ? cpak_bank_num = 1 : (0);
                (b & N64_DD) ? cpak_bank_num = 2 : (0);
                (b & N64_DL) ? cpak_bank_num = 3 : (0);
                (b & N64_ST) ? cpak_bank_num = VIRTUAL_PAK : (0);

                //Create the filename
                char filename[32];
                snprintf(filename, sizeof(filename), "CONTROLLER_PAK_%02u%s", (unsigned int)cpak_bank_num, CPAK_SAVE_EXT);

                //Scan controllers to see if cpak is in use
                for (uint32_t i = 0; i < MAX_CONTROLLERS; i++)
                {
                    if (n64_in_dev[i].cpak->id == cpak_bank_num && cpak_bank_num != VIRTUAL_PAK)
                    {
                        debug_print_status("[MAIN] WARNING: cpak in use by C%u. Setting to rpak\n", i);
                        n64_in_dev[c].next_peripheral = PERI_RPAK;
                        break;
                    }
                }

                //cpak wasn't in use, so allocate it in ram
                if (n64_in_dev[c].next_peripheral != PERI_RPAK && cpak_bank_num != VIRTUAL_PAK)
                {
                    n64_in_dev[c].cpak->data = memory_alloc_ram(filename, CPAK_SIZE, MEMORY_READ_WRITE);
                    //TODO: create an initalized CPAK data if file is empty or does not exist?!
                }

                if (n64_in_dev[c].cpak->data != NULL)
                {
                    debug_print_status("[MAIN] C%u to cpak %u\n", c, cpak_bank_num);
                    n64_in_dev[c].cpak->virtual_is_active = 0;
                    n64_in_dev[c].cpak->id = cpak_bank_num;
                }
                else if (cpak_bank_num == VIRTUAL_PAK)
                {
                    debug_print_status("[MAIN] C%u to virtual cpak\n", c);
                    n64_virtualpak_init(n64_in_dev[c].cpak);
                }
                else
                {
                    debug_print_error("[MAIN] ERROR: Could not alloc RAM for %s, setting to rpak\n", filename);
                    n64_in_dev[c].next_peripheral = PERI_RPAK;
                }
            }
        }

        //Simulate a peripheral change time. The peripheral goes to NONE
        //for a short period. Some games need this.
        if (n64_in_dev[c].current_peripheral == PERI_NONE && (n64hal_millis() - timer_peri_change[c]) > PERI_CHANGE_TIME)
        {
            n64_in_dev[c].current_peripheral = n64_in_dev[c].next_peripheral;
            tft_flag_update();
        }

        //Update the virtual pak if required
        if (n64_in_dev[c].cpak->virtual_update_req == 1)
        {
            //For the USB64-INFO1 Page, I write the controller info (PID,VID etc)
            char msg[256];
            n64_virtualpak_update(n64_in_dev[c].cpak); //Update so we get the right page
            uint8_t c_page = n64_virtualpak_get_controller_page();
            sprintf(msg, "%u:0x%04x/0x%04x\n%.15s\n%.15s\n",
                        c_page + 1,
                        input_get_id_vendor(c_page),
                        input_get_id_product(c_page),
                        input_get_manufacturer_string(c_page),
                        input_get_product_string(c_page));
            n64_virtualpak_write_info_1(msg);

            //Normal update
            n64_virtualpak_update(n64_in_dev[c].cpak);
        }

    } //END FOR LOOP
} // MAIN LOOP

/* PRINTF HANDLING */
static uint32_t ring_buffer_pos = 0;
static char ring_buffer[4096];
extern "C" void _putchar(char character)
{
    ring_buffer[ring_buffer_pos] = character;
    ring_buffer_pos = (ring_buffer_pos + 1) % sizeof(ring_buffer);
}

static void ring_buffer_init()
{
    memset(ring_buffer, 0xFF, sizeof(ring_buffer));
}

static void ring_buffer_flush()
{
    static uint32_t _print_cursor = 0;
    while (ring_buffer[_print_cursor] != 0xFF)
    {
        n64hal_debug_write(ring_buffer[_print_cursor]);
        tft_add_log(ring_buffer[_print_cursor]);
        ring_buffer[_print_cursor] = 0xFF;
        _print_cursor = (_print_cursor + 1) % sizeof(ring_buffer);
    }
}
