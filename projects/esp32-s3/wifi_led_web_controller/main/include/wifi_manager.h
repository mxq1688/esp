#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"

// WiFi配置
#define WIFI_AP_SSID "ESP32-S3-LED"
#define WIFI_AP_PASSWORD "12345678"
#define WIFI_AP_CHANNEL 1
#define WIFI_AP_MAX_CONN 4
#define WIFI_AP_BEACON_INTERVAL 100

// NVS键值
#define NVS_NAMESPACE "wifi_config"
#define NVS_KEY_SSID "ssid"
#define NVS_KEY_PASSWORD "password"
#define NVS_KEY_MODE "mode"

// 使用ESP-IDF内置的wifi_mode_t类型
// typedef enum {
//     WIFI_MODE_AP = 0,
//     WIFI_MODE_STA = 1,
//     WIFI_MODE_APSTA = 2
// } wifi_mode_t;

// 函数声明
esp_err_t wifi_manager_init(void);
esp_err_t wifi_connect_sta(const char *ssid, const char *password);
esp_err_t wifi_disconnect_sta(void);
esp_err_t wifi_start_ap(void);
esp_err_t wifi_stop_ap(void);
esp_err_t wifi_factory_reset(void);
bool wifi_is_connected(void);
bool wifi_is_ap_mode(void);
const char* wifi_get_ip_string(void);
wifi_mode_t wifi_get_current_mode(void);
esp_err_t wifi_save_config(const char *ssid, const char *password);
esp_err_t wifi_load_config(char *ssid, char *password);

#endif // WIFI_MANAGER_H
