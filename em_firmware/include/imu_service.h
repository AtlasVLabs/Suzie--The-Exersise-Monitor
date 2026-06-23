#pragma once

#include <stdint.h>
#include "bmi160.h"
#include "math.h"

typedef struct read_accel_struct
{
    int16_t ax, ay, az;
    float ax_g, ay_g, az_g;
    float a_mag; // accel mag
    float a_abs;

} accel_samples;

// read accel values
void read_accel(accel_samples *sample);