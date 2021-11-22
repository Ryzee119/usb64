// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _INPUT_H
#define _INPUT_H

#include <Arduino.h>

typedef struct
{
    uint16_t keypad;
    uint16_t randnet_matrix;
} randnet_map_t;

static const randnet_map_t randnet_map[] = {
    {KEY_ESC, 0x0A08},         //Escape
    {KEY_F1, 0x0B01},          // F1
    {KEY_F2, 0x0A01},          // F2
    {KEY_F3, 0x0B08},          // F3
    {KEY_F4, 0x0A07},          // F4
    {KEY_F5, 0x0B07},          // F5
    {KEY_F6, 0x0A02},          // F6
    {KEY_F7, 0x0B02},          // F7
    {KEY_F8, 0x0A03},          // F8
    {KEY_F9, 0x0B03},          // F9
    {KEY_F10, 0x0A04},         // F10
    {KEY_F11, 0x0203},         // F11
    {KEY_F12, 0x0B06},         // F12
    {KEY_NUM_LOCK, 0x0A05},    // Num Lock
    {KEY_PRINTSCREEN, 0x0B05}, //Japanese Key below Numlock LED
    {KEY_SCROLL_LOCK, 0x0208}, //Japanese Key below Caps Lock LED
    {KEY_PAUSE, 0x0207},       // Japanese Key below Power LED
    {KEY_TILDE, 0x0D05},       //~
    {KEY_1, 0x0C05},           //Number 1
    {KEY_2, 0x0505},           //Number 2
    {KEY_3, 0x0605},           //Number 3
    {KEY_4, 0x0705},           //Number 4
    {KEY_5, 0x0805},           //Number 5
    {KEY_6, 0x0905},           //Number 6
    {KEY_7, 0x0906},           //Number 7
    {KEY_8, 0x0806},           //Number 8
    {KEY_9, 0x0706},           //Number 9
    {KEY_0, 0x0606},           //Number 0
    {KEYPAD_MINUS, 0x0506},    //-
    {KEYPAD_PLUS, 0x0C06},     //^
    {KEY_BACKSPACE, 0x0D06},   //Backspace
    {KEY_TAB, 0x0D01},         //Tab
    {KEY_Q, 0x0C01},           //Q
    {KEY_W, 0x0501},           //W
    {KEY_E, 0x0601},           //E
    {KEY_R, 0x0701},           //R
    {KEY_T, 0x0801},           //T
    {KEY_Y, 0x0901},           //Y
    {KEY_U, 0x0904},           //U
    {KEY_I, 0x0804},           //I
    {KEY_O, 0x0704},           //O
    {KEY_P, 0x0604},           //P
    {KEY_QUOTE, 0x0504},       //'
    {KEY_LEFT_BRACE, 0x0C04},  //{
    {KEY_RIGHT_BRACE, 0x0406}, //}
    {KEY_CAPS_LOCK, 0x0F05},   //Caps Lock
    {KEY_A, 0x0D07},           //A
    {KEY_S, 0x0C07},           //S
    {KEY_D, 0x0507},           //D
    {KEY_F, 0x0607},           //F
    {KEY_G, 0x0707},           //G
    {KEY_H, 0x0807},           //H
    {KEY_J, 0x0907},           //J
    {KEY_K, 0x0903},           //K
    {KEY_L, 0x0803},           //L
    {KEYPAD_PLUS, 0x0703},     //+
    {KEYPAD_ASTERIX, 0x0603},  //*
    {KEY_ENTER, 0x0D04},       //Enter
    {104, 0x0E01},             //Left Shift
    {KEY_Z, 0x0D08},           //Z
    {KEY_X, 0x0C08},           //X
    {KEY_C, 0x0508},           //C
    {KEY_V, 0x0608},           //V
    {KEY_B, 0x0708},           //B
    {KEY_N, 0x0808},           //N
    {KEY_M, 0x0908},           //M
    {KEY_COMMA, 0x0902},       //<
    {KEY_PERIOD, 0x0802},      //>
    {KEY_SLASH, 0x0702},       //?
    {KEY_MINUS, 0x1004},       //- (Long dash)
    {KEY_UP, 0x0204},          //Up Cursor
    {104, 0x0E06},             //Right Shift
    {103, 0x1107},             //Ctrl
    {110, 0x0F07},             //Opt
    {KEY_SEMICOLON, 0x1105},   //| (Pipes)
    {105, 0x1008},             //Alt
    {KEYPAD_1, 0x1002},        //Japanese 'alphanumeric key'
    {KEY_SPACE, 0x0602},       //Space
    {KEYPAD_2, 0x0E02},        //Japanese 'kana'
    {KEYPAD_3, 0x1006},        //Japanese Character
    {KEY_END, 0x0206},         //End 行末
    {KEY_LEFT, 0x0205},        //Left Cursor
    {KEY_DOWN, 0x0305},        //Down Cursor
    {KEY_RIGHT, 0x0405},       //Right Cursor
};

typedef enum
{
    USB_MOUSE,
    USB_KB,
    USB_GAMECONTROLLER,
    HW_GAMECONTROLLER,
    I2C_GAMECONTROLLER
} input_type_t;

typedef struct
{
    void *driver;
    input_type_t type;
} input;

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
