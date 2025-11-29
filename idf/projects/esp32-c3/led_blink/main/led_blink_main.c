/*
 * ESP32-C3-DevKitM-1 LED Rainbow Effect Example
 * 
 * This example demonstrates rainbow color transitions with brightness control
 * The LED will cycle through different colors with varying brightness
 * The onboard LED is connected to GPIO8
 */

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "LED_RAINBOW";

/* ESP32-C3-DevKitM-1 onboard LED is connected to GPIO8 */
#define BLINK_GPIO 8

/* LED strip configuration for WS2812 RGB LED */
#define LED_STRIP_BLINK_GPIO 8
#define LED_STRIP_LED_NUMBERS 1

/* Rainbow effect parameters */
#define MAX_BRIGHTNESS 100  // Reduced for better color visibility
#define MIN_BRIGHTNESS 10   // Keep some minimum brightness
#define BRIGHTNESS_STEP 2
#define COLOR_STEP 2
#define FADE_DELAY_MS 30    // Faster transitions

static led_strip_handle_t led_strip;

/* Function to convert HSV to RGB */
static void hsv_to_rgb(int hue, int saturation, int value, uint8_t *red, uint8_t *green, uint8_t *blue)
{
    int region, remainder, p, q, t;

    if (saturation == 0) {
        *red = *green = *blue = value;
        return;
    }

    region = hue / 43;
    remainder = (hue - (region * 43)) * 6;

    p = (value * (255 - saturation)) >> 8;
    q = (value * (255 - ((saturation * remainder) >> 8))) >> 8;
    t = (value * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            *red = value; *green = t; *blue = p;
            break;
        case 1:
            *red = q; *green = value; *blue = p;
            break;
        case 2:
            *red = p; *green = value; *blue = t;
            break;
        case 3:
            *red = p; *green = q; *blue = value;
            break;
        case 4:
            *red = t; *green = p; *blue = value;
            break;
        default:
            *red = value; *green = p; *blue = q;
            break;
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Configuring WS2812 RGB LED on GPIO%d", LED_STRIP_BLINK_GPIO);
    
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_BLINK_GPIO,
        .max_leds = LED_STRIP_LED_NUMBERS, // at least one LED on board
    };
    
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void app_main(void)
{
    int brightness = MIN_BRIGHTNESS;
    int brightness_direction = 1; // 1 for increasing, -1 for decreasing
    int hue = 0;  // Color hue (0-255)
    uint8_t red, green, blue;
    
    ESP_LOGI(TAG, "Starting LED Rainbow Effect for ESP32-C3-DevKitM-1");
    ESP_LOGI(TAG, "LED will cycle through rainbow colors with brightness changes");
    
    /* Configure the peripheral according to the LED type */
    configure_led();
    
    while (1) {
        /* Convert HSV to RGB */
        hsv_to_rgb(hue, 255, brightness, &red, &green, &blue);
        
        /* Set the LED pixel with current color and brightness */
        led_strip_set_pixel(led_strip, 0, red, green, blue);
        led_strip_refresh(led_strip);
        
        /* Log color and brightness info periodically */
        static int log_counter = 0;
        if (log_counter % 50 == 0) {
            ESP_LOGI(TAG, "Hue: %d, Brightness: %d, RGB: (%d,%d,%d)", 
                     hue, brightness, red, green, blue);
        }
        log_counter++;
        
        /* Update brightness */
        brightness += brightness_direction * BRIGHTNESS_STEP;
        
        /* Change brightness direction when reaching limits */
        if (brightness >= MAX_BRIGHTNESS) {
            brightness = MAX_BRIGHTNESS;
            brightness_direction = -1;
        } else if (brightness <= MIN_BRIGHTNESS) {
            brightness = MIN_BRIGHTNESS;
            brightness_direction = 1;
        }
        
        /* Update color hue continuously */
        hue += COLOR_STEP;
        if (hue >= 256) {
            hue = 0;
            ESP_LOGI(TAG, "Completed one full rainbow cycle!");
        }
        
        /* Small delay for smooth transitions */
        vTaskDelay(FADE_DELAY_MS / portTICK_PERIOD_MS);
    }
}