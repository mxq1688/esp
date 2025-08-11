/*
 * ESP32-C3-DevKitM-1 LED Blink Example
 * 
 * This example demonstrates how to blink the onboard LED on ESP32-C3-DevKitM-1
 * The onboard LED is connected to GPIO8
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "LED_BLINK";

/* ESP32-C3-DevKitM-1 onboard LED is connected to GPIO8 */
#define BLINK_GPIO 8

/* LED strip configuration for WS2812 RGB LED */
#define LED_STRIP_BLINK_GPIO 8
#define LED_STRIP_LED_NUMBERS 1

static led_strip_handle_t led_strip;

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
    bool led_on_off = false;
    
    ESP_LOGI(TAG, "Starting LED Blink example for ESP32-C3-DevKitM-1");
    
    /* Configure the peripheral according to the LED type */
    configure_led();
    
    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", led_on_off == true ? "ON" : "OFF");
        
        if (led_on_off) {
            /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
            led_strip_set_pixel(led_strip, 0, 16, 16, 16);
            /* Refresh the strip to send data */
            led_strip_refresh(led_strip);
        } else {
            /* Set all LED off to clear all pixels */
            led_strip_clear(led_strip);
        }
        
        led_on_off = !led_on_off;
        
        /* Configure the blink period */
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}