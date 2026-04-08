#pragma once

#include <stddef.h>

#include "esp_err.h"
#include "esp_lcd_panel_ops.h"

esp_err_t rgb_display_init(void);
void *rgb_display_get_framebuffer(void);
size_t rgb_display_get_stride_bytes(void);
int rgb_display_width(void);
int rgb_display_height(void);
esp_lcd_panel_handle_t rgb_display_panel_handle(void);
