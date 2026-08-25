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

    sample->gx_rad_s =
        ((float)sample->gx / BMI160_GYRO_SCALE_2000DPS) * DEG_TO_RAD;

    sample->gy_rad_s =
        ((float)sample->gy / BMI160_GYRO_SCALE_2000DPS) * DEG_TO_RAD;

    sample->gz_rad_s =
        ((float)sample->gz / BMI160_GYRO_SCALE_2000DPS) * DEG_TO_RAD;

    sample->g_mag = sqrtf(
        (sample->gx_rad_s * sample->gx_rad_s) +
        (sample->gy_rad_s * sample->gy_rad_s) +
        (sample->gz_rad_s * sample->gz_rad_s)); // gyro magnitude
}