#pragma once

#include "esp_err.h"
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

// WiFi配置
#define WIFI_AP_SSID        "ESP32-S3-ML307R"
#define WIFI_AP_PASSWORD    "12345678"
#define WIFI_AP_CHANNEL     1
#define WIFI_AP_MAX_CONN    4

// WiFi状态
typedef enum {
    WIFI_STATE_DISCONNECTED = 0,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_AP_MODE,
    WIFI_STATE_ERROR
} wifi_state_t;

// WiFi信息结构体
typedef struct {
    char ssid[32];
    char password[64];
    wifi_state_t state;
    char ip_address[16];
    int rssi;
} wifi_info_t;

/**
 * @brief 初始化WiFi管理器
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief 启动AP模式
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_start_ap(void);

/**
 * @brief 连接到WiFi网络
 * 
 * @param ssid WiFi SSID
 * @param password WiFi密码
 * @return esp_err_t 
 */
esp_err_t wifi_manager_connect(const char *ssid, const char *password);

/**
 * @brief 断开WiFi连接
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_disconnect(void);

/**
 * @brief 获取WiFi状态
 * 
 * @return wifi_state_t 
 */
wifi_state_t wifi_manager_get_state(void);

/**
 * @brief 获取WiFi信息
 * 
 * @param info WiFi信息结构体指针
 * @return esp_err_t 
 */
esp_err_t wifi_manager_get_info(wifi_info_t *info);

/**
 * @brief 检查是否已连接WiFi
 * 
 * @return true 已连接
 * @return false 未连接
 */
bool wifi_manager_is_connected(void);

/**
 * @brief 启用NAPT（网络地址转换）用于互联网共享
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_enable_napt(void);

#ifdef __cplusplus
}
#endif
