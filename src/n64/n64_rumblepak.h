/*
 * n64_rumblepak.h
 *
 *  Created on: 22Apr.,2020
 *      Author: wendl
 */

#ifndef N64_N64_RUMBLEPAK_H_
#define N64_N64_RUMBLEPAK_H_

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

#endif /* N64_N64_RUMBLEPAK_H_ */
