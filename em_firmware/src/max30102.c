#include "max30102.h"

#define MAX30102_PART_ID 0X15

static i2c_master_dev_handle_t max30102_handle = NULL;
static const char *TAG = "MAX30102";

esp_err_t MAX30102_init(i2c_master_bus_handle_t bus_handle)
{
    if (bus_handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MAX30102_ADDR,
        .scl_speed_hz = 400000, // 400khz
    };

    // return if device adding was a faliure
    ESP_RETURN_ON_ERROR(
        i2c_master_bus_add_device(bus_handle, &dev_config, &max30102_handle),
        TAG,
        "failed to add MAX30102 to I2C bus");

    // value of part ID
    uint8_t part_id = 0xAA; // an unlikely number to show up
    // check if connected
    ESP_RETURN_ON_ERROR(
        MAX30102_read_reg(MAX30102_REG_PART_ID, &part_id),
        TAG,
        "failed to read MAX30102 part ID register");

    if (part_id != MAX30102_PART_ID)
    {
        return ESP_ERR_NOT_FOUND;
    }
    return ESP_OK;
}

esp_err_t MAX30102_config(const max30102_config_t *config)
{
    if (config == NULL)
        return ESP_ERR_INVALID_ARG;

    // reset the sensor
    ESP_RETURN_ON_ERROR(
        MAX30102_reset(),
        TAG,
        "failed to reset MAX30102");

    // Clear the FIFO
    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(MAX30102_REG_FIFO_WR_PTR, 0x00),
        TAG,
        "failed to clear FIFO write pointer");

    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(MAX30102_REG_OVF_COUNTER, 0x00),
        TAG,
        "failed to clear FIFO overflow counter");

    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(MAX30102_REG_FIFO_RD_PTR, 0x00),
        TAG,
        "failed to clear FIFO read pointer");

    // Set FIFO behavior

    uint8_t fifo_config = ((config->sample_avg & 0x7) << 5) |
                          ((config->fifo_rollover_en & 0x1) << 4) |
                          (config->fifo_almost_full & 0xF);

    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(MAX30102_REG_FIFO_CONFIG, fifo_config),
        TAG,
        "failed to write the FIFO config reg");

    // Set sample rate / pulse width / ADC range
    uint8_t spo2_config = (config->adc_range & 0x03) << 5 |
                          (config->sample_rate & 0x07) << 2 |
                          (config->pulse_width & 0x03);

    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(MAX30102_REG_SPO2_CONFIG, spo2_config),
        TAG,
        "failed to write the SpO2 config reg");
    // Set LED current
    // red led
    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(MAX30102_REG_LED1_PA, config->red_led_current),
        TAG,
        "failed to set red LED current");
    // IR led
    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(MAX30102_REG_LED2_PA, config->ir_led_current),
        TAG,
        "failed to set IR LED current");

    // Set mode
    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(MAX30102_REG_MODE_CONFIG, config->led_mode),
        TAG,
        "failed to set MAX30102 mode");

    return ESP_OK;
}

esp_err_t MAX30102_write_reg(
    uint8_t reg,
    uint8_t value)
{
    // writing to an 8bit register
    const uint8_t data[2] = {
        reg,
        value,
    };

    return i2c_master_transmit(
        max30102_handle,
        data,
        sizeof(data),
        -1);
}

esp_err_t MAX30102_read_reg(
    uint8_t reg,
    uint8_t *value)
{
    return i2c_master_transmit_receive(
        max30102_handle,
        &reg,
        1,
        value,
        1,
        -1);
}

esp_err_t max30102_read_bytes(
    uint8_t reg,
    uint8_t *buffer,
    size_t length)
{
    return i2c_master_transmit_receive(
        max30102_handle,
        &reg,
        1,
        buffer,
        length,
        -1);
}

esp_err_t MAX30102_write_field(
    uint8_t reg,
    uint8_t mask,
    uint8_t shift,
    uint8_t value)
{

    uint8_t reg_value = 0x00; // what is in our register
    // read the register
    ESP_RETURN_ON_ERROR(
        MAX30102_read_reg(reg, &reg_value),
        TAG,
        "failed to read register before updating bit field");

    // clear out bits we need to change
    reg_value &= ~mask;
    // add in the new value shifted to the right place
    reg_value |= (value << shift) & mask;
    // write the new value t the register
    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(reg, reg_value),
        TAG,
        "failed to write updated bit field to register");
    return ESP_OK;
}

esp_err_t MAX30102_reset(void)
{
    ESP_RETURN_ON_ERROR(
        MAX30102_write_field(
            MAX30102_REG_MODE_CONFIG,
            0x40,
            6,
            1),
        TAG,
        "failed to set MAX30102 reset bit");

    // read reset bit till it is 0 again
    uint8_t value = 0x0;
    do
    {
        ESP_RETURN_ON_ERROR(
            MAX30102_read_reg(
                MAX30102_REG_MODE_CONFIG,
                &value),
            TAG,
            "cannot read the reset bit");

        // mask and bit shift
        value = (value & 0x40) >> 6;

    } while (value);

    return ESP_OK;
}

esp_err_t MAX30102_shutdown_toggle(bool *isShutDown)
{
    if (isShutDown == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t value = 0x00;

    // read SHDN bit
    ESP_RETURN_ON_ERROR(
        MAX30102_read_reg(
            MAX30102_REG_MODE_CONFIG,
            &value),
        TAG,
        "failed to read mode config reg");

    // flip the SHDN bit (bit 7)
    value ^= (1 << 7);

    // put it back where u found it
    ESP_RETURN_ON_ERROR(
        MAX30102_write_reg(
            MAX30102_REG_MODE_CONFIG,
            value),
        TAG,
        "failed to write mode config reg");

    *isShutDown = (value & (1 << 7)) != 0;
    return ESP_OK;
}