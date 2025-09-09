#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include "esp_http_server.h"
#include "cJSON.h"
#include "ai_assistant.h"

// API配置
#define API_MAX_JSON_LEN 4096
#define API_MAX_RESPONSE_LEN 8192
#define API_MAX_PARAM_LEN 256

// API错误码
typedef enum {
    API_ERROR_NONE = 0,
    API_ERROR_INVALID_REQUEST,
    API_ERROR_INVALID_PARAMETER,
    API_ERROR_NOT_FOUND,
    API_ERROR_INTERNAL_ERROR,
    API_ERROR_UNAUTHORIZED,
    API_ERROR_FORBIDDEN,
    API_ERROR_TIMEOUT,
    API_ERROR_SERVICE_UNAVAILABLE
} api_error_code_t;

// API响应结构
typedef struct {
    api_error_code_t error_code;
    char message[256];
    cJSON *data;
    uint32_t timestamp;
} api_response_t;

// 函数声明
esp_err_t api_handlers_init(void);
esp_err_t api_handlers_register(httpd_handle_t server);
esp_err_t api_handlers_unregister(httpd_handle_t server);

// API处理器函数
esp_err_t api_handler_get_status(httpd_req_t *req);
esp_err_t api_handler_get_config(httpd_req_t *req);
esp_err_t api_handler_set_config(httpd_req_t *req);
esp_err_t api_handler_voice_command(httpd_req_t *req);
esp_err_t api_handler_ai_chat(httpd_req_t *req);
esp_err_t api_handler_voice_upload(httpd_req_t *req);
esp_err_t api_handler_voice_download(httpd_req_t *req);
esp_err_t api_handler_system_info(httpd_req_t *req);
esp_err_t api_handler_restart(httpd_req_t *req);
esp_err_t api_handler_factory_reset(httpd_req_t *req);

// 工具函数
esp_err_t api_handlers_send_json_response(httpd_req_t *req, const api_response_t *response);
esp_err_t api_handlers_send_error_response(httpd_req_t *req, api_error_code_t error_code, const char *message);
esp_err_t api_handlers_parse_json_request(httpd_req_t *req, cJSON **json);
esp_err_t api_handlers_validate_request(httpd_req_t *req, const char *required_fields[], size_t field_count);
const char* api_handlers_get_error_message(api_error_code_t error_code);

#endif // API_HANDLERS_H
