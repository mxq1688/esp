/**
 * @file captive_portal.h
 * @brief Captive Portal for WiFi configuration via web interface
 */

#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include "esp_err.h"
#include <stdbool.h>

// AP configuration
#define AP_SSID         "NeoPixel-Clock"
#define AP_PASSWORD     ""              // 开放网络，无密码
#define AP_CHANNEL      1
#define AP_MAX_CONN     4

// Web server port
#define WEB_SERVER_PORT 80

/**
 * @brief Initialize and start the captive portal
 * 
 * Starts AP mode and web server for WiFi configuration
 * 
 * @return ESP_OK on success
 */
esp_err_t captive_portal_start(void);

/**
 * @brief Stop the captive portal
 * 
 * Stops web server and AP mode
 * 
 * @return ESP_OK on success
 */
esp_err_t captive_portal_stop(void);

/**
 * @brief Check if captive portal is running
 * 
 * @return true if running, false otherwise
 */
bool captive_portal_is_running(void);

/**
 * @brief Check if new WiFi credentials have been configured
 * 
 * @return true if new credentials available
 */
bool captive_portal_has_new_config(void);

/**
 * @brief Get configured SSID
 * 
 * @param ssid Buffer to store SSID (min 33 bytes)
 * @param len Buffer length
 * @return ESP_OK on success
 */
esp_err_t captive_portal_get_ssid(char *ssid, size_t len);

/**
 * @brief Get configured password
 * 
 * @param password Buffer to store password (min 65 bytes)
 * @param len Buffer length
 * @return ESP_OK on success
 */
esp_err_t captive_portal_get_password(char *password, size_t len);

/**
 * @brief Clear the new config flag after reading credentials
 */
void captive_portal_clear_new_config(void);

/**
 * @brief Load saved WiFi credentials from NVS
 * 
 * @param ssid Buffer to store SSID
 * @param ssid_len SSID buffer length
 * @param password Buffer to store password
 * @param password_len Password buffer length
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no config saved
 */
esp_err_t captive_portal_load_config(char *ssid, size_t ssid_len, char *password, size_t password_len);

/**
 * @brief Clear saved WiFi credentials from NVS
 * 
 * @return ESP_OK on success
 */
esp_err_t captive_portal_clear_config(void);

/**
 * @brief Start web server in STA mode for reconfiguration
 * 
 * This allows reconfiguring WiFi when already connected to a network
 * 
 * @return ESP_OK on success
 */
esp_err_t captive_portal_start_sta_server(void);

/**
 * @brief Stop STA mode web server
 * 
 * @return ESP_OK on success
 */
esp_err_t captive_portal_stop_sta_server(void);

#endif // CAPTIVE_PORTAL_H

