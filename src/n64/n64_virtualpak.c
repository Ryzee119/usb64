// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "printf.h"
#include "usb64_conf.h"
#include "n64_mempak.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_controller.h"
#include "n64_wrapper.h"

#define HEADING MENU_LINE1
#define SUBHEADING MENU_LINE2

#define MENU_TPAK MENU_LINE4
#define MENU_CONTROLLER_SETTINGS MENU_LINE5
#define MENU_INFO0 MENU_LINE6
#define MENU_INFO1 MENU_LINE7

#define CHANGE_CONTROLLER MENU_LINE15
#define MENU_MAIN MENU_LINE16
#define RETURN MENU_MAIN

static uint32_t controller_page = 0;
static uint32_t current_menu = MENU_MAIN;
static char buff[64];
static uint32_t num_roms = 0;
static char *gbrom_filenames[MAX_GBROMS] = {NULL};  //Gameboy ROM file name list
static char *gbrom_titlenames[MAX_GBROMS] = {NULL}; //Gameboy ROM cart title list

char info_text_0[256];
char info_text_1[256];

//First 32 bytes of mempak. First byte must be 0x81. This small section is in RAM as the console writes to it
uint8_t n64_virtualpak_scratch[0x20] = {
    0x81, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};

//First 0x300 bytes of mempak. I took this from a mempak I generated on my n64.
//const as the console only needs to read from this area
const uint8_t n64_virtualpak_header[0x300] = {
    0x81, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0x1A, 0x5F, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0x66, 0x25, 0x99, 0xCD,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0x1A, 0x5F, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0x66, 0x25, 0x99, 0xCD,
    0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0x1A, 0x5F, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0x66, 0x25, 0x99, 0xCD,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0x1A, 0x5F, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0x66, 0x25, 0x99, 0xCD,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x51, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x51, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03};

//This is the notesTable located at address 0x300 to 0x500 in the mempack address space.
//This initialises the title blocks as all blank titles using 1 page each.
//Basically what is displayed on the mempak manager page.
/* 0x4E = N (Media Type Cartridge)
 * 0x41 = R Game Name Code of sorts. ZL = Zelda, PO = Pokemon Snap.
 * 0x43 = Y RY =  Ryan :)
 * 0x45 = E Region "North America"
 * 0x35 = 5 Publisher Code
 * 0x48 = H Publisher Code
 * 0x00 = Always 0x00?
 * 0x05 = Between 5 and 127, Page Index
 * 0x02 = ?
 * 0x03 = ?
 */
uint8_t n64_virtualpak_note_table[0x200] = {
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x05, 0x02, 0x03, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x06, 0x02, 0x03, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x07, 0x02, 0x03, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x08, 0x02, 0x03, 0x00, 0x00, 0x1D, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x09, 0x02, 0x03, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x0A, 0x02, 0x03, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x0B, 0x02, 0x03, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x0C, 0x02, 0x03, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x0D, 0x02, 0x03, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x0E, 0x02, 0x03, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x0F, 0x02, 0x03, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x10, 0x02, 0x03, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x11, 0x02, 0x01, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x12, 0x02, 0x01, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x13, 0x02, 0x01, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D,
    0x4E, 0x41, 0x43, 0x45, 0x35, 0x48, 0x00, 0x14, 0x02, 0x01, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00,
    0x1A, 0x2B, 0x26, 0x32, 0x26, 0x1E, 0x27, 0x1A, 0x22, 0x2B, 0x1C, 0x28, 0x26, 0x1B, 0x1A, 0x2D};

/*
 * Function: Converts and writes an ASCII string to the mempak note table
 * ----------------------------
 *   Returns void
 *
 *   msg: Pointer to a null terminated ASCII string
 *   line: What line to write the string in the mempak menu [0 to 15]
 *   ext: Setting to 1 will write the string to the extension section of each title. (4 chars wide)
 */
static void n64_virtualpak_write_string(char *msg, uint8_t line, uint8_t ext)
{
    //Obtained from pulling known save titles and a bit of trial and error
    static const uint8_t MEMPACK_CHARMAP[] =
        {
            '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            '!', '"', '#', '\'', '*', '+', ',', '-', '.', '/', ':', '=', '?', '@'
        };

    uint32_t max_len;
    uint32_t len = 255;

    (ext == 1) ? (max_len = 4) : (max_len = 16);

    for (uint32_t i = 0; i < max_len; i++)
    {
        uint8_t n64char = 0;

        //If string terminator, fix length
        if (msg[i] == '\0')
            len = i;

        //Force upper case
        if (msg[i] > 96)
            msg[i] -= 32;

        //Handle some unique cases
        if (msg[i] == '_')
            msg[i] = '-'; //replace _ with -

        //Find a match in the CHARMAP
        for (uint32_t j = 0; j < sizeof(MEMPACK_CHARMAP); j++)
        {
            if (msg[i] == MEMPACK_CHARMAP[j])
                n64char = j;
        }

        //Pad with spaces if past end or the character is unsupported
        if (i > len || n64char == 0)
            n64char = 15;

        //Sanity check this
        if (line > 15)
            line = 15;

        //Write the character to the note table
        if (ext)
            n64_virtualpak_note_table[(32 * line) + 12 + i] = n64char;
        else
            n64_virtualpak_note_table[(32 * line) + 16 + i] = n64char;
    }
}

void n64_virtualpak_init(n64_mempack *vpak)
{
    vpak->virtual_is_active = 1;
    vpak->virtual_selected_row = MENU_MAIN;
    current_menu = MENU_MAIN;

    //Clear up any previous memory allocations
    for (int i = 0; i < num_roms; i++)
    {
        if (gbrom_filenames[i] != NULL)
            free(gbrom_filenames[i]);
        if (gbrom_titlenames[i] != NULL)
            free(gbrom_titlenames[i]);

        gbrom_filenames[i] = NULL;
        gbrom_titlenames[i] = NULL;
    }

    num_roms = n64hal_list_gb_roms(gbrom_filenames, MAX_GBROMS);

    //For each ROM, extract the ROM info and rom title
    for (int i = 0; i < num_roms; i++)
    {
        uint8_t gb_header[0x100];
        gameboycart gb_cart;

        n64hal_unbuffered_read(gbrom_filenames[i], 0x100, gb_header, sizeof(gb_header));
        gb_init_cart(&gb_cart, gb_header, gbrom_filenames[i]);

        //Copy the gb cart title (from the rom header into an array)
        gbrom_titlenames[i] = (char *)malloc(strlen(gb_cart.title) + 1);
        strcpy(gbrom_titlenames[i], gb_cart.title);
    }
    n64_virtualpak_update(vpak);
}

void n64_virtualpak_read32(uint16_t address, uint8_t *rx_buff)
{
    if (address < 0x20)
        memcpy(rx_buff, &n64_virtualpak_scratch[address], 32);
    else if (address < 0x300)
        memcpy(rx_buff, &n64_virtualpak_header[address], 32);
    else if (address < 0x500)
        memcpy(rx_buff, &n64_virtualpak_note_table[address - 0x300], 32);
    else
        memset(rx_buff, 0x00, 32);
}

void n64_virtualpak_write32(uint16_t address, uint8_t *tx_buff)
{
    //If address is in scratch space, write it.
    //Ignore other writes. Dont actually want the N64 to write over stuff
    //If this scratch space doesnt get written, n64 assumes corrupt mempak.
    if (address < 0x20)
        memcpy(&n64_virtualpak_scratch[address], tx_buff, 32);
}

void n64_virtualpak_update(n64_mempack *vpak)
{
    n64_settings *settings = n64_settings_get();
    if (settings == NULL)
    {
        return;
    }
    //Clear the screen;
    char alpha[2] = {'A', '\0'};
    for (uint32_t i = 0; i < 15; i++)
    {
        n64_virtualpak_write_string("-", i, MENU_NAME_FIELD);
        n64_virtualpak_write_string(alpha, i, MENU_EXT_FIELD);
        alpha[0]++;
    }

    //Handle generic header and footer options
    switch (vpak->virtual_selected_row)
    {
    case CHANGE_CONTROLLER:
    case SUBHEADING:
        controller_page++;
        if (controller_page >= MAX_CONTROLLERS)
            controller_page = 0;
        break;
    case HEADING:
    case RETURN:
        current_menu = MENU_MAIN;
        break;
    }

    //Print generic headers and footers
    n64_virtualpak_write_string("USB64 - RYZEE119", HEADING, MENU_NAME_FIELD);
    sprintf(buff, "CONTROLLER %u", controller_page + 1);
    n64_virtualpak_write_string(buff, SUBHEADING, MENU_NAME_FIELD);
    n64_virtualpak_write_string("________________", SUBHEADING + 1, MENU_NAME_FIELD);
    n64_virtualpak_write_string("CHANGE CONT", CHANGE_CONTROLLER, MENU_NAME_FIELD);
    n64_virtualpak_write_string("RETURN", RETURN, MENU_NAME_FIELD);

    /* Print the required menu and handle actions specific to each menu */
    if (current_menu == MENU_MAIN)
    {
        switch (vpak->virtual_selected_row)
        {
        case MENU_TPAK:
            current_menu = MENU_TPAK;
            break;
        case MENU_CONTROLLER_SETTINGS:
            current_menu = MENU_CONTROLLER_SETTINGS;
            break;
        case MENU_INFO0:
            current_menu = MENU_INFO0;
            break;
        case MENU_INFO1:
            current_menu = MENU_INFO1;
            break;
        default:
            n64_virtualpak_write_string("TPAK SETTINGS", MENU_TPAK, MENU_NAME_FIELD);
            n64_virtualpak_write_string("CONT SETTINGS", MENU_CONTROLLER_SETTINGS, MENU_NAME_FIELD);
            n64_virtualpak_write_string("USB64 INFO1", MENU_INFO0, MENU_NAME_FIELD);
            n64_virtualpak_write_string("USB64 INFO2", MENU_INFO1, MENU_NAME_FIELD);
            break;
        }
        vpak->virtual_selected_row = -1;
    }

    if (current_menu == MENU_TPAK)
    {
        n64_virtualpak_write_string("TPAK SETTINGS", SUBHEADING + 2, MENU_NAME_FIELD);
        //A row has been selected
        uint32_t selected_row = vpak->virtual_selected_row;
        if (selected_row != -1)
        {
            //...which happens to be a ROM so change the default ROM
            uint32_t selected_rom = selected_row - (SUBHEADING + 2);
            if (selected_rom < num_roms)
            {
                strcpy(settings->default_tpak_rom[controller_page],
                       gbrom_filenames[selected_rom]);
                n64_settings_update_checksum(settings);
            }
        }

        //Print a * next to the selected ROM.
        for (uint32_t i = 0; i < num_roms; i++)
        {
            n64_virtualpak_write_string(gbrom_titlenames[i], SUBHEADING + 2 + i, MENU_NAME_FIELD);
            //This ROM matches default!
            if (strcmp(gbrom_filenames[i], settings->default_tpak_rom[controller_page]) == 0)
                n64_virtualpak_write_string("****", SUBHEADING + 2 + i, MENU_EXT_FIELD);
        }
        vpak->virtual_selected_row = -1;
    }

    if (current_menu == MENU_CONTROLLER_SETTINGS)
    {
        sprintf(buff, "CONTROLLER %u", controller_page + 1);
        n64_virtualpak_write_string(buff, SUBHEADING + 0, MENU_NAME_FIELD);
        n64_virtualpak_write_string("________________", SUBHEADING + 1, MENU_NAME_FIELD);
        n64_virtualpak_write_string("CONT SETTINGS", SUBHEADING + 2, MENU_NAME_FIELD);

        n64_virtualpak_write_string("SENSITIVITY+", SUBHEADING + 4, MENU_NAME_FIELD);
        n64_virtualpak_write_string("SENSITIVITY-", SUBHEADING + 5, MENU_NAME_FIELD);

        n64_virtualpak_write_string("DEADZONE+", SUBHEADING + 7, MENU_NAME_FIELD);
        n64_virtualpak_write_string("DEADZONE-", SUBHEADING + 8, MENU_NAME_FIELD);

        n64_virtualpak_write_string("SNAP TOGGLE", SUBHEADING + 10, MENU_NAME_FIELD);

        n64_virtualpak_write_string("RESTORE DEFAULT", SUBHEADING + 12, MENU_NAME_FIELD);

        //A row has been selected, adjust settings accordingly
        uint32_t selected_row = vpak->virtual_selected_row;
        if (selected_row != -1)
        {
            if (selected_row == SUBHEADING + 4 && settings->sensitivity[controller_page] < 4)
                settings->sensitivity[controller_page]++;
            if (selected_row == SUBHEADING + 5 && settings->sensitivity[controller_page] > 0)
                settings->sensitivity[controller_page]--;
            if (selected_row == SUBHEADING + 7 && settings->deadzone[controller_page] < 4)
                settings->deadzone[controller_page]++;
            if (selected_row == SUBHEADING + 8 && settings->deadzone[controller_page] > 0)
                settings->deadzone[controller_page]--;
            if (selected_row == SUBHEADING + 10)
                settings->snap_axis[controller_page] ^= 1;
            if (selected_row == SUBHEADING + 12)
            {
                settings->deadzone[controller_page] = DEFAULT_DEADZONE;
                settings->sensitivity[controller_page] = DEFAULT_SENSITIVITY;
                settings->snap_axis[controller_page] = DEFAULT_SNAP;
            }
            n64_settings_update_checksum(settings);
        }

        //Print the current values of each setting
        sprintf(buff, "%03u\0", settings->sensitivity[controller_page]);
        n64_virtualpak_write_string(buff, SUBHEADING + 4, MENU_EXT_FIELD);

        sprintf(buff, "%03u\0", settings->deadzone[controller_page]);
        n64_virtualpak_write_string(buff, SUBHEADING + 7, MENU_EXT_FIELD);

        sprintf(buff, "%03u\0", settings->snap_axis[controller_page]);
        n64_virtualpak_write_string(buff, SUBHEADING + 10, MENU_EXT_FIELD);

        vpak->virtual_selected_row = -1;
    }

    if (current_menu == MENU_INFO0 || current_menu == MENU_INFO1)
    {
        n64_virtualpak_write_string("INFO PAGE", SUBHEADING + 2, MENU_NAME_FIELD);
        char *msg = NULL;
        char line_text[17] = {0};
        uint32_t pos = 0;
        uint32_t line = SUBHEADING + 3;

        if (current_menu == MENU_INFO0)
            msg = info_text_0;
        if (current_menu == MENU_INFO1)
            msg = info_text_0;

        while (*msg && msg != NULL && pos < sizeof(info_text_0))
        {
            //On a line break or end of line print that line and prep to print on next line
            if ((*msg == '\n' || pos == 16) && line < 15)
            {
                n64_virtualpak_write_string(line_text, line, MENU_NAME_FIELD);
                memset(line_text, 0x00, sizeof(line_text));
                line += 1;
                pos = 0;
            }
            else if (pos < 16)
            {
                line_text[pos++] = *msg;
            }
            msg++;
        }
        vpak->virtual_selected_row = -1;
    }

    vpak->virtual_update_req = 0;
}

void n64_virtualpak_write_info_1(char *msg)
{
    strncpy(info_text_0, msg, sizeof(info_text_0));
}

void n64_virtualpak_write_info_2(char *msg)
{
    strncpy(info_text_1, msg, sizeof(info_text_1));
}

uint8_t n64_virtualpak_get_controller_page()
{
    return controller_page;
}