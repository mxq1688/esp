/*
 * ESP32-C3 Servo Motor Control Example
 * 
 * This example demonstrates how to control a servo motor using PWM
 * The servo will perform various movements including:
 * - Basic angle positioning
 * - Smooth sweeping motion
 * - Step-by-step movement
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_adc/adc_oneshot.h"
#include "include/servo_driver.h"
#include "include/joystick_led_controller.h"
#include "include/bluetooth_controller.h"

static const char *TAG = "SERVO_CONTROL";

/* Servo configuration */
#define SERVO_GPIO_PIN          GPIO_NUM_2     // GPIO pin for servo signal (you can change this)
#define SERVO_LEDC_CHANNEL      LEDC_CHANNEL_0 // LEDC channel
#define SERVO_MIN_PULSE_WIDTH   500            // 0.5ms for minimum position/speed
#define SERVO_MAX_PULSE_WIDTH   2500           // 2.5ms for maximum position/speed
#define SERVO_MAX_ANGLE         360            // Support 360 degrees continuous rotation
#define SERVO_STOP_PULSE_WIDTH  1500           // 1.5ms for stop (continuous rotation mode)

/* Joystick and LED configuration */
#define JOYSTICK_X_AXIS_CHANNEL ADC_CHANNEL_0  // GPIO0 - ADC1_CH0
#define JOYSTICK_Y_AXIS_CHANNEL ADC_CHANNEL_1  // GPIO1 - ADC1_CH1
#define JOYSTICK_BUTTON_PIN     GPIO_NUM_3     // GPIO3 for joystick button
#define LED_GPIO_PIN            GPIO_NUM_8     // GPIO8 for WS2812 RGB LED
#define LED_COUNT               1              // Number of LEDs

/* Demo parameters */
#define DEMO_STEP_DELAY_MS      50             // Delay for smooth movement
#define DEMO_PAUSE_MS           2000           // Pause between demo sequences
#define CONTINUOUS_ROTATION_ENABLED  true     // Enable continuous rotation mode
#define JOYSTICK_CONTROL_ENABLED     true     // Enable joystick control mode

static servo_handle_t servo_motor;
static joystick_led_handle_t joystick_led_controller;

// 蓝牙控制回调函数
static void bluetooth_angle_callback(uint16_t angle)
{
    ESP_LOGI(TAG, "Bluetooth: Set servo angle to %d°", angle);
    servo_set_angle(&servo_motor, angle);
}

static void bluetooth_led_callback(bool state)
{
    ESP_LOGI(TAG, "Bluetooth: Set LED state to %s", state ? "ON" : "OFF");
    if (state) {
        led_on(&joystick_led_controller);
    } else {
        led_off(&joystick_led_controller);
    }
}

/* Demo sequence 1: Basic positioning (0-360 degrees) */
static void demo_basic_positioning(void)
{
    ESP_LOGI(TAG, "=== Demo 1: Extended Range Positioning ===");
    
    if (CONTINUOUS_ROTATION_ENABLED) {
        ESP_LOGI(TAG, "Testing full 360-degree range");
        uint16_t angles[] = {0, 60, 120, 180, 240, 300, 360, 180, 0};
        int num_angles = sizeof(angles) / sizeof(angles[0]);
        
        for (int i = 0; i < num_angles; i++) {
            ESP_LOGI(TAG, "Moving to %d degrees", angles[i]);
            esp_err_t ret = servo_set_angle(&servo_motor, angles[i]);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set angle: %s", esp_err_to_name(ret));
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second
        }
    } else {
        ESP_LOGI(TAG, "Standard 180-degree positioning");
        uint16_t angles[] = {0, 45, 90, 135, 180, 90, 0};
        int num_angles = sizeof(angles) / sizeof(angles[0]);
        
        for (int i = 0; i < num_angles; i++) {
            ESP_LOGI(TAG, "Moving to %d degrees", angles[i]);
            esp_err_t ret = servo_set_angle(&servo_motor, angles[i]);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set angle: %s", esp_err_to_name(ret));
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second
        }
    }
}

/* Demo sequence: Continuous rotation speed control */
static void demo_continuous_rotation(void)
{
    if (!CONTINUOUS_ROTATION_ENABLED) {
        ESP_LOGI(TAG, "=== Continuous rotation disabled ===");
        return;
    }
    
    ESP_LOGI(TAG, "=== Demo: Continuous Rotation Control ===");
    
    // Clockwise rotation at different speeds
    ESP_LOGI(TAG, "Clockwise rotation - Slow");
    servo_set_angle(&servo_motor, 200); // Slow clockwise
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Clockwise rotation - Medium");
    servo_set_angle(&servo_motor, 250); // Medium clockwise
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Clockwise rotation - Fast");
    servo_set_angle(&servo_motor, 300); // Fast clockwise
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Stop
    ESP_LOGI(TAG, "Stop");
    servo_set_angle(&servo_motor, 180); // Stop position
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Counter-clockwise rotation
    ESP_LOGI(TAG, "Counter-clockwise rotation - Slow");
    servo_set_angle(&servo_motor, 160); // Slow counter-clockwise
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Counter-clockwise rotation - Medium");
    servo_set_angle(&servo_motor, 130); // Medium counter-clockwise
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Counter-clockwise rotation - Fast");
    servo_set_angle(&servo_motor, 80); // Fast counter-clockwise
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Stop
    ESP_LOGI(TAG, "Final stop");
    servo_set_angle(&servo_motor, 180); // Stop position
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/* Demo sequence 2: Smooth sweeping */
static void demo_smooth_sweep(void)
{
    ESP_LOGI(TAG, "=== Demo 2: Smooth Sweeping ===");
    
    // Sweep from 0 to 180 degrees smoothly
    ESP_LOGI(TAG, "Sweeping from 0° to 180°");
    esp_err_t ret = servo_move_smooth(&servo_motor, 180, DEMO_STEP_DELAY_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed smooth movement: %s", esp_err_to_name(ret));
        return;
    }
    
    vTaskDelay(pdMS_TO_TICKS(DEMO_PAUSE_MS));
    
    // Sweep back from 180 to 0 degrees
    ESP_LOGI(TAG, "Sweeping from 180° to 0°");
    ret = servo_move_smooth(&servo_motor, 0, DEMO_STEP_DELAY_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed smooth movement: %s", esp_err_to_name(ret));
        return;
    }
}

/* Demo sequence 3: Oscillation */
static void demo_oscillation(void)
{
    ESP_LOGI(TAG, "=== Demo 3: Oscillation ===");
    
    for (int cycle = 0; cycle < 3; cycle++) {
        ESP_LOGI(TAG, "Oscillation cycle %d/3", cycle + 1);
        
        // Quick movements between 30° and 150°
        servo_set_angle(&servo_motor, 30);
        vTaskDelay(pdMS_TO_TICKS(300));
        
        servo_set_angle(&servo_motor, 150);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    
    // Return to center
    servo_set_angle(&servo_motor, 90);
}

/* Demo sequence 4: Step-by-step movement */
static void demo_step_movement(void)
{
    ESP_LOGI(TAG, "=== Demo 4: Step Movement ===");
    
    // Move in 30-degree steps
    for (int angle = 0; angle <= 180; angle += 30) {
        ESP_LOGI(TAG, "Step to %d degrees", angle);
        servo_set_angle(&servo_motor, angle);
        vTaskDelay(pdMS_TO_TICKS(800));
    }
    
    // Move back in 45-degree steps
    for (int angle = 180; angle >= 0; angle -= 45) {
        ESP_LOGI(TAG, "Step to %d degrees", angle);
        servo_set_angle(&servo_motor, angle);
        vTaskDelay(pdMS_TO_TICKS(600));
    }
}

/* Demo sequence: Joystick control mode */
static void demo_joystick_control(void)
{
    if (!JOYSTICK_CONTROL_ENABLED) {
        ESP_LOGI(TAG, "=== Joystick control disabled ===");
        return;
    }
    
    ESP_LOGI(TAG, "=== Demo: Joystick Control Mode ===");
    ESP_LOGI(TAG, "Use joystick to control servo, press button to toggle LED");
    ESP_LOGI(TAG, "Running for 30 seconds...");
    
    TickType_t start_time = xTaskGetTickCount();
    TickType_t duration = pdMS_TO_TICKS(30000); // 30 seconds
    
    bool last_button_state = false;
    int hue_counter = 0;
    
    while ((xTaskGetTickCount() - start_time) < duration) {
        joystick_data_t joystick_data;
        esp_err_t ret = joystick_read(&joystick_led_controller, &joystick_data);
        
        if (ret == ESP_OK) {
            // 使用X轴控制舵机角度 (映射到0-360度)
            // 摇杆值范围: -100 到 +100
            // 舵机角度范围: 0 到 360
            uint16_t servo_angle = (joystick_data.x_value + 100) * 360 / 200;
            if (servo_angle > 360) servo_angle = 360;
            
            // 设置舵机角度
            servo_set_angle(&servo_motor, servo_angle);
            
            // 检测按钮按下事件 (边沿检测)
            if (joystick_data.button_pressed && !last_button_state) {
                ESP_LOGI(TAG, "Button pressed - Toggling LED");
                led_toggle(&joystick_led_controller);
            }
            last_button_state = joystick_data.button_pressed;
            
            // 使用Y轴控制LED颜色
            if (joystick_led_controller.led_state) {
                hue_counter = (joystick_data.y_value + 100) * 360 / 200;
                if (hue_counter > 360) hue_counter = 360;
                led_set_hsv(&joystick_led_controller, hue_counter, 255, 100);
            }
            
            // 日志输出 (每500ms输出一次)
            static TickType_t last_log_time = 0;
            TickType_t current_time = xTaskGetTickCount();
            if ((current_time - last_log_time) > pdMS_TO_TICKS(500)) {
                ESP_LOGI(TAG, "X:%d Y:%d Btn:%d | Servo:%d° LED:%s Hue:%d", 
                        joystick_data.x_value, joystick_data.y_value, 
                        joystick_data.button_pressed, servo_angle,
                        joystick_led_controller.led_state ? "ON" : "OFF", hue_counter);
                last_log_time = current_time;
            }
        } else {
            // 如果摇杆读取失败，至少测试按钮功能
            bool button_pressed = !gpio_get_level(JOYSTICK_BUTTON_PIN);
            if (button_pressed && !last_button_state) {
                ESP_LOGI(TAG, "Button pressed (direct GPIO) - Toggling LED");
                led_toggle(&joystick_led_controller);
            }
            last_button_state = button_pressed;
            
            // 简单日志输出
            static TickType_t last_log_time = 0;
            TickType_t current_time = xTaskGetTickCount();
            if ((current_time - last_log_time) > pdMS_TO_TICKS(1000)) {
                ESP_LOGI(TAG, "Joystick read failed, but button test: %s", 
                        button_pressed ? "Pressed" : "Released");
                last_log_time = current_time;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz更新频率
    }
    
    ESP_LOGI(TAG, "Joystick control demo completed");
    
    // 恢复默认状态
    servo_set_angle(&servo_motor, 90);
    led_set_state(&joystick_led_controller, false);
}

/* Initialize servo motor */
static esp_err_t init_servo(void)
{
    ESP_LOGI(TAG, "Initializing servo motor");
    
    servo_config_t config = {
        .gpio_pin = SERVO_GPIO_PIN,
        .ledc_channel = SERVO_LEDC_CHANNEL,
        .min_pulse_width_us = SERVO_MIN_PULSE_WIDTH,
        .max_pulse_width_us = SERVO_MAX_PULSE_WIDTH,
        .max_angle = SERVO_MAX_ANGLE
    };
    
    esp_err_t ret = servo_init(&servo_motor, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize servo: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Servo motor initialized successfully");
    ESP_LOGI(TAG, "Configuration:");
    ESP_LOGI(TAG, "  GPIO Pin: %d", config.gpio_pin);
    ESP_LOGI(TAG, "  LEDC Channel: %d", config.ledc_channel);
    ESP_LOGI(TAG, "  Pulse Width: %d-%d μs", config.min_pulse_width_us, config.max_pulse_width_us);
    ESP_LOGI(TAG, "  Max Angle: %d degrees", config.max_angle);
    
    return ESP_OK;
}

/* Initialize joystick and LED controller */
static esp_err_t init_joystick_led(void)
{
    ESP_LOGI(TAG, "Initializing joystick and LED controller");
    
    joystick_led_config_t config = {
        .x_axis_channel = JOYSTICK_X_AXIS_CHANNEL,
        .y_axis_channel = JOYSTICK_Y_AXIS_CHANNEL,
        .button_pin = JOYSTICK_BUTTON_PIN,
        .led_pin = LED_GPIO_PIN,
        .led_count = LED_COUNT,
        .x_center = 2048,   // Will be calibrated automatically
        .y_center = 2048,   // Will be calibrated automatically
        .deadzone = 100     // ADC deadzone
    };
    
    esp_err_t ret = joystick_led_init(&joystick_led_controller, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize joystick and LED: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Joystick and LED controller initialized successfully");
    ESP_LOGI(TAG, "Hardware configuration:");
    ESP_LOGI(TAG, "  Joystick X-axis: GPIO0 (ADC1_CH0)");
    ESP_LOGI(TAG, "  Joystick Y-axis: GPIO1 (ADC1_CH1)");
    ESP_LOGI(TAG, "  Joystick Button: GPIO3");
    ESP_LOGI(TAG, "  LED Strip: GPIO8 (WS2812 RGB)");
    
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP32-C3 Servo Control with Joystick, LED and Bluetooth ===");
    ESP_LOGI(TAG, "Hardware Configuration:");
    ESP_LOGI(TAG, "  Servo: GPIO%d (LEDC Channel %d)", SERVO_GPIO_PIN, SERVO_LEDC_CHANNEL);
    ESP_LOGI(TAG, "  Joystick X: ADC1_CH%d (GPIO%d)", JOYSTICK_X_AXIS_CHANNEL, 0);
    ESP_LOGI(TAG, "  Joystick Y: ADC1_CH%d (GPIO%d)", JOYSTICK_Y_AXIS_CHANNEL, 1);
    ESP_LOGI(TAG, "  Joystick Button: GPIO%d", JOYSTICK_BUTTON_PIN);
    ESP_LOGI(TAG, "  LED: GPIO%d (WS2812)", LED_GPIO_PIN);
    ESP_LOGI(TAG, "  Bluetooth: BLE enabled");
    
    // 初始化舵机
    esp_err_t ret = init_servo();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize servo, stopping");
        return;
    }
    
    // 初始化摇杆和LED控制器
    ret = init_joystick_led();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize joystick/LED, stopping");
        return;
    }
    
    ESP_LOGI(TAG, "All components initialized successfully");
    ESP_LOGI(TAG, "Starting joystick control mode...");
    
    // 等待系统稳定
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 舵机初始位置 - 中心位置
    servo_set_angle(&servo_motor, 90); // 设置为中心位置
    ESP_LOGI(TAG, "Servo set to center position (90°)");
    
    // 主循环 - 摇杆控制模式
    ESP_LOGI(TAG, "=== Joystick Control Mode ===");
    ESP_LOGI(TAG, "Use joystick X-axis to control servo rotation");
    ESP_LOGI(TAG, "Press joystick button to toggle LED");
    ESP_LOGI(TAG, "Use joystick Y-axis to change LED color (when LED is ON)");
    
    bool last_button_state = false;
    int hue_counter = 0;
    
    while (1) {
        joystick_data_t joystick_data;
        esp_err_t ret = joystick_read(&joystick_led_controller, &joystick_data);
        
        if (ret == ESP_OK) {
            // 摇杆X轴和Y轴都可以控制舵机，覆盖0-180度范围
            uint16_t servo_angle;
            
            // 选择变化较大的轴来控制
            if (abs(joystick_data.x_value) > abs(joystick_data.y_value)) {
                // X轴变化大，使用X轴控制
                servo_angle = (joystick_data.x_value + 100) * 180 / 200;
            } else {
                // Y轴变化大，使用Y轴控制
                servo_angle = (joystick_data.y_value + 100) * 180 / 200;
            }
            
            // 限制角度范围
            if (servo_angle > 180) servo_angle = 180;
            if (servo_angle < 0) servo_angle = 0;
            
            // 设置舵机角度
            servo_set_angle(&servo_motor, servo_angle);
            
            // 检测按钮按下事件 (边沿检测)
            if (joystick_data.button_pressed && !last_button_state) {
                ESP_LOGI(TAG, "Button pressed - Toggling LED");
                led_toggle(&joystick_led_controller);
            }
            last_button_state = joystick_data.button_pressed;
            
            // LED亮度跟随舵机角度变化
            if (joystick_led_controller.led_state) {
                // 使用Y轴控制颜色
                hue_counter = (joystick_data.y_value + 100) * 360 / 200;
                if (hue_counter > 360) hue_counter = 360;
                
                // 亮度跟随舵机角度：0度=最暗，180度=最亮
                uint8_t brightness = (servo_angle * 255) / 180;
                if (brightness > 255) brightness = 255;
                
                led_set_hsv(&joystick_led_controller, hue_counter, 255, brightness);
            }
            
            // 日志输出 (每1000ms输出一次)
            static TickType_t last_log_time = 0;
            TickType_t current_time = xTaskGetTickCount();
            if ((current_time - last_log_time) > pdMS_TO_TICKS(1000)) {
                uint8_t brightness = (servo_angle * 255) / 180;
                ESP_LOGI(TAG, "X:%d Y:%d Btn:%d | Servo:%d° LED:%s Hue:%d Bright:%d", 
                        joystick_data.x_value, joystick_data.y_value, 
                        joystick_data.button_pressed, servo_angle,
                        joystick_led_controller.led_state ? "ON" : "OFF", hue_counter, brightness);
                last_log_time = current_time;
            }
        } else {
            // 如果摇杆读取失败，至少测试按钮功能
            bool button_pressed = !gpio_get_level(JOYSTICK_BUTTON_PIN);
            if (button_pressed && !last_button_state) {
                ESP_LOGI(TAG, "Button pressed (direct GPIO) - Toggling LED");
                led_toggle(&joystick_led_controller);
            }
            last_button_state = button_pressed;
            
            // 简单日志输出
            static TickType_t last_log_time = 0;
            TickType_t current_time = xTaskGetTickCount();
            if ((current_time - last_log_time) > pdMS_TO_TICKS(2000)) {
                ESP_LOGI(TAG, "Joystick read failed, but button test: %s", 
                        button_pressed ? "Pressed" : "Released");
                last_log_time = current_time;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz更新频率
    }
}
