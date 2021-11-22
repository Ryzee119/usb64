// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include "USBHost_t36.h"
#include "usb64_conf.h"
#include "n64_controller.h"
#include "input.h"
#include "printf.h"
#include "tft.h"

//USB Host Interface
USBHost usbh;

#if (ENABLE_USB_HUB == 1)
USBHub hub1(usbh);
USBHub hub2(usbh);
USBHub hub3(usbh);
USBHub hub4(usbh);
#endif

JoystickController joy1(usbh);
JoystickController joy2(usbh);
JoystickController joy3(usbh);
JoystickController joy4(usbh);
JoystickController joy5(usbh);
JoystickController joy6(usbh);
JoystickController joy7(usbh);
JoystickController joy8(usbh);

USBHIDParser hid1(usbh);
USBHIDParser hid2(usbh);
USBHIDParser hid3(usbh);
USBHIDParser hid4(usbh);
MouseController mouse1(usbh);
MouseController mouse2(usbh);
MouseController mouse3(usbh);
MouseController mouse4(usbh);
KeyboardController kb1(usbh);
KeyboardController kb2(usbh);
KeyboardController kb3(usbh);
KeyboardController kb4(usbh);

JoystickController *gamecontroller[] = {&joy1, &joy2, &joy3, &joy4, &joy5, &joy6, &joy7, &joy8};
MouseController *mousecontroller[] = {&mouse1, &mouse2, &mouse3, &mouse4};
KeyboardController *kbcontroller[] = {&kb1, &kb2, &kb3, &kb4};

uint32_t hardwired1;

#define MAX_USB_CONTROLLERS (8)
static input input_devices[MAX_CONTROLLERS];
static uint8_t kb_keys_pressed[RANDNET_MAX_BUTTONS];

static void kb_pressed_cb(uint8_t keycode)
{
    //Check if its already pressed
    for (int i = 0; i < RANDNET_MAX_BUTTONS; i++)
    {
        if (kb_keys_pressed[i] == keycode)
        {
            return;
        }
    }
    //Register the keypress
    for (int i = 0; i < RANDNET_MAX_BUTTONS; i++)
    {
        if (kb_keys_pressed[i] == 0)
        {
            kb_keys_pressed[i] = keycode;
            break;
        }
    }
}

static void kb_released_cb(uint8_t keycode)
{
    for (int i = 0; i < RANDNET_MAX_BUTTONS; i++)
    {
        if (kb_keys_pressed[i] == keycode)
        {
            kb_keys_pressed[i] = 0;
        }
    }
}

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
            {
                debug_print_status("[INPUT] Cleared device from slot %u\n", i);
                tft_flag_update();
            }
            input_devices[i].driver = NULL;
        }
    }

#if (ENABLE_HARDWIRED_CONTROLLER >=1)
    //Find hardwired game controller
    if (digitalRead(HW_EN) == 0)
    {
        //Hardwired will always overwrite the first slot
        if (input_devices[0].driver != &hardwired1)
        {
            input_devices[0].driver = &hardwired1;
            input_devices[0].type = HW_GAMECONTROLLER;
            debug_print_status("[INPUT] Registered hardwired gamecontroller to slot %u\n", 0);
            tft_flag_update();
        }
    }
#endif

    //Find new game controllers
    for (uint32_t i = 0; i < MAX_USB_CONTROLLERS; i++)
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
                        gamecontroller[i]->setLEDs(j + 2);
                        tft_flag_update();
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
                        input_devices[j].type = USB_MOUSE;
                        debug_print_status("[INPUT] Register mouse to slot %u\n", j);
                        tft_flag_update();
                        break;
                    }
                }
            }
        }
    }
#endif
#if (MAX_KB >= 1)
    //Find new keyboard
    for (int i = 0; i < MAX_KB; i++)
    {
        //Keyboard is connected
        USBHIDInput *kb = (USBHIDInput *)kbcontroller[i];
        if (*kb == true)
        {
            //Is it already a registered input device
            bool already_registered = false;
            for (int j = 0; j < MAX_CONTROLLERS; j++)
            {
                if (kbcontroller[i] == input_devices[j].driver)
                {
                    already_registered = true;
                    break;
                }
            }
            //Its a new keyboard, find empty slot and register it now
            if (already_registered == false)
            {
                for (int j = 0; j < MAX_CONTROLLERS; j++)
                {
                    if (input_devices[j].driver == NULL)
                    {
                        input_devices[j].driver = kbcontroller[i];
                        input_devices[j].type = USB_KB;
                        debug_print_status("[INPUT] Register keyboard to slot %u\n", j);
                        tft_flag_update();
                        kbcontroller[i]->attachRawPress(kb_pressed_cb);
                        kbcontroller[i]->attachRawRelease(kb_released_cb);
                        break;
                    }
                }
            }
        }
    }
#endif
}

uint16_t input_get_state(uint8_t id, void *response, bool *combo_pressed)
{
    uint32_t _buttons = 0;
    int32_t _axis[JoystickController::STANDARD_AXIS_COUNT] = {0};
    int32_t right_axis[2] = {0};

    *combo_pressed = 0;

    if (_check_id(id) == 0)
        return 0;

    if (response == NULL)
        return 0;

    if (input_is(id, USB_GAMECONTROLLER))
    {
        //Prep N64 response
        n64_buttonmap *state = (n64_buttonmap *)response;
        state->dButtons = 0;

        //Get latest info from USB devices
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        _buttons = joy->getButtons();

        for (uint8_t i = 0; i < JoystickController::STANDARD_AXIS_COUNT; i++)
        {
            _axis[i] = joy->getAxis(i);
        }
        joy->joystickDataClear();

        switch (joy->joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            //Digital usb_buttons
            //FIXME Modifier to make A,B,X,Y be C buttons
            if (_buttons & (1 << 0))  state->dButtons |= N64_DU;  //DUP
            if (_buttons & (1 << 1))  state->dButtons |= N64_DD;  //DDOWN
            if (_buttons & (1 << 2))  state->dButtons |= N64_DL;  //DLEFT
            if (_buttons & (1 << 3))  state->dButtons |= N64_DR;  //DRIGHT
            if (_buttons & (1 << 4))  state->dButtons |= N64_ST;  //START
            if (_buttons & (1 << 5))  state->dButtons |= 0;       //BACK
            if (_buttons & (1 << 6))  state->dButtons |= 0;       //LS
            if (_buttons & (1 << 7))  state->dButtons |= 0;       //RS
            if (_buttons & (1 << 8))  state->dButtons |= N64_LB;  //LB
            if (_buttons & (1 << 9))  state->dButtons |= N64_RB;  //RB
            if (_buttons & (1 << 10)) state->dButtons |= 0;       //XBOX BUTTON
            if (_buttons & (1 << 11)) state->dButtons |= 0;       //XBOX SYNC
            if (_buttons & (1 << 12)) state->dButtons |= N64_A;   //A
            if (_buttons & (1 << 13)) state->dButtons |= N64_B;   //B
            if (_buttons & (1 << 14)) state->dButtons |= N64_B;   //X
            if (_buttons & (1 << 15)) state->dButtons |= 0;       //Y
            if (_buttons & (1 << 7))  state->dButtons |= N64_CU | //RS triggers
                                                      N64_CD | //all C usb_buttons
                                                      N64_CL |
                                                      N64_CR;
            //Analog stick (Normalise 0 to +/-100)
            state->x_axis = _axis[0] * 100 / 32768;
            state->y_axis = _axis[1] * 100 / 32768;

            //Z button
            if (_axis[4] > 10) state->dButtons |= N64_Z; //LT
            if (_axis[5] > 10) state->dButtons |= N64_Z; //RT

            //C usb_buttons
            if (_axis[2] > 16000)  state->dButtons |= N64_CR;
            if (_axis[2] < -16000) state->dButtons |= N64_CL;
            if (_axis[3] > 16000)  state->dButtons |= N64_CU;
            if (_axis[3] < -16000) state->dButtons |= N64_CD;

            //Button to hold for 'combos'
            if (combo_pressed)
                *combo_pressed = (_buttons & (1 << 5)); //back

            //Map right axis for dual stick mode
            right_axis[0] = _axis[2] * 100 / 32768;
            right_axis[1] = _axis[3] * 100 / 32768;

            break;
        case JoystickController::XBOXONE:
            if (_buttons & (1 << 8))   state->dButtons |= N64_DU;   //DUP
            if (_buttons & (1 << 9))   state->dButtons |= N64_DD;   //DDOWN
            if (_buttons & (1 << 10))  state->dButtons |= N64_DL;   //DLEFT
            if (_buttons & (1 << 11))  state->dButtons |= N64_DR;   //DRIGHT
            if (_buttons & (1 << 2))   state->dButtons |= N64_ST;   //START
            if (_buttons & (1 << 3))   state->dButtons |= 0;        //BACK
            if (_buttons & (1 << 14))  state->dButtons |= 0;        //LS
            if (_buttons & (1 << 15))  state->dButtons |= 0;        //RS
            if (_buttons & (1 << 12))  state->dButtons |= N64_LB;   //LB
            if (_buttons & (1 << 13))  state->dButtons |= N64_RB;   //RB
            if (_buttons & (1 << 4))   state->dButtons |= N64_A;    //A
            if (_buttons & (1 << 5))   state->dButtons |= N64_B;    //B
            if (_buttons & (1 << 6))   state->dButtons |= N64_B;    //X
            if (_buttons & (1 << 7))   state->dButtons |= 0;        //Y
            if (_buttons & (1 << 15))   state->dButtons |= N64_CU | //RS triggers
                                                        N64_CD | //all C usb_buttons
                                                        N64_CL |
                                                        N64_CR;
            //Analog stick (Normalise 0 to +/-100)
            state->x_axis = _axis[0] * 100 / 32768;
            state->y_axis = _axis[1] * 100 / 32768;

            //Z button
            if (_axis[3] > 10) state->dButtons |= N64_Z; //LT
            if (_axis[4] > 10) state->dButtons |= N64_Z; //RT

            //C usb_buttons
            if (_axis[2] > 16000)  state->dButtons |= N64_CR;
            if (_axis[2] < -16000) state->dButtons |= N64_CL;
            if (_axis[5] > 16000)  state->dButtons |= N64_CU;
            if (_axis[5] < -16000) state->dButtons |= N64_CD;

            //Button to hold for 'combos'
            if (combo_pressed)
                *combo_pressed = (_buttons & (1 << 3)); //back

            right_axis[0] = _axis[2] * 100 / 32768;
            right_axis[1] = _axis[5] * 100 / 32768;
            break;
        case JoystickController::PS4:
            if (_buttons & (1 << 9))  state->dButtons |= N64_ST;  //START
            if (_buttons & (1 << 4))  state->dButtons |= N64_LB;  //L1
            if (_buttons & (1 << 5))  state->dButtons |= N64_RB;  //R1
            if (_buttons & (1 << 6))  state->dButtons |= N64_Z;   //L2
            if (_buttons & (1 << 7))  state->dButtons |= N64_Z;   //R2
            if (_buttons & (1 << 1))  state->dButtons |= N64_A;   //X
            if (_buttons & (1 << 0))  state->dButtons |= N64_B;   //SQUARE
            if (_buttons & (1 << 2))  state->dButtons |= N64_B;   //CIRCLE
            if (_buttons & (1 << 11)) state->dButtons |= N64_CU | //RS triggers
                                                      N64_CD | //all C usb_buttons
                                                      N64_CL |
                                                      N64_CR;
            //Analog stick (Normalise 0 to +/-100)
            state->x_axis =  (_axis[0] - 127) * 100 / 127;
            state->y_axis = -(_axis[1] - 127) * 100 / 127;

            //D Pad button
            switch(_axis[9])
            {
                case 0: state->dButtons |= N64_DU; break;
                case 1: state->dButtons |= N64_DU | N64_DR; break;
                case 2: state->dButtons |= N64_DR; break;
                case 3: state->dButtons |= N64_DR | N64_DD; break;
                case 4: state->dButtons |= N64_DD; break;
                case 5: state->dButtons |= N64_DD | N64_DL; break;
                case 6: state->dButtons |= N64_DL; break;
                case 7: state->dButtons |= N64_DL | N64_DU; break;
            }

            //C usb_buttons
            if (_axis[2] > 256/2 + 64)  state->dButtons |= N64_CR;
            if (_axis[2] < 256/2 - 64)  state->dButtons |= N64_CL;
            if (_axis[5] > 256/2 + 64)  state->dButtons |= N64_CD;
            if (_axis[5] < 256/2 - 64)  state->dButtons |= N64_CU;

            //Button to hold for 'combos'
            if (combo_pressed)
                *combo_pressed = (_buttons & (1 << 8)); //back

            //Map right axis for dual stick mode
            right_axis[0] =  (_axis[2] - 127) * 100 / 127;
            right_axis[1] = -(_axis[5] - 127) * 100 / 127;
            break;
        case JoystickController::UNKNOWN:
            #if (0)
            //Mapper helper
            static uint32_t print_slower = 0;
            if (millis() - print_slower > 100)
            {
                debug_print_status("%04x %04i %04i %04i %04i\n", _buttons, _axis[0], _axis[1], _axis[2], _axis[3]);
                if (_buttons)
                {
                    int bit = 0;
                    while ((_buttons & (1 << bit++)) == 0);
                    debug_print_status("button bit: %i\n", bit-1);
                }
                print_slower = millis();
            }
            #endif
            //Generic HID controllers //FIXME: Load from file?
            //Example of a basic Chinese NES HID Controller. The button mapping nubmers are from a bit of trial and error.
            //You can use the mapper helper above to assist.
            //The controller doesnt have enough buttons, so we're missing alot here.
            //NEXT SNES Controller
            if (joy->idVendor() == 0x0810 && joy->idProduct() == 0xE501)
            {
                if (_buttons & (1 << 9))  state->dButtons |= N64_ST;
                if (_buttons & (1 << 4))  state->dButtons |= N64_Z;
                if (_buttons & (1 << 6))  state->dButtons |= N64_RB;
                if (_buttons & (1 << 2)) state->dButtons |= N64_A;
                if (_buttons & (1 << 1)) state->dButtons |= N64_B;
                if (_buttons & (1 << 3)) state->dButtons |= N64_B;

                //Button to hold for 'combos'
                if (combo_pressed)
                    *combo_pressed = (_buttons & (1 << 8)); //back

                //Analog stick (Normalise 0 to +/-100)
                state->x_axis = (_axis[0] - 127) * 100 / 127;
                state->y_axis = - (_axis[1] - 127) * 100 / 127;
            }
            break;
        //TODO: OTHER USB CONTROLLERS
        case JoystickController::PS3:
        case JoystickController::PS3_MOTION:
        default:
            break;
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
#if (MAX_MICE >= 1)
    else if (input_is(id, USB_MOUSE))
    {
        //Prep N64 response
        n64_buttonmap *state = (n64_buttonmap *)response;
        state->dButtons = 0;

        //Get latest info from USB devices
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
        state->x_axis =  _axis[0] * MOUSE_SENSITIVITY;
        state->y_axis = -_axis[1] * MOUSE_SENSITIVITY;
        if (_buttons & (1 << 0)) state->dButtons |= N64_A;   //A
        if (_buttons & (1 << 1)) state->dButtons |= N64_B;   //B
        if (_buttons & (1 << 2)) state->dButtons |= N64_ST;  //ST
    }
#endif

#if (MAX_KB >= 1)
    else if (input_is(id, USB_KB))
    {
        //Prep N64 response
        n64_randnet_kb *state = (n64_randnet_kb *)response;

        //Get latest info from USB devices
        KeyboardController *kb = (KeyboardController *)input_devices[id].driver;

        kb->capsLock((state->led_state & RANDNET_LED_CAPSLOCK) != 0);
        kb->numLock((state->led_state & RANDNET_LED_NUMLOCK)  != 0);
        kb->scrollLock((state->led_state & RANDNET_LED_POWER) != 0);
        uint8_t home_key_flag = 0;
        //Map up to 3 keys to the randnet response packet
        for (int i = 0; i < RANDNET_MAX_BUTTONS; i++)
        {
            state->buttons[i] = 0;
            if (kb_keys_pressed[i] == 0)
                continue;

            if (kb_keys_pressed[i] == (uint8_t)(KEY_HOME & 0xFF))
            {
                home_key_flag = 1;
                continue;
            }

            //Keyboard is pressed, get the corressponding randnet code and put it in the response
            uint16_t randnet_code = 0;
            for (uint32_t j = 0; j < (sizeof(randnet_map) / sizeof(randnet_map_t)); j++)
            {
                if (kb_keys_pressed[i] == (uint8_t)(randnet_map[j].keypad & 0xFF))
                {
                    randnet_code = randnet_map[j].randnet_matrix;
                }
            }
            state->buttons[i] = randnet_code;
        }

        (home_key_flag) ? state->flags |= RANDNET_FLAG_HOME_KEY :
                          state->flags &= ~RANDNET_FLAG_HOME_KEY;
    }
#endif

#if (ENABLE_HARDWIRED_CONTROLLER >=1)
    else if (input_is(id, HW_GAMECONTROLLER))
    {
        n64_buttonmap *state = (n64_buttonmap *)response;
        state->dButtons = 0;

        if (!digitalRead(HW_A)) state->dButtons |= N64_A;
        if (!digitalRead(HW_B)) state->dButtons |= N64_B;
        if (!digitalRead(HW_CU)) state->dButtons |= N64_CU;
        if (!digitalRead(HW_CD)) state->dButtons |= N64_CD;
        if (!digitalRead(HW_CL)) state->dButtons |= N64_CL;
        if (!digitalRead(HW_CR)) state->dButtons |= N64_CR;
        if (!digitalRead(HW_DU)) state->dButtons |= N64_DU;
        if (!digitalRead(HW_DD)) state->dButtons |= N64_DD;
        if (!digitalRead(HW_DL)) state->dButtons |= N64_DL;
        if (!digitalRead(HW_DR)) state->dButtons |= N64_DR;
        if (!digitalRead(HW_START)) state->dButtons |= N64_ST;
        if (!digitalRead(HW_Z)) state->dButtons |= N64_Z;
        if (!digitalRead(HW_L)) state->dButtons |= N64_LB;
        if (!digitalRead(HW_R)) state->dButtons |= N64_RB;
        
        //10bit ADC
        state->x_axis = analogRead(HW_X) * 200 / 1024 - 100; //+/-100
        state->y_axis = analogRead(HW_Y) * 200 / 1024 - 100; //+/-100
        
        if (combo_pressed)
            *combo_pressed = !digitalRead(HW_L) && !digitalRead(HW_R); //FIXME: ADD A COMBO INPUT?
    }
#endif
    else
    {
        return 0;
    }

    return 1;
}

void input_apply_rumble(int id, uint8_t strength)
{
    JoystickController *joy;
    if (input_is(id, USB_GAMECONTROLLER))
    {
        joy = (JoystickController *)input_devices[id].driver;
        joy->setRumble(strength, strength, 20);
    }
#if (ENABLE_HARDWIRED_CONTROLLER >=1)
    else if (input_is(id, HW_GAMECONTROLLER))
    {
        (strength > 0) ? digitalWrite(HW_RUMBLE, HIGH) : digitalWrite(HW_RUMBLE, LOW);
    }
#endif
}

bool input_is_connected(int id)
{
    bool connected = false;
    if (_check_id(id) == 0)
        return false;

    if (input_is(id, USB_GAMECONTROLLER))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        if (*joy == true)
            connected = true;
    }

#if (MAX_MICE >=1)   
    else if (input_is(id, USB_MOUSE))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        if (*mouse == true)
            connected = true;
    }
#endif

#if (MAX_KB >=1)   
    else if (input_is(id, USB_KB))
    {
        USBHIDInput *kb = (USBHIDInput *)input_devices[id].driver;
        if (*kb == true)
            connected = true;
    }
#endif
    
#if (ENABLE_HARDWIRED_CONTROLLER >=1)
    else if (input_is(id, HW_GAMECONTROLLER))
    {
        if (digitalRead(HW_EN) == 0)
            connected = true;
    }
#endif

    return connected;
}

bool input_is(int id, input_type_t type)
{
    if (_check_id(id) == 0)
        return false;
    if (input_devices[id].type == type)
        return true;
    return false;
}

uint16_t input_get_id_product(int id)
{
    if (_check_id(id) == 0 || input_is_connected(id) == 0)
        return 0;

    if (input_is(id, USB_GAMECONTROLLER))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        return joy->idProduct();
    }
    else if (input_is(id, USB_MOUSE))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        return mouse->idProduct();
    }
    else if (input_is(id, USB_KB))
    {
        USBHIDInput *kb = (USBHIDInput *)input_devices[id].driver;
        return kb->idProduct();
    }
    else if (input_is(id, HW_GAMECONTROLLER))
    {
        return 0xBEEF;
    }

    return 0;
}

uint16_t input_get_id_vendor(int id)
{
    if (_check_id(id) == 0 || input_is_connected(id) == 0)
        return 0;

    if (input_is(id, USB_GAMECONTROLLER))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        return joy->idVendor();
    }
    else if (input_is(id, USB_MOUSE))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        return mouse->idVendor();
    }
    else if (input_is(id, USB_KB))
    {
        USBHIDInput *kb = (USBHIDInput *)input_devices[id].driver;
        return kb->idVendor();
    }
    else if (input_is(id, HW_GAMECONTROLLER))
    {
        return 0xDEAD;
    }

    return 0;
}

const char *input_get_manufacturer_string(int id)
{
    static const char NC[] = "NOT CONNECTED";
    if (_check_id(id) == 0 || input_is_connected(id) == false)
        return NC;

    if (input_is(id, USB_GAMECONTROLLER))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        return (const char *)joy->manufacturer();
    }
    else if (input_is(id, USB_MOUSE))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        return (const char *)mouse->manufacturer();
    }
    else if (input_is(id, USB_KB))
    {
        USBHIDInput *kb = (USBHIDInput *)input_devices[id].driver;
        return (const char *)kb->manufacturer();
    }
    else if (input_is(id, HW_GAMECONTROLLER))
    {
        return "USB64";
    }

    return NC;
}

const char *input_get_product_string(int id)
{
    static const char NC[] = "";
    if (_check_id(id) == 0 || input_is_connected(id) == 0)
        return NC;

    if (input_is(id, USB_GAMECONTROLLER))
    {
        JoystickController *joy = (JoystickController *)input_devices[id].driver;
        return (const char *)joy->product();
    }
    else if (input_is(id, USB_MOUSE))
    {
        USBHIDInput *mouse = (USBHIDInput *)input_devices[id].driver;
        return (const char *)mouse->product();
    }
    else if (input_is(id, USB_KB))
    {
        USBHIDInput *kb = (USBHIDInput *)input_devices[id].driver;
        return (const char *)kb->product();
    }
    else if (input_is(id, HW_GAMECONTROLLER))
    {
        return "HARDWIRED";
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
