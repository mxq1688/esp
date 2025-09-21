#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "ml307r_driver.h"
#include "wifi_manager.h"
#include "web_server.h"

static const char *TAG = "MAIN";

// 任务句柄
static TaskHandle_t ml307r_task_handle = NULL;
static TaskHandle_t status_task_handle = NULL;

// ML307R监控任务
static void ml307r_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "ML307R monitor task started");
    
    while (1) {
        if (ml307r_is_ready()) {
            // 检查网络状态
            ml307r_network_info_t network_info;
            esp_err_t ret = ml307r_get_network_info(&network_info);
            
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Network: %s, Signal: %d dBm, Connected: %s", 
                         network_info.operator_name, 
                         network_info.signal_strength,
                         network_info.is_connected ? "Yes" : "No");
            }
        } else {
            ESP_LOGW(TAG, "ML307R not ready, current state: %d", ml307r_get_state());
        }
        
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30秒检查一次
    }
}

// 系统状态监控任务
static void status_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Status monitor task started");
    
    while (1) {
        // 打印系统状态
        ESP_LOGI(TAG, "=== System Status ===");
        ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "ML307R state: %d", ml307r_get_state());
        ESP_LOGI(TAG, "WiFi state: %d", wifi_manager_get_state());
        ESP_LOGI(TAG, "Web server running: %s", web_server_is_running() ? "Yes" : "No");
        
        vTaskDelay(pdMS_TO_TICKS(60000)); // 60秒打印一次状态
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "ESP32-S3 ML307R 4G Hotspot Controller");
    ESP_LOGI(TAG, "Version: 1.0.0");
    ESP_LOGI(TAG, "=================================");

    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "✅ NVS initialized");

    // 初始化WiFi管理器
    ret = wifi_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ Failed to initialize WiFi manager: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "✅ WiFi manager initialized");

    // 启动AP模式
    ret = wifi_manager_start_ap();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ Failed to start AP mode: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "✅ WiFi AP mode started");

    // 启用NAPT用于互联网共享
    ret = wifi_manager_enable_napt();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "⚠️  Failed to enable NAPT: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "✅ NAPT enabled for internet sharing");
    }

    // 启动Web服务器
    ret = web_server_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ Failed to start web server: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "✅ Web server started");

    // 初始化ML307R模块
    ESP_LOGI(TAG, "Initializing ML307R module...");
    ret = ml307r_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "⚠️  ML307R initialization returned: %s", esp_err_to_name(ret));
        ESP_LOGW(TAG, "⚠️  But ML307R module seems to be working, continuing...");
    } else {
        ESP_LOGI(TAG, "✅ ML307R module initialized");
    }
    
    // 尝试建立4G数据连接（即使初始化失败也尝试）
    ESP_LOGI(TAG, "Attempting to establish 4G data connection...");
    ret = ml307r_establish_data_connection();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "⚠️  Failed to establish 4G data connection: %s", esp_err_to_name(ret));
        ESP_LOGW(TAG, "⚠️  Hotspot will work without internet access");
    } else {
        ESP_LOGI(TAG, "✅ 4G data connection established");
    }
    
    // 创建ML307R监控任务
    xTaskCreate(ml307r_monitor_task, "ml307r_monitor", 4096, NULL, 5, &ml307r_task_handle);
    ESP_LOGI(TAG, "✅ ML307R monitor task created");

    // 创建状态监控任务
    xTaskCreate(status_monitor_task, "status_monitor", 3072, NULL, 3, &status_task_handle);
    ESP_LOGI(TAG, "✅ Status monitor task created");

    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "🚀 System initialization completed!");
    ESP_LOGI(TAG, "📱 Access web interface at: http://192.168.4.1");
    ESP_LOGI(TAG, "📶 WiFi AP: ESP32-S3-ML307R");
    ESP_LOGI(TAG, "🔑 Password: 12345678");
    ESP_LOGI(TAG, "=================================");

    // 主循环
    while (1) {
        // 检查任务健康状态
        if (ml307r_task_handle != NULL && eTaskGetState(ml307r_task_handle) == eDeleted) {
            ESP_LOGW(TAG, "ML307R monitor task died, recreating...");
            xTaskCreate(ml307r_monitor_task, "ml307r_monitor", 4096, NULL, 5, &ml307r_task_handle);
        }
        
        if (status_task_handle != NULL && eTaskGetState(status_task_handle) == eDeleted) {
            ESP_LOGW(TAG, "Status monitor task died, recreating...");
            xTaskCreate(status_monitor_task, "status_monitor", 3072, NULL, 3, &status_task_handle);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10秒检查一次任务状态
    }
}
