/**
 * @file clock_display.c
 * @brief Clock display logic implementation
 */

#include "clock_display.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "clock_display";
static int animation_brightness = 0;
static int animation_direction = 5;

esp_err_t clock_display_init(void)
{
    ESP_LOGI(TAG, "Initializing clock display");
    return neopixel_init();
}

esp_err_t clock_display_update(int hours, int minutes, int seconds)
{
    esp_err_t ret;

    // Convert to 12-hour format
    hours = hours % 12;

    // Calculate LED positions
    int hour_led = (hours * 5 + minutes / 12 + LED_OFFSET) % 60;
    int minute_led = (minutes + LED_OFFSET) % 60;
    int second_led = (seconds + LED_OFFSET) % 60;

    // 定义颜色
    rgb_color_t black = {0, 0, 0};
    rgb_color_t dim_hour_color = {
        .r = DIM_HOUR_COLOR_R,
        .g = DIM_HOUR_COLOR_G,
        .b = DIM_HOUR_COLOR_B
    };
    rgb_color_t second_color = {
        .r = SECOND_COLOR_R,
        .g = SECOND_COLOR_G,
        .b = SECOND_COLOR_B
    };
    rgb_color_t minute_color = {
        .r = MINUTE_COLOR_R,
        .g = MINUTE_COLOR_G,
        .b = MINUTE_COLOR_B
    };
    rgb_color_t hour_color = {
        .r = HOUR_COLOR_R,
        .g = HOUR_COLOR_G,
        .b = HOUR_COLOR_B
    };

    // 一次性设置所有像素，避免使用 clear
    // 先将所有像素设为黑色，再设置时钟指针
    for (int i = 0; i < 60; i++) {
        // 判断当前像素应该是什么颜色
        if (i == hour_led) {
            neopixel_set_pixel(i, hour_color);
        } else if (i == minute_led) {
            neopixel_set_pixel(i, minute_color);
        } else if (i == second_led) {
            neopixel_set_pixel(i, second_color);
        } else if (i == (hour_led + 1) % 60 || i == (hour_led + 59) % 60) {
            neopixel_set_pixel(i, dim_hour_color);
        } else {
            neopixel_set_pixel(i, black);
        }
    }

    // Refresh the display
    ret = neopixel_refresh();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to refresh display: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t clock_display_connecting_animation(void)
{
    esp_err_t ret;

    // Update brightness
    animation_brightness += animation_direction;
    if (animation_brightness <= 0 || animation_brightness >= 255) {
        animation_direction = -animation_direction;
        animation_brightness += animation_direction; // Correct overshoot
    }

    // Clear all LEDs
    ret = neopixel_clear();
    if (ret != ESP_OK) {
        return ret;
    }

    // Set blue pulsing LEDs at 12, 3, 6, and 9 o'clock positions
    rgb_color_t blue_color = {
        .r = 0,
        .g = 0,
        .b = (uint8_t)animation_brightness
    };

    neopixel_set_pixel((0 + LED_OFFSET) % 60, blue_color);   // 12 o'clock
    neopixel_set_pixel((15 + LED_OFFSET) % 60, blue_color);  // 3 o'clock
    neopixel_set_pixel((30 + LED_OFFSET) % 60, blue_color);  // 6 o'clock
    neopixel_set_pixel((45 + LED_OFFSET) % 60, blue_color);  // 9 o'clock

    // Refresh the display
    ret = neopixel_refresh();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to refresh display: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t clock_display_error(void)
{
    esp_err_t ret;

    // Clear all LEDs
    ret = neopixel_clear();
    if (ret != ESP_OK) {
        return ret;
    }

    // Set red LED at 6 o'clock position
    rgb_color_t red_color = {
        .r = 255,
        .g = 0,
        .b = 0
    };
    neopixel_set_pixel((30 + LED_OFFSET) % 60, red_color);

    // Refresh the display
    ret = neopixel_refresh();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to refresh display: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

