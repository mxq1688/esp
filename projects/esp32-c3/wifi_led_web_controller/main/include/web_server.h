/*
 * Web Server for ESP32-C3 LED Controller
 * 
 * HTTP服务器和WebSocket通信
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_http_server.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 服务器配置 */
#define WEB_SERVER_PORT         80
#define WEB_SERVER_MAX_CLIENTS  10
#define WEB_SERVER_STACK_SIZE   8192

/* URI路径定义 */
#define URI_ROOT                "/"
#define URI_API_STATUS          "/api/status"
#define URI_API_WIFI            "/api/wifi"
#define URI_API_WIFI_SCAN       "/api/wifi/scan"
#define URI_API_WIFI_CONNECT    "/api/wifi/connect"
#define URI_API_LED_STATUS      "/api/led/status"
#define URI_API_LED_COLOR       "/api/led/color"
#define URI_API_LED_POWER       "/api/led/power"
#define URI_API_LED_EFFECT      "/api/led/effect"
#define URI_API_LED_BRIGHTNESS  "/api/led/brightness"
#define URI_API_SYSTEM_INFO     "/api/system/info"
#define URI_API_SYSTEM_RESET    "/api/system/reset"
#define URI_WS                  "/ws"

/* WebSocket消息类型 */
typedef enum {
    WS_MSG_LED_STATUS,
    WS_MSG_WIFI_STATUS,
    WS_MSG_SYSTEM_STATUS,
    WS_MSG_ERROR
} ws_message_type_t;

/* WebSocket消息结构 */
typedef struct {
    ws_message_type_t type;
    char* data;
    size_t data_len;
} ws_message_t;

/* 函数声明 */

/**
 * @brief 启动Web服务器
 * @return httpd_handle_t 服务器句柄，失败返回NULL
 */
httpd_handle_t web_server_start(void);

/**
 * @brief 停止Web服务器
 * @param server 服务器句柄
 * @return ESP_OK on success
 */
esp_err_t web_server_stop(httpd_handle_t server);

/**
 * @brief 注册所有URI处理器
 * @param server 服务器句柄
 * @return ESP_OK on success
 */
esp_err_t web_server_register_handlers(httpd_handle_t server);

/**
 * @brief 发送WebSocket消息给所有客户端
 * @param message WebSocket消息
 * @return ESP_OK on success
 */
esp_err_t web_server_ws_send_all(const ws_message_t* message);

/**
 * @brief 发送WebSocket消息给指定客户端
 * @param fd 客户端文件描述符
 * @param message WebSocket消息
 * @return ESP_OK on success
 */
esp_err_t web_server_ws_send(int fd, const ws_message_t* message);

/**
 * @brief 获取活跃的WebSocket连接数
 * @return 连接数
 */
int web_server_get_ws_clients_count(void);

/**
 * @brief 设置CORS头部
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t web_server_set_cors_headers(httpd_req_t *req);

/**
 * @brief 解析JSON请求体
 * @param req HTTP请求
 * @param buffer 缓冲区
 * @param buffer_size 缓冲区大小
 * @return 解析的JSON对象，失败返回NULL
 */
cJSON* web_server_parse_json_body(httpd_req_t *req, char* buffer, size_t buffer_size);

/**
 * @brief 发送JSON响应
 * @param req HTTP请求
 * @param json JSON对象
 * @return ESP_OK on success
 */
esp_err_t web_server_send_json_response(httpd_req_t *req, cJSON* json);

/**
 * @brief 发送错误响应
 * @param req HTTP请求
 * @param status HTTP状态码
 * @param message 错误消息
 * @return ESP_OK on success
 */
esp_err_t web_server_send_error_response(httpd_req_t *req, httpd_err_code_t status, const char* message);

/**
 * @brief 创建系统信息JSON
 * @return JSON对象
 */
cJSON* web_server_create_system_info_json(void);

/**
 * @brief 创建WiFi状态JSON
 * @return JSON对象
 */
cJSON* web_server_create_wifi_status_json(void);

/**
 * @brief 创建LED状态JSON
 * @return JSON对象
 */
cJSON* web_server_create_led_status_json(void);

#ifdef __cplusplus
}
#endif

#endif // WEB_SERVER_H
