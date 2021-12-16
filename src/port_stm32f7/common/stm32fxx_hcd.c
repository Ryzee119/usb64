/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, hathach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

#include "tusb_option.h"
#include "host/hcd.h"
#include "host/usbh.h"
#include "stm32f7xx_hal.h"
#if CFG_TUSB_MCU == OPT_MCU_STM32F7
#include "device/dcd.h"

#define NUM_PIPES 11

static HCD_HandleTypeDef hhcd;
static bool port_connected = 0;

//For an IN, this field is the buffer size that the application has reserved for the transfer. The
//application is expected to program this field as an integer multiple of the maximum packet
//size for IN transactions (periodic and non-periodic).
//FIXME, how to make this more efficient?
static uint8_t _local_buffer[512];
static uint32_t user_len;
static uint8_t *user_buff;

static uint8_t _alloc_pipe()
{
    for (int i = 0; i < NUM_PIPES; i++)
    {
        if (hhcd.hc[i].max_packet == 0)
        {
            TU_LOG3("Allocated pipe %d\n\n\n", i);
            return i;
        }
    }
    TU_ASSERT(0);
}

static uint8_t _find_pipe(uint8_t dev_addr, uint8_t ep_addr)
{
    //For control pipe we share the input and output pipe, so remove the direction bit.
    if ((ep_addr & 0x7F) == 0)
    {
        ep_addr &= 0x7F;
    }
    for (int i = 0; i < NUM_PIPES; i++)
    {
        //STM HC pipes dont store the direction bit in the ep num filed, so re-add from ep_is_in it if not a control pipe so we can find it.
        uint8_t hc_ep_addr = hhcd.hc[i].ep_num | ((hhcd.hc[i].ep_is_in && ep_addr != 0x00) ? 0x80 : 0x00);
        if ((hhcd.hc[i].dev_addr == dev_addr) && (hc_ep_addr == ep_addr))
        {
            TU_LOG2("Found pipe in slot %d for address %02x, ep_addr %02x\n", i, dev_addr, ep_addr);
            return i;
        }
    }
    TU_ASSERT(0);
}

static void deferred_resetport(void *param)
{
    HAL_HCD_ResetPort(&hhcd);
    //Device is reset, alert TinyUSB.
    hcd_event_device_attach(0, false);
}

void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd)
{
    //Need the reset port after connection to properly enable it.
    //Create a event to be handled outside the ISR.
    port_connected = true;
    hcd_event_t event =
        {
            .event_id = USBH_EVENT_FUNC_CALL,
            .func_call.func = deferred_resetport,
            .func_call.param = NULL,
        };
    hcd_event_handler(&event, true);
}

void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd)
{
    TU_LOG3("%s\n", __FUNCTION__);
    port_connected = 0;
    hcd_event_device_remove(0, true);
}

void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state)
{
    HCD_HCTypeDef *hc = &hhcd->hc[chnum];
    uint8_t ep_addr = hc->ep_num | ((hc->ep_is_in) ? 0x80 : 0x00);
    if (urb_state == URB_DONE)
    {
/*
        if (hc->ep_is_in && user_buff)
        {
            memcpy(user_buff, _local_buffer, user_len);
        }
*/
        TU_LOG3("Transfer SUCCESS devadr%02x ep:%02x\n", hc->dev_addr, ep_addr);
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, hc->xfer_len, XFER_RESULT_SUCCESS, true);
    }
    else if (urb_state == URB_STALL)
    {
        TU_LOG1("Transfer ERROR: URB_STALL\n");
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, hc->xfer_len, XFER_RESULT_STALLED, true);
    }
    else if (urb_state == URB_ERROR)
    {
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, hc->xfer_len, XFER_RESULT_FAILED, true);
        TU_LOG1("Transfer ERROR: URB_ERROR\n");
    }
    else if (urb_state == URB_IDLE || urb_state == URB_IDLE)
    {
        //Resubmit URB (this happens sometimes)
        HAL_HCD_HC_SubmitRequest(hhcd, chnum, hc->ep_is_in, hc->ep_type, 1, hc->xfer_buff, hc->xfer_len, 0);
    }
}

// HCD closes all opened endpoints belong to this device
void hcd_device_close(uint8_t rhport, uint8_t dev_addr)
{
    (void)rhport;
    for (int i = 0; i < NUM_PIPES + 1; i++)
    {
        if (hhcd.hc[i].dev_addr == dev_addr && hhcd.hc[i].max_packet > 0)
        {
            TU_LOG3("Closing pipe %d\n", i);
            HAL_HCD_HC_Halt(&hhcd, i);
            tu_memclr(&hhcd.hc[i], sizeof(HCD_HCTypeDef));
        }
    }
}

// Enable USB interrupt
void hcd_int_enable(uint8_t rhport)
{
    (void)rhport;
    __HAL_HCD_ENABLE(&hhcd);
}

// Disable USB interrupt
void hcd_int_disable(uint8_t rhport)
{
    (void)rhport;
    __HAL_HCD_DISABLE(&hhcd);
}

// Get frame number (1ms)
uint32_t hcd_frame_number(uint8_t rhport)
{
    return HAL_HCD_GetCurrentFrame(&hhcd);
}

// Get the current connect status of roothub port
bool hcd_port_connect_status(uint8_t rhport)
{
    (void)rhport;
    return port_connected;
}

// Reset USB bus on the port
void hcd_port_reset(uint8_t rhport)
{
    (void)rhport;
    HAL_HCD_ResetPort(&hhcd);
}

// Get port link speed
tusb_speed_t hcd_port_speed_get(uint8_t rhport)
{
    uint32_t speed = HAL_HCD_GetCurrentSpeed(&hhcd);
    switch (speed)
    {
    case 0:
        return TUSB_SPEED_HIGH;
    case 1:
        return TUSB_SPEED_FULL;
    case 2:
        return TUSB_SPEED_LOW;
    default:
        return TUSB_SPEED_FULL;
    }
    return TUSB_SPEED_INVALID;
}

// Initialize controller to host mode
bool hcd_init(uint8_t rhport)
{
    (void)rhport;
    HAL_StatusTypeDef res = HAL_ERROR;
    hhcd.Instance = USB_OTG_FS;
    hhcd.Init.Host_channels = NUM_PIPES;
    hhcd.Init.dma_enable = 0;
    hhcd.Init.low_power_enable = 0;
    hhcd.Init.phy_itface = HCD_PHY_EMBEDDED;
    hhcd.Init.Sof_enable = 0;
    hhcd.Init.speed = HCD_SPEED_FULL;
    hhcd.Init.vbus_sensing_enable = 0;
    hhcd.Init.lpm_enable = 0;
    res = HAL_HCD_Init(&hhcd);
    if (res == HAL_OK)
    {
        res = HAL_HCD_Start(&hhcd);
    }
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    return (res == HAL_OK);
}

bool hcd_edpt_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_endpoint_t const *ep_desc)
{
    (void)rhport;
    TU_LOG3("%s\n", __FUNCTION__);
    int ret = HAL_OK;

    uint8_t ep_type = EP_TYPE_CTRL;
    switch (ep_desc->bmAttributes.xfer)
    {
    case TUSB_XFER_CONTROL:
        ep_type = EP_TYPE_CTRL;
        break;
    case TUSB_XFER_ISOCHRONOUS:
        ep_type = EP_TYPE_ISOC;
        break;
    case TUSB_XFER_BULK:
        ep_type = EP_TYPE_BULK;
        break;
    case TUSB_XFER_INTERRUPT:
        ep_type = EP_TYPE_INTR;
        break;
    }
    uint8_t ch_num = _alloc_pipe();
    tu_memclr(&hhcd.hc[ch_num], sizeof(HCD_HCTypeDef));
    TU_ASSERT(ep_desc->wMaxPacketSize > 0); //FIXME?, I relay on mps > 0 for pipe allocation
    ret = HAL_HCD_HC_Init(&hhcd, ch_num, ep_desc->bEndpointAddress, dev_addr, HAL_HCD_GetCurrentSpeed(&hhcd), ep_type, ep_desc->wMaxPacketSize);
    TU_LOG3("Opened pipe for devaddr:%d epaddr:%02x in slot %d\n", dev_addr, ep_desc->bEndpointAddress, ch_num);
    return (ret == HAL_OK);
}

bool hcd_setup_send(uint8_t rhport, uint8_t dev_addr, uint8_t const setup_packet[8])
{
    (void)rhport;
    uint8_t ch_num = _find_pipe(dev_addr, 0x00);

    HAL_StatusTypeDef ret = HAL_HCD_HC_SubmitRequest(&hhcd,
                                                     ch_num,
                                                     0,
                                                     EP_TYPE_CTRL,
                                                     0,
                                                     (uint8_t *)setup_packet,
                                                     8,
                                                     0);
    return ret == HAL_OK;
}

bool hcd_edpt_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr, uint8_t *buffer, uint16_t buflen)
{
    (void)rhport;

    tusb_dir_t const ep_dir = tu_edpt_dir(ep_addr);
    uint8_t ch_num = _find_pipe(dev_addr, ep_addr);
    TU_LOG2("hcd_edpt_xfer on pipe for devaddr:%d epaddr:%02x in slot %d, ep_type %d, ep_dir %d, len %d, togglein %d dir bool %d\n",
            dev_addr, ep_addr, ch_num, hhcd.hc[ch_num].ep_type, ep_dir, buflen, hhcd.hc[ch_num].toggle_in, (ep_dir == TUSB_DIR_IN) ? 1 : 0);

    if (hhcd.hc[ch_num].urb_state == URB_NOTREADY || hhcd.hc[ch_num].urb_state == URB_NYET || (hhcd.hc[ch_num].state != HC_IDLE && hhcd.hc[ch_num].state != HC_XFRC))
    {
        TU_LOG1("Error: Endpoint %02x is busy, urb: %02x hc: %02x\n", ep_addr, hhcd.hc[ch_num].urb_state, hhcd.hc[ch_num].state);
        TU_ASSERT(0);
    }

/*
    //FXIME. STM32F7 expects packets in multiples of maxpacket size. TinyUSB doesnt always do this so can overflow buffers
    if ((ep_dir == TUSB_DIR_IN && (buflen % hhcd.hc[ch_num].max_packet != 0)))
    {
        TU_ASSERT(buflen < sizeof(_local_buffer));
        user_buff = buffer;
        user_len = buflen;
        buffer = _local_buffer;
    }
    else
    {
        user_buff = NULL;
    }
*/

    HAL_StatusTypeDef ret = HAL_HCD_HC_SubmitRequest(&hhcd,
                                                     ch_num,
                                                     (ep_dir == TUSB_DIR_IN) ? 1 : 0,
                                                     hhcd.hc[ch_num].ep_type,
                                                     1,
                                                     buffer,
                                                     buflen,
                                                     0);
                                            
    return (ret == HAL_OK);
}

void hcd_int_handler(uint8_t rhport)
{
    HAL_HCD_IRQHandler(&hhcd);
}

bool hcd_edpt_clear_stall(uint8_t dev_addr, uint8_t ep_addr)
{
    uint8_t ch_num = _find_pipe(dev_addr, ep_addr);
    hhcd.hc[ch_num].urb_state = URB_IDLE;
    return true;
}

#endif
