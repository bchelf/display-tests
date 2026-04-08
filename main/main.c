#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "board_qualia_s3.h"
#include "esp_err.h"
#include "esp_log.h"
#include "panel_480x480.h"
#include "pca9554.h"
#include "rgb_display.h"

static const char *TAG = "main";

static inline uint16_t rgb565(uint8_t r5, uint8_t g6, uint8_t b5)
{
    return (uint16_t) ((r5 << 11) | (g6 << 5) | b5);
}

static void draw_rect(uint16_t *fb, int width, int stride_pixels, int x0, int y0, int x1, int y1, uint16_t color)
{
    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 > width) {
        x1 = width;
    }
    if (y1 > rgb_display_height()) {
        y1 = rgb_display_height();
    }

    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            fb[y * stride_pixels + x] = color;
        }
    }
}

static void render_test_pattern(void)
{
    const int width = rgb_display_width();
    const int height = rgb_display_height();
    const int stride_pixels = rgb_display_get_stride_bytes() / sizeof(uint16_t);
    uint16_t *fb = (uint16_t *) rgb_display_get_framebuffer();

    ESP_LOGI(TAG, "render test pattern to %dx%d framebuffer", width, height);

    const uint16_t bars[8] = {
        rgb565(31, 0, 0),
        rgb565(31, 32, 0),
        rgb565(0, 63, 0),
        rgb565(0, 63, 31),
        rgb565(0, 0, 31),
        rgb565(31, 0, 31),
        rgb565(31, 63, 31),
        rgb565(0, 0, 0),
    };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int bar = (x * 8) / width;
            fb[y * stride_pixels + x] = bars[bar];
        }
    }

    for (int x = 0; x < width; ++x) {
        fb[x] = rgb565(31, 63, 31);
        fb[(height - 1) * stride_pixels + x] = rgb565(31, 63, 31);
    }

    for (int y = 0; y < height; ++y) {
        fb[y * stride_pixels] = rgb565(31, 63, 31);
        fb[y * stride_pixels + (width - 1)] = rgb565(31, 63, 31);
    }

    const int checker_size = 24;
    for (int y = 120; y < 360; ++y) {
        for (int x = 120; x < 360; ++x) {
            const bool white = (((x - 120) / checker_size) ^ ((y - 120) / checker_size)) & 1;
            fb[y * stride_pixels + x] = white ? rgb565(31, 63, 31) : rgb565(0, 0, 0);
        }
    }

    draw_rect(fb, width, stride_pixels, 0, 0, 24, 24, rgb565(31, 0, 0));
    draw_rect(fb, width, stride_pixels, width - 24, 0, width, 24, rgb565(0, 63, 0));
    draw_rect(fb, width, stride_pixels, 0, height - 24, 24, height, rgb565(0, 0, 31));
    draw_rect(fb, width, stride_pixels, width - 24, height - 24, width, height, rgb565(31, 63, 31));
}

static void log_button_changes_task(void *arg)
{
    (void) arg;

    uint8_t last_inputs = 0xFF;

    while (true) {
        uint8_t inputs = 0xFF;
        if (pca9554_read_inputs(&inputs) == ESP_OK && inputs != last_inputs) {
            ESP_LOGI(TAG,
                     "PCA9554 inputs=0x%02X UP=%s DN=%s TP_IRQ=%s",
                     inputs,
                     board_button_is_pressed(inputs, BTN_UP_BIT) ? "pressed" : "released",
                     board_button_is_pressed(inputs, BTN_DN_BIT) ? "pressed" : "released",
                     board_button_is_pressed(inputs, TP_IRQ_BIT) ? "active" : "idle");
            last_inputs = inputs;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(board_i2c_init());
    ESP_ERROR_CHECK(pca9554_init(BOARD_I2C_PORT, PCA9554_ADDR));
    ESP_ERROR_CHECK(pca9554_set_direction(board_pca9554_default_direction_mask()));
    ESP_ERROR_CHECK(pca9554_write_outputs(board_pca9554_default_output_mask()));

    ESP_LOGI(TAG, "PCA9554 dir=0x%02X out=0x%02X", pca9554_get_direction_shadow(), pca9554_get_output_shadow());

    ESP_ERROR_CHECK(panel_480x480_backlight_set(false));
    ESP_ERROR_CHECK(panel_480x480_hard_reset());
    ESP_ERROR_CHECK(panel_480x480_init());
    ESP_ERROR_CHECK(rgb_display_init());

    render_test_pattern();

    ESP_ERROR_CHECK(panel_480x480_backlight_set(true));

    xTaskCreate(log_button_changes_task, "button_log", 3072, NULL, 5, NULL);
}
