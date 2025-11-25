#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include "esp_http_server.h"

/**
 * @brief 注册所有API处理器
 * 
 * @param server HTTP服务器句柄
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t api_handlers_register(httpd_handle_t server);

/**
 * @brief 系统状态API处理器
 */
esp_err_t api_status_handler(httpd_req_t *req);

/**
 * @brief 摄像头配置API处理器
 */
esp_err_t api_camera_config_handler(httpd_req_t *req);

/**
 * @brief 摄像头流API处理器
 */
esp_err_t api_camera_stream_handler(httpd_req_t *req);

/**
 * @brief 摄像头抓拍API处理器
 */
esp_err_t api_camera_capture_handler(httpd_req_t *req);

/**
 * @brief 网络信息API处理器
 */
esp_err_t api_network_info_handler(httpd_req_t *req);

/**
 * @brief 4G热点控制API处理器
 */
esp_err_t api_hotspot_control_handler(httpd_req_t *req);

#endif // API_HANDLERS_H

