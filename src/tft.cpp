// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "common.h"
#include "n64_controller.h"
#include "input.h"
#include "memory.h"
#include "tft.h"
#include "fileio.h"

#define GUILITE_ON
#include "GuiLite.h"

#include "controller_icon.h"
#include "usb64_logo.h"

static const int WIDTH = TFT_FRAMEBUFFER_WIDTH;
static const int HEIGHT = TFT_FRAMEBUFFER_HEIGHT;
static uint8_t _tft_page = 0;
static uint8_t _tft_page_changed = 1;
static uint8_t _tft_max_pages = 2;
static uint8_t _tft_update_needed = 0;

static const uint8_t _tft_log_max_lines = 15;
static char *_tft_log_text_lines[_tft_log_max_lines];

extern n64_input_dev_t n64_in_dev[MAX_CONTROLLERS];
extern n64_transferpak n64_tpak[MAX_CONTROLLERS];

extern n64_input_dev_t n64_in_dev[MAX_CONTROLLERS];
extern n64_transferpak n64_tpak[MAX_CONTROLLERS];

static char text_buff[32];

extern const LATTICE_FONT_INFO Arial_14_GL;
extern const LATTICE_FONT_INFO Arial_19_GL;
extern c_surface *psurface_guilite;
static c_label n64_status;
static c_label fileio_status;
static c_label extram_size;
static c_label controller_status[4];
static c_label controller_id[4];
static c_label tft_log[_tft_log_max_lines];

static const char *n64_peri_to_string(n64_input_dev_t *c)
{
    static char text_buff[32];

    if (input_is_connected(c->id) == 0)
    {
        return "NOT CONNECTED";
    }

    if (c->type == N64_MOUSE)
    {
        return "N64 MOUSE";
    }

    if (c->type == N64_RANDNET)
    {
        return "N64 RANDNET";
    }

    if (input_is_dualstick_mode(c->id))
    {
        return "DUAL STICK MODE";
    }

    switch (c->current_peripheral)
    {
    case PERI_NONE:
        return "NO PERIPHERAL";
    case PERI_RPAK:
        return "RUMBLE-PAK";
    case PERI_CPAK:
        if (c->cpak->virtual_is_active)
            return "VIRTUAL-PAK";

        snprintf(text_buff, sizeof(text_buff), "CPAK (BANK %u)", (unsigned int)c->cpak->id);
        return text_buff;
    case PERI_TPAK:
        snprintf(text_buff, sizeof(text_buff), "TPAK (%s)", (c->tpak->gbcart->rom == NULL) ? "NO ROM" : c->tpak->gbcart->title);
        return text_buff;
    default:
        return "UNKNOWN";
    }
}

FLASHMEM void tft_init()
{
    tft_dev_init();

    if (psurface_guilite == NULL)
    {
        return;
    }
    
    psurface_guilite->fill_rect(0, 0, WIDTH, HEIGHT, TFT_BG_COLOR, Z_ORDER_LEVEL_0);

    static c_image usb64_image;
    memset(&usb64_image, 0, sizeof(c_image));
    BITMAP_INFO _image;
    _image.color_bits = 16;
    _image.height = 35;
    _image.width = 120;
    _image.pixel_color_array = usb64_logo;
    usb64_image.draw_image(psurface_guilite, Z_ORDER_LEVEL_0, &_image, 0, 0, TFT_BG_COLOR);

    //Draw RAM status
    snprintf(text_buff, sizeof(text_buff), "Detected RAM: %uMB", memory_get_ext_ram_size());
    extram_size.set_surface(psurface_guilite);
    extram_size.set_bg_color(TFT_BG_COLOR);
    extram_size.set_font_color(GL_RGB(255, 255, 255));
    extram_size.set_wnd_pos(125, 0, 1, Arial_14_GL.height);
    extram_size.set_font_type(&Arial_14_GL);
    extram_size.set_str(text_buff);
    extram_size.show_window();

    tft_force_update();
}

void tft_try_update()
{
#if (0)
    //Dump the framebuffer to a file on the SD Card, 10 seconds after power up. Assuming 16bit display.
    //Convert to png with
    //ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgb565le -s 320x240 -i tft_dump.bin -f image2 -vcodec png tft_dump.png
    if (n64hal_millis() > 10000)
    {
        fileio_write_to_file("tft_dump.bin", (uint8_t *)tft_dev_get_fb(), WIDTH * HEIGHT * 2);
        debug_print_status("TFT framebuffer dumped\n");
        while (1) yield();
    }
#endif

    if (_tft_update_needed == 0)
    {
        return;
    }

    if (tft_dev_is_busy())
    {
        return;
    }

    tft_force_update();
}

uint8_t tft_change_page(uint8_t page)
{
    _tft_page = (_tft_page + 1) % _tft_max_pages;
    _tft_page_changed = 1;
    return _tft_page;
}

void tft_force_update()
{
    //These are drawn once when the TFT page has changed.
    if (_tft_page_changed)
    {
        _tft_page_changed = 0;
        if (_tft_page == 0)
        {
            psurface_guilite->fill_rect(0, 40, WIDTH, HEIGHT, TFT_BG_COLOR, Z_ORDER_LEVEL_0);
            static c_image controller_image;
            BITMAP_INFO _image;
            memset(&controller_image, 0, sizeof(c_image));
            _image.color_bits = 16;
            _image.height = 45;
            _image.width = 48;
            _image.pixel_color_array = controller_icon;
            controller_image.draw_image(psurface_guilite, Z_ORDER_LEVEL_0, &_image, 0, 45 + ((HEIGHT - 45) * 0 / 4), TFT_BG_COLOR);
            controller_image.draw_image(psurface_guilite, Z_ORDER_LEVEL_0, &_image, 0, 45 + ((HEIGHT - 45) * 1 / 4), TFT_BG_COLOR);
            controller_image.draw_image(psurface_guilite, Z_ORDER_LEVEL_0, &_image, 0, 45 + ((HEIGHT - 45) * 2 / 4), TFT_BG_COLOR);
            controller_image.draw_image(psurface_guilite, Z_ORDER_LEVEL_0, &_image, 0, 45 + ((HEIGHT - 45) * 3 / 4), TFT_BG_COLOR);
        }
        else if (_tft_page == 1)
        {
            psurface_guilite->fill_rect(0, 40, WIDTH, HEIGHT, TFT_BG_COLOR, Z_ORDER_LEVEL_0);
        }
    }

    //Draw dynamic items here. There are drawn everytime a TFT update is flagged.
    if (_tft_page == 0)
    {
        //Draw controller status and peripheral type
        for (int i = 0; i < 4; i++)
        {
            uint32_t colour = input_is_connected(i) ? GL_RGB(0, 255, 0) : GL_RGB(255, 255, 255);
            controller_status[i].set_surface(psurface_guilite);
            controller_status[i].set_bg_color(TFT_BG_COLOR);
            controller_status[i].set_font_color(colour);
            controller_status[i].set_wnd_pos(50, (45 + 0) + ((HEIGHT - 45) * i / 4), WIDTH, Arial_19_GL.height);
            controller_status[i].set_font_type(&Arial_19_GL);
            controller_status[i].set_str(n64_peri_to_string(&n64_in_dev[i]));
            controller_status[i].show_window();

            snprintf(text_buff, sizeof(text_buff), "0x%04x/0x%04x", input_get_id_vendor(i), input_get_id_product(i));
            controller_id[i].set_surface(psurface_guilite);
            controller_id[i].set_font_color(colour);
            controller_id[i].set_bg_color(TFT_BG_COLOR);
            controller_id[i].set_wnd_pos(50, (45 + 20) + ((HEIGHT - 45) * i / 4), WIDTH, Arial_19_GL.height);
            controller_id[i].set_font_type(&Arial_19_GL);
            controller_id[i].set_str(text_buff);
            controller_id[i].show_window();
        }
    }
    else if (_tft_page == 1)
    {
        psurface_guilite->fill_rect(0, 40, WIDTH, HEIGHT, TFT_BG_COLOR, Z_ORDER_LEVEL_0);
        for (int i = 0; i < _tft_log_max_lines; i++)
        {
            if (_tft_log_text_lines[i] == NULL)
                break;

            tft_log[i].set_surface(psurface_guilite);
            tft_log[i].set_bg_color(TFT_BG_COLOR);
            tft_log[i].set_font_color(GL_RGB(255, 255, 255));
            tft_log[i].set_wnd_pos(0, 45 + i * Arial_14_GL.height, WIDTH, Arial_14_GL.height);
            tft_log[i].set_font_type(&Arial_14_GL);
            tft_log[i].set_str(_tft_log_text_lines[i]);
            tft_log[i].show_window();
        }
    }

    //Draw N64 console status
    uint32_t colour;
    const char *n64_status_text;
    if (n64hal_input_read(N64_CONSOLE_SENSE_PIN) == 0)
    {
        n64_status_text = "N64 is OFF";
        colour = GL_RGB(255, 0, 0);
    }
    else
    {
        n64_status_text = "N64 is ON";
        colour = GL_RGB(0, 255, 0);
    }
    n64_status.set_surface(psurface_guilite);
    n64_status.set_bg_color(TFT_BG_COLOR);
    n64_status.set_font_color(colour);
    n64_status.set_wnd_pos(WIDTH - (10 * 8), 0, 100, Arial_14_GL.height);
    n64_status.set_font_type(&Arial_14_GL);
    n64_status.set_str(n64_status_text);
    n64_status.show_window();

    //Draw SD Card status
    const char *fileio_status_text;
    if (fileio_detected() == 0)
    {
        fileio_status_text = "SD Not Detected";
        colour = GL_RGB(255, 0, 0);
    }
    else
    {
        fileio_status_text = "SD Detected";
        colour = GL_RGB(0, 255, 0);
    }
    n64_status.set_surface(psurface_guilite);
    n64_status.set_bg_color(TFT_BG_COLOR);
    n64_status.set_font_color(colour);
    n64_status.set_wnd_pos(125, Arial_14_GL.height, 100, Arial_14_GL.height);
    n64_status.set_font_type(&Arial_14_GL);
    n64_status.set_str(fileio_status_text);
    n64_status.show_window();

    //Draw the current games name
    n64_status_text = n64_get_current_game();
    n64_status.set_surface(psurface_guilite);
    n64_status.set_bg_color(TFT_BG_COLOR);
    n64_status.set_font_color(GL_RGB(255, 255, 255));
    n64_status.set_wnd_pos(125, Arial_14_GL.height * 2, 200, Arial_14_GL.height);
    n64_status.set_font_type(&Arial_14_GL);
    n64_status.set_str(n64_status_text);
    n64_status.show_window();

    tft_dev_draw(true);

    _tft_update_needed = 0;
}

void tft_flag_update()
{
    _tft_update_needed = 1;
}

void tft_add_log(char c)
{
    static int tft_log_line_num = 0;
    static uint32_t tft_log_pos = 0;
    static char tft_log[256] = {0};

    //Add character to tft log screen
    tft_log[tft_log_pos] = c;

    //Build a new line to display
    if (c == '\n')
    {
        tft_log[tft_log_pos] = '\0';
        _tft_log_text_lines[tft_log_line_num] = (char *)malloc(strlen(tft_log) + 1);
        if (_tft_log_text_lines[tft_log_line_num] != NULL)
        {
            strcpy(_tft_log_text_lines[tft_log_line_num], tft_log);
            tft_log_line_num++;
        }
        tft_log_pos = 0;
        tft_flag_update();
    }
    else
    {
        tft_log_pos = (tft_log_pos + 1) % 256;
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
}