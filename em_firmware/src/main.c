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
#include "driver/gpio.h"

#define REP_DECAY_MS 6700 // 6.7 SECOND

// variables---------------------------------------------
int16_t ax, ay, az;             // raw accel values
float ax_g, ay_g, az_g;         // accel values in gs
float a_mag = 0;                // accel mag
float a_abs = 0;                // absolute accel sans gravity (in gs)
double accel_alpha = .60;       // alpha value to smooth out accel
float accel_filt = 0;           // accel filtered
float noise = 0;                // system noise
float accel_filt_mod = 0;       // accel filt modified for easier peak detection
int p_count = 0;                // peak count
float rep_count = 0;            // rep count
int startup_samples = 20;       // samples to ignore during startup
float p_env = 0;                // peak envelope
float p_threshold = 0;          // peak threshold
float p_alpha_release = .25;    // alpha for the peak envelope. Slow decent after peak in signal
float cutoff = 0.12f;           // clamp for our accel signal
float rep_decay_start_time = 0; // countdown for our rep decay

// flags---------------------------------------------------
bool isAbove = false;   // is the accel signal above p_threshold
bool initDecay = false; // has the rep decay countdown started

void app_main()
{
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
        // simple timer
        uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (startup_samples > 0)
        {
            startup_samples--;
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // read accel values
        BMI160_read_accel_raw(&ax, &ay, &az);
        // turn those into gs
        ax_g = ax / 8192.0f;
        ay_g = ay / 8192.0f;
        az_g = az / 8192.0f;

        a_mag = sqrtf(ax_g * ax_g + ay_g * ay_g + az_g * az_g); // accel magnitude
        a_abs = fabsf(a_mag - 1.0f);                            // abs magnitude

        // reading from uart (debugging only)
        char ch[6];
        int len = uart_read_bytes(UART_NUM_0, &ch, 5, 0); // timeout = 0, non-blocking
        if (len > 0)
        {
            ch[len] = '\0';
            float value = strtof(ch, NULL);
            cutoff = value;
        }

        // Seperate noise from motion
        accel_filt = accel_filt + accel_alpha * (a_abs - accel_filt);
        noise = a_abs - accel_filt;

        // modifying the signal a bit
        if (accel_filt > 0)
            accel_filt_mod = sqrtf(accel_filt) + .5; // add a baseline and sqrt the signal to compress the signal. We just need to count peaks here
        else
            accel_filt_mod = 0;

        // clamping to stop with noise
        if (accel_filt <= cutoff)
            accel_filt_mod = 0; // clamping down our modded signal based off the filtered signal

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

            // new signal start? reset rep decay timer
            initDecay = false;
            rep_decay_start_time = 0;
        }
        else
        {
            isAbove = false;
            // end of a signal clump
            if (accel_filt_mod == 0)
            {
                // turn p_count into reps
                if (p_count > 0)
                {
                    rep_count += ((float)p_count - 1.0f) / 2.0f;
                    if (rep_count < 0)
                        rep_count = 0;

                    // reset p_count
                    p_count = 0;
                }

                if (!initDecay)
                {
                    // rep decay for half reps
                    rep_decay_start_time = now_ms;
                    initDecay = true; // we have started the decay
                }

                // check decay timer
                if (now_ms - rep_decay_start_time >= REP_DECAY_MS)
                {
                    rep_count = floorf(rep_count);
                }
            }
        }

        // debuging purposes
        // printf(">cutoff:%f\n", cutoff);
        // printf(">a_mag:%f\n", a_mag);
        // printf(">a_abs:%f\n", a_abs);
        // printf(">accel_filt:%f\n", accel_filt);
        // printf(">noise:%f\n", noise);
        // printf(">alpha:%f\n", accel_alpha);
        // printf(">p_env:%f\n", p_env);
        // printf(">p_alpha_release:%f\n", p_alpha_release);
        printf(">p_threshold:%f\n", p_threshold);
        printf(">accel_filt_mod:%f\n", accel_filt_mod);
        printf(">p_count:%d\n", p_count);
        printf(">rep_count:%f\n", rep_count);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
