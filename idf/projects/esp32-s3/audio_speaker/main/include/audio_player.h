/**
 * @file audio_player.h
 * @brief ESP32-S3 I2S 音频播放器驱动
 * 
 * 支持通过 I2S 接口输出音频到外部 DAC（如 MAX98357A）或喇叭
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief I2S 引脚配置
 * 
 * 默认配置 (可根据实际接线修改):
 * - BCLK (位时钟):  GPIO 15
 * - WS (字选择/LRCK): GPIO 16  
 * - DOUT (数据输出): GPIO 17
 * 
 * 如果使用 MAX98357A 模块:
 * - BCLK -> BCLK
 * - LRC  -> WS
 * - DIN  -> DOUT
 * - VIN  -> 3.3V 或 5V
 * - GND  -> GND
 */
#define I2S_BCLK_PIN    15
#define I2S_WS_PIN      16
#define I2S_DOUT_PIN    17

/**
 * @brief 默认音频采样率
 */
#define AUDIO_SAMPLE_RATE   44100

/**
 * @brief 音频位深度
 */
#define AUDIO_BITS_PER_SAMPLE   16

/**
 * @brief 设置采样率并重新初始化 I2S
 * 
 * @param sample_rate 采样率 (Hz)
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t audio_player_set_sample_rate(uint32_t sample_rate);

/**
 * @brief 初始化音频播放器
 * 
 * @return esp_err_t ESP_OK 成功, 其他值表示失败
 */
esp_err_t audio_player_init(void);

/**
 * @brief 释放音频播放器资源
 * 
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t audio_player_deinit(void);

/**
 * @brief 播放 PCM 音频数据
 * 
 * @param data PCM 音频数据指针 (16位有符号, 单声道或立体声)
 * @param len 数据长度 (字节)
 * @param wait_ms 等待超时时间 (毫秒)
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t audio_player_play(const int16_t *data, size_t len, uint32_t wait_ms);

/**
 * @brief 播放正弦波测试音
 * 
 * @param frequency 频率 (Hz)
 * @param duration_ms 持续时间 (毫秒)
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t audio_player_play_tone(uint32_t frequency, uint32_t duration_ms);

/**
 * @brief 播放内置示例音频
 * 
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t audio_player_play_sample(void);

/**
 * @brief 设置音量 (0-100)
 * 
 * @param volume 音量值 (0-100)
 */
void audio_player_set_volume(uint8_t volume);

/**
 * @brief 获取当前音量
 * 
 * @return uint8_t 当前音量值 (0-100)
 */
uint8_t audio_player_get_volume(void);

/**
 * @brief 直接播放立体声 PCM 数据
 * 
 * @param data 立体声 PCM 数据 (左右声道交替)
 * @param len 数据长度 (字节)
 * @param wait_ms 等待超时时间 (毫秒)
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t audio_player_play_stereo(const int16_t *data, size_t len, uint32_t wait_ms);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_PLAYER_H

