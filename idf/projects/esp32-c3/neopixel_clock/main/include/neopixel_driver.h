/**
 * @file neopixel_driver.h
 * @brief NeoPixel LED strip driver using RMT peripheral
 */

#ifndef NEOPIXEL_DRIVER_H
#define NEOPIXEL_DRIVER_H

#include <stdint.h>
#include "esp_err.h"

// LED configuration
#define LED_STRIP_GPIO      10      // GPIO pin for LED data
#define LED_STRIP_NUM_LEDS  60      // Number of LEDs in the strip
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)  // 10MHz resolution

// Color structure
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

/**
 * @brief Initialize the NeoPixel LED strip
 * 
 * @return ESP_OK on success
 */
esp_err_t neopixel_init(void);

/**
 * @brief Set a single LED color
 * 
 * @param index LED index (0 to NUM_LEDS-1)
 * @param color RGB color
 * @return ESP_OK on success
 */
esp_err_t neopixel_set_pixel(uint32_t index, rgb_color_t color);

/**
 * @brief Get a single LED color
 * 
 * @param index LED index (0 to NUM_LEDS-1)
 * @param color Pointer to store RGB color
 * @return ESP_OK on success
 */
esp_err_t neopixel_get_pixel(uint32_t index, rgb_color_t *color);

/**
 * @brief Clear all LEDs (set to black)
 * 
 * @return ESP_OK on success
 */
esp_err_t neopixel_clear(void);

/**
 * @brief Refresh the LED strip to show the current buffer
 * 
 * @return ESP_OK on success
 */
esp_err_t neopixel_refresh(void);

/**
 * @brief Deinitialize the NeoPixel LED strip
 * 
 * @return ESP_OK on success
 */
esp_err_t neopixel_deinit(void);

#endif // NEOPIXEL_DRIVER_H

