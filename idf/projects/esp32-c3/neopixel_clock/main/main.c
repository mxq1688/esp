/**
 * @file main.c
 * @brief NeoPixel Clock Main Application
 * 
 * This application creates a clock using a NeoPixel LED strip,
 * synchronizing time via WiFi and NTP.
 * 
 * WiFi Configuration Flow:
 * 1. On startup, check NVS for saved WiFi credentials
 * 2. If no credentials, start AP mode for configuration
 * 3. If credentials exist, try to connect
 * 4. If connection fails, start AP mode
 * 5. After successful connection, start web server for reconfiguration
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_random.h"
#include "nvs_flash.h"

#include "neopixel_driver.h"
#include "wifi_manager.h"
#include "clock_display.h"
#include "captive_portal.h"
#include "light_show.h"

static const char *TAG = "main";

// Time tracking
static time_t last_ntp_update = 0;

// Application state
typedef enum {
    APP_STATE_INIT,
    APP_STATE_AP_CONFIG,      // AP mode, waiting for WiFi config
    APP_STATE_CONNECTING,     // Connecting to WiFi
    APP_STATE_CONNECTED,      // Connected, clock running
    APP_STATE_ERROR           // Error state
} app_state_t;

static app_state_t s_app_state = APP_STATE_INIT;
static bool s_time_synced = false;
static bool s_animation_playing = false;  // 防止 clock_task 干扰动画

/**
 * @brief HSV to RGB conversion
 */
static rgb_color_t hsv_to_rgb(int hue, int sat, int val)
{
    hue = hue % 360;
    int h = hue / 60;
    int f = (hue % 60) * 255 / 60;
    int p = val * (255 - sat) / 255;
    int q = val * (255 - (sat * f / 255)) / 255;
    int t = val * (255 - (sat * (255 - f) / 255)) / 255;
    
    switch (h) {
        case 0: return (rgb_color_t){val, t, p};
        case 1: return (rgb_color_t){q, val, p};
        case 2: return (rgb_color_t){p, val, t};
        case 3: return (rgb_color_t){p, q, val};
        case 4: return (rgb_color_t){t, p, val};
        default: return (rgb_color_t){val, p, q};
    }
}

/**
 * @brief 动画1: 时钟指针归位
 * 模拟时钟指针从12点开始旋转，暗示"时间同步完成"
 */
static void animation_clock_hands(void)
{
    ESP_LOGI(TAG, "Animation: Clock Hands");
    
    // 秒针(绿色)快速转一圈
    for (int pos = 0; pos < 60; pos++) {
        neopixel_clear();
        // 秒针 - 绿色，带尾迹
        for (int tail = 0; tail < 5; tail++) {
            int p = (pos - tail + 60) % 60;
            int brightness = 180 - tail * 35;
            if (brightness > 0) {
                rgb_color_t green = {0, brightness, 0};
                neopixel_set_pixel(p, green);
            }
        }
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    // 分针(蓝色)转半圈
    for (int pos = 0; pos < 30; pos++) {
        neopixel_clear();
        // 分针 - 蓝色，带尾迹
        for (int tail = 0; tail < 8; tail++) {
            int p = (pos - tail + 60) % 60;
            int brightness = 200 - tail * 25;
            if (brightness > 0) {
                rgb_color_t blue = {0, brightness / 3, brightness};
                neopixel_set_pixel(p, blue);
            }
        }
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(30));
    }
    
    // 时针(红色)移动到当前位置
    for (int pos = 0; pos < 15; pos++) {
        neopixel_clear();
        // 时针 - 红色/橙色
        for (int tail = 0; tail < 3; tail++) {
            int p = (pos * 4 - tail + 60) % 60;  // 时针每格代表5分钟位置
            int brightness = 200 - tail * 50;
            if (brightness > 0) {
                rgb_color_t red = {brightness, brightness / 4, 0};
                neopixel_set_pixel(p, red);
            }
        }
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(60));
    }
    
    // 最终：三针同时显示并闪烁
    for (int flash = 0; flash < 3; flash++) {
        neopixel_clear();
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // 显示12点、3点、6点位置
        rgb_color_t red = {180, 40, 0};
        rgb_color_t green = {0, 180, 0};
        rgb_color_t blue = {0, 40, 180};
        neopixel_set_pixel(0, red);    // 12点 - 时针
        neopixel_set_pixel(15, blue);  // 3点 - 分针
        neopixel_set_pixel(30, green); // 6点 - 秒针
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief 动画3: 时间涟漪扩散
 * 从12点位置向两边扩散，在6点汇合
 */
static void animation_ripple(void)
{
    ESP_LOGI(TAG, "Animation: Time Ripple");
    
    // 涟漪扩散 - 多层波纹
    for (int wave = 0; wave < 3; wave++) {
        int hue_base = wave * 120;  // 每波不同颜色
        
        for (int spread = 0; spread <= 30; spread++) {
            neopixel_clear();
            
            // 绘制当前波和之前的波
            for (int w = 0; w <= wave; w++) {
                int wave_spread = spread - w * 10;
                if (wave_spread < 0) continue;
                if (wave_spread > 30) wave_spread = 30;
                
                int hue = (hue_base + w * 120) % 360;
                
                // 波头最亮，向后渐暗
                for (int tail = 0; tail < 6 && tail <= wave_spread; tail++) {
                    int brightness = 180 - tail * 30;
                    if (brightness <= 0) continue;
                    
                    rgb_color_t color = hsv_to_rgb(hue, 255, brightness);
                    
                    // 顺时针
                    int pos_cw = (wave_spread - tail) % 60;
                    neopixel_set_pixel(pos_cw, color);
                    
                    // 逆时针
                    int pos_ccw = (60 - (wave_spread - tail)) % 60;
                    if (pos_ccw != pos_cw) {
                        neopixel_set_pixel(pos_ccw, color);
                    }
                }
            }
            
            neopixel_refresh();
            vTaskDelay(pdMS_TO_TICKS(25));
        }
    }
    
    // 汇合后全圈彩虹渐变
    for (int brightness = 0; brightness <= 150; brightness += 10) {
        for (int i = 0; i < 60; i++) {
            int hue = (i * 6) % 360;
            rgb_color_t color = hsv_to_rgb(hue, 255, brightness);
            neopixel_set_pixel(i, color);
        }
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 渐暗
    for (int brightness = 150; brightness >= 0; brightness -= 10) {
        for (int i = 0; i < 60; i++) {
            int hue = (i * 6) % 360;
            rgb_color_t color = hsv_to_rgb(hue, 255, brightness);
            neopixel_set_pixel(i, color);
        }
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

/**
 * @brief 动画4: 彩虹收缩到时钟
 * 彩虹流水转圈后收缩聚集到时/分/秒位置
 */
static void animation_rainbow_clock(void)
{
    ESP_LOGI(TAG, "Animation: Rainbow to Clock");
    
    // 彩虹流水转2圈
    for (int round = 0; round < 2; round++) {
        for (int offset = 0; offset < 60; offset++) {
            for (int i = 0; i < 60; i++) {
                int hue = ((i + offset) * 6) % 360;
                rgb_color_t color = hsv_to_rgb(hue, 255, 120);
                neopixel_set_pixel(i, color);
            }
            neopixel_refresh();
            vTaskDelay(pdMS_TO_TICKS(15));
        }
    }
    
    // 收缩动画 - 颜色逐渐聚集到三个位置
    // 目标位置: 0(12点-红), 20(4点-蓝), 40(8点-绿)
    int targets[3] = {0, 20, 40};
    rgb_color_t target_colors[3] = {{200, 50, 0}, {0, 50, 200}, {0, 200, 50}};
    
    for (int step = 0; step < 30; step++) {
        neopixel_clear();
        
        float progress = (float)step / 30.0f;
        
        for (int i = 0; i < 60; i++) {
            // 找到最近的目标
            int nearest_target = 0;
            int min_dist = 60;
            for (int t = 0; t < 3; t++) {
                int dist = (i - targets[t] + 60) % 60;
                if (dist > 30) dist = 60 - dist;
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest_target = t;
                }
            }
            
            // 根据进度计算是否应该显示
            int threshold = (int)(30 * (1.0f - progress));
            if (min_dist <= threshold) {
                // 颜色从彩虹渐变到目标颜色
                int hue = (i * 6) % 360;
                rgb_color_t rainbow = hsv_to_rgb(hue, 255, 120);
                rgb_color_t target = target_colors[nearest_target];
                
                rgb_color_t color = {
                    (uint8_t)(rainbow.r * (1 - progress) + target.r * progress),
                    (uint8_t)(rainbow.g * (1 - progress) + target.g * progress),
                    (uint8_t)(rainbow.b * (1 - progress) + target.b * progress)
                };
                
                // 越靠近中心越亮
                int brightness_factor = 255 - min_dist * 8;
                if (brightness_factor < 50) brightness_factor = 50;
                color.r = color.r * brightness_factor / 255;
                color.g = color.g * brightness_factor / 255;
                color.b = color.b * brightness_factor / 255;
                
                neopixel_set_pixel(i, color);
            }
        }
        
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    // 最终只剩三个点闪烁
    for (int flash = 0; flash < 4; flash++) {
        neopixel_clear();
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(100));
        
        for (int t = 0; t < 3; t++) {
            // 每个点带一点光晕
            for (int offset = -1; offset <= 1; offset++) {
                int pos = (targets[t] + offset + 60) % 60;
                int brightness = (offset == 0) ? 255 : 80;
                rgb_color_t color = {
                    target_colors[t].r * brightness / 255,
                    target_colors[t].g * brightness / 255,
                    target_colors[t].b * brightness / 255
                };
                neopixel_set_pixel(pos, color);
            }
        }
        neopixel_refresh();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief Play WiFi connected celebration animation
 * 随机选择三种动画之一
 */
static void play_wifi_connected_animation(void)
{
    s_animation_playing = true;
    
    // 随机选择动画 (0, 1, 2)
    int animation = esp_random() % 3;
    
    ESP_LOGI(TAG, "Playing WiFi connected animation (type %d)...", animation);
    
    switch (animation) {
        case 0:
            animation_clock_hands();
            break;
        case 1:
            animation_ripple();
            break;
        case 2:
            animation_rainbow_clock();
            break;
    }
    
    // 清屏
    neopixel_clear();
    neopixel_refresh();
    
    s_animation_playing = false;
    ESP_LOGI(TAG, "WiFi connected animation finished");
}

/**
 * @brief WiFi configuration task
 * Handles AP mode configuration and monitors for new config
 */
static void wifi_config_task(void *pvParameters)
{
    char ssid[33] = {0};
    char password[65] = {0};
    
    while (1) {
        // Check if new config is available from captive portal
        if (captive_portal_has_new_config()) {
            captive_portal_get_ssid(ssid, sizeof(ssid));
            captive_portal_get_password(password, sizeof(password));
            captive_portal_clear_new_config();
            
            ESP_LOGI(TAG, "New WiFi config received: %s", ssid);
            
            // Stop AP mode
            captive_portal_stop();
            
            // Try to connect
            s_app_state = APP_STATE_CONNECTING;
            wifi_manager_connect(ssid, password);
            
            // Wait for connection
            esp_err_t ret = wifi_manager_wait_connected(30000); // 30 seconds
            
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Connected to WiFi successfully!");
                
                // Play success animation
                play_wifi_connected_animation();
                
                // Immediately sync NTP time
                ESP_LOGI(TAG, "Syncing time with NTP server...");
                if (ntp_sync_time() == ESP_OK) {
                    s_time_synced = true;
                    time(&last_ntp_update);  // 记录同步时间
                    ESP_LOGI(TAG, "Time synchronized!");
                }
                
                s_app_state = APP_STATE_CONNECTED;
                
                // Start STA web server for reconfiguration
                captive_portal_start_sta_server();
                
                // Print IP address
                char ip_str[16];
                if (wifi_manager_get_ip(ip_str, sizeof(ip_str)) == ESP_OK) {
                    ESP_LOGI(TAG, "======================================");
                    ESP_LOGI(TAG, "Device IP: %s", ip_str);
                    ESP_LOGI(TAG, "Access http://%s to reconfigure WiFi", ip_str);
                    ESP_LOGI(TAG, "======================================");
                }
            } else {
                ESP_LOGW(TAG, "Failed to connect, restarting AP mode");
                s_app_state = APP_STATE_AP_CONFIG;
                captive_portal_start();
            }
        }
        
        // Monitor connection status when connected
        if (s_app_state == APP_STATE_CONNECTED && !wifi_is_connected()) {
            ESP_LOGW(TAG, "WiFi disconnected, will retry...");
            // WiFi manager will auto-retry
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Main clock task
 */
static void clock_task(void *pvParameters)
{
    time_t now;
    struct tm timeinfo;

    ESP_LOGI(TAG, "Clock task started");

    while (1) {
        // 如果正在播放动画，跳过显示更新
        if (s_animation_playing) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        switch (s_app_state) {
            case APP_STATE_INIT:
            case APP_STATE_CONNECTING:
                // Show connecting animation
                clock_display_connecting_animation();
                break;
                
            case APP_STATE_AP_CONFIG:
                // Show AP mode animation (different from connecting)
                // Use a slower, pulsing animation to indicate config mode
                {
                    static int pulse_dir = 1;
                    static int pulse_val = 0;
                    pulse_val += pulse_dir * 5;
                    if (pulse_val >= 100) pulse_dir = -1;
                    if (pulse_val <= 0) pulse_dir = 1;
                    
                    // Show pulsing purple for AP mode
                    neopixel_clear();
                    rgb_color_t purple = {pulse_val, 0, pulse_val};
                    for (int i = 0; i < 60; i += 5) {
                        neopixel_set_pixel(i, purple);
                    }
                    neopixel_refresh();
                }
                break;
                
            case APP_STATE_CONNECTED:
                time(&now);
                
                // Check if it's time for periodic NTP update (every hour)
                // 只有在已同步且距离上次更新超过1小时才更新
                if (s_time_synced && last_ntp_update > 0 && (now - last_ntp_update >= NTP_SYNC_INTERVAL_MS / 1000)) {
                    ESP_LOGI(TAG, "Periodic NTP update (last update %ld seconds ago)", (long)(now - last_ntp_update));
                    ntp_sync_time();
                    time(&last_ntp_update);
                }

                // Check if light show is active
                if (light_show_is_active()) {
                    light_show_update();
                } else if (s_time_synced) {
                    // Update clock display
                    localtime_r(&now, &timeinfo);
                    clock_display_update(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                } else {
                    // 时间未同步，显示彩色呼吸灯（每次呼吸换颜色）
                    static int breath_val = 0;
                    static int breath_dir = 3;
                    static int color_index = 0;
                    
                    breath_val += breath_dir;
                    if (breath_val >= 120) {
                        breath_dir = -3;
                    }
                    if (breath_val <= 0) {
                        breath_dir = 3;
                        color_index = (color_index + 1) % 6;  // 换下一个颜色
                    }
                    
                    // 6种颜色循环
                    rgb_color_t color;
                    switch (color_index) {
                        case 0: color = (rgb_color_t){breath_val, 0, breath_val / 2}; break;        // 玫红
                        case 1: color = (rgb_color_t){0, breath_val, breath_val / 2}; break;        // 青色
                        case 2: color = (rgb_color_t){breath_val, breath_val / 2, 0}; break;        // 橙色
                        case 3: color = (rgb_color_t){breath_val / 2, 0, breath_val}; break;        // 紫色
                        case 4: color = (rgb_color_t){0, breath_val, 0}; break;                     // 绿色
                        default: color = (rgb_color_t){0, breath_val / 2, breath_val}; break;      // 蓝色
                    }
                    
                    for (int i = 0; i < 60; i++) {
                        neopixel_set_pixel(i, color);
                    }
                    neopixel_refresh();
                }
                break;
                
            case APP_STATE_ERROR:
                clock_display_error();
                break;
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

    // Initialize light show
    light_show_init();
    
    // LED test
    ESP_LOGI(TAG, "Testing LEDs...");
    rgb_color_t red = {255, 0, 0};
    rgb_color_t green = {0, 255, 0};
    rgb_color_t blue = {0, 0, 255};
    rgb_color_t white = {255, 255, 255};
    
    neopixel_set_pixel(0, red);
    neopixel_set_pixel(15, green);
    neopixel_set_pixel(30, blue);
    neopixel_set_pixel(45, white);
    neopixel_refresh();
    
    ESP_LOGI(TAG, "LED test: 4 LEDs should be lit");
    vTaskDelay(pdMS_TO_TICKS(2000));
    neopixel_clear();

    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi...");
    ret = wifi_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi initialization failed");
        s_app_state = APP_STATE_ERROR;
        return;
    }

    // Check for saved WiFi credentials
    char ssid[33] = {0};
    char password[65] = {0};
    
    if (captive_portal_load_config(ssid, sizeof(ssid), password, sizeof(password)) == ESP_OK) {
        // Found saved credentials, try to connect
        ESP_LOGI(TAG, "Found saved WiFi config: %s", ssid);
        s_app_state = APP_STATE_CONNECTING;
        
        wifi_manager_connect(ssid, password);
        ret = wifi_manager_wait_connected(30000); // 30 seconds timeout
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Connected to saved WiFi network");
            
            // Play success animation
            play_wifi_connected_animation();
            
            // Immediately sync NTP time
            ESP_LOGI(TAG, "Syncing time with NTP server...");
            if (ntp_sync_time() == ESP_OK) {
                s_time_synced = true;
                time(&last_ntp_update);  // 记录同步时间
                ESP_LOGI(TAG, "Time synchronized!");
            }
            
            s_app_state = APP_STATE_CONNECTED;
            
            // Start web server for reconfiguration
            captive_portal_start_sta_server();
            
            // Print IP address
            char ip_str[16];
            if (wifi_manager_get_ip(ip_str, sizeof(ip_str)) == ESP_OK) {
                ESP_LOGI(TAG, "======================================");
                ESP_LOGI(TAG, "Device IP: %s", ip_str);
                ESP_LOGI(TAG, "Access http://%s to reconfigure WiFi", ip_str);
                ESP_LOGI(TAG, "======================================");
            }
        } else {
            ESP_LOGW(TAG, "Failed to connect to saved WiFi, starting AP mode");
            s_app_state = APP_STATE_AP_CONFIG;
            captive_portal_start();
            ESP_LOGI(TAG, "======================================");
            ESP_LOGI(TAG, "Connect to WiFi: NeoPixel-Clock");
            ESP_LOGI(TAG, "Then open http://192.168.4.1");
            ESP_LOGI(TAG, "======================================");
        }
    } else {
        // No saved credentials, start AP mode
        ESP_LOGI(TAG, "No saved WiFi config, starting AP mode");
        s_app_state = APP_STATE_AP_CONFIG;
        captive_portal_start();
        ESP_LOGI(TAG, "======================================");
        ESP_LOGI(TAG, "Connect to WiFi: NeoPixel-Clock");
        ESP_LOGI(TAG, "Then open http://192.168.4.1");
        ESP_LOGI(TAG, "======================================");
    }

    // Create tasks
    xTaskCreate(clock_task, "clock_task", 4096, NULL, 5, NULL);
    xTaskCreate(wifi_config_task, "wifi_config_task", 4096, NULL, 4, NULL);

    ESP_LOGI(TAG, "=== NeoPixel Clock Started ===");
}
