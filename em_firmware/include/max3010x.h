#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

#define MAX3010X_ADDR 0X57

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


esp_err_t MAX3010X_init(i2c_master_bus_handle_t bus_handle);
esp_err_t MAX3010X_config(void);

esp_err_t MAX3010X_write_reg(uint8_t reg, uint8_t value);
esp_err_t MAX3010X_read_reg(uint8_t reg, uint8_t *value);
esp_err_t MAX3010X_read_bytes(uint8_t reg, uint8_t *buffer, size_t length);