#include "driver/i2c_master.h"
#include "driver/uart.h"
#include <stdio.h>
#include <stdbool.h>
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
#include "imu_service.h"
#include "signal_processing.h"
#include "timers.h"

#define REP_DECAY_MS 6700 // 6.7 SECOND

// variables---------------------------------------------
short ax, ay, az;               // raw accel values
float ax_g, ay_g, az_g;         // accel values in gs
float a_mag = 0;                // accel mag
float a_abs = 0;                // absolute accel sans gravity (in gs)
double accel_alpha = 0.08;      // alpha value to smooth out accel //was 0.6
float accel_filt = 0;           // accel filtered
float noise = 0;                // system noise
float accel_filt_mod = 0;       // accel filt modified for easier peak detection
int p_count = 0;                // peak count
float rep_count = 0;            // rep count
int startup_samples = 20;       // samples to ignore during startup
float p_env = 0;                // peak envelope
float p_threshold = 0;          // peak threshold
float p_alpha_release = .04;    // alpha for the peak envelope. Slow decent after peak in signal
float cutoff = 0.12f;           // clamp for our accel signal
float rep_decay_start_time = 0; // countdown for our rep decay
float effort_avg = 0;           // an estimation of the effort put in by the user
uint32_t p_t1 = 0, p_t2 = 0;    // peak time 1 and 2
float delta = 0, freq = 0;
bool peak = true;
uint64_t ind = 0;
uint32_t now_ms = 0;

lp_filt_output_t sig1,
    sig2, sig3, sig4;

// accel sample
accel_samples_t accel_sample;
lp_filt_output_t accel_filt_out;
peak_det_params_t pdp;
// flags---------------------------------------------------
bool isAbove = false;   // is the accel signal above p_threshold
bool initDecay = false; // has the rep decay countdown started

// new stopwatch object
stopwatch_obj p_interval_obj;

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

        if (startup_samples > 0)
        {
            startup_samples--;
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // READING ACCEL VALUES
        read_accel(&accel_sample);

        // // // reading from uart (debugging only)
        // char ch[6];
        // int len = uart_read_bytes(UART_NUM_0, &ch, 5, 0); // timeout = 0, non-blocking
        // if (len > 0)
        // {
        //     ch[len] = '\0';
        //     float value = strtof(ch, NULL);
        //     accel_alpha = value;
        // }

        // // Seperate noise from motion
        lowpass_filter(&accel_filt_out, accel_alpha, accel_sample.a_abs);
        // rep detection formula R = P-1/2

        env_creation_irsf(&p_env, accel_filt_out.sig_filt, p_alpha_release);

        // // threshold (95% of the envelope)
        p_threshold = p_env * .95;

        peak_detection(accel_filt_out.sig_filt, p_threshold, &pdp);

        // debuging purposes
        // printf(">cutoff:%f\n", cutoff);
        // printf(">a_mag:%f\n", accel_sample.a_mag);
        // printf(">a_abs:%f\n", accel_sample.a_abs);
        printf(">sig_filt:%f\n", accel_filt_out.sig_filt);
        printf(">p_threshold:%f\n", p_threshold);
        printf(">interval:%f\n", pdp.p_interval_s);
        printf(">p_flag:%d\n", pdp.peakFlag);
        printf(">effort:%f\n", pdp.interval_effort);
        printf(">effort/s:%f\n", pdp.effort_s);

        // printf(">accel_filt_mod:%f\n", accel_filt_mod);
        // printf(">noise:%f\n", noise);
        // printf(">alpha:%f\n", accel_alpha);
        printf(">p_env:%f\n", p_env);
        // printf(">p_alpha_release:%f\n", p_alpha_release);
        // printf(">p_count:%d\n", pdp.p_count);
        // printf(">rep_count:%f\n", pdp.rep_count);
        // printf(">p_count:%d\n", pdp.p_count);
        // printf(">rep_count:%f\n", pdp.rep_count);
        // printf(">delta:%f\n", delta);
        // printf(">freq:%f\n", freq);
        // printf(">peak:%d\n", peak);

        // printf("%lld,%f\n", ind++, sig1.sig_filt);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
