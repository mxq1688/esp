/**
 * @file wifi_manager.h
 * @brief WiFi connection and NTP time synchronization manager
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include <stdbool.h>

// WiFi credentials - modify these for your network
#define WIFI_SSID      "hm"
#define WIFI_PASSWORD  "11111111"

// NTP configuration
#define NTP_SERVER     "ntp.aliyun.com"  // 阿里云 NTP 服务器（国内）
#define GMT_OFFSET_SEC (28800)       // GMT+8 中国时区 (8 * 3600)
#define DAYLIGHT_OFFSET_SEC (0)      // 中国不使用夏令时

// Time sync interval
#define NTP_SYNC_INTERVAL_MS (3600000)  // 1 hour

/**
 * @brief Initialize WiFi and connect to the configured network
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Check if WiFi is connected
 * 
 * @return true if connected, false otherwise
 */
bool wifi_is_connected(void);

/**
 * @brief Initialize SNTP and synchronize time
 * 
 * @return ESP_OK on success
 */
esp_err_t ntp_sync_time(void);

/**
 * @brief Check if time has been synchronized
 * 
 * @return true if time is synced, false otherwise
 */
bool is_time_synced(void);

#endif // WIFI_MANAGER_H

