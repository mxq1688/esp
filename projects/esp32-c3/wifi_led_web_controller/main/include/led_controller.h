/*
 * LED Controller for ESP32-C3
 * 
 * RGB LED PWM控制和特效管理
 */

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "esp_err.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO定义 */
#define LED_RED_GPIO        3   // 红色LED
#define LED_GREEN_GPIO      4   // 绿色LED
#define LED_BLUE_GPIO       5   // 蓝色LED
#define STATUS_LED_GPIO     8   // 状态LED (内置)
#define BUTTON_GPIO         9   // 按钮 (内置)

/* PWM配置 */
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES       LEDC_TIMER_13_BIT // 8192 levels
#define LEDC_FREQUENCY      5000  // 5kHz
#define LEDC_MAX_DUTY       8191  // 2^13 - 1

/* LED通道定义 */
#define LEDC_CHANNEL_RED    LEDC_CHANNEL_0
#define LEDC_CHANNEL_GREEN  LEDC_CHANNEL_1
#define LEDC_CHANNEL_BLUE   LEDC_CHANNEL_2

/* 颜色结构体 */
typedef struct {
    uint16_t red;       // 0-255
    uint16_t green;     // 0-255
    uint16_t blue;      // 0-255
    uint8_t brightness; // 0-100%
} rgb_color_t;

/* HSV颜色结构体 */
typedef struct {
    float hue;          // 0-360°
    float saturation;   // 0-1
    float value;        // 0-1
} hsv_color_t;

/* LED状态结构体 */
typedef struct {
    rgb_color_t color;
    bool power_on;
    char effect_mode[32];
    uint16_t effect_speed;  // 1-100
    bool effect_running;
} led_state_t;

/* LED特效类型 */
typedef enum {
    LED_EFFECT_STATIC,
    LED_EFFECT_RAINBOW,
    LED_EFFECT_BREATHING,
    LED_EFFECT_BLINK,
    LED_EFFECT_FADE,
    LED_EFFECT_STROBE,
    LED_EFFECT_PULSE,
    LED_EFFECT_WAVE
} led_effect_type_t;

/* 预设颜色 */
extern const rgb_color_t COLOR_RED;
extern const rgb_color_t COLOR_GREEN;
extern const rgb_color_t COLOR_BLUE;
extern const rgb_color_t COLOR_WHITE;
extern const rgb_color_t COLOR_YELLOW;
extern const rgb_color_t COLOR_CYAN;
extern const rgb_color_t COLOR_MAGENTA;
extern const rgb_color_t COLOR_ORANGE;
extern const rgb_color_t COLOR_PURPLE;
extern const rgb_color_t COLOR_PINK;

/* 函数声明 */

/**
 * @brief 初始化LED控制器
 * @return ESP_OK on success
 */
esp_err_t led_controller_init(void);

/**
 * @brief 设置RGB颜色
 * @param color RGB颜色
 * @return ESP_OK on success
 */
esp_err_t led_set_color(const rgb_color_t* color);

/**
 * @brief 设置RGB颜色 (单独参数)
 * @param red 红色值 (0-255)
 * @param green 绿色值 (0-255)
 * @param blue 蓝色值 (0-255)
 * @return ESP_OK on success
 */
esp_err_t led_set_rgb(uint16_t red, uint16_t green, uint16_t blue);

/**
 * @brief 设置HSV颜色
 * @param hsv HSV颜色
 * @return ESP_OK on success
 */
esp_err_t led_set_hsv(const hsv_color_t* hsv);

/**
 * @brief 设置亮度
 * @param brightness 亮度 (0-100%)
 * @return ESP_OK on success
 */
esp_err_t led_set_brightness(uint8_t brightness);

/**
 * @brief 开启/关闭LED
 * @param power_on true为开启，false为关闭
 * @return ESP_OK on success
 */
esp_err_t led_set_power(bool power_on);

/**
 * @brief 切换LED开关状态
 * @return ESP_OK on success
 */
esp_err_t led_toggle_power(void);

/**
 * @brief 启动LED特效
 * @param effect 特效类型
 * @param speed 特效速度 (1-100)
 * @return ESP_OK on success
 */
esp_err_t led_start_effect(led_effect_type_t effect, uint16_t speed);

/**
 * @brief 停止LED特效
 * @return ESP_OK on success
 */
esp_err_t led_stop_effect(void);

/**
 * @brief 获取当前LED状态
 * @return 当前LED状态指针
 */
const led_state_t* led_get_state(void);

/**
 * @brief HSV转RGB
 * @param hsv HSV颜色
 * @param rgb 输出RGB颜色
 */
void led_hsv_to_rgb(const hsv_color_t* hsv, rgb_color_t* rgb);

/**
 * @brief RGB转HSV
 * @param rgb RGB颜色
 * @param hsv 输出HSV颜色
 */
void led_rgb_to_hsv(const rgb_color_t* rgb, hsv_color_t* hsv);

/**
 * @brief 颜色渐变
 * @param from 起始颜色
 * @param to 目标颜色
 * @param progress 进度 (0.0-1.0)
 * @param result 输出颜色
 */
void led_color_fade(const rgb_color_t* from, const rgb_color_t* to, float progress, rgb_color_t* result);

/**
 * @brief 启动动画 (系统启动时)
 */
void led_startup_animation(void);

/**
 * @brief WiFi连接成功指示
 */
void led_wifi_connected_indication(void);

/**
 * @brief WiFi断开连接指示
 */
void led_wifi_disconnected_indication(void);

/**
 * @brief 恢复出厂设置指示
 */
void led_factory_reset_indication(void);

/**
 * @brief 设置预设颜色
 * @param preset_index 预设索引 (0-9)
 * @return ESP_OK on success
 */
esp_err_t led_set_preset_color(uint8_t preset_index);

/**
 * @brief 保存当前颜色到NVS
 * @return ESP_OK on success
 */
esp_err_t led_save_state_to_nvs(void);

/**
 * @brief 从NVS加载颜色
 * @return ESP_OK on success
 */
esp_err_t led_load_state_from_nvs(void);

#ifdef __cplusplus
}
#endif

#endif // LED_CONTROLLER_H
