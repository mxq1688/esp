#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "esp_err.h"
#include "driver/gpio.h"
#include <stdint.h>
#include <stdbool.h>

// LED配置 - ESP32-S3适配
// ESP32-S3-DevKitC-1 板载RGB LED引脚:
// - 首次发布版本: GPIO48
// - v1.1版本: GPIO38
// 如果GPIO48不工作，请尝试GPIO38
#define LED_GPIO_V1 GPIO_NUM_48    // ESP32-S3 DevKitC-1 首次发布版本
#define LED_GPIO_V11 GPIO_NUM_38   // ESP32-S3 DevKitC-1 v1.1版本
#define LED_GPIO LED_GPIO_V11      // 默认使用v1.1版本的GPIO38
#define LED_STRIP_LENGTH 1         // 单个LED

// 按钮配置
#define BUTTON_GPIO GPIO_NUM_0  // ESP32-S3 DevKitC-1板载按钮

// RGB颜色结构
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t brightness;
} rgb_color_t;

// 特效类型
typedef enum {
    LED_EFFECT_NONE = 0,
    LED_EFFECT_RAINBOW,
    LED_EFFECT_BREATH,
    LED_EFFECT_BLINK,
    LED_EFFECT_FADE
} led_effect_t;

// 函数声明
esp_err_t led_controller_init(void);
esp_err_t led_set_color(const rgb_color_t *color);
esp_err_t led_set_power(bool power);
esp_err_t led_toggle_power(void);
esp_err_t led_set_brightness(uint8_t brightness);
esp_err_t led_set_effect(led_effect_t effect);
esp_err_t led_startup_animation(void);
esp_err_t led_wifi_connected_indication(void);
esp_err_t led_wifi_disconnected_indication(void);
bool led_get_power_state(void);
rgb_color_t led_get_current_color(void);

#endif // LED_CONTROLLER_H
