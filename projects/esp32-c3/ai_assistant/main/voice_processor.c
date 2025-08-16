#include "voice_processor.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include <string.h>

static const char *TAG = "VOICE_PROCESSOR";

// 语音配置
static voice_config_t voice_config = {
    .sample_rate = 16000,
    .channels = 1,
    .bits_per_sample = 16,
    .language = "zh-CN"
};

static tts_config_t tts_config = {
    .voice_type = "female",
    .speed = 1,
    .pitch = 1,
    .volume = 1
};

// 语音识别回调
static voice_recognize_callback_t recognize_callback = NULL;

// 语音识别任务句柄
static TaskHandle_t voice_task_handle = NULL;
static bool voice_recognition_running = false;

// I2S配置
#define I2S_NUM I2S_NUM_0
#define I2S_BCK_IO 26
#define I2S_WS_IO 25
#define I2S_DO_IO 22
#define I2S_DI_IO 21

// 语音识别任务
static void voice_recognition_task(void *pvParameters)
{
    ESP_LOGI(TAG, "语音识别任务启动");
    
    // 模拟语音识别过程
    while (voice_recognition_running) {
        // 这里应该实现真正的语音识别
        // 暂时模拟识别结果
        vTaskDelay(pdMS_TO_TICKS(5000)); // 5秒模拟一次识别
        
        if (recognize_callback != NULL) {
            // 模拟识别到"你好小智"
            recognize_callback("你好小智", 0.85);
        }
    }
    
    ESP_LOGI(TAG, "语音识别任务结束");
    vTaskDelete(NULL);
}

esp_err_t voice_processor_init(void)
{
    ESP_LOGI(TAG, "初始化语音处理器");
    
    // 配置I2S
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX,
        .sample_rate = voice_config.sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO
    };
    
    esp_err_t ret = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S驱动安装失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2s_set_pin(I2S_NUM, &pin_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S引脚配置失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "语音处理器初始化完成");
    ESP_LOGI(TAG, "采样率: %d Hz", voice_config.sample_rate);
    ESP_LOGI(TAG, "语言: %s", voice_config.language);
    
    return ESP_OK;
}

esp_err_t voice_recognize_start(void)
{
    if (voice_recognition_running) {
        ESP_LOGW(TAG, "语音识别已在运行");
        return ESP_OK;
    }
    
    voice_recognition_running = true;
    
    BaseType_t ret = xTaskCreate(voice_recognition_task, 
                                "voice_recognition", 
                                4096, 
                                NULL, 
                                5, 
                                &voice_task_handle);
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "语音识别任务创建失败");
        voice_recognition_running = false;
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "语音识别已启动");
    return ESP_OK;
}

esp_err_t voice_recognize_stop(void)
{
    if (!voice_recognition_running) {
        ESP_LOGW(TAG, "语音识别未在运行");
        return ESP_OK;
    }
    
    voice_recognition_running = false;
    
    if (voice_task_handle != NULL) {
        vTaskDelete(voice_task_handle);
        voice_task_handle = NULL;
    }
    
    ESP_LOGI(TAG, "语音识别已停止");
    return ESP_OK;
}

esp_err_t voice_synthesize(const char *text, const char *voice_type)
{
    if (text == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "语音合成: %s", text);
    
    // 这里应该实现真正的TTS
    // 暂时模拟语音合成过程
    
    // 生成简单的音频数据（正弦波）
    const int sample_rate = 16000;
    const int duration_ms = 1000; // 1秒
    const int samples = sample_rate * duration_ms / 1000;
    const float frequency = 440.0; // A4音符
    
    int16_t *audio_buffer = malloc(samples * sizeof(int16_t));
    if (audio_buffer == NULL) {
        ESP_LOGE(TAG, "音频缓冲区分配失败");
        return ESP_ERR_NO_MEM;
    }
    
    // 生成正弦波
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sample_rate;
        audio_buffer[i] = (int16_t)(32767.0 * sin(2.0 * M_PI * frequency * t));
    }
    
    // 播放音频
    size_t bytes_written = 0;
    esp_err_t ret = i2s_write(I2S_NUM, audio_buffer, samples * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    
    free(audio_buffer);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "音频播放失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "语音合成完成，播放了 %zu 字节", bytes_written);
    return ESP_OK;
}

esp_err_t voice_set_config(voice_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&voice_config, config, sizeof(voice_config_t));
    ESP_LOGI(TAG, "语音配置已更新");
    
    return ESP_OK;
}

esp_err_t voice_set_tts_config(tts_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&tts_config, config, sizeof(tts_config_t));
    ESP_LOGI(TAG, "TTS配置已更新");
    
    return ESP_OK;
}

esp_err_t voice_play_audio(const char *audio_data, size_t length)
{
    if (audio_data == NULL || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t bytes_written = 0;
    esp_err_t ret = i2s_write(I2S_NUM, audio_data, length, &bytes_written, portMAX_DELAY);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "音频播放失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "音频播放完成，播放了 %zu 字节", bytes_written);
    return ESP_OK;
}

esp_err_t voice_record_audio(char *audio_buffer, size_t buffer_size)
{
    if (audio_buffer == NULL || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t bytes_read = 0;
    esp_err_t ret = i2s_read(I2S_NUM, audio_buffer, buffer_size, &bytes_read, portMAX_DELAY);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "音频录制失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "音频录制完成，录制了 %zu 字节", bytes_read);
    return ESP_OK;
}

esp_err_t voice_set_recognize_callback(voice_recognize_callback_t callback)
{
    recognize_callback = callback;
    ESP_LOGI(TAG, "语音识别回调已设置");
    return ESP_OK;
}
