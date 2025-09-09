#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSER_H

#include "driver/i2s.h"
#include "driver/dac.h"
#include "esp_ringbuf.h"
#include "ai_assistant.h"

// 音频处理配置
#define AUDIO_PROCESSOR_SAMPLE_RATE 16000
#define AUDIO_PROCESSOR_BITS_PER_SAMPLE 16
#define AUDIO_PROCESSOR_CHANNELS 1
#define AUDIO_PROCESSOR_BUFFER_SIZE 4096
#define AUDIO_PROCESSOR_MAX_RECORD_TIME_MS 10000 // 最大录音10秒

// 音频处理状态
typedef enum {
    AUDIO_PROCESSOR_STATE_IDLE = 0,
    AUDIO_PROCESSOR_STATE_RECORDING,
    AUDIO_PROCESSOR_STATE_PLAYING,
    AUDIO_PROCESSOR_STATE_PROCESSING,
    AUDIO_PROCESSOR_STATE_ERROR
} audio_processor_state_t;

// 音频格式
typedef enum {
    AUDIO_FORMAT_PCM = 0,
    AUDIO_FORMAT_WAV,
    AUDIO_FORMAT_MP3,
    AUDIO_FORMAT_AAC
} audio_format_t;

// 音频处理配置结构
typedef struct {
    i2s_port_t i2s_port;
    i2s_pin_config_t pin_config;
    i2s_config_t i2s_config;
    uint32_t sample_rate;
    uint32_t bits_per_sample;
    uint32_t channels;
    audio_format_t format;
    uint8_t volume;
    bool enable_noise_reduction;
    bool enable_echo_cancellation;
    bool enable_auto_gain_control;
} audio_processor_config_t;

// 音频数据回调函数类型
typedef void (*audio_data_callback_t)(const uint8_t *data, size_t len, void *user_data);

// 函数声明
esp_err_t audio_processor_init(void);
esp_err_t audio_processor_record_start(void);
esp_err_t audio_processor_record_stop(void);
esp_err_t audio_processor_play_start(const uint8_t *audio_data, size_t len);
esp_err_t audio_processor_play_stop(void);
esp_err_t audio_processor_set_config(const audio_processor_config_t *config);
esp_err_t audio_processor_get_config(audio_processor_config_t *config);
audio_processor_state_t audio_processor_get_state(void);
esp_err_t audio_processor_set_volume(uint8_t volume);
esp_err_t audio_processor_set_format(audio_format_t format);
esp_err_t audio_processor_set_callback(audio_data_callback_t callback, void *user_data);
esp_err_t audio_processor_enable_noise_reduction(bool enable);
esp_err_t audio_processor_enable_echo_cancellation(bool enable);
esp_err_t audio_processor_enable_auto_gain_control(bool enable);

// 内部函数
static esp_err_t audio_processor_i2s_init(void);
static esp_err_t audio_processor_i2s_deinit(void);
static esp_err_t audio_processor_record_task(void *pvParameters);
static esp_err_t audio_processor_play_task(void *pvParameters);
static esp_err_t audio_processor_process_audio(const uint8_t *input, uint8_t *output, size_t len);
static esp_err_t audio_processor_noise_reduction(const uint8_t *input, uint8_t *output, size_t len);
static esp_err_t audio_processor_echo_cancellation(const uint8_t *input, uint8_t *output, size_t len);
static esp_err_t audio_processor_auto_gain_control(const uint8_t *input, uint8_t *output, size_t len);

#endif // AUDIO_PROCESSOR_H
