#include "imu_service.h"

// read accel values
void read_accel(accel_samples *sample)
{
    if (sample == NULL)
        return;
    // read accel values
    BMI160_read_accel_raw(&sample->ax, &sample->ay, &sample->az);
    // turn those into gs
    sample->ax_g = sample->ax / 8192.0f;
    sample->ay_g = sample->ay / 8192.0f;
    sample->az_g = sample->az / 8192.0f;

    sample->a_mag = sqrtf(
        (sample->ax_g * sample->ax_g) +
        (sample->ay_g * sample->ay_g) +
        (sample->az_g * sample->az_g));          // accel magnitude
    sample->a_abs = fabsf(sample->a_mag - 1.0f); // abs magnitude
}