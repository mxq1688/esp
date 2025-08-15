/*
 * Web Server Implementation for ESP32-C3 LED Controller
 */

#include "web_server.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include <string.h>

// 前向声明API处理器
esp_err_t api_root_handler(httpd_req_t *req);
esp_err_t api_style_handler(httpd_req_t *req);
esp_err_t api_script_handler(httpd_req_t *req);
esp_err_t api_manifest_handler(httpd_req_t *req);
esp_err_t api_sw_handler(httpd_req_t *req);
esp_err_t api_status_handler(httpd_req_t *req);
esp_err_t api_led_color_handler(httpd_req_t *req);
esp_err_t api_led_power_handler(httpd_req_t *req);
esp_err_t api_led_effect_handler(httpd_req_t *req);
esp_err_t api_options_handler(httpd_req_t *req);

static const char *TAG = "WEB_SERVER";

static httpd_handle_t server = NULL;

// 移除SPIFFS初始化
esp_err_t web_server_start(void)
{
    ESP_LOGI(TAG, "Starting web server...");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 16;

    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Error starting server!");
        return ESP_FAIL;
    }

    // 注册URI处理器
    if (web_server_register_handlers(server) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register URI handlers");
        httpd_stop(server);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Web server started successfully");
    return ESP_OK;
}

// 注册URI处理器
esp_err_t web_server_register_handlers(httpd_handle_t server)
{
    // 静态文件处理器
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = api_root_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &root_uri);

    httpd_uri_t style_uri = {
        .uri = "/style.css",
        .method = HTTP_GET,
        .handler = api_style_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &style_uri);

    httpd_uri_t script_uri = {
        .uri = "/script.js",
        .method = HTTP_GET,
        .handler = api_script_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &script_uri);

    httpd_uri_t manifest_uri = {
        .uri = "/manifest.json",
        .method = HTTP_GET,
        .handler = api_manifest_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &manifest_uri);

    httpd_uri_t sw_uri = {
        .uri = "/sw.js",
        .method = HTTP_GET,
        .handler = api_sw_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &sw_uri);

    // API处理器
    httpd_uri_t status_uri = {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = api_status_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &status_uri);

    httpd_uri_t led_color_uri = {
        .uri = "/api/led/color",
        .method = HTTP_POST,
        .handler = api_led_color_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &led_color_uri);

    httpd_uri_t led_power_uri = {
        .uri = "/api/led/power",
        .method = HTTP_POST,
        .handler = api_led_power_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &led_power_uri);

    httpd_uri_t led_effect_uri = {
        .uri = "/api/led/effect",
        .method = HTTP_POST,
        .handler = api_led_effect_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &led_effect_uri);

    // CORS预检请求
    httpd_uri_t options_uri = {
        .uri = "/api/*",
        .method = HTTP_OPTIONS,
        .handler = api_options_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &options_uri);

    return ESP_OK;
}

// 设置CORS头部
void web_server_set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400");
}

// 发送JSON响应
esp_err_t web_server_send_json_response(httpd_req_t *req, void *json)
{
    web_server_set_cors_headers(req);
    httpd_resp_set_type(req, "application/json");
    
    char *json_str = cJSON_Print((cJSON*)json);
    if (!json_str) {
        return ESP_FAIL;
    }
    
    esp_err_t ret = httpd_resp_send(req, json_str, strlen(json_str));
    free(json_str);
    return ret;
}

// 解析JSON请求体
void* web_server_parse_json_body(httpd_req_t *req, char *buffer, size_t buffer_size)
{
    int received = httpd_req_recv(req, buffer, buffer_size - 1);
    if (received <= 0) {
        return NULL;
    }
    
    buffer[received] = '\0';
    return cJSON_Parse(buffer);
}

// 发送错误响应
esp_err_t web_server_send_error_response(httpd_req_t *req, httpd_err_code_t error, const char *message)
{
    web_server_set_cors_headers(req);
    httpd_resp_set_type(req, "application/json");
    
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "error");
    cJSON_AddNumberToObject(json, "code", error);
    cJSON_AddStringToObject(json, "message", message);
    
    char *json_str = cJSON_Print(json);
    cJSON_Delete(json);
    
    if (!json_str) {
        return ESP_FAIL;
    }
    
    esp_err_t ret = httpd_resp_send(req, json_str, strlen(json_str));
    free(json_str);
    return ret;
}