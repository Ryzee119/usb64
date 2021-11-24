// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

/* gb_write_cart and gb_read_cart adapted from https://github.com/deltabeard/Peanut-GB/ which is shared
 * under the MIT license
 * All other gamecart info from https://gbdev.gg8.se/wiki/articles/Main_Page
 * Tranferpak emulation is my own RE.
 */

#include "usb64_conf.h"
#include "n64_mempak.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_controller.h"
#include "n64_wrapper.h"

static uint32_t _gb_get_rom_size(uint8_t rom_type)
{
    uint32_t rom_len = 0;
    switch (rom_type)
    {
    case (KB_32):   rom_len = 32768UL;   break;
    case (KB_64):   rom_len = 65536UL;   break;
    case (KB_128):  rom_len = 131072UL;  break;
    case (KB_256):  rom_len = 262144UL;  break;
    case (KB_512):  rom_len = 524288UL;  break;
    case (KB_1024): rom_len = 1048576UL; break;
    case (KB_2048): rom_len = 2097152UL; break;
    case (KB_4096): rom_len = 4194304UL; break;
    case (KB_8192): rom_len = 8388608UL; break;
    case (KB_1152): rom_len = 1179648UL; break;
    case (KB_1280): rom_len = 1310720UL; break;
    case (KB_1536): rom_len = 1572864UL; break;
    default:
        rom_len = 0;
        debug_print_error("[TPAK] ERROR: Unknown ROM size\n");
        break;
    }
    return rom_len;
}

static uint32_t _gb_get_ram_size(uint8_t sram_type, uint8_t mbc_type)
{
    uint32_t sram_len = 0;
    switch (sram_type)
    {
    case (B_2048):   sram_len = 2048;   break;
    case (B_8192):   sram_len = 8192;   break;
    case (B_32768):  sram_len = 32768;  break;
    case (B_65536):  sram_len = 65536;  break;
    case (B_131072): sram_len = 131072; break;
    default:         sram_len = 0;      break;
    }

    //MBC2 chip contains 512x4 bits of built in RAM.
    if (mbc_type == MBC2 || mbc_type == MBC2_BAT)
    {
        sram_len = 256;
    }
    return sram_len;
}

//Returns a plain integer of the mbc.
static uint8_t _gb_get_mbc_number(uint8_t mbc_type)
{
    switch (mbc_type)
    {
    case MBC1:
    case MBC1_RAM:
    case MBC1_RAM_BAT:
        return 1;
    case MBC2:
    case MBC2_BAT:
        return 2;
    case MBC3:
    case MBC3_RAM:
    case MBC3_RAM_BAT:
    case MBC3_TIM_BAT:
    case MBC3_TIM_RAM_BAT:
        return 3;
    case MBC4:
    case MBC4_RAM:
    case MBC4_RAM_BAT:
        return 4;
    case MBC5:
    case MBC5_RAM:
    case MBC5_RAM_BAT:
    case MBC5_RUM:
    case MBC5_RUM_RAM:
    case MBC5_RUM_RAM_BAT:
        return 5;
    default:
        return 0;
    }
}

static void gb_write_cart(uint16_t addr, gameboycart *gb, uint8_t *inBuffer)
{
    uint8_t mbc = _gb_get_mbc_number(gb->mbc);
    uint8_t val = inBuffer[31];
    switch (addr >> 12)
    {
    case 0x0:
    case 0x1:
        if (mbc == 2 && addr & 0x10)
        {
            return;
        }
        else if (mbc > 0 && gb->ramsize > 0)
        {
            gb->enable_cart_ram = ((val & 0x0F) == 0x0A);
            debug_print_tpak("[TPAK] MBC%u - Enable Cart Ram\n", mbc);
        }
        return;

    case 0x2:
        if (mbc == 5)
        {
            //MBC5 lower ROM bank byte is set
            gb->selected_rom_bank = (gb->selected_rom_bank & 0x100) | val;
            gb->selected_rom_bank = gb->selected_rom_bank % gb->num_rom_banks;
            debug_print_tpak("[TPAK] MBC%u - ROM Bank changed to %u/%u\n", mbc,
                             gb->selected_rom_bank,
                             gb->num_rom_banks);
            return;
        }

        /* Intentional fall through. */

    case 0x3:
        if (mbc == 1)
        {
            //selected_rom_bank = val & 0x7;
            gb->selected_rom_bank = (val & 0x1F) | (gb->selected_rom_bank & 0x60);

            if ((gb->selected_rom_bank & 0x1F) == 0x00)
                gb->selected_rom_bank++;
        }
        else if (mbc == 2 && addr & 0x10)
        {
            gb->selected_rom_bank = val & 0x0F;

            if (!gb->selected_rom_bank)
                gb->selected_rom_bank++;
        }
        else if (mbc == 3)
        {
            gb->selected_rom_bank = val & 0x7F;

            if (!gb->selected_rom_bank)
                gb->selected_rom_bank++;
        }
        else if (mbc == 5)
        {
            //MBC5 has a 9th ROM bank bit
            gb->selected_rom_bank = ((val & 0x01) << 8) | (gb->selected_rom_bank & 0xFF);
        }
        gb->selected_rom_bank = gb->selected_rom_bank % gb->num_rom_banks;

        debug_print_tpak("[TPAK] MBC%u - ROM Bank changed to %u/%u\n", mbc,
                         gb->selected_rom_bank,
                         gb->num_rom_banks);
        return;

    case 0x4:
    case 0x5:
        if (mbc == 1)
        {
            gb->selected_ram_bank = (val & 3);
            gb->selected_rom_bank = ((val & 3) << 5) | (gb->selected_rom_bank & 0x1F);
            gb->selected_rom_bank = gb->selected_rom_bank % gb->num_rom_banks;
        }
        else if (mbc == 3)
        {
            gb->selected_ram_bank = val;
        }
        else if (mbc == 5)
        {
            gb->selected_ram_bank = (val & 0x0F);
        }
        debug_print_tpak("[TPAK] MBC%u - RAM Bank changed to %u\n", mbc, gb->selected_ram_bank);
        return;

    case 0x6:
    case 0x7:
        gb->cart_mode_select = (val & 1);
        debug_print_tpak("[TPAK] MBC%u - Cart Mode changed to %u\n", mbc, gb->cart_mode_select);
        return;

    case 0xA:
    case 0xB:
        if (gb->ramsize > 0 && gb->enable_cart_ram)
        {
            if (mbc == 3 && gb->selected_ram_bank >= 0x08)
            {
                gb->rtc[gb->selected_ram_bank - 0x08] = val;
                n64hal_rtc_write(&gb->rtc_bits.high, &gb->rtc_bits.yday,
                                 &gb->rtc_bits.hour, &gb->rtc_bits.min, &gb->rtc_bits.sec);
                debug_print_tpak("[TPAK] MBC%u - RTC Write Reg %02x, Val: %u\n", mbc, gb->selected_ram_bank, val);
            }
            else if (gb->cart_mode_select && gb->selected_ram_bank < gb->num_ram_banks)
            {
                n64hal_write_extram(inBuffer, gb->ram, addr - CART_RAM_ADDR + (gb->selected_ram_bank * CRAM_BANK_SIZE), 32);
            }
            else if (gb->num_ram_banks)
            {
                n64hal_write_extram(inBuffer, gb->ram, addr - CART_RAM_ADDR, 32);
            }
        }
        return;
    }
    return;
}

static void gb_read_cart(uint16_t addr, gameboycart *gb, uint8_t *outBuffer)
{
    uint8_t mbc = _gb_get_mbc_number(gb->mbc);
    switch (addr >> 12)
    {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
        n64hal_read_extram(outBuffer, gb->rom, addr, 32);
        return;

    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
        if (mbc == 1 && gb->cart_mode_select)
        {
            n64hal_read_extram(outBuffer, gb->rom, addr + ((gb->selected_rom_bank & 0x1F) - 1) * ROM_BANK_SIZE, 32);
        }
        else
        {
            n64hal_read_extram(outBuffer, gb->rom, addr + (gb->selected_rom_bank - 1) * ROM_BANK_SIZE, 32);
        }
        return;

    case 0xA:
    case 0xB:
        if (gb->ramsize > 0 && gb->enable_cart_ram)
        {
            if (mbc == 3 && gb->selected_ram_bank >= 0x08)
            {
                n64hal_rtc_read(&gb->rtc_bits.high, &gb->rtc_bits.yday,
                                &gb->rtc_bits.hour, &gb->rtc_bits.min, &gb->rtc_bits.sec);
                memset(outBuffer, gb->rtc[gb->selected_ram_bank - 0x08], 32);
                debug_print_tpak("[TPAK] MBC%u - RTC Read Reg %02x\n", mbc, gb->selected_ram_bank);
            }
            else if ((gb->cart_mode_select || mbc != 1) && gb->selected_ram_bank < gb->num_ram_banks)
            {
                n64hal_read_extram(outBuffer, gb->ram, addr - CART_RAM_ADDR + (gb->selected_ram_bank * CRAM_BANK_SIZE), 32);
            }
            else
            {
                n64hal_read_extram(outBuffer, gb->ram, addr - CART_RAM_ADDR, 32);
            }
        }
        return;
    }
    return;
}

//Get the actual cart MBC address from the address sent to the tpak.
static uint16_t _tpak_get_mbc_address(uint16_t tpakAddress, uint8_t bank)
{
    return ((tpakAddress & 0xFFE0) - 0xC000) + ((bank & 3) * 0x4000);
}

void tpak_write(n64_transferpak *tp, uint16_t raw_address, uint8_t *data)
{
    uint16_t mbc_address = _tpak_get_mbc_address(raw_address, tp->selected_mbc_bank);
    gb_write_cart(mbc_address, tp->gbcart, data);
}

void tpak_read(n64_transferpak *tp, uint16_t raw_address, uint8_t *data)
{
    uint16_t mbc_address = _tpak_get_mbc_address(raw_address, tp->selected_mbc_bank);
    gb_read_cart(mbc_address, tp->gbcart, data);
}

void tpak_reset(n64_transferpak *tpak)
{
    tpak->access_state = 0;
    tpak->access_state_changed = 0;
    tpak->selected_mbc_bank = 0;
    tpak->power_state = 0;
}

void gb_init_cart(gameboycart *cart, uint8_t *gb_header, char *filename)
{
    //Fixed header area in all valid gameboy roms.
    const uint8_t GB_HEADER_LOGO[] = {0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
                                      0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
                                      0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
                                      0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
                                      0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
                                      0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E};

    if (memcmp(GB_HEADER_LOGO, &gb_header[GB_LOGO_OFFSET - 0x100], 0x18) != 0)
    {
        debug_print_error("[TPAK] ERROR: gb_init_cart: GB header not valid\n");
        return;
    }

    memset(cart->title, '\0', 15);
    memcpy(cart->title, &gb_header[GB_TITLE_OFFSET - 0x100], 15);
    cart->mbc = gb_header[GB_MBCTYPE_OFFSET - 0x100];
    cart->romsize = _gb_get_rom_size(gb_header[GB_ROMSIZE_OFFSET - 0x100]);
    cart->ramsize = _gb_get_ram_size(gb_header[GB_RAMSIZE_OFFSET - 0x100], cart->mbc);
    cart->selected_rom_bank = 1;
    cart->selected_ram_bank = 0;
    cart->enable_cart_ram = 0;
    cart->cart_mode_select = 0;
    cart->num_ram_banks = cart->ramsize / CRAM_BANK_SIZE;
    cart->num_rom_banks = cart->romsize / ROM_BANK_SIZE;

    memcpy(cart->filename, filename, sizeof(cart->filename));
    debug_print_tpak("[TPAK] gb_init_cart: GB Name: %.15s\n", cart->title);
    debug_print_tpak("[TPAK] gb_init_cart: ROM Bytes: %lu\n", cart->romsize);
    debug_print_tpak("[TPAK] gb_init_cart: SRAM Bytes: %lu\n", cart->ramsize);
    debug_print_tpak("[TPAK] gb_init_cart: MBC Type: 0x%02x\n", cart->mbc);
}

uint8_t gb_has_battery(uint8_t mbc_type)
{
    switch (mbc_type)
    {
    case MBC1_RAM_BAT:
    case MBC2_BAT:
    case MBC3_RAM_BAT:
    case MBC3_TIM_BAT:
    case MBC3_TIM_RAM_BAT:
    case MBC4_RAM_BAT:
    case MBC5_RAM_BAT:
    case MBC5_RUM_RAM_BAT:
        return 1;
    }
    return 0;
}

//TODO:?
//Kept my RE notes here for future ref
void gb_set_pokemon_time(gameboycart *cart)
{
    /* Pokemon game time works by storing a base time in the saved game located from address 0x2044 to 0x2047
     * The base time is set at the beginning of a new game when prompted for the Day, Hour and Minute by\
     * Prof. Oak/Mum and it never changes as long as the saved game exists.
     * When you load the saved game, the cart reads the RTC registers then adds the RTC offset to the base time to determine
     * the actual game time. After startup the gameboy internal clocks area used to track time when the power is on.
     *
     * The Base Time which is set when you start a new game by the prompts from Prof Oak are stored in the below SRAM Offsets:
     * 0x2044 = Day  (Values can be 0x00 to 0x06 = Sunday to Saturday)
     * 0x2045 = Hour (Values can be 0x00 to 0x18 = 1AM to 12PM)
     * 0x2046 = Minutes (Values can be 0x00 to 0x3B = 0 to 59 Minutes)
     * 0x2047 = Seconds (Values can be 0x00 to 0x3B = 0 to 59 Minutes)
     *
     * Address 0x2049 to 0x204C contain the actual game time after the Gameboy adds the RTC offset to the base time.
     * This updates every time you save, modifying them doesn't do anything as its just written over by the
     * Gameboy/Emulator when you save.
     *
     * As the Gameboy only reads the base time and the RTC registers on startup there is no way to change the time mid game
     * even if you hack new values into the SRAM or RTC. The game only reads them at startup!
     * A real RTC will keep running in the background even when powered down, so the next time you turn the game on it reads
     * the RTC registers and recalculate the actual game time to have time pass even when the game is off.
     * Emulators often save the RTC registers and reloads them next time you start the game to resume it left off next time
     * you start playing.
     *
     * So the only way to modify the day/time in the game is to do it before the game starts.
     * It will then load in the modified values and won't know any different!
     * To prevent having to modify the actual save file which probably isn't a good idea generally,
     * you can also modify the RTC registers which are added to the base time values by the Gameboy itself.
     * This is what this function does!
     *
     */
    /*
    uint8_t baseDay, baseHour, baseMin, baseSec;
    memcpy(&baseDay,  cart->ram + 0x2044, 1);
    memcpy(&baseHour, cart->ram + 0x2045, 1);
    memcpy(&baseMin,  cart->ram + 0x2046, 1);
    memcpy(&baseSec,  cart->ram + 0x2047, 1);

    debug_print_tpak("[TPAK] Base d:%u h:%u m:%u s:%u\n",baseDay,baseHour,baseMin,baseSec);

    uint16_t d;
    uint8_t h, m, s;
    n64hal_rtc_read(&d, &h, &m, &s);

    debug_print_tpak("[TPAK] Actual d:%u h:%u m:%u s:%u\n",d,h,m,s);
    */
}
