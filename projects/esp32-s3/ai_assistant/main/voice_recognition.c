/**
 * @file voice_recognition.c
 * @brief Voice Recognition Implementation for ESP32-S3 AI Assistant
 * @author AI Assistant Team
 * @version 1.0.0
 */

#include "voice_recognition.h"
#include "ai_assistant.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "driver/i2s.h"
#include "freertos/task.h"

static const char *TAG = "VOICE_RECOGNITION";

// 语音识别状态和配置
static voice_recognition_state_t s_voice_state = VOICE_RECOGNITION_STATE_IDLE;
static voice_recognition_config_t s_voice_config = {
    .i2s_port = I2S_NUM_0,
    .pin_config = {
        .bck_io_num = 4,     // INMP441 SCK
        .ws_io_num = 5,      // INMP441 WS
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 6     // INMP441 SD
    },
    .i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = VOICE_RECOGNITION_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,  // INMP441输出24位，用32位接收
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,   // INMP441配置为左声道
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    },
    .sample_rate = VOICE_RECOGNITION_SAMPLE_RATE,
    .bits_per_sample = VOICE_RECOGNITION_BITS_PER_SAMPLE,
    .channels = VOICE_RECOGNITION_CHANNELS,
    .auto_start = true,
    .confidence_threshold = 0.8f
};

// 音频缓冲区
static uint8_t s_audio_buffer[VOICE_RECOGNITION_MAX_AUDIO_LEN];
static size_t s_audio_len = 0;
static TaskHandle_t s_voice_task_handle = NULL;
static bool s_recording = false;

/**
 * @brief 语音录制任务
 */
static void voice_recording_task(void *pvParameters)
{
    uint8_t buffer[VOICE_RECOGNITION_BUFFER_SIZE];
    size_t bytes_read = 0;
    
    ai_assistant_log_info(TAG, "Voice recording task started");
    
    while (1) {
        if (s_recording && s_voice_state == VOICE_RECOGNITION_STATE_LISTENING) {
            // 读取I2S数据
            esp_err_t ret = i2s_read(s_voice_config.i2s_port, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
            if (ret == ESP_OK && bytes_read > 0) {
                // 检查缓冲区空间
                if (s_audio_len + bytes_read < VOICE_RECOGNITION_MAX_AUDIO_LEN) {
                    memcpy(s_audio_buffer + s_audio_len, buffer, bytes_read);
                    s_audio_len += bytes_read;
                } else {
                    // 缓冲区满，停止录制并处理
                    ai_assistant_log_info(TAG, "Audio buffer full, processing...");
                    s_recording = false;
                    s_voice_state = VOICE_RECOGNITION_STATE_PROCESSING;
                    
                    // 处理音频数据
                    voice_recognition_process(s_audio_buffer, s_audio_len);
                    
                    // 重置缓冲区
                    s_audio_len = 0;
                    s_voice_state = VOICE_RECOGNITION_STATE_LISTENING;
                    s_recording = true;
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

/**
 * @brief 初始化语音识别
 */
esp_err_t voice_recognition_init(void)
{
    ai_assistant_log_info(TAG, "Initializing Voice Recognition");
    
    // 初始化I2S
    esp_err_t ret = voice_recognition_i2s_init();
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to initialize I2S for voice recognition");
        return ret;
    }
    
    // 创建语音录制任务
    xTaskCreatePinnedToCore(voice_recording_task, "voice_record", 4096, NULL, 5, &s_voice_task_handle, 0);
    if (s_voice_task_handle == NULL) {
        ai_assistant_log_error(TAG, "Failed to create voice recording task");
        return ESP_ERR_NO_MEM;
    }
    
    s_voice_state = VOICE_RECOGNITION_STATE_IDLE;
    ai_assistant_log_info(TAG, "Voice Recognition initialized successfully");
    return ESP_OK;
}

/**
 * @brief 启动语音识别
 */
esp_err_t voice_recognition_start(void)
{
    ai_assistant_log_info(TAG, "Starting Voice Recognition");
    
    if (s_voice_state != VOICE_RECOGNITION_STATE_IDLE) {
        ai_assistant_log_error(TAG, "Voice recognition is not in idle state");
        return ESP_ERR_INVALID_STATE;
    }
    
    // 启动I2S
    esp_err_t ret = i2s_start(s_voice_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to start I2S");
        return ret;
    }
    
    // 重置音频缓冲区
    s_audio_len = 0;
    s_recording = true;
    s_voice_state = VOICE_RECOGNITION_STATE_LISTENING;
    
    ai_assistant_log_info(TAG, "Voice Recognition started");
    return ESP_OK;
}

/**
 * @brief 停止语音识别
 */
esp_err_t voice_recognition_stop(void)
{
    ai_assistant_log_info(TAG, "Stopping Voice Recognition");
    
    s_recording = false;
    s_voice_state = VOICE_RECOGNITION_STATE_IDLE;
    
    // 停止I2S
    esp_err_t ret = i2s_stop(s_voice_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to stop I2S");
        return ret;
    }
    
    ai_assistant_log_info(TAG, "Voice Recognition stopped");
    return ESP_OK;
}

/**
 * @brief 处理音频数据
 */
esp_err_t voice_recognition_process(const uint8_t *audio_data, size_t len)
{
    if (audio_data == NULL || len == 0) {
        ai_assistant_log_error(TAG, "Invalid audio data");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Processing audio data (%d bytes)", len);
    
    s_voice_state = VOICE_RECOGNITION_STATE_PROCESSING;
    
    // 这里应该调用实际的语音识别服务
    // 为了演示，我们创建一个模拟的识别结果
    voice_recognition_result_t result = {
        .text = "你好小智，今天天气怎么样？",
        .confidence = 0.95f,
        .command_type = VOICE_CMD_WEATHER,
        .timestamp = esp_timer_get_time()
    };
    
    // 检查置信度
    if (result.confidence >= s_voice_config.confidence_threshold) {
        // 发送到语音处理队列
        if (voice_queue != NULL) {
            if (xQueueSend(voice_queue, &result, pdMS_TO_TICKS(1000)) != pdTRUE) {
                ai_assistant_log_error(TAG, "Failed to send voice result to queue");
                return ESP_FAIL;
            }
        }
        
        ai_assistant_log_info(TAG, "Voice recognition successful: %s (confidence: %.2f)", 
                            result.text, result.confidence);
    } else {
        ai_assistant_log_info(TAG, "Voice recognition confidence too low: %.2f", result.confidence);
    }
    
    s_voice_state = VOICE_RECOGNITION_STATE_IDLE;
    return ESP_OK;
}

/**
 * @brief 设置语音识别配置
 */
esp_err_t voice_recognition_set_config(const voice_recognition_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_voice_config, config, sizeof(voice_recognition_config_t));
    ai_assistant_log_info(TAG, "Voice recognition configuration updated");
    return ESP_OK;
}

/**
 * @brief 获取语音识别配置
 */
esp_err_t voice_recognition_get_config(voice_recognition_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_voice_config, sizeof(voice_recognition_config_t));
    return ESP_OK;
}

/**
 * @brief 获取语音识别状态
 */
voice_recognition_state_t voice_recognition_get_state(void)
{
    return s_voice_state;
}

/**
 * @brief 设置置信度阈值
 */
esp_err_t voice_recognition_set_confidence_threshold(float threshold)
{
    if (threshold < 0.0f || threshold > 1.0f) {
        ai_assistant_log_error(TAG, "Invalid confidence threshold: %.2f", threshold);
        return ESP_ERR_INVALID_ARG;
    }
    
    s_voice_config.confidence_threshold = threshold;
    ai_assistant_log_info(TAG, "Confidence threshold set to: %.2f", threshold);
    return ESP_OK;
}

/**
 * @brief 初始化I2S
 */
static esp_err_t voice_recognition_i2s_init(void)
{
    ai_assistant_log_info(TAG, "Initializing I2S for voice recognition");
    
    // 安装I2S驱动
    esp_err_t ret = i2s_driver_install(s_voice_config.i2s_port, &s_voice_config.i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to install I2S driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 设置I2S引脚
    ret = i2s_set_pin(s_voice_config.i2s_port, &s_voice_config.pin_config);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to set I2S pins: %s", esp_err_to_name(ret));
        i2s_driver_uninstall(s_voice_config.i2s_port);
        return ret;
    }
    
    ai_assistant_log_info(TAG, "I2S initialized successfully");
    return ESP_OK;
}

/**
 * @brief 反初始化I2S
 */
static esp_err_t voice_recognition_i2s_deinit(void)
{
    ai_assistant_log_info(TAG, "Deinitializing I2S for voice recognition");
    
    esp_err_t ret = i2s_driver_uninstall(s_voice_config.i2s_port);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to uninstall I2S driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ai_assistant_log_info(TAG, "I2S deinitialized successfully");
    return ESP_OK;
}

/**
 * @brief 发送音频到服务器进行识别
 */
static esp_err_t voice_recognition_send_to_server(const uint8_t *audio_data, size_t len, char *text, size_t text_len)
{
    // 这里应该实现实际的语音识别API调用
    // 例如调用百度语音识别、讯飞语音识别等服务
    
    ai_assistant_log_info(TAG, "Sending audio to recognition server (mock implementation)");
    
    // 模拟识别结果
    const char *mock_results[] = {
        "你好小智",
        "今天天气怎么样",
        "播放音乐",
        "开灯",
        "关灯",
        "现在几点了",
        "帮我查询新闻"
    };
    
    int index = esp_random() % (sizeof(mock_results) / sizeof(mock_results[0]));
    strncpy(text, mock_results[index], text_len - 1);
    text[text_len - 1] = '\0';
    
    return ESP_OK;
}

/**
 * @brief 解析语音识别响应
 */
static esp_err_t voice_recognition_parse_response(const char *json_response, voice_recognition_result_t *result)
{
    if (json_response == NULL || result == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 解析JSON响应
    cJSON *json = cJSON_Parse(json_response);
    if (json == NULL) {
        ai_assistant_log_error(TAG, "Failed to parse JSON response");
        return ESP_FAIL;
    }
    
    // 提取识别文本
    cJSON *text_item = cJSON_GetObjectItem(json, "text");
    if (text_item != NULL && cJSON_IsString(text_item)) {
        strncpy(result->text, text_item->valuestring, sizeof(result->text) - 1);
        result->text[sizeof(result->text) - 1] = '\0';
    }
    
    // 提取置信度
    cJSON *confidence_item = cJSON_GetObjectItem(json, "confidence");
    if (confidence_item != NULL && cJSON_IsNumber(confidence_item)) {
        result->confidence = (float)confidence_item->valuedouble;
    } else {
        result->confidence = 0.8f; // 默认置信度
    }
    
    // 设置时间戳
    result->timestamp = esp_timer_get_time();
    
    // 分类命令类型（这里可以调用NLP处理器）
    result->command_type = VOICE_CMD_UNKNOWN;
    
    cJSON_Delete(json);
    return ESP_OK;
}
