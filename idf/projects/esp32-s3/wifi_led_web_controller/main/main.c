/*
 * ESP32-S3 WiFi LED Web Controller
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "cJSON.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

#include "wifi_manager.h"
#include "led_controller.h"
#include "web_server.h"

static const char *TAG = "ESP32S3_MAIN";

/* 全局变量 */
httpd_handle_t g_server = NULL;
esp_timer_handle_t status_timer = NULL;

/* 按钮处理任务 */
static void button_task(void *pvParameters)
{
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    
    bool last_state = true;
    uint32_t press_time = 0;
    
    while (1) {
        bool current_state = gpio_get_level(BUTTON_GPIO);
        
        if (last_state && !current_state) {
            press_time = esp_timer_get_time() / 1000;
            ESP_LOGI(TAG, "Button pressed");
        }
        else if (!last_state && current_state) {
            uint32_t duration = (esp_timer_get_time() / 1000) - press_time;
            ESP_LOGI(TAG, "Button released, duration: %" PRIu32 "ms", duration);
            
            if (duration > 50 && duration < 1000) {
                led_toggle_power();
                ESP_LOGI(TAG, "LED toggled via button");
            } else if (duration >= 3000) {
                ESP_LOGI(TAG, "Factory reset requested");
                wifi_factory_reset();
                esp_restart();
            }
        }
        
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/* 系统状态定时器回调 */
static void system_status_timer_callback(void* arg)
{
    static uint32_t counter = 0;
    counter++;
    
    if (counter % 60 == 0) {
        ESP_LOGI(TAG, "System Status - Uptime: %" PRIu32 "min, Free Heap: %" PRIu32 " bytes", 
                 counter / 2, esp_get_free_heap_size());
        
        const char *ip = wifi_get_ip_string();
        ESP_LOGI(TAG, "Network - %s, IP: %s", 
                 wifi_is_connected() ? "STA connected" : "disconnected",
                 (ip && strlen(ip) > 0) ? ip : "0.0.0.0");
    }
}

/* 应用程序主函数 */
void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 WiFi LED Web Controller Starting...");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "Chip: %s Rev v%d.%d", CONFIG_IDF_TARGET, chip_info.revision / 100, chip_info.revision % 100);
    
    // 1. 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");
    
    // 2. 初始化LED控制器
    ESP_ERROR_CHECK(led_controller_init());
    ESP_LOGI(TAG, "LED controller initialized");
    
    // 3. 显示启动动画
    led_startup_animation();
    
    // 4. 初始化WiFi管理器
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_LOGI(TAG, "WiFi manager initialized");
    
    // 5. 启动Web服务器
    ESP_LOGI(TAG, "Starting web server...");
    if (web_server_start() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start web server");
        return;
    }
    ESP_LOGI(TAG, "Web server started successfully");
    
    const char *ip = wifi_get_ip_string();
    bool have_ip = (ip && strlen(ip) > 0 && strcmp(ip, "0.0.0.0") != 0);
    ESP_LOGI(TAG, "Access URLs:");
    ESP_LOGI(TAG, "  - AP Mode: http://192.168.4.1");
    if (have_ip) {
        ESP_LOGI(TAG, "  - STA Mode: http://%s", ip);
    } else {
        ESP_LOGI(TAG, "  - STA Mode: waiting for IP ...");
    }
    
    // 6. 创建系统状态定时器
    esp_timer_create_args_t timer_args = {
        .callback = system_status_timer_callback,
        .name = "system_status"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &status_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(status_timer, 500000));
    
    // 7. 启动按钮处理任务
    xTaskCreate(button_task, "button", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Button task started");
    
    // 8. 系统初始化完成
    ESP_LOGI(TAG, "=== ESP32-S3 WiFi LED Controller Ready ===");
    ESP_LOGI(TAG, "Free heap: %" PRIu32 " bytes", esp_get_free_heap_size());
    
    // 主循环
    while (1) {
        static bool last_wifi_state = false;
        bool current_wifi_state = wifi_is_connected();
        
        if (current_wifi_state != last_wifi_state) {
            if (current_wifi_state) {
                ESP_LOGI(TAG, "WiFi connected - IP: %s", wifi_get_ip_string());
                led_wifi_connected_indication();
            } else {
                ESP_LOGI(TAG, "WiFi disconnected");
                led_wifi_disconnected_indication();
            }
            last_wifi_state = current_wifi_state;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
