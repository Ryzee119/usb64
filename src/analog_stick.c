/* MIT License
 * 
 * Copyright (c) [2020] [Ryan Wendland]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "usb64_conf.h"

void apply_deadzone(float *out_x, float *out_y, float x, float y, float dz_low, float dz_high) {
    float magnitude = sqrtf(x * x + y * y);
    if (magnitude > dz_low) {
        //Scale such that output magnitude is in the range [0.0f, 1.0f]
        float allowed_range = 1.0f - dz_high - dz_low;
        float normalised_magnitude = (magnitude - dz_low) / allowed_range;
        if (normalised_magnitude > 1.0f)
            normalised_magnitude = 1.0f;
        float scale = normalised_magnitude / magnitude;
        *out_x = x * scale;
        *out_y = y * scale;
    }
    else {
        //Stick is in the inner dead zone
        *out_x = 0.0f;
        *out_y = 0.0f;
    }
}

float apply_sensitivity(int sensitivity, float *x, float *y)
{
    float range;
    switch (sensitivity)
    {
        case 4:  range = 1.10f; break; // +/-110
        case 3:  range = 0.95f; break;
        case 2:  range = 0.85f; break;
        case 1:  range = 0.75f; break;
        case 0:  range = 0.65f; break; // +/-65
        default: range = 0.85f; break;
    }
    *x *= range; *y *= range;

    return range;
}

void apply_snap(float range, float *x, float *y)
{
     //+/- SNAP_RANGE degrees within a 45 degree angle will snap (MAX is 45/2)
    const int snap = SNAP_RANGE;
    float magnitude = sqrtf(powf(*x,2) + powf(*y,2));

    //Only snap if magnitude is >=90%
    if (magnitude >= 0.90f * range)
    {
        int angle = atan2f(*y, *x) * 180.0f / 3.14f;

        //Between 0-360 degrees
        if (angle < 0) angle = 360 + angle;

        //Temp variable between 0-45 degrees
        int a = angle;
        while (a > 45) a-=45;

        //Snap to 45 degree segments
        if ((a <= 0 + snap) || (a >= 45 - snap))
        {
            angle += snap;
            angle -= angle % 45;
            *x = magnitude * cosf(angle * 3.14f / 180.0f);
            *y = magnitude * sinf(angle * 3.14f / 180.0f);
        }
    }
}

void apply_octa_correction(float *x, float *y)
{
    //Fixme
}
