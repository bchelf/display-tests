#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"

#define LCD_DE       2
#define LCD_VSYNC    42
#define LCD_HSYNC    41
#define LCD_PCLK     1

#define BOARD_I2C_SDA   8
#define BOARD_I2C_SCL   18
#define BOARD_SPI_SCK   5
#define BOARD_SPI_MISO  6
#define BOARD_SPI_MOSI  7
#define BOARD_CS        15
#define BOARD_A0        17
#define BOARD_A1        16
#define BOARD_TX        43
#define BOARD_RX        44

#define PCA9554_ADDR      0x3F
#define TFT_SCK_BIT       0
#define TFT_CS_BIT        1
#define TFT_RESET_BIT     2
#define TP_IRQ_BIT        3
#define BACKLIGHT_BIT     4
#define BTN_UP_BIT        5
#define BTN_DN_BIT        6
#define TFT_MOSI_BIT      7

#define BOARD_I2C_PORT       I2C_NUM_0
#define BOARD_I2C_FREQ_HZ    400000

#define PCA9554_BIT(n)       (1U << (n))

extern const int board_lcd_red_pins[5];
extern const int board_lcd_green_pins[6];
extern const int board_lcd_blue_pins[5];

esp_err_t board_i2c_init(void);
uint8_t board_pca9554_default_direction_mask(void);
uint8_t board_pca9554_default_output_mask(void);
bool board_button_is_pressed(uint8_t input_state, uint8_t bit_index);
