/*
 * API Handlers for ESP32-C3 LED Controller
 * 
 * RESTful API端点处理函数
 */

#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include "esp_http_server.h"
#include "esp_err.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

/* API响应状态 */
typedef enum {
    API_STATUS_OK,
    API_STATUS_ERROR,
    API_STATUS_INVALID_PARAM,
    API_STATUS_NOT_FOUND,
    API_STATUS_UNAUTHORIZED
} api_status_t;

/* API错误代码 */
typedef enum {
    API_ERROR_NONE = 0,
    API_ERROR_INVALID_JSON = 1001,
    API_ERROR_MISSING_PARAM = 1002,
    API_ERROR_INVALID_VALUE = 1003,
    API_ERROR_WIFI_FAILED = 2001,
    API_ERROR_LED_FAILED = 3001,
    API_ERROR_SYSTEM_ERROR = 9001
} api_error_code_t;

/* HTTP处理器函数声明 */

/**
 * @brief OPTIONS预检请求处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_options_handler(httpd_req_t *req);

/**
 * @brief 根页面处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_root_handler(httpd_req_t *req);

/**
 * @brief 系统状态API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_status_handler(httpd_req_t *req);

/**
 * @brief 系统信息API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_system_info_handler(httpd_req_t *req);

/**
 * @brief 系统重启API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_system_reset_handler(httpd_req_t *req);

/**
 * @brief WiFi状态API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_wifi_status_handler(httpd_req_t *req);

/**
 * @brief WiFi扫描API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_wifi_scan_handler(httpd_req_t *req);

/**
 * @brief WiFi连接API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_wifi_connect_handler(httpd_req_t *req);

/**
 * @brief LED状态API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_led_status_handler(httpd_req_t *req);

/**
 * @brief LED颜色控制API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_led_color_handler(httpd_req_t *req);

/**
 * @brief LED电源控制API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_led_power_handler(httpd_req_t *req);

/**
 * @brief LED亮度控制API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_led_brightness_handler(httpd_req_t *req);

/**
 * @brief LED特效控制API处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_led_effect_handler(httpd_req_t *req);

/**
 * @brief WebSocket处理器
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_websocket_handler(httpd_req_t *req);

/**
 * @brief OPTIONS请求处理器 (CORS预检)
 * @param req HTTP请求
 * @return ESP_OK on success
 */
esp_err_t api_options_handler(httpd_req_t *req);

/* 辅助函数 */

/**
 * @brief 创建API成功响应
 * @param data 数据对象 (可选)
 * @return JSON响应对象
 */
cJSON* api_create_success_response(cJSON* data);

/**
 * @brief 创建API错误响应
 * @param error_code 错误代码
 * @param message 错误消息
 * @return JSON响应对象
 */
cJSON* api_create_error_response(api_error_code_t error_code, const char* message);

/**
 * @brief 验证JSON参数
 * @param json JSON对象
 * @param required_fields 必需字段数组
 * @param field_count 字段数量
 * @return true if valid
 */
bool api_validate_json_params(cJSON* json, const char* required_fields[], int field_count);

/**
 * @brief 记录API请求
 * @param req HTTP请求
 * @param api_name API名称
 */
void api_log_request(httpd_req_t *req, const char* api_name);

/**
 * @brief 检查API访问权限
 * @param req HTTP请求
 * @return true if authorized
 */
bool api_check_authorization(httpd_req_t *req);

/**
 * @brief 获取客户端IP地址
 * @param req HTTP请求
 * @param ip_str IP地址字符串缓冲区
 * @param max_len 缓冲区最大长度
 * @return ESP_OK on success
 */
esp_err_t api_get_client_ip(httpd_req_t *req, char* ip_str, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif // API_HANDLERS_H
