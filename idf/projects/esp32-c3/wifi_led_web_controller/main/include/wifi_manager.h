/*
 * WiFi Manager for ESP32-C3
 * 
 * 提供WiFi连接管理功能
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* WiFi配置常量 */
#define WIFI_SSID_MAX_LEN       32
#define WIFI_PASSWORD_MAX_LEN   64
#define WIFI_MAX_RETRY          10

/* 默认STA模式配置 */
#define ESP_WIFI_STA_SSID       "mem2"
#define ESP_WIFI_STA_PASS       "md11180829"

/* AP模式配置 (作为备用) */
#define ESP_WIFI_AP_SSID        "ESP32C3-LED-Controller"
#define ESP_WIFI_AP_PASS        "12345678"
#define ESP_WIFI_AP_CHANNEL     1
#define ESP_WIFI_AP_MAX_STA     4

/* WiFi管理器事件 */
typedef enum {
    WIFI_MANAGER_EVENT_STA_CONNECTED,
    WIFI_MANAGER_EVENT_STA_DISCONNECTED,
    WIFI_MANAGER_EVENT_AP_STARTED,
    WIFI_MANAGER_EVENT_AP_STOPPED,
    WIFI_MANAGER_EVENT_STA_JOINED,
    WIFI_MANAGER_EVENT_STA_LEFT
} wifi_manager_event_t;

/* WiFi状态 */
typedef enum {
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_AP_MODE,
    WIFI_STATE_AP_STA_MODE
} wifi_state_t;

/* WiFi配置结构 */
typedef struct {
    char ssid[WIFI_SSID_MAX_LEN];
    char password[WIFI_PASSWORD_MAX_LEN];
    bool save_to_nvs;
} wifi_config_sta_t;

/* WiFi事件回调函数类型 */
typedef void (*wifi_event_callback_t)(wifi_manager_event_t event, void* data);

/* 函数声明 */

/**
 * @brief 初始化WiFi管理器
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief 连接到WiFi网络
 * @param ssid WiFi网络名称
 * @param password WiFi密码
 * @param save_to_nvs 是否保存到NVS
 * @return ESP_OK on success
 */
esp_err_t wifi_connect_sta(const char* ssid, const char* password, bool save_to_nvs);

/**
 * @brief 启动AP模式
 * @param ssid AP网络名称
 * @param password AP密码
 * @return ESP_OK on success
 */
esp_err_t wifi_start_ap(const char* ssid, const char* password);

/**
 * @brief 启用或禁用AP模式
 * @param enable true启用AP模式，false禁用AP模式
 * @return ESP_OK on success
 */
esp_err_t wifi_enable_ap_mode(bool enable);

/**
 * @brief 停止WiFi连接
 * @return ESP_OK on success
 */
esp_err_t wifi_disconnect(void);

/**
 * @brief 检查WiFi是否已连接
 * @return true if connected
 */
bool wifi_is_connected(void);

/**
 * @brief 检查是否处于AP模式
 * @return true if in AP mode
 */
bool wifi_is_ap_mode(void);

/**
 * @brief 获取WiFi状态
 * @return current WiFi state
 */
wifi_state_t wifi_get_state(void);

/**
 * @brief 获取IP地址字符串
 * @return IP address string
 */
const char* wifi_get_ip_string(void);

/**
 * @brief 获取MAC地址
 * @param mac MAC地址缓冲区 (6 bytes)
 * @return ESP_OK on success
 */
esp_err_t wifi_get_mac_address(uint8_t* mac, wifi_interface_t ifx);

/**
 * @brief 扫描WiFi网络
 * @param max_aps 最大AP数量
 * @return number of APs found
 */
int wifi_scan_networks(wifi_ap_record_t* ap_info, int max_aps);

/**
 * @brief 恢复出厂设置
 * @return ESP_OK on success
 */
esp_err_t wifi_factory_reset(void);

/**
 * @brief 注册WiFi事件回调
 * @param callback 回调函数
 * @return ESP_OK on success
 */
esp_err_t wifi_register_event_callback(wifi_event_callback_t callback);

/**
 * @brief 获取信号强度
 * @return RSSI value
 */
int8_t wifi_get_rssi(void);

/**
 * @brief 获取连接的AP信息
 * @param ap_info AP信息结构体指针
 * @return ESP_OK on success
 */
esp_err_t wifi_get_ap_info(wifi_ap_record_t* ap_info);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H
