// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "tusb.h"
#include "printf.h"

void USB_OTG2_IRQHandler(void)
{
    tuh_int_handler(1);
}

void usbh_dev_init()
{
    // Teensy 4.0 PLL & USB PHY powerup
    while (1)
    {
        uint32_t n = CCM_ANALOG_PLL_USB2;
        if (n & CCM_ANALOG_PLL_USB2_DIV_SELECT)
        {
            CCM_ANALOG_PLL_USB2_CLR = 0xC000; // get out of 528 MHz mode
            CCM_ANALOG_PLL_USB2_SET = CCM_ANALOG_PLL_USB2_BYPASS;
            CCM_ANALOG_PLL_USB2_CLR = CCM_ANALOG_PLL_USB2_POWER |
                                      CCM_ANALOG_PLL_USB2_DIV_SELECT |
                                      CCM_ANALOG_PLL_USB2_ENABLE |
                                      CCM_ANALOG_PLL_USB2_EN_USB_CLKS;
            continue;
        }
        if (!(n & CCM_ANALOG_PLL_USB2_ENABLE))
        {
            CCM_ANALOG_PLL_USB2_SET = CCM_ANALOG_PLL_USB2_ENABLE; // enable
            continue;
        }
        if (!(n & CCM_ANALOG_PLL_USB2_POWER))
        {
            CCM_ANALOG_PLL_USB2_SET = CCM_ANALOG_PLL_USB2_POWER; // power up
            continue;
        }
        if (!(n & CCM_ANALOG_PLL_USB2_LOCK))
        {
            continue; // wait for lock
        }
        if (n & CCM_ANALOG_PLL_USB2_BYPASS)
        {
            CCM_ANALOG_PLL_USB2_CLR = CCM_ANALOG_PLL_USB2_BYPASS; // turn off bypass
            continue;
        }
        if (!(n & CCM_ANALOG_PLL_USB2_EN_USB_CLKS))
        {
            CCM_ANALOG_PLL_USB2_SET = CCM_ANALOG_PLL_USB2_EN_USB_CLKS; // enable
            continue;
        }
        break; // USB2 PLL up and running
    }
    // turn on USB clocks (should already be on)
    CCM_CCGR6 |= CCM_CCGR6_USBOH3(CCM_CCGR_ON);
    // turn on USB2 PHY
    USBPHY2_CTRL_CLR = USBPHY_CTRL_SFTRST | USBPHY_CTRL_CLKGATE;
    USBPHY2_CTRL_SET = USBPHY_CTRL_ENUTMILEVEL2 | USBPHY_CTRL_ENUTMILEVEL3;
    USBPHY2_PWD = 0;

#ifdef ARDUINO_TEENSY41
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40 = 5;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_40 = 0x0008; // slow speed, weak 150 ohm drive
    GPIO8_GDIR |= 1 << 26;
    GPIO8_DR_SET = 1 << 26;
#endif

    delay(10);
    NVIC_DISABLE_IRQ(IRQ_USB2);
    attachInterruptVector(IRQ_USB2, USB_OTG2_IRQHandler);
}
