// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include "usb64_conf.h"
#include "controller_icon.h"
#include "usb64_logo.h"
#include "GuiLite.h"

struct EXTERNAL_GFX_OP null_gfx_op;

static void _draw_pixel(int x, int y, unsigned int rgb)
{
    //Device specific pixel draw
}

static void _fill_rect(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    //Device specific rect fill
}

void tft_dev_draw(bool force)
{
    //Device specific draw start. This may be start DMA etc.
}

static void _tft_assert(const char *file, int line)
{
    debug_print_error("[TFT] Error: Assert in %s on line %d\n", file, line);
    while (1);
}

static void _tft_log_out(const char *log)
{
    debug_print_status(log);
}

void tft_dev_init()
{
    null_gfx_op.draw_pixel = _draw_pixel;
    null_gfx_op.fill_rect = _fill_rect;
#if (ENABLE_TFT_DISPLAY >= 1)
    //Device specific init here
#endif
    register_debug_function(_tft_assert, _tft_log_out);
}

void *tft_dev_get_fb()
{
    return NULL;
}

bool tft_dev_is_busy()
{
#if (ENABLE_TFT_DISPLAY >= 1)
    return 0;
#endif
}
