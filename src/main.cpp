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
#include "tft.h"


static void ring_buffer_init(void);
static void ring_buffer_flush();

n64_controller n64_c[MAX_CONTROLLERS];
n64_settings *settings;
int n64_is_on = 0;

#if (MAX_CONTROLLERS >= 1)
void n64_controller1_clock_edge()
{
    if(input_is_connected(0) && n64_is_on)
        n64_controller_hande_new_edge(&n64_c[0]);
}
#endif
#if (MAX_CONTROLLERS >= 2)
void n64_controller2_clock_edge()
{
    if(input_is_connected(1) && n64_is_on)
        n64_controller_hande_new_edge(&n64_c[1]);
}
#endif
#if (MAX_CONTROLLERS >= 3)
void n64_controller3_clock_edge()
{
    if(input_is_connected(2) && n64_is_on)
        n64_controller_hande_new_edge(&n64_c[2]);
}
#endif
#if (MAX_CONTROLLERS >= 4)
void n64_controller4_clock_edge()
{
    if(input_is_connected(3) && n64_is_on)
        n64_controller_hande_new_edge(&n64_c[3]);
}
#endif

extern "C" {
extern void startup_early_hook(void)
{
    //Get these up as early as possible.
    pinMode(N64_CONTROLLER_1_PIN, INPUT_PULLUP);
    pinMode(N64_CONTROLLER_2_PIN, INPUT_PULLUP);
    pinMode(N64_CONTROLLER_3_PIN, INPUT_PULLUP);
    pinMode(N64_CONTROLLER_4_PIN, INPUT_PULLUP);
    pinMode(N64_CONSOLE_SENSE, INPUT_PULLDOWN);
}
}

void setup()
{
    //Init the serial port and ring buffer
    serial_port.begin(256000);

    ring_buffer_init();
    fileio_init();
    memory_init();
    input_init();
    tft_init();
    n64_subsystem_init(n64_c);

    //Read in settings from flash
    settings = (n64_settings *)memory_alloc_ram(SETTINGS_FILENAME, sizeof(n64_settings), READ_WRITE);
    n64_settings_init(settings);

    //Set up N64 sense pin. To determine is the N64 is turned on or off
    //Input is connected to the N64 3V3 line on the controller port.
    pinMode(N64_CONSOLE_SENSE, INPUT_PULLDOWN);

    pinMode(N64_FRAME, OUTPUT);

    pinMode(USER_LED_PIN, OUTPUT);

#if (ENABLE_HARDWIRED_CONTROLLER >=1)
    pinMode(HW_A, INPUT_PULLUP);
    pinMode(HW_B, INPUT_PULLUP);
    pinMode(HW_CU, INPUT_PULLUP);
    pinMode(HW_CD, INPUT_PULLUP);
    pinMode(HW_CL, INPUT_PULLUP);
    pinMode(HW_CR, INPUT_PULLUP);
    pinMode(HW_DU, INPUT_PULLUP);
    pinMode(HW_DD, INPUT_PULLUP);
    pinMode(HW_DL, INPUT_PULLUP);
    pinMode(HW_DR, INPUT_PULLUP);
    pinMode(HW_START, INPUT_PULLUP);
    pinMode(HW_Z, INPUT_PULLUP);
    pinMode(HW_R, INPUT_PULLUP);
    pinMode(HW_L, INPUT_PULLUP);
    pinMode(HW_EN, INPUT_PULLUP);
    pinMode(HW_RUMBLE, OUTPUT);
#endif

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

    digitalWrite(USER_LED_PIN, HIGH);
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
            if (input_is_gamecontroller(c))
            {
                n64_buttonmap *new_state = (n64_buttonmap *)n64_response[c];
                input_get_state(c, new_state,  &n64_combo);

                if(n64_c[c].is_mouse == true || n64_c[c].is_kb == true)
                {
                    n64_c[c].is_mouse = false;
                    n64_c[c].is_kb = false;
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
                    n64_c[c].b_state.dButtons = new_state->dButtons;
                    n64_c[c].b_state.x_axis = new_state->x_axis;
                    n64_c[c].b_state.y_axis = new_state->y_axis;
                }
            }
#if (MAX_MICE >= 1)
            else if (input_is_mouse(c))
            {
                n64_buttonmap *new_state = (n64_buttonmap *)n64_response[c];
                input_get_state(c, new_state,  &n64_combo);

                if(n64_c[c].is_mouse == false)
                {
                    n64_c[c].is_kb = false;
                    n64_c[c].is_mouse = true;
                    tft_flag_update();
                }
                n64_c[c].b_state.dButtons = new_state->dButtons;
                n64_c[c].b_state.x_axis = new_state->x_axis;
                n64_c[c].b_state.y_axis = new_state->y_axis;
            }
#endif
#if (MAX_KB >= 1)
            else if (input_is_kb(c))
            {
                n64_randnet_kb *new_state = (n64_randnet_kb *)n64_response[c];
                //Maintain the old led state
                new_state->led_state = n64_c[c].kb_state.led_state;

                input_get_state(c, new_state,  &n64_combo);

                if (n64_c[c].is_kb == false)
                {
                    n64_c[c].is_mouse = false;
                    n64_c[c].is_kb = true;
                    tft_flag_update();
                }
                memcpy(&n64_c[c].kb_state, new_state, sizeof(n64_randnet_kb));
            }
#endif
        }

        //Get a copy of the latest n64 button presses to handle the below combos
        uint16_t n64_buttons = 0;
        if (input_is_gamecontroller(c))
        {
            n64_buttonmap *new_state = (n64_buttonmap *)n64_response[c];
            n64_buttons = new_state->dButtons;
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
        n64_is_on = digitalRead(N64_CONSOLE_SENSE);
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

        //Measure Teensy temp and if it has changed, flag a TFT update
        static int32_t teensy_temp = 0;
        if (abs((int32_t)tempmonGetTemp() - teensy_temp) > 2)
        {
            teensy_temp = (int32_t)tempmonGetTemp();
            tft_flag_update();
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
            if (n64_c[c].current_peripheral == PERI_NONE)
                break; //Already changing peripheral

            timer_peri_change[c] = millis();

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
                    n64_c[c].tpak->gbcart->ram = NULL; //RAM not free'd intentionally
                    n64_c[c].tpak->gbcart->rom = NULL;
                }
            }

            if (n64_c[c].rpak != NULL)
            {
                if (n64_c[c].rpak->state != RUMBLE_APPLIED)
                {
                    input_apply_rumble(c, 0x00);
                    n64_c[c].rpak->state = RUMBLE_APPLIED;
                }
            }

            /* HANDLE NEXT PERIPHERAL */
            n64_c[c].current_peripheral = PERI_NONE; //Go to none whilst changing
            tft_force_update();

            //Changing peripheral to RUMBLEPAK
            if (n64_buttons & N64_LB)
            {
                n64_c[c].next_peripheral = PERI_RUMBLE;
                debug_print_status("[MAIN] C%u to rpak\n", c);
            }

            //Changing peripheral to TPAK
            if (n64_buttons & N64_RB)
            {
                n64_c[c].next_peripheral = PERI_TPAK;
                debug_print_status("[MAIN] C%u to tpak\n", c);

                gameboycart *gb_cart = n64_c[c].tpak->gbcart;
                uint8_t gb_header[0x100];

                strcpy(gb_cart->filename, settings->default_tpak_rom[c]);
                if (gb_cart->filename[0] != '\0' && memory_get_ext_ram_size() > 0)
                {
                    fileio_read_from_file(gb_cart->filename, 0x100, gb_header, sizeof(gb_header));
                    gb_init_cart(gb_cart, gb_header, settings->default_tpak_rom[c]);

                    if (gb_cart->romsize > 0)
                    {
                        gb_cart->rom = memory_alloc_ram(n64_c[c].tpak->gbcart->filename, gb_cart->romsize, READ_ONLY);
                    }

                    if (gb_cart->ramsize > 0)
                    {
                        //Readback savefile from Flash, replace .gb or .gbc with save file extension
                        char save_filename[MAX_FILENAME_LEN];
                        strcpy(save_filename, n64_c[c].tpak->gbcart->filename);
                        strcpy(strrchr(save_filename, '.'), GAMEBOY_SAVE_EXT);
                                                                                         /*WRITE ONLY IF CART HAS BATTERY*/
                        gb_cart->ram = memory_alloc_ram(save_filename, gb_cart->ramsize, gb_has_battery(gb_cart->mbc) == 0);
                    }

                    if (gb_cart->rom == NULL || (gb_cart->ram == NULL && gb_cart->ramsize > 0))
                    {
                        n64_c[c].next_peripheral = PERI_RUMBLE; //Error, just set to rumblepak
                        debug_print_error("[MAIN] ERROR: Could not allocate rom or ram buffer for %s\n", n64_c[c].tpak->gbcart->filename);
                        n64_c[c].tpak->gbcart->romsize = 0;
                        n64_c[c].tpak->gbcart->ramsize = 0;
                        n64_c[c].tpak->gbcart->ram = NULL;
                        if (gb_cart->rom !=NULL) memory_free_item(gb_cart->rom);
                    }
                }
                else
                {
                    n64_c[c].next_peripheral = PERI_RUMBLE; //Error, just set to rumblepak
                    if (gb_cart->filename[0] == '\0')
                        debug_print_error("[MAIN] ERROR: No default TPAK ROM set or no ROMs found\n");
                    else if (memory_get_ext_ram_size() == 0)
                        debug_print_error("[MAIN] ERROR: No external RAM installed. TPAK disabled\n");
                    else
                        debug_print_error("[MAIN] ERROR: Could not read %s\n", gb_cart->filename);
                }
            }

            //Changing peripheral to MEMPAK
            if ((n64_buttons & N64_DU || n64_buttons & N64_DD ||
                 n64_buttons & N64_DL || n64_buttons & N64_DR ||
                 n64_buttons & N64_ST))
            {
                n64_c[c].next_peripheral = PERI_MEMPAK;

                //Allocate mempack based on combo if available
                uint32_t mempak_bank = 0;
                uint16_t b = n64_buttons;
                (b & N64_DU) ? mempak_bank = 0 : (0);
                (b & N64_DR) ? mempak_bank = 1 : (0);
                (b & N64_DD) ? mempak_bank = 2 : (0);
                (b & N64_DL) ? mempak_bank = 3 : (0);
                (b & N64_ST) ? mempak_bank = VIRTUAL_PAK : (0);

                //Create the filename
                char filename[32];
                snprintf(filename, sizeof(filename), "MEMPAK%02u%s", mempak_bank, MEMPAK_SAVE_EXT);

                //Scan controllers to see if mempack is in use
                for (uint32_t i = 0; i < MAX_CONTROLLERS; i++)
                {
                    if (n64_c[i].mempack->id == mempak_bank && mempak_bank != VIRTUAL_PAK)
                    {
                        debug_print_status("[MAIN] WARNING: mpak in use by C%u. Setting to rpak\n", i);
                        n64_c[c].next_peripheral = PERI_RUMBLE;
                        break;
                    }
                }

                //Mempack wasn't in use, so allocate it in ram
                if (n64_c[c].next_peripheral != PERI_RUMBLE && mempak_bank != VIRTUAL_PAK)
                {
                    n64_c[c].mempack->data = memory_alloc_ram(filename, MEMPAK_SIZE, READ_WRITE);
                }

                if (n64_c[c].mempack->data != NULL)
                {
                    debug_print_status("[MAIN] C%u to mpak %u\n", c, mempak_bank);
                    n64_c[c].mempack->virtual_is_active = 0;
                    n64_c[c].mempack->id = mempak_bank;
                }
                else if (mempak_bank == VIRTUAL_PAK)
                {
                    debug_print_status("[MAIN] C%u to virtual pak\n", c);
                    n64_virtualpak_init(n64_c[c].mempack);
                }
                else
                {
                    debug_print_error("[MAIN] ERROR: Could not alloc RAM for %s, setting to rpak\n", filename);
                    n64_c[c].next_peripheral = PERI_RUMBLE;
                }
            }
        }

        //Simulate a peripheral change time. The peripheral goes to NONE
        //for a short period. Some games need this.
        if (n64_c[c].current_peripheral == PERI_NONE && (millis() - timer_peri_change[c]) > PERI_CHANGE_TIME)
        {
            n64_c[c].current_peripheral = n64_c[c].next_peripheral;
            tft_flag_update();
        }

        //Update the virtual pak if required
        if (n64_c[c].mempack->virtual_update_req == 1)
        {
            //For the USB64-INFO1 Page, I write the controller info (PID,VID etc)
            char msg[256];
            n64_virtualpak_update(n64_c[c].mempack); //Update so we get the right page
            uint8_t c_page = n64_virtualpak_get_controller_page();
            sprintf(msg, "%u:0x%04x/0x%04x\n%.15s\n%.15s\n",
                        c_page + 1,
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
    tft_add_log(character);
    ring_buffer_pos++;
    if (ring_buffer_pos >= sizeof(ring_buffer))
        ring_buffer_pos = 0;
    ring_buffer_flush();
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
