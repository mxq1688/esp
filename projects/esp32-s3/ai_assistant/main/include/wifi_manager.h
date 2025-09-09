#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

// WiFi配置
#define WIFI_MAXIMUM_RETRY 5
#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASSWORD_MAX_LEN 64

// WiFi状态
typedef enum {
    WIFI_STATE_DISCONNECTED = 0,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_ERROR
} wifi_manager_state_t;

// WiFi配置结构
typedef struct {
    char ssid[WIFI_SSID_MAX_LEN];
    char password[WIFI_PASSWORD_MAX_LEN];
    wifi_auth_mode_t auth_mode;
    uint8_t max_retry;
} wifi_manager_config_t;

// 函数声明
esp_err_t wifi_manager_init(void);
esp_err_t wifi_manager_start(void);
esp_err_t wifi_manager_stop(void);
esp_err_t wifi_manager_scan(wifi_ap_record_t *ap_records, uint16_t *ap_count);
esp_err_t wifi_manager_connect(const char *ssid, const char *password);
esp_err_t wifi_manager_disconnect(void);
wifi_manager_state_t wifi_manager_get_state(void);
esp_err_t wifi_manager_get_ip_info(esp_netif_ip_info_t *ip_info);
esp_err_t wifi_manager_set_config(const wifi_manager_config_t *config);
esp_err_t wifi_manager_get_config(wifi_manager_config_t *config);

#endif // WIFI_MANAGER_H
