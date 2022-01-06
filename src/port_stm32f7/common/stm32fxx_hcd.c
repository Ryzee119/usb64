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

//Have active pipes. STM32F7 is 12 for FS and 16 for HS
#define MAX_PIPES 12
#ifndef MAX_ENDPOINTS
#define MAX_ENDPOINTS 16
#endif
//The amount of shared channels reserved for interrupt transfer queue. If you are polling many interrupt endpoints
//increasing this can increase the overall responsiveness.
//This must be < MAX_PIPES, and must allow for free pipes for control, bulk and iso transfers
#ifndef INTR_CHANNELS
#define INTR_CHANNELS 1
#endif
static uint8_t intr_lock[INTR_CHANNELS] = {0}; //Make sure we dont use a pipe thats already in use.

typedef struct
{
    bool allocated;
    bool active;
    uint32_t last_frame;
    uint8_t dev_addr;
    uint8_t ch_num;
    uint8_t ep_type;
    tusb_desc_endpoint_t ep_desc;
    uint8_t *xfer_buff;
    uint32_t xfer_len;
    uint8_t toggle_in;
    uint8_t toggle_out;
} usbh_endpoint_t;

usbh_endpoint_t usbh_endpoint[MAX_ENDPOINTS] = {0};

static HCD_HandleTypeDef hhcd;
static bool root_port_connected = 0;

static usbh_endpoint_t *_alloc_endpoint(uint8_t dev_addr, tusb_desc_endpoint_t *ep_desc)
{
    for (int i = 0; i < MAX_ENDPOINTS; i++)
    {
        if (usbh_endpoint[i].allocated == false)
        {
            tu_memclr(&usbh_endpoint[i], sizeof(usbh_endpoint_t));
            usbh_endpoint[i].allocated = true;
            usbh_endpoint[i].dev_addr = dev_addr;
            memcpy(&usbh_endpoint[i].ep_desc, ep_desc, sizeof(tusb_desc_endpoint_t));
            switch (ep_desc->bmAttributes.xfer)
            {
            case TUSB_XFER_CONTROL:
                usbh_endpoint[i].ep_type = EP_TYPE_CTRL;
                break;
            case TUSB_XFER_ISOCHRONOUS:
                usbh_endpoint[i].ep_type = EP_TYPE_ISOC;
                break;
            case TUSB_XFER_BULK:
                usbh_endpoint[i].ep_type = EP_TYPE_BULK;
                break;
            case TUSB_XFER_INTERRUPT:
                usbh_endpoint[i].ep_type = EP_TYPE_INTR;
                break;
            }

            usbh_endpoint[i].ch_num = 0xFF;
            //Interrupt endpoints share host channels. Scheduled in SOF
            if (usbh_endpoint[i].ep_type == EP_TYPE_INTR)
            {
                usbh_endpoint[i].ch_num = 0;
            }
            else
            {
                //First 2 channels are reserved for interrupt endpoints (in and out)
                for (int ch_num = INTR_CHANNELS; ch_num < MAX_PIPES; ch_num++)
                {
                    if (hhcd.hc[ch_num].max_packet == 0)
                    {
                        usbh_endpoint[i].ch_num = ch_num;
                        break;
                    }
                }
            }
            if (usbh_endpoint[i].ch_num != 0xFF)
            {
                return &usbh_endpoint[i];
            }
            else
            {
                break;
            }
        }
    }
    return NULL;
}

static usbh_endpoint_t *_find_endpoint(uint8_t dev_addr, uint8_t ep_addr)
{
    //If it's a control pipe, remove direction bit. Use same endpoint for IN/OUT
    if (ep_addr == 0x80)
    {
        ep_addr = 0x00;
    }
    for (int i = 0; i < MAX_ENDPOINTS; i++)
    {
        if (usbh_endpoint[i].allocated == false)
        {
            continue;
        }
        if (usbh_endpoint[i].dev_addr == dev_addr && usbh_endpoint[i].ep_desc.bEndpointAddress == ep_addr)
        {
            return &usbh_endpoint[i];
        }
    }
    return NULL;
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
    root_port_connected = true;
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
    root_port_connected = 0;
    hcd_event_device_remove(0, true);
}

void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd)
{
    //Handle all queued interrupt transfers
    static int intr_in_queue_head = 0;
    int intr_channel = 0;
    for (int i = intr_in_queue_head; i < (MAX_ENDPOINTS + intr_in_queue_head); i++)
    {
        if (intr_channel >= INTR_CHANNELS)
        {
            return;
        }
        uint8_t ep_index = i % MAX_ENDPOINTS;
        if (!usbh_endpoint[ep_index].active)
        {
            continue;
        }

        usbh_endpoint_t *ep = &usbh_endpoint[ep_index];

        if (ep->ep_type == EP_TYPE_INTR && intr_lock[intr_channel] == 0)
        {
            if ((HAL_HCD_GetCurrentFrame(hhcd) - ep->last_frame) >= ep->ep_desc.bInterval)
            {
                //Get the currently active channel and store the toggle values
                HCD_HCTypeDef *hc = &hhcd->hc[intr_channel];
                uint8_t old_ep_addr = hc->ep_num | ((hc->ep_is_in) ? 0x80 : 0x00);
                usbh_endpoint_t *old_ep = _find_endpoint(hc->dev_addr, old_ep_addr);
                if (old_ep != NULL)
                {
                    old_ep->toggle_in = hc->toggle_in;
                    old_ep->toggle_out = hc->toggle_out;
                }
                else
                {
                    TU_LOG1("ERROR, could not find ep\n");
                }

                //TU_LOG1("inter %d\n", ep->ep_desc.bInterval);
                ep->ch_num = intr_channel;
                HAL_HCD_HC_Init(hhcd,
                                ep->ch_num,
                                ep->ep_desc.bEndpointAddress,
                                ep->dev_addr,
                                HAL_HCD_GetCurrentSpeed(hhcd), //FIXME tuh_speed_get?
                                ep->ep_type,
                                ep->ep_desc.wMaxPacketSize);
                hc->toggle_in = ep->toggle_in;
                hc->toggle_out = ep->toggle_out;
                HAL_HCD_HC_SubmitRequest(hhcd,
                                         ep->ch_num,
                                         (tu_edpt_dir(ep->ep_desc.bEndpointAddress) == TUSB_DIR_IN) ? 1 : 0,
                                         ep->ep_type,
                                         1,
                                         ep->xfer_buff,
                                         ep->xfer_len,
                                         0);
                ep->last_frame = HAL_HCD_GetCurrentFrame(hhcd);
                intr_in_queue_head = (i + 1) % MAX_ENDPOINTS;
                intr_lock[intr_channel] = ep->dev_addr;
                intr_channel++;
                //TU_LOG1("tx: %02x %d\n", ep->dev_addr, ep->toggle_in);
            }
        }
    }
}

void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t ch_num, HCD_URBStateTypeDef urb_state)
{
    HCD_HCTypeDef *hc = &hhcd->hc[ch_num];
    uint8_t ep_addr = hc->ep_num | ((hc->ep_is_in) ? 0x80 : 0x00);
    usbh_endpoint_t *ep = _find_endpoint(hc->dev_addr, ep_addr);
    uint32_t actual_length = HAL_HCD_HC_GetXferCount(hhcd, ch_num);

    //Interrupt transfer is done, stop the transfer. Wait for user callback to reschedule
    if (hc->ep_type == EP_TYPE_INTR)
    {
        if ((urb_state == URB_DONE || urb_state == URB_STALL || urb_state == URB_ERROR))
        {
            ep->active = 0;
        }
        intr_lock[ch_num] = 0;
    }

    if (urb_state == URB_DONE)
    {
        TU_LOG3("Transfer SUCCESS devadr%02x ep:%02x %d bytes\n", hc->dev_addr, ep_addr, actual_length);
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, actual_length, XFER_RESULT_SUCCESS, true);
    }
    else if (urb_state == URB_STALL)
    {
        TU_LOG1("Transfer ERROR: URB_STALL, state %02x dev addr: %02x\n", hc->state, hc->dev_addr);
        hc->state = HC_IDLE;
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, actual_length, XFER_RESULT_STALLED, true);
    }
    else if (urb_state == URB_ERROR)
    {
        TU_LOG1("Transfer ERROR: URB_ERROR, state %02x dev addr: %02x\n", hc->state, hc->dev_addr);
        hc->state = HC_IDLE;
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, actual_length, XFER_RESULT_FAILED, true);
    }
}

// HCD closes all opened endpoints belong to this device
void hcd_device_close(uint8_t rhport, uint8_t dev_addr)
{
    (void)rhport;
    for (int i = 0; i < MAX_ENDPOINTS; i++)
    {
        usbh_endpoint_t *ep = &usbh_endpoint[i];
        if (ep->allocated == true && ep->dev_addr == dev_addr)
        {
            TU_LOG1("Closing pipe %d on dev %02x\n", i, dev_addr);
            if (hhcd.hc[ep->ch_num].dev_addr == ep->dev_addr)
            {
                tu_memclr(&hhcd.hc[ep->ch_num], sizeof(HCD_HCTypeDef));
            }
            tu_memclr(ep, sizeof(usbh_endpoint_t));
        }
    }
    //Release any interrupt channel locks
    for (int i = 0; i < INTR_CHANNELS; i++)
    {
        if (intr_lock[i] == dev_addr)
        {
            intr_lock[i] = 0;
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
    return root_port_connected;
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
    hhcd.Init.Host_channels = MAX_PIPES;
    hhcd.Init.dma_enable = 0;
    hhcd.Init.low_power_enable = 0;
    hhcd.Init.phy_itface = HCD_PHY_EMBEDDED;
    hhcd.Init.Sof_enable = 1;
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

    usbh_endpoint_t *new_ep = _alloc_endpoint(dev_addr, (tusb_desc_endpoint_t *)ep_desc);
    TU_ASSERT(new_ep != NULL);
    if (new_ep->ep_type != EP_TYPE_INTR)
    {
        //For non interrupt endpoints, I allocate a pipe in the host controller immediately.
        tu_memclr(&hhcd.hc[new_ep->ch_num], sizeof(HCD_HCTypeDef));
        TU_ASSERT(ep_desc->wMaxPacketSize > 0); //FIXME?, I rely on mps > 0 for pipe allocation
        ret = HAL_HCD_HC_Init(&hhcd,
                              new_ep->ch_num,
                              new_ep->ep_desc.bEndpointAddress,
                              new_ep->dev_addr,
                              HAL_HCD_GetCurrentSpeed(&hhcd), //FIXME tuh_speed_get?
                              new_ep->ep_type,
                              new_ep->ep_desc.wMaxPacketSize);
    }
    TU_LOG1("Opened pipe for devaddr:%d epaddr:%02x in slot %d\n", dev_addr, new_ep->ep_desc.bEndpointAddress, new_ep->ch_num);
    return (ret == HAL_OK);
}

bool hcd_setup_send(uint8_t rhport, uint8_t dev_addr, uint8_t const setup_packet[8])
{
    (void)rhport;
    TU_LOG3("%s\n", __FUNCTION__);
    usbh_endpoint_t *ep = _find_endpoint(dev_addr, 0x00);
    HAL_StatusTypeDef ret = HAL_HCD_HC_SubmitRequest(&hhcd,
                                                     ep->ch_num,
                                                     0,
                                                     EP_TYPE_CTRL,
                                                     0,
                                                     (uint8_t *)setup_packet,
                                                     8,
                                                     0);
    return (ret == HAL_OK);
}

bool hcd_edpt_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr, uint8_t *buffer, uint16_t buflen)
{
    (void)rhport;
    HAL_StatusTypeDef ret = HAL_OK;
    usbh_endpoint_t *ep = _find_endpoint(dev_addr, ep_addr);
    TU_ASSERT(ep != NULL);

    ep->active = 1;
    ep->xfer_buff = buffer;
    ep->xfer_len = buflen;

    if (ep->ep_type != EP_TYPE_INTR)
    {
        HCD_HCTypeDef *hc = &hhcd.hc[ep->ch_num];
        if (hc->urb_state == URB_NOTREADY || hc->urb_state == URB_NYET ||
            (hc->state != HC_IDLE && hc->state != HC_XFRC))
        {
            TU_LOG1("Error: Endpoint %02x is busy, urb: %02x hc: %02x\n", ep_addr, hc->urb_state, hc->state);
            TU_ASSERT(0);
        }
        ret = HAL_HCD_HC_SubmitRequest(&hhcd,
                                       ep->ch_num,
                                       (tu_edpt_dir(ep_addr) == TUSB_DIR_IN) ? 1 : 0,
                                       ep->ep_type,
                                       1,
                                       buffer,
                                       buflen,
                                       0);
    }

    TU_LOG2("hcd_edpt_xfer on pipe for devaddr:%d epaddr:%02x in slot %d, ep_type %d, ep_dir %d, len %d\n",
            dev_addr, ep_addr, ep->ch_num, ep->ep_type, tu_edpt_dir(ep_addr), buflen);

    return (ret == HAL_OK);
}

void hcd_int_handler(uint8_t rhport)
{
    HAL_HCD_IRQHandler(&hhcd);
}

bool hcd_edpt_clear_stall(uint8_t dev_addr, uint8_t ep_addr)
{
    usbh_endpoint_t *ep = _find_endpoint(dev_addr, ep_addr);
    if (ep != NULL)
    {
        hhcd.hc[ep->ch_num].urb_state = URB_IDLE;
    }
    return true;
}

#endif