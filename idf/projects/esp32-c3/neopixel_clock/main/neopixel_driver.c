/**
 * @file neopixel_driver.c
 * @brief NeoPixel LED strip driver implementation using RMT peripheral
 */

#include "neopixel_driver.h"
#include "led_strip.h"
#include "esp_log.h"

static const char *TAG = "neopixel";
static led_strip_handle_t led_strip = NULL;

esp_err_t neopixel_init(void)
{
    ESP_LOGI(TAG, "Initializing NeoPixel LED strip on GPIO %d with %d LEDs", 
             LED_STRIP_GPIO, LED_STRIP_NUM_LEDS);

    // LED strip general initialization
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_STRIP_NUM_LEDS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    // LED strip RMT configuration
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
        .flags.with_dma = false,
    };

    // Create LED strip object
    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip: %s", esp_err_to_name(ret));
        return ret;
    }

    // Clear all LEDs
    ret = neopixel_clear();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear LED strip: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "NeoPixel initialized successfully");
    return ESP_OK;
}

esp_err_t neopixel_set_pixel(uint32_t index, rgb_color_t color)
{
    if (led_strip == NULL) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (index >= LED_STRIP_NUM_LEDS) {
        ESP_LOGE(TAG, "LED index %lu out of range (max: %d)", index, LED_STRIP_NUM_LEDS - 1);
        return ESP_ERR_INVALID_ARG;
    }

    return led_strip_set_pixel(led_strip, index, color.r, color.g, color.b);
}

esp_err_t neopixel_clear(void)
{
    if (led_strip == NULL) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = led_strip_clear(led_strip);
    if (ret != ESP_OK) {
        return ret;
    }

    return led_strip_refresh(led_strip);
}

esp_err_t neopixel_refresh(void)
{
    if (led_strip == NULL) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    return led_strip_refresh(led_strip);
}

esp_err_t neopixel_deinit(void)
{
    if (led_strip == NULL) {
        ESP_LOGW(TAG, "LED strip already deinitialized");
        return ESP_OK;
    }

    esp_err_t ret = led_strip_del(led_strip);
    if (ret == ESP_OK) {
        led_strip = NULL;
        ESP_LOGI(TAG, "NeoPixel deinitialized");
    }

    return ret;
}

