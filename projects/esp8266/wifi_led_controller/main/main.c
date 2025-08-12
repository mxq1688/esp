/*
 * ESP8266 WiFi LED Controller
 * 
 * 功能特性：
 * - WiFi连接和AP模式支持
 * - HTTP Web服务器
 * - RGB LED PWM控制 (GPIO12, GPIO13, GPIO15)
 * - RESTful API接口
 * - 实时颜色和亮度控制
 * - 多种LED特效模式
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/pwm.h"
#include "esp_http_server.h"
#include "cJSON.h"

static const char *TAG = "ESP8266_LED_CONTROLLER";

/* WiFi配置 - 请修改为您的WiFi信息 */
#define WIFI_SSID      "Your_WiFi_SSID"
#define WIFI_PASSWORD  "Your_WiFi_Password"

/* AP模式配置 */
#define ESP_WIFI_AP_SSID      "ESP8266_LED_Controller"
#define ESP_WIFI_AP_PASS      "12345678"
#define ESP_WIFI_AP_CHANNEL   1
#define MAX_STA_CONN          4

/* LED GPIO配置 */
#define LED_RED_GPIO    12
#define LED_GREEN_GPIO  13
#define LED_BLUE_GPIO   15

/* PWM配置 */
#define PWM_PERIOD_US   1000    // 1kHz PWM频率
#define PWM_MAX_DUTY    1023    // ESP8266 PWM最大占空比

/* FreeRTOS事件组 */
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* LED状态结构体 */
typedef struct {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint8_t brightness;  // 0-100%
    bool power_on;
    char effect_mode[32];
} led_state_t;

static led_state_t current_led_state = {
    .red = 255,
    .green = 255,
    .blue = 255,
    .brightness = 50,
    .power_on = false,
    .effect_mode = "static"
};

static httpd_handle_t server = NULL;
static int s_retry_num = 0;

/* PWM引脚和占空比数组 */
static uint32_t pwm_gpio[3] = {LED_RED_GPIO, LED_GREEN_GPIO, LED_BLUE_GPIO};
static uint32_t pwm_duties[3] = {0, 0, 0};
static int16_t pwm_phase[3] = {0, 0, 0};

/* 特效控制 */
static TaskHandle_t effect_task_handle = NULL;
static bool effect_running = false;

void app_main(void)
{
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP8266 WiFi LED Controller Starting...");

    // 初始化PWM
    pwm_init(PWM_PERIOD_US, pwm_duties, 3, pwm_gpio);
    pwm_set_phases(pwm_phase);
    pwm_start();
    
    ESP_LOGI(TAG, "PWM initialized on GPIO %d, %d, %d", 
             LED_RED_GPIO, LED_GREEN_GPIO, LED_BLUE_GPIO);

    // 初始化WiFi
    wifi_init_sta();

    // 启动HTTP服务器
    server = start_webserver();
    if (server) {
        ESP_LOGI(TAG, "HTTP Server started successfully");
        ESP_LOGI(TAG, "Open browser and navigate to: http://192.168.4.1 (AP mode) or your IP address");
    }

    // 主循环
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}