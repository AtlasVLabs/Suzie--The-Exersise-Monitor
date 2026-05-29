#include "bmi160.h"

static i2c_master_dev_handle_t bmi160_handle = NULL;
static const char *TAG = "BMI160";

#define BMI160_PART_ID 0xD1

esp_err_t BMI160_init(i2c_master_bus_handle_t bus_handle)
{
    if (bus_handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BMI160_I2C_ADDR_HIGH,
        .scl_speed_hz = 400000, // 400khz
    };

    // return if device adding was a faliure
    ESP_RETURN_ON_ERROR(
        i2c_master_bus_add_device(bus_handle, &dev_config, &bmi160_handle),
        TAG,
        "failed to add bmi160 to I2C bus");

    if (bmi160_handle == NULL)
    {
        ESP_LOGE(TAG, "BMI160 handle is NULL after add_device");
        return ESP_FAIL;
    }

    // value of part ID
    uint8_t part_id = 0x00;
    // check if connected
    ESP_RETURN_ON_ERROR(
        BMI160_read_reg(BMI160_REG_CHIP_ID, &part_id),
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

    // soft reset
    ESP_RETURN_ON_ERROR(
        BMI160_soft_reset(),
        TAG,
        "BMI160 soft reset failed");

    // set accel pwm mode (apm)
    ESP_RETURN_ON_ERROR(
        BMI160_set_accel_pwr_mode(config->apm),
        TAG,
        "failed to set accel power mode");

    ESP_RETURN_ON_ERROR(
        BMI160_set_gyro_pwr_mode(config->gpm),
        TAG,
        "failed to set gyro power mode");

    // accel and gyro configs
    ESP_RETURN_ON_ERROR(
        BMI160_write_reg(BMI160_REG_ACC_CONF, 0x28),
        TAG,
        "failed to write ACC_CONF");

    ESP_RETURN_ON_ERROR(
        BMI160_write_reg(BMI160_REG_ACC_RANGE, 0x05),
        TAG,
        "failed to write ACC_RANGE");

    ESP_RETURN_ON_ERROR(
        BMI160_write_reg(BMI160_REG_GYR_CONF, 0x28),
        TAG,
        "failed to write GYR_CONF");

    ESP_RETURN_ON_ERROR(
        BMI160_write_reg(BMI160_REG_GYR_RANGE, 0x02),
        TAG,
        "failed to write GYR_RANGE");

    return ESP_OK;
}

esp_err_t BMI160_write_reg(
    uint8_t reg,
    uint8_t value)
{
    if (bmi160_handle == NULL)
    {
        ESP_LOGE(TAG, "BMI160 write failed: device handle is NULL");
        return ESP_ERR_INVALID_STATE;
    }

    // writing to an 8bit register
    const uint8_t data[2] = {
        reg,
        value,
    };

    return i2c_master_transmit(
        bmi160_handle,
        data,
        sizeof(data),
        -1);
}

esp_err_t BMI160_read_reg(
    uint8_t reg,
    uint8_t *value)
{
    if (bmi160_handle == NULL)
    {
        ESP_LOGE(TAG, "BMI160 write failed: device handle is NULL");
        return ESP_ERR_INVALID_STATE;
    }

    if (value == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return i2c_master_transmit_receive(
        bmi160_handle,
        &reg,
        1,
        value,
        1,
        -1);
}

esp_err_t BMI160_read_bytes(
    uint8_t reg,
    uint8_t *buffer,
    size_t length)
{
    return i2c_master_transmit_receive(
        bmi160_handle,
        &reg,
        1,
        buffer,
        length,
        -1);
}

esp_err_t BMI160_write_field(
    uint8_t reg,
    uint8_t mask,
    uint8_t shift,
    uint8_t value)
{
    uint8_t reg_value = 0x00; // what is in our register
    // read the register
    ESP_RETURN_ON_ERROR(
        BMI160_read_reg(reg, &reg_value),
        TAG,
        "failed to read register before updating bit field");

    // clear out bits we need to change
    reg_value &= ~mask;
    // add in the new value shifted to the right place
    reg_value |= (value << shift) & mask;
    // write the new value t the register
    ESP_RETURN_ON_ERROR(
        BMI160_write_reg(reg, reg_value),
        TAG,
        "failed to write updated bit field to register");
    return ESP_OK;
}

esp_err_t BMI160_soft_reset(void)
{
    ESP_RETURN_ON_ERROR(
        BMI160_write_reg(BMI160_REG_CMD, 0xB6),
        TAG,
        "failed to soft reset BMI160");

    vTaskDelay(pdMS_TO_TICKS(50));

    return ESP_OK;
}

esp_err_t BMI160_int_reset(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t BMI160_step_ctr_reset(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t BMI160_set_accel_pwr_mode(accel_pwr_mode_t apm)
{
    ESP_RETURN_ON_ERROR(
        BMI160_write_reg(BMI160_REG_CMD, apm),
        TAG,
        "failed to set BMI160 accel power mode");

    // rounded up from 3.2
    vTaskDelay(pdMS_TO_TICKS(4));

    return ESP_OK;
}

esp_err_t BMI160_set_gyro_pwr_mode(gyro_pwr_mode_t gpm)
{
    ESP_RETURN_ON_ERROR(
        BMI160_write_reg(BMI160_REG_CMD, gpm),
        TAG,
        "failed to set BMI160 gyro power mode");

    // 55ms delay for startup
    vTaskDelay(pdMS_TO_TICKS(55));

    return ESP_OK;
}

static int16_t BMI160_make_int16(uint8_t lsb, uint8_t msb)
{
    return (int16_t)((msb << 8) | lsb);
}

esp_err_t BMI160_read_accel_raw(int16_t *ax, int16_t *ay, int16_t *az)
{
    if (ax == NULL || ay == NULL || az == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[6] = {0};

    ESP_RETURN_ON_ERROR(
        BMI160_read_bytes(BMI160_REG_ACC_X_LSB, data, sizeof(data)),
        TAG,
        "failed to read accel data");

    *ax = BMI160_make_int16(data[0], data[1]);
    *ay = BMI160_make_int16(data[2], data[3]);
    *az = BMI160_make_int16(data[4], data[5]);

    return ESP_OK;
}

esp_err_t BMI160_read_gyro_raw(int16_t *gx, int16_t *gy, int16_t *gz)
{
    if (gx == NULL || gy == NULL || gz == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[6] = {0};

    ESP_RETURN_ON_ERROR(
        BMI160_read_bytes(BMI160_REG_GYR_X_LSB, data, sizeof(data)),
        TAG,
        "failed to read gyro data");

    *gx = BMI160_make_int16(data[0], data[1]);
    *gy = BMI160_make_int16(data[2], data[3]);
    *gz = BMI160_make_int16(data[4], data[5]);

    return ESP_OK;
}