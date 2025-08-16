#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "WIFI_MANAGER";

// WiFi事件组位
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// WiFi配置
static wifi_config_t wifi_config = {
    .ssid = "",
    .password = "",
    .max_retry = 5,
    .enable_ap = false,
    .ap_ssid = "AI-Assistant",
    .ap_password = "12345678"
};

// 全局变量
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static wifi_status_t current_status = WIFI_DISCONNECTED;
static char current_ip[16] = "0.0.0.0";
static wifi_event_callback_t event_callback = NULL;

// WiFi事件处理函数
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi站点模式启动");
        current_status = WIFI_CONNECTING;
        if (event_callback) {
            event_callback(current_status);
        }
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi连接断开");
        current_status = WIFI_DISCONNECTED;
        if (event_callback) {
            event_callback(current_status);
        }
        if (s_retry_num < wifi_config.max_retry) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "重试连接到AP，第%d次", s_retry_num);
        } else {
            ESP_LOGE(TAG, "连接AP失败，已达到最大重试次数");
            current_status = WIFI_FAILED;
            if (event_callback) {
                event_callback(current_status);
            }
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "获取到IP地址:" IPSTR, IP2STR(&event->ip_info.ip));
        snprintf(current_ip, sizeof(current_ip), IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        current_status = WIFI_CONNECTED;
        if (event_callback) {
            event_callback(current_status);
        }
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_init(void)
{
    ESP_LOGI(TAG, "初始化WiFi管理器");
    
    // 创建事件组
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "事件组创建失败");
        return ESP_FAIL;
    }
    
    // 初始化TCP/IP适配器
    ESP_ERROR_CHECK(esp_netif_init());
    
    // 创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // 创建默认网络接口
    esp_netif_create_default_wifi_sta();
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册事件处理器
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));
    
    // 设置WiFi模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    // 启动WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi管理器初始化完成");
    
    return ESP_OK;
}

esp_err_t wifi_connect(const char *ssid, const char *password)
{
    if (ssid == NULL || password == NULL) {
        ESP_LOGE(TAG, "SSID或密码为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "连接到WiFi: %s", ssid);
    
    // 配置WiFi
    wifi_config_t wifi_config_local = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    strcpy((char*)wifi_config_local.sta.ssid, ssid);
    strcpy((char*)wifi_config_local.sta.password, password);
    
    // 设置WiFi配置
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_local));
    
    // 清除事件位
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    
    // 开始连接
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    // 等待连接结果
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(10000));
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi连接成功");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "WiFi连接失败");
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "WiFi连接超时");
        return ESP_ERR_TIMEOUT;
    }
}

esp_err_t wifi_disconnect(void)
{
    ESP_LOGI(TAG, "断开WiFi连接");
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    current_status = WIFI_DISCONNECTED;
    if (event_callback) {
        event_callback(current_status);
    }
    return ESP_OK;
}

esp_err_t wifi_reconnect(void)
{
    ESP_LOGI(TAG, "重新连接WiFi");
    s_retry_num = 0;
    return esp_wifi_connect();
}

bool wifi_is_connected(void)
{
    return current_status == WIFI_CONNECTED;
}

wifi_status_t wifi_get_status(void)
{
    return current_status;
}

const char* wifi_get_ip(void)
{
    return current_ip;
}

esp_err_t wifi_set_config(wifi_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&wifi_config, config, sizeof(wifi_config_t));
    ESP_LOGI(TAG, "WiFi配置已更新");
    
    return ESP_OK;
}

esp_err_t wifi_start_ap(const char *ssid, const char *password)
{
    if (ssid == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "启动WiFi热点: %s", ssid);
    
    // 配置AP
    wifi_config_t wifi_config_local = {
        .ap = {
            .ssid = "",
            .ssid_len = 0,
            .channel = 1,
            .password = "",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false
            },
        },
    };
    
    strcpy((char*)wifi_config_local.ap.ssid, ssid);
    wifi_config_local.ap.ssid_len = strlen(ssid);
    
    if (password != NULL && strlen(password) >= 8) {
        strcpy((char*)wifi_config_local.ap.password, password);
    } else {
        wifi_config_local.ap.authmode = WIFI_AUTH_OPEN;
    }
    
    // 设置WiFi模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    
    // 设置AP配置
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_local));
    
    ESP_LOGI(TAG, "WiFi热点已启动");
    return ESP_OK;
}

esp_err_t wifi_stop_ap(void)
{
    ESP_LOGI(TAG, "停止WiFi热点");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    return ESP_OK;
}

esp_err_t wifi_set_event_callback(wifi_event_callback_t callback)
{
    event_callback = callback;
    ESP_LOGI(TAG, "WiFi事件回调已设置");
    return ESP_OK;
}
