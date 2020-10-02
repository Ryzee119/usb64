// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "USBHost_t36.h"
#include "usb64_conf.h"
#include "n64_controller.h"
#include "input.h"
#include "memory.h"
#include "printf.h"
#include "ILI9341_t3n.h"
#include "tft.h"
#include "diskio_wrapper.h"
#include "fileio.h"

#if (ENABLE_TFT_DISPLAY >= 1)
#include "controller_icon.h"
#include "usb64_logo.h"
#include "ili9341_t3n_font_Arial.h"
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO);
DMAMEM uint16_t _framebuffer[320 * 240];

static uint8_t _tft_page = 0;
static uint8_t _tft_max_pages = 2;
static uint8_t _tft_update_needed = 0;

static const uint8_t _tft_log_max_lines = 15;
static char *_tft_log_text_lines[_tft_log_max_lines];

extern n64_controller n64_c[MAX_CONTROLLERS];
extern n64_transferpak n64_tpak[MAX_CONTROLLERS];

static const char *n64_peri_to_string(n64_controller *c)
{
    static char text_buff[32];

    //Handle some specific cases
    if (input_is_connected(c->id) == 0)
    {
        tft.setTextColor(ILI9341_WHITE);
        return "NOT CONNECTED";
    }

    if (c->is_mouse)
    {
        tft.setTextColor(ILI9341_GREEN);
        return "N64 MOUSE";
    }

    if (input_is_dualstick_mode(c->id))
    {
        tft.setTextColor(ILI9341_GREEN);
        return "DUAL STICK MODE";
    }

    switch (c->current_peripheral)
    {
    case PERI_NONE:
        tft.setTextColor(ILI9341_WHITE);
        return "NO PERIPHERAL";

    case PERI_RUMBLE:
        tft.setTextColor(ILI9341_GREEN);
        return "RUMBLE PAK";

    case PERI_MEMPAK:
        tft.setTextColor(ILI9341_GREEN);
        if (c->mempack->virtual_is_active)
            return "VIRTUAL PAK";

        snprintf(text_buff, sizeof(text_buff), "MPAK (BANK %u)", c->mempack->id);
        return text_buff;

    case PERI_TPAK:
        tft.setTextColor(ILI9341_GREEN);
        snprintf(text_buff, sizeof(text_buff), "TPAK (%s)", (c->tpak->gbcart->rom == NULL) ? "NO ROM" : c->tpak->gbcart->title);
        return text_buff;

    default:
        tft.setTextColor(ILI9341_RED);
        return "UNKNOWN";
    }
}

static void write_controller_status(int controller, int line, const char *text)
{
    const int x_margin = 50;
    const int y_margin = 30;
    const int text_height = 14;
    const int line_padding = 2;
    const int controller_padding = 22;

    //Clear old text
    tft.fillRect(x_margin, y_margin + text_height + controller_padding * controller + (controller * 2 + line) * text_height + line_padding * line,
                 tft.width() - x_margin,
                 text_height + 2, BG_COLOUR);

    //Draw new text
    tft.setCursor(x_margin, y_margin + text_height + controller_padding * controller +
                                (controller * 2 + line) * text_height + line_padding * line);

    tft.print(text);
}
#endif

void tft_init()
{
#if (ENABLE_TFT_DISPLAY >= 1)
    tft.begin();
    tft.setRotation(TFT_ROTATION);
    tft.setFrameBuffer(_framebuffer);
    tft.useFrameBuffer(true);

    tft_force_update();
#endif
}

void tft_try_update()
{
#if (ENABLE_TFT_DISPLAY >= 1)

#if (0)
    //Dump the framebuffer to a file on the SD Card, 10 seconds after power up.
    //Convert to png with
    //ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgb565le -s 320x240 -i tft_dump.bin -f image2 -vcodec png tft_dump.png
    if (millis() > 10000)
    {
        fileio_write_to_file("tft_dump.bin", (uint8_t *)_framebuffer, sizeof(_framebuffer));
        while (1);
    }
#endif

    if (_tft_update_needed == 0)
        return;

    if (tft.asyncUpdateActive())
        return;

    tft_force_update();
#endif
}

uint8_t tft_change_page(uint8_t page)
{
#if (ENABLE_TFT_DISPLAY >= 1)
    (page >= _tft_max_pages) ? _tft_page = 0 : _tft_page = page;
    return _tft_page;
#else
    return 0;
#endif
}

void tft_force_update()
{
#if (ENABLE_TFT_DISPLAY >= 1)
    static int8_t current_page = -1;
    char text_buff[64];
    uint8_t page_changed = 0;

    while (tft.asyncUpdateActive());

    if (_tft_page != current_page)
    {
        page_changed = 1;
        current_page = _tft_page;
    }

    /* Draw static elements for page */
    if (page_changed)
    {
        tft.fillScreen(BG_COLOUR);

        //Draw usb64 logo
        tft.writeRect(0, 0, 120, 35, (uint16_t *)usb64_logo);

        tft.setFont(Arial_8);

        //Write the detected external ram
        tft.setTextColor(ILI9341_WHITE);
        snprintf(text_buff, sizeof(text_buff), "Detected RAM: %uMB", memory_get_ext_ram_size());
        tft.setCursor(125, 0);
        tft.print(text_buff);

        //Write the detected SD card size
        uint32_t sd_size = (_disk_volume_num_blocks() / 1024) * _disk_volume_get_block_size() / 1024;
        if (sd_size == 0)
            tft.setTextColor(ILI9341_RED);
        snprintf(text_buff, sizeof(text_buff), "SD: %uMiB", sd_size);
        tft.setCursor(125, 10);
        tft.print(text_buff);

        //Write the current page number
        tft.setTextColor(ILI9341_WHITE);
        snprintf(text_buff, sizeof(text_buff), "%u/%u", _tft_page + 1, _tft_max_pages);
        tft.setCursor(tft.width() - 10 * 6, 10);
        tft.print(text_buff);

        tft.setTextColor(ILI9341_WHITE);

        switch (_tft_page)
        {
        case 0:
            //Draw the four controller images
            tft.writeRect(0, 40, 48, 45, (uint16_t *)controller_icon);
            tft.writeRect(0, 90, 48, 45, (uint16_t *)controller_icon);
            tft.writeRect(0, 140, 48, 45, (uint16_t *)controller_icon);
            tft.writeRect(0, 190, 48, 45, (uint16_t *)controller_icon);
            break;
        case 1:
            break;
        }
    }

    /* Draw dynamic elements for each page */
    //Draw N64 sense status
    tft.setFont(Arial_8);
    tft.setCursor(tft.width() - 10 * 6, 0);
    tft.fillRect(tft.getCursorX(), tft.getCursorY(), tft.width() - tft.getCursorX(), 8, BG_COLOUR);
    if (digitalRead(N64_CONSOLE_SENSE) == 0)
    {
        tft.setTextColor(ILI9341_RED);
        tft.print("N64 is OFF");
    }
    else
    {
        tft.setTextColor(ILI9341_GREEN);
        tft.print("N64 is ON");
    }

    //These elements are present on specific pages only.
    switch (_tft_page)
    {
    case 0:
        //Print controller status screen
        tft.setFont(Arial_13);
        for (uint32_t i = 0; i < MAX_CONTROLLERS; i++)
        {
            snprintf(text_buff, sizeof(text_buff), "0x%04x/0x%04x\n",
                     input_get_id_vendor(i),
                     input_get_id_product(i));

            write_controller_status(i, 0, n64_peri_to_string(&n64_c[i]));
            write_controller_status(i, 1, text_buff);
        }
        break;
    case 1:
        //Print the debug log screen
        tft.fillRect(0, 40, tft.width(), tft.height() - 40, BG_COLOUR);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(0, 40);
        for (uint32_t i = 0; i < _tft_log_max_lines; i++)
        {
            if (_tft_log_text_lines[i] == NULL)
                break;

            tft.print(_tft_log_text_lines[i]);
        }

        break;
    }
    tft.updateScreenAsync();
    _tft_update_needed = 0;
#endif
}

void tft_flag_update()
{
#if (ENABLE_TFT_DISPLAY >= 1)
    _tft_update_needed = 1;
#endif
}

void tft_add_log(char c)
{
#if (ENABLE_TFT_DISPLAY >= 1)
    static int tft_log_line_num = 0;
    static uint32_t tft_log_pos = 0;
    static char tft_log[256] = {0};

    //Add character to tft log screen
    tft_log[tft_log_pos] = c;
    tft_log_pos++;

    //Build a new line to display
    if (c == '\n')
    {
        tft_log[tft_log_pos] = '\0';
        _tft_log_text_lines[tft_log_line_num] = (char *)malloc(strlen(tft_log) + 1);
        strcpy(_tft_log_text_lines[tft_log_line_num], tft_log);
        tft_log_line_num++;
        tft_log_pos = 0;
        tft_flag_update();
    }

    //Exceeded max lines, remove oldest line and shift lines up by one
    if (tft_log_line_num >= _tft_log_max_lines)
    {
        free(_tft_log_text_lines[0]);
        for (uint32_t i = 0; i < _tft_log_max_lines - 1; i++)
        {
            _tft_log_text_lines[i] = _tft_log_text_lines[i + 1];
        }
        _tft_log_text_lines[_tft_log_max_lines - 1] = NULL;
        tft_log_line_num--;
    }
#endif
}