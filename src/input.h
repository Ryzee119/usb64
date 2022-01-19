// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _INPUT_H
#define _INPUT_H

#include "common.h"
#include "tusb.h"

typedef struct
{
    uint8_t keypad;
    uint16_t randnet_matrix;
} randnet_map_t;

static const randnet_map_t randnet_map[] PROGMEM = {
    {HID_KEY_ESCAPE, 0x0A08},         //Escape
    {HID_KEY_F1, 0x0B01},          // F1
    {HID_KEY_F2, 0x0A01},          // F2
    {HID_KEY_F3, 0x0B08},          // F3
    {HID_KEY_F4, 0x0A07},          // F4
    {HID_KEY_F5, 0x0B07},          // F5
    {HID_KEY_F6, 0x0A02},          // F6
    {HID_KEY_F7, 0x0B02},          // F7
    {HID_KEY_F8, 0x0A03},          // F8
    {HID_KEY_F9, 0x0B03},          // F9
    {HID_KEY_F10, 0x0A04},         // F10
    {HID_KEY_F11, 0x0203},         // F11
    {HID_KEY_F12, 0x0B06},         // F12
    {HID_KEY_NUM_LOCK, 0x0A05},    // Num Lock
    {HID_KEY_PRINT_SCREEN, 0x0B05}, //Japanese Key below Numlock LED
    {HID_KEY_SCROLL_LOCK, 0x0208}, //Japanese Key below Caps Lock LED
    {HID_KEY_PAUSE, 0x0207},       // Japanese Key below Power LED
    {HID_KEY_GRAVE, 0x0D05},       //~
    {HID_KEY_1, 0x0C05},           //Number 1
    {HID_KEY_2, 0x0505},           //Number 2
    {HID_KEY_3, 0x0605},           //Number 3
    {HID_KEY_4, 0x0705},           //Number 4
    {HID_KEY_5, 0x0805},           //Number 5
    {HID_KEY_6, 0x0905},           //Number 6
    {HID_KEY_7, 0x0906},           //Number 7
    {HID_KEY_8, 0x0806},           //Number 8
    {HID_KEY_9, 0x0706},           //Number 9
    {HID_KEY_0, 0x0606},           //Number 0
    {HID_KEY_KEYPAD_SUBTRACT, 0x0506},    //-
    {HID_KEY_KEYPAD_ADD, 0x0C06},     //^
    {HID_KEY_BACKSPACE, 0x0D06},   //Backspace
    {HID_KEY_TAB, 0x0D01},         //Tab
    {HID_KEY_Q, 0x0C01},           //Q
    {HID_KEY_W, 0x0501},           //W
    {HID_KEY_E, 0x0601},           //E
    {HID_KEY_R, 0x0701},           //R
    {HID_KEY_T, 0x0801},           //T
    {HID_KEY_Y, 0x0901},           //Y
    {HID_KEY_U, 0x0904},           //U
    {HID_KEY_I, 0x0804},           //I
    {HID_KEY_O, 0x0704},           //O
    {HID_KEY_P, 0x0604},           //P
    {HID_KEY_APOSTROPHE, 0x0504},       //'
    {HID_KEY_BRACKET_LEFT, 0x0C04},  //{
    {HID_KEY_BRACKET_RIGHT, 0x0406}, //}
    {HID_KEY_CAPS_LOCK, 0x0F05},   //Caps Lock
    {HID_KEY_A, 0x0D07},           //A
    {HID_KEY_S, 0x0C07},           //S
    {HID_KEY_D, 0x0507},           //D
    {HID_KEY_F, 0x0607},           //F
    {HID_KEY_G, 0x0707},           //G
    {HID_KEY_H, 0x0807},           //H
    {HID_KEY_J, 0x0907},           //J
    {HID_KEY_K, 0x0903},           //K
    {HID_KEY_L, 0x0803},           //L
    {HID_KEY_KEYPAD_ADD, 0x0703},     //+
    {HID_KEY_KEYPAD_MULTIPLY, 0x0603},  //*
    {HID_KEY_ENTER, 0x0D04},       //Enter
    {HID_KEY_SHIFT_LEFT, 0x0E01},             //Left Shift
    {HID_KEY_Z, 0x0D08},           //Z
    {HID_KEY_X, 0x0C08},           //X
    {HID_KEY_C, 0x0508},           //C
    {HID_KEY_V, 0x0608},           //V
    {HID_KEY_B, 0x0708},           //B
    {HID_KEY_N, 0x0808},           //N
    {HID_KEY_M, 0x0908},           //M
    {HID_KEY_COMMA, 0x0902},       //<
    {HID_KEY_PERIOD, 0x0802},      //>
    {HID_KEY_SLASH, 0x0702},       //?
    {HID_KEY_MINUS, 0x1004},       //- (Long dash)
    {HID_KEY_ARROW_UP, 0x0204},          //Up Cursor
    {HID_KEY_SHIFT_RIGHT, 0x0E06},             //Right Shift
    {HID_KEY_CONTROL_LEFT, 0x1107},//Ctrl
    {HID_KEY_GUI_LEFT, 0x0F07},    //Opt
    {HID_KEY_SEMICOLON, 0x1105},   //| (Pipes)
    {HID_KEY_ALT_LEFT, 0x1008},    //Alt
    {HID_KEY_KEYPAD_1, 0x1002},        //Japanese 'alphanumeric key'
    {HID_KEY_SPACE, 0x0602},       //Space
    {HID_KEY_KEYPAD_2, 0x0E02},        //Japanese 'kana'
    {HID_KEY_KEYPAD_3, 0x1006},        //Japanese Character
    {HID_KEY_END, 0x0206},         //End 行末
    {HID_KEY_ARROW_LEFT, 0x0205},        //Left Cursor
    {HID_KEY_ARROW_DOWN, 0x0305},        //Down Cursor
    {HID_KEY_ARROW_RIGHT, 0x0405},       //Right Cursor
};

typedef enum
{
    INPUT_NONE,
    INPUT_MOUSE,
    INPUT_KEYBOARD,
    INPUT_GAMECONTROLLER,
} input_type_t;

typedef enum input_backend
{
    BACKEND_NONE,
    BACKEND_XINPUT,
    BACKEND_HID_KEYBOARD,
    BACKEND_HID_MOUSE,
    BACKEND_HARDWIRED,
} input_backend_t;

typedef struct input_driver
{
    uint8_t slot;
    input_type_t type;
    input_backend_t backend;
    uint8_t _data[CFG_TUH_XINPUT_EPIN_BUFSIZE];
    uint8_t *data;
    uint16_t uid;
    bool (*set_rumble)(uint8_t dev_addr, uint8_t instance, uint8_t lValue, uint8_t rValue, bool block);
    bool (*set_led)(uint8_t dev_addr, uint8_t instance, uint8_t quadrant, bool block);
}input_driver_t;

void input_init();
void input_update_input_devices();
bool input_is_connected(int id);
bool input_is(int id, input_type_t type);
uint16_t input_get_id_product(int id);
uint16_t input_get_id_vendor(int id);
const char *input_get_manufacturer_string(int id);
const char *input_get_product_string(int id);
uint16_t input_get_state(uint8_t id, void *n64_response, bool *combo_pressed);
void input_apply_rumble(int id, uint8_t strength);
void input_enable_dualstick_mode(int id);
void input_disable_dualstick_mode(int id);
bool input_is_dualstick_mode(int id);

#endif
