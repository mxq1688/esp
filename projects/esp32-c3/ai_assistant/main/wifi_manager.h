#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"

// WiFi配置
typedef struct {
    char ssid[32];
    char password[64];
    int max_retry;
    bool enable_ap;
    char ap_ssid[32];
    char ap_password[64];
} wifi_config_t;

// WiFi状态
typedef enum {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_FAILED
} wifi_status_t;

// 函数声明
esp_err_t wifi_init(void);
esp_err_t wifi_connect(const char *ssid, const char *password);
esp_err_t wifi_disconnect(void);
esp_err_t wifi_reconnect(void);
bool wifi_is_connected(void);
wifi_status_t wifi_get_status(void);
const char* wifi_get_ip(void);
esp_err_t wifi_set_config(wifi_config_t *config);
esp_err_t wifi_start_ap(const char *ssid, const char *password);
esp_err_t wifi_stop_ap(void);

// WiFi事件回调函数类型
typedef void (*wifi_event_callback_t)(wifi_status_t status);

// 设置WiFi事件回调
esp_err_t wifi_set_event_callback(wifi_event_callback_t callback);

#endif // WIFI_MANAGER_H
