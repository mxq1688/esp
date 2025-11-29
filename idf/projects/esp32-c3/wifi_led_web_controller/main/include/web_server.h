/*
 * Web Server for ESP32-C3 LED Controller
 * 
 * HTTP服务器和WebSocket通信
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_http_server.h"
#include "esp_err.h"
#include "cJSON.h"

// Web服务器配置
#define WEB_SERVER_PORT 80
#define WEB_SERVER_MAX_CLIENTS 4
#define WEB_SERVER_STACK_SIZE 8192

// 函数声明
esp_err_t web_server_start(void);
esp_err_t web_server_register_handlers(httpd_handle_t server);

// 工具函数声明
void web_server_set_cors_headers(httpd_req_t *req);
esp_err_t web_server_send_json_response(httpd_req_t *req, void *json);
void* web_server_parse_json_body(httpd_req_t *req, char *buffer, size_t buffer_size);
esp_err_t web_server_send_error_response(httpd_req_t *req, httpd_err_code_t error, const char *message);

// 错误代码类型定义
typedef enum {
    API_ERROR_NONE = 0,
    API_ERROR_INVALID_PARAMETER,
    API_ERROR_INTERNAL_ERROR,
    API_ERROR_NOT_FOUND
} api_error_code_t;

// 错误响应创建函数
cJSON* api_create_error_response(api_error_code_t error_code, const char* message);

#endif // WEB_SERVER_H
