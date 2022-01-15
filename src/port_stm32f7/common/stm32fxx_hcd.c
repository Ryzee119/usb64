/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, hathach (tinyusb.org)
 * Copyright (c) 2022, Ryzee119
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

static HCD_HandleTypeDef hhcd;
static bool root_port_connected = 0;

//STM32F7 is 12 for FS and 16 for HS
#define MAX_PIPES 12
#ifndef MAX_ENDPOINTS
#define MAX_ENDPOINTS 32
#endif

//The amount of shared channels reserved for interrupt transfer queue. If you are polling many interrupt endpoints
//increasing this can increase the overall responsiveness.
//This must be < MAX_PIPES, and MAX_ENDPOINTS - INTR_CHANNELS must allow for free pipes for control, bulk and iso transfers
#ifndef INTR_CHANNELS
#define INTR_CHANNELS 6
#endif
//Single shared control channel just after the reserved interrupt channels
#define CTRL_CHANNEL INTR_CHANNELS

//Locks to make sure we dont use a interrupt pipe thats already in use.
static uint8_t intr_lock[INTR_CHANNELS] = {0};

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
    uint8_t toggle;
} usbh_endpoint_t;

usbh_endpoint_t usbh_endpoint[MAX_ENDPOINTS] = {0};

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
            else if (usbh_endpoint[i].ep_type == EP_TYPE_CTRL)
            {
                usbh_endpoint[i].ch_num = CTRL_CHANNEL;
            }
            //Bulk and iso are allocated their own channel, allocated after interrupt and control channels
            else
            {
                for (int ch_num = CTRL_CHANNEL; ch_num < MAX_PIPES; ch_num++)
                {
                    if (hhcd.hc[ch_num].max_packet == 0)
                    {
                        usbh_endpoint[i].ch_num = ch_num;
                        hhcd.hc[ch_num].max_packet = 8; //Set to a standard non-zero number.
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
                TU_LOG1("ERROR: No more host channels!\n");
                break;
            }
        }
    }
    return NULL;
}

static usbh_endpoint_t *_find_endpoint(uint8_t dev_addr, uint8_t ep_addr)
{
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

static bool _endpoint_xfer(usbh_endpoint_t *ep, uint8_t is_setup)
{
    HAL_StatusTypeDef ret = HAL_OK;
    HCD_HCTypeDef *hc = &hhcd.hc[ep->ch_num];
    uint8_t hc_ep_addr = hc->ep_num | ((hc->ep_is_in) ? 0x80 : 0x00);
    if (hc->dev_addr != ep->dev_addr || hc_ep_addr != ep->ep_desc.bEndpointAddress || hc->dev_addr == 0)
    {
        ret = HAL_HCD_HC_Init(&hhcd,
                              ep->ch_num,
                              ep->ep_desc.bEndpointAddress,
                              ep->dev_addr,
                              HAL_HCD_GetCurrentSpeed(&hhcd), //FIXME tuh_speed_get?
                              ep->ep_type,
                              ep->ep_desc.wMaxPacketSize);
    }

    uint8_t *toggle = (hc->ep_is_in) ? &hc->toggle_in : &hc->toggle_out;
    *toggle = ep->toggle;

    ep->active = 1;
    ep->last_frame = HAL_HCD_GetCurrentFrame(&hhcd);

    if (ret == HAL_OK)
    {
        ret = HAL_HCD_HC_SubmitRequest(&hhcd,
                                       ep->ch_num,
                                       (tu_edpt_dir(ep->ep_desc.bEndpointAddress) == TUSB_DIR_IN) ? 1 : 0,
                                       ep->ep_type,
                                       (is_setup) ? 0 : 1,
                                       (uint8_t *)ep->xfer_buff,
                                       ep->xfer_len,
                                       0);
    }
    return (ret == HAL_OK);
}

static void deferred_resetport(void *param)
{
    HAL_HCD_ResetPort(&hhcd);
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
        usbh_endpoint_t *ep = &usbh_endpoint[ep_index];

        if (ep->active == 0)
        {
            continue;
        }

        if (ep->ep_type == EP_TYPE_INTR && intr_lock[intr_channel] == 0)
        {
            if ((HAL_HCD_GetCurrentFrame(hhcd) - ep->last_frame) >= ep->ep_desc.bInterval)
            {
                ep->ch_num = intr_channel;
                _endpoint_xfer(ep, 0);
                intr_in_queue_head = (i + 1) % MAX_ENDPOINTS;
                intr_lock[intr_channel] = ep->dev_addr;
                intr_channel++;
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
        intr_lock[ch_num] = 0;
    }

    if (urb_state == URB_DONE)
    {
        TU_LOG2("Transfer SUCCESS devaddr %02x ep:%02x %d bytes, channel %d\n", hc->dev_addr, ep_addr, actual_length, ch_num);
        ep->toggle = (hc->ep_is_in) ? hc->toggle_in ^ 1 : hc->toggle_out;
        ep->active = 0;
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, actual_length, XFER_RESULT_SUCCESS, true);
    }
    else if (urb_state == URB_STALL)
    {
        TU_LOG1("Transfer ERROR: URB_STALL, state %02x dev addr: %02x channel %d\n", hc->state, hc->dev_addr, ch_num);
        hc->state = HC_IDLE;
        ep->active = 0;
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, actual_length, XFER_RESULT_STALLED, true);
    }
    else if (urb_state == URB_ERROR || hc->state == HC_XACTERR)
    {
        TU_LOG1("Transfer ERROR: URB_ERROR, state %02x dev addr: %02x channel %d\n", hc->state, hc->dev_addr, ch_num);
        hc->state = HC_IDLE;
        ep->active = 0;
        hcd_event_xfer_complete(hc->dev_addr, ep_addr, actual_length, XFER_RESULT_FAILED, true);
    }
    //Automatically retry output NAK.
    else if (urb_state == URB_NOTREADY && hc->state == HC_NAK && hc->ep_is_in == 0)
    {
        //Retry output NAK. FIXME? Retry count? Only for control endpoint?
        uint32_t tmpreg;
        USB_OTG_GlobalTypeDef *USBx = hhcd->Instance;
        uint32_t USBx_BASE = (uint32_t)USBx;
        tmpreg = USBx_HC(ch_num)->HCCHAR;
        tmpreg &= ~USB_OTG_HCCHAR_CHDIS;
        tmpreg |= USB_OTG_HCCHAR_CHENA;
        USBx_HC(ch_num)->HCCHAR = tmpreg;
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
            TU_LOG2("Closing pipe %d on dev %02x\n", i, dev_addr);
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
    HAL_StatusTypeDef res = HAL_ERROR;
    hhcd.Instance = (rhport == 0) ? USB_OTG_FS : USB_OTG_HS;
    hhcd.Init.Host_channels = MAX_PIPES;
    hhcd.Init.dma_enable = 0;
    hhcd.Init.low_power_enable = 0;
    hhcd.Init.phy_itface =  (rhport == 0) ? HCD_PHY_EMBEDDED : USB_OTG_ULPI_PHY;
    hhcd.Init.Sof_enable = 1;
    hhcd.Init.speed = (rhport == 0) ? HCD_SPEED_FULL : HCD_SPEED_HIGH;
    hhcd.Init.vbus_sensing_enable = 0;
    hhcd.Init.lpm_enable = 0;
    res = HAL_HCD_Init(&hhcd);
    if (res == HAL_OK)
    {
        res = HAL_HCD_Start(&hhcd);
    }
    HAL_NVIC_EnableIRQ((rhport == 0) ? OTG_FS_IRQn : OTG_HS_IRQn);
    return (res == HAL_OK);
}

bool hcd_edpt_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_endpoint_t const *ep_desc)
{
    (void)rhport;

    usbh_endpoint_t *ep = _alloc_endpoint(dev_addr, (tusb_desc_endpoint_t *)ep_desc);
    TU_ASSERT(ep != NULL);
    TU_LOG2("Opened pipe for devaddr:%d epaddr:%02x in slot %d\n", dev_addr, ep->ep_desc.bEndpointAddress, ep->ch_num);
    if (ep_desc->bEndpointAddress == 0x00)
    {
        //If control endpoint, also opening IN pipe
        ep = _alloc_endpoint(dev_addr, (tusb_desc_endpoint_t *)ep_desc);
        ep->ep_desc.bEndpointAddress |= 0x80;
        TU_ASSERT(ep != NULL);
        TU_LOG2("Opened pipe for devaddr:%d epaddr:%02x in slot %d\n", dev_addr, ep->ep_desc.bEndpointAddress, ep->ch_num);
    }
    return true;
}

bool hcd_setup_send(uint8_t rhport, uint8_t dev_addr, uint8_t const setup_packet[8])
{
    (void)rhport;

    usbh_endpoint_t *ep = _find_endpoint(dev_addr, 0x00);
    TU_ASSERT(ep != NULL);

    ep->xfer_buff = (uint8_t *)setup_packet;
    ep->xfer_len = 8;

    TU_LOG2("hcd_setup_send on pipe for devaddr:%d in slot %d, ep_type %d, len %d\n",
            dev_addr, ep->ch_num, ep->ep_type, 8);

    return _endpoint_xfer(ep, 1);
}

bool hcd_edpt_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr, uint8_t *buffer, uint16_t buflen)
{
    (void)rhport;
    bool ret = true;
    usbh_endpoint_t *ep = _find_endpoint(dev_addr, ep_addr);
    TU_ASSERT(ep != NULL);

    ep->xfer_buff = buffer;
    ep->xfer_len = buflen;

    TU_LOG2("hcd_edpt_xfer on pipe for devaddr:%d epaddr:%02x in slot %d, ep_type %d, ep_dir %d, len %d\n",
            ep->dev_addr, ep->ep_desc.bEndpointAddress, ep->ch_num, ep->ep_type, tu_edpt_dir(ep_addr), buflen);

    //Non interrupt endpoints, queue immediately.
    if (ep->ep_type != EP_TYPE_INTR)
    {
        ret = _endpoint_xfer(ep, 0);
    }
    //Otherwise, mark it as after and it will be schedule in SOF interrupt
    else
    {
        ep->active = 1;
    }

    return ret;
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