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
#include "voice_processor.h"
#include "web_interface.h"
#include "wifi_manager.h"

static const char *TAG = "AI_ASSISTANT";

// 全局队列用于任务间通信
QueueHandle_t voice_queue;
QueueHandle_t ai_response_queue;

// AI小智的个性化设置
typedef struct {
    char name[32];
    char personality[256];
    char voice_type[16];
    int response_speed;
} ai_personality_t;

static ai_personality_t ai_config = {
    .name = "小智",
    .personality = "我是一个友好的AI助手，可以帮助你回答问题、控制设备、播放音乐等。",
    .voice_type = "female",
    .response_speed = 1
};

// 语音处理任务
void voice_processing_task(void *pvParameters)
{
    voice_data_t voice_data;
    
    ESP_LOGI(TAG, "语音处理任务启动");
    
    while (1) {
        if (xQueueReceive(voice_queue, &voice_data, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "收到语音输入: %s", voice_data.text);
            
            // 处理语音命令
            ai_response_t response;
            if (ai_process_command(voice_data.text, &response) == ESP_OK) {
                // 发送响应到语音合成队列
                xQueueSend(ai_response_queue, &response, 0);
            }
        }
    }
}

// AI响应处理任务
void ai_response_task(void *pvParameters)
{
    ai_response_t response;
    
    ESP_LOGI(TAG, "AI响应处理任务启动");
    
    while (1) {
        if (xQueueReceive(ai_response_queue, &response, 0) == pdTRUE) {
            ESP_LOGI(TAG, "AI响应: %s", response.text);
            
            // 语音合成
            voice_synthesize(response.text, ai_config.voice_type);
            
            // 发送到Web界面
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
        
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30秒检查一次
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== AI小智启动 ===");
    ESP_LOGI(TAG, "版本: 1.0.0");
    ESP_LOGI(TAG, "AI助手: %s", ai_config.name);
    
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 创建队列
    voice_queue = xQueueCreate(10, sizeof(voice_data_t));
    ai_response_queue = xQueueCreate(10, sizeof(ai_response_t));
    
    if (voice_queue == NULL || ai_response_queue == NULL) {
        ESP_LOGE(TAG, "队列创建失败");
        return;
    }
    
    // 初始化WiFi
    wifi_init();
    
    // 初始化AI引擎
    ai_engine_init(&ai_config);
    
    // 初始化语音处理器
    voice_processor_init();
    
    // 初始化Web界面
    web_interface_init();
    
    // 创建任务
    xTaskCreate(voice_processing_task, "voice_proc", 4096, NULL, 5, NULL);
    xTaskCreate(ai_response_task, "ai_response", 4096, NULL, 5, NULL);
    xTaskCreate(system_monitor_task, "sys_monitor", 2048, NULL, 3, NULL);
    
    ESP_LOGI(TAG, "=== AI小智初始化完成 ===");
    ESP_LOGI(TAG, "你可以通过以下方式与小智交互:");
    ESP_LOGI(TAG, "1. 语音命令: 直接说话");
    ESP_LOGI(TAG, "2. Web界面: 访问 http://%s", wifi_get_ip());
    ESP_LOGI(TAG, "3. 移动端: 扫描二维码连接");
    
    // 播放启动音效
    voice_synthesize("小智已启动，随时为您服务", ai_config.voice_type);
}
