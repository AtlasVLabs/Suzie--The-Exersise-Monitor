#include "max3010x.h"
static i2c_master_dev_handle_t max3010x_handle = NULL;
esp_err_t MAX3010X_init(i2c_master_bus_handle_t bus_handle)
{
    if (bus_handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MAX3010X_ADDR,
        .scl_speed_hz = 400000, // 400khz
    };

    return i2c_master_bus_add_device(bus_handle, &dev_config, &max3010x_handle);
}

esp_err_t MAX3010X_config()
{
    return ESP_OK;
}

esp_err_t MAX3010X_write_reg(uint8_t reg, uint8_t value)
{
    // writing to an 8bit register
    const uint8_t data[2] = {
        reg,
        value,
    };

    return i2c_master_transmit(
        max3010x_handle,
        data,
        sizeof(data),
        -1);
}

esp_err_t MAX3010X_read_reg(uint8_t reg, uint8_t *value)
{
    return i2c_master_transmit_receive(
        max3010x_handle,
        &reg,
        1,
        value,
        1,
        -1);
}

esp_err_t max30102_read_bytes(uint8_t reg, uint8_t *buffer, size_t length)
{
    return i2c_master_transmit_receive(
        max3010x_handle,
        &reg,
        1,
        buffer,
        length,
        -1);
}
