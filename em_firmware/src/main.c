#include "driver/i2c_master.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "max30102.h"
#include "bmi160.h"
#include "i2c_bus.h"

void app_main()
{
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

    while (1)
    {
        BMI160_read_accel_raw(&ax, &ay, &az);

        vTaskDelay(pdMS_TO_TICKS(1));

        ax_g = ax / 8192.0f;
        ay_g = ay / 8192.0f;
        az_g = az / 8192.0f;

        printf(">ax_g:%f\n", ax_g);
        printf(">ay_g:%f\n", ay_g);
        printf(">az_g:%f\n", az_g);
    }
}

// Read sensor task

// Process Data task

//_______________________________________

// FUNCTIONS
