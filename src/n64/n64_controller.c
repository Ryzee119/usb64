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
#include <stdio.h>
#include <string.h>
#include "usb64_conf.h"
#include "n64_mempak.h"
#include "n64_virtualpak.h"
#include "n64_rumblepak.h"
#include "n64_settings.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_controller.h"
#include "n64_wrapper.h"

//Uncomment to enable mempak read address CRC checks. I have implement it
//for mempak writes already. Reads are low risk but impact  speed alot so may
//cause issues
//#define USE_N64_ADDRESS_CRC

n64_rumblepak n64_rpak[MAX_CONTROLLERS];
n64_mempack n64_mpack[MAX_CONTROLLERS];
n64_transferpak n64_tpak[MAX_CONTROLLERS];
gameboycart gb_cart[MAX_CONTROLLERS];

void n64_subsystem_init(n64_controller *controllers)
{
    // INITIALISE THE N64 STRUCTS //
    for (uint32_t i = 0; i < MAX_CONTROLLERS; i++)
    {
        controllers[i].id = i;
        controllers[i].current_bit = 7;
        controllers[i].current_byte = 0;
        controllers[i].current_peripheral = PERI_RUMBLE;
        controllers[i].next_peripheral = controllers[i].current_peripheral;
        controllers[i].rpak = &n64_rpak[i];
        controllers[i].mempack = &n64_mpack[i];
        controllers[i].mempack->id = VIRTUAL_PAK;
        controllers[i].mempack->data = NULL;
        controllers[i].tpak = &n64_tpak[i];
        controllers[i].tpak->gbcart = &gb_cart[i];
    }

    //Setup the Controller pin IO mapping and interrupts
    n64hal_hs_tick_init();
}

static uint8_t n64_get_crc(uint8_t *data)
{
    //Generated from http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
    //N64 CRC poly is x^7 + x^2 + x^0 (0x85), initial value = 0
    static uint8_t crctable[256] = {
        0x00, 0x85, 0x8F, 0x0A, 0x9B, 0x1E, 0x14, 0x91, 0xB3, 0x36, 0x3C,
        0xB9, 0x28, 0xAD, 0xA7, 0x22, 0xE3, 0x66, 0x6C, 0xE9, 0x78, 0xFD,
        0xF7, 0x72, 0x50, 0xD5, 0xDF, 0x5A, 0xCB, 0x4E, 0x44, 0xC1, 0x43,
        0xC6, 0xCC, 0x49, 0xD8, 0x5D, 0x57, 0xD2, 0xF0, 0x75, 0x7F, 0xFA,
        0x6B, 0xEE, 0xE4, 0x61, 0xA0, 0x25, 0x2F, 0xAA, 0x3B, 0xBE, 0xB4,
        0x31, 0x13, 0x96, 0x9C, 0x19, 0x88, 0x0D, 0x07, 0x82, 0x86, 0x03,
        0x09, 0x8C, 0x1D, 0x98, 0x92, 0x17, 0x35, 0xB0, 0xBA, 0x3F, 0xAE,
        0x2B, 0x21, 0xA4, 0x65, 0xE0, 0xEA, 0x6F, 0xFE, 0x7B, 0x71, 0xF4,
        0xD6, 0x53, 0x59, 0xDC, 0x4D, 0xC8, 0xC2, 0x47, 0xC5, 0x40, 0x4A,
        0xCF, 0x5E, 0xDB, 0xD1, 0x54, 0x76, 0xF3, 0xF9, 0x7C, 0xED, 0x68,
        0x62, 0xE7, 0x26, 0xA3, 0xA9, 0x2C, 0xBD, 0x38, 0x32, 0xB7, 0x95,
        0x10, 0x1A, 0x9F, 0x0E, 0x8B, 0x81, 0x04, 0x89, 0x0C, 0x06, 0x83,
        0x12, 0x97, 0x9D, 0x18, 0x3A, 0xBF, 0xB5, 0x30, 0xA1, 0x24, 0x2E,
        0xAB, 0x6A, 0xEF, 0xE5, 0x60, 0xF1, 0x74, 0x7E, 0xFB, 0xD9, 0x5C,
        0x56, 0xD3, 0x42, 0xC7, 0xCD, 0x48, 0xCA, 0x4F, 0x45, 0xC0, 0x51,
        0xD4, 0xDE, 0x5B, 0x79, 0xFC, 0xF6, 0x73, 0xE2, 0x67, 0x6D, 0xE8,
        0x29, 0xAC, 0xA6, 0x23, 0xB2, 0x37, 0x3D, 0xB8, 0x9A, 0x1F, 0x15,
        0x90, 0x01, 0x84, 0x8E, 0x0B, 0x0F, 0x8A, 0x80, 0x05, 0x94, 0x11,
        0x1B, 0x9E, 0xBC, 0x39, 0x33, 0xB6, 0x27, 0xA2, 0xA8, 0x2D, 0xEC,
        0x69, 0x63, 0xE6, 0x77, 0xF2, 0xF8, 0x7D, 0x5F, 0xDA, 0xD0, 0x55,
        0xC4, 0x41, 0x4B, 0xCE, 0x4C, 0xC9, 0xC3, 0x46, 0xD7, 0x52, 0x58,
        0xDD, 0xFF, 0x7A, 0x70, 0xF5, 0x64, 0xE1, 0xEB, 0x6E, 0xAF, 0x2A,
        0x20, 0xA5, 0x34, 0xB1, 0xBB, 0x3E, 0x1C, 0x99, 0x93, 0x16, 0x87,
        0x02, 0x08, 0x8D};
    uint8_t crc = 0;
    for (uint32_t byte = 0; byte < 32; byte++)
    {
        crc = (uint8_t)(crctable[(uint8_t)(data[byte] ^ crc)]);
    }
    return crc; //Returns the non-inverted CRC of a 32-byte data stream
}

//Need to pass the 16bit address WITH the address CRC bits populated.
//Return 1 if the address CRC from the console matches the internal calculation.
static uint8_t n64_compare_addr_crc(uint16_t encoded_add_console)
{
    //See http://svn.navi.cx/misc/trunk/wasabi/devices/cube64/notes/addr_encoder.py
    static uint8_t crctable[11] = {
        0x15, 0x1F, 0x0B, 0x16, 0x19, 0x07, 0x0E, 0x1C, 0x0D, 0x1A, 0x01};
    uint16_t encoded_add_cont = encoded_add_console & 0xFFE0;
    for (uint32_t i = 0; i < sizeof(crctable); i++)
    {
        if (encoded_add_cont & (1 << (i + 5)))
            encoded_add_cont ^= crctable[i];
    }
    return encoded_add_cont == encoded_add_console;
}

static void n64_send_stream(uint8_t *txbuff, uint32_t len, n64_controller *c)
{
    uint32_t cycle_cnt = 0;
    uint32_t current_byte = 0;
    uint32_t current_bit = 8;
    uint32_t hs_clock = n64hal_hs_tick_get_speed();
    uint32_t U_SEC = hs_clock / 1000000; //clocks per microsecond
    n64hal_hs_tick_reset();
    cycle_cnt = n64hal_hs_tick_get();
    while (len > 0)
    {
        while (n64hal_hs_tick_get() < cycle_cnt);
        n64hal_input_swap(c, N64_OUTPUT); //OUTPUT_PP will pull low
        (txbuff[current_byte] & 0x80) ? (cycle_cnt += 1 * U_SEC) : (cycle_cnt += 3 * U_SEC);
        while (n64hal_hs_tick_get() < cycle_cnt);
        n64hal_input_swap(c, N64_INPUT);
        (txbuff[current_byte] & 0x80) ? (cycle_cnt += 3 * U_SEC) : (cycle_cnt += 1 * U_SEC);
        txbuff[current_byte] = txbuff[current_byte] << 1;
        current_bit--;

        //next byte if current byte complete
        if (current_bit == 0)
        {
            current_bit = 8;
            current_byte++;
            len--;
        }
    }

    //Send stop bit. Pull low for 2us, then release.
    while (n64hal_hs_tick_get() < cycle_cnt);
    n64hal_input_swap(c, N64_OUTPUT);
    cycle_cnt += 2 * U_SEC;
    while (n64hal_hs_tick_get() < cycle_cnt);
    n64hal_input_swap(c, N64_INPUT); //Release bus. We're done
}

static void n64_reset_stream(n64_controller *cont)
{
    cont->current_bit = 7;
    cont->current_byte = 0;
    cont->data_buffer[0] = 0;
}

static void inline n64_wait_micros(uint32_t micros){
    uint32_t clocks = micros * (n64hal_hs_tick_get_speed() / 1000000);
    while (n64hal_hs_tick_get() < clocks);
}

//This function is called in the falling edge of the n64 data bus.
void n64_controller_hande_new_edge(n64_controller *cont)
{
    uint32_t start_clock = n64hal_hs_tick_get();

    static uint16_t peri_address[MAX_CONTROLLERS] = {0};
    static uint32_t peri_access[MAX_CONTROLLERS] = {0};
    static uint32_t bus_timer[MAX_CONTROLLERS] = {0};

    //Hardcoded controller responses for indentify requests
    static uint8_t n64_mouse[]          = {0x02, 0x00, 0x00};
    static uint8_t n64_cont_with_peri[] = {0x05, 0x00, 0x01};
    static uint8_t n64_cont_no_peri[]   = {0x05, 0x00, 0x02};
    static uint8_t n64_cont_crc_error[] = {0x05, 0x00, 0x04};

    //If bus has been idle for 300us, start of a new stream.
    if ((n64hal_hs_tick_get() - bus_timer[cont->id]) > (300 * n64hal_hs_tick_get_speed() / 1000000))
    {
        n64_reset_stream(cont);
        peri_access[cont->id] = 0;
    }

    //If byte has completed, increment buffer for next byte and reset bit counter.
    if (cont->current_bit == -1)
    {
        cont->current_bit = 7;
        cont->current_byte++;
        (cont->current_byte > N64_MAX_POS) ? cont->current_byte = 0 : (0);
        cont->data_buffer[cont->current_byte] = 0x00;
    }

    //Wait for ~1.05us to pass since falling edge before reading bit
    uint32_t end_clock = start_clock + (n64hal_hs_tick_get_speed() / 950000);
    while (n64hal_hs_tick_get() < end_clock);

    //Read bit
    cont->data_buffer[cont->current_byte] |= n64hal_input_read(cont) << cont->current_bit;
    cont->current_bit -= 1;

    //Reset idle timer
    n64hal_hs_tick_reset();
    bus_timer[cont->id] = n64hal_hs_tick_get();

    //If byte 0 has been completed, we need to identify what the command is
    if (cont->current_byte == 1)
    {
        switch (cont->data_buffer[N64_COMMAND_POS])
        {
        case N64_IDENTIFY:
        case N64_CONTROLLER_RESET:
            if (cont->is_mouse)
                memcpy(&cont->data_buffer[N64_DATA_POS], n64_mouse, 3);

            else if (cont->current_peripheral != PERI_NONE && !cont->crc_error)
                memcpy(&cont->data_buffer[N64_DATA_POS], n64_cont_with_peri, 3);

            else if (cont->current_peripheral != PERI_NONE && cont->crc_error)
                memcpy(&cont->data_buffer[N64_DATA_POS], n64_cont_crc_error, 3);

            else
                memcpy(&cont->data_buffer[N64_DATA_POS], n64_cont_no_peri, 3);

            cont->crc_error = 0;
            n64_wait_micros(2);
            n64_send_stream(&cont->data_buffer[N64_DATA_POS], 3, cont);
            n64_reset_stream(cont);
            break;

        case N64_CONTROLLER_STATUS:
            n64_wait_micros(2);
            n64_send_stream((uint8_t *)&cont->b_state, 4, cont);
            n64_reset_stream(cont);
            cont->b_state.dButtons = 0x0000;
            break;
        case N64_PERI_READ:
        case N64_PERI_WRITE:
            peri_access[cont->id] = 1;
            break;
        default:
            peri_access[cont->id] = 0;
            n64_reset_stream(cont);
            break;
        }
    }

    //If we are accessing the peripheral bus, let's handle that
    if (peri_access[cont->id] == 1)
    {
        /*WRITE ACCESS*/
        //If there was a 'write' command to the peripheral bus, check if all 32 bytes of data have been received
        if (cont->data_buffer[N64_COMMAND_POS] == N64_PERI_WRITE && cont->current_byte == (N64_DATA_POS + 32))
        {

            peri_address[cont->id] = (cont->data_buffer[N64_ADDRESS_MSB_POS] << 8 |
                                      cont->data_buffer[N64_ADDRESS_LSB_POS]);

            if (!n64_compare_addr_crc(peri_address[cont->id]))
            {
                cont->crc_error = 1;
                debug_print_error("ERROR: Address CRC Error %04x\n", peri_address[cont->id]);
            }

            peri_address[cont->id] &= 0xFFE0;
            cont->data_buffer[N64_CRC_POS] = n64_get_crc(&cont->data_buffer[N64_DATA_POS]);
            //If no peripheral, the CRC is inverted
            if (cont->current_peripheral == PERI_NONE)
                cont->data_buffer[N64_CRC_POS] = ~cont->data_buffer[N64_CRC_POS];


            //Send the data CRC out straight away. N64 expects this very quickly
            n64_send_stream(&cont->data_buffer[N64_CRC_POS], 1, cont);

            //Now handle the write command
            switch (peri_address[cont->id] >> 12)
            {
                case 0x0:
                    //VIRTUAL MEMPAK NOTE TABLE HOOK
                    if (cont->mempack->virtual_is_active && cont->current_peripheral == PERI_MEMPAK &&
                        peri_address[cont->id] >= 0x300 && peri_address[cont->id] < 0x500)
                    {
                        /*
                         * When you 'delete' a note from the mempak manager, I can hook the
                         * address being deleted and determine what row you selected.
                         * The note table is located between 0x300 and 0x500 in the mempak
                         * and has 32bytes (0x20) per note.
                         * I use this as a hacky menu for the N64
                         */
                        uint32_t row = (peri_address[cont->id] - 0x300) / 0x20; //What row you have 'selected' 0-15
                        cont->mempack->virtual_update_req = 1;
                        cont->mempack->virtual_selected_row = row;
                        debug_print_n64("N64: Virtualpak write at row %u\n", row);
                        break;
                    }
                    //Intentional fallthrough
                case 0x1:
                case 0x2:
                case 0x3:
                case 0x4:
                case 0x5:
                case 0x6:
                case 0x7:
                    if (cont->current_peripheral == PERI_MEMPAK && !cont->crc_error)
                        n64_mempack_write32(cont->mempack, peri_address[cont->id], &cont->data_buffer[N64_DATA_POS]);
                    break;
                case 0x8:
                    if (cont->current_peripheral == PERI_TPAK)
                    {
                        //N64 writes 32 bytes of 0x84 to turn on the TPAK
                        (cont->data_buffer[N64_DATA_POS] == 0x84) ? cont->tpak->power_state = 1 : (0);
                        (cont->data_buffer[N64_DATA_POS] == 0xFE) ? tpak_reset(cont->tpak)      : (0);
                        debug_print_tpak("TPAK: Powerstate set to %u\n", cont->tpak->power_state);
                    }
                    else if (cont->current_peripheral == PERI_RUMBLE)
                    {
                        //N64 writes 32 bytes of 0x80 to initialise the rumblepak, 0xFE to reset it
                        (cont->data_buffer[N64_DATA_POS] == 0x80) ? cont->rpak->initialised = 1 : (0);
                        (cont->data_buffer[N64_DATA_POS] == 0xFE) ? cont->rpak->initialised = 0 : (0);
                        debug_print_n64("N64: Rumblepak Status %u\n", cont->rpak->initialised);
                    }
                    break;
                case 0xA:
                    if (cont->current_peripheral == PERI_TPAK)
                    {
                        //0x00, 0x01, or 0x02 and switches over the MBC memory space.
                        cont->tpak->selected_mbc_bank = cont->data_buffer[N64_DATA_POS];
                        debug_print_tpak("TPAK: MBC bank changed to %u\n",  cont->tpak->selected_mbc_bank);
                    }
                    break;
                case 0xB:
                    if (cont->current_peripheral == PERI_TPAK)
                    {
                        cont->tpak->access_state_changed = cont->tpak->access_state != cont->data_buffer[N64_DATA_POS];
                        cont->tpak->access_state = cont->data_buffer[N64_DATA_POS];
                        debug_print_tpak("TPAK: Access state set to %u\n",  cont->tpak->access_state);
                    }
                    break;
                case 0xC:
                     if (cont->current_peripheral == PERI_RUMBLE)
                     {
                         (cont->data_buffer[N64_DATA_POS] == 0x01) ? (cont->rpak->state = RUMBLE_START) :
                                                                     (cont->rpak->state = RUMBLE_STOP);
                         break;
                     }
                     //Intentional fallthrough
                case 0xD:
                case 0xE:
                case 0xF:
                    if (cont->current_peripheral == PERI_TPAK)
                    {
                        tpak_write(cont->tpak, peri_address[cont->id], &cont->data_buffer[N64_DATA_POS]);
                    }
                    break;
            }

            peri_access[cont->id] = 0;
            n64_reset_stream(cont);
        }

        /* READ ACCESS
         * If we are reading from mempak, check if the address bytes have been
         * received (i.e cont->current_byte and bit is pass the address bits)
         *
         * Reads from address 0x8000 to 0x9FFF has specific responses based on the peripheral
         * installed. This is handled in the code below
         */
        else if (cont->data_buffer[N64_COMMAND_POS] == N64_PERI_READ
#ifdef USE_N64_ADDRESS_CRC
                 && cont->current_byte == N64_DATA_POS /* N64_ADDRESS_LSB_POS */
#else
                 && cont->current_byte == N64_ADDRESS_LSB_POS && cont->current_bit == 4
#endif
        )
        {
            memset(&cont->data_buffer[N64_DATA_POS], 0x00, 32); //N64 responds with 0x00s unless otherwise set
            peri_address[cont->id] = (cont->data_buffer[N64_ADDRESS_MSB_POS] << 8 |
                                      cont->data_buffer[N64_ADDRESS_LSB_POS]);

#ifdef USE_N64_ADDRESS_CRC
            if (!n64_compare_addr_crc(peri_address[cont->id]))
            {
                cont->crc_error = 1;
                debug_print_error("ERROR: Address CRC Error %04x\n", peri_address[cont->id]);
            }
#endif

            //Clear the address CRC bits
            peri_address[cont->id] &= 0xFFE0;
            switch(peri_address[cont->id] >> 12)
            {
                case 0x0: //0x0000 - 0x0FFFF
                case 0x1: //0x1000 = 0x1FFFF
                    if (cont->current_peripheral == PERI_TPAK && cont->tpak->power_state == 1)
                    {
                        memset(&cont->data_buffer[N64_DATA_POS], 0x84, 32);
                        debug_print_tpak("TPAK: Request powerstate. Sending 0x84s (its enabled)\n");
                        break;
                    }
                    //Fall through intentional
                case 0x3: //0x3000 - 0x3FFF
                case 0xB: //0xB000 - 0xBFFF
                    if (cont->current_peripheral == PERI_TPAK)
                    {
                        if (cont->tpak->power_state != 1) break;
                        if (cont->tpak->gbcart == NULL)
                        {
                            memset(&cont->data_buffer[N64_DATA_POS], 0x44, 32); //Return 0x44's if no cart is installed.
                            break;
                        }

                        memset(&cont->data_buffer[N64_DATA_POS], (cont->tpak->access_state) ? 0x89 : 0x80, 32);
                        debug_print_tpak("TPAK: Request powerstate. Sending 0x%02x\n", (cont->tpak->access_state) ? 0x89 : 0x80);
                        //Set bit 2 of the first return value if the access mode was changed since last check.
                        cont->data_buffer[N64_DATA_POS] |= (cont->tpak->access_state_changed << 2);
                        cont->tpak->access_state_changed = 0;
                        break;
                    }
                    //Fall through intentional
                case 0x4:
                case 0x5:
                case 0x6:
                case 0x7:
                    if (cont->current_peripheral == PERI_MEMPAK)
                    {
                        n64_mempack_read32(cont->mempack, peri_address[cont->id], &cont->data_buffer[N64_DATA_POS]);
                    }
                    break;
                case 0x8:
                case 0x9:
                    //If rumblepak is initialised, respond with 32bytes of 0x80.
                    if (cont->current_peripheral == PERI_RUMBLE && cont->rpak->initialised == 1)
                    {
                        memset(&cont->data_buffer[N64_DATA_POS], 0x80, 32);
                        debug_print_n64("N64: Request rumblepak state. Sending 0x80s (its initialised)\n");
                    }
                    //If tpak is powered up, respond with 32bytes of 0x84.
                    else if (cont->current_peripheral == PERI_TPAK && cont->tpak->power_state == 1)
                    {
                        memset(&cont->data_buffer[N64_DATA_POS], 0x84, 32);
                        debug_print_tpak("TPAK: Request tpak power state. Sending 0x84s (its powered up)\n");
                    }
                    break;
                case 0xC:
                case 0xD:
                case 0xE:
                case 0xF:
                    if(cont->current_peripheral == PERI_TPAK && cont->tpak->gbcart)
                    {
                        if (cont->tpak->power_state != 1) break;
                        tpak_read(cont->tpak, peri_address[cont->id], &cont->data_buffer[N64_DATA_POS]);
                    }
                    break;
            }

            //Calculate the CRC of the data buffer and place it at the end of the packet.
            cont->data_buffer[N64_CRC_POS] = n64_get_crc(&cont->data_buffer[N64_DATA_POS]);

            //CRC is inverted when no peripheral is installed.
            if (cont->current_peripheral == PERI_NONE)
                cont->data_buffer[N64_CRC_POS] = ~cont->data_buffer[N64_CRC_POS];

#ifdef USE_N64_ADDRESS_CRC
            n64_wait_micros(5);
#else
            n64_wait_micros(30);
#endif
            n64_send_stream(&cont->data_buffer[N64_DATA_POS], 33, cont);

            peri_access[cont->id] = 0;
            n64_reset_stream(cont);
        }
    }
}
