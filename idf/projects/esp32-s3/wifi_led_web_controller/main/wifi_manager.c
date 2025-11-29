/*
 * WiFi Manager Implementation for ESP32-S3
 * 
 * WiFi连接管理和事件处理
 */

#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/event_groups.h"
#include <string.h>
#include "esp_mac.h"

#include "lwip/lwip_napt.h"
#include <stdint.h>

// 某些LWIP版本需要手动声明原型，否则会出现implicit declaration
#ifndef IP_NAPT_DECLARED
#define IP_NAPT_DECLARED
extern void ip_napt_enable(uint32_t addr, int enable);
#endif

static const char *TAG = "WIFI_MANAGER";

/* WiFi事件组 */
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_AP_STARTED_BIT BIT2

/* 全局变量 */
static esp_netif_t *s_netif_sta = NULL;
static esp_netif_t *s_netif_ap = NULL;
static bool s_wifi_connected = false;
static bool s_ap_mode = false;
static int s_retry_num = 0;
static char s_ip_string[16] = {0};
static char s_ap_ip_string[16] = {0};
static wifi_config_t s_wifi_config = {0};
static wifi_config_t s_wifi_ap_config = {0};

/* WiFi重试配置 */
#define WIFI_MAX_RETRY 5

/* WiFi事件处理器 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA started, attempting to connect...");
        esp_wifi_connect();
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP (%d/%d)", s_retry_num, WIFI_MAX_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to WiFi network: %s", (char*)s_wifi_config.sta.ssid);
        }
        s_wifi_connected = false;
        strcpy(s_ip_string, "0.0.0.0");
        
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        snprintf(s_ip_string, sizeof(s_ip_string), IPSTR, IP2STR(&event->ip_info.ip));
        
        ESP_LOGI(TAG, "WiFi connected successfully!");
        ESP_LOGI(TAG, "Device IP address: %s", s_ip_string);
        ESP_LOGI(TAG, "Web control address: http://%s", s_ip_string);
        ESP_LOGI(TAG, "Network: %s", (char*)s_wifi_config.sta.ssid);
        
        s_retry_num = 0;
        s_wifi_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        
        // 保存成功连接的WiFi配置到NVS
        char* ssid = (char*)s_wifi_config.sta.ssid;
        char* password = (char*)s_wifi_config.sta.password;
        if (strlen(ssid) > 0) {
            esp_err_t save_result = wifi_save_config(ssid, password);
            if (save_result == ESP_OK) {
                ESP_LOGI(TAG, "WiFi configuration saved to NVS: %s", ssid);
            } else {
                ESP_LOGW(TAG, "Failed to save WiFi config to NVS: %s", esp_err_to_name(save_result));
            }
        }
        
        // 启用NAPT，将AP侧客户端流量经STA侧转发到外网
        esp_netif_ip_info_t ap_ip;
        if (esp_netif_get_ip_info(s_netif_ap, &ap_ip) == ESP_OK) {
            // 将默认路由设置到STA侧
            esp_netif_set_default_netif(s_netif_sta);
            // 启用NAPT（LWIP NAT），参数为AP接口IP地址
            ip_napt_enable(ap_ip.ip.addr, 1);
            ESP_LOGI(TAG, "NAPT enabled on AP IP: " IPSTR, IP2STR(&ap_ip.ip));
        } else {
            ESP_LOGW(TAG, "Failed to get AP IP info, NAPT not enabled");
        }
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "WiFi AP started. SSID: %s", (char*)s_wifi_ap_config.ap.ssid);
        s_ap_mode = true;
        esp_netif_ip_info_t ip_info;
        ESP_ERROR_CHECK(esp_netif_get_ip_info(s_netif_ap, &ip_info));
        snprintf(s_ap_ip_string, sizeof(s_ap_ip_string), IPSTR, IP2STR(&ip_info.ip));
        ESP_LOGI(TAG, "AP IP: %s", s_ap_ip_string);
        xEventGroupSetBits(s_wifi_event_group, WIFI_AP_STARTED_BIT);
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP) {
        ESP_LOGI(TAG, "WiFi AP stopped.");
        s_ap_mode = false;
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
        ESP_LOGI(TAG, "Device connected to hotspot - NAT should provide internet access");
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" left, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

/* WiFi管理器初始化 */
esp_err_t wifi_manager_init(void)
{
    esp_err_t ret = ESP_OK;
    
    // 创建WiFi事件组
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create WiFi event group");
        return ESP_ERR_NO_MEM;
    }
    
    // 初始化TCP/IP适配器
    ESP_ERROR_CHECK(esp_netif_init());
    
    // 创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // 创建STA网络接口
    s_netif_sta = esp_netif_create_default_wifi_sta();
    if (s_netif_sta == NULL) {
        ESP_LOGE(TAG, "Failed to create STA netif");
        return ESP_FAIL;
    }
    
    // 创建AP网络接口
    s_netif_ap = esp_netif_create_default_wifi_ap();
    if (s_netif_ap == NULL) {
        ESP_LOGE(TAG, "Failed to create AP netif");
        return ESP_FAIL;
    }
    
    // 注册事件处理器
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
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 设置WiFi模式为APSTA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    
    // 配置AP
    memset(&s_wifi_ap_config, 0, sizeof(wifi_config_t));
    strcpy((char*)s_wifi_ap_config.ap.ssid, WIFI_AP_SSID);
    strcpy((char*)s_wifi_ap_config.ap.password, WIFI_AP_PASSWORD);
    s_wifi_ap_config.ap.ssid_len = strlen(WIFI_AP_SSID);
    s_wifi_ap_config.ap.channel = WIFI_AP_CHANNEL;
    s_wifi_ap_config.ap.max_connection = WIFI_AP_MAX_CONN;
    s_wifi_ap_config.ap.beacon_interval = WIFI_AP_BEACON_INTERVAL;
    s_wifi_ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    s_wifi_ap_config.ap.pairwise_cipher = WIFI_CIPHER_TYPE_NONE;
    // s_wifi_ap_config.ap.group_cipher = WIFI_CIPHER_TYPE_NONE; // 移除不存在的属性
    s_wifi_ap_config.ap.ftm_responder = false;
    s_wifi_ap_config.ap.pmf_cfg.capable = true;
    s_wifi_ap_config.ap.pmf_cfg.required = false;
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &s_wifi_ap_config));
    
    // 启动WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // 启动AP（简化版本，不使用单独的启动函数）
    // ESP_ERROR_CHECK(esp_wifi_start_ap());
    
    ESP_LOGI(TAG, "WiFi manager initialized");
    ESP_LOGI(TAG, "AP SSID: %s, Password: %s", WIFI_AP_SSID, WIFI_AP_PASSWORD);
    
    return ret;
}

/* 连接到WiFi网络 */
esp_err_t wifi_connect_sta(const char *ssid, const char *password)
{
    if (ssid == NULL || password == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(&s_wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char*)s_wifi_config.sta.ssid, ssid);
    strcpy((char*)s_wifi_config.sta.password, password);
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &s_wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    ESP_LOGI(TAG, "Attempting to connect to WiFi: %s", ssid);
    
    return ESP_OK;
}

/* 断开WiFi连接 */
esp_err_t wifi_disconnect_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    s_wifi_connected = false;
    strcpy(s_ip_string, "0.0.0.0");
    return ESP_OK;
}

/* 启动AP模式 */
esp_err_t wifi_start_ap(void)
{
    // 简化版本，AP已经在初始化时启动
    ESP_LOGI(TAG, "AP mode already started");
    return ESP_OK;
}

/* 停止AP模式 */
esp_err_t wifi_stop_ap(void)
{
    // 简化版本，不停止AP
    ESP_LOGI(TAG, "AP mode stop not implemented");
    s_ap_mode = false;
    return ESP_OK;
}

/* 工厂重置 */
esp_err_t wifi_factory_reset(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_erase_all(nvs_handle);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "WiFi configuration cleared from NVS");
    }
    
    // 断开当前连接
    wifi_disconnect_sta();
    
    return ESP_OK;
}

/* 检查WiFi连接状态 */
bool wifi_is_connected(void)
{
    return s_wifi_connected;
}

/* 检查AP模式状态 */
bool wifi_is_ap_mode(void)
{
    return s_ap_mode;
}

/* 获取IP地址字符串 */
const char* wifi_get_ip_string(void)
{
    return s_ip_string;
}

/* 获取当前WiFi模式 */
wifi_mode_t wifi_get_current_mode(void)
{
    if (s_wifi_connected && s_ap_mode) {
        return WIFI_MODE_APSTA;
    } else if (s_wifi_connected) {
        return WIFI_MODE_STA;
    } else if (s_ap_mode) {
        return WIFI_MODE_AP;
    } else {
        return WIFI_MODE_AP; // 默认AP模式
    }
}

/* 保存WiFi配置到NVS */
esp_err_t wifi_save_config(const char *ssid, const char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    err = nvs_set_str(nvs_handle, NVS_KEY_SSID, ssid);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving SSID to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    err = nvs_set_str(nvs_handle, NVS_KEY_PASSWORD, password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving password to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "WiFi configuration saved to NVS");
    }
    
    return err;
}

/* 从NVS加载WiFi配置 */
esp_err_t wifi_load_config(char *ssid, char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    size_t ssid_len = 32;
    size_t password_len = 64;
    
    err = nvs_get_str(nvs_handle, NVS_KEY_SSID, ssid, &ssid_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading SSID from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    err = nvs_get_str(nvs_handle, NVS_KEY_PASSWORD, password, &password_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading password from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "WiFi configuration loaded from NVS: %s", ssid);
    
    return ESP_OK;
}
