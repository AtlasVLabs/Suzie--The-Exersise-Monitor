#include "driver/i2c_master.h"
#include "driver/uart.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "max30102.h"
#include "bmi160.h"
#include "i2c_bus.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void app_main()
{
    // variables---------------------------------------------
    int16_t ax, ay, az;          // raw accel values
    float ax_g, ay_g, az_g;      // accel values in gs
    double accel_alpha = .60;    // alpha value to smooth out accel
    float accel_filt = 0;        // accel filtered
    float noise = 0;             // system noise
    float accel_filt_mod = 0;    // accel filt modified for easier peak detection
    int p_count = 0;             // peak count
    float p_env = 0;             // peak envelope
    float p_threshold = 0;       // peak threshold
    float p_alpha_release = .25; // alpha for the peak envelope. Slow decent after peak in signal
    bool isAbove = false;        // is the accel signal above p_threshold
    bool peak_ready = false;     // are we ready to read peaks?

    // inits-------------------------------------------------
    uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
    i2c_bus_init();
    MAX30102_init(get_master_bus_handle());
    BMI160_init(get_master_bus_handle());
    max30102_config_t maxconfig = MAX30102_CONFIG_DEFAULT();
    bmi160_config_t bmiconfig = {
        .apm = bmi160_accel_pwr_normal,
        .gpm = bmi160_gyro_pwr_normal};
    BMI160_config(&bmiconfig);
    MAX30102_config(&maxconfig);

    while (1)
    {
        // read accel values
        BMI160_read_accel_raw(&ax, &ay, &az);
        // turn those into gs
        ax_g = ax / 8192.0f;
        ay_g = ay / 8192.0f;
        az_g = az / 8192.0f;

        float a_mag = sqrtf(ax_g * ax_g + ay_g * ay_g + az_g * az_g); // accel magnitude
        float a_abs = fabsf(a_mag - 1.0f);                            // abs magnitude

        // reading from uart (debugging only)
        char ch[5];
        int len = uart_read_bytes(UART_NUM_0, &ch, 5, 0); // timeout = 0, non-blocking
        if (len > 0)
        {
            float value = strtof(ch, NULL);
            if (p_alpha_release + value >= 0 && p_alpha_release + value <= 1)
                p_alpha_release += value;
            else if (p_alpha_release + value > 1)
                p_alpha_release = 1;
            else if (p_alpha_release + value < 0)
                p_alpha_release = 0;
        }

        // Seperate noise from motion
        accel_filt = accel_filt + accel_alpha * (a_abs - accel_filt);
        noise = a_abs - accel_filt;

        // clamping to stop with noise
        if (accel_filt < 0.15)
            accel_filt = 0;

        // modifying the signal a bit
        if (accel_filt > 0)
            accel_filt_mod = sqrtf(accel_filt) + .5; // add a baseline and sqrt the signal to compress the signal. We just need to count peaks here
        else
            accel_filt_mod = 0;

        // define the envelope for peak detection
        if (accel_filt_mod > p_env)
            p_env = accel_filt_mod; // instant rise
        else
            p_env = p_env + p_alpha_release * (accel_filt_mod - p_env); // slow fall

        // threshold (95% of the envelope)
        p_threshold = p_env * .95;

        if (accel_filt_mod > 0.8f)
        {
            if (accel_filt_mod > p_threshold)
            {
                isAbove = true;
            }
            if (isAbove && accel_filt_mod <= p_threshold)
            {
                p_count++;
                isAbove = false;
            }
        }
        else
        {
            isAbove = false;
            peak_ready = false;
        }

        // debuging purposes
        printf(">a_mag:%f\n", a_mag);
        printf(">a_abs:%f\n", a_abs);
        printf(">accel_filt:%f\n", accel_filt);
        printf(">noise:%f\n", noise);
        printf(">alpha:%f\n", accel_alpha);
        printf(">p_env:%f\n", p_env);
        printf(">p_alpha_release:%f\n", p_alpha_release);
        printf(">p_threshold:%f\n", p_threshold);
        printf(">accel_filt_mod:%f\n", accel_filt_mod);
        printf(">p_count:%d\n", p_count);

        // make an envelope for peak detection
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
