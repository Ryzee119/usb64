// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "common.h"
#include "n64_controller.h"
#include "input.h"
#include "memory.h"
#include "tft.h"
#include "fileio.h"

#include "lvgl.h"
#include "controller_icon.h"
#include "usb64_logo.h"

static const int WIDTH = TFT_FRAMEBUFFER_WIDTH;
static const int HEIGHT = TFT_FRAMEBUFFER_HEIGHT;
static uint8_t _tft_page = 0;
static const uint8_t _tft_max_pages = 2;

static const uint8_t _tft_log_max_lines = 15;
static char *_tft_log_text_lines[_tft_log_max_lines];

extern n64_input_dev_t n64_in_dev[MAX_CONTROLLERS];
extern n64_transferpak n64_tpak[MAX_CONTROLLERS];
extern n64_input_dev_t n64_in_dev[MAX_CONTROLLERS];
extern n64_transferpak n64_tpak[MAX_CONTROLLERS];

static bool input_dirty[MAX_CONTROLLERS];
static bool log_dirty = true;
static bool n64_status;
static lv_obj_t *log_page;
static lv_obj_t *log_lines[_tft_log_max_lines];
static lv_obj_t *main_page;
static lv_obj_t *ram_label;
static lv_obj_t *sd_label;
static lv_obj_t *n64_status_label;
static lv_obj_t *controller_status_label[4];
static const char *NOT_CONNECTED = "NOT CONNECTED\n0x0000/0x0000";

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

void lvgl_putstring(const char *buf)
{
    usb64_printf("%s", buf);
}

FLASHMEM void tft_init()
{
    lv_init();
    lv_log_register_print_cb(lvgl_putstring);
    tft_dev_init();

    lv_obj_t *scr = lv_scr_act();

    lv_obj_t *usb64_image = lv_canvas_create(scr);
    lv_canvas_set_buffer(usb64_image, (void *)usb64_logo, 120, 35, LV_IMG_CF_TRUE_COLOR);
    lv_obj_update_layout(usb64_image);

    main_page = lv_obj_create(scr);
    lv_obj_set_size(main_page, lv_obj_get_width(scr), lv_obj_get_height(scr) - lv_obj_get_height(usb64_image));
    lv_obj_set_style_pad_all(main_page, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(main_page, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(main_page, LV_COLOR_MAKE(17,20,16), LV_PART_MAIN);
    lv_obj_set_pos(main_page, 0, lv_obj_get_height(usb64_image));

    log_page = lv_obj_create(scr);
    lv_obj_set_size(log_page, lv_obj_get_width(scr), lv_obj_get_height(scr) - lv_obj_get_height(usb64_image));
    lv_obj_set_style_pad_all(log_page, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(log_page, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(log_page, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(log_page, LV_COLOR_MAKE(17,20,16), LV_PART_MAIN);
    lv_obj_set_pos(log_page, 0, lv_obj_get_height(usb64_image));
    lv_obj_add_flag(log_page, LV_OBJ_FLAG_HIDDEN);

    ram_label = lv_label_create(scr);
    sd_label = lv_label_create(scr);
    n64_status_label = lv_label_create(scr);

    lv_label_set_text_fmt(ram_label, "RAM: %uMB", memory_get_ext_ram_size());

    if (fileio_detected())
    {
        lv_label_set_text_fmt(sd_label, "SD: Detected");
        lv_obj_set_style_text_color(sd_label, LV_COLOR_MAKE(0,255,0), LV_PART_MAIN);
    }
    else
    {
        lv_label_set_text_fmt(sd_label, "SD: Not Detected");
        lv_obj_set_style_text_color(sd_label, LV_COLOR_MAKE(255,0,0), LV_PART_MAIN);
    }

    lv_label_set_text_fmt(n64_status_label, "N64 is OFF");
    lv_obj_set_style_text_color(n64_status_label, LV_COLOR_MAKE(255,0,0), LV_PART_MAIN);

    lv_obj_update_layout(ram_label);
    lv_obj_update_layout(sd_label);
    lv_obj_update_layout(n64_status_label);

    lv_obj_align(ram_label, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_align(sd_label,  LV_ALIGN_TOP_RIGHT, 0, 10);
    lv_obj_align(n64_status_label, LV_ALIGN_TOP_RIGHT, 0, 20);

    lv_obj_t *n64_icon[MAX_CONTROLLERS];
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        lv_coord_t h = lv_obj_get_height(main_page);
        n64_icon[i] = lv_canvas_create(main_page);
        lv_canvas_set_buffer(n64_icon[i], (void *)controller_icon, 48, 45, LV_IMG_CF_TRUE_COLOR);
        lv_obj_align(n64_icon[i], LV_ALIGN_TOP_LEFT, 0, 0 + ((h - 0) * i / 4));

        controller_status_label[i] = lv_label_create(main_page);
        lv_obj_align(controller_status_label[i], LV_ALIGN_TOP_LEFT, 50, 0 + ((h - 0) * i / 4));
        lv_obj_set_style_text_font(controller_status_label[i], &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(controller_status_label[i], LV_COLOR_MAKE(255,255,255), LV_PART_MAIN);
        lv_label_set_text(controller_status_label[i], NOT_CONNECTED);
        input_dirty[i] = true;
    }

    lv_obj_set_layout(log_page, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(log_page, LV_FLEX_FLOW_COLUMN);
    for (int i = 0; i < _tft_log_max_lines; i++)
    {
        log_lines[i] = lv_label_create(log_page);
        lv_obj_set_style_pad_all(log_lines[i], 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(log_lines[i], 0, LV_PART_MAIN);
        lv_obj_align(log_lines[i], LV_ALIGN_LEFT_MID, 0, 0);
        lv_label_set_text(log_lines[i], "");
    }

    n64_status = false;

    lv_obj_update_layout(scr);
    tft_update();
}

uint8_t tft_change_page(uint8_t page)
{
    _tft_page = (_tft_page + 1) % _tft_max_pages;
    if (_tft_page == 0)
    {
        lv_obj_add_flag(log_page, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_clear_flag(main_page, LV_OBJ_FLAG_HIDDEN); 
    }
    else
    {
        lv_obj_clear_flag(log_page, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(main_page, LV_OBJ_FLAG_HIDDEN); 
    }
    return _tft_page;
}

void tft_update()
{
    lv_task_handler();
    if (lv_obj_is_visible(main_page))
    {
        for (int i = 0; i < MAX_CONTROLLERS; i++)
        {
            if (input_dirty[i] == false)
            {
                continue;
            }
            input_dirty[i] = false;
            lv_color_t colour;
            if (input_is_connected(i))
                colour = LV_COLOR_MAKE(0, 255, 0);
            else
                colour = LV_COLOR_MAKE(255, 255, 255);
            const char *peri_str = n64_peri_to_string(&n64_in_dev[i]);
            lv_obj_set_style_text_color(controller_status_label[i], colour, LV_PART_MAIN);
            lv_label_set_text_fmt(controller_status_label[i], "%s\n0x%04x/0x%04x", peri_str,
                                  input_get_id_vendor(i), input_get_id_product(i));
        }

        if (n64_status != n64hal_input_read(N64_CONSOLE_SENSE_PIN))
        {
            lv_color_t colour;
            const char *n64_status_text;
            n64_status = n64hal_input_read(N64_CONSOLE_SENSE_PIN);
            if (n64_status)
            {
                n64_status_text = "N64 is ON";
                colour = LV_COLOR_MAKE(0, 255, 0);
            }
            else
            {
                n64_status_text = "N64 is OFF";
                colour = LV_COLOR_MAKE(255, 0, 0);
            }
            lv_obj_set_style_text_color(n64_status_label, colour, LV_PART_MAIN);
            lv_label_set_text(n64_status_label, n64_status_text);
        }
    }
    else if (lv_obj_is_visible(log_page) && log_dirty)
    {
        log_dirty = false;
        for (int i = 0; i < _tft_log_max_lines; i++)
        {
            if (_tft_log_text_lines[i] == NULL) break;
            lv_label_set_text(log_lines[i], _tft_log_text_lines[i]);
        }
    }
}

void tft_flag_update(uint8_t controller)
{
    if (controller >=0 && controller < sizeof(input_dirty))
    {
        input_dirty[controller] = true;
    }
    else
    {
        log_dirty = true;
    }
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