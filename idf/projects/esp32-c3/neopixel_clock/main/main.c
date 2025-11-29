/**
 * @file main.c
 * @brief NeoPixel Clock Main Application
 * 
 * This application creates a clock using a NeoPixel LED strip,
 * synchronizing time via WiFi and NTP.
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "neopixel_driver.h"
#include "wifi_manager.h"
#include "clock_display.h"

static const char *TAG = "main";

// Time tracking
static time_t last_ntp_update = 0;

/**
 * @brief Main clock task
 */
static void clock_task(void *pvParameters)
{
    time_t now;
    struct tm timeinfo;
    bool wifi_connected = false;
    bool time_synced = false;
    int connection_timeout = 0;
    const int max_connection_timeout = 120; // 2 minutes (120 * 1000ms / 50ms)

    ESP_LOGI(TAG, "Clock task started");

    while (1) {
        wifi_connected = wifi_is_connected();

        if (wifi_connected) {
            // Check if time needs to be synced
            if (!time_synced) {
                ESP_LOGI(TAG, "Syncing time with NTP server...");
                if (ntp_sync_time() == ESP_OK) {
                    time_synced = true;
                    time(&last_ntp_update);
                    ESP_LOGI(TAG, "Time synchronized successfully");
                }
            }

            // Check if it's time for periodic NTP update (every hour)
            time(&now);
            if (time_synced && (now - last_ntp_update >= NTP_SYNC_INTERVAL_MS / 1000)) {
                ESP_LOGI(TAG, "Periodic NTP update");
                ntp_sync_time();
                time(&last_ntp_update);
            }

            // Update clock display
            if (time_synced) {
                localtime_r(&now, &timeinfo);
                clock_display_update(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            }
        } else {
            // Show connecting animation
            if (connection_timeout < max_connection_timeout) {
                clock_display_connecting_animation();
                connection_timeout++;
            } else {
                // Connection timeout - show error
                clock_display_error();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Update every 50ms
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== NeoPixel Clock Starting ===");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());

    // Initialize clock display (and NeoPixel)
    esp_err_t ret = clock_display_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize clock display: %s", esp_err_to_name(ret));
        return;
    }

    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi...");
    ret = wifi_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "WiFi initialization failed, will retry in background");
    }

    // Create clock task
    xTaskCreate(clock_task, "clock_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "=== NeoPixel Clock Started ===");
}

