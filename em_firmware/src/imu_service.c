#include "imu_service.h"

// read accel values
void read_accel(accel_samples_t *sample)
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

void read_gyro(gyro_samples_t *sample)
{
    if (sample == NULL)
        return;
    // read accel values
    BMI160_read_gyro_raw(&sample->gx, &sample->gy, &sample->gz);

    sample->gx_dps = sample->gx / 65.6f;
    sample->gy_dps = sample->gy / 65.6f;
    sample->gz_dps = sample->gz / 65.6f;

    sample->g_mag = sqrtf(
        (sample->gx * sample->gx) +
        (sample->gy * sample->gy) +
        (sample->gz * sample->gz)); // gyro magnitude
}