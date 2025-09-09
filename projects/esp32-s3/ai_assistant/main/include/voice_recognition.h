#ifndef VOICE_RECOGNITION_H
#define VOICE_RECOGNITION_H

#include "driver/i2s.h"
#include "esp_ringbuf.h"
#include "ai_assistant.h"

// 语音识别配置
#define VOICE_RECOGNITION_SAMPLE_RATE 16000
#define VOICE_RECOGNITION_BITS_PER_SAMPLE 16
#define VOICE_RECOGNITION_CHANNELS 1
#define VOICE_RECOGNITION_BUFFER_SIZE 4096
#define VOICE_RECOGNITION_MAX_AUDIO_LEN 32000 // 2秒音频

// 语音识别状态
typedef enum {
    VOICE_RECOGNITION_STATE_IDLE = 0,
    VOICE_RECOGNITION_STATE_LISTENING,
    VOICE_RECOGNITION_STATE_PROCESSING,
    VOICE_RECOGNITION_STATE_ERROR
} voice_recognition_state_t;

// 语音识别配置结构
typedef struct {
    i2s_port_t i2s_port;
    i2s_pin_config_t pin_config;
    i2s_config_t i2s_config;
    uint32_t sample_rate;
    uint32_t bits_per_sample;
    uint32_t channels;
    bool auto_start;
    float confidence_threshold;
} voice_recognition_config_t;

// 函数声明
esp_err_t voice_recognition_init(void);
esp_err_t voice_recognition_start(void);
esp_err_t voice_recognition_stop(void);
esp_err_t voice_recognition_process(const uint8_t *audio_data, size_t len);
esp_err_t voice_recognition_set_config(const voice_recognition_config_t *config);
esp_err_t voice_recognition_get_config(voice_recognition_config_t *config);
voice_recognition_state_t voice_recognition_get_state(void);
esp_err_t voice_recognition_set_confidence_threshold(float threshold);

// 内部函数
static esp_err_t voice_recognition_i2s_init(void);
static esp_err_t voice_recognition_i2s_deinit(void);
static esp_err_t voice_recognition_process_audio(const uint8_t *audio_data, size_t len);
static esp_err_t voice_recognition_send_to_server(const uint8_t *audio_data, size_t len, char *text, size_t text_len);
static esp_err_t voice_recognition_parse_response(const char *json_response, voice_recognition_result_t *result);

#endif // VOICE_RECOGNITION_H
