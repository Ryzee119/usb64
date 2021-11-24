// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "usb64_conf.h"

void astick_apply_deadzone(float *out_x, float *out_y, float x, float y, float dz_low, float dz_high) {
    float magnitude = sqrtf(powf(x,2) + powf(y,2));
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

float astick_apply_sensitivity(int sensitivity, float *x, float *y)
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

void astick_apply_snap(float range, float *x, float *y)
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

//Input range is expected to a circle with radius 1.00f.
//This should be normalised/deadzone corrected before entering this function.
void astick_apply_octa_correction(float *x, float *y)
{
    /* A 90 degree quadrant of the octa output.
     * The calculation is performed between 0-45degree section as it is mirror for each quadrant.
     *                          
     *  ####                    
     *      #####               
     *           #####          
     *                ##        
     *                @ #       
     *              @@   #      
     *            @@      @ <-xint,yint  
     *     m45->@@       / #    
     *        @@     /      #   
     *      @@    /<-out_mag # <- octa line (m1*x + c1)
     *    @@   /              # 
     *  @@  /  | angle         #
     * 
     * 0                      1.0
    */
    #define D2R(a) (a * 3.1415f/180.0f)
    static const float m45 = MAG_AT_45DEG;
    float angle = atanf(*y / *x); //-90 to +90deg
    //Make it 0 to 90deg
    if (angle < 0) angle += D2R(90);

    //Make it 0 to 45deg
    if (angle > D2R(45)) angle = D2R(90) - angle;

    //Build octagonal line for intersection
    float m1 = (0 - m45 * sinf(D2R(45))) / (1 - m45 * cosf(D2R(45)));
    float c1 = -m1;

    //Draw another line from the input angle
    float x3 = cosf(angle);
    float y3 = sinf(angle);
    float m3 = y3 / x3;
    
    //Calculate intersection between line and octagon.
    float xint = (0 - c1) / (m1 - m3);
    float yint = m1 * xint + c1;

    //Calculate magnitude of line until intersection.
    float out_mag = sqrtf(xint * xint + yint * yint);

    //Output corrected x,y
    *x *= out_mag;
    *y *= out_mag;
}
