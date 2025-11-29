/**
 * @file wifi_manager.h
 * @brief WiFi connection and NTP time synchronization manager
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include <stdbool.h>

// WiFi credentials - modify these for your network
#define WIFI_SSID      "YourWifiSSID"
#define WIFI_PASSWORD  "YourWifiPassword"

// NTP configuration
#define NTP_SERVER     "pool.ntp.org"
#define GMT_OFFSET_SEC (-18000)      // GMT offset in seconds (e.g., -18000 for EST)
#define DAYLIGHT_OFFSET_SEC (3600)   // Daylight saving time offset

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

