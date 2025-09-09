/**
 * @file audio_processor.c
 * @brief Audio Processor Implementation for ESP32-S3 AI Assistant
 * @author AI Assistant Team
 * @version 1.0.0
 */

#include "audio_processor.h"
#include "ai_assistant.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "freertos/task.h"
#include "esp_dsp.h"

static const char *TAG = "AUDIO_PROCESSOR";

// 音频处理状态和配置
static audio_processor_state_t s_audio_state = AUDIO_PROCESSOR_STATE_IDLE;
static audio_processor_config_t s_audio_config = {
    .i2s_port = I2S_NUM_0,
    .pin_config = {
        .bck_io_num = 4,     // INMP441 SCK
        .ws_io_num = 5,      // INMP441 WS
        .data_out_num = 16,  // MAX98357A DIN
        .data_in_num = 6     // INMP441 SD
    },
    .i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX,
        .sample_rate = AUDIO_PROCESSOR_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    },
    .sample_rate = AUDIO_PROCESSOR_SAMPLE_RATE,
    .bits_per_sample = AUDIO_PROCESSOR_BITS_PER_SAMPLE,
    .channels = AUDIO_PROCESSOR_CHANNELS,
    .format = AUDIO_FORMAT_PCM,
    .volume = 80,
    .enable_noise_reduction = true,
    .enable_echo_cancellation = true,
    .enable_auto_gain_control = true
};

// 音频处理缓冲区
static uint8_t s_record_buffer[AUDIO_PROCESSOR_BUFFER_SIZE];
static uint8_t s_play_buffer[AUDIO_PROCESSOR_BUFFER_SIZE];
static uint8_t s_processed_buffer[AUDIO_PROCESSOR_BUFFER_SIZE];

// 任务句柄和回调
static TaskHandle_t s_record_task_handle = NULL;
static TaskHandle_t s_play_task_handle = NULL;
static audio_data_callback_t s_audio_callback = NULL;
static void *s_callback_user_data = NULL;

// 录音和播放控制
static bool s_recording = false;
static bool s_playing = false;
static const uint8_t *s_play_data = NULL;
static size_t s_play_len = 0;
static size_t s_play_offset = 0;

/**
 * @brief 录音任务
 */
static esp_err_t audio_processor_record_task(void *pvParameters)
{
    size_t bytes_read = 0;
    
    ai_assistant_log_info(TAG, "Audio record task started");
    
    while (1) {
        if (s_recording && s_audio_state == AUDIO_PROCESSOR_STATE_RECORDING) {
            // 读取音频数据
            esp_err_t ret = i2s_read(s_audio_config.i2s_port, 
                                   s_record_buffer, 
                                   sizeof(s_record_buffer), 
                                   &bytes_read, 
                                   portMAX_DELAY);
            
            if (ret == ESP_OK && bytes_read > 0) {
                // 处理音频数据
                audio_processor_process_audio(s_record_buffer, s_processed_buffer, bytes_read);
                
                // 调用回调函数
                if (s_audio_callback != NULL) {
                    s_audio_callback(s_processed_buffer, bytes_read, s_callback_user_data);
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    return ESP_OK;
}

/**
 * @brief 播放任务
 */
static esp_err_t audio_processor_play_task(void *pvParameters)
{
    size_t bytes_written = 0;
    
    ai_assistant_log_info(TAG, "Audio play task started");
    
    while (1) {
        if (s_playing && s_audio_state == AUDIO_PROCESSOR_STATE_PLAYING && s_play_data != NULL) {
            size_t chunk_size = AUDIO_PROCESSOR_BUFFER_SIZE;
            if (s_play_offset + chunk_size > s_play_len) {
                chunk_size = s_play_len - s_play_offset;
            }
            
            if (chunk_size > 0) {
                // 复制数据到播放缓冲区并处理
                memcpy(s_play_buffer, s_play_data + s_play_offset, chunk_size);
                audio_processor_process_audio(s_play_buffer, s_processed_buffer, chunk_size);
                
                // 写入I2S
                esp_err_t ret = i2s_write(s_audio_config.i2s_port, 
                                        s_processed_buffer, 
                                        chunk_size, 
                                        &bytes_written, 
                                        portMAX_DELAY);
                
                if (ret == ESP_OK) {
                    s_play_offset += bytes_written;
                } else {
                    ai_assistant_log_error(TAG, "I2S write failed: %s", esp_err_to_name(ret));
                    break;
                }
            } else {
                // 播放完成
                ai_assistant_log_info(TAG, "Audio playback completed");
                s_playing = false;
                s_audio_state = AUDIO_PROCESSOR_STATE_IDLE;
                s_play_data = NULL;
                s_play_len = 0;
                s_play_offset = 0;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    return ESP_OK;
}

/**
 * @brief 初始化音频处理器
 */
esp_err_t audio_processor_init(void)
{
    ai_assistant_log_info(TAG, "Initializing Audio Processor");
    
    // 初始化I2S
    esp_err_t ret = audio_processor_i2s_init();
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to initialize I2S");
        return ret;
    }
    
    // 创建录音任务
    xTaskCreatePinnedToCore((TaskFunction_t)audio_processor_record_task, 
                           "audio_record", 
                           4096, 
                           NULL, 
                           5, 
                           &s_record_task_handle, 
                           0);
    
    // 创建播放任务
    xTaskCreatePinnedToCore((TaskFunction_t)audio_processor_play_task, 
                           "audio_play", 
                           4096, 
                           NULL, 
                           5, 
                           &s_play_task_handle, 
                           1);
    
    if (s_record_task_handle == NULL || s_play_task_handle == NULL) {
        ai_assistant_log_error(TAG, "Failed to create audio processing tasks");
        return ESP_ERR_NO_MEM;
    }
    
    s_audio_state = AUDIO_PROCESSOR_STATE_IDLE;
    ai_assistant_log_info(TAG, "Audio Processor initialized successfully");
    return ESP_OK;
}

/**
 * @brief 开始录音
 */
esp_err_t audio_processor_record_start(void)
{
    ai_assistant_log_info(TAG, "Starting audio recording");
    
    if (s_audio_state != AUDIO_PROCESSOR_STATE_IDLE) {
        ai_assistant_log_error(TAG, "Audio processor is not idle");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 启动I2S
    esp_err_t ret = i2s_start(s_audio_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to start I2S: %s", esp_err_to_name(ret));
        return ret;
    }
    
    s_recording = true;
    s_audio_state = AUDIO_PROCESSOR_STATE_RECORDING;
    
    ai_assistant_log_info(TAG, "Audio recording started");
    return ESP_OK;
}

/**
 * @brief 停止录音
 */
esp_err_t audio_processor_record_stop(void)
{
    ai_assistant_log_info(TAG, "Stopping audio recording");
    
    s_recording = false;
    s_audio_state = AUDIO_PROCESSOR_STATE_IDLE;
    
    ai_assistant_log_info(TAG, "Audio recording stopped");
    return ESP_OK;
}

/**
 * @brief 开始播放
 */
esp_err_t audio_processor_play_start(const uint8_t *audio_data, size_t len)
{
    if (audio_data == NULL || len == 0) {
        ai_assistant_log_error(TAG, "Invalid audio data");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Starting audio playback (%d bytes)", len);
    
    if (s_audio_state != AUDIO_PROCESSOR_STATE_IDLE) {
        ai_assistant_log_error(TAG, "Audio processor is not idle");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 启动I2S
    esp_err_t ret = i2s_start(s_audio_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to start I2S: %s", esp_err_to_name(ret));
        return ret;
    }
    
    s_play_data = audio_data;
    s_play_len = len;
    s_play_offset = 0;
    s_playing = true;
    s_audio_state = AUDIO_PROCESSOR_STATE_PLAYING;
    
    ai_assistant_log_info(TAG, "Audio playback started");
    return ESP_OK;
}

/**
 * @brief 停止播放
 */
esp_err_t audio_processor_play_stop(void)
{
    ai_assistant_log_info(TAG, "Stopping audio playback");
    
    s_playing = false;
    s_audio_state = AUDIO_PROCESSOR_STATE_IDLE;
    s_play_data = NULL;
    s_play_len = 0;
    s_play_offset = 0;
    
    ai_assistant_log_info(TAG, "Audio playback stopped");
    return ESP_OK;
}

/**
 * @brief 设置音频处理器配置
 */
esp_err_t audio_processor_set_config(const audio_processor_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_audio_config, config, sizeof(audio_processor_config_t));
    ai_assistant_log_info(TAG, "Audio processor configuration updated");
    return ESP_OK;
}

/**
 * @brief 获取音频处理器配置
 */
esp_err_t audio_processor_get_config(audio_processor_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_audio_config, sizeof(audio_processor_config_t));
    return ESP_OK;
}

/**
 * @brief 获取音频处理器状态
 */
audio_processor_state_t audio_processor_get_state(void)
{
    return s_audio_state;
}

/**
 * @brief 设置音量
 */
esp_err_t audio_processor_set_volume(uint8_t volume)
{
    if (volume > 100) {
        ai_assistant_log_error(TAG, "Invalid volume level: %d", volume);
        return ESP_ERR_INVALID_ARG;
    }
    
    s_audio_config.volume = volume;
    ai_assistant_log_info(TAG, "Volume set to: %d", volume);
    return ESP_OK;
}

/**
 * @brief 设置音频格式
 */
esp_err_t audio_processor_set_format(audio_format_t format)
{
    s_audio_config.format = format;
    ai_assistant_log_info(TAG, "Audio format set to: %d", format);
    return ESP_OK;
}

/**
 * @brief 设置音频数据回调
 */
esp_err_t audio_processor_set_callback(audio_data_callback_t callback, void *user_data)
{
    s_audio_callback = callback;
    s_callback_user_data = user_data;
    ai_assistant_log_info(TAG, "Audio callback set");
    return ESP_OK;
}

/**
 * @brief 启用/禁用噪声抑制
 */
esp_err_t audio_processor_enable_noise_reduction(bool enable)
{
    s_audio_config.enable_noise_reduction = enable;
    ai_assistant_log_info(TAG, "Noise reduction %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}

/**
 * @brief 启用/禁用回声消除
 */
esp_err_t audio_processor_enable_echo_cancellation(bool enable)
{
    s_audio_config.enable_echo_cancellation = enable;
    ai_assistant_log_info(TAG, "Echo cancellation %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}

/**
 * @brief 启用/禁用自动增益控制
 */
esp_err_t audio_processor_enable_auto_gain_control(bool enable)
{
    s_audio_config.enable_auto_gain_control = enable;
    ai_assistant_log_info(TAG, "Auto gain control %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}

/**
 * @brief 初始化I2S
 */
static esp_err_t audio_processor_i2s_init(void)
{
    ai_assistant_log_info(TAG, "Initializing I2S for audio processor");
    
    // 安装I2S驱动
    esp_err_t ret = i2s_driver_install(s_audio_config.i2s_port, &s_audio_config.i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to install I2S driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 设置I2S引脚
    ret = i2s_set_pin(s_audio_config.i2s_port, &s_audio_config.pin_config);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to set I2S pins: %s", esp_err_to_name(ret));
        i2s_driver_uninstall(s_audio_config.i2s_port);
        return ret;
    }
    
    ai_assistant_log_info(TAG, "I2S initialized successfully");
    return ESP_OK;
}

/**
 * @brief 反初始化I2S
 */
static esp_err_t audio_processor_i2s_deinit(void)
{
    ai_assistant_log_info(TAG, "Deinitializing I2S for audio processor");
    
    esp_err_t ret = i2s_driver_uninstall(s_audio_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to uninstall I2S driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ai_assistant_log_info(TAG, "I2S deinitialized successfully");
    return ESP_OK;
}

/**
 * @brief 处理音频数据
 */
static esp_err_t audio_processor_process_audio(const uint8_t *input, uint8_t *output, size_t len)
{
    if (input == NULL || output == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 复制输入到输出作为基础
    memcpy(output, input, len);
    
    // 应用音频处理算法
    if (s_audio_config.enable_noise_reduction) {
        audio_processor_noise_reduction(input, output, len);
    }
    
    if (s_audio_config.enable_echo_cancellation) {
        audio_processor_echo_cancellation(output, output, len);
    }
    
    if (s_audio_config.enable_auto_gain_control) {
        audio_processor_auto_gain_control(output, output, len);
    }
    
    // 应用音量控制
    int16_t *samples = (int16_t*)output;
    size_t sample_count = len / 2;
    float volume_factor = s_audio_config.volume / 100.0f;
    
    for (size_t i = 0; i < sample_count; i++) {
        samples[i] = (int16_t)(samples[i] * volume_factor);
    }
    
    return ESP_OK;
}

/**
 * @brief 噪声抑制处理
 */
static esp_err_t audio_processor_noise_reduction(const uint8_t *input, uint8_t *output, size_t len)
{
    // 简单的噪声抑制算法 - 低通滤波
    int16_t *input_samples = (int16_t*)input;
    int16_t *output_samples = (int16_t*)output;
    size_t sample_count = len / 2;
    
    static int16_t prev_sample = 0;
    float alpha = 0.8f; // 滤波系数
    
    for (size_t i = 0; i < sample_count; i++) {
        output_samples[i] = (int16_t)(alpha * input_samples[i] + (1 - alpha) * prev_sample);
        prev_sample = output_samples[i];
    }
    
    return ESP_OK;
}

/**
 * @brief 回声消除处理
 */
static esp_err_t audio_processor_echo_cancellation(const uint8_t *input, uint8_t *output, size_t len)
{
    // 简单的回声消除算法 - 延迟减法
    int16_t *input_samples = (int16_t*)input;
    int16_t *output_samples = (int16_t*)output;
    size_t sample_count = len / 2;
    
    static int16_t delay_buffer[1024] = {0};
    static size_t delay_index = 0;
    
    for (size_t i = 0; i < sample_count; i++) {
        // 从延迟缓冲区中减去回声
        int16_t echo = delay_buffer[delay_index];
        output_samples[i] = input_samples[i] - (echo >> 2); // 减去1/4的回声
        
        // 更新延迟缓冲区
        delay_buffer[delay_index] = input_samples[i];
        delay_index = (delay_index + 1) % 1024;
    }
    
    return ESP_OK;
}

/**
 * @brief 自动增益控制处理
 */
static esp_err_t audio_processor_auto_gain_control(const uint8_t *input, uint8_t *output, size_t len)
{
    // 简单的AGC算法
    int16_t *input_samples = (int16_t*)input;
    int16_t *output_samples = (int16_t*)output;
    size_t sample_count = len / 2;
    
    // 计算RMS
    float rms = 0;
    for (size_t i = 0; i < sample_count; i++) {
        rms += input_samples[i] * input_samples[i];
    }
    rms = sqrtf(rms / sample_count);
    
    // 计算增益
    float target_rms = 8192.0f; // 目标RMS值
    float gain = 1.0f;
    if (rms > 0) {
        gain = target_rms / rms;
        if (gain > 4.0f) gain = 4.0f; // 限制最大增益
        if (gain < 0.25f) gain = 0.25f; // 限制最小增益
    }
    
    // 应用增益
    for (size_t i = 0; i < sample_count; i++) {
        int32_t sample = (int32_t)(input_samples[i] * gain);
        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;
        output_samples[i] = (int16_t)sample;
    }
    
    return ESP_OK;
}
