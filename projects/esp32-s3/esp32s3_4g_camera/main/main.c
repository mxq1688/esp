#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "include/camera_driver.h"
#include "include/ml307r_driver.h"
#include "include/image_processor.h"
#include "include/web_server.h"

static const char *TAG = "MAIN";

// WiFi配置
#define WIFI_AP_SSID      "ESP32-S3-Camera"
#define WIFI_AP_PASSWORD  "12345678"
#define WIFI_AP_CHANNEL   1
#define WIFI_AP_MAX_CONN  4

// 任务句柄
static TaskHandle_t ml307r_task_handle = NULL;
static TaskHandle_t camera_task_handle = NULL;
static TaskHandle_t status_task_handle = NULL;

// WiFi事件处理器
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "Station "MACSTR" left, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

// 初始化WiFi AP模式
static esp_err_t wifi_init_ap(void)
{
    ESP_LOGI(TAG, "Initializing WiFi AP mode...");

    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    // WiFi初始化配置
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件处理器
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    // WiFi配置
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = WIFI_AP_CHANNEL,
            .password = WIFI_AP_PASSWORD,
            .max_connection = WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    // 如果密码为空，使用开放模式
    if (strlen(WIFI_AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "✅ WiFi AP initialized");
    ESP_LOGI(TAG, "SSID: %s", WIFI_AP_SSID);
    ESP_LOGI(TAG, "Password: %s", WIFI_AP_PASSWORD);
    ESP_LOGI(TAG, "IP: 192.168.4.1");

    return ESP_OK;
}

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
                ESP_LOGI(TAG, "📡 Network: %s, Signal: %d dBm, Connected: %s", 
                         network_info.operator_name, 
                         network_info.signal_strength,
                         network_info.is_connected ? "Yes" : "No");
            }
        } else {
            ESP_LOGD(TAG, "ML307R not ready, state: %d", ml307r_get_state());
        }
        
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30秒检查一次
    }
}

// 摄像头监控任务
static void camera_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Camera monitor task started");
    
    while (1) {
        if (camera_driver_is_ready()) {
            // 定期采集一帧图像以保持摄像头活跃
            camera_fb_t *fb = camera_driver_capture();
            if (fb != NULL) {
                ESP_LOGD(TAG, "📷 Camera test capture: %zu bytes", fb->len);
                camera_driver_release_frame(fb);
            }
        } else {
            ESP_LOGW(TAG, "Camera not ready, state: %d", camera_driver_get_state());
        }
        
        vTaskDelay(pdMS_TO_TICKS(60000)); // 60秒检查一次
    }
}

// 系统状态监控任务
static void status_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Status monitor task started");
    
    while (1) {
        // 打印系统状态
        ESP_LOGI(TAG, "=== 系统状态 ===");
        ESP_LOGI(TAG, "可用内存: %lu bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "摄像头状态: %d", camera_driver_get_state());
        ESP_LOGI(TAG, "ML307R状态: %d", ml307r_get_state());
        ESP_LOGI(TAG, "Web服务器: %s", web_server_is_running() ? "运行中" : "已停止");
        
        vTaskDelay(pdMS_TO_TICKS(60000)); // 60秒打印一次状态
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "ESP32-S3 4G 远程摄像头系统");
    ESP_LOGI(TAG, "版本: 1.0.0");
    ESP_LOGI(TAG, "=================================");

    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "✅ NVS初始化完成");

    // 初始化WiFi AP模式
    ret = wifi_init_ap();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ WiFi AP初始化失败: %s", esp_err_to_name(ret));
        return;
    }

    // 初始化摄像头
    ESP_LOGI(TAG, "正在初始化摄像头...");
    ret = camera_driver_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ 摄像头初始化失败: %s", esp_err_to_name(ret));
        ESP_LOGW(TAG, "⚠️  系统将继续运行，但摄像头功能不可用");
    } else {
        ESP_LOGI(TAG, "✅ 摄像头初始化成功");
    }

    // 初始化图像处理器
    ret = image_processor_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ 图像处理器初始化失败: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "✅ 图像处理器初始化成功");
    }

    // 启动Web服务器
    ret = web_server_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ Web服务器启动失败: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "✅ Web服务器已启动");

    // 初始化ML307R模块（可选，如果没有4G模块则跳过）
    ESP_LOGI(TAG, "正在初始化ML307R 4G模块...");
    ret = ml307r_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "⚠️  ML307R初始化返回: %s", esp_err_to_name(ret));
        ESP_LOGW(TAG, "⚠️  系统将继续运行，但4G功能可能不可用");
    } else {
        ESP_LOGI(TAG, "✅ ML307R模块初始化成功");
        
        // 尝试建立4G数据连接
        ESP_LOGI(TAG, "正在建立4G数据连接...");
        ret = ml307r_establish_data_connection();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "⚠️  4G数据连接建立失败: %s", esp_err_to_name(ret));
        } else {
            ESP_LOGI(TAG, "✅ 4G数据连接已建立");
        }
    }

    // 创建监控任务
    xTaskCreatePinnedToCore(ml307r_monitor_task, "ml307r_monitor", 4096, NULL, 5, &ml307r_task_handle, 0);
    ESP_LOGI(TAG, "✅ ML307R监控任务已创建");

    xTaskCreatePinnedToCore(camera_monitor_task, "camera_monitor", 4096, NULL, 5, &camera_task_handle, 1);
    ESP_LOGI(TAG, "✅ 摄像头监控任务已创建");

    xTaskCreatePinnedToCore(status_monitor_task, "status_monitor", 3072, NULL, 3, &status_task_handle, 0);
    ESP_LOGI(TAG, "✅ 状态监控任务已创建");

    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "🚀 系统初始化完成！");
    ESP_LOGI(TAG, "📱 访问Web界面: http://192.168.4.1");
    ESP_LOGI(TAG, "📶 WiFi热点: %s", WIFI_AP_SSID);
    ESP_LOGI(TAG, "🔑 密码: %s", WIFI_AP_PASSWORD);
    ESP_LOGI(TAG, "=================================");

    // 主循环 - 监控任务健康状态
    while (1) {
        // 检查任务状态
        if (ml307r_task_handle != NULL && eTaskGetState(ml307r_task_handle) == eDeleted) {
            ESP_LOGW(TAG, "ML307R监控任务已终止，正在重新创建...");
            xTaskCreatePinnedToCore(ml307r_monitor_task, "ml307r_monitor", 4096, NULL, 5, &ml307r_task_handle, 0);
        }
        
        if (camera_task_handle != NULL && eTaskGetState(camera_task_handle) == eDeleted) {
            ESP_LOGW(TAG, "摄像头监控任务已终止，正在重新创建...");
            xTaskCreatePinnedToCore(camera_monitor_task, "camera_monitor", 4096, NULL, 5, &camera_task_handle, 1);
        }
        
        if (status_task_handle != NULL && eTaskGetState(status_task_handle) == eDeleted) {
            ESP_LOGW(TAG, "状态监控任务已终止，正在重新创建...");
            xTaskCreatePinnedToCore(status_monitor_task, "status_monitor", 3072, NULL, 3, &status_task_handle, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10秒检查一次任务状态
    }
}

