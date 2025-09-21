#pragma once

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 系统状态API处理器
 * 
 * @param req HTTP请求
 * @return esp_err_t 
 */
esp_err_t api_status_handler(httpd_req_t *req);

/**
 * @brief ML307R网络信息API处理器
 * 
 * @param req HTTP请求
 * @return esp_err_t 
 */
esp_err_t api_network_info_handler(httpd_req_t *req);

/**
 * @brief 热点控制API处理器
 * 
 * @param req HTTP请求
 * @return esp_err_t 
 */
esp_err_t api_hotspot_control_handler(httpd_req_t *req);

/**
 * @brief 热点配置API处理器
 * 
 * @param req HTTP请求
 * @return esp_err_t 
 */
esp_err_t api_hotspot_config_handler(httpd_req_t *req);

/**
 * @brief ML307R重启API处理器
 * 
 * @param req HTTP请求
 * @return esp_err_t 
 */
esp_err_t api_ml307r_reset_handler(httpd_req_t *req);

/**
 * @brief WiFi连接API处理器
 * 
 * @param req HTTP请求
 * @return esp_err_t 
 */
esp_err_t api_wifi_connect_handler(httpd_req_t *req);

#ifdef __cplusplus
}
#endif
