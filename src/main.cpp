#include <Arduino.h>
#include "USBHost_t36.h"
#include "ff.h"
#include "qspi.h"
#include "printf.h"
#include "n64_wrapper.h"
#include "n64_conf.h"
#include "n64_controller.h"
#include "n64_virtualpak.h"

#define hwserial Serial
n64_controller n64_c[MAX_CONTROLLERS];

//USB Host Interface
USBHost usbh;
JoystickController joy1(usbh);
JoystickController joy2(usbh);
JoystickController joy3(usbh);
JoystickController joy4(usbh);
JoystickController *gamecontroller[] = {&joy1, &joy2, &joy3, &joy4};

void _putchar(char character)
{
    hwserial.write(character);
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
#if (1)
FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}
#endif

void setup()
{
    hwserial.begin(500000);
    while(!hwserial);
    printf("usb64 by Ryzee119\r\n");
    usbh.begin();
    n64_init_subsystem(n64_c);

    n64_c[0].gpio_pin = N64_CONTROLLER_1_PIN;
    n64_c[1].gpio_pin = N64_CONTROLLER_2_PIN;
    n64_c[2].gpio_pin = N64_CONTROLLER_3_PIN;
    n64_c[3].gpio_pin = N64_CONTROLLER_4_PIN;

    pinMode(N64_CONTROLLER_1_PIN, INPUT_PULLUP);
    pinMode(N64_CONTROLLER_2_PIN, INPUT_PULLUP);
    //pinMode(N64_CONTROLLER_3_PIN, INPUT_PULLUP);
    //pinMode(N64_CONTROLLER_4_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_1_PIN), n64_controller1_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_2_PIN), n64_controller2_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_3_PIN), n64_controller3_clock_edge, FALLING);
    //attachInterrupt(digitalPinToInterrupt(N64_CONTROLLER_4_PIN), n64_controller4_clock_edge, FALLING);

#if (0) //RAW FLASH ACCESS TESTING
    qspi_init(NULL, NULL);
    uint8_t test_buff[32768] = {0};
    uint32_t k = sizeof(test_buff);
    //qspi_erase_chip();
    qspi_erase(0, k);
    printf("qspi_erased\r\n");
    qspi_read(0, k, test_buff);
    for (uint32_t i = 0; i < k; i++)
    {
        if (test_buff[i] != 0xFF)
            printf("%02x ", test_buff[i]);
    }
    printf("qspi_read\r\n");
    memset(test_buff, 0xCC, k);
    qspi_write(0, k, test_buff);
    memset(test_buff, 0x00, k);
    qspi_read(0, k, test_buff);
    for (uint32_t i = 0; i < k; i++)
    {
        if (test_buff[i] != 0xCC)
            printf("%02x ", test_buff[i]);
    }
    printf("Done\r\n");
    while (1)
        ;
#endif

    //qspi_erase_chip();
    
    FATFS fs; FIL fil; UINT bw; FRESULT res;
    //Check that the flash chip is formatted for FAT acess
    BYTE work[4096];
    MKFS_PARM defopt = {FM_FAT, 1, 1, 1, 4096}; /* Default parameter */
    qspi_init(NULL, NULL);
    if (f_mount(&fs, "", 1) != FR_OK)
    {
        printf("Error mounting, probably not format correctly\r\n");
        res = f_mkfs("", &defopt, work, sizeof(work));
    }
    f_mount(0, "", 0);

#if (0) //ERASE AND FORMAT
    //qspi_erase_chip();
    printf("f_mkfs\r\n");
    if (res != FR_OK)
    {
        printf("Error f_mkfs\r\n");
        while (1)
            ;
    }
#endif
#if (0) //MOUNT AND WRITE/READ FILE
    res = f_mount(&fs, "", 0);
    if (res != FR_OK)
    {
        printf("Error f_mount\r\n");
        while (1)
            ;
    }
    res = f_open(&fil, "hello.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        printf("Error f_open\r\n");
        while (1)
        {
            uint8_t buf[32];
            qspi_read(0, 32, buf);
            delay(1);
        }
    }
    f_write(&fil, "Hello, World!\r\n", 15, &bw);
    if (bw != 15)
    {
        printf("Error f_write\r\n");
        while (1)
            ;
    }
    f_close(&fil);
    f_mount(0, "", 0);
#endif
#if (1) //SCAN AND PRINT FILE LIST
    char buff[256];
    f_mount(&fs, "", 0);
    res = scan_files(buff);
    f_mount(0, "", 0);
#endif
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
        switch (gamecontroller[c]->joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            //Digital usb_buttons
            if (usb_buttons[c] & (1 << 0))  n64_c[c].bState.dButtons |= N64_DU; //DUP
            if (usb_buttons[c] & (1 << 1))  n64_c[c].bState.dButtons |= N64_DD; //DDOWN
            if (usb_buttons[c] & (1 << 2))  n64_c[c].bState.dButtons |= N64_DL; //DLEFT
            if (usb_buttons[c] & (1 << 3))  n64_c[c].bState.dButtons |= N64_DR; //DRIGHT
            if (usb_buttons[c] & (1 << 4))  n64_c[c].bState.dButtons |= N64_ST; //START
            if (usb_buttons[c] & (1 << 5))  n64_c[c].bState.dButtons |= 0; //BACK
            if (usb_buttons[c] & (1 << 6))  n64_c[c].bState.dButtons |= 0; //LS
            if (usb_buttons[c] & (1 << 7))  n64_c[c].bState.dButtons |= 0; //RS
            if (usb_buttons[c] & (1 << 8))  n64_c[c].bState.dButtons |= N64_LB; //LB
            if (usb_buttons[c] & (1 << 9))  n64_c[c].bState.dButtons |= N64_RB; //RB
            if (usb_buttons[c] & (1 << 10)) n64_c[c].bState.dButtons |= 0; //XBOX BUTTON
            if (usb_buttons[c] & (1 << 11)) n64_c[c].bState.dButtons |= 0; //XBOX SYNC
            if (usb_buttons[c] & (1 << 12)) n64_c[c].bState.dButtons |= N64_A; //A
            if (usb_buttons[c] & (1 << 13)) n64_c[c].bState.dButtons |= N64_B; //B
            if (usb_buttons[c] & (1 << 14)) n64_c[c].bState.dButtons |= N64_B; //X
            if (usb_buttons[c] & (1 << 15)) n64_c[c].bState.dButtons |= 0; //Y
            if (usb_buttons[c] & (1 << 7))  n64_c[c].bState.dButtons |= N64_CU | //RS triggers
                                                                        N64_CD | //all C usb_buttons
                                                                        N64_CL |
                                                                        N64_CR;
            //Analog stick
            n64_c[c].bState.x_axis = axis[c][0] * 85 / 32768;
            n64_c[c].bState.y_axis = axis[c][1] * 85 / 32768;

            //Z button
            if (axis[c][4] > 10) n64_c[c].bState.dButtons |= N64_Z; //LT
            if (axis[c][5] > 10) n64_c[c].bState.dButtons |= N64_Z; //RT

            //C usb_buttons
            if (axis[c][2] > 16000)  n64_c[c].bState.dButtons |= N64_CR;
            if (axis[c][2] < -16000) n64_c[c].bState.dButtons |= N64_CL;
            if (axis[c][3] > 16000)  n64_c[c].bState.dButtons |= N64_CU;
            if (axis[c][3] < -16000) n64_c[c].bState.dButtons |= N64_CD;

            n64_combo = (usb_buttons[c] & (1 << 5));

            break;

        default:
            break;
        }
        //Copy the current state of the n64 buttons into another variable for later use.
        n64_buttons[c] = n64_c[c].bState.dButtons;

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

            //Changing peripheral, backup mempack RAM to Flash if required
            if (n64_c[c].current_peripheral == PERI_MEMPCK && n64_c[c].mempack->dirty &&
                n64_c[c].mempack->data != NULL)
            {
                uint8_t filename[32];
                snprintf((char *)filename, sizeof(filename), "MEMPAK%02u.MPK", n64_c[c].mempack->id);
                uint32_t t = micros();
                n64_c[c].current_peripheral = NONE;
                n64hal_backup_sram_to_flash(filename,
                                            n64_c[c].mempack->data,
                                            MEMPAK_SIZE);
                n64_c[c].mempack->dirty = 0;
                printf("Time taken to backup %s to flash : %u us ", filename, micros() - t);
            }

            //Changing peripheral, backup gameboy cart RAM to Flash if required
            if (n64_c[c].current_peripheral == PERI_TPAK)
            {
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
                        n64hal_backup_sram_to_flash(n64_c[c].tpak->installedCart->filename,
                                                    n64_c[c].tpak->installedCart->ram,
                                                    n64_c[c].tpak->installedCart->ramsize);
                    }
                }
            }

            //You selected rumblepak
            if (n64_buttons[c] & N64_LB)
            {
                n64_c[c].current_peripheral = PERI_NONE;
                n64_c[c].next_peripheral = PERI_RUMBLE;
                timer_peripheral_change = millis();
                printf("Changing controller %u's peripheral to rumblepak\r\n", c);
            }

            if (n64_buttons[c] & N64_RB)
            {
                n64_c[c].current_peripheral = PERI_NONE;
                n64_c[c].next_peripheral = PERI_TPAK;
                timer_peripheral_change = millis();
                printf("Changing controller %u's peripheral to tpak\r\n", c);
            }

            //You selected mempack
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
                    printf("Setting mempak bank %u to controller %u ", mempak_bank, c);
                    uint32_t t = micros();
                    n64_c[c].mempack->id = mempak_bank;
                    if (mempak_bank != VIRTUAL_PAK)
                    {
                        n64_c[c].mempack->data = (uint8_t *)malloc(MEMPAK_SIZE);
                        n64_c[c].mempack->virtual_is_active = 0;

                        uint8_t filename[32];
                        snprintf((char *)filename, sizeof(filename), "MEMPAK%02u.MPK", n64_c[c].mempack->id);
                        n64hal_read_sram_from_flash(filename,
                                                    n64_c[c].mempack->data,
                                                    MEMPAK_SIZE);
                    }
                    else
                    {
                        n64_c[c].mempack->virtual_is_active = 1;
                        n64_c[c].mempack->virtual_selected_row = -1;
                        n64_c[c].mempack->virtual_update_req = 1;
                    }
                    printf("[Read time: %u us]\r\n", micros() - t);
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
            //n64_virtualpak_update(n64_c[c].mempack);
        }

    } //FOR LOOP
} // MAIN LOOP