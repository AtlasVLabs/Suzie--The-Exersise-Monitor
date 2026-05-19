#include "driver/i2c_master.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "max30102.h"
#include "i2c_bus.h"

void app_main()
{
    i2c_bus_init();
    MAX30102_init(get_master_bus_handle());

    max30102_config_t maxconfig = MAX30102_CONFIG_DEFAULT();
    MAX30102_config(&maxconfig);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Read sensor task

// Process Data task

//_______________________________________

// FUNCTIONS
