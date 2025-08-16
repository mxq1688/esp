#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "esp_err.h"
#include "ai_engine.h"

// Web服务器配置
typedef struct {
    int port;
    char hostname[64];
    bool enable_ssl;
    char ssl_cert[256];
    char ssl_key[256];
} web_config_t;

// WebSocket消息类型
typedef enum {
    WS_MSG_CHAT,
    WS_MSG_VOICE,
    WS_MSG_CONTROL,
    WS_MSG_STATUS
} ws_msg_type_t;

// WebSocket消息结构
typedef struct {
    ws_msg_type_t type;
    char data[512];
    int length;
} ws_message_t;

// 函数声明
esp_err_t web_interface_init(void);
esp_err_t web_interface_start(void);
esp_err_t web_interface_stop(void);
esp_err_t web_send_response(ai_response_t *response);
esp_err_t web_send_status(const char *status);
esp_err_t web_set_config(web_config_t *config);
esp_err_t web_broadcast_message(const char *message);
esp_err_t web_handle_chat_message(const char *message);

// WebSocket回调函数类型
typedef void (*ws_message_callback_t)(ws_message_t *message);

// 设置WebSocket消息回调
esp_err_t web_set_ws_callback(ws_message_callback_t callback);

#endif // WEB_INTERFACE_H
