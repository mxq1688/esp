/**
 * @file wifi_audio.h
 * @brief WiFi 网络音频播放器
 * 
 * 从网络下载 WAV 音频文件并播放
 */

#ifndef WIFI_AUDIO_H
#define WIFI_AUDIO_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi 配置
 */
#define WIFI_SSID       "mem2"
#define WIFI_PASSWORD   "md11180829"

/**
 * @brief 初始化 WiFi 连接
 * 
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t wifi_audio_init_wifi(void);

/**
 * @brief 等待 WiFi 连接成功
 * 
 * @param timeout_ms 超时时间 (毫秒)
 * @return esp_err_t ESP_OK 连接成功, ESP_ERR_TIMEOUT 超时
 */
esp_err_t wifi_audio_wait_connected(uint32_t timeout_ms);

/**
 * @brief 从网络下载并播放 WAV 音频
 * 
 * @param url WAV 文件的 URL
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t wifi_audio_play_url(const char *url);

/**
 * @brief 播放网络测试音频
 * 
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t wifi_audio_play_test(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_AUDIO_H

