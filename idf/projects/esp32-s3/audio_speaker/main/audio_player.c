/**
 * @file audio_player.c
 * @brief ESP32-S3 I2S 音频播放器实现
 * 
 * 适配 MAX98357 I2S 功放模块
 */

#include "include/audio_player.h"
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

static const char *TAG = "audio_player";

// I2S 通道句柄
static i2s_chan_handle_t tx_handle = NULL;

// 当前采样率
static uint32_t current_sample_rate = AUDIO_SAMPLE_RATE;

// 音量 (0-100)
static uint8_t current_volume = 80;

// PI常量
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief 内部函数：用指定采样率初始化 I2S
 */
static esp_err_t audio_player_init_i2s(uint32_t sample_rate)
{
    ESP_LOGI(TAG, "初始化 I2S (采样率: %lu Hz)...", (unsigned long)sample_rate);
    
    // I2S 通道配置
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    
    esp_err_t ret = i2s_new_channel(&chan_cfg, &tx_handle, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建 I2S 通道失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // I2S 标准模式配置 - MAX98357 需要立体声格式
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_DOUT_PIN,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    
    ret = i2s_channel_init_std_mode(tx_handle, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "初始化 I2S 标准模式失败: %s", esp_err_to_name(ret));
        i2s_del_channel(tx_handle);
        tx_handle = NULL;
        return ret;
    }
    
    ret = i2s_channel_enable(tx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "启用 I2S 通道失败: %s", esp_err_to_name(ret));
        i2s_del_channel(tx_handle);
        tx_handle = NULL;
        return ret;
    }
    
    current_sample_rate = sample_rate;
    ESP_LOGI(TAG, "I2S 初始化完成 (采样率: %lu Hz)", (unsigned long)sample_rate);
    
    return ESP_OK;
}

/**
 * @brief 初始化音频播放器
 */
esp_err_t audio_player_init(void)
{
    ESP_LOGI(TAG, "初始化 I2S 音频播放器 (MAX98357)...");
    esp_err_t ret = audio_player_init_i2s(AUDIO_SAMPLE_RATE);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "引脚配置: BCLK=%d, WS=%d, DOUT=%d", I2S_BCLK_PIN, I2S_WS_PIN, I2S_DOUT_PIN);
    }
    return ret;
}

/**
 * @brief 设置采样率并重新初始化 I2S
 */
esp_err_t audio_player_set_sample_rate(uint32_t sample_rate)
{
    if (sample_rate == current_sample_rate && tx_handle != NULL) {
        return ESP_OK;  // 采样率相同，无需重新初始化
    }
    
    ESP_LOGI(TAG, "切换采样率: %lu -> %lu Hz", 
             (unsigned long)current_sample_rate, (unsigned long)sample_rate);
    
    // 先释放现有资源
    if (tx_handle != NULL) {
        i2s_channel_disable(tx_handle);
        i2s_del_channel(tx_handle);
        tx_handle = NULL;
    }
    
    // 重新初始化
    return audio_player_init_i2s(sample_rate);
}

/**
 * @brief 释放音频播放器资源
 */
esp_err_t audio_player_deinit(void)
{
    if (tx_handle != NULL) {
        i2s_channel_disable(tx_handle);
        i2s_del_channel(tx_handle);
        tx_handle = NULL;
        ESP_LOGI(TAG, "I2S 音频播放器已释放");
    }
    return ESP_OK;
}

/**
 * @brief 播放 PCM 音频数据 (单声道输入，自动转为立体声)
 * 
 * @param data 单声道 PCM 数据
 * @param len 数据长度 (字节)
 * @param wait_ms 超时时间 (毫秒)
 */
esp_err_t audio_player_play(const int16_t *data, size_t len, uint32_t wait_ms)
{
    if (tx_handle == NULL) {
        ESP_LOGE(TAG, "音频播放器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 单声道采样数
    size_t sample_count = len / sizeof(int16_t);
    
    // 分配立体声缓冲区 (大小翻倍)
    int16_t *stereo_data = (int16_t *)malloc(sample_count * 2 * sizeof(int16_t));
    if (stereo_data == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return ESP_ERR_NO_MEM;
    }
    
    // 转换为立体声并应用音量
    float volume_factor = current_volume / 100.0f;
    for (size_t i = 0; i < sample_count; i++) {
        int16_t sample = (int16_t)(data[i] * volume_factor);
        stereo_data[i * 2] = sample;      // 左声道
        stereo_data[i * 2 + 1] = sample;  // 右声道
    }
    
    size_t bytes_written = 0;
    esp_err_t ret = i2s_channel_write(tx_handle, stereo_data, sample_count * 2 * sizeof(int16_t), 
                                      &bytes_written, pdMS_TO_TICKS(wait_ms));
    
    free(stereo_data);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S 写入失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

/**
 * @brief 播放正弦波测试音
 * 
 * 生成立体声格式数据 (左右声道相同) 发送到 MAX98357
 */
esp_err_t audio_player_play_tone(uint32_t frequency, uint32_t duration_ms)
{
    if (tx_handle == NULL) {
        ESP_LOGE(TAG, "音频播放器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "播放 %lu Hz 正弦波, 持续 %lu ms (采样率: %lu Hz)", 
             (unsigned long)frequency, (unsigned long)duration_ms, (unsigned long)current_sample_rate);
    
    // 计算需要的采样数 (使用当前采样率)
    uint32_t total_samples = (current_sample_rate * duration_ms) / 1000;
    
    // 分块生成和播放，避免占用太多内存
    // 立体声格式：每个采样点需要 2 个 int16_t (左声道 + 右声道)
    const uint32_t chunk_samples = 512;  // 每块采样数
    int16_t *buffer = (int16_t *)malloc(chunk_samples * 2 * sizeof(int16_t));  // 立体声缓冲区
    if (buffer == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return ESP_ERR_NO_MEM;
    }
    
    float volume_factor = current_volume / 100.0f;
    float amplitude = 32767.0f * 0.8f * volume_factor;  // 80% 最大振幅，避免削波
    float phase_increment = 2.0f * M_PI * frequency / current_sample_rate;
    float phase = 0.0f;
    
    uint32_t samples_remaining = total_samples;
    
    while (samples_remaining > 0) {
        uint32_t samples_to_generate = (samples_remaining > chunk_samples) ? chunk_samples : samples_remaining;
        
        // 生成立体声数据：左声道、右声道交替
        for (uint32_t i = 0; i < samples_to_generate; i++) {
            int16_t sample = (int16_t)(amplitude * sinf(phase));
            buffer[i * 2] = sample;      // 左声道
            buffer[i * 2 + 1] = sample;  // 右声道 (相同数据)
            phase += phase_increment;
            if (phase >= 2.0f * M_PI) {
                phase -= 2.0f * M_PI;
            }
        }
        
        size_t bytes_written = 0;
        // 写入立体声数据 (字节数 = 采样数 * 2通道 * 2字节)
        esp_err_t ret = i2s_channel_write(tx_handle, buffer, samples_to_generate * 2 * sizeof(int16_t), 
                                          &bytes_written, pdMS_TO_TICKS(1000));
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "I2S 写入失败: %s", esp_err_to_name(ret));
            free(buffer);
            return ret;
        }
        
        samples_remaining -= samples_to_generate;
    }
    
    free(buffer);
    ESP_LOGI(TAG, "正弦波播放完成");
    
    return ESP_OK;
}

/**
 * @brief 播放内置示例音频 - 播放一段简单的旋律
 */
esp_err_t audio_player_play_sample(void)
{
    ESP_LOGI(TAG, "播放示例音频 - 《小星星》旋律");
    
    // 音符频率定义 (Hz)
    typedef struct {
        uint32_t freq;      // 频率
        uint32_t duration;  // 持续时间 (ms)
    } note_t;
    
    // C4 = 262 Hz, D4 = 294 Hz, E4 = 330 Hz, F4 = 349 Hz, G4 = 392 Hz, A4 = 440 Hz
    #define C4  262
    #define D4  294
    #define E4  330
    #define F4  349
    #define G4  392
    #define A4  440
    #define REST 0
    
    // 《小星星》旋律 (Twinkle Twinkle Little Star)
    const note_t melody[] = {
        // 一闪一闪亮晶晶
        {C4, 400}, {C4, 400}, {G4, 400}, {G4, 400},
        {A4, 400}, {A4, 400}, {G4, 800},
        // 满天都是小星星
        {F4, 400}, {F4, 400}, {E4, 400}, {E4, 400},
        {D4, 400}, {D4, 400}, {C4, 800},
        // 挂在天上放光明
        {G4, 400}, {G4, 400}, {F4, 400}, {F4, 400},
        {E4, 400}, {E4, 400}, {D4, 800},
        // 好像许多小眼睛
        {G4, 400}, {G4, 400}, {F4, 400}, {F4, 400},
        {E4, 400}, {E4, 400}, {D4, 800},
        // 一闪一闪亮晶晶
        {C4, 400}, {C4, 400}, {G4, 400}, {G4, 400},
        {A4, 400}, {A4, 400}, {G4, 800},
        // 满天都是小星星
        {F4, 400}, {F4, 400}, {E4, 400}, {E4, 400},
        {D4, 400}, {D4, 400}, {C4, 800},
    };
    
    const int melody_length = sizeof(melody) / sizeof(melody[0]);
    
    for (int i = 0; i < melody_length; i++) {
        if (melody[i].freq == REST) {
            // 休止符 - 播放静音
            vTaskDelay(pdMS_TO_TICKS(melody[i].duration));
        } else {
            // 播放音符
            esp_err_t ret = audio_player_play_tone(melody[i].freq, melody[i].duration);
            if (ret != ESP_OK) {
                return ret;
            }
        }
        // 音符之间的短暂间隔
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    ESP_LOGI(TAG, "示例音频播放完成");
    return ESP_OK;
}

/**
 * @brief 设置音量
 */
void audio_player_set_volume(uint8_t volume)
{
    if (volume > 100) {
        volume = 100;
    }
    current_volume = volume;
    ESP_LOGI(TAG, "音量设置为: %d%%", volume);
}

/**
 * @brief 获取当前音量
 */
uint8_t audio_player_get_volume(void)
{
    return current_volume;
}

/**
 * @brief 直接播放立体声 PCM 数据 (已经是立体声格式)
 * 
 * @param data 立体声 PCM 数据 (左右声道交替)
 * @param len 数据长度 (字节)
 * @param wait_ms 超时时间 (毫秒)
 */
esp_err_t audio_player_play_stereo(const int16_t *data, size_t len, uint32_t wait_ms)
{
    if (tx_handle == NULL) {
        ESP_LOGE(TAG, "音频播放器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t bytes_written = 0;
    esp_err_t ret = i2s_channel_write(tx_handle, data, len, &bytes_written, pdMS_TO_TICKS(wait_ms));
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S 写入失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

