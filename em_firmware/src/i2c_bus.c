#include "i2c_bus.h"

static i2c_master_bus_handle_t bus_handle = NULL;
esp_err_t i2c_bus_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t err_code = i2c_new_master_bus(&bus_config, &bus_handle);
    return err_code;
}

i2c_master_bus_handle_t get_master_bus_handle(void)
{
    return bus_handle;
}

