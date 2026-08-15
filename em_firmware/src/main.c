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

accel_samples_t accel_sample = {0};
gyro_samples_t gyro_sample = {0};
lp_filt_output_t accel_filt_out = {0};
peak_det_params_t pdp = {0};

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

    printf("here 1\n");
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
    printf("here 3\n");
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

        //-------------------------------------------------------------
        // Gyro stuff
        //-------------------------------------------------------------

        // remove the bias (hardcoded)
        //  float gx_corrected = gyro_sample.gx_dps

        // // Seperate noise from motion
        lowpass_filter(&accel_filt_out, accel_alpha, accel_sample.a_abs);
        // rep detection formula R = P-1/2

        env_creation_irsf(&p_env, accel_filt_out.sig_filt, p_alpha_release);

        // // threshold (95% of the envelope)
        p_threshold = p_env * .95;

        peak_detection(accel_filt_out.sig_filt, p_threshold, &pdp);

        // reps

        // debuging purposes
        printf(">sig_filt:%f\n", accel_filt_out.sig_filt);
        printf(">p_threshold:%f\n", p_threshold);
        // printf(">interval:%f\n", pdp.p_interval_s);
        // printf(">p_flag:%d\n", pdp.peakFlag);
        // printf(">effort:%f\n", pdp.interval_effort);
        // printf(">effort/s:%f\n", pdp.effort_s);
        // printf("here 1");

        printf(">gx:%f\n", gyro_sample.gx_dps);
        printf(">gy:%f\n", gyro_sample.gy_dps);
        printf(">gz:%f\n", gyro_sample.gz_dps);

        printf("here 4\n");
        fflush(stdout);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// #include "driver/gpio.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"

// #define USER_LED GPIO_NUM_21

// static const char *TAG = "TEST";
// void app_main(void)
// {
//     gpio_config_t led_config = {
//         .pin_bit_mask = 1ULL << USER_LED,
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_up_en = GPIO_PULLUP_DISABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_DISABLE};

//     gpio_config(&led_config);

//     while (true)
//     {
//         gpio_set_level(USER_LED, 0); // ON: active-low
//         vTaskDelay(pdMS_TO_TICKS(1000));

//         gpio_set_level(USER_LED, 1); // OFF
//         vTaskDelay(pdMS_TO_TICKS(1000));

//         // printf("printf test\n");
//         // fflush(stdout);

//         // ESP_LOGI(TAG, "ESP_LOGI test");

//         printf("hiya dude\n");
//     }
// }