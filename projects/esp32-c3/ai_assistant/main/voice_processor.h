#ifndef VOICE_PROCESSOR_H
#define VOICE_PROCESSOR_H

#include "esp_err.h"
#include "ai_engine.h"

// 语音识别配置
typedef struct {
    int sample_rate;
    int channels;
    int bits_per_sample;
    char language[16];
} voice_config_t;

// 语音合成配置
typedef struct {
    char voice_type[16];
    int speed;
    int pitch;
    int volume;
} tts_config_t;

// 函数声明
esp_err_t voice_processor_init(void);
esp_err_t voice_recognize_start(void);
esp_err_t voice_recognize_stop(void);
esp_err_t voice_synthesize(const char *text, const char *voice_type);
esp_err_t voice_set_config(voice_config_t *config);
esp_err_t voice_set_tts_config(tts_config_t *config);
esp_err_t voice_play_audio(const char *audio_data, size_t length);
esp_err_t voice_record_audio(char *audio_buffer, size_t buffer_size);

// 语音识别回调函数类型
typedef void (*voice_recognize_callback_t)(const char *text, float confidence);

// 设置语音识别回调
esp_err_t voice_set_recognize_callback(voice_recognize_callback_t callback);

#endif // VOICE_PROCESSOR_H
