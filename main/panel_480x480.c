#include "panel_480x480.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "board_qualia_s3.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "pca9554.h"

static const char *TAG = "panel_480x480";

#define PANEL_INIT_DELAY_FLAG 0x80

/*
 * Default profile:
 * TL034WVS05-B1477A 3.4" 480x480 panel on Adafruit Qualia ESP32-S3.
 *
 * Source:
 * https://learn.adafruit.com/adafruit-qualia-esp32-s3-for-rgb666-displays/qualia-rgb666-with-tl034-3-4-480x480-square-display
 *
 * Table format:
 *   cmd, arg_count | optional PANEL_INIT_DELAY_FLAG, args..., [delay_ms if flagged]
 *
 * Replace this table if your 480x480 panel is different.
 */
static const uint8_t s_panel_init_sequence[] = {
    0xFF, 0x05, 0x77, 0x01, 0x00, 0x00, 0x13,
    0xEF, 0x01, 0x08,
    0xFF, 0x05, 0x77, 0x01, 0x00, 0x00, 0x10,
    0xC0, 0x02, 0x3B, 0x00,
    0xC1, 0x02, 0x12, 0x0A,
    0xC2, 0x02, 0x07, 0x03,
    0xC3, 0x01, 0x02,
    0xCC, 0x01, 0x10,
    0xCD, 0x01, 0x08,
    0xB0, 0x10, 0x0F, 0x11, 0x17, 0x15, 0x15, 0x09, 0x0C, 0x08, 0x08, 0x26, 0x04, 0x59, 0x16, 0x66, 0x2D, 0x1F,
    0xB1, 0x10, 0x0F, 0x11, 0x17, 0x15, 0x15, 0x09, 0x0C, 0x08, 0x08, 0x26, 0x04, 0x59, 0x16, 0x66, 0x2D, 0x1F,
    0xFF, 0x05, 0x77, 0x01, 0x00, 0x00, 0x11,
    0xB0, 0x01, 0x6D,
    0xB1, 0x01, 0x3A,
    0xB2, 0x01, 0x01,
    0xB3, 0x01, 0x80,
    0xB5, 0x01, 0x49,
    0xB7, 0x01, 0x85,
    0xB8, 0x01, 0x20,
    0xC1, 0x01, 0x78,
    0xC2, 0x01, 0x78,
    0xD0, 0x01, 0x88,
    0xE0, 0x03, 0x00, 0x00, 0x02,
    0xE1, 0x0B, 0x07, 0x00, 0x09, 0x00, 0x06, 0x00, 0x08, 0x00, 0x00, 0x33, 0x33,
    0xE2, 0x0D, 0x11, 0x11, 0x33, 0x33, 0xF6, 0x00, 0xF6, 0x00, 0xF6, 0x00, 0xF6, 0x00, 0x00,
    0xE3, 0x04, 0x00, 0x00, 0x11, 0x11,
    0xE4, 0x02, 0x44, 0x44,
    0xE5, 0x10, 0x0F, 0xF3, 0x3D, 0xFF, 0x11, 0xF5, 0x3D, 0xFF, 0x0B, 0xEF, 0x3D, 0xFF, 0x0D, 0xF1, 0x3D, 0xFF,
    0xE6, 0x04, 0x00, 0x00, 0x11, 0x11,
    0xE7, 0x02, 0x44, 0x44,
    0xE8, 0x10, 0x0E, 0xF2, 0x3D, 0xFF, 0x10, 0xF4, 0x3D, 0xFF, 0x0A, 0xEE, 0x3D, 0xFF, 0x0C, 0xF0, 0x3D, 0xFF,
    0xE9, 0x02, 0x36, 0x00,
    0xEB, 0x07, 0x00, 0x01, 0xE4, 0xE4, 0x44, 0xAA, 0x10,
    0xEC, 0x02, 0x3C, 0x00,
    0xED, 0x10, 0xFF, 0x45, 0x67, 0xFA, 0x01, 0x2B, 0xCF, 0xFF, 0xFF, 0xFC, 0xB2, 0x10, 0xAF, 0x76, 0x54, 0xFF,
    0xEF, 0x06, 0x10, 0x0D, 0x04, 0x08, 0x3F, 0x1F,
    0xFF, 0x05, 0x77, 0x01, 0x00, 0x00, 0x00,
    0x35, 0x01, 0x00,
    0x3A, 0x01, 0x66,
    0x11, PANEL_INIT_DELAY_FLAG, 120,
    0x29, PANEL_INIT_DELAY_FLAG, 50,
};

static const panel_timing_config_t s_timing = {
    .pclk_hz = 16000000,
    .width = 480,
    .height = 480,
    .hsync_pulse_width = 20,
    .hsync_front_porch = 40,
    .hsync_back_porch = 40,
    .vsync_pulse_width = 10,
    .vsync_front_porch = 20,
    .vsync_back_porch = 20,
    .hsync_idle_low = false,
    .vsync_idle_low = false,
    .de_idle_high = false,
    .pclk_active_high = false,
    .pclk_idle_high = false,
};

static esp_err_t panel_apply_outputs(uint8_t value)
{
    return pca9554_write_outputs(value);
}

static esp_err_t panel_set_output_level(uint8_t bit, bool high)
{
    const uint8_t mask = PCA9554_BIT(bit);
    const uint8_t next_value = high ? (pca9554_get_output_shadow() | mask)
                                    : (pca9554_get_output_shadow() & (uint8_t) ~mask);
    return panel_apply_outputs(next_value);
}

static esp_err_t panel_bus_idle(void)
{
    uint8_t value = pca9554_get_output_shadow();
    value |= PCA9554_BIT(TFT_CS_BIT);
    value &= (uint8_t) ~PCA9554_BIT(TFT_SCK_BIT);
    value &= (uint8_t) ~PCA9554_BIT(TFT_MOSI_BIT);
    return panel_apply_outputs(value);
}

static esp_err_t panel_shift9(uint16_t word)
{
    uint8_t value = pca9554_get_output_shadow();
    value &= (uint8_t) ~PCA9554_BIT(TFT_CS_BIT);
    value &= (uint8_t) ~PCA9554_BIT(TFT_SCK_BIT);
    ESP_RETURN_ON_ERROR(panel_apply_outputs(value), TAG, "assert CS failed");

    for (int bit = 8; bit >= 0; --bit) {
        if (word & (1U << bit)) {
            value |= PCA9554_BIT(TFT_MOSI_BIT);
        } else {
            value &= (uint8_t) ~PCA9554_BIT(TFT_MOSI_BIT);
        }

        ESP_RETURN_ON_ERROR(panel_apply_outputs(value), TAG, "set MOSI failed");
        esp_rom_delay_us(1);

        value |= PCA9554_BIT(TFT_SCK_BIT);
        ESP_RETURN_ON_ERROR(panel_apply_outputs(value), TAG, "set SCK high failed");
        esp_rom_delay_us(1);

        value &= (uint8_t) ~PCA9554_BIT(TFT_SCK_BIT);
        ESP_RETURN_ON_ERROR(panel_apply_outputs(value), TAG, "set SCK low failed");
        esp_rom_delay_us(1);
    }

    value |= PCA9554_BIT(TFT_CS_BIT);
    value &= (uint8_t) ~PCA9554_BIT(TFT_MOSI_BIT);
    return panel_apply_outputs(value);
}

static esp_err_t panel_write_command(uint8_t command, const uint8_t *data, size_t data_len)
{
    ESP_RETURN_ON_ERROR(panel_shift9(command), TAG, "command 0x%02X failed", command);

    for (size_t i = 0; i < data_len; ++i) {
        ESP_RETURN_ON_ERROR(panel_shift9(0x100U | data[i]), TAG, "data byte %u failed", (unsigned) i);
    }

    return ESP_OK;
}

static esp_err_t panel_run_init_sequence(const uint8_t *sequence, size_t len)
{
    size_t index = 0;

    while (index < len) {
        const uint8_t command = sequence[index++];
        uint8_t arg_count = sequence[index++];
        const bool has_delay = (arg_count & PANEL_INIT_DELAY_FLAG) != 0;
        arg_count &= (uint8_t) ~PANEL_INIT_DELAY_FLAG;

        ESP_RETURN_ON_FALSE(index + arg_count <= len, ESP_ERR_INVALID_SIZE, TAG, "init sequence truncated");
        ESP_RETURN_ON_ERROR(panel_write_command(command, &sequence[index], arg_count), TAG, "init command 0x%02X failed", command);
        index += arg_count;

        if (has_delay) {
            ESP_RETURN_ON_FALSE(index < len, ESP_ERR_INVALID_SIZE, TAG, "missing delay byte");
            vTaskDelay(pdMS_TO_TICKS(sequence[index++]));
        }
    }

    return ESP_OK;
}

esp_err_t panel_480x480_hard_reset(void)
{
    ESP_LOGI(TAG, "panel hard reset");
    ESP_RETURN_ON_ERROR(panel_set_output_level(TFT_RESET_BIT, false), TAG, "reset low failed");
    vTaskDelay(pdMS_TO_TICKS(20));
    ESP_RETURN_ON_ERROR(panel_set_output_level(TFT_RESET_BIT, true), TAG, "reset high failed");
    vTaskDelay(pdMS_TO_TICKS(120));
    return panel_bus_idle();
}

esp_err_t panel_480x480_init(void)
{
    ESP_LOGI(TAG, "panel init sequence start");
    ESP_RETURN_ON_ERROR(panel_bus_idle(), TAG, "bus idle failed");
    ESP_RETURN_ON_ERROR(panel_run_init_sequence(s_panel_init_sequence, sizeof(s_panel_init_sequence)), TAG, "panel init sequence failed");
    ESP_LOGI(TAG, "panel init sequence done");
    return ESP_OK;
}

esp_err_t panel_480x480_backlight_set(bool on)
{
    ESP_LOGI(TAG, "backlight %s", on ? "on" : "off");
    return panel_set_output_level(BACKLIGHT_BIT, on);
}

const panel_timing_config_t *panel_480x480_get_timing(void)
{
    return &s_timing;
}
