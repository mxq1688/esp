/**
 * @file voice_synthesis.c
 * @brief Voice Synthesis Implementation for ESP32-S3 AI Assistant
 * @author AI Assistant Team
 * @version 1.0.0
 */

#include "voice_synthesis.h"
#include "ai_assistant.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "driver/i2s.h"
#include "driver/dac.h"
#include "freertos/task.h"

static const char *TAG = "VOICE_SYNTHESIS";

// 语音合成状态和配置
static voice_synthesis_state_t s_synthesis_state = VOICE_SYNTHESIS_STATE_IDLE;
static voice_synthesis_config_t s_synthesis_config = {
    .i2s_port = I2S_NUM_1,
    .pin_config = {
        .bck_io_num = 7,     // MAX98357A BCLK
        .ws_io_num = 15,     // MAX98357A LRC
        .data_out_num = 16,  // MAX98357A DIN
        .data_in_num = I2S_PIN_NO_CHANGE
    },
    .i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = VOICE_SYNTHESIS_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // MAX98357A支持16位
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,   // 单声道输出到左声道
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    },
    .sample_rate = VOICE_SYNTHESIS_SAMPLE_RATE,
    .bits_per_sample = VOICE_SYNTHESIS_BITS_PER_SAMPLE,
    .channels = VOICE_SYNTHESIS_CHANNELS,
    .volume = 80,
    .voice_name = "xiaoyun",
    .language = 0, // 0=中文, 1=英文
    .enable_ssml = false
};

// 音频播放缓冲区
static uint8_t *s_audio_data = NULL;
static size_t s_audio_len = 0;
static TaskHandle_t s_synthesis_task_handle = NULL;
static bool s_playing = false;

/**
 * @brief 音频播放任务
 */
static void audio_playback_task(void *pvParameters)
{
    size_t bytes_written = 0;
    size_t offset = 0;
    
    ai_assistant_log_info(TAG, "Audio playback task started");
    
    while (1) {
        if (s_playing && s_synthesis_state == VOICE_SYNTHESIS_STATE_PLAYING && s_audio_data != NULL) {
            // 播放音频数据
            size_t chunk_size = VOICE_SYNTHESIS_BUFFER_SIZE;
            if (offset + chunk_size > s_audio_len) {
                chunk_size = s_audio_len - offset;
            }
            
            if (chunk_size > 0) {
                esp_err_t ret = i2s_write(s_synthesis_config.i2s_port, 
                                        s_audio_data + offset, 
                                        chunk_size, 
                                        &bytes_written, 
                                        portMAX_DELAY);
                if (ret == ESP_OK) {
                    offset += bytes_written;
                } else {
                    ai_assistant_log_error(TAG, "I2S write failed: %s", esp_err_to_name(ret));
                    break;
                }
            } else {
                // 播放完成
                ai_assistant_log_info(TAG, "Audio playback completed");
                s_playing = false;
                s_synthesis_state = VOICE_SYNTHESIS_STATE_IDLE;
                
                // 释放音频数据
                if (s_audio_data != NULL) {
                    free(s_audio_data);
                    s_audio_data = NULL;
                    s_audio_len = 0;
                }
                offset = 0;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

/**
 * @brief 初始化语音合成
 */
esp_err_t voice_synthesis_init(void)
{
    ai_assistant_log_info(TAG, "Initializing Voice Synthesis");
    
    // 初始化I2S
    esp_err_t ret = voice_synthesis_i2s_init();
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to initialize I2S for voice synthesis");
        return ret;
    }
    
    // 创建音频播放任务
    xTaskCreatePinnedToCore(audio_playback_task, "audio_play", 4096, NULL, 5, &s_synthesis_task_handle, 1);
    if (s_synthesis_task_handle == NULL) {
        ai_assistant_log_error(TAG, "Failed to create audio playback task");
        return ESP_ERR_NO_MEM;
    }
    
    s_synthesis_state = VOICE_SYNTHESIS_STATE_IDLE;
    ai_assistant_log_info(TAG, "Voice Synthesis initialized successfully");
    return ESP_OK;
}

/**
 * @brief 语音合成并播放
 */
esp_err_t voice_synthesis_speak(const char *text)
{
    if (text == NULL || strlen(text) == 0) {
        ai_assistant_log_error(TAG, "Text cannot be NULL or empty");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Starting voice synthesis for: %s", text);
    
    if (s_synthesis_state != VOICE_SYNTHESIS_STATE_IDLE) {
        ai_assistant_log_error(TAG, "Voice synthesis is busy");
        return ESP_ERR_INVALID_STATE;
    }
    
    s_synthesis_state = VOICE_SYNTHESIS_STATE_PROCESSING;
    
    // 发送文本到语音合成服务
    uint8_t *audio_data = NULL;
    size_t audio_len = 0;
    
    esp_err_t ret = voice_synthesis_send_to_server(text, &audio_data, &audio_len);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to synthesize speech");
        s_synthesis_state = VOICE_SYNTHESIS_STATE_IDLE;
        return ret;
    }
    
    // 播放音频
    ret = voice_synthesis_play_audio(audio_data, audio_len);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to play synthesized audio");
        if (audio_data != NULL) {
            free(audio_data);
        }
        s_synthesis_state = VOICE_SYNTHESIS_STATE_IDLE;
        return ret;
    }
    
    ai_assistant_log_info(TAG, "Voice synthesis started successfully");
    return ESP_OK;
}

/**
 * @brief 停止语音合成
 */
esp_err_t voice_synthesis_stop(void)
{
    ai_assistant_log_info(TAG, "Stopping Voice Synthesis");
    
    s_playing = false;
    s_synthesis_state = VOICE_SYNTHESIS_STATE_IDLE;
    
    // 停止I2S
    esp_err_t ret = i2s_stop(s_synthesis_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to stop I2S");
    }
    
    // 释放音频数据
    if (s_audio_data != NULL) {
        free(s_audio_data);
        s_audio_data = NULL;
        s_audio_len = 0;
    }
    
    ai_assistant_log_info(TAG, "Voice Synthesis stopped");
    return ESP_OK;
}

/**
 * @brief 设置语音合成配置
 */
esp_err_t voice_synthesis_set_config(const voice_synthesis_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_synthesis_config, config, sizeof(voice_synthesis_config_t));
    ai_assistant_log_info(TAG, "Voice synthesis configuration updated");
    return ESP_OK;
}

/**
 * @brief 获取语音合成配置
 */
esp_err_t voice_synthesis_get_config(voice_synthesis_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_synthesis_config, sizeof(voice_synthesis_config_t));
    return ESP_OK;
}

/**
 * @brief 获取语音合成状态
 */
voice_synthesis_state_t voice_synthesis_get_state(void)
{
    return s_synthesis_state;
}

/**
 * @brief 设置音量
 */
esp_err_t voice_synthesis_set_volume(uint8_t volume)
{
    if (volume > 100) {
        ai_assistant_log_error(TAG, "Invalid volume level: %d", volume);
        return ESP_ERR_INVALID_ARG;
    }
    
    s_synthesis_config.volume = volume;
    ai_assistant_log_info(TAG, "Volume set to: %d", volume);
    return ESP_OK;
}

/**
 * @brief 设置语音
 */
esp_err_t voice_synthesis_set_voice(const char *voice_name)
{
    if (voice_name == NULL) {
        ai_assistant_log_error(TAG, "Voice name cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(s_synthesis_config.voice_name, voice_name, sizeof(s_synthesis_config.voice_name) - 1);
    s_synthesis_config.voice_name[sizeof(s_synthesis_config.voice_name) - 1] = '\0';
    
    ai_assistant_log_info(TAG, "Voice set to: %s", s_synthesis_config.voice_name);
    return ESP_OK;
}

/**
 * @brief 初始化I2S
 */
static esp_err_t voice_synthesis_i2s_init(void)
{
    ai_assistant_log_info(TAG, "Initializing I2S for voice synthesis");
    
    // 安装I2S驱动
    esp_err_t ret = i2s_driver_install(s_synthesis_config.i2s_port, &s_synthesis_config.i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to install I2S driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 设置I2S引脚
    ret = i2s_set_pin(s_synthesis_config.i2s_port, &s_synthesis_config.pin_config);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to set I2S pins: %s", esp_err_to_name(ret));
        i2s_driver_uninstall(s_synthesis_config.i2s_port);
        return ret;
    }
    
    ai_assistant_log_info(TAG, "I2S initialized successfully");
    return ESP_OK;
}

/**
 * @brief 反初始化I2S
 */
static esp_err_t voice_synthesis_i2s_deinit(void)
{
    ai_assistant_log_info(TAG, "Deinitializing I2S for voice synthesis");
    
    esp_err_t ret = i2s_driver_uninstall(s_synthesis_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to uninstall I2S driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ai_assistant_log_info(TAG, "I2S deinitialized successfully");
    return ESP_OK;
}

/**
 * @brief 发送文本到服务器进行语音合成
 */
static esp_err_t voice_synthesis_send_to_server(const char *text, uint8_t **audio_data, size_t *audio_len)
{
    if (text == NULL || audio_data == NULL || audio_len == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Sending text to TTS server (mock implementation)");
    
    // 模拟语音合成 - 生成简单的正弦波音频
    size_t sample_count = strlen(text) * 8000; // 每个字符约0.5秒
    size_t data_size = sample_count * 2; // 16位音频
    
    *audio_data = (uint8_t*)malloc(data_size);
    if (*audio_data == NULL) {
        ai_assistant_log_error(TAG, "Failed to allocate audio buffer");
        return ESP_ERR_NO_MEM;
    }
    
    int16_t *samples = (int16_t*)*audio_data;
    float frequency = 440.0f; // A4音符
    float amplitude = 0.3f * (s_synthesis_config.volume / 100.0f);
    
    for (size_t i = 0; i < sample_count; i++) {
        float t = (float)i / s_synthesis_config.sample_rate;
        samples[i] = (int16_t)(amplitude * 32767 * sinf(2 * M_PI * frequency * t));
    }
    
    *audio_len = data_size;
    
    ai_assistant_log_info(TAG, "Generated %d bytes of audio data", data_size);
    return ESP_OK;
}

/**
 * @brief 播放音频数据
 */
static esp_err_t voice_synthesis_play_audio(const uint8_t *audio_data, size_t len)
{
    if (audio_data == NULL || len == 0) {
        ai_assistant_log_error(TAG, "Invalid audio data");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Starting audio playback (%d bytes)", len);
    
    // 启动I2S
    esp_err_t ret = i2s_start(s_synthesis_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to start I2S: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 设置音频数据
    s_audio_data = (uint8_t*)audio_data;
    s_audio_len = len;
    s_playing = true;
    s_synthesis_state = VOICE_SYNTHESIS_STATE_PLAYING;
    
    return ESP_OK;
}

/**
 * @brief 解析语音合成响应
 */
static esp_err_t voice_synthesis_parse_response(const char *json_response, uint8_t **audio_data, size_t *audio_len)
{
    if (json_response == NULL || audio_data == NULL || audio_len == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 解析JSON响应
    cJSON *json = cJSON_Parse(json_response);
    if (json == NULL) {
        ai_assistant_log_error(TAG, "Failed to parse JSON response");
        return ESP_FAIL;
    }
    
    // 提取音频数据URL或Base64编码的音频数据
    cJSON *audio_url_item = cJSON_GetObjectItem(json, "audio_url");
    cJSON *audio_data_item = cJSON_GetObjectItem(json, "audio_data");
    
    if (audio_url_item != NULL && cJSON_IsString(audio_url_item)) {
        // 从URL下载音频数据
        ai_assistant_log_info(TAG, "Downloading audio from URL: %s", audio_url_item->valuestring);
        // 这里应该实现HTTP下载逻辑
    } else if (audio_data_item != NULL && cJSON_IsString(audio_data_item)) {
        // 解码Base64音频数据
        ai_assistant_log_info(TAG, "Decoding Base64 audio data");
        // 这里应该实现Base64解码逻辑
    }
    
    cJSON_Delete(json);
    return ESP_OK;
}
