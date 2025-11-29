#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "esp_err.h"
#include "esp_http_client.h"

// AI响应结构体
typedef struct {
    char text[512];
    char action[64];
    int confidence;
    char emotion[32];
} ai_response_t;

// AI个性化配置
typedef struct {
    char name[32];
    char personality[256];
    char voice_type[16];
    int response_speed;
} ai_personality_t;

// 语音数据结构体
typedef struct {
    char text[256];
    float confidence;
    int timestamp;
} voice_data_t;

// 函数声明
esp_err_t ai_engine_init(ai_personality_t *config);
esp_err_t ai_process_command(const char *command, ai_response_t *response);
esp_err_t ai_set_personality(ai_personality_t *config);
esp_err_t ai_get_response_from_api(const char *query, ai_response_t *response);
esp_err_t ai_process_local_command(const char *command, ai_response_t *response);

// 本地命令处理函数
esp_err_t ai_handle_weather_query(const char *query, ai_response_t *response);
esp_err_t ai_handle_time_query(const char *query, ai_response_t *response);
esp_err_t ai_handle_device_control(const char *command, ai_response_t *response);
esp_err_t ai_handle_music_control(const char *command, ai_response_t *response);
esp_err_t ai_handle_chat(const char *message, ai_response_t *response);

#endif // AI_ENGINE_H
