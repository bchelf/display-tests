#include "rgb_display.h"

#include <stdbool.h>
#include <stdint.h>

#include "board_qualia_s3.h"
#include "esp_check.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_log.h"
#include "panel_480x480.h"

static const char *TAG = "rgb_display";

static esp_lcd_panel_handle_t s_panel_handle;
static void *s_framebuffer;
static size_t s_stride_bytes;

esp_err_t rgb_display_init(void)
{
    if (s_panel_handle != NULL) {
        return ESP_OK;
    }

    const panel_timing_config_t *timing = panel_480x480_get_timing();

    /*
     * The ESP-IDF RGB bus expects D0..D15 ordering for RGB565:
     * B0..B4, G0..G5, R0..R4.
     */
    const int data_gpio_nums[16] = {
        board_lcd_blue_pins[0],
        board_lcd_blue_pins[1],
        board_lcd_blue_pins[2],
        board_lcd_blue_pins[3],
        board_lcd_blue_pins[4],
        board_lcd_green_pins[0],
        board_lcd_green_pins[1],
        board_lcd_green_pins[2],
        board_lcd_green_pins[3],
        board_lcd_green_pins[4],
        board_lcd_green_pins[5],
        board_lcd_red_pins[0],
        board_lcd_red_pins[1],
        board_lcd_red_pins[2],
        board_lcd_red_pins[3],
        board_lcd_red_pins[4],
    };

    const esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_width = 16,
        .bits_per_pixel = 16,
        .num_fbs = 1,
        .bounce_buffer_size_px = 480 * 10,
        .disp_gpio_num = -1,
        .pclk_gpio_num = LCD_PCLK,
        .vsync_gpio_num = LCD_VSYNC,
        .hsync_gpio_num = LCD_HSYNC,
        .de_gpio_num = LCD_DE,
        .data_gpio_nums = {
            data_gpio_nums[0],  data_gpio_nums[1],  data_gpio_nums[2],  data_gpio_nums[3],
            data_gpio_nums[4],  data_gpio_nums[5],  data_gpio_nums[6],  data_gpio_nums[7],
            data_gpio_nums[8],  data_gpio_nums[9],  data_gpio_nums[10], data_gpio_nums[11],
            data_gpio_nums[12], data_gpio_nums[13], data_gpio_nums[14], data_gpio_nums[15],
        },
        .timings = {
            .pclk_hz = timing->pclk_hz,
            .h_res = timing->width,
            .v_res = timing->height,
            .hsync_pulse_width = timing->hsync_pulse_width,
            .hsync_back_porch = timing->hsync_back_porch,
            .hsync_front_porch = timing->hsync_front_porch,
            .vsync_pulse_width = timing->vsync_pulse_width,
            .vsync_back_porch = timing->vsync_back_porch,
            .vsync_front_porch = timing->vsync_front_porch,
            .flags = {
                .hsync_idle_low = timing->hsync_idle_low,
                .vsync_idle_low = timing->vsync_idle_low,
                .de_idle_high = timing->de_idle_high,
                .pclk_active_neg = !timing->pclk_active_high,
                .pclk_idle_high = timing->pclk_idle_high,
            },
        },
        .flags = {
#if CONFIG_SPIRAM
            .fb_in_psram = true,
#else
            .fb_in_psram = false,
#endif
            .double_fb = false,
        },
    };

    ESP_LOGI(TAG,
             "RGB init %ux%u pclk=%luHz fb_in_psram=%d",
             (unsigned) timing->width,
             (unsigned) timing->height,
             (unsigned long) timing->pclk_hz,
             panel_config.flags.fb_in_psram);

    ESP_RETURN_ON_ERROR(esp_lcd_new_rgb_panel(&panel_config, &s_panel_handle), TAG, "esp_lcd_new_rgb_panel failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel_handle), TAG, "esp_lcd_panel_reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel_handle), TAG, "esp_lcd_panel_init failed");
    ESP_RETURN_ON_ERROR(esp_lcd_rgb_panel_get_frame_buffer(s_panel_handle, 1, &s_framebuffer), TAG, "get framebuffer failed");

    s_stride_bytes = timing->width * sizeof(uint16_t);
    ESP_LOGI(TAG, "framebuffer=%p stride=%u bytes", s_framebuffer, (unsigned) s_stride_bytes);
    return ESP_OK;
}

void *rgb_display_get_framebuffer(void)
{
    return s_framebuffer;
}

size_t rgb_display_get_stride_bytes(void)
{
    return s_stride_bytes;
}

int rgb_display_width(void)
{
    return panel_480x480_get_timing()->width;
}

int rgb_display_height(void)
{
    return panel_480x480_get_timing()->height;
}

esp_lcd_panel_handle_t rgb_display_panel_handle(void)
{
    return s_panel_handle;
}
