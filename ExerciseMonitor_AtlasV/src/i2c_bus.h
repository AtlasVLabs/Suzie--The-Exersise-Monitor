#include "driver/i2c_master.h"
#include "esp_err.h"

// data and clock pins
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

esp_err_t i2c_bus_init(i2c_master_bus_handle_t *bus_handle);