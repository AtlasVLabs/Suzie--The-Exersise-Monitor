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

    uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0); //

    i2c_bus_init();
    MAX30102_init(get_master_bus_handle());
    BMI160_init(get_master_bus_handle());

    max30102_config_t maxconfig = MAX30102_CONFIG_DEFAULT();
    bmi160_config_t bmiconfig = {
        .apm = bmi160_accel_pwr_normal,
        .gpm = bmi160_gyro_pwr_normal};

    BMI160_config(&bmiconfig);
    MAX30102_config(&maxconfig);

    int16_t ax, ay, az;
    float ax_g, ay_g, az_g;
    double accel_alpha = .3;
    double jitter_alpha = .75;
    float a_filtered = 0, noise = 0, jitter = 0;
    float temp = 0;

    while (1)
    {
        BMI160_read_accel_raw(&ax, &ay, &az);

        ax_g = ax / 8192.0f;
        ay_g = ay / 8192.0f;
        az_g = az / 8192.0f;

        // printf(">ax_g:%f\n", ax_g);
        // printf(">ay_g:%f\n", ay_g);
        // printf(">az_g:%f\n", az_g);

        // seperation of noise vs reg motion
        // get our magnitude
        float a_mag = sqrtf(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
        float a_abs = fabsf(a_mag - 1.0f);

        // // reading from uart
        // char ch[5];

        // int len = uart_read_bytes(UART_NUM_0, &ch, 5, 0); // timeout = 0, non-blocking
        // if (len > 0)
        // {
        //     float value = strtof(ch, NULL);
        //     if (alpha + value >= 0 && alpha + value <= 1)
        //     {
        //         alpha += value;
        //     }
        //     else if (alpha + value > 1)
        //     {
        //         alpha = 1;
        //     }
        //     else if (alpha + value < 0)
        //     {
        //         alpha = 0;
        //     }
        // }

        // signal = fast + slow
        a_filtered = a_filtered + accel_alpha * (a_abs - a_filtered);
        noise = a_abs - a_filtered;
        
        //should help with some noise
        if(a_filtered < 0.2){
            a_filtered = 0;
        }


        temp = temp + jitter_alpha * (a_abs - temp);
        jitter = a_abs - temp;
        jitter = jitter * jitter;

        printf(">a_mag:%f\n", a_mag);
        printf(">a_filtered:%f\n", a_filtered);
        printf(">noise:%f\n", noise);
        printf(">jitter:%f\n", jitter);
        printf(">jitterx2:%f\n", jitter * jitter);
       
        printf(">temp:%f\n", temp);

        // make an envelope for peak detection

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Read sensor task

// Process Data task

//_______________________________________

// FUNCTIONS
