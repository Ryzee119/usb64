// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "common.h"
#include "n64_controller.h"
#include "input.h"
#include "tft.h"
#include "tusb.h"

#define MAX_USB_CONTROLLERS (CFG_TUH_DEVICE_MAX)

input_driver_t input_devices[MAX_USB_CONTROLLERS];

uint32_t hardwired1;

static input_driver_t *find_slot(uint16_t uid)
{
    //See if input device already exists
    for (int i = 0; i < MAX_USB_CONTROLLERS; i++)
    {
        if (input_devices[i].uid == uid && uid > 0)
        {
            return &input_devices[i];
        }
    }
    //Allocate new input device
    for (int i = 0; i < MAX_USB_CONTROLLERS; i++)
    {
        if (input_devices[i].type == INPUT_NONE && uid == 0)
        {
            input_devices[i].slot = i;
            return &input_devices[i];
        }
    }
    return NULL;
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
{
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    if (itf_protocol != HID_ITF_PROTOCOL_NONE)
    {
        input_driver_t *in_dev = find_slot(0);

        in_dev->type    = (itf_protocol == HID_ITF_PROTOCOL_MOUSE) ? INPUT_MOUSE : INPUT_KEYBOARD;
        in_dev->backend = (itf_protocol == HID_ITF_PROTOCOL_MOUSE) ? BACKEND_HID_MOUSE : BACKEND_HID_KEYBOARD;
        in_dev->uid = dev_addr << 8 | instance;
        in_dev->set_rumble = NULL;
        in_dev->set_led = NULL;
        in_dev->data = in_dev->_data;

        tuh_hid_receive_report(dev_addr, instance);
    }
}

// Invoked when received report from device via interrupt endpoint
static uint32_t hid_data_tick[MAX_USB_CONTROLLERS] = {0};
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
    input_driver_t *in_dev = find_slot(dev_addr << 8 | instance);

    if (in_dev != NULL && itf_protocol != HID_ITF_PROTOCOL_NONE)
    {
        hid_data_tick[in_dev->slot] = n64hal_millis();
        memcpy(in_dev->data, report, TU_MIN(len, CFG_TUH_XINPUT_EPIN_BUFSIZE));
    }

    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    uint16_t uid = dev_addr << 8 | instance;
    input_driver_t *in_dev = find_slot(uid);
    while (in_dev != NULL)
    {
        memset(in_dev, 0, sizeof(input_driver_t));
        in_dev = find_slot(uid);
    }
}

void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    xinputh_interface_t *xid_itf = (xinputh_interface_t *)report;
    uint16_t uid = dev_addr << 8 | instance;
    input_driver_t *in_dev = find_slot(uid);
    if (in_dev != NULL && xid_itf->new_pad_data == true)
    {
        memcpy(in_dev->data, &xid_itf->pad, TU_MIN(len, CFG_TUH_XINPUT_EPIN_BUFSIZE));
    }
    else if (in_dev == NULL && xid_itf->type == XBOX360_WIRELESS)
    {
        if (xid_itf->connected == true)
        {
            tuh_xinput_mount_cb(dev_addr, instance, xid_itf);
        }
    }
    else if (in_dev != NULL && xid_itf->type == XBOX360_WIRELESS)
    {
        if (xid_itf->connected == false)
        {
            tuh_xinput_umount_cb(dev_addr, instance);
        }
    }
    tuh_xinput_receive_report(dev_addr, instance);
}

void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, const xinputh_interface_t *xinput_itf)
{
    //If this is a Xbox 360 Wireless controller, dont register yet. Need to wait for a connection packet
    //on the in pipe.
    if (xinput_itf->type == XBOX360_WIRELESS && xinput_itf->connected == false)
    {
        tuh_xinput_receive_report(dev_addr, instance);
        return;
    }

    input_driver_t *in_dev = find_slot(0);

    in_dev->type = INPUT_GAMECONTROLLER;
    in_dev->backend = BACKEND_XINPUT;
    in_dev->uid = dev_addr << 8 | instance;
    in_dev->set_rumble = tuh_xinput_set_rumble;
    in_dev->set_led = tuh_xinput_set_led;
    in_dev->data = in_dev->_data;

    tuh_xinput_set_led(dev_addr, instance, 0, true);
    tuh_xinput_set_led(dev_addr, instance, in_dev->slot + 1, true);
    tuh_xinput_set_rumble(dev_addr, instance, 0, 0, true);
    tuh_xinput_receive_report(dev_addr, instance);
}

void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    uint16_t uid = dev_addr << 8 | instance;
    input_driver_t *in_dev = find_slot(uid);
    while (in_dev != NULL)
    {
        memset(in_dev, 0, sizeof(input_driver_t));
        in_dev = find_slot(uid);
    }
}

static int _check_id(uint8_t id)
{
    if (id >= MAX_USB_CONTROLLERS)
        return 0;

    if (input_devices[id].backend == BACKEND_NONE)
        return 0;

    return 1;
}

FLASHMEM void input_init()
{
    tusb_init();
    memset(input_devices, 0x00, sizeof(input_devices));
}

void input_update_input_devices()
{
    tuh_task();
}

uint16_t input_get_state(uint8_t id, void *response, bool *combo_pressed)
{
    *combo_pressed = 0;

    if (_check_id(id) == 0)
        return 0;

    if (response == NULL)
        return 0;

    input_driver_t *in_dev = &input_devices[id];

    if (in_dev == NULL)
        return 0;

    if (input_is(id, INPUT_GAMECONTROLLER))
    {
        //Prep N64 response
        n64_buttonmap *state = (n64_buttonmap *)response;
        state->dButtons = 0;

        int32_t right_axis[2] = {0};
        if (in_dev->backend == BACKEND_XINPUT)
        {
            xinput_gamepad_t *pad = (xinput_gamepad_t *)in_dev->data;
            if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)         state->dButtons |= N64_DU;  //DUP
            if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)       state->dButtons |= N64_DD;  //DDOWN
            if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)       state->dButtons |= N64_DL;  //DLEFT
            if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)      state->dButtons |= N64_DR;  //DRIGHT
            if (pad->wButtons & XINPUT_GAMEPAD_START)           state->dButtons |= N64_ST;  //START
            if (pad->wButtons & XINPUT_GAMEPAD_BACK)            state->dButtons |= 0;       //BACK
            if (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB)      state->dButtons |= 0;       //LS
            if (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)     state->dButtons |= 0;       //RS
            if (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)   state->dButtons |= N64_LB;  //LB
            if (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)  state->dButtons |= N64_RB;  //RB
            if (pad->wButtons & (1 << 10))                      state->dButtons |= 0;       //XBOX BUTTON
            if (pad->wButtons & (1 << 11))                      state->dButtons |= 0;       //XBOX SYNC
            if (pad->wButtons & XINPUT_GAMEPAD_A)               state->dButtons |= N64_A;   //A
            if (pad->wButtons & XINPUT_GAMEPAD_B)               state->dButtons |= N64_B;   //B
            if (pad->wButtons & XINPUT_GAMEPAD_X)               state->dButtons |= N64_B;   //X
            if (pad->wButtons & XINPUT_GAMEPAD_Y)               state->dButtons |= 0;       //Y
            if (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)     state->dButtons |= N64_CU | //RS triggers
                                                                                   N64_CD | //all C usb_buttons
                                                                                   N64_CL |
                                                                                   N64_CR;

            //Analog stick (Normalise 0 to +/-100)
            state->x_axis = pad->sThumbLX * 100 / 32768;
            state->y_axis = pad->sThumbLY * 100 / 32768;

            //Z button
            if (pad->bLeftTrigger > 10) state->dButtons |= N64_Z; //LT
            if (pad->bRightTrigger > 10) state->dButtons |= N64_Z; //RT

            //C usb_buttons
            if (pad->sThumbRX > 16000)  state->dButtons |= N64_CR;
            if (pad->sThumbRX < -16000) state->dButtons |= N64_CL;
            if (pad->sThumbRY > 16000)  state->dButtons |= N64_CU;
            if (pad->sThumbRY < -16000) state->dButtons |= N64_CD;

            //Button to hold for 'combos'
            *combo_pressed = (pad->wButtons & XINPUT_GAMEPAD_BACK); //back

            //Map right axis for dual stick mode
            right_axis[0] = pad->sThumbRX * 100 / 32768;
            right_axis[1] = pad->sThumbRY * 100 / 32768;
        }

        //Use 2.4 GOODHEAD layout, axis not inverted
        if(input_is_dualstick_mode(id) && (id % 2 == 0))
        {
            //Main controller mapping overwritten for dualstick mode
            state->dButtons &= ~N64_Z;
            if (state->dButtons & N64_LB)
                state->dButtons |= N64_Z;
            state->dButtons &= ~N64_LB;
            state->dButtons &= ~N64_RB;
            state->x_axis = right_axis[0];
            state->y_axis = right_axis[1];
        }
        else if(input_is_dualstick_mode(id) && (id % 2 != 0))
        {
            //Mirror controller mapping overwritten for dualstick mode
            if (state->dButtons & N64_RB)
                state->dButtons |= N64_Z;
            state->dButtons &= N64_Z;
        }

        //Assert reset bit if L+R+START is pressed. Start bit is cleared.
        if ((state->dButtons & N64_LB) && (state->dButtons & N64_RB) && (state->dButtons & N64_ST))
        {
            state->dButtons &= ~N64_ST;
            state->dButtons |= N64_RES;
        }
    }
    else if (input_is(id, INPUT_MOUSE))
    {
        //N64 report is basically a n64 controller response
        n64_buttonmap *state = (n64_buttonmap *)response;
        hid_mouse_report_t *report = (hid_mouse_report_t *)in_dev->data;
        bool new_data = state->x_axis != report->x || state->y_axis != -report->y;

        state->dButtons = 0;
        state->x_axis = report->x;
        state->y_axis = -report->y;

        if (report->buttons & (1 << 0)) state->dButtons |= N64_A;   //A left click
        if (report->buttons & (1 << 1)) state->dButtons |= N64_B;   //B right click
        if (report->buttons & (1 << 2)) state->dButtons |= N64_ST;  //ST middle click

        //Need to do this for stale data and mouse wont send a 0 byte if its not moving
        if (n64hal_millis() - hid_data_tick[in_dev->slot] > 10)
        {
            report->x = 0;
            report->y = 0;
        }
    }

    return 1;
}

void input_apply_rumble(int id, uint8_t strength)
{
    if (_check_id(id) == 0)
        return;

    input_driver_t *in_dev = &input_devices[id];
    if (in_dev->set_rumble)
    {
        uint8_t dev_addr = in_dev->uid >> 8;
        uint8_t instance = in_dev->uid & 0xFF;
        in_dev->set_rumble(dev_addr, instance, strength, strength, true);
    }
}

bool input_is_connected(int id)
{
    if (id >= MAX_USB_CONTROLLERS)
        return false;

    return (input_devices[id].backend != BACKEND_NONE);
}

bool input_is(int id, input_type_t type)
{
    if (id >= MAX_USB_CONTROLLERS)
        return false;

    return (input_devices[id].type == type);
}

uint16_t input_get_id_product(int id)
{
    if (_check_id(id) == 0)
        return 0;

    input_driver_t *in_dev = &input_devices[id];

    uint16_t pid, vid;
    tuh_vid_pid_get(in_dev->uid >> 8, &vid, &pid);
    return pid;
}

uint16_t input_get_id_vendor(int id)
{
    if (_check_id(id) == 0)
        return 0;

    input_driver_t *in_dev = &input_devices[id];

    uint16_t pid, vid;
    tuh_vid_pid_get(in_dev->uid >> 8, &vid, &pid);
    return vid;
}

const char *input_get_manufacturer_string(int id)
{
    //FIXME
    static const char NC[] = "NOT CONNECTED";
    if (_check_id(id) == 0 || input_is_connected(id) == false)
        return NC;

    return "USB64";
}

const char *input_get_product_string(int id)
{
    //FIXME
    static const char NC[] = "";
    return NC;
}

void input_enable_dualstick_mode(int id)
{
    if (_check_id(id) == 0)
        return;

    //Only player 1 or player 3
    if ((id != 0 && id != 2) || id + 1 >= MAX_CONTROLLERS)
        return;

    //Copy driver into the next slot to make a 'fake' input
    memcpy(&input_devices[id + 1], &input_devices[id], sizeof(input_driver_t));
}

void input_disable_dualstick_mode(int id)
{
    if (_check_id(id) == 0)
        return;

    if ((id != 0 && id != 2) || id + 1 >= MAX_CONTROLLERS)
        return;

    //Clear the 'fake' controller driver
    memset(&input_devices[id + 1], 0, sizeof(input_driver_t ));
}

bool input_is_dualstick_mode(int id)
{
    if (_check_id(id) == 0)
        return false;

    //Check if this is a 'fake' mirror of a main controller
    if ((id == 1 || id == 3) && input_devices[id].uid == input_devices[id - 1].uid)
        return true;

    if (id + 1 >= MAX_CONTROLLERS)
        return false;

    //Check if this is a main controller that is mirrored
    if (input_devices[id].uid == input_devices[id + 1].uid)
        return true;
    
    return false;
}
