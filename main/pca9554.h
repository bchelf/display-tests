#pragma once

#include <stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"

esp_err_t pca9554_init(i2c_port_t port, uint8_t addr);
esp_err_t pca9554_set_direction(uint8_t dir_mask);
esp_err_t pca9554_write_outputs(uint8_t value);
esp_err_t pca9554_read_inputs(uint8_t *value);
esp_err_t pca9554_set_bits(uint8_t mask);
esp_err_t pca9554_clear_bits(uint8_t mask);
uint8_t pca9554_get_direction_shadow(void);
uint8_t pca9554_get_output_shadow(void);
