// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "common.h"
#include "printf.h"
#include "controller_icon.h"
#include "usb64_logo.h"

#include "lvgl.h"
#include "ILI9341_t3n.h"

static ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO);
static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t draw_buf;
static DMAMEM uint8_t _framebuffer[TFT_WIDTH * TFT_HEIGHT * TFT_PIXEL_SIZE];

//LVGL Callback that should update the LCD
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    tft.updateScreenAsync();
}

//LVGL Callback that will be called whilse the LCD is updating
static void disp_wait(lv_disp_drv_t *disp_drv)
{
    if (tft.asyncUpdateActive() == false)
    {
        lv_disp_flush_ready(disp_drv);
    }
}

/*
 * Function: Perform any device specific LCD display setup here.
 * ----------------------------
 *   Returns: Void
 */
void tft_dev_init()
{
    tft.begin();
    tft.setRotation(TFT_ROTATION);
    tft.setFrameBuffer((uint16_t *)_framebuffer);
    tft.useFrameBuffer(true);
    tft.fillScreen(0x10A2);
    tft.updateScreen();

    lv_disp_draw_buf_init(&draw_buf, _framebuffer, NULL, TFT_WIDTH * TFT_HEIGHT);
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = disp_flush;
    disp_drv.wait_cb = disp_wait;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.direct_mode = 1;
    disp_drv.antialiasing = 0;

    lv_disp_drv_register(&disp_drv);
    lv_disp_t * disp = lv_disp_get_default();
    lv_timer_set_period(disp->refr_timer, 100);
    return;
}
