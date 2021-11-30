// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include "usb64_conf.h"
#include "controller_icon.h"
#include "usb64_logo.h"
#include "GuiLite.h"

c_surface *psurface_guilite = NULL;
c_display *pdisplay_guilite = NULL;
#if TFT_USE_FRAMEBUFFER
static uint8_t _framebuffer[TFT_WIDTH * TFT_HEIGHT * TFT_PIXEL_SIZE];
#else
struct EXTERNAL_GFX_OP my_gfx_op;
#endif

//Stub for GuiLite. Remove if not required.
extern "C" void delay_ms(unsigned short nms)
{
}

static void _tft_assert(const char *file, int line)
{
    debug_print_error("[TFT] Error: Assert in %s on line %d\n", file, line);
    while (1)
        ;
}

static void _tft_log_out(const char *log)
{
    debug_print_status(log);
}

#if TFT_USE_FRAMEBUFFER == 0
/*
 * Function: Draw a pixel to your display at a given coordinate. Required if NOT using a framebuffer
 * ----------------------------
 *   Returns: Void
 *
 *   x: horizontal pixel position
 *   y: vertical pixel position
 *   rgb: RGB888 colour. You can use GL_RGB_32_to_16 to convert to RGB565 if needed.
 */
static void _draw_pixel(int x, int y, unsigned int rgb)
{
    //Device specific pixel draw
}

/*
 * Function: Fill a given rectangle area. Required if NOT using a framebuffer
 * ----------------------------
 *   Returns: Void
 *
 *   x0,y0: Top left rectangle pixel position
 *   x1,y1: Bottom right pixel position
 *   rgb: RGB888 colour. You can use GL_RGB_32_to_16 to convert to RGB565 if needed.
 */
static void _fill_rect(int x0, int y0, int x1, int y1, unsigned int rgb)
{

}
#endif

/*
 * Function: Start TFT draw. Required if using a framebuffer. This should draw the framebuffer to the screen using DMA or similar.
 * ----------------------------
 *   Returns: Void
 *
 *   force: If force is true, you must wait for any previous screen updates to finished then update.
 */
void tft_dev_draw(bool force)
{
    //Draw _framebuffer to screen if TFT_USE_FRAMEBUFFER is set.
}

/*
 * Function: Perform any device specific LCD display setup here.
 * ----------------------------
 *   Returns: Void
 */
void tft_dev_init()
{
#if TFT_USE_FRAMEBUFFER
    static c_surface surface(TFT_WIDTH, TFT_HEIGHT, 2, Z_ORDER_LEVEL_0);
    static c_display display(_framebuffer, TFT_WIDTH, TFT_HEIGHT, &surface);
    psurface_guilite = &surface;
    pdisplay_guilite = &display;
#else
    static c_surface_no_fb surface(TFT_WIDTH, TFT_HEIGHT, 2, &my_gfx_op, Z_ORDER_LEVEL_0);
    static c_display display(NULL, TFT_WIDTH, TFT_HEIGHT, &surface);
    my_gfx_op.draw_pixel = _draw_pixel;
    my_gfx_op.fill_rect = _fill_rect;
#endif
    psurface_guilite = &surface;
    pdisplay_guilite = &display;
    register_debug_function(_tft_assert, _tft_log_out);
#if (ENABLE_TFT_DISPLAY >= 1)
    //Device specific init here
#endif
}

bool tft_dev_is_busy()
{
    return 0;
}
