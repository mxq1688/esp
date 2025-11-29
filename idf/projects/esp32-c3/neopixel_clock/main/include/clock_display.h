/**
 * @file clock_display.h
 * @brief Clock display logic for NeoPixel LED ring
 */

#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

#include "esp_err.h"
#include "neopixel_driver.h"

// Clock face configuration
#define LED_OFFSET 27  // LED number at 12 o'clock position (start with 0)

// Color definitions for clock hands
#define HOUR_COLOR_R    128
#define HOUR_COLOR_G    50
#define HOUR_COLOR_B    35

#define DIM_HOUR_COLOR_R    8
#define DIM_HOUR_COLOR_G    4
#define DIM_HOUR_COLOR_B    2

#define MINUTE_COLOR_R  192
#define MINUTE_COLOR_G  164
#define MINUTE_COLOR_B  164

#define SECOND_COLOR_R  16
#define SECOND_COLOR_G  16
#define SECOND_COLOR_B  64

/**
 * @brief Initialize clock display
 * 
 * @return ESP_OK on success
 */
esp_err_t clock_display_init(void);

/**
 * @brief Update clock display with current time
 * 
 * @param hours Current hour (0-23)
 * @param minutes Current minute (0-59)
 * @param seconds Current second (0-59)
 * @return ESP_OK on success
 */
esp_err_t clock_display_update(int hours, int minutes, int seconds);

/**
 * @brief Show connecting animation (pulsing blue)
 * 
 * @return ESP_OK on success
 */
esp_err_t clock_display_connecting_animation(void);

/**
 * @brief Show error state (red at 6 o'clock)
 * 
 * @return ESP_OK on success
 */
esp_err_t clock_display_error(void);

#endif // CLOCK_DISPLAY_H

