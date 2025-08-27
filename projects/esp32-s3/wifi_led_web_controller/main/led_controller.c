/*
 * LED Controller Implementation for ESP32-S3
 * 
 * WS2812 RGB LED控制实现
 */

#include "led_controller.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led_strip.h"
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
static gpio_num_t s_active_led_gpio = LED_GPIO;  // 实际使用的GPIO引脚
static led_strip_handle_t s_led_strip = NULL;    // LED Strip句柄

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

/* 测试WS2812 LED */
static bool test_ws2812_led(gpio_num_t gpio_pin)
{
    ESP_LOGI(TAG, "Testing WS2812 LED on GPIO%d...", gpio_pin);
    
    // 创建临时LED Strip配置
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_pin,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out = false,
    };
    
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    
    led_strip_handle_t test_strip = NULL;
    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &test_strip);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to create LED strip on GPIO%d: %s", gpio_pin, esp_err_to_name(ret));
        return false;
    }
    
    // 测试LED - 红绿蓝闪烁
    uint8_t colors[][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
    for (int i = 0; i < 3; i++) {
        led_strip_set_pixel(test_strip, 0, colors[i][0], colors[i][1], colors[i][2]);
        led_strip_refresh(test_strip);
        vTaskDelay(pdMS_TO_TICKS(200));
        led_strip_clear(test_strip);
        led_strip_refresh(test_strip);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // 清理
    led_strip_del(test_strip);
    
    ESP_LOGI(TAG, "WS2812 test on GPIO%d completed", gpio_pin);
    return true;
}

/* LED控制器初始化 */
esp_err_t led_controller_init(void)
{
    ESP_LOGI(TAG, "Initializing WS2812 LED controller...");
    
    // 创建互斥锁
    s_led_mutex = xSemaphoreCreateMutex();
    if (s_led_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create LED mutex");
        return ESP_FAIL;
    }
    
    // 自动探测WS2812数据脚：按 GPIO48 -> GPIO38 -> GPIO2 顺序
    const gpio_num_t probe_pins[] = { LED_GPIO_V1, LED_GPIO_V11, GPIO_NUM_2 };
    const char *probe_names[] = { "GPIO48", "GPIO38", "GPIO2" };
    bool found = false;
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "Probing WS2812 on %s...", probe_names[i]);
        if (test_ws2812_led(probe_pins[i])) {
            s_active_led_gpio = probe_pins[i];
            ESP_LOGI(TAG, "WS2812 detected on %s", probe_names[i]);
            found = true;
            break;
        }
    }
    if (!found) {
        ESP_LOGW(TAG, "WS2812 not detected on GPIO48/GPIO38/GPIO2, fallback to default %d", (int)LED_GPIO);
        s_active_led_gpio = LED_GPIO;
    }

    // 使用选中的引脚创建LED Strip句柄
    led_strip_config_t strip_config = {
        .strip_gpio_num = s_active_led_gpio,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 初始状态：LED关闭
    led_strip_clear(s_led_strip);
    led_strip_refresh(s_led_strip);
    
    s_led_initialized = true;
    ESP_LOGI(TAG, "WS2812 LED controller initialized - GPIO:%d", s_active_led_gpio);
    
    return ESP_OK;
}

/* 更新LED输出 */
static esp_err_t led_update_output(void)
{
    if (!s_led_initialized || s_led_strip == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!s_power_on) {
        // LED关闭
        led_strip_clear(s_led_strip);
        led_strip_refresh(s_led_strip);
    } else {
        // LED开启，设置RGB颜色和亮度
        uint8_t r = (s_current_color.r * s_current_color.brightness) / 100;
        uint8_t g = (s_current_color.g * s_current_color.brightness) / 100;
        uint8_t b = (s_current_color.b * s_current_color.brightness) / 100;
        
        led_strip_set_pixel(s_led_strip, 0, r, g, b);
        led_strip_refresh(s_led_strip);
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
    
    // WS2812启动动画 - 彩虹色循环
    uint8_t rainbow_colors[][3] = {
        {255, 0, 0},   // 红
        {255, 127, 0}, // 橙
        {255, 255, 0}, // 黄
        {0, 255, 0},   // 绿
        {0, 0, 255},   // 蓝
        {75, 0, 130},  // 靛
        {148, 0, 211}  // 紫
    };
    
    for (int i = 0; i < 7; i++) {
        led_strip_set_pixel(s_led_strip, 0, rainbow_colors[i][0], rainbow_colors[i][1], rainbow_colors[i][2]);
        led_strip_refresh(s_led_strip);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    // 清除
    led_strip_clear(s_led_strip);
    led_strip_refresh(s_led_strip);
    
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
        led_strip_set_pixel(s_led_strip, 0, 0, 255, 0); // 绿色
        led_strip_refresh(s_led_strip);
        vTaskDelay(pdMS_TO_TICKS(100));
        led_strip_clear(s_led_strip);
        led_strip_refresh(s_led_strip);
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
        led_strip_set_pixel(s_led_strip, 0, 255, 0, 0); // 红色
        led_strip_refresh(s_led_strip);
        vTaskDelay(pdMS_TO_TICKS(100));
        led_strip_clear(s_led_strip);
        led_strip_refresh(s_led_strip);
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
