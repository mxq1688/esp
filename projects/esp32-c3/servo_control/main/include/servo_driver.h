/*
 * ESP32-C3 Servo Motor Control Driver
 * 
 * This header file defines the servo motor control interface
 * Supports standard servo motors (0-180 degrees) using PWM control
 */

#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Servo configuration structure */
typedef struct {
    gpio_num_t gpio_pin;           // GPIO pin connected to servo signal wire
    ledc_channel_t ledc_channel;   // LEDC channel to use (0-7)
    uint16_t min_pulse_width_us;   // Minimum pulse width in microseconds (typically 500-1000us)
    uint16_t max_pulse_width_us;   // Maximum pulse width in microseconds (typically 2000-2500us)
    uint16_t max_angle;            // Maximum angle in degrees (typically 180)
} servo_config_t;

/* Servo handle */
typedef struct {
    servo_config_t config;
    bool initialized;
    uint16_t current_angle;
} servo_handle_t;

/**
 * @brief Initialize servo motor
 * 
 * @param servo Servo handle
 * @param config Servo configuration
 * @return esp_err_t ESP_OK on success
 */
esp_err_t servo_init(servo_handle_t *servo, const servo_config_t *config);

/**
 * @brief Set servo angle
 * 
 * @param servo Servo handle
 * @param angle Target angle in degrees (0-180)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t servo_set_angle(servo_handle_t *servo, uint16_t angle);

/**
 * @brief Get current servo angle
 * 
 * @param servo Servo handle
 * @return uint16_t Current angle in degrees
 */
uint16_t servo_get_angle(servo_handle_t *servo);

/**
 * @brief Deinitialize servo motor
 * 
 * @param servo Servo handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t servo_deinit(servo_handle_t *servo);

/**
 * @brief Smooth servo movement to target angle
 * 
 * @param servo Servo handle
 * @param target_angle Target angle in degrees
 * @param step_delay_ms Delay between each step in milliseconds
 * @return esp_err_t ESP_OK on success
 */
esp_err_t servo_move_smooth(servo_handle_t *servo, uint16_t target_angle, uint32_t step_delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_DRIVER_H */
