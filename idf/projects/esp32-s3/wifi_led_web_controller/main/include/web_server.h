#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"

// Web服务器配置
#define WEB_SERVER_PORT 80
#define WEB_SERVER_MAX_URI_HANDLERS 16
#define WEB_SERVER_STACK_SIZE 8192

// 函数声明
esp_err_t web_server_start(void);
esp_err_t web_server_stop(void);
httpd_handle_t web_server_get_handle(void);

#endif // WEB_SERVER_H
