// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef N64_N64_RUMBLEPAK_H_
#define N64_N64_RUMBLEPAK_H_

#ifdef __cplusplus
extern "C" {
#endif

enum n64_rumble_state
{
    RUMBLE_STOP = 0,
    RUMBLE_START,
    RUMBLE_APPLIED
};

typedef struct
{
    int8_t initialised;
    enum n64_rumble_state state;
} n64_rumblepak;

#ifdef __cplusplus
}
#endif

#endif /* N64_N64_RUMBLEPAK_H_ */
