// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifndef _ANALOG_STICK_H
#define _ANALOG_STICK_H

#include "usb64_conf.h"

void astick_apply_deadzone(float *out_x, float *out_y, float x, float y, float dz_low, float dz_high);
float astick_apply_sensitivity(int sensitivity, float *x, float *y);
void astick_apply_snap(float range, float *x, float *y);
void astick_apply_octa_correction(float *x, float *y);

#endif
