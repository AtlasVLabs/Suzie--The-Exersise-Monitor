#pragma once

#include <stdint.h>
#include "bmi160.h"
#include "math.h"

#define BMI160_GYRO_SCALE_2000DPS 16.4f
#define DEG_TO_RAD 0.01745329252f

typedef struct read_accel_struct
{
    int16_t ax, ay, az;
    float ax_g, ay_g, az_g;
    float a_mag; // accel mag
    float a_abs;

} accel_samples_t;

typedef struct read_gyro_struct
{
    int16_t gx, gy, gz;
    float gx_rad_s,
        gy_rad_s,
        gz_rad_s;
    float g_mag; // gyro mag

} gyro_samples_t;

// read accel values
void read_accel(accel_samples_t *sample);
// read gyro values
void read_gyro(gyro_samples_t *sample);