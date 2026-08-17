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

accel_samples_t accel_sample = {0};
gyro_samples_t gyro_sample = {0};
lp_filt_output_t accel_filt_out = {0};
peak_det_params_t pdp = {0};
stopwatch_obj sample_watch = {0};

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

        int64_t sample_interval_us;
        bool valid_dt =
            stopwatch_lap(&sample_watch, &sample_interval_us);
        //-------------------------------------------------------------
        // Gyro stuff
        //-------------------------------------------------------------

        // remove the bias (hardcoded)
        float gx_corrected = gyro_sample.gx_dps - 0.68f;
        float gy_corrected = gyro_sample.gy_dps - 0.03f;
        float gz_corrected = gyro_sample.gz_dps - (-0.13f);

        // // Seperate noise from motion
        lowpass_filter(&accel_filt_out, accel_alpha, accel_sample.a_abs);
        // rep detection formula R = P-1/2

        // integration
        if (valid_dt)
        {
            float dt_s = sample_interval_us / 1000000.0f;

            if (dt_s > 0.0f && dt_s < 0.1f)
            {
                pdp.interval_effort +=
                    accel_filt_out.sig_filt * dt_s;

                gx_angle += gx_corrected * dt_s;
                gy_angle += gy_corrected * dt_s;
                gz_angle += gz_corrected * dt_s;
            }
        }

        env_creation_irsf(&p_env, accel_filt_out.sig_filt, p_alpha_release);

        // // threshold (95% of the envelope)
        p_threshold = p_env * .95;

        peak_detection(accel_filt_out.sig_filt, p_threshold, &pdp);

        if (pdp.interval_complete)
        {

            if (pdp.p_interval_s > 0.0f)
            {
                pdp.average_intensity =
                    pdp.interval_effort / pdp.p_interval_s;
            }
        }

        // reps

        // debuging purposes
        printf(">sig_filt:%f\n", accel_filt_out.sig_filt);
        printf(">p_threshold:%f\n", p_threshold);
        // printf(">interval:%f\n", pdp.p_interval_s);
        // printf(">p_flag:%d\n", pdp.peakFlag);
        // printf(">effort:%f\n", pdp.interval_effort);
        // printf(">effort/s:%f\n", pdp.effort_s);
        // printf("here 1");

        printf(">gx:%f\n", gx_corrected);
        printf(">gy:%f\n", gy_corrected);
        printf(">gz:%f\n", gz_corrected);

        fflush(stdout);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}