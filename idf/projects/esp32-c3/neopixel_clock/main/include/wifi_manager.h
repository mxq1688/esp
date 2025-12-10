/**
 * @file wifi_manager.h
 * @brief WiFi connection and NTP time synchronization manager
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>

// NTP configuration
#define NTP_SERVER     "ntp.aliyun.com"  // 阿里云 NTP 服务器（国内）
#define GMT_OFFSET_SEC (28800)       // GMT+8 中国时区 (8 * 3600)
#define DAYLIGHT_OFFSET_SEC (0)      // 中国不使用夏令时

// Time sync interval
#define NTP_SYNC_INTERVAL_MS (3600000)  // 1 hour

/**
 * @brief Initialize WiFi subsystem
 * 
 * This only initializes WiFi, does not connect to any network.
 * Call wifi_manager_connect() to connect to a specific network.
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Connect to a WiFi network
 * 
 * @param ssid Network SSID
 * @param password Network password (can be NULL for open networks)
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_connect(const char *ssid, const char *password);

/**
 * @brief Wait for WiFi connection to complete
 * 
 * @param timeout_ms Timeout in milliseconds
 * @return ESP_OK if connected, ESP_FAIL if failed, ESP_ERR_TIMEOUT if timeout
 */
esp_err_t wifi_manager_wait_connected(uint32_t timeout_ms);

/**
 * @brief Check if WiFi is connected
 * 
 * @return true if connected, false otherwise
 */
bool wifi_is_connected(void);

/**
 * @brief Disconnect from current WiFi network
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_disconnect(void);

/**
 * @brief Get current IP address as string
 * 
 * @param ip_str Buffer to store IP string (min 16 bytes)
 * @param len Buffer length
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_get_ip(char *ip_str, size_t len);

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
