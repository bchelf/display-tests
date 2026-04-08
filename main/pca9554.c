#include "pca9554.h"

#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_check.h"
#include "esp_log.h"

enum {
    PCA9554_REG_INPUT = 0x00,
    PCA9554_REG_OUTPUT = 0x01,
    PCA9554_REG_POLARITY = 0x02,
    PCA9554_REG_CONFIG = 0x03,
};

typedef struct {
    bool initialized;
    i2c_port_t port;
    uint8_t addr;
    uint8_t output_shadow;
    uint8_t direction_shadow;
} pca9554_state_t;

static const char *TAG = "pca9554";
static pca9554_state_t s_state;

static esp_err_t pca9554_write_reg(uint8_t reg, uint8_t value)
{
    const uint8_t payload[2] = {reg, value};
    return i2c_master_write_to_device(s_state.port, s_state.addr, payload, sizeof(payload), pdMS_TO_TICKS(100));
}

static esp_err_t pca9554_read_reg(uint8_t reg, uint8_t *value)
{
    ESP_RETURN_ON_FALSE(value != NULL, ESP_ERR_INVALID_ARG, TAG, "value pointer is null");
    return i2c_master_write_read_device(s_state.port, s_state.addr, &reg, sizeof(reg), value, 1, pdMS_TO_TICKS(100));
}

esp_err_t pca9554_init(i2c_port_t port, uint8_t addr)
{
    s_state = (pca9554_state_t) {
        .initialized = true,
        .port = port,
        .addr = addr,
        .output_shadow = 0x00,
        .direction_shadow = 0xFF,
    };

    ESP_RETURN_ON_ERROR(pca9554_write_reg(PCA9554_REG_POLARITY, 0x00), TAG, "failed to disable polarity inversion");

    esp_err_t err = pca9554_read_reg(PCA9554_REG_OUTPUT, &s_state.output_shadow);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "failed to read output shadow, using reset default: %s", esp_err_to_name(err));
        s_state.output_shadow = 0x00;
    }

    err = pca9554_read_reg(PCA9554_REG_CONFIG, &s_state.direction_shadow);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "failed to read direction shadow, using reset default: %s", esp_err_to_name(err));
        s_state.direction_shadow = 0xFF;
    }

    ESP_LOGI(TAG, "init complete on I2C port %d addr 0x%02X", port, addr);
    return ESP_OK;
}

esp_err_t pca9554_set_direction(uint8_t dir_mask)
{
    ESP_RETURN_ON_FALSE(s_state.initialized, ESP_ERR_INVALID_STATE, TAG, "driver not initialized");
    ESP_RETURN_ON_ERROR(pca9554_write_reg(PCA9554_REG_CONFIG, dir_mask), TAG, "failed to write direction register");
    s_state.direction_shadow = dir_mask;
    return ESP_OK;
}

esp_err_t pca9554_write_outputs(uint8_t value)
{
    ESP_RETURN_ON_FALSE(s_state.initialized, ESP_ERR_INVALID_STATE, TAG, "driver not initialized");
    ESP_RETURN_ON_ERROR(pca9554_write_reg(PCA9554_REG_OUTPUT, value), TAG, "failed to write output register");
    s_state.output_shadow = value;
    return ESP_OK;
}

esp_err_t pca9554_read_inputs(uint8_t *value)
{
    ESP_RETURN_ON_FALSE(s_state.initialized, ESP_ERR_INVALID_STATE, TAG, "driver not initialized");
    return pca9554_read_reg(PCA9554_REG_INPUT, value);
}

esp_err_t pca9554_set_bits(uint8_t mask)
{
    return pca9554_write_outputs(s_state.output_shadow | mask);
}

esp_err_t pca9554_clear_bits(uint8_t mask)
{
    return pca9554_write_outputs(s_state.output_shadow & (uint8_t) ~mask);
}

uint8_t pca9554_get_direction_shadow(void)
{
    return s_state.direction_shadow;
}

uint8_t pca9554_get_output_shadow(void)
{
    return s_state.output_shadow;
}
