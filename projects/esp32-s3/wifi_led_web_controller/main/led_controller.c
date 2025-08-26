/*
 * LED Controller Implementation for ESP32-S3
 * 
 * 简化的LED控制实现，使用GPIO直接控制
 */

#include "led_controller.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <math.h>
#include <string.h>

static const char *TAG = "LED_CONTROLLER";

/* 全局变量 */
static rgb_color_t s_current_color = {255, 255, 255, 50};
static bool s_power_on = false;
static led_effect_t s_current_effect = LED_EFFECT_NONE;
static TaskHandle_t s_effect_task_handle = NULL;
static SemaphoreHandle_t s_led_mutex = NULL;
static bool s_led_initialized = false;

/* 预设颜色定义 */
const rgb_color_t COLOR_RED       = {255, 0,   0,   100};
const rgb_color_t COLOR_GREEN     = {0,   255, 0,   100};
const rgb_color_t COLOR_BLUE      = {0,   0,   255, 100};
const rgb_color_t COLOR_WHITE     = {255, 255, 255, 100};
const rgb_color_t COLOR_YELLOW    = {255, 255, 0,   100};
const rgb_color_t COLOR_CYAN      = {0,   255, 255, 100};
const rgb_color_t COLOR_MAGENTA   = {255, 0,   255, 100};
const rgb_color_t COLOR_ORANGE    = {255, 165, 0,   100};
const rgb_color_t COLOR_PURPLE    = {128, 0,   128, 100};
const rgb_color_t COLOR_PINK      = {255, 192, 203, 100};

/* NVS配置键 */
#define NVS_NAMESPACE "led_config"
#define NVS_KEY_RED "red"
#define NVS_KEY_GREEN "green"
#define NVS_KEY_BLUE "blue"
#define NVS_KEY_BRIGHTNESS "brightness"
#define NVS_KEY_POWER "power"
#define NVS_KEY_EFFECT "effect"

/* LED控制器初始化 */
esp_err_t led_controller_init(void)
{
    ESP_LOGI(TAG, "Initializing LED controller...");
    
    // 创建互斥锁
    s_led_mutex = xSemaphoreCreateMutex();
    if (s_led_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create LED mutex");
        return ESP_FAIL;
    }
    
    // 配置GPIO作为LED输出
    ESP_LOGI(TAG, "Configuring LED GPIO: %d", LED_GPIO);
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 初始状态：LED关闭
    gpio_set_level(LED_GPIO, 0);
    
    s_led_initialized = true;
    ESP_LOGI(TAG, "LED controller initialized - GPIO:%d", LED_GPIO);
    
    return ESP_OK;
}

/* 更新LED输出 */
static esp_err_t led_update_output(void)
{
    if (!s_led_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!s_power_on) {
        // LED关闭
        gpio_set_level(LED_GPIO, 0);
    } else {
        // LED开启（简化版本，不考虑颜色，只考虑亮度）
        int brightness = s_current_color.brightness;
        if (brightness > 50) {
            gpio_set_level(LED_GPIO, 1);
        } else {
            gpio_set_level(LED_GPIO, 0);
        }
    }
    
    return ESP_OK;
}

/* 设置LED颜色 */
esp_err_t led_set_color(const rgb_color_t *color)
{
    if (color == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_current_color = *color;
        esp_err_t ret = led_update_output();
        xSemaphoreGive(s_led_mutex);
        return ret;
    }
    
    return ESP_ERR_TIMEOUT;
}

/* 设置LED电源状态 */
esp_err_t led_set_power(bool power)
{
    if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_power_on = power;
        esp_err_t ret = led_update_output();
        xSemaphoreGive(s_led_mutex);
        return ret;
    }
    
    return ESP_ERR_TIMEOUT;
}

/* 切换LED电源状态 */
esp_err_t led_toggle_power(void)
{
    return led_set_power(!s_power_on);
}

/* 设置亮度 */
esp_err_t led_set_brightness(uint8_t brightness)
{
    if (brightness > 100) {
        brightness = 100;
    }
    
    if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_current_color.brightness = brightness;
        esp_err_t ret = led_update_output();
        xSemaphoreGive(s_led_mutex);
        return ret;
    }
    
    return ESP_ERR_TIMEOUT;
}

/* 特效任务 */
static void led_effect_task(void *pvParameters)
{
    led_effect_t effect = (led_effect_t)pvParameters;
    uint32_t counter = 0;
    
    while (1) {
        if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            switch (effect) {
                case LED_EFFECT_BLINK:
                    // 闪烁特效
                    s_current_color.brightness = (counter % 20 < 10) ? 100 : 0;
                    break;
                    
                case LED_EFFECT_BREATH:
                    // 呼吸特效（简化版）
                    s_current_color.brightness = (counter % 20 < 10) ? 100 : 20;
                    break;
                    
                case LED_EFFECT_FADE:
                    // 渐变特效（简化版）
                    s_current_color.brightness = (counter % 20 < 10) ? 100 : 50;
                    break;
                    
                default:
                    break;
            }
            
            led_update_output();
            xSemaphoreGive(s_led_mutex);
        }
        
        counter++;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/* 设置LED特效 */
esp_err_t led_set_effect(led_effect_t effect)
{
    // 停止当前特效任务
    if (s_effect_task_handle != NULL) {
        vTaskDelete(s_effect_task_handle);
        s_effect_task_handle = NULL;
    }
    
    s_current_effect = effect;
    
    if (effect != LED_EFFECT_NONE) {
        // 启动新的特效任务
        xTaskCreate(led_effect_task, "led_effect", 2048, (void*)effect, 5, &s_effect_task_handle);
    }
    
    return ESP_OK;
}

/* 启动动画 */
esp_err_t led_startup_animation(void)
{
    ESP_LOGI(TAG, "Starting LED startup animation...");
    
    // 简单的启动动画
    for (int i = 0; i < 3; i++) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    // 设置为低亮度
    rgb_color_t startup_color = {255, 255, 255, 20};
    led_set_color(&startup_color);
    led_set_power(true);
    
    ESP_LOGI(TAG, "LED startup animation completed");
    return ESP_OK;
}

/* WiFi连接指示 */
esp_err_t led_wifi_connected_indication(void)
{
    // 绿色闪烁3次表示WiFi连接成功
    for (int i = 0; i < 3; i++) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // 恢复之前的颜色
    led_set_color(&s_current_color);
    led_set_power(s_power_on);
    
    return ESP_OK;
}

/* WiFi断开指示 */
esp_err_t led_wifi_disconnected_indication(void)
{
    // 红色闪烁3次表示WiFi断开
    for (int i = 0; i < 3; i++) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // 恢复之前的颜色
    led_set_color(&s_current_color);
    led_set_power(s_power_on);
    
    return ESP_OK;
}

/* 获取LED电源状态 */
bool led_get_power_state(void)
{
    return s_power_on;
}

/* 获取当前颜色 */
rgb_color_t led_get_current_color(void)
{
    return s_current_color;
}
