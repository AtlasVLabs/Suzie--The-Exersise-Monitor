#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "max30102.h"
#include "bmi160.h"
#include "i2c_bus.h"
#include "imu_service.h"
#include "signal_processing.h"
#include "timers.h"

#define USER_LED GPIO_NUM_21

// variables---------------------------------------------
float accel_alpha = 0.08;    // alpha value to smooth out accel //was 0.6
int p_count = 0;             // peak count
int startup_samples = 20;    // samples to ignore during startup
float p_env = 0;             // peak envelope
float p_threshold = 0;       // peak threshold
float p_alpha_release = .04; // alpha for the peak envelope. Slow decent after peak in signal

float gx_angle, gy_angle, gz_angle;
float accel_signal_sum = 0;

float intervalCount = 0;
float interval_effort = 0;

accel_samples_t accel_sample = {0};
gyro_samples_t gyro_sample = {0};

lp_filt_output_t accel_filt_out = {0};

lp_filt_output_t gyro_x_filt_out = {0};
lp_filt_output_t gyro_y_filt_out = {0};
lp_filt_output_t gyro_z_filt_out = {0};

peak_det_params_t pdp = {0};
stopwatch_obj sample_watch = {0};
int64_t sample_interval_us;
void app_main()
{
    // inits-------------------------------------------------

    gpio_config_t led_config = {
        .pin_bit_mask = 1ULL << USER_LED,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&led_config);

    gpio_set_level(USER_LED, 0);

    vTaskDelay(pdMS_TO_TICKS(5000));

    fflush(stdout);
    i2c_bus_init();
    // MAX30102_init(get_master_bus_handle());
    printf("here 2\n");
    fflush(stdout);

    BMI160_init(get_master_bus_handle());
    bmi160_config_t bmiconfig = {
        .apm = bmi160_accel_pwr_normal,
        .gpm = bmi160_gyro_pwr_normal};
    BMI160_config(&bmiconfig);
    // MAX30102_config(&maxconfig);

    fflush(stdout);

    while (1)
    {

        if (startup_samples > 0)
        {
            startup_samples--;
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // READING ACCEL VALUES
        read_accel(&accel_sample);
        // Reading gyro values
        read_gyro(&gyro_sample);

        // // timer for each sample read

        bool valid_dt =
            stopwatch_lap(&sample_watch, &sample_interval_us);
        //-------------------------------------------------------------
        // Gyro stuff
        //-------------------------------------------------------------

        // remove the bias (hardcoded)
        float gx_corrected = gyro_sample.gx_rad_s;
        float gy_corrected = gyro_sample.gy_rad_s;
        float gz_corrected = gyro_sample.gz_rad_s;

        // // Seperate noise from motion
        lowpass_filter(&accel_filt_out, accel_alpha, accel_sample.a_abs);

        lowpass_filter(&gyro_x_filt_out, 0.08, gx_corrected);
        lowpass_filter(&gyro_y_filt_out, 0.1, gy_corrected);
        lowpass_filter(&gyro_z_filt_out, 0.1, gz_corrected);

        // // integration
        if (valid_dt)
        {
            float dt_s = sample_interval_us / 1000000.0f;

            if (dt_s > 0.0f && dt_s < 0.1f)
            {
                interval_effort += accel_filt_out.sig_filt * dt_s;

                gx_angle += gx_corrected * dt_s;
                gy_angle += gy_corrected * dt_s;
                gz_angle += gz_corrected * dt_s;
            }
        }

        // env_creation_irsf(&p_env, accel_filt_out.sig_filt, p_alpha_release);

        // // // threshold (95% of the envelope)
        // p_threshold = p_env * .95;

        // if (accel_filt_out.sig_filt > 0.1f)
        //     accel_signal_sum += accel_filt_out.sig_filt;
        // else
        //     accel_signal_sum = 0.0f;

        // if (peak_detection(accel_filt_out.sig_filt, p_threshold, &pdp))
        // {
        //     int64_t interval_us;

        //     // returns true at the end of an interval
        //     if (stopwatch_lap(&pdp.s_obj, &interval_us))
        //     {
        //         pdp.p_interval_s =
        //             interval_us / 1000000.0f;

        //         pdp.interval_complete = true;
        //     }
        // }

        bool result = cycle_detection(gx_corrected);

        // reps
        if (result)
        {
            intervalCount++;
        }

        // debuging purposes
        // printf(">sig_filt:%f\n", accel_filt_out.sig_filt);
        printf(">p_threshold:%f\n", p_threshold);
        // printf(">interval:%f\n", pdp.p_interval_s);
        // printf(">p_flag:%d\n", pdp.peakFlag);
        // printf(">effort:%f\n", pdp.interval_effort);
        // printf(">effort/s:%f\n", pdp.effort_s);

        printf(">interval:%f\n", intervalCount);
        printf(">gx:%f\n", gx_corrected);
        printf(">counts:%f\n", (float)result * 10);
        printf(">posThresh:%f\n", 4.0f);
        printf(">negThresh:%f\n", -4.0f);
        // printf(">gx:%f\n", gx_angle);

        printf(">gx_filt:%f\n", gyro_x_filt_out.sig_filt);

        fflush(stdout);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}