#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BMI160_I2C_ADDR_DEFAULT 0x68

#define BMI160_REG_CHIP_ID 0x00
#define BMI160_REG_ERR 0x02
#define BMI160_REG_PMU_STATUS 0x03

#define BMI160_REG_GYR_X_LSB 0x0C
#define BMI160_REG_ACC_X_LSB 0x12

#define BMI160_REG_STATUS 0x1B

#define BMI160_REG_ACC_CONF 0x40
#define BMI160_REG_ACC_RANGE 0x41
#define BMI160_REG_GYR_CONF 0x42
#define BMI160_REG_GYR_RANGE 0x43

#define BMI160_REG_CMD 0x7E
#define BMI160_CHIP_ID_VALUE 0xD1

typedef struct
{
    accel_pwr_mode_t apm;
    gyro_pwr_mode_t gpm;

} bmi160_config_t;

typedef enum
{
    bmi160_accel_pwr_suspend = 0x10,
    bmi160_accel_pwr_normal = 0x11,
    bmi160_accel_pwr_low_pwr = 0x12,

}
accel_pwr_mode_t;

typedef enum
{
    bmi160_agyro_pwr_suspend = 0x14,
    bmi160_agyro_pwr_normal = 0x15,
    bmi160_agyro_pwr_fast_startup = 0x17
} gyro_pwr_mode_t;

esp_err_t BMI160_init(i2c_master_bus_handle_t bus_handle);
esp_err_t BMI160_config(const bmi160_config_t *);

// generic functions

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

esp_err_t BMI160_soft_reset(void);

esp_err_t BMI160_int_reset(void);

esp_err_t BMI160_step_ctr_reset(void);

esp_err_t BMI160_set_accel_pwr_mode(accel_pwr_mode);

esp_err_t BMI160_set_gyro_pwr_mode(gyro_pwr_mode);