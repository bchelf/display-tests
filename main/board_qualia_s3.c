#include "board_qualia_s3.h"

#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "board";

const int board_lcd_red_pins[5] = {11, 10, 9, 46, 3};
const int board_lcd_green_pins[6] = {48, 47, 21, 14, 13, 12};
const int board_lcd_blue_pins[5] = {40, 39, 38, 0, 45};

esp_err_t board_i2c_init(void)
{
    const i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BOARD_I2C_SDA,
        .scl_io_num = BOARD_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = BOARD_I2C_FREQ_HZ,
        .clk_flags = 0,
    };

    ESP_LOGI(TAG, "init I2C on SDA=%d SCL=%d @ %d Hz", BOARD_I2C_SDA, BOARD_I2C_SCL, BOARD_I2C_FREQ_HZ);
    ESP_RETURN_ON_ERROR(i2c_param_config(BOARD_I2C_PORT, &config), TAG, "i2c_param_config failed");
    return i2c_driver_install(BOARD_I2C_PORT, config.mode, 0, 0, 0);
}

uint8_t board_pca9554_default_direction_mask(void)
{
    return PCA9554_BIT(TP_IRQ_BIT) |
           PCA9554_BIT(BTN_UP_BIT) |
           PCA9554_BIT(BTN_DN_BIT);
}

uint8_t board_pca9554_default_output_mask(void)
{
    return PCA9554_BIT(TFT_CS_BIT) |
           PCA9554_BIT(TFT_RESET_BIT);
}

bool board_button_is_pressed(uint8_t input_state, uint8_t bit_index)
{
    return (input_state & PCA9554_BIT(bit_index)) == 0;
}
