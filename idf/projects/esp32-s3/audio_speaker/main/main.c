/**
 * @file main.c
 * @brief ESP32-S3 音频播放器主程序
 * 
 * 支持:
 * - 本地测试音 (正弦波、《小星星》)
 * - 网络音频流播放
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "include/audio_player.h"
#include "include/wifi_audio.h"

static const char *TAG = "main";

// 选择播放模式
#define PLAY_MODE_LOCAL     0   // 本地测试音
#define PLAY_MODE_NETWORK   1   // 网络音频

// 当前播放模式 - 修改这里切换模式
#define CURRENT_PLAY_MODE   PLAY_MODE_LOCAL

/**
 * @brief 打印系统信息
 */
static void print_system_info(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    ESP_LOGI(TAG, "==========================================");
    ESP_LOGI(TAG, "    ESP32-S3 音频播放器");
    ESP_LOGI(TAG, "==========================================");
    ESP_LOGI(TAG, "芯片: %s, %d核", 
             (chip_info.model == CHIP_ESP32S3) ? "ESP32-S3" : "Unknown",
             chip_info.cores);
    ESP_LOGI(TAG, "特性: WiFi%s%s",
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "最小可用堆: %lu bytes", (unsigned long)esp_get_minimum_free_heap_size());
    ESP_LOGI(TAG, "==========================================");
}

/**
 * @brief 打印接线说明
 */
static void print_wiring_guide(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========== 接线指南 (MAX98357) ==========");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "  ESP32-S3      MAX98357");
    ESP_LOGI(TAG, "  ─────────────────────────");
    ESP_LOGI(TAG, "  GPIO 15  -->  BCLK");
    ESP_LOGI(TAG, "  GPIO 16  -->  LRC");
    ESP_LOGI(TAG, "  GPIO 17  -->  DIN");
    ESP_LOGI(TAG, "  3.3V     -->  VIN");
    ESP_LOGI(TAG, "  GND      -->  GND");
    ESP_LOGI(TAG, "  喇叭+    <--  Speaker+");
    ESP_LOGI(TAG, "  喇叭-    <--  Speaker-");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==========================================");
    ESP_LOGI(TAG, "");
}

/**
 * @brief 本地音频演示任务
 */
static void local_audio_task(void *pvParameters)
{
    ESP_LOGI(TAG, "本地音频演示任务启动");
    
    audio_player_set_volume(70);
    
    while (1) {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, ">>> 开始本地音频演示 <<<");
        
        // 播放 440Hz 测试音
        ESP_LOGI(TAG, "[1/2] 播放 440Hz 测试音 - 1秒");
        audio_player_play_tone(440, 1000);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 播放《小星星》
        ESP_LOGI(TAG, "[2/2] 播放《小星星》旋律");
        audio_player_play_sample();
        
        ESP_LOGI(TAG, ">>> 演示完成，30秒后重复 <<<");
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

/**
 * @brief 网络音频播放任务
 */
static void network_audio_task(void *pvParameters)
{
    ESP_LOGI(TAG, "网络音频任务启动");
    
    // 初始化 WiFi
    esp_err_t ret = wifi_audio_init_wifi();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi 初始化失败");
        vTaskDelete(NULL);
        return;
    }
    
    // 等待 WiFi 连接
    ret = wifi_audio_wait_connected(30000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi 连接失败，切换到本地播放");
        // 切换到本地播放
        while (1) {
            audio_player_play_tone(440, 1000);
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
    
    audio_player_set_volume(80);
    
    while (1) {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, ">>> 播放网络音频 <<<");
        
        // 播放网络测试音频
        wifi_audio_play_test();
        
        ESP_LOGI(TAG, ">>> 30秒后重复 <<<");
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

/**
 * @brief 主函数
 */
void app_main(void)
{
    // 打印系统信息
    print_system_info();
    
    // 打印接线指南
    print_wiring_guide();
    
    // 初始化音频播放器
    ESP_LOGI(TAG, "正在初始化音频播放器...");
    esp_err_t ret = audio_player_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "音频播放器初始化失败: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "音频播放器初始化成功!");
    ESP_LOGI(TAG, "");
    
#if CURRENT_PLAY_MODE == PLAY_MODE_NETWORK
    ESP_LOGI(TAG, "模式: 网络音频播放");
    ESP_LOGI(TAG, "请确保已配置 WiFi (wifi_audio.h)");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "3秒后开始...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    xTaskCreate(network_audio_task, "network_audio", 8192, NULL, 5, NULL);
#else
    ESP_LOGI(TAG, "模式: 本地音频播放");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "3秒后开始播放...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    xTaskCreate(local_audio_task, "local_audio", 4096, NULL, 5, NULL);
#endif
}
