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

#include <stdint.h>
#include <string.h>
#include "stdio.h"

#include "n64_mempak.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_controller.h"
#include "n64_wrapper.h"

static uint16_t _tpak_get_mbc_address(uint16_t tpakAddress, uint8_t bank)
{
    return ((tpakAddress & 0xFFE0) - 0xC000) + ((bank & 3) * 0x4000);
}

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
        debug_print_error("ERROR: Unknown ROM size\n");
        break;
    }
    return rom_len;
}

static uint32_t _gb_gb_get_rom_size(uint8_t sram_type, uint8_t mbc_type)
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

static void gb_write_cart(uint16_t mbc_address, n64_transferpak *tpak, uint8_t *inBuffer)
{
    gameboycart *cart = tpak->gbcart;
    uint8_t mbc = _gb_get_mbc_number(cart->mbc);
    uint16_t addr = mbc_address;
    uint8_t val = inBuffer[0];
    switch (addr >> 12)
    {
    case 0x0:
    case 0x1:
        if (mbc == 2 && addr & 0x10)
            return;
        else if (mbc > 0 && cart->ram)
            tpak->ram_enabled = ((val & 0x0F) == 0x0A);
        return;

    case 0x2:
        if (mbc == 5)
        {
            tpak->current_rom_bank = (tpak->current_rom_bank & 0x100) | val;
            return;
        }
        /* Intentional fall through. */
    case 0x3:
        if (mbc == 1)
        {
            //selected_rom_bank = val & 0x7;
            tpak->current_rom_bank = (val & 0x1F) | (tpak->current_rom_bank & 0x60);

            if ((tpak->current_rom_bank & 0x1F) == 0x00)
                tpak->current_rom_bank++;
        }
        else if (mbc == 2 && addr & 0x10)
        {
            tpak->current_rom_bank = val & 0x0F;

            if (!tpak->current_rom_bank)
                tpak->current_rom_bank++;
        }
        else if (mbc == 3)
        {
            tpak->current_rom_bank = val & 0x7F;

            if (!tpak->current_rom_bank)
                tpak->current_rom_bank++;
        }
        else if (mbc == 5)
            tpak->current_rom_bank = (val & 0x01) << 8 | (tpak->current_rom_bank & 0xFF);

        return;

    case 0x4:
    case 0x5:
        if (mbc == 1)
        {
            tpak->current_ram_bank = (val & 3);
            tpak->current_rom_bank = ((val & 3) << 5) | (tpak->current_rom_bank & 0x1F);
        }
        else if (mbc == 3)
            tpak->current_ram_bank = val;
        else if (mbc == 5)
            tpak->current_ram_bank = (val & 0x0F);

        return;

    case 0x6:
    case 0x7:
        tpak->banking_mode = (val & 1);
        return;

    case 0xA:
    case 0xB:
        if (cart->ram && tpak->ram_enabled)
        {
            if (mbc == 3 && tpak->current_ram_bank >= 0x08)
            {
                //gb->cart_rtc[tpak->current_ram_bank - 0x08] = val;
                debug_print_n64("RTC controler write\n");
            }
            else if (tpak->banking_mode)
            {
                uint16_t add = mbc_address - 0xA000 + (tpak->current_ram_bank * 0x2000);
                n64hal_sram_write(inBuffer, cart->ram, add, 32);
                cart->dirty = 1;
            }
            else if (cart->ramsize > 0)
            {
                n64hal_sram_write(inBuffer, cart->ram, mbc_address - 0xA000, 32);
            }
        }

        return;
    }
}

static void gb_read_cart(uint16_t mbc_address, n64_transferpak *tpak, uint8_t *outBuffer)
{
    gameboycart *cart = tpak->gbcart;
    uint8_t mbc = _gb_get_mbc_number(cart->mbc);
    uint16_t addr = mbc_address;
    switch (addr >> 12)
    {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
        n64hal_rom_fastread(cart, addr, outBuffer, 32);
        return;

    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
        if (mbc == 1 && tpak->banking_mode)
        {
            addr = (mbc_address - 0x4000) + (tpak->current_rom_bank * 0x4000);
            n64hal_rom_fastread(cart, addr, outBuffer, 32);
            return;
        }
        else
        {
            addr = (mbc_address - 0x4000) + ((tpak->current_rom_bank - 1) * 0x4000);
            n64hal_rom_fastread(cart, addr, outBuffer, 32);
            return;
        }

    case 0xA:
    case 0xB:
        if (cart->ramsize > 0 && tpak->ram_enabled)
        {
            if (mbc == 3 && tpak->current_ram_bank >= 0x08)
            {
                //return gb->cart_rtc[tpak->current_ram_bank - 0x08];
                return;
            }
            else if (tpak->banking_mode || mbc != 1)
            {
                addr = mbc_address - 0xA000 + (tpak->current_ram_bank * 0x2000);
                return n64hal_sram_read(outBuffer, cart->ram, addr, 32);
            }
            else
                addr = mbc_address - 0xA000;
                return n64hal_sram_read(outBuffer, cart->ram, addr, 32);
        }
    }
}

void tpak_write(n64_transferpak *tp, uint16_t raw_peri_address, uint8_t *data)
{
    uint16_t mbc_address = _tpak_get_mbc_address(raw_peri_address, tp->current_mbc_bank);
    gb_write_cart(mbc_address, tp, data);
}

void tpak_read(n64_transferpak *tp, uint16_t raw_peri_address, uint8_t *data)
{
    uint16_t mbc_address = _tpak_get_mbc_address(raw_peri_address, tp->current_mbc_bank);
    gb_read_cart(mbc_address, tp, data);
}

void tpak_reset(n64_transferpak *tpak)
{
    tpak->access_state = 0;
    tpak->access_state_changed = 0;
    tpak->banking_mode = 0;
    tpak->current_mbc_bank = 0;
    tpak->current_ram_bank = 0;
    tpak->current_rom_bank = 1;
    tpak->power_state = 0;
    tpak->ram_enabled = 0;
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

    if (memcmp(GB_HEADER_LOGO, &gb_header[GB_LOGO_OFFSET - 0x100], 0x18) == 0)
    {
        memset(cart->title, '\0', 15);
        memcpy(cart->title, &gb_header[GB_TITLE_OFFSET - 0x100], 15);
        cart->mbc = gb_header[GB_MBCTYPE_OFFSET - 0x100];
        cart->romsize = _gb_get_rom_size(gb_header[GB_ROMSIZE_OFFSET - 0x100]);
        cart->ramsize = _gb_gb_get_rom_size(gb_header[GB_RAMSIZE_OFFSET - 0x100], cart->mbc);
        memcpy(cart->filename, filename, sizeof(cart->filename));
#if (0)
        debug_print_n64("GB Name: %.15s\r\n", cart->title);
        debug_print_n64("ROM Bytes: %lu\r\n", cart->romsize);
        debug_print_n64("SRAM Bytes: %lu\r\n", cart->ramsize);
        debug_print_n64("MBC Type: 0x%02x\r\n", cart->mbc);
#endif
    }
    else
    {
        debug_print_n64("GB header not valid\r\n");
    }
}

//To do?
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
    n64hal_sram_read(&baseDay,  cart->ram, 0x2044, 1);
    n64hal_sram_read(&baseHour, cart->ram, 0x2045, 1);
    n64hal_sram_read(&baseMin,  cart->ram, 0x2046, 1);
    n64hal_sram_read(&baseSec,  cart->ram, 0x2047, 1);

    debug_print_n64("Base d:%u h:%u m:%u s:%u\r\n",baseDay,baseHour,baseMin,baseSec);

    uint16_t d;
    uint8_t h, m, s;
    n64hal_rtc_read(&d, &h, &m, &s);

    debug_print_n64("Actual d:%u h:%u m:%u s:%u\r\n",d,h,m,s);
    */
}
