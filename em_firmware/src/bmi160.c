#include "bmi160.h"

static i2c_master_dev_handle_t bmi160_handle = NULL;
static const char *TAG = "MAX30102";

#define BMI160_PART_ID 0xD1

esp_err_t BMI160_init(i2c_master_bus_handle_t bus_handle)
{
    if (bus_handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BMI160_I2C_ADDR_DEFAULT,
        .scl_speed_hz = 400000, // 400khz
    };

    // return if device adding was a faliure
    ESP_RETURN_ON_ERROR(
        i2c_master_bus_add_device(bus_handle, &dev_config, &bmi160_handle),
        TAG,
        "failed to add bmi160 to I2C bus");

    // value of part ID
    uint8_t part_id = 0xAA; // an unlikely number to show up
    // check if connected
    ESP_RETURN_ON_ERROR(
        MAX30102_read_reg(BMI160_REG_CHIP_ID, &part_id),
        TAG,
        "failed to read BMI160 part ID register");

    if (part_id != BMI160_PART_ID)
    {
        return ESP_ERR_NOT_FOUND;
    }
    return ESP_OK;
}
esp_err_t BMI160_config(const bmi160_config_t *config)
{
    if (config == NULL)
        return ESP_ERR_INVALID_ARG;
}

esp_err_t BMI160_write_reg(
    uint8_t reg,
    uint8_t value);

esp_err_t BMI160_read_reg(
    uint8_t reg,
    uint8_t *value);

esp_err_t BMI160_read_bytes(
    uint8_t reg,
    uint8_t *buffer,
    size_t length);

esp_err_t BMI160_write_field(
    uint8_t reg,
    uint8_t mask,
    uint8_t shift,
    uint8_t value);

esp_err_t BMI160_reset(void);

esp_err_t BMI160_shutdown_toggle(bool *isShutDown);