/*
 * WiFi Manager Implementation for ESP32-C3
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

static const char *TAG = "WIFI_MANAGER";

/* WiFi事件组 */
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_AP_STARTED_BIT BIT2

/* 全局变量 */
static esp_netif_t *s_netif_sta = NULL;
static esp_netif_t *s_netif_ap = NULL; // Add for AP interface
static wifi_state_t s_wifi_state = WIFI_STATE_DISCONNECTED;
static int s_retry_num = 0;
static wifi_event_callback_t s_event_callback = NULL;
static char s_ip_string[16] = {0}; // STA IP
static char s_ap_ip_string[16] = {0}; // AP IP
static wifi_config_t s_wifi_config = {0};
static wifi_config_t s_wifi_ap_config = {0}; // Add for AP config

/* NVS WiFi配置键 */
#define NVS_NAMESPACE "wifi_config"
#define NVS_KEY_SSID "ssid"
#define NVS_KEY_PASSWORD "password"
#define NVS_KEY_CONFIGURED "configured"
#define NVS_KEY_AP_SSID "ap_ssid"
#define NVS_KEY_AP_PASSWORD "ap_password"

/* 前向声明 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data);
static esp_err_t wifi_save_config_to_nvs(const char* ssid, const char* password);
static esp_err_t wifi_load_config_from_nvs(char* ssid, char* password);
static esp_err_t wifi_save_ap_config_to_nvs(const char* ssid, const char* password);
static esp_err_t wifi_load_ap_config_from_nvs(char* ssid, char* password);

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
            ESP_LOGE(TAG, "❌ Failed to connect to WiFi network: %s", (char*)s_wifi_config.sta.ssid);
            ESP_LOGE(TAG, "Please check network name and password!");
        }
        s_wifi_state = WIFI_STATE_DISCONNECTED;
        strcpy(s_ip_string, "0.0.0.0");
        
        if (s_event_callback) {
            s_event_callback(WIFI_MANAGER_EVENT_STA_DISCONNECTED, event_data);
        }
        
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        snprintf(s_ip_string, sizeof(s_ip_string), IPSTR, IP2STR(&event->ip_info.ip));
        
        // 打印超级显眼的IP地址信息
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉");
        ESP_LOGI(TAG, "🎉                                          🎉");
        ESP_LOGI(TAG, "🎉    ✅ WiFi连接成功！                     🎉");
        ESP_LOGI(TAG, "🎉    📱 设备IP地址: %s           🎉", s_ip_string);
        ESP_LOGI(TAG, "🎉    🌐 Web控制地址: http://%s      🎉", s_ip_string);
        ESP_LOGI(TAG, "🎉    📶 网络: %s                   🎉", (char*)s_wifi_config.sta.ssid);
        ESP_LOGI(TAG, "🎉                                          🎉");
        ESP_LOGI(TAG, "🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉");
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "💡 请在Web界面中输入IP地址: %s", s_ip_string);
        ESP_LOGI(TAG, "");
        
        s_retry_num = 0;
        s_wifi_state = WIFI_STATE_CONNECTED;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        
        // 保存成功连接的WiFi配置到NVS
        char* ssid = (char*)s_wifi_config.sta.ssid;
        char* password = (char*)s_wifi_config.sta.password;
        if (strlen(ssid) > 0) {
            esp_err_t save_result = wifi_save_config_to_nvs(ssid, password);
            if (save_result == ESP_OK) {
                ESP_LOGI(TAG, "✅ WiFi configuration saved to NVS: %s", ssid);
            } else {
                ESP_LOGW(TAG, "Failed to save WiFi config to NVS: %s", esp_err_to_name(save_result));
            }
        }
        
        if (s_event_callback) {
            s_event_callback(WIFI_MANAGER_EVENT_STA_CONNECTED, event_data);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "WiFi AP started. SSID: %s", (char*)s_wifi_ap_config.ap.ssid);
        s_wifi_state = WIFI_STATE_AP_MODE;
        esp_netif_ip_info_t ip_info;
        ESP_ERROR_CHECK(esp_netif_get_ip_info(s_netif_ap, &ip_info));
        snprintf(s_ap_ip_string, sizeof(s_ap_ip_string), IPSTR, IP2STR(&ip_info.ip));
        ESP_LOGI(TAG, "AP IP: %s", s_ap_ip_string);
        xEventGroupSetBits(s_wifi_event_group, WIFI_AP_STARTED_BIT);
        if (s_event_callback) {
            s_event_callback(WIFI_MANAGER_EVENT_AP_STARTED, event_data);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP) {
        ESP_LOGI(TAG, "WiFi AP stopped.");
        if (s_wifi_state == WIFI_STATE_AP_MODE || s_wifi_state == WIFI_STATE_AP_STA_MODE) {
            s_wifi_state = WIFI_STATE_DISCONNECTED; // Or WIFI_STATE_CONNECTED if STA is still active
        }
        if (s_event_callback) {
            s_event_callback(WIFI_MANAGER_EVENT_AP_STOPPED, event_data);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
        if (s_event_callback) {
            s_event_callback(WIFI_MANAGER_EVENT_STA_JOINED, event_data);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" left, AID=%d",
                 MAC2STR(event->mac), event->aid);
        if (s_event_callback) {
            s_event_callback(WIFI_MANAGER_EVENT_STA_LEFT, event_data);
        }
    }
}

/* 保存WiFi配置到NVS */
static esp_err_t wifi_save_config_to_nvs(const char* ssid, const char* password)
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
    
    err = nvs_set_u8(nvs_handle, NVS_KEY_CONFIGURED, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving configured flag to NVS: %s", esp_err_to_name(err));
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
static esp_err_t wifi_load_config_from_nvs(char* ssid, char* password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS handle not found, using default config");
        return err;
    }
    
    uint8_t configured = 0;
    err = nvs_get_u8(nvs_handle, NVS_KEY_CONFIGURED, &configured);
    if (err != ESP_OK || configured != 1) {
        ESP_LOGW(TAG, "WiFi not configured in NVS");
        nvs_close(nvs_handle);
        return ESP_ERR_NOT_FOUND;
    }
    
    size_t ssid_len = WIFI_SSID_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_KEY_SSID, ssid, &ssid_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error loading SSID from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    size_t password_len = WIFI_PASSWORD_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_KEY_PASSWORD, password, &password_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error loading password from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "WiFi configuration loaded from NVS: SSID=%s", ssid);
    return ESP_OK;
}

/* 保存AP WiFi配置到NVS */
static esp_err_t wifi_save_ap_config_to_nvs(const char* ssid, const char* password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }
    
    err = nvs_set_str(nvs_handle, NVS_KEY_AP_SSID, ssid);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving AP SSID to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    err = nvs_set_str(nvs_handle, NVS_KEY_AP_PASSWORD, password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving AP password to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "AP WiFi configuration saved to NVS");
    }
    
    return err;
}

/* 从NVS加载AP WiFi配置 */
static esp_err_t wifi_load_ap_config_from_nvs(char* ssid, char* password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS handle not found for AP config, using default config");
        return err;
    }
    
    size_t ssid_len = WIFI_SSID_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_KEY_AP_SSID, ssid, &ssid_len);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error loading AP SSID from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    size_t password_len = WIFI_PASSWORD_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_KEY_AP_PASSWORD, password, &password_len);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error loading AP password from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "AP WiFi configuration loaded from NVS: SSID=%s", ssid);
    return ESP_OK;
}

/* 公共函数实现 */

esp_err_t wifi_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing WiFi manager...");
    
    // 创建事件组
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create WiFi event group");
        return ESP_FAIL;
    }
    
    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // 创建网络接口 - STA和AP模式
    s_netif_sta = esp_netif_create_default_wifi_sta();
    s_netif_ap = esp_netif_create_default_wifi_ap();
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册事件处理器
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                              &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                              &wifi_event_handler, NULL));
    
    // 设置WiFi模式 - AP+STA模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    
    // 配置STA模式
    char sta_ssid[WIFI_SSID_MAX_LEN] = {0};
    char sta_password[WIFI_PASSWORD_MAX_LEN] = {0};

    if (wifi_load_config_from_nvs(sta_ssid, sta_password) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded STA config from NVS. SSID: %s", sta_ssid);
        strncpy((char*)s_wifi_config.sta.ssid, sta_ssid, sizeof(s_wifi_config.sta.ssid) - 1);
        strncpy((char*)s_wifi_config.sta.password, sta_password, sizeof(s_wifi_config.sta.password) - 1);
        s_wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &s_wifi_config));
        ESP_LOGI(TAG, "🔗 Connecting to WiFi network: %s", (char*)s_wifi_config.sta.ssid);
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_LOGI(TAG, "✅ WiFi connection initiated - waiting for IP address...");
    } else {
        ESP_LOGW(TAG, "No STA config found in NVS, starting AP only.");
        // 尝试启动AP模式，如果NVS中没有STA配置
        wifi_start_ap(ESP_WIFI_AP_SSID, ESP_WIFI_AP_PASS);
    }

    // 配置AP模式 (使用默认值或从NVS加载)
    char ap_ssid[WIFI_SSID_MAX_LEN] = {0};
    char ap_password[WIFI_PASSWORD_MAX_LEN] = {0};

    if (wifi_load_ap_config_from_nvs(ap_ssid, ap_password) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded AP config from NVS. SSID: %s", ap_ssid);
        strncpy((char*)s_wifi_ap_config.ap.ssid, ap_ssid, sizeof(s_wifi_ap_config.ap.ssid) - 1);
        strncpy((char*)s_wifi_ap_config.ap.password, ap_password, sizeof(s_wifi_ap_config.ap.password) - 1);
    } else {
        ESP_LOGW(TAG, "No AP config found in NVS, using default AP settings.");
        strncpy((char*)s_wifi_ap_config.ap.ssid, ESP_WIFI_AP_SSID, sizeof(s_wifi_ap_config.ap.ssid) - 1);
        strncpy((char*)s_wifi_ap_config.ap.password, ESP_WIFI_AP_PASS, sizeof(s_wifi_ap_config.ap.password) - 1);
    }

    s_wifi_ap_config.ap.channel = ESP_WIFI_AP_CHANNEL;
    s_wifi_ap_config.ap.max_connection = ESP_WIFI_AP_MAX_STA;
    s_wifi_ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &s_wifi_ap_config));
    
    // For dual mode, start STA first then AP, or just AP if STA config fails
    // The actual start of AP is moved to wifi_start_ap function to allow explicit control

    ESP_LOGI(TAG, "WiFi manager initialized successfully");
    return ESP_OK;
}

esp_err_t wifi_connect_sta(const char* ssid, const char* password, bool save_to_nvs)
{
    if (ssid == NULL || strlen(ssid) == 0) {
        ESP_LOGE(TAG, "Invalid SSID");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);
    
    // 重置重试计数
    s_retry_num = 0;
    s_wifi_state = WIFI_STATE_CONNECTING;
    
    // 配置STA模式
    memset(&s_wifi_config, 0, sizeof(s_wifi_config));
    strncpy((char*)s_wifi_config.sta.ssid, ssid, sizeof(s_wifi_config.sta.ssid) - 1);
    if (password && strlen(password) > 0) {
        strncpy((char*)s_wifi_config.sta.password, password, sizeof(s_wifi_config.sta.password) - 1);
    }
    s_wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &s_wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // 保存配置到NVS
    if (save_to_nvs) {
        wifi_save_config_to_nvs(ssid, password ? password : "");
    }
    
    return ESP_OK;
}

esp_err_t wifi_start_ap(const char* ssid, const char* password)
{
    if (ssid == NULL || strlen(ssid) == 0) {
        ESP_LOGE(TAG, "Invalid AP SSID");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Starting AP: %s", ssid);

    strncpy((char*)s_wifi_ap_config.ap.ssid, ssid, sizeof(s_wifi_ap_config.ap.ssid) - 1);
    if (password && strlen(password) > 0) {
        strncpy((char*)s_wifi_ap_config.ap.password, password, sizeof(s_wifi_ap_config.ap.password) - 1);
    } else {
        // If no password, set open authentication
        s_wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &s_wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_start()); // Start both AP and STA

    // Save AP config to NVS
    wifi_save_ap_config_to_nvs(ssid, password ? password : "");

    s_wifi_state = WIFI_STATE_AP_MODE; // This might be overridden if STA connects later
    return ESP_OK;
}

esp_err_t wifi_disconnect(void)
{
    ESP_LOGI(TAG, "Disconnecting WiFi...");
    
    esp_err_t err = esp_wifi_disconnect();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disconnect WiFi: %s", esp_err_to_name(err));
        return err;
    }
    
    s_wifi_state = WIFI_STATE_DISCONNECTED;
    strcpy(s_ip_string, "0.0.0.0");
    strcpy(s_ap_ip_string, "0.0.0.0"); // Reset AP IP
    
    return ESP_OK;
}

bool wifi_is_connected(void)
{
    return (s_wifi_state == WIFI_STATE_CONNECTED);
}

bool wifi_is_ap_mode(void)
{
    return (s_wifi_state == WIFI_STATE_AP_MODE || s_wifi_state == WIFI_STATE_AP_STA_MODE);
}

wifi_state_t wifi_get_state(void)
{
    return s_wifi_state;
}

const char* wifi_get_ip_string(void)
{
    if (s_wifi_state == WIFI_STATE_CONNECTED) {
        esp_netif_t *netif = s_netif_sta ? s_netif_sta : esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif) {
            esp_netif_ip_info_t ip_info;
            if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
                snprintf(s_ip_string, sizeof(s_ip_string), IPSTR, IP2STR(&ip_info.ip));
                return s_ip_string;
            }
        }
    } else if (s_wifi_state == WIFI_STATE_AP_MODE || s_wifi_state == WIFI_STATE_AP_STA_MODE) {
        esp_netif_t *netif = s_netif_ap ? s_netif_ap : esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
        if (netif) {
            esp_netif_ip_info_t ip_info;
            if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
                snprintf(s_ap_ip_string, sizeof(s_ap_ip_string), IPSTR, IP2STR(&ip_info.ip));
                return s_ap_ip_string;
            }
        }
    }
    strcpy(s_ip_string, "0.0.0.0"); // Default for STA if not connected
    strcpy(s_ap_ip_string, "0.0.0.0"); // Default for AP if not active
    return s_ip_string; // Return STA IP by default, or AP IP if only AP is active
}

esp_err_t wifi_get_mac_address(uint8_t* mac, wifi_interface_t ifx)
{
    if (mac == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return esp_wifi_get_mac(ifx, mac);
}

int wifi_scan_networks(wifi_ap_record_t* ap_info, int max_aps)
{
    if (ap_info == NULL || max_aps <= 0) {
        return 0;
    }
    
    ESP_LOGI(TAG, "Scanning WiFi networks...");
    
    uint16_t number = max_aps;
    esp_wifi_scan_start(NULL, true);
    esp_wifi_scan_get_ap_records(&number, ap_info);
    esp_wifi_scan_stop();
    
    ESP_LOGI(TAG, "Found %d WiFi networks", number);
    return number;
}

esp_err_t wifi_factory_reset(void)
{
    ESP_LOGI(TAG, "Performing WiFi factory reset...");
    
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_erase_all(nvs_handle);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "WiFi configuration erased from NVS");
    }
    
    return ESP_OK;
}

esp_err_t wifi_register_event_callback(wifi_event_callback_t callback)
{
    s_event_callback = callback;
    return ESP_OK;
}

int8_t wifi_get_rssi(void)
{
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return ap_info.rssi;
    }
    return -100; // 无信号
}

esp_err_t wifi_get_ap_info(wifi_ap_record_t* ap_info)
{
    if (ap_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return esp_wifi_sta_get_ap_info(ap_info);
}
