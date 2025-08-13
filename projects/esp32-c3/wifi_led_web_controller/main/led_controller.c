/*
 * LED Controller Implementation for ESP32-C3
 * 
 * RGB LED PWM控制和特效实现
 */

#include "led_controller.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/semphr.h"
#include <math.h>
#include <string.h>

static const char *TAG = "LED_CONTROLLER";

/* 全局变量 */
static led_state_t s_led_state = {
    .color = {255, 255, 255, 50},
    .power_on = false,
    .effect_mode = "static",
    .effect_speed = 50,
    .effect_running = false
};

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
#define NVS_KEY_EFFECT_SPEED "effect_speed"

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
    
    // 配置定时器
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    // 配置红色通道
    ledc_channel_config_t ledc_channel_red = {
        .channel    = LEDC_CHANNEL_RED,
        .duty       = 0,
        .gpio_num   = LED_RED_GPIO,
        .speed_mode = LEDC_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER,
        .flags.output_invert = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_red));
    
    // 配置绿色通道
    ledc_channel_config_t ledc_channel_green = {
        .channel    = LEDC_CHANNEL_GREEN,
        .duty       = 0,
        .gpio_num   = LED_GREEN_GPIO,
        .speed_mode = LEDC_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER,
        .flags.output_invert = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_green));
    
    // 配置蓝色通道
    ledc_channel_config_t ledc_channel_blue = {
        .channel    = LEDC_CHANNEL_BLUE,
        .duty       = 0,
        .gpio_num   = LED_BLUE_GPIO,
        .speed_mode = LEDC_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER,
        .flags.output_invert = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_blue));
    
    s_led_initialized = true;
    ESP_LOGI(TAG, "LED controller initialized - R:%d G:%d B:%d", LED_RED_GPIO, LED_GREEN_GPIO, LED_BLUE_GPIO);
    
    return ESP_OK;
}

/* 更新PWM输出 */
static esp_err_t led_update_pwm_output(void)
{
    if (!s_led_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint32_t red_duty = 0, green_duty = 0, blue_duty = 0;
    
    if (s_led_state.power_on) {
        red_duty = (s_led_state.color.red * s_led_state.color.brightness * LEDC_MAX_DUTY) / (255 * 100);
        green_duty = (s_led_state.color.green * s_led_state.color.brightness * LEDC_MAX_DUTY) / (255 * 100);
        blue_duty = (s_led_state.color.blue * s_led_state.color.brightness * LEDC_MAX_DUTY) / (255 * 100);
    }
    
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_RED, red_duty));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_GREEN, green_duty));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_BLUE, blue_duty));
    
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_RED));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_GREEN));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_BLUE));
    
    return ESP_OK;
}

/* 设置RGB颜色 */
esp_err_t led_set_color(const rgb_color_t* color)
{
    if (color == NULL || !s_led_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_led_state.color = *color;
        esp_err_t ret = led_update_pwm_output();
        xSemaphoreGive(s_led_mutex);
        
        ESP_LOGI(TAG, "Color set to R:%d G:%d B:%d Brightness:%d%%", 
                 color->red, color->green, color->blue, color->brightness);
        return ret;
    }
    
    return ESP_ERR_TIMEOUT;
}

/* 设置RGB颜色值 */
esp_err_t led_set_rgb(uint16_t red, uint16_t green, uint16_t blue)
{
    rgb_color_t color = {
        .red = (red > 255) ? 255 : red,
        .green = (green > 255) ? 255 : green,
        .blue = (blue > 255) ? 255 : blue,
        .brightness = s_led_state.color.brightness
    };
    
    return led_set_color(&color);
}

/* 设置亮度 */
esp_err_t led_set_brightness(uint8_t brightness)
{
    if (brightness > 100 || !s_led_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_led_state.color.brightness = brightness;
        esp_err_t ret = led_update_pwm_output();
        xSemaphoreGive(s_led_mutex);
        
        ESP_LOGI(TAG, "Brightness set to %d%%", brightness);
        return ret;
    }
    
    return ESP_ERR_TIMEOUT;
}

/* 设置电源状态 */
esp_err_t led_set_power(bool power_on)
{
    if (!s_led_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_led_state.power_on = power_on;
        esp_err_t ret = led_update_pwm_output();
        xSemaphoreGive(s_led_mutex);
        
        ESP_LOGI(TAG, "LED power %s", power_on ? "ON" : "OFF");
        return ret;
    }
    
    return ESP_ERR_TIMEOUT;
}

/* 切换电源状态 */
esp_err_t led_toggle_power(void)
{
    return led_set_power(!s_led_state.power_on);
}

/* 获取LED状态 */
const led_state_t* led_get_state(void)
{
    return &s_led_state;
}

/* HSV转RGB */
void led_hsv_to_rgb(const hsv_color_t* hsv, rgb_color_t* rgb)
{
    if (hsv == NULL || rgb == NULL) {
        return;
    }
    
    float h = fmod(hsv->hue, 360.0f);
    float s = (hsv->saturation > 1.0f) ? 1.0f : (hsv->saturation < 0.0f) ? 0.0f : hsv->saturation;
    float v = (hsv->value > 1.0f) ? 1.0f : (hsv->value < 0.0f) ? 0.0f : hsv->value;
    
    float c = v * s;
    float x = c * (1.0f - fabsf(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    
    float r, g, b;
    
    if (h >= 0 && h < 60) {
        r = c; g = x; b = 0;
    } else if (h >= 60 && h < 120) {
        r = x; g = c; b = 0;
    } else if (h >= 120 && h < 180) {
        r = 0; g = c; b = x;
    } else if (h >= 180 && h < 240) {
        r = 0; g = x; b = c;
    } else if (h >= 240 && h < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    rgb->red = (uint16_t)((r + m) * 255);
    rgb->green = (uint16_t)((g + m) * 255);
    rgb->blue = (uint16_t)((b + m) * 255);
}

/* LED特效任务 */
static void led_effect_task(void *pvParameters)
{
    uint16_t hue = 0;
    uint8_t brightness_val = 10;
    int8_t brightness_dir = 1;
    bool blink_state = false;
    uint32_t effect_delay = 100;
    
    rgb_color_t temp_color;
    hsv_color_t hsv;
    
    while (s_led_state.effect_running) {
        effect_delay = 200 - (s_led_state.effect_speed * 2);
        if (effect_delay < 10) effect_delay = 10;
        
        if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (strcmp(s_led_state.effect_mode, "rainbow") == 0) {
                hsv.hue = hue;
                hsv.saturation = 1.0f;
                hsv.value = 1.0f;
                led_hsv_to_rgb(&hsv, &temp_color);
                temp_color.brightness = s_led_state.color.brightness;
                
                s_led_state.color.red = temp_color.red;
                s_led_state.color.green = temp_color.green;
                s_led_state.color.blue = temp_color.blue;
                led_update_pwm_output();
                
                hue = (hue + 2) % 360;
                
            } else if (strcmp(s_led_state.effect_mode, "breathing") == 0) {
                s_led_state.color.brightness = brightness_val;
                led_update_pwm_output();
                
                brightness_val += brightness_dir * 2;
                if (brightness_val >= 100) {
                    brightness_val = 100;
                    brightness_dir = -1;
                } else if (brightness_val <= 10) {
                    brightness_val = 10;
                    brightness_dir = 1;
                }
                
            } else if (strcmp(s_led_state.effect_mode, "blink") == 0) {
                s_led_state.power_on = blink_state;
                led_update_pwm_output();
                blink_state = !blink_state;
                effect_delay = 500;
            }
            
            xSemaphoreGive(s_led_mutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(effect_delay));
    }
    
    if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        strcpy(s_led_state.effect_mode, "static");
        s_led_state.power_on = true;
        led_update_pwm_output();
        xSemaphoreGive(s_led_mutex);
    }
    
    s_effect_task_handle = NULL;
    vTaskDelete(NULL);
}

/* 启动LED特效 */
esp_err_t led_start_effect(led_effect_type_t effect, uint16_t speed)
{
    if (!s_led_initialized || speed > 100) {
        return ESP_ERR_INVALID_ARG;
    }
    
    led_stop_effect();
    
    if (xSemaphoreTake(s_led_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_led_state.effect_speed = speed;
        
        switch (effect) {
            case LED_EFFECT_RAINBOW:
                strcpy(s_led_state.effect_mode, "rainbow");
                break;
            case LED_EFFECT_BREATHING:
                strcpy(s_led_state.effect_mode, "breathing");
                break;
            case LED_EFFECT_BLINK:
                strcpy(s_led_state.effect_mode, "blink");
                break;
            default:
                strcpy(s_led_state.effect_mode, "static");
                xSemaphoreGive(s_led_mutex);
                return ESP_OK;
        }
        
        if (effect != LED_EFFECT_STATIC) {
            s_led_state.effect_running = true;
            s_led_state.power_on = true;
            
            xTaskCreate(led_effect_task, "led_effect", 3072, NULL, 5, &s_effect_task_handle);
            ESP_LOGI(TAG, "LED effect started: %s (speed: %d)", s_led_state.effect_mode, speed);
        }
        
        xSemaphoreGive(s_led_mutex);
    }
    
    return ESP_OK;
}

/* 停止LED特效 */
esp_err_t led_stop_effect(void)
{
    if (s_effect_task_handle != NULL) {
        s_led_state.effect_running = false;
        
        int timeout = 10;
        while (s_effect_task_handle != NULL && timeout-- > 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        if (s_effect_task_handle != NULL) {
            vTaskDelete(s_effect_task_handle);
            s_effect_task_handle = NULL;
        }
        
        ESP_LOGI(TAG, "LED effect stopped");
    }
    
    return ESP_OK;
}

/* 启动动画 */
void led_startup_animation(void)
{
    ESP_LOGI(TAG, "Playing startup animation...");
    
    hsv_color_t hsv = {0, 1.0f, 1.0f};
    rgb_color_t rgb;
    
    for (int i = 0; i < 360; i += 20) {
        hsv.hue = i;
        led_hsv_to_rgb(&hsv, &rgb);
        rgb.brightness = 30;
        
        led_set_color(&rgb);
        led_set_power(true);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    led_set_power(false);
}

/* WiFi连接指示 */
void led_wifi_connected_indication(void)
{
    rgb_color_t green = {0, 255, 0, 50};
    
    for (int i = 0; i < 3; i++) {
        led_set_color(&green);
        led_set_power(true);
        vTaskDelay(pdMS_TO_TICKS(200));
        led_set_power(false);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/* WiFi断开指示 */
void led_wifi_disconnected_indication(void)
{
    rgb_color_t red = {255, 0, 0, 50};
    
    for (int i = 0; i < 2; i++) {
        led_set_color(&red);
        led_set_power(true);
        vTaskDelay(pdMS_TO_TICKS(300));
        led_set_power(false);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/* 其他未实现的函数存根 */
esp_err_t led_set_hsv(const hsv_color_t* hsv) { return ESP_OK; }
void led_rgb_to_hsv(const rgb_color_t* rgb, hsv_color_t* hsv) { }
void led_color_fade(const rgb_color_t* from, const rgb_color_t* to, float progress, rgb_color_t* result) { }
void led_factory_reset_indication(void) { }
esp_err_t led_set_preset_color(uint8_t preset_index) { return ESP_OK; }
esp_err_t led_save_state_to_nvs(void) { return ESP_OK; }
esp_err_t led_load_state_from_nvs(void) { return ESP_OK; }