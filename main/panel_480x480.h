#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

typedef struct {
    uint32_t pclk_hz;
    uint16_t width;
    uint16_t height;
    uint16_t hsync_pulse_width;
    uint16_t hsync_front_porch;
    uint16_t hsync_back_porch;
    uint16_t vsync_pulse_width;
    uint16_t vsync_front_porch;
    uint16_t vsync_back_porch;
    bool hsync_idle_low;
    bool vsync_idle_low;
    bool de_idle_high;
    bool pclk_active_high;
    bool pclk_idle_high;
} panel_timing_config_t;

esp_err_t panel_480x480_hard_reset(void);
esp_err_t panel_480x480_init(void);
esp_err_t panel_480x480_backlight_set(bool on);
const panel_timing_config_t *panel_480x480_get_timing(void);
