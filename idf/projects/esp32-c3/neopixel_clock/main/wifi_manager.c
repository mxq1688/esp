/**
 * @file wifi_manager.c
 * @brief WiFi connection and NTP time synchronization implementation
 */

#include "wifi_manager.h"
#include "captive_portal.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>

static const char *TAG = "wifi_manager";

// Event group for WiFi status
static EventGroupHandle_t s_wifi_event_group = NULL;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;
#define MAX_RETRY_COUNT 5

static bool s_time_synced = false;
static bool s_wifi_initialized = false;
static esp_netif_t *s_sta_netif = NULL;

// Current WiFi credentials
static char s_current_ssid[33] = {0};
static char s_current_password[65] = {0};

// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        if (strlen(s_current_ssid) > 0) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "WiFi started, connecting to %s...", s_current_ssid);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "Disconnected from WiFi, reason: %d", event->reason);
        
        if (s_retry_num < MAX_RETRY_COUNT) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry connecting to WiFi (%d/%d)", s_retry_num, MAX_RETRY_COUNT);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to WiFi after %d attempts", MAX_RETRY_COUNT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
}

// SNTP time sync notification callback
static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Time synchronized with NTP server");
    s_time_synced = true;
}

esp_err_t wifi_manager_init(void)
{
    if (s_wifi_initialized) {
        ESP_LOGW(TAG, "WiFi already initialized");
        return ESP_OK;
    }

    esp_err_t ret;

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create event group
    if (s_wifi_event_group == NULL) {
        s_wifi_event_group = xEventGroupCreate();
    }

    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    if (s_sta_netif == NULL) {
        s_sta_netif = esp_netif_create_default_wifi_sta();
    }

    // Initialize WiFi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi initialization finished");

    return ESP_OK;
}

esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    if (!s_wifi_initialized) {
        ESP_LOGE(TAG, "WiFi not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!ssid || strlen(ssid) == 0) {
        ESP_LOGE(TAG, "Invalid SSID");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);

    // Store credentials
    strncpy(s_current_ssid, ssid, sizeof(s_current_ssid) - 1);
    if (password) {
        strncpy(s_current_password, password, sizeof(s_current_password) - 1);
    } else {
        s_current_password[0] = '\0';
    }

    // Reset retry counter and event bits
    s_retry_num = 0;
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    // Disconnect if already connected
    esp_wifi_disconnect();

    // Configure WiFi
    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (password && strlen(password) > 0) {
        strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();

    return ESP_OK;
}

esp_err_t wifi_manager_wait_connected(uint32_t timeout_ms)
{
    if (!s_wifi_initialized || s_wifi_event_group == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(timeout_ms));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi successfully");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to WiFi");
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "WiFi connection timeout");
        return ESP_ERR_TIMEOUT;
    }
}

bool wifi_is_connected(void)
{
    if (s_wifi_event_group == NULL) {
        return false;
    }

    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

esp_err_t wifi_manager_disconnect(void)
{
    if (!s_wifi_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_wifi_disconnect();
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    s_current_ssid[0] = '\0';
    s_current_password[0] = '\0';
    
    return ESP_OK;
}

esp_err_t wifi_manager_get_ip(char *ip_str, size_t len)
{
    if (!ip_str || len < 16) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(s_sta_netif, &ip_info) == ESP_OK) {
        snprintf(ip_str, len, IPSTR, IP2STR(&ip_info.ip));
        return ESP_OK;
    }
    
    return ESP_FAIL;
}

esp_err_t ntp_sync_time(void)
{
    static bool sntp_initialized = false;
    
    ESP_LOGI(TAG, "Initializing SNTP");

    // Set timezone for China (GMT+8) - 先设置时区
    setenv("TZ", "CST-8", 1);
    tzset();

    // 如果已经初始化过，只需等待同步
    if (sntp_initialized) {
        // 检查是否已经同步过
        if (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            char strftime_buf[64];
            strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
            ESP_LOGI(TAG, "Time already synced: %s", strftime_buf);
            return ESP_OK;
        }
        
        // 等待同步完成
        ESP_LOGI(TAG, "Waiting for time synchronization...");
        int retry = 0;
        const int retry_count = 30;
        while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
            ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        if (retry >= retry_count) {
            ESP_LOGW(TAG, "Sync timeout, restarting SNTP...");
            esp_sntp_stop();
            sntp_initialized = false;
            // 继续下面的初始化流程
        } else {
            goto sync_done;
        }
    }

    // 首次初始化
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    
    // 使用国内可访问的 NTP 服务器
    esp_sntp_setservername(0, "ntp.aliyun.com");         // 阿里云 NTP
    esp_sntp_setservername(1, "ntp.tencent.com");        // 腾讯云 NTP
    esp_sntp_setservername(2, "ntp.ntsc.ac.cn");         // 中科院国家授时中心
    
    esp_sntp_init();
    sntp_initialized = true;

    ESP_LOGI(TAG, "Waiting for time synchronization...");

    // Wait for time to be set (with timeout)
    int retry = 0;
    const int retry_count = 30; // 30 seconds timeout
    while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (retry >= retry_count) {
        ESP_LOGE(TAG, "Failed to synchronize time after %d seconds", retry_count);
        return ESP_ERR_TIMEOUT;
    }

sync_done:
    // Get and display current time
    {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        ESP_LOGI(TAG, "Current time: %s", strftime_buf);
    }

    return ESP_OK;
}

bool is_time_synced(void)
{
    return s_time_synced;
}
