#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"

#define MAX30102_ADDR 0X57

// Status
#define MAX30102_REG_INTR_STATUS_1 0x00
#define MAX30102_REG_INTR_STATUS_2 0x01
#define MAX30102_REG_INTR_ENABLE_1 0x02
#define MAX30102_REG_INTR_ENABLE_2 0x03

// FIFO
#define MAX30102_REG_FIFO_WR_PTR 0x04
#define MAX30102_REG_OVF_COUNTER 0x05
#define MAX30102_REG_FIFO_RD_PTR 0x06
#define MAX30102_REG_FIFO_DATA 0x07
#define MAX30102_REG_FIFO_CONFIG 0x08

// Config
#define MAX30102_REG_MODE_CONFIG 0x09
#define MAX30102_REG_SPO2_CONFIG 0x0A
#define MAX30102_REG_LED1_PA 0x0C // Red LED current
#define MAX30102_REG_LED2_PA 0x0D // IR LED current

// Die temperature
#define MAX30102_REG_TEMP_INT 0x1F
#define MAX30102_REG_TEMP_FRAC 0x20
#define MAX30102_REG_TEMP_CONFIG 0x21

// Part ID
#define MAX30102_REG_REV_ID 0xFE
#define MAX30102_REG_PART_ID 0xFF

#define MAX30102_CONFIG_DEFAULT() {   \
    .led_mode = 0x3,                  \
                                      \
    .sample_avg = 0x0,                \
    .sample_rate = 0x1,               \
    .pulse_width = 0x3,               \
    .adc_range = MAX30102_ADC_8192NA, \
                                      \
    .red_led_current = 0x24,          \
    .ir_led_current = 0x24,           \
                                      \
    .fifo_rollover_en = true,         \
    .fifo_almost_full = 0x0F,         \
                                      \
    .int_en = true}

typedef struct
{
    uint8_t led_mode;
    uint8_t sample_avg;
    uint8_t sample_rate;
    uint8_t pulse_width;
    uint8_t adc_range;
    uint8_t red_led_current;
    uint8_t ir_led_current;
    uint8_t fifo_almost_full;
    bool fifo_rollover_en;
    bool int_en;
} max30102_config_t;

esp_err_t MAX30102_init(i2c_master_bus_handle_t bus_handle);
esp_err_t MAX30102_config(const max30102_config_t *);

// generic functions

esp_err_t MAX30102_write_reg(
    uint8_t reg,
    uint8_t value);

esp_err_t MAX30102_read_reg(
    uint8_t reg,
    uint8_t *value);

esp_err_t MAX30102_read_bytes(
    uint8_t reg,
    uint8_t *buffer,
    size_t length);

/// @brief change values for that specific field
/// @param reg is the register we want changed
/// @param mask tells us exactly what we want changed
/// @param shift is what we need to offset our new value to fit that position
/// @param value is what we are changing it to
/// @return esp error code
esp_err_t MAX30102_write_field(
    uint8_t reg,
    uint8_t mask,
    uint8_t shift,
    uint8_t value);

// specific functions

/// @brief Reset the MAX30102
/// @param
/// @return esp error code
esp_err_t MAX30102_reset(void);
esp_err_t MAX30102_shutdown(void);
