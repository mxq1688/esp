#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "cJSON.h"

#include "ai_engine.h"
// #include "voice_processor.h"  // ESP32-C3不支持音频处理
#include "web_interface.h"
#include "wifi_manager.h"

static const char *TAG = "AI_ASSISTANT_C3";

// 全局队列用于任务间通信 (仅保留Web交互队列)
QueueHandle_t web_request_queue;
QueueHandle_t ai_response_queue;

// AI小智的个性化设置 (ESP32-C3版本)
typedef struct {
    char name[32];
    char personality[256];
    char interface_type[16];
    int response_speed;
} ai_personality_t;

static ai_personality_t ai_config = {
    .name = "小智C3",
    .personality = "我是一个基于ESP32-C3的AI助手，可以通过Web界面帮助你回答问题、控制设备等。",
    .interface_type = "web",
    .response_speed = 1
};

// Web请求处理任务
void web_request_task(void *pvParameters)
{
    char request_text[512];
    
    ESP_LOGI(TAG, "Web请求处理任务启动");
    
    while (1) {
        if (xQueueReceive(web_request_queue, request_text, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "收到Web请求: %s", request_text);
            
            // 处理AI命令
            ai_response_t response;
            if (ai_process_command(request_text, &response) == ESP_OK) {
                // 发送响应到Web界面
                xQueueSend(ai_response_queue, &response, 0);
            }
        }
    }
}

// AI响应处理任务 (ESP32-C3版本 - 仅Web输出)
void ai_response_task(void *pvParameters)
{
    ai_response_t response;
    
    ESP_LOGI(TAG, "AI响应处理任务启动");
    
    while (1) {
        if (xQueueReceive(ai_response_queue, &response, 0) == pdTRUE) {
            ESP_LOGI(TAG, "AI响应: %s", response.text);
            
            // 发送到Web界面 (ESP32-C3主要交互方式)
            web_send_response(&response);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// 系统状态监控任务
void system_monitor_task(void *pvParameters)
{
    while (1) {
        // 检查WiFi连接状态
        if (wifi_is_connected()) {
            ESP_LOGI(TAG, "WiFi连接正常");
        } else {
            ESP_LOGW(TAG, "WiFi连接断开，尝试重连");
            wifi_reconnect();
        }
        
        // 检查系统内存
        size_t free_heap = esp_get_free_heap_size();
        ESP_LOGI(TAG, "可用内存: %zu bytes", free_heap);
        
        // ESP32-C3特有状态检查
        ESP_LOGI(TAG, "ESP32-C3 AI助手运行正常");
        
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30秒检查一次
    }
}

// ESP32-C3专用的Web交互接口
esp_err_t esp32c3_web_command_handler(const char *command)
{
    if (web_request_queue != NULL) {
        if (xQueueSend(web_request_queue, command, pdMS_TO_TICKS(1000)) == pdTRUE) {
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP32-C3 AI小智启动 ====");
    ESP_LOGI(TAG, "版本: 1.0.0-ESP32C3");
    ESP_LOGI(TAG, "AI助手: %s", ai_config.name);
    ESP_LOGI(TAG, "交互方式: Web界面 (无音频功能)");
    
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 创建队列 (仅Web交互队列)
    web_request_queue = xQueueCreate(10, 512);  // 支持较长的Web请求
    ai_response_queue = xQueueCreate(10, sizeof(ai_response_t));
    
    if (web_request_queue == NULL || ai_response_queue == NULL) {
        ESP_LOGE(TAG, "队列创建失败");
        return;
    }
    
    // 初始化WiFi
    ESP_LOGI(TAG, "初始化WiFi...");
    wifi_init();
    
    // 初始化AI引擎
    ESP_LOGI(TAG, "初始化AI引擎...");
    ai_engine_init(&ai_config);
    
    // 跳过语音处理器初始化 (ESP32-C3不支持)
    ESP_LOGI(TAG, "跳过音频初始化 (ESP32-C3不支持)");
    
    // 初始化Web界面
    ESP_LOGI(TAG, "初始化Web界面...");
    web_interface_init();
    
    // 创建任务 (仅非音频任务)
    xTaskCreate(web_request_task, "web_req", 4096, NULL, 5, NULL);
    xTaskCreate(ai_response_task, "ai_response", 4096, NULL, 5, NULL);
    xTaskCreate(system_monitor_task, "sys_monitor", 2048, NULL, 3, NULL);
    
    ESP_LOGI(TAG, "=== ESP32-C3 AI小智初始化完成 ====");
    ESP_LOGI(TAG, "交互方式:");
    ESP_LOGI(TAG, "1. Web界面: 访问 http://%s", wifi_get_ip());
    ESP_LOGI(TAG, "2. 移动端: 扫描二维码连接");
    ESP_LOGI(TAG, "注意: ESP32-C3版本不支持语音功能");
    
    ESP_LOGI(TAG, "ESP32-C3 AI助手就绪，请通过Web界面与我交互！");
}
