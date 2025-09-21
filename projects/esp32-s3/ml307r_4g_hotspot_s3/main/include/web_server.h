#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 启动Web服务器
 * 
 * @return esp_err_t 
 */
esp_err_t web_server_start(void);

/**
 * @brief 停止Web服务器
 * 
 * @return esp_err_t 
 */
esp_err_t web_server_stop(void);

/**
 * @brief 检查Web服务器是否运行
 * 
 * @return true 服务器运行中
 * @return false 服务器未运行
 */
bool web_server_is_running(void);

#ifdef __cplusplus
}
#endif
