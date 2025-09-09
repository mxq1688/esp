#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_http_server.h"
#include "ai_assistant.h"

// Web服务器配置
#define WEB_SERVER_MAX_URI_LEN 64
#define WEB_SERVER_MAX_HANDLERS 16
#define WEB_SERVER_STACK_SIZE 8192

// Web服务器状态
typedef enum {
    WEB_SERVER_STATE_STOPPED = 0,
    WEB_SERVER_STATE_STARTING,
    WEB_SERVER_STATE_RUNNING,
    WEB_SERVER_STATE_STOPPING,
    WEB_SERVER_STATE_ERROR
} web_server_state_t;

// Web服务器配置结构
typedef struct {
    uint16_t port;
    uint16_t max_uri_handlers;
    size_t stack_size;
    uint32_t recv_wait_timeout;
    uint32_t send_wait_timeout;
    bool enable_cors;
    char cors_origin[64];
} web_server_config_t;

// 函数声明
esp_err_t web_server_init(void);
esp_err_t web_server_start(void);
esp_err_t web_server_stop(void);
esp_err_t web_server_set_config(const web_server_config_t *config);
esp_err_t web_server_get_config(web_server_config_t *config);
web_server_state_t web_server_get_state(void);
httpd_handle_t web_server_get_handle(void);
esp_err_t web_server_register_uri_handler(const httpd_uri_t *uri_handler);
esp_err_t web_server_unregister_uri_handler(const char *uri, httpd_method_t method);

// 内部函数
static esp_err_t web_server_config_handler(httpd_req_t *req);
static esp_err_t web_server_status_handler(httpd_req_t *req);
static esp_err_t web_server_voice_handler(httpd_req_t *req);
static esp_err_t web_server_ai_handler(httpd_req_t *req);
static esp_err_t web_server_files_handler(httpd_req_t *req);
static esp_err_t web_server_cors_handler(httpd_req_t *req);

#endif // WEB_SERVER_H
