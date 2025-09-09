#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "esp_http_client.h"
#include "cJSON.h"
#include "ai_assistant.h"

// AI引擎配置
#define AI_ENGINE_MAX_URL_LEN 256
#define AI_ENGINE_MAX_API_KEY_LEN 128
#define AI_ENGINE_MAX_REQUEST_LEN 2048
#define AI_ENGINE_MAX_RESPONSE_LEN 4096

// AI引擎类型
typedef enum {
    AI_ENGINE_OPENAI = 0,
    AI_ENGINE_BAIDU,
    AI_ENGINE_TENCENT,
    AI_ENGINE_CUSTOM
} ai_engine_type_t;

// AI引擎配置结构
typedef struct {
    ai_engine_type_t type;
    char api_key[AI_ENGINE_MAX_API_KEY_LEN];
    char server_url[AI_ENGINE_MAX_URL_LEN];
    uint32_t timeout_ms;
    bool enable_streaming;
} ai_engine_config_t;

// 函数声明
esp_err_t ai_engine_init(void);
esp_err_t ai_engine_set_config(const ai_engine_config_t *config);
esp_err_t ai_engine_get_config(ai_engine_config_t *config);
esp_err_t ai_engine_process_command(const char *command, ai_response_t *response);
esp_err_t ai_engine_set_api_key(const char *api_key);
esp_err_t ai_engine_set_server_url(const char *url);
esp_err_t ai_engine_test_connection(void);

// 内部函数
static esp_err_t ai_engine_send_request(const char *command, char *response, size_t response_len);
static esp_err_t ai_engine_parse_response(const char *json_response, ai_response_t *response);
static esp_err_t ai_engine_handle_openai(const char *command, ai_response_t *response);
static esp_err_t ai_engine_handle_baidu(const char *command, ai_response_t *response);
static esp_err_t ai_engine_handle_tencent(const char *command, ai_response_t *response);

#endif // AI_ENGINE_H
