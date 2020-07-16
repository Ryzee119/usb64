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

#ifndef _N64_TRANSFERPAK_GBCARTS_h
#define _N64_TRANSFERPAK_GBCARTS_h

#ifdef __cplusplus
extern "C" {
#endif

//GAMEBOY
//Header offsets - ref https://web.archive.org/web/20141105020940/http://problemkaputt.de/pandocs.htm
#define GB_LOGO_OFFSET 0x0104    //48 bytes
#define GB_TITLE_OFFSET 0x0134   //16 bytes
#define GB_MANUF_OFFSET 0x013F   //4 bytes
#define GB_MBCTYPE_OFFSET 0x0147 //1 byte
#define GB_ROMSIZE_OFFSET 0x0148 //1 byte
#define GB_RAMSIZE_OFFSET 0x0149 //1 byte

//Cart Types
#define ROM_ONLY 0x00
#define ROM_RAM 0x08
#define ROM_RAM_BAT 0x09
#define MBC1 0x01
#define MBC1_RAM 0x02
#define MBC1_RAM_BAT 0x03
#define MBC2 0x05
#define MBC2_BAT 0x06
#define MBC3 0x11
#define MBC3_RAM 0x12
#define MBC3_RAM_BAT 0x13
#define MBC3_TIM_BAT 0x0F
#define MBC3_TIM_RAM_BAT 0x10
#define MBC4 0x15
#define MBC4_RAM 0x16
#define MBC4_RAM_BAT 0x17
#define MBC5 0x19
#define MBC5_RAM 0x1A
#define MBC5_RAM_BAT 0x1B
#define MBC5_RUM 0x1C
#define MBC5_RUM_RAM 0x1D
#define MBC5_RUM_RAM_BAT 0x1E

//ROM Sizes
#define KB_32 0x00
#define KB_64 0x01
#define KB_128 0x02
#define KB_256 0x03
#define KB_512 0x04
#define KB_1024 0x05
#define KB_2048 0x06
#define KB_4096 0x07
#define KB_8192 0x08
#define KB_1152 0x52
#define KB_1280 0x53
#define KB_1536 0x54

//RAM Sizes
#define NONE 0x00 //If MBC2, 00 includes a built-in RAM of 512 x 4 bits.
#define B_2048 0x01
#define B_8192 0x02
#define B_32768 0x03
#define B_131072 0x04
#define B_65536 0x05

typedef struct
{
    uint8_t mbc;       //What mbc the cart uses, extracted from the ROM header
    uint8_t title[15]; //The cart game title extracted from the ROM header
    uint32_t romsize;  //The cart rom size extracted from the ROM header
    uint32_t ramsize;  //The cart ram size extracted from the ROM header
    uint8_t *ram;      //Pointer to RAM data (if used)
    uint8_t *rom;      //Pointer to rom data (if used)

    //RTC
    uint8_t rtc_second; //0-59 (0-3Bh)
    uint8_t rtc_minute; //0-59 (0-3Bh)
    uint8_t rtc_hour;   //0-23 (0-17h)
    uint16_t rtc_day;   //0-365. 9 bit register.
    uint8_t rtc_update; //Flag is set when the STM RTC should be updated due to a request from the GB game.

    //FATFS
    uint8_t filename[256]; //Filename of ROM in FATFS flash
    uint8_t dirty;         //If RAM data has changed. So we know if we need to backup to Flash

} gameboycart;

typedef struct
{
    uint8_t powerState;         //Writing 0x84 to 0x8001 turns this on. Writing 0xFE to 0x8001 turns this off
    uint8_t accessState;        //Writing 0x01 to 0xB010 turns this on. Writing 0x00 to 0xB010 turns this off.
    uint8_t accessStateChanged; //This is set if the accessState was just changed.
    uint8_t currentRAMBank;
    uint8_t currentROMBank;
    uint8_t currentMBCBank; //Banks to access the overall MBC space. 0x0000 up to 0x7FFF.
    uint8_t ramEnabled;     //0x00 Disabled, 0x0A Enabled
    uint8_t bankingMode;    //0x00 ROM Banking Mode:
                            //When 0x00, 0x4000 is the RAM Bank Number from 00 to 03),
                            //When 0x01 RAM Banking Mode (The RAM Bank is used as the two upper bits of the ROM Bank)
    gameboycart *gbcart;
    uint8_t activeRomID;
} n64_transferpak;

//Prototypes
void gb_initGameBoyCart(gameboycart *cart, uint8_t *gb_header, char *filename);
uint32_t gb_getRomSize(uint8_t rom_type);
uint32_t gb_getSramSize(uint8_t sram_type, uint8_t mbc_type);

void gb_writeCartROMOnly(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer);
void gb_writeCartMBC1(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer);
void gb_writeCartMBC2(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer);
void gb_writeCartMBC3(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer);
void gb_writeCartMBC4(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer);
void gb_writeCartMBC5(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *inBuffer);

void gb_readCartROMOnly(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer);
void gb_readCartMBC1(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer);
void gb_readCartMBC2(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer);
void gb_readCartMBC3(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer);
void gb_readCartMBC4(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer);
void gb_readCartMBC5(uint16_t MBCAddress, n64_transferpak *tpak, uint8_t *outBuffer);

uint16_t tpak_getMBCAddress(uint16_t tpakAddress, uint8_t bank);
void tpak_reset(n64_transferpak *tpak);

#ifdef __cplusplus
}
#endif

#endif
