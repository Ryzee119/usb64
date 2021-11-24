// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "usb64_conf.h"
#include "controller_icon.h"
#include "usb64_logo.h"
#include "printf.h"
#include "GuiLite.h"

#if (ENABLE_TFT_DISPLAY >= 1)
#include "ILI9341_t3n.h"
static ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO);
static DMAMEM uint16_t _framebuffer[320 * 240];
#else
//Stub a framebuffer
static uint16_t _framebuffer[1];
#endif

struct EXTERNAL_GFX_OP t4_gfx_op;

static void _draw_pixel(int x, int y, unsigned int rgb)
{
#if (ENABLE_TFT_DISPLAY >= 1)
    tft.drawPixel(x, y, GL_RGB_32_to_16(rgb));
#endif
}

static void _fill_rect(int x0, int y0, int x1, int y1, unsigned int rgb)
{
#if (ENABLE_TFT_DISPLAY >= 1)
    tft.fillRect(x0, y0, x1 - x0, y1 - y0, GL_RGB_32_to_16(rgb));
#endif
}

void tft_dev_draw(bool force)
{
#if (ENABLE_TFT_DISPLAY >= 1)
    if (!force && tft.asyncUpdateActive())
    {
        return;
    }
    while (tft.asyncUpdateActive())
        ;
    tft.updateScreenAsync();
#endif
}

static void _tft_assert(const char *file, int line)
{
    debug_print_error("[TFT] Error: Assert in %s on line %d\n", file, line);
    while (1)
        yield();
}

static void _tft_log_out(const char *log)
{
    debug_print_status(log);
}

void tft_dev_init()
{
    t4_gfx_op.draw_pixel = _draw_pixel;
    t4_gfx_op.fill_rect = _fill_rect;

#if (ENABLE_TFT_DISPLAY >= 1)
    tft.begin();
    tft.setRotation(TFT_ROTATION);
    tft.setFrameBuffer(_framebuffer);
    tft.useFrameBuffer(true);
#endif

    register_debug_function(_tft_assert, _tft_log_out);
}

void *tft_dev_get_fb()
{
    return _framebuffer;
}

bool tft_dev_is_busy()
{
#if (ENABLE_TFT_DISPLAY >= 1)
    return tft.asyncUpdateActive();
#endif
}
