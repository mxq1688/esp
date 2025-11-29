#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"
#include <stdbool.h>

/**
 * @brief 启动Web服务器
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t web_server_start(void);

/**
 * @brief 停止Web服务器
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t web_server_stop(void);

/**
 * @brief 检查Web服务器是否运行
 * 
 * @return true 运行中, false 已停止
 */
bool web_server_is_running(void);

/**
 * @brief 获取Web服务器句柄
 * 
 * @return httpd_handle_t 服务器句柄
 */
httpd_handle_t web_server_get_handle(void);

#endif // WEB_SERVER_H

