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
#include "printf.h"

void tpak_reset(n64_transferpak *tpak)
{
    tpak->accessState = 0;
    tpak->accessStateChanged = 0;
    tpak->bankingMode = 0;
    tpak->currentMBCBank = 0;
    tpak->currentRAMBank = 0;
    tpak->currentROMBank = 1;
    tpak->powerState = 0;
    tpak->ramEnabled = 0;
}

uint16_t tpak_getMBCAddress(uint16_t tpakAddress, uint8_t bank)
{
    return ((tpakAddress & 0xFFE0) - 0xC000) + ((bank & 3) * 0x4000);
}

uint32_t gb_getRomSize(uint8_t rom_type)
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
        rom_len = 32768UL;
        printf("Error, unknown ROM size. Assuming 32kB\n");
        break;
    }
    return rom_len;
}

uint32_t gb_getSramSize(uint8_t sram_type, uint8_t mbc_type)
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

void gb_writeCartROMOnly(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer)
{
    printf("gb_writeCartROMOnly not implemented\n");
    //Not expecting to actually write anything here.
}

void gb_readCartROMOnly(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer)
{
    //flash_read(MBCAddress+cart->flash_romoffset, outBuffer,32);
    //printf("gb_readCartROMOnly not implemented\n");
    n64hal_rom_read(tpak->gbcart, MBCAddress, outBuffer, 32);
}

/* MBC1 */
void gb_writeCartMBC1(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer)
{
    printf("gb_writeCartMBC1 not implemented\n");
}

void gb_readCartMBC1(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer)
{
    printf("gb_readCartMBC1 not implemented\n");
}
/* END MBC1 */

/* MBC2 */
void gb_writeCartMBC2(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer)
{
    printf("gb_writeCartMBC2 not implemented\n");
}

void gb_readCartMBC2(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer)
{
    printf("gb_readCartMBC2 not implemented\n");
}
/* END MBC2 */

/* MBC3 */
void gb_writeCartMBC3(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer)
{
    gameboycart *cart = tpak->gbcart;
    //0000-1FFF - RAM and Timer Enable
    if (MBCAddress >= 0x0000 && MBCAddress <= 0x1FFF)
    {
        if (inBuffer[31] == 0x0A)
        { //The 32nd byte is 0x0A to enable ram
            tpak->ramEnabled = 1;
        }
        else if (inBuffer[31] == 0x00)
        {
            tpak->ramEnabled = 0;
        }
        else
        {
            printf("Error: Unknown RAM enable command\n");
        }
    }
    //2000-3FFF - Control ROM Bank Number
    else if (MBCAddress >= 0x2000 && MBCAddress <= 0x3FFF)
    {
        tpak->currentROMBank = inBuffer[0] & 0x7F;
        //A value of 00h, will actually select Bank 01h instead
        if (tpak->currentROMBank == 0)
        {
            tpak->currentROMBank = 1;
        }
    }
    //4000-5FFF - Control RAM Bank Number - or - RTC Register Select
    else if (MBCAddress >= 0x4000 && MBCAddress <= 0x5FFF)
    {
        tpak->currentRAMBank = inBuffer[0];
        //Writing a value in range for 00h-07h maps the corresponding external RAM Bank (if any) into memory at A000-BFFF
        if (inBuffer[0] < 0x08)
        {
            tpak->bankingMode = 0;
        }
        //RTC Banking - When writing a value of 08h-0Ch, this will map the corresponding RTC register into memory at A000-BFFF
        //0x08 to 0x0C for seconds, minutes, hours, day lower 8-bits, day upper (bit 0 = MSB of day counter)
        else
        {
            tpak->bankingMode = inBuffer[0];
        }
    }
    //6000-7FFF - Latch Clock Data
    else if (MBCAddress >= 0x6000 && MBCAddress <= 0x7FFF)
    {
        if (inBuffer[0] == 0x01)
        {
            //printf("Latch RTC Registers "); 
        }
    }
    //0xA000-0xBFFF Cart RAM Write OR RTC Register Write
    else if (MBCAddress >= 0xA000 && MBCAddress <= 0xBFFF)
    {
        //If RAM Bank is 0x08-0x0C, write to RTC registers.
        if (tpak->currentRAMBank >= 0x08 && tpak->currentRAMBank <= 0x0c)
        {
            switch (tpak->bankingMode)
            {
            case 0x08: cart->rtc_second = inBuffer[0]; break;
            case 0x09: cart->rtc_minute = inBuffer[0]; break;
            case 0x0A: cart->rtc_hour = inBuffer[0];   break;
            case 0x0B: cart->rtc_day = inBuffer[0];    break;
            case 0x0C:
                cart->rtc_day &= 0x00FF;
                cart->rtc_day |= ((uint16_t)inBuffer[0]) << 8;
                break;
            default:
                printf("Invalid write RTC bankingMode\n");
                break;
            }
            cart->rtc_update = 1;
        }
        //If cart has RAM has ram write data packet to it
        else if (tpak->ramEnabled == 1)
        {
            uint16_t add = MBCAddress - 0xA000 + (tpak->currentRAMBank * 0x2000);
            n64hal_sram_write(inBuffer, cart->ram, add, 32);
            cart->dirty = 1;
        }
        else
        {
            printf("Error: Could not write RAM. %u\n", tpak->ramEnabled);
        }
    }
    else
    {
        printf("Error: Bad read at MBC address 0x%04x and ROMBank %u\n", MBCAddress,
                                                                         tpak->currentROMBank);
    }
}

void gb_readCartMBC3(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer)
{
    gameboycart *cart = tpak->gbcart;
    uint32_t cartAddress = MBCAddress;
    uint16_t readAddress;

    //0000-3FFF - Cart Flash Access Non Banked Section, Direct access.
    if (MBCAddress >= 0x0000 && MBCAddress <= 0x3FFF)
    {
        cartAddress = MBCAddress;
        n64hal_rom_read(cart, cartAddress, outBuffer, 32);
    }
    //4000-7FFF - Cart Flash Access Banked Section.
    else if (MBCAddress >= 0x4000 && MBCAddress <= 0x7FFF)
    {
        cartAddress = (MBCAddress - 0x4000) + (tpak->currentROMBank * 0x4000);
        n64hal_rom_read(cart, cartAddress, outBuffer, 32);
    }
    //A000 to BFFF - Cart RAM or RTC Register Access
    else if (MBCAddress >= 0xA000 && MBCAddress <= 0xBFFF)
    {
        //RTC Register or external RAM Bank
        /*
            00h  RAM Access
            08h  RTC S  Seconds   0-59 (0-3Bh)
            09h  RTC M  Minutes   0-59 (0-3Bh)
            0Ah  RTC H  Hours     0-23 (0-17h)
            0Bh  RTC DL Lower 8 bits of Day Counter (0-FFh)
            0Ch  RTC DH Upper 1 bit of Day Counter, Carry Bit, Halt Flag
                        Bit 0  Most significant bit of Day Counter (Bit 8)
                        Bit 6  Halt (0=Active, 1=Stop Timer)
                        Bit 7  Day Counter Carry Bit (1=Counter Overflow)
        */
        switch (tpak->bankingMode)
        {
        case 0x00:
            readAddress = MBCAddress - 0xA000 + (tpak->currentRAMBank * 0x2000);
            n64hal_sram_read(outBuffer, cart->ram, readAddress, 32);
            break;
        case 0x08:
            memset(outBuffer, cart->rtc_second, 32);
            break;
        case 0x09:
            memset(outBuffer, cart->rtc_minute, 32);
            break;
        case 0x0A:
            memset(outBuffer, cart->rtc_hour, 32);
            break;
        case 0x0B:
            memset(outBuffer, (uint8_t)(cart->rtc_day), 32);
            break;
        case 0x0C:
            memset(outBuffer, (uint8_t)(cart->rtc_day >> 8), 32);
            break;
        default:
            printf("Invalid Read bankingMode ");
            break;
        }
    }
    else
    {
        printf("Error: Bad read at MBC address 0x%04x and ROMBank %u\n", MBCAddress,
                                                                         tpak->currentROMBank);
    }
}
/* END MBC3 */

/* MBC4 */
void gb_writeCartMBC4(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer)
{
    printf("gb_writeCartMBC4 not implemented\n");
}

void gb_readCartMBC4(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer)
{
    printf("gb_readCartMBC4 not implemented\n");
}
/* END MBC4 */

/* MBC5 */
void gb_writeCartMBC5(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer)
{
    printf("gb_writeCartMBC5 not implemented\n");
}

void gb_readCartMBC5(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer)
{
    printf("gb_readCartMBC5 not implemented\n");
}
/* END MBC5 */

void gb_initGameBoyCart(gameboycart *cart, uint8_t *gb_header, char *filename)
{
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
        cart->romsize = gb_getRomSize(gb_header[GB_ROMSIZE_OFFSET - 0x100]);
        cart->ramsize = gb_getSramSize(gb_header[GB_RAMSIZE_OFFSET - 0x100], cart->mbc);
        memcpy(cart->filename, filename, sizeof(cart->filename));
        #if (0)
        printf("GB Name: %.15s\r\n", (char *)cart->title);
        printf("ROM Bytes: %lu\r\n", cart->romsize);
        printf("SRAM Bytes: %lu\r\n", cart->ramsize);
        printf("MBC Type: 0x%02x\r\n", cart->mbc);
        #endif
    }
    else
    {
        printf("GB header not valid\r\n");
    }
}

void gb_pokemonSetTime(gameboycart *cart)
{
    /* Pokemon game time works by storing a base time in the saved game located from address 0x2044 to 0x2047
     * The base time is set at the beginning of a new game when prompted for the Day, Hour and Minute by Prof. Oak/Mum and it never changes
     * as long as the saved game exists.
     * When you load the saved game, the cart reads the RTC registers then adds the RTC offset to the base time to determine
     * the actual game time. After startup the gameboy internal clocks area used to track time when the power is on.
     *
     * The Base Time which is set when you start a new game by the prompts from Prof Oak are stored in the below SRAM Offsets:
     * 0x2044 = Day  (Values can be 0x00 to 0x06 = Sunday to Saturday)
     * 0x2045 = Hour (Values can be 0x00 to 0x18 = 1AM to 12PM)
     * 0x2046 = Minutes (Values can be 0x00 to 0x3B = 0 to 59 Minutes)
     * 0x2047 = Seconds (Values can be 0x00 to 0x3B = 0 to 59 Minutes)
     *
     * Address 0x2049 to 0x204C contain the actual game time after the Gameboy adds the RTC offset to the base time. This updates every time you save, modifying them
     * doesn't do anything as its just written over by the Gameboy/Emulator when you save.
     *
     * As the Gameboy only reads the base time and the RTC registers on startup there is no way to change the time mid game even if you hack new values
     * into the SRAM or RTC. The game only reads them at startup!
     * A real RTC will keep running in the background even when powered down, so the next time you turn the game on it reads the RTC registers and recalculate
     * the actual game time to have time pass even when the game is off. Emulators often save the RTC registers and reloads them next time you start the
     * game to resume it left off next time you start playing.
     *
     * So the only way to modify the day/time in the game is to do it before the game starts. It will then load in the modified values and won't know
     * any different! To prevent having to modify the actual save file which probably isn't a good idea generally,
     * you can also modify the RTC registers which are added to the base time values by the Gameboy itself! This is what this function does!
     *
     */
    //uint8_t baseDay, baseHour, baseMin, baseSec;
    /*n64hal_sram_read(&baseDay, cart->ram, 0x2044, 1);
    n64hal_sram_read(&baseHour, cart->ram, 0x2045, 1);
    n64hal_sram_read(&baseMin, cart->ram,0x2046, 1);
    n64hal_sram_read(&baseSec, cart->ram, 0x2047, 1);*/

    //printf("Base d:%u h:%u m:%u s:%u\r\n",baseDay,baseHour,baseMin,baseSec);

    //uint16_t d;
    //uint8_t h, m, s;
    //n64hal_rtc_read(&d, &h, &m, &s);

    //printf("Actual d:%u h:%u m:%u s:%u\r\n",d,h,m,s);
}
