#ifndef VOICE_SYNTHESIS_H
#define VOICE_SYNTHESIS_H

#include "driver/i2s.h"
#include "driver/dac.h"
#include "esp_http_client.h"
#include "ai_assistant.h"

// 语音合成配置
#define VOICE_SYNTHESIS_SAMPLE_RATE 22050
#define VOICE_SYNTHESIS_BITS_PER_SAMPLE 16
#define VOICE_SYNTHESIS_CHANNELS 1
#define VOICE_SYNTHESIS_BUFFER_SIZE 4096
#define VOICE_SYNTHESIS_MAX_TEXT_LEN 512

// 语音合成状态
typedef enum {
    VOICE_SYNTHESIS_STATE_IDLE = 0,
    VOICE_SYNTHESIS_STATE_PROCESSING,
    VOICE_SYNTHESIS_STATE_PLAYING,
    VOICE_SYNTHESIS_STATE_ERROR
} voice_synthesis_state_t;

// 语音合成配置结构
typedef struct {
    i2s_port_t i2s_port;
    i2s_pin_config_t pin_config;
    i2s_config_t i2s_config;
    uint32_t sample_rate;
    uint32_t bits_per_sample;
    uint32_t channels;
    uint8_t volume;
    char voice_name[32];
    uint8_t language;
    bool enable_ssml;
} voice_synthesis_config_t;

// 函数声明
esp_err_t voice_synthesis_init(void);
esp_err_t voice_synthesis_speak(const char *text);
esp_err_t voice_synthesis_stop(void);
esp_err_t voice_synthesis_set_config(const voice_synthesis_config_t *config);
esp_err_t voice_synthesis_get_config(voice_synthesis_config_t *config);
voice_synthesis_state_t voice_synthesis_get_state(void);
esp_err_t voice_synthesis_set_volume(uint8_t volume);
esp_err_t voice_synthesis_set_voice(const char *voice_name);

// 内部函数
static esp_err_t voice_synthesis_i2s_init(void);
static esp_err_t voice_synthesis_i2s_deinit(void);
static esp_err_t voice_synthesis_send_to_server(const char *text, uint8_t **audio_data, size_t *audio_len);
static esp_err_t voice_synthesis_play_audio(const uint8_t *audio_data, size_t len);
static esp_err_t voice_synthesis_parse_response(const char *json_response, uint8_t **audio_data, size_t *audio_len);

#endif // VOICE_SYNTHESIS_H
