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
#include "usb64_conf.h"
#include "n64_controller.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "tusb.h"

typedef struct
{
    char name[MAX_FILENAME_LEN];
    uint8_t *data;
    int len;
    int non_volatile;
} sram_storage;

n64_controller n64_c[MAX_CONTROLLERS];
n64_settings* settings;

//USB Host Interface
USBHost usbh;
#if (MAX_CONTROLLERS >= 1)
JoystickController joy1(usbh);
#endif
#if (MAX_CONTROLLERS >= 2)
JoystickController joy2(usbh);
#endif
#if (MAX_CONTROLLERS >= 3)
JoystickController joy3(usbh);
#endif
#if (MAX_CONTROLLERS >= 4)
JoystickController joy4(usbh);
#endif

#if MAX_CONTROLLERS == 1
JoystickController *gamecontroller[] = {&joy1};
#elif MAX_CONTROLLERS == 2
JoystickController *gamecontroller[] = {&joy1, &joy2};
#elif MAX_CONTROLLERS == 3
JoystickController *gamecontroller[] = {&joy1, &joy2, &joy3};
#elif MAX_CONTROLLERS == 4
JoystickController *gamecontroller[] = {&joy1, &joy2, &joy3, &joy4};
#endif

static void apply_deadzone(float* out_x, float* out_y, float x, float y, float dz_low, float dz_high) {
    float magnitude = sqrtf(x * x + y * y);
    if (magnitude > dz_low) {
        //Scale such that output magnitude is in the range [0.0f, 1.0f]
        float allowed_range = 1.0f - dz_high - dz_low;
        float normalised_magnitude = (magnitude - dz_low) / allowed_range;
        if (normalised_magnitude > 1.0f)
            normalised_magnitude = 1.0f;
        float scale = normalised_magnitude / magnitude;
        *out_x = x * scale;
        *out_y = y * scale;
    }
    else {
        //Stick is in the inner dead zone
        *out_x = 0.0f;
        *out_y = 0.0f;
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
        if (strcmp(sram[i].name, (const char *)name) == 0)
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
            if (sram[i].data == NULL) break;
            sram[i].len = alloc_len;
            strcpy(sram[i].name, name);
            if(non_volatile)
            {
                n64hal_sram_restore_from_file((uint8_t *)sram[i].name,
                                            sram[i].data,
                                            sram[i].len);
                sram[i].non_volatile = 1;
            }
            else
            {
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
    for (unsigned int i = 0; i < sizeof(sram) / sizeof(sram[0]); i++)
    {
        if(sram[i].len == 0 || sram[i].data == NULL || sram[i].non_volatile == 0)
            continue;
        n64hal_sram_backup_to_file((uint8_t*)sram[i].name, sram[i].data, sram[i].len);
    }
}


//TinyUSB interrupt handler (Used for CDC and MSC)
static void tusbd_interrupt(void)
{
    tud_int_handler(0);
}

//Ring buffer is used to buffer printf outputs to the terminal
//Printing to the serial port is handles in main()
static uint32_t ring_buffer_pos = 0;
static char ring_buffer[4096];
void _putchar(char character)
{
    ring_buffer[ring_buffer_pos++] = character;
    if (ring_buffer_pos >= sizeof(ring_buffer))
        ring_buffer_pos = 0;
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

void setup()
{
    //Init the serial port and ring buffer
    serial_port.begin(115200);
    memset(ring_buffer,0xFF,sizeof(ring_buffer));

    //Check that the flash chip is formatted for FAT access
    //If it's not, format it! Should only happen once
    extern FATFS fs; 
    qspi_init(NULL, NULL);
    if (f_mount(&fs, "", 1) != FR_OK)
    {
        debug_print_error("ERROR: Could not mount FATFS, probably not formatted correctly. Formatting flash...\n");
        MKFS_PARM defopt = {FM_FAT, 1, 0, 0, 4096};
        BYTE work[256];
        f_mkfs("", &defopt, work, sizeof(work));
    }

    //If a n64 console isn't detected, start the USB Device MSC driver
    pinMode(N64_CONSOLE_SENSE, INPUT_PULLDOWN);
    delay(N64_CONSOLE_SENSE_DELAY);
    if (digitalRead(N64_CONSOLE_SENSE) == 1)
    {
        attachInterruptVector(IRQ_USB1, &tusbd_interrupt);
        tusb_init();
        while (1)
        {
            tud_task();
            flush_ring_buffer();
        }
    }

    usbh.begin();

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

    NVIC_SET_PRIORITY(IRQ_USB1, 1);
}

static bool n64_combo = false;
void loop()
{
    static uint32_t usb_buttons[MAX_CONTROLLERS] = {0};
    static uint16_t n64_buttons[MAX_CONTROLLERS] = {0};
    static int8_t n64_x_axis[MAX_CONTROLLERS] = {0};
    static int8_t n64_y_axis[MAX_CONTROLLERS] = {0};
    static int32_t axis[MAX_CONTROLLERS][6] = {0};

    flush_ring_buffer();

    for (int c = 0; c < MAX_CONTROLLERS; c++)
    {
        //If a change in buttons or axis has been detected
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
            //Analog stick (Normalise 0 to +/-100)
            n64_x_axis[c] = axis[c][0] * 100 / 32768;
            n64_y_axis[c] = axis[c][1] * 100 / 32768;

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

        /* Apply analog stick options */
        n64_settings *settings = n64_settings_get();
        float x, y, deadzone;

        //Apply Deadzone
        deadzone = settings->deadzone[c] / 10.0f; //(0 to 40%)
        apply_deadzone(&x, &y, n64_x_axis[c] / 100.0f, n64_y_axis[c] / 100.0f, deadzone, 0.05f);

        //Apply Sensitivity
        float range = 100.0f;
        switch (settings->sensitivity[c])
        {
        case 4: range=1.10f; break; // +/-110
        case 3: range=0.95f; break;
        case 2: range=0.85f; break;
        case 1: range=0.75f; break;
        case 0: range=0.65f; break; // +/-65
        }
        x*=range; y*=range;

        //Apply angle snap (some controllers will do this internally so I can't really disable it if they do)
        if (settings->snap_axis[c])
        {
            int angle = atan2f(y, x) * 180.0f / 3.14f;
            float magnitude = sqrtf(powf(x,2) + powf(y,2));
            if (angle < 0)
                angle = 360 + angle;

            //Snap to 45°C segments
            const int snap = SNAP_RANGE; //+/- 10 degrees within a 45 degree angle will snap
            int a = angle;
            while (a > 45) a-=45;
            if ((a <= 0 + snap) || (a >= 45 - snap))
            {
                angle += snap;
                angle -= angle % 45;
                //Only snap if near max range
                if (magnitude >= 0.90f * range)
                {
                    x = magnitude * cosf(angle * 3.14f/180.0f);
                    y = magnitude * sinf(angle * 3.14f/180.0f);
                }
            }
        }

        //Apply Octagonal correct FIXME

        //Apply corrected values to axis variables
        n64_x_axis[c] = x * 100.0f;
        n64_y_axis[c] = y * 100.0f;

        //Apply digital buttons and axis to n64 controller if combo button isnt pressed
        if (n64_combo == 0)
        {
            n64_c[c].bState.dButtons = n64_buttons[c];
            n64_c[c].bState.x_axis = n64_x_axis[c];
            n64_c[c].bState.y_axis = n64_y_axis[c];
        }

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

            /* CLEAR CURRENT PERIPHERALS */
            n64_c[c].mempack->data = NULL;
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
                strcpy((char *)gb_cart->filename, settings->default_tpak_rom[c]);
                if (n64hal_rom_fastread(gb_cart, 0x100, gb_header, sizeof(gb_header)))
                {
                    //Init the gb_cart struct using header info
                    gb_init_cart(gb_cart,
                                       gb_header,
                                       settings->default_tpak_rom[c]);

                    char save_filename[MAX_FILENAME_LEN];
                    if (gb_cart->ramsize > 0)
                    {
                        //Readback savefile from Flash, replace .gb or .gbc with .sav
                        char *file_name = (char *)n64_c[c].tpak->gbcart->filename;
                        strcpy(save_filename, file_name);
                        strcpy(strrchr(save_filename, '.'), ".sav");

                        uint8_t mbc = gb_cart->mbc;
                        uint8_t volatile_flag = 0;
                        //Only the MBC has a battery, set the non volatile flag for the SRAM.
                        if (mbc == ROM_RAM_BAT      || mbc == ROM_RAM_BAT  ||
                            mbc == MBC1_RAM_BAT     || mbc == MBC2_BAT     ||
                            mbc == MBC3_RAM_BAT     || mbc == MBC3_TIM_BAT ||
                            mbc == MBC3_TIM_RAM_BAT || mbc == MBC4_RAM_BAT ||
                            mbc == MBC5_RAM_BAT     || mbc == MBC5_RUM_RAM_BAT)
                        {
                            volatile_flag = 1;
                        }

                        gb_cart->ram = alloc_sram(save_filename, gb_cart->ramsize, volatile_flag);
                        if (gb_cart->ram == NULL)
                            n64_c[c].next_peripheral = PERI_RUMBLE; //Error, set to rumblepak
                    }
                }
                else
                {
                    debug_print_error("ERROR: Could not read %s\n", gb_cart->filename);
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
                uint16_t b = n64_buttons[c];
                (b & N64_DU) ? mempak_bank = 0 : (0);
                (b & N64_DR) ? mempak_bank = 1 : (0);
                (b & N64_DD) ? mempak_bank = 2 : (0);
                (b & N64_DL) ? mempak_bank = 3 : (0);
                (b & N64_ST) ? mempak_bank = VIRTUAL_PAK : (0);
                
                //Create the filename
                uint8_t filename[32];
                snprintf((char *)filename, sizeof(filename), "MEMPAK%02u.MPK", mempak_bank);

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
                    n64_c[c].mempack->data = alloc_sram((const char *)filename, MEMPAK_SIZE, 1);
                }

                if (n64_c[c].mempack->data != NULL)
                {
                    debug_print_status("C%u to mpak %u\n", c, mempak_bank);
                    n64_c[c].mempack->virtual_is_active = 0;
                    n64_c[c].mempack->id = mempak_bank;
                }
                else if (mempak_bank != VIRTUAL_PAK)
                {
                    n64_c[c].next_peripheral = PERI_RUMBLE; //Error, set to rumblepak
                }

                if (mempak_bank == VIRTUAL_PAK)
                {
                    debug_print_status("C%u to virtual pak\n", c);
                    n64_virtualpak_init(n64_c[c].mempack);
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
            //Print the Connected USB devices to the virtual pak for info
            char msg[256];
            const char *nc = "NOT CONNECTED";
            snprintf(msg, sizeof(msg),
            "1:0x%04x/0x%04x\n%.15s\n%.15s\n"
            #if (MAX_CONTROLLERS >= 2)
            "2:0x%04x/0x%04x\n%.15s\n%.15s\n"
            #endif
            #if (MAX_CONTROLLERS >= 3)
            "3:0x%04x/0x%04x\n%.15s\n%.15s\n"
            #endif
            #if (MAX_CONTROLLERS >= 4)
            "4:0x%04x/0x%04x\n%.15s\n%.15s\n"
            #endif
            ,joy1.idVendor(), joy1.idProduct(), (joy1.manufacturer() != NULL) ? (const char*)joy1.manufacturer() : nc,
                                                (joy1.product()      != NULL) ? (const char*)joy1.product()      : " "
            #if (MAX_CONTROLLERS >= 2)
            ,joy2.idVendor(), joy2.idProduct(), (joy2.manufacturer() != NULL) ? (const char*)joy2.manufacturer() : nc,
                                                (joy2.product()      != NULL) ? (const char*)joy2.product()      : " "
            #endif
            #if (MAX_CONTROLLERS >= 3)
            ,joy3.idVendor(), joy3.idProduct(), (joy3.manufacturer() != NULL) ? (const char*)joy3.manufacturer() : nc,
                                                (joy3.product()      != NULL) ? (const char*)joy3.product()      : " "
            #endif
            #if (MAX_CONTROLLERS >= 4)
            ,joy4.idVendor(), joy4.idProduct(), (joy4.manufacturer() != NULL) ? (const char*)joy4.manufacturer() : nc,
                                                (joy4.product()      != NULL) ? (const char*)joy4.product()      : " "
            #endif
            );
            n64_virtualpak_write_info_1(msg);
            n64_virtualpak_update(n64_c[c].mempack);
        }

        //If you pressed the combo to flush sram to flash, handle it here
        static uint8_t flushing[4] = {0};
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
