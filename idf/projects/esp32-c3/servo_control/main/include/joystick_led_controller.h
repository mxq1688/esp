/*
 * ESP32-C3 摇杆和LED控制器
 * 
 * 该头文件定义了摇杆输入采集和LED控制接口
 * 支持模拟摇杆(ADC)输入和数字按钮输入，以及WS2812 RGB LED控制
 */

#ifndef JOYSTICK_LED_CONTROLLER_H
#define JOYSTICK_LED_CONTROLLER_H

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "led_strip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 摇杆和LED配置 */
typedef struct {
    // 摇杆ADC配置
    adc_channel_t x_axis_channel;      // X轴ADC通道
    adc_channel_t y_axis_channel;      // Y轴ADC通道
    gpio_num_t button_pin;             // 摇杆按钮GPIO引脚
    
    // LED配置
    gpio_num_t led_pin;                // LED GPIO引脚
    uint32_t led_count;                // LED数量
    
    // 摇杆校准参数
    uint16_t x_center;                 // X轴中心值
    uint16_t y_center;                 // Y轴中心值
    uint16_t deadzone;                 // 死区范围
} joystick_led_config_t;

/* 摇杆读取数据结构 */
typedef struct {
    int16_t x_value;                   // X轴值 (-100 到 +100)
    int16_t y_value;                   // Y轴值 (-100 到 +100)
    bool button_pressed;               // 按钮状态
    uint16_t x_raw;                    // X轴原始ADC值
    uint16_t y_raw;                    // Y轴原始ADC值
} joystick_data_t;

/* 控制器句柄 */
typedef struct {
    joystick_led_config_t config;
    adc_oneshot_unit_handle_t adc_handle;
    led_strip_handle_t led_strip;
    bool initialized;
    bool led_state;                    // LED当前状态
    uint8_t led_brightness;            // LED亮度 (0-255)
    uint8_t led_hue;                   // LED色调 (0-360)
} joystick_led_handle_t;

/**
 * @brief 初始化摇杆和LED控制器
 * 
 * @param handle 控制器句柄
 * @param config 配置参数
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t joystick_led_init(joystick_led_handle_t *handle, const joystick_led_config_t *config);

/**
 * @brief 读取摇杆数据
 * 
 * @param handle 控制器句柄
 * @param data 输出的摇杆数据
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t joystick_read(joystick_led_handle_t *handle, joystick_data_t *data);

/**
 * @brief 设置LED状态
 * 
 * @param handle 控制器句柄
 * @param on LED开关状态
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t led_set_state(joystick_led_handle_t *handle, bool on);

/**
 * @brief 设置LED颜色
 * 
 * @param handle 控制器句柄
 * @param red 红色分量 (0-255)
 * @param green 绿色分量 (0-255)
 * @param blue 蓝色分量 (0-255)
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t led_set_color(joystick_led_handle_t *handle, uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief 设置LED HSV颜色
 * 
 * @param handle 控制器句柄
 * @param hue 色调 (0-360)
 * @param saturation 饱和度 (0-100)
 * @param value 亮度 (0-100)
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t led_set_hsv(joystick_led_handle_t *handle, uint16_t hue, uint8_t saturation, uint8_t value);

/**
 * @brief 切换LED状态
 * 
 * @param handle 控制器句柄
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t led_toggle(joystick_led_handle_t *handle);

/**
 * @brief 校准摇杆中心位置
 * 
 * @param handle 控制器句柄
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t joystick_calibrate_center(joystick_led_handle_t *handle);

/**
 * @brief 反初始化摇杆和LED控制器
 * 
 * @param handle 控制器句柄
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t joystick_led_deinit(joystick_led_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* JOYSTICK_LED_CONTROLLER_H */
