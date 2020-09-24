// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "USBHost_t36.h"
#include "usb64_conf.h"
#include "n64_controller.h"
#include "input.h"
#include "printf.h"

//USB Host Interface
USBHost usbh;

#if (ENABLE_USB_HUB == 1)
USBHub hub1(usbh);
#endif

#if (MAX_CONTROLLERS >= 1)
JoystickController joy1(usbh);
#if (MAX_MICE >= 1)
USBHIDParser hid1(usbh);
MouseController mouse1(usbh);
#endif
#endif
#if (MAX_CONTROLLERS >= 2)
JoystickController joy2(usbh);
#if (MAX_MICE >= 2)
USBHIDParser hid2(usbh);
MouseController mouse2(usbh);
#endif
#endif
#if (MAX_CONTROLLERS >= 3)
JoystickController joy3(usbh);
#if (MAX_MICE >= 3)
USBHIDParser hid3(usbh);
MouseController mouse3(usbh);
#endif
#endif
#if (MAX_CONTROLLERS >= 4)
JoystickController joy4(usbh);
#if (MAX_MICE >= 4)
USBHIDParser hid4(usbh);
MouseController mouse4(usbh);
#endif
#endif

#if MAX_CONTROLLERS == 1
JoystickController *gamecontroller[] = {&joy1};
#elif MAX_CONTROLLERS == 2
JoystickController *gamecontroller[] = {&joy1, &joy2};
#elif MAX_CONTROLLERS == 3
JoystickController *gamecontroller[] = {&joy1, &joy2, &joy3};
#elif MAX_CONTROLLERS == 4
JoystickController *gamecontroller[] = {&joy1, &joy2, &joy3, &joy4};
#endif

#if MAX_MICE == 1
MouseController *mousecontroller[] = {&mouse1, NULL, NULL, NULL};
#elif MAX_MICE == 2
MouseController *mousecontroller[] = {&mouse1, &mouse2, NULL, NULL};
#elif MAX_MICE == 3
MouseController *mousecontroller[] = {&mouse1, &mouse2, &mouse3.NULL};
#elif MAX_MICE == 4
MouseController *mousecontroller[] = {&mouse1, &mouse2, &mouse3, &mouse4};
#endif

#define MOUSE 0
#define USB_GAMECONTROLLER 1
#define HW_GAMECONTROLLER 2
#define I2C_GAMECONTROLLER 3

typedef struct
{
    void *driver;
    int type;
} input;

static input input_devices[MAX_CONTROLLERS];

static int _check_id(uint8_t id)
{
    if (id > MAX_CONTROLLERS)
        return 0;
    if (input_devices[id].driver == NULL)
        return 0;

    return 1;
}

void input_init()
{
    usbh.begin();
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        input_devices[i].driver = NULL;
        input_devices[i].type = USB_GAMECONTROLLER;
    }
}

void input_update_input_devices()
{
    //Clear disconnected devices
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        if (input_is_connected(i) == 0)
        {
            if (input_devices[i].driver != NULL)
                debug_print_status("[INPUT] Cleared device from slot %u\n", i);
            input_devices[i].driver = NULL;
        }
    }
    //Find new game controllers
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        //Game controller is connected
        if (*(gamecontroller[i]) == true)
        {
            //Is it already a registered input device
            bool already_registered = false;
            for (int j = 0; j < MAX_CONTROLLERS; j++)
            {
                if (gamecontroller[i] == input_devices[j].driver)
                {
                    already_registered = true;
                    break;
                }
            }
            //Its a new controller, find empty slot and register it now
            if (already_registered == false)
            {
                for (int j = 0; j < MAX_CONTROLLERS; j++)
                {
                    if (input_devices[j].driver == NULL)
                    {
                        input_devices[j].driver = gamecontroller[i];
                        input_devices[j].type = USB_GAMECONTROLLER;
                        debug_print_status("[INPUT] Registered gamecontroller to slot %u\n", j);
                        break;
                    }
                }
            }
        }
    }
#if (MAX_MICE >= 1)
    //Find new mice
    for (int i = 0; i < MAX_MICE; i++)
    {
        //Mouse is connected
        USBHIDInput *m = (USBHIDInput *)mousecontroller[i];
        if (*m == true)
        {
            //Is it already a registered input device
            bool already_registered = false;
            for (int j = 0; j < MAX_CONTROLLERS; j++)
            {
                if (mousecontroller[i] == input_devices[j].driver)
                {
                    already_registered = true;
                    break;
                }
            }
            //Its a new mouse, find empty slot and register it now
            if (already_registered == false)
            {
                for (int j = 0; j < MAX_CONTROLLERS; j++)
                {
                    if (input_devices[j].driver == NULL)
                    {
                        input_devices[j].driver = mousecontroller[i];
                        input_devices[j].type = MOUSE;
                        debug_print_status("[INPUT] Register mouse to slot %u\n", j);
                        break;
                    }
                }
            }
        }
    }
#endif
}

uint16_t input_get_buttons(uint8_t id, uint32_t *raw_buttons, int32_t *raw_axis, uint32_t max_axis,
                                       uint16_t *n64_buttons, int8_t *n64_x_axis, int8_t *n64_y_axis, bool *combo_pressed)
{
    uint32_t _buttons = 0;
    int32_t _axis[max_axis] = {0};

    if (_check_id(id) == 0)
        return 0;

    if (input_is_gamecontroller(id))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        _buttons = joy->getButtons();

        for (uint8_t i = 0; i < max_axis; i++)
        {
            _axis[i] = joy->getAxis(i);
        }
        joy->joystickDataClear();

        switch (joy->joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            //Digital usb_buttons
            if (n64_buttons == NULL || n64_x_axis == NULL || n64_y_axis == NULL)
                break;
            if (_buttons & (1 << 0))  *n64_buttons |= N64_DU;  //DUP
            if (_buttons & (1 << 1))  *n64_buttons |= N64_DD;  //DDOWN
            if (_buttons & (1 << 2))  *n64_buttons |= N64_DL;  //DLEFT
            if (_buttons & (1 << 3))  *n64_buttons |= N64_DR;  //DRIGHT
            if (_buttons & (1 << 4))  *n64_buttons |= N64_ST;  //START
            if (_buttons & (1 << 5))  *n64_buttons |= 0;       //BACK
            if (_buttons & (1 << 6))  *n64_buttons |= 0;       //LS
            if (_buttons & (1 << 7))  *n64_buttons |= 0;       //RS
            if (_buttons & (1 << 8))  *n64_buttons |= N64_LB;  //LB
            if (_buttons & (1 << 9))  *n64_buttons |= N64_RB;  //RB
            if (_buttons & (1 << 10)) *n64_buttons |= 0;       //XBOX BUTTON
            if (_buttons & (1 << 11)) *n64_buttons |= 0;       //XBOX SYNC
            if (_buttons & (1 << 12)) *n64_buttons |= N64_A;   //A
            if (_buttons & (1 << 13)) *n64_buttons |= N64_B;   //B
            if (_buttons & (1 << 14)) *n64_buttons |= N64_B;   //X
            if (_buttons & (1 << 15)) *n64_buttons |= 0;       //Y
            if (_buttons & (1 << 7))  *n64_buttons |= N64_CU | //RS triggers
                                                            N64_CD | //all C usb_buttons
                                                            N64_CL |
                                                            N64_CR;
            //Analog stick (Normalise 0 to +/-100)
            *n64_x_axis = _axis[0] * 100 / 32768;
            *n64_y_axis = _axis[1] * 100 / 32768;

            //Z button
            if (_axis[4] > 10) *n64_buttons |= N64_Z; //LT
            if (_axis[5] > 10) *n64_buttons |= N64_Z; //RT

            //C usb_buttons
            if (_axis[2] > 16000)  *n64_buttons |= N64_CR;
            if (_axis[2] < -16000) *n64_buttons |= N64_CL;
            if (_axis[3] > 16000)  *n64_buttons |= N64_CU;
            if (_axis[3] < -16000) *n64_buttons |= N64_CD;

            //Button to hold for 'combos'
            if (combo_pressed)
                *combo_pressed = (_buttons & (1 << 5)); //back
            break;
        //TODO: OTHER USB CONTROLLERS
        default:
            break;
        }

        //Use 2.4 GOODHEAD layout, axis not inverted
        if(input_is_dualstick_mode(id) && (id % 2 == 0))
        {
            //Main controller mapping overwritten for dualstick mode
            *n64_buttons &= ~N64_Z;
            if (*n64_buttons & N64_LB)
                *n64_buttons |= N64_Z;
            *n64_x_axis = _axis[2] * 100 / 32768;
            *n64_y_axis = _axis[3] * 100 / 32768;
        }
        else if(input_is_dualstick_mode(id) && (id % 2 != 0))
        {
            //Mirror controller mapping overwritten for dualstick mode
            if (*n64_buttons & N64_RB)
                *n64_buttons |= N64_Z;
        }
    }
#if (MAX_MICE >= 1)
    else if (input_is_mouse(id))
    {
        MouseController *mouse = (MouseController *)input_devices[id].driver;
        _buttons = mouse->getButtons();

        _axis[0] = mouse->getMouseX();
        _axis[1] = mouse->getMouseY();

        static uint32_t idle_timer[4] = {0};
        if (mouse->available()) idle_timer[id] = millis();
        mouse->mouseDataClear();
        if (millis() - idle_timer[id] > 100)
        {
            _axis[0] = 0;
            _axis[1] = 0;
        }

        //Mouse input is pretty standard, Map to N64 mouse
        if (_axis[0] * MOUSE_SENSITIVITY > 127) _axis[0] = 127;
        if (_axis[1] * MOUSE_SENSITIVITY > 127) _axis[1] = 127;
        if (_axis[0] * MOUSE_SENSITIVITY < -128) _axis[0] = -128;
        if (_axis[1] * MOUSE_SENSITIVITY < -128) _axis[1] = -128;
        if (n64_x_axis) *n64_x_axis =  _axis[0] * MOUSE_SENSITIVITY;
        if (n64_y_axis) *n64_y_axis = -_axis[1] * MOUSE_SENSITIVITY;
        if (n64_buttons)
        {
            if (_buttons & (1 << 0)) *n64_buttons |= N64_A;   //A
            if (_buttons & (1 << 1)) *n64_buttons |= N64_B;   //B
            if (_buttons & (1 << 2)) *n64_buttons |= N64_ST;  //ST
        }
    }
#endif
    else
    {
        return 0;
    }

    //Output the raw data too
    if (raw_buttons) *raw_buttons = _buttons;
    for (uint32_t i = 0; i < max_axis; i ++)
    {
        if (raw_axis) raw_axis[i] = _axis[i];
    }

    return 1;
}

void input_apply_rumble(int id, uint8_t stength)
{
    JoystickController *joy;
    if (input_is_gamecontroller(id))
    {
        joy = (JoystickController *)input_devices[id].driver;
        joy->setRumble(stength, stength, 20);
    }
}

bool input_is_connected(int id)
{
    bool connected = false;
    if (_check_id(id) == 0)
        return false;

    if (input_is_gamecontroller(id))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        if (*joy == true)
            connected = true;
    }

    else if (input_is_mouse(id))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        if (*mouse == true)
            connected = true;
    }

    return connected;
}

bool input_is_mouse(int id)
{
    if (_check_id(id) == 0)
        return false;
    if (input_devices[id].type == MOUSE)
        return true;
    return false;
}

bool input_is_gamecontroller(int id)
{
    if (_check_id(id) == 0)
        return false;
    if (input_devices[id].type == USB_GAMECONTROLLER)
        return true;
    return false;
}

uint16_t input_get_id_product(int id)
{
    if (_check_id(id) == 0 || input_is_connected(id) == 0)
        return 0;

    if (input_is_gamecontroller(id))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        return joy->idProduct();
    }
    else if (input_is_mouse(id))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        return mouse->idProduct();
    }

    return 0;
}

uint16_t input_get_id_vendor(int id)
{
    if (_check_id(id) == 0 || input_is_connected(id) == 0)
        return 0;

    if (input_is_gamecontroller(id))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        return joy->idVendor();
    }
    else if (input_is_mouse(id))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        return mouse->idVendor();
    }

    return 0;
}

const char *input_get_manufacturer_string(int id)
{
    static const char NC[] = "NOT CONNECTED";
    if (_check_id(id) == 0 || input_is_connected(id) == false)
        return NC;

    if (input_is_gamecontroller(id))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        return (const char *)joy->manufacturer();
    }
    else if (input_is_mouse(id))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        return (const char *)mouse->manufacturer();
    }

    return NC;
}

const char *input_get_product_string(int id)
{
    static const char NC[] = "";
    if (_check_id(id) == 0 || input_is_connected(id) == 0)
        return NC;

    if (input_is_gamecontroller(id))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        return (const char *)joy->product();
    }
    else if (input_is_mouse(id))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        return (const char *)mouse->product();
    }

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
    memcpy(&input_devices[id + 1], &input_devices[id], sizeof(input));

}

void input_disable_dualstick_mode(int id)
{
    if (_check_id(id) == 0)
        return;

    if ((id != 0 && id != 2) || id + 1 >= MAX_CONTROLLERS)
        return;

    //Clear the 'fake' controller driver
    input_devices[id + 1].driver = NULL;

}

bool input_is_dualstick_mode(int id)
{
    if (_check_id(id) == 0)
        return false;

    //Check if this is a 'fake' mirror of a main controller
    if ((id == 1 || id == 3) && input_devices[id].driver == input_devices[id - 1].driver)
        return true;

    if (id + 1 >= MAX_CONTROLLERS)
        return false;

    //Check if this is a main controller that is mirrored
    if (input_devices[id].driver == input_devices[id + 1].driver)
        return true;
    
    return false;
}