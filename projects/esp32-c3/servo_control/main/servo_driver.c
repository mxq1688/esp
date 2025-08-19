/*
 * ESP32-C3 Servo Motor Control Driver Implementation
 * 
 * This file implements servo motor control using LEDC PWM
 * Standard servo motors require 50Hz PWM with 1-2ms pulse width
 */

#include "include/servo_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "SERVO_DRIVER";

/* Default servo parameters */
#define SERVO_PWM_FREQUENCY     50      // 50Hz for standard servos
#define SERVO_PWM_RESOLUTION    LEDC_TIMER_14_BIT  // ESP32-C3 supports up to 14-bit resolution
#define SERVO_PWM_TIMER         LEDC_TIMER_0
#define SERVO_PWM_MODE          LEDC_LOW_SPEED_MODE

/* Calculate duty cycle from pulse width */
static uint32_t pulse_width_to_duty(uint16_t pulse_width_us, uint32_t max_duty)
{
    // PWM period is 20ms (20000us) for 50Hz
    uint32_t period_us = 1000000 / SERVO_PWM_FREQUENCY;
    return (uint32_t)(((uint64_t)pulse_width_us * max_duty) / period_us);
}

esp_err_t servo_init(servo_handle_t *servo, const servo_config_t *config)
{
    if (!servo || !config) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }

    // Copy configuration
    memcpy(&servo->config, config, sizeof(servo_config_t));
    servo->initialized = false;
    servo->current_angle = 0;

    ESP_LOGI(TAG, "Initializing servo on GPIO%d, channel %d", config->gpio_pin, config->ledc_channel);

    // Configure LEDC timer
    ledc_timer_config_t timer_config = {
        .speed_mode       = SERVO_PWM_MODE,
        .timer_num        = SERVO_PWM_TIMER,
        .duty_resolution  = SERVO_PWM_RESOLUTION,
        .freq_hz          = SERVO_PWM_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    
    esp_err_t ret = ledc_timer_config(&timer_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC timer: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure LEDC channel
    ledc_channel_config_t channel_config = {
        .speed_mode     = SERVO_PWM_MODE,
        .channel        = config->ledc_channel,
        .timer_sel      = SERVO_PWM_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = config->gpio_pin,
        .duty           = 0,
        .hpoint         = 0
    };
    
    ret = ledc_channel_config(&channel_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(ret));
        return ret;
    }

    servo->initialized = true;
    ESP_LOGI(TAG, "Servo initialized successfully");
    
    // Set to initial position (0 degrees)
    return servo_set_angle(servo, 0);
}

esp_err_t servo_set_angle(servo_handle_t *servo, uint16_t angle)
{
    if (!servo || !servo->initialized) {
        ESP_LOGE(TAG, "Servo not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // For continuous rotation servos, allow angles beyond max_angle for speed control
    if (angle > servo->config.max_angle && servo->config.max_angle <= 180) {
        ESP_LOGE(TAG, "Angle %d exceeds maximum %d", angle, servo->config.max_angle);
        return ESP_ERR_INVALID_ARG;
    }
    
    // For 360-degree servos, normalize angle to 0-360 range
    if (servo->config.max_angle > 180) {
        angle = angle % (servo->config.max_angle + 1);
    }

    // Calculate pulse width based on angle
    uint16_t pulse_width_range = servo->config.max_pulse_width_us - servo->config.min_pulse_width_us;
    uint16_t pulse_width_us = servo->config.min_pulse_width_us + 
                             (pulse_width_range * angle) / servo->config.max_angle;

    // Calculate duty cycle
    uint32_t max_duty = (1 << SERVO_PWM_RESOLUTION) - 1;
    uint32_t duty = pulse_width_to_duty(pulse_width_us, max_duty);

    ESP_LOGD(TAG, "Setting angle %d°: pulse_width=%dus, duty=%lu", angle, pulse_width_us, duty);

    // Set PWM duty cycle
    esp_err_t ret = ledc_set_duty(SERVO_PWM_MODE, servo->config.ledc_channel, duty);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set duty: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = ledc_update_duty(SERVO_PWM_MODE, servo->config.ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update duty: %s", esp_err_to_name(ret));
        return ret;
    }

    servo->current_angle = angle;
    ESP_LOGI(TAG, "Servo set to %d degrees", angle);
    return ESP_OK;
}

uint16_t servo_get_angle(servo_handle_t *servo)
{
    if (!servo || !servo->initialized) {
        return 0;
    }
    return servo->current_angle;
}

esp_err_t servo_deinit(servo_handle_t *servo)
{
    if (!servo || !servo->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Deinitializing servo");
    
    // Stop the PWM signal
    esp_err_t ret = ledc_stop(SERVO_PWM_MODE, servo->config.ledc_channel, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop LEDC: %s", esp_err_to_name(ret));
    }

    servo->initialized = false;
    servo->current_angle = 0;
    
    return ret;
}

esp_err_t servo_move_smooth(servo_handle_t *servo, uint16_t target_angle, uint32_t step_delay_ms)
{
    if (!servo || !servo->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (target_angle > servo->config.max_angle) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t current = servo->current_angle;
    ESP_LOGI(TAG, "Smooth movement from %d° to %d°", current, target_angle);

    if (current == target_angle) {
        return ESP_OK;
    }

    int step = (target_angle > current) ? 1 : -1;
    
    while (current != target_angle) {
        current += step;
        esp_err_t ret = servo_set_angle(servo, current);
        if (ret != ESP_OK) {
            return ret;
        }
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
    }

    ESP_LOGI(TAG, "Smooth movement completed");
    return ESP_OK;
}
