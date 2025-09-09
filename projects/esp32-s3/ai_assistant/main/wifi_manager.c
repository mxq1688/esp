/**
 * @file wifi_manager.c
 * @brief WiFi Manager Implementation for ESP32-S3 AI Assistant
 * @author AI Assistant Team
 * @version 1.0.0
 */

#include "wifi_manager.h"
#include "ai_assistant.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"

static const char *TAG = "WIFI_MANAGER";

// WiFi事件组
static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int WIFI_FAIL_BIT = BIT1;

// WiFi状态变量
static wifi_manager_state_t s_wifi_state = WIFI_STATE_DISCONNECTED;
static wifi_manager_config_t s_wifi_config = {
    .ssid = "AI_Assistant",
    .password = "12345678",
    .auth_mode = WIFI_AUTH_WPA2_PSK,
    .max_retry = WIFI_MAXIMUM_RETRY
};
static int s_retry_num = 0;
static esp_netif_t *s_sta_netif = NULL;

/**
 * @brief WiFi事件处理器
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        s_wifi_state = WIFI_STATE_CONNECTING;
        ai_assistant_log_info(TAG, "WiFi station started, connecting...");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < s_wifi_config.max_retry) {
            esp_wifi_connect();
            s_retry_num++;
            s_wifi_state = WIFI_STATE_CONNECTING;
            ai_assistant_log_info(TAG, "Retry to connect to the AP (attempt %d/%d)", 
                                s_retry_num, s_wifi_config.max_retry);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            s_wifi_state = WIFI_STATE_ERROR;
            ai_assistant_log_error(TAG, "Failed to connect to WiFi after %d attempts", s_wifi_config.max_retry);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ai_assistant_log_info(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        s_wifi_state = WIFI_STATE_CONNECTED;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(ai_event_group, AI_WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief 初始化WiFi管理器
 */
esp_err_t wifi_manager_init(void)
{
    ai_assistant_log_info(TAG, "Initializing WiFi Manager");
    
    // 创建事件组
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ai_assistant_log_error(TAG, "Failed to create WiFi event group");
        return ESP_ERR_NO_MEM;
    }
    
    // 创建默认WiFi STA
    s_sta_netif = esp_netif_create_default_wifi_sta();
    if (s_sta_netif == NULL) {
        ai_assistant_log_error(TAG, "Failed to create default WiFi STA");
        return ESP_FAIL;
    }
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册事件处理器
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    ai_assistant_log_info(TAG, "WiFi Manager initialized successfully");
    return ESP_OK;
}

/**
 * @brief 启动WiFi管理器
 */
esp_err_t wifi_manager_start(void)
{
    ai_assistant_log_info(TAG, "Starting WiFi Manager");
    
    // 配置WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = s_wifi_config.auth_mode,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    // 复制SSID和密码
    strncpy((char*)wifi_config.sta.ssid, s_wifi_config.ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, s_wifi_config.password, sizeof(wifi_config.sta.password) - 1);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ai_assistant_log_info(TAG, "WiFi started, connecting to SSID: %s", s_wifi_config.ssid);
    
    // 等待连接结果
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ai_assistant_log_info(TAG, "Connected to WiFi SSID: %s", s_wifi_config.ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ai_assistant_log_error(TAG, "Failed to connect to WiFi SSID: %s", s_wifi_config.ssid);
        return ESP_FAIL;
    } else {
        ai_assistant_log_error(TAG, "Unexpected WiFi event");
        return ESP_ERR_TIMEOUT;
    }
}

/**
 * @brief 停止WiFi管理器
 */
esp_err_t wifi_manager_stop(void)
{
    ai_assistant_log_info(TAG, "Stopping WiFi Manager");
    
    ESP_ERROR_CHECK(esp_wifi_stop());
    s_wifi_state = WIFI_STATE_DISCONNECTED;
    
    ai_assistant_log_info(TAG, "WiFi Manager stopped");
    return ESP_OK;
}

/**
 * @brief 扫描WiFi网络
 */
esp_err_t wifi_manager_scan(wifi_ap_record_t *ap_records, uint16_t *ap_count)
{
    if (ap_records == NULL || ap_count == NULL) {
        ai_assistant_log_error(TAG, "Invalid parameters for WiFi scan");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Starting WiFi scan");
    
    // 启动扫描
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    
    // 获取扫描结果
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(ap_count, ap_records));
    
    ai_assistant_log_info(TAG, "WiFi scan completed, found %d networks", *ap_count);
    return ESP_OK;
}

/**
 * @brief 连接到指定WiFi网络
 */
esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    if (ssid == NULL) {
        ai_assistant_log_error(TAG, "SSID cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Connecting to WiFi: %s", ssid);
    
    // 更新配置
    strncpy(s_wifi_config.ssid, ssid, sizeof(s_wifi_config.ssid) - 1);
    if (password != NULL) {
        strncpy(s_wifi_config.password, password, sizeof(s_wifi_config.password) - 1);
    } else {
        s_wifi_config.password[0] = '\0';
    }
    
    // 断开当前连接
    if (s_wifi_state == WIFI_STATE_CONNECTED) {
        esp_wifi_disconnect();
    }
    
    // 重新启动连接
    return wifi_manager_start();
}

/**
 * @brief 断开WiFi连接
 */
esp_err_t wifi_manager_disconnect(void)
{
    ai_assistant_log_info(TAG, "Disconnecting from WiFi");
    
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    s_wifi_state = WIFI_STATE_DISCONNECTED;
    
    ai_assistant_log_info(TAG, "WiFi disconnected");
    return ESP_OK;
}

/**
 * @brief 获取WiFi状态
 */
wifi_manager_state_t wifi_manager_get_state(void)
{
    return s_wifi_state;
}

/**
 * @brief 获取IP信息
 */
esp_err_t wifi_manager_get_ip_info(esp_netif_ip_info_t *ip_info)
{
    if (ip_info == NULL) {
        ai_assistant_log_error(TAG, "IP info pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_sta_netif == NULL) {
        ai_assistant_log_error(TAG, "WiFi STA interface not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    return esp_netif_get_ip_info(s_sta_netif, ip_info);
}

/**
 * @brief 设置WiFi配置
 */
esp_err_t wifi_manager_set_config(const wifi_manager_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_wifi_config, config, sizeof(wifi_manager_config_t));
    ai_assistant_log_info(TAG, "WiFi configuration updated");
    return ESP_OK;
}

/**
 * @brief 获取WiFi配置
 */
esp_err_t wifi_manager_get_config(wifi_manager_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_wifi_config, sizeof(wifi_manager_config_t));
    return ESP_OK;
}
