/*
 * Web Server Implementation for ESP32-C3 LED Controller
 */

#include "web_server.h"
#include "api_handlers.h"
#include "esp_log.h"
#include "esp_system.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "WEB_SERVER";

/* WebSocket客户端管理 */
static int ws_clients[WEB_SERVER_MAX_CLIENTS] = {0};
static int ws_client_count = 0;
static SemaphoreHandle_t ws_clients_mutex = NULL;

/* 启动Web服务器 */
httpd_handle_t web_server_start(void)
{
    ESP_LOGI(TAG, "Starting web server...");
    
    // 初始化WebSocket客户端管理
    ws_clients_mutex = xSemaphoreCreateMutex();
    if (ws_clients_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create WebSocket clients mutex");
        return NULL;
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = WEB_SERVER_PORT;
    config.max_open_sockets = WEB_SERVER_MAX_CLIENTS;
    config.stack_size = WEB_SERVER_STACK_SIZE;
    config.lru_purge_enable = true;
    
    httpd_handle_t server = NULL;
    
    // 启动HTTP服务器
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "HTTP server started on port %d", config.server_port);
        
        // 注册URI处理器
        if (web_server_register_handlers(server) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register URI handlers");
            httpd_stop(server);
            return NULL;
        }
        
        ESP_LOGI(TAG, "Web server started successfully");
        return server;
    }
    
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return NULL;
}

/* 注册URI处理器 */
esp_err_t web_server_register_handlers(httpd_handle_t server)
{
    ESP_LOGI(TAG, "Registering URI handlers...");
    
    // 根页面
    httpd_uri_t root_uri = {
        .uri = URI_ROOT,
        .method = HTTP_GET,
        .handler = api_root_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &root_uri);
    
    // 为所有API路径注册OPTIONS处理器 (CORS预检)
    httpd_uri_t options_status_uri = {
        .uri = URI_API_STATUS,
        .method = HTTP_OPTIONS,
        .handler = api_options_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &options_status_uri);
    
    httpd_uri_t options_led_color_uri = {
        .uri = URI_API_LED_COLOR,
        .method = HTTP_OPTIONS,
        .handler = api_options_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &options_led_color_uri);
    
    httpd_uri_t options_led_power_uri = {
        .uri = URI_API_LED_POWER,
        .method = HTTP_OPTIONS,
        .handler = api_options_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &options_led_power_uri);
    
    httpd_uri_t options_led_effect_uri = {
        .uri = URI_API_LED_EFFECT,
        .method = HTTP_OPTIONS,
        .handler = api_options_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &options_led_effect_uri);
    
    // 系统状态API
    httpd_uri_t status_uri = {
        .uri = URI_API_STATUS,
        .method = HTTP_GET,
        .handler = api_status_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &status_uri);
    
    // LED颜色控制API
    httpd_uri_t led_color_uri = {
        .uri = URI_API_LED_COLOR,
        .method = HTTP_POST,
        .handler = api_led_color_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &led_color_uri);
    
    // LED电源控制API
    httpd_uri_t led_power_uri = {
        .uri = URI_API_LED_POWER,
        .method = HTTP_POST,
        .handler = api_led_power_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &led_power_uri);
    
    // LED特效控制API
    httpd_uri_t led_effect_uri = {
        .uri = URI_API_LED_EFFECT,
        .method = HTTP_POST,
        .handler = api_led_effect_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &led_effect_uri);
    
    ESP_LOGI(TAG, "URI handlers registered successfully");
    return ESP_OK;
}

/* 设置CORS头部 */
esp_err_t web_server_set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    
    return ESP_OK;
}

/* 解析JSON请求体 */
cJSON* web_server_parse_json_body(httpd_req_t *req, char* buffer, size_t buffer_size)
{
    int remaining = req->content_len;
    
    if (remaining >= buffer_size) {
        ESP_LOGW(TAG, "Request content too large: %d bytes", remaining);
        return NULL;
    }
    
    int received = httpd_req_recv(req, buffer, remaining);
    if (received <= 0) {
        ESP_LOGW(TAG, "Failed to receive request body");
        return NULL;
    }
    
    buffer[received] = '\0';
    
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        ESP_LOGW(TAG, "Failed to parse JSON: %s", buffer);
    }
    
    return json;
}

/* 发送JSON响应 */
esp_err_t web_server_send_json_response(httpd_req_t *req, cJSON* json)
{
    web_server_set_cors_headers(req);
    httpd_resp_set_type(req, "application/json");
    
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    esp_err_t ret = httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    
    free(json_string);
    return ret;
}

/* 发送错误响应 */
esp_err_t web_server_send_error_response(httpd_req_t *req, httpd_err_code_t status, const char* message)
{
    web_server_set_cors_headers(req);
    httpd_resp_set_type(req, "application/json");
    
    const char* status_str;
    switch(status) {
        case HTTPD_400_BAD_REQUEST: status_str = "400 Bad Request"; break;
        case HTTPD_404_NOT_FOUND: status_str = "404 Not Found"; break;
        case HTTPD_500_INTERNAL_SERVER_ERROR: status_str = "500 Internal Server Error"; break;
        default: status_str = "400 Bad Request"; break;
    }
    httpd_resp_set_status(req, status_str);
    
    cJSON *json = cJSON_CreateObject();
    cJSON *error = cJSON_CreateString(message);
    cJSON_AddItemToObject(json, "error", error);
    
    esp_err_t ret = web_server_send_json_response(req, json);
    
    cJSON_Delete(json);
    return ret;
}

/* 其他函数的简化实现 */
esp_err_t web_server_stop(httpd_handle_t server) {
    if (server) {
        return httpd_stop(server);
    }
    return ESP_ERR_INVALID_ARG;
}

esp_err_t web_server_ws_send_all(const ws_message_t* message) { return ESP_OK; }
esp_err_t web_server_ws_send(int fd, const ws_message_t* message) { return ESP_OK; }
int web_server_get_ws_clients_count(void) { return ws_client_count; }
cJSON* web_server_create_system_info_json(void) { return cJSON_CreateObject(); }
cJSON* web_server_create_wifi_status_json(void) { return cJSON_CreateObject(); }
cJSON* web_server_create_led_status_json(void) { return cJSON_CreateObject(); }