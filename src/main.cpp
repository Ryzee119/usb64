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

#define serial_port Serial
n64_controller n64_c[MAX_CONTROLLERS];
gameboycart gb_cart[MAX_CONTROLLERS];

//USB Host Interface
USBHost usbh;
JoystickController joy1(usbh);
JoystickController joy2(usbh);
JoystickController joy3(usbh);
JoystickController joy4(usbh);
JoystickController *gamecontroller[] = {&joy1, &joy2, &joy3, &joy4};

void _putchar(char character)
{
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

static int scan_files(const char *path, uint8_t print)
{
    FRESULT res; DIR dir; UINT num_files = 0;
    static FILINFO fno;
    res = f_opendir(&dir, path);
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
                break;
            if (!(fno.fattrib & AM_DIR))
            {
                num_files++;
                if (print)
                    printf("%s\n", fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return num_files;
}

void setup()
{
    serial_port.begin(500000);
    while(!serial_port);
    usbh.begin();

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
    NVIC_SET_PRIORITY(IRQ_GPIO6789, 1);
    attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_1_PIN), n64_controller1_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_2_PIN), n64_controller2_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_3_PIN), n64_controller3_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_4_PIN), n64_controller4_clock_edge, FALLING);

    extern FATFS fs; BYTE work[4096];
    //Check that the flash chip is formatted for FAT access
    MKFS_PARM defopt = {FM_FAT, 1, 0, 0, 4096};
    qspi_init(NULL, NULL);
    if (f_mount(&fs, "", 1) != FR_OK)
    {
        printf("Error mounting, probably not format correctly\r\n");
        f_mkfs("", &defopt, work, sizeof(work));
    }
}

static bool n64_combo = false;
void loop()
{
    static uint32_t usb_buttons[MAX_CONTROLLERS] = {0};
    static uint16_t n64_buttons[MAX_CONTROLLERS] = {0};
    static int32_t axis[MAX_CONTROLLERS][6] = {0};
    for (int c = 0; c < MAX_CONTROLLERS; c++)
    {
        //If a change is buttons or axis has been detected
        if (gamecontroller[c]->available())
        {
            usb_buttons[c] = gamecontroller[c]->getButtons();
            for (uint8_t i = 0; i < (sizeof(axis[c]) / sizeof(axis[c][0])); i++)
            {
                axis[c][i] = gamecontroller[c]->getAxis(i);
            }
            gamecontroller[c]->joystickDataClear();
        }

        //Map usb controllers to n64 controller
        n64_buttons[c] = 0x0000;
        switch (gamecontroller[c]->joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            //Digital usb_buttons
            if (usb_buttons[c] & (1 << 0))  n64_buttons[c] |= N64_DU; //DUP
            if (usb_buttons[c] & (1 << 1))  n64_buttons[c] |= N64_DD; //DDOWN
            if (usb_buttons[c] & (1 << 2))  n64_buttons[c] |= N64_DL; //DLEFT
            if (usb_buttons[c] & (1 << 3))  n64_buttons[c] |= N64_DR; //DRIGHT
            if (usb_buttons[c] & (1 << 4))  n64_buttons[c] |= N64_ST; //START
            if (usb_buttons[c] & (1 << 5))  n64_buttons[c] |= 0; //BACK
            if (usb_buttons[c] & (1 << 6))  n64_buttons[c] |= 0; //LS
            if (usb_buttons[c] & (1 << 7))  n64_buttons[c] |= 0; //RS
            if (usb_buttons[c] & (1 << 8))  n64_buttons[c] |= N64_LB; //LB
            if (usb_buttons[c] & (1 << 9))  n64_buttons[c] |= N64_RB; //RB
            if (usb_buttons[c] & (1 << 10)) n64_buttons[c] |= 0; //XBOX BUTTON
            if (usb_buttons[c] & (1 << 11)) n64_buttons[c] |= 0; //XBOX SYNC
            if (usb_buttons[c] & (1 << 12)) n64_buttons[c] |= N64_A; //A
            if (usb_buttons[c] & (1 << 13)) n64_buttons[c] |= N64_B; //B
            if (usb_buttons[c] & (1 << 14)) n64_buttons[c] |= N64_B; //X
            if (usb_buttons[c] & (1 << 15)) n64_buttons[c] |= 0; //Y
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

            n64_combo = (usb_buttons[c] & (1 << 5)); //back

            break;

        default:
            break;
        }

        //Apply digital buttons to controller
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
        static uint32_t timer_peripheral_change = 0;
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

            //Changing peripheral from MEMPAK
            if (n64_c[c].current_peripheral == PERI_MEMPCK &&
                n64_c[c].mempack->dirty &&
                n64_c[c].mempack->data != NULL &&
                n64_c[c].mempack->id != VIRTUAL_PAK)
            {
                uint8_t filename[32];
                snprintf((char *)filename, sizeof(filename), "MEMPAK%02u.MPK", n64_c[c].mempack->id);
                n64_c[c].current_peripheral = NONE;
                n64hal_sram_backup_to_file(filename,
                                            n64_c[c].mempack->data,
                                            MEMPAK_SIZE);
                n64_c[c].mempack->dirty = 0;
            }

            //Changing peripheral from VIRTUAL PAK
            if (n64_c[c].current_peripheral == PERI_MEMPCK &&
                n64_c[c].mempack->dirty &&
                n64_c[c].mempack->id == VIRTUAL_PAK)
            {
                n64_c[c].current_peripheral = NONE;
                n64_settings_write();
                n64_c[c].mempack->dirty = 0;
            }

            //Changing peripheral from TPAK
            if (n64_c[c].current_peripheral == PERI_TPAK)
            {
                //Do we need to backup ram?
                if (n64_c[c].tpak->installedCart != NULL && n64_c[c].tpak->installedCart->dirty)
                {
                    uint8_t mbc = n64_c[c].tpak->installedCart->mbc;
                    if (mbc == ROM_RAM_BAT      || mbc == ROM_RAM_BAT  ||
                        mbc == MBC1_RAM_BAT     || mbc == MBC2_BAT     ||
                        mbc == MBC3_RAM_BAT     || mbc == MBC3_TIM_BAT ||
                        mbc == MBC3_TIM_RAM_BAT || mbc == MBC4_RAM_BAT ||
                        mbc == MBC5_RAM_BAT     || mbc == MBC5_RUM_RAM_BAT)
                    {
                        n64_c[c].current_peripheral = NONE;
                        //Replace .gb or .gbc with .sav
                        char* file_name = (char*)n64_c[c].tpak->installedCart->filename;
                        char* new_filename = (char *)malloc(sizeof(file_name) + 5);
                        strcpy(new_filename, file_name);
                        char* ext = strrchr(new_filename, '.');
                        strcpy(ext, ".sav");
                        printf("Cart has RAM, backing up!\n");
                        printf("filename: %s\n", new_filename);
                        printf("ramsize, %u\n", n64_c[c].tpak->installedCart->ramsize);
                        n64hal_sram_backup_to_file((uint8_t*)new_filename,
                                                   n64_c[c].tpak->installedCart->ram,
                                                   n64_c[c].tpak->installedCart->ramsize);
                        free(new_filename);
                    }
                }

                //Clean up
                if (n64_c[c].tpak->installedCart != NULL)
                {
                    n64_c[c].tpak->installedCart->romsize = 0;
                    n64_c[c].tpak->installedCart->ramsize = 0;
                    if (n64_c[c].tpak->installedCart->ram)
                        free(n64_c[c].tpak->installedCart->ram);
                }

                //Now it's a TPAK with no cart installed
                n64_c[c].tpak->installedCart = NULL;

            }

            //Changing peripheral to RUMBLEPAK
            if (n64_buttons[c] & N64_LB)
            {
                n64_c[c].current_peripheral = PERI_NONE;
                n64_c[c].next_peripheral = PERI_RUMBLE;
                timer_peripheral_change = millis();
                printf("Changing controller %u's peripheral to rumblepak\r\n", c);
            }

            //Changing peripheral to TPAK
            if (n64_buttons[c] & N64_RB)
            {
                n64_c[c].current_peripheral = PERI_NONE;
                n64_c[c].next_peripheral = PERI_TPAK;
                timer_peripheral_change = millis();
                printf("Changing controller %u's peripheral to tpak\r\n", c);

                n64_settings* settings = n64_settings_get();
                //Find a free gb_cart object
                int cart = 0;
                for (; cart < MAX_CONTROLLERS; cart++)
                {
                    if (gb_cart[cart].romsize == 0)
                        break;
                }
                //If a free gb_cart has been found
                if(cart < MAX_CONTROLLERS)
                {
                    uint8_t gb_header[0x100];
                    //Read the ROM header and init the gb_cart struct
                    strcpy((char*)gb_cart[cart].filename, settings->default_tpak_rom[c]);
                    if(n64hal_rom_read(&gb_cart[cart], 0x100, gb_header, sizeof(gb_header)))
                    {
                        gb_initGameBoyCart(&gb_cart[cart],
                                           gb_header,
                                           settings->default_tpak_rom[c]);
                                if (gb_cart[cart].ramsize > 0)
                                    gb_cart[cart].ram = (uint8_t*)malloc(gb_cart[cart].ramsize);
                                else
                                    gb_cart[cart].ram = NULL;
                        n64_c[c].tpak->installedCart = &gb_cart[cart];

                        //TODO READBACK RAM FROM FLASH
                    }
                    else
                    {
                        printf("ERROR: Could not read %s\n", gb_cart[cart].filename);
                        n64_c[c].tpak->installedCart = NULL;
                    }
                }
                else
                {
                    printf("ERROR: No free gb_cart objects found\n");
                }
                tpak_reset(n64_c[c].tpak);
            }

            //Changing peripheral to MEMPAK
            if (n64_buttons[c] & N64_DU || n64_buttons[c] & N64_DD ||
                n64_buttons[c] & N64_DL || n64_buttons[c] & N64_DR ||
                n64_buttons[c] & N64_ST)
            {

                n64_c[c].current_peripheral = PERI_NONE;
                n64_c[c].mempack->id = VIRTUAL_PAK;

                if (n64_c[c].mempack->data != NULL)
                {
                    //Data should already be backed up at this point
                    free(n64_c[c].mempack->data);
                    n64_c[c].mempack->data = NULL;
                }

                //Allocate mempack based on combo if available
                int8_t mempak_bank = 0;
                uint16_t dpad = n64_buttons[c];
                (dpad & N64_DU) ? mempak_bank = 0 : (0);
                (dpad & N64_DR) ? mempak_bank = 1 : (0);
                (dpad & N64_DD) ? mempak_bank = 2 : (0);
                (dpad & N64_DL) ? mempak_bank = 3 : (0);
                (dpad & N64_ST) ? mempak_bank = 4 : (0);

                //Scan controllers to see if mempack is in use
                for (int i = 0; i < MAX_CONTROLLERS; i++)
                {
                    if (n64_c[i].mempack->id == mempak_bank && n64_c[i].mempack->id != VIRTUAL_PAK)
                    {
                        printf("Mempak already in use by controller setting to rumble %u\r\n", i);
                        n64_c[c].next_peripheral = PERI_RUMBLE;
                        mempak_bank = -1;
                    }
                    else
                    {
                        //Mempack is free, let's go!
                        break;
                    }
                }
                if (mempak_bank != -1)
                {
                    printf("Setting mempak bank %u to controller %u\n", mempak_bank, c);
                    n64_c[c].mempack->id = mempak_bank;
                    if (mempak_bank != VIRTUAL_PAK)
                    {
                        n64_c[c].mempack->data = (uint8_t *)malloc(MEMPAK_SIZE);
                        n64_c[c].mempack->virtual_is_active = 0;

                        uint8_t filename[32];
                        snprintf((char *)filename, sizeof(filename), "MEMPAK%02u.MPK", n64_c[c].mempack->id);
                        n64hal_sram_restore_from_file(filename,
                                                      n64_c[c].mempack->data,
                                                      MEMPAK_SIZE);
                    }
                    else
                    {
                        n64_virtualpak_init(n64_c[c].mempack);
                    }
                    n64_c[c].next_peripheral = PERI_MEMPCK;
                }
                timer_peripheral_change = millis();
            }
        }

        if (n64_c[c].current_peripheral == PERI_NONE &&
            (millis() - timer_peripheral_change) > 750)
        {
            n64_c[c].current_peripheral = n64_c[c].next_peripheral;
        }

        if (n64_c[c].mempack->virtual_update_req == 1)
        {
            n64_virtualpak_update(n64_c[c].mempack);
        }

    } //FOR LOOP

    if (serial_port.available())
    {
        noInterrupts();
        static char serial_buff[256];
        static char filename[256];
        static BYTE work[4096];
        static MKFS_PARM defopt = {FM_FAT, 1, 0, 0, 4096}; /* Default parameter */
        static FATFS *_fs = (FATFS *)malloc(sizeof(FATFS));
        static FIL fil; static FRESULT res; static UINT br;
        static String serial_buff_str;
        static uint32_t sector_size, total_sectors, free_sectors = 0, num_files;
        int len = 0;
        uint8_t c = serial_port.read();

        switch (c)
        {
        /* 0xA0: Sends a welcome string */
        case 0xA0:
            sprintf(serial_buff, "usb64 by Ryzee119 Build date: %s\n", __DATE__);
            serial_port.write(serial_buff);
            break;
        /* 0xA1: Send a list of files present on the FATFS system memory */
        case 0xA1:
            num_files = scan_files("", 0);
            snprintf(serial_buff, sizeof(serial_buff), "A1,%u\n", num_files);
            serial_port.write(serial_buff);
            scan_files("", 1);
            printf("Sent %u files\n", num_files);
            break;
        /* 0xA2: Download a file from the FATFS system to the host */
        case 0xA2:
            printf("download\n");
            break;
        /* 0xA3: Upload a file from the host and write to the FATFS system */
        case 0xA3:
            memset(filename,0x00,sizeof(filename));
            serial_port.readBytesUntil('\0', filename, sizeof(filename));
            res = f_open(&fil, (const TCHAR *)filename, FA_WRITE | FA_CREATE_ALWAYS);
            if (res != FR_OK)
            {
                printf("Error opening %s for WRITE\r\n", filename);
                break;
            }
            serial_port.setTimeout(100);
            len = 0;
            do {
                len = serial_port.readBytes(serial_buff, sizeof(serial_buff));
                res = f_write(&fil, serial_buff, len, &br);
                if (res != FR_OK)
                {
                    printf("Error writing %s\r\n", filename);
                    break;
                }
            } while (len > 0);
            f_close(&fil);
            printf("Wrote %s ok\n", filename);
            break;
        /* 0xA4: Delete a file from the FATFS system */
        case 0xA4:
            serial_buff_str = serial_port.readStringUntil('\0');
            serial_buff_str.toCharArray(serial_buff, serial_buff_str.length());
            res = f_unlink(serial_buff);
            if (res != FR_OK)
            {
                printf("Error deleting %s with error: %i\n", serial_buff, res);
                break;
            }
            printf("Delete ok\n");
            break;
        /* 0xA5: Send free space and total space of the FATFS system to the host */
        case 0xA5:
            if (_fs->fs_type == 0)
                f_mount(_fs, "", 1);
            qspi_get_flash_properties(&sector_size, NULL);
            f_getfree("", &free_sectors, &_fs);
            total_sectors = (_fs->n_fatent - 2);
            snprintf(serial_buff, sizeof(serial_buff), "A5,%u,%u\n", total_sectors * sector_size,
                                                                     free_sectors  * sector_size);
            serial_port.write(serial_buff);
            break;
        case 0xA6:
            f_mkfs("", &defopt, work, sizeof(work));
            printf("Formatted\n");
        default:
            printf("Unknown\n");
            break;
        }
        interrupts();
    }
} // MAIN LOOP