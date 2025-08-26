/*
 * Web Server Implementation for ESP32-S3 LED Controller
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
esp_err_t api_status_handler(httpd_req_t *req);
esp_err_t api_led_color_handler(httpd_req_t *req);
esp_err_t api_led_power_handler(httpd_req_t *req);
esp_err_t api_led_effect_handler(httpd_req_t *req);
esp_err_t api_wifi_connect_handler(httpd_req_t *req);
esp_err_t api_options_handler(httpd_req_t *req);

// 前向声明注册函数
esp_err_t web_server_register_handlers(httpd_handle_t server);

static const char *TAG = "WEB_SERVER";

static httpd_handle_t server = NULL;

/* 启动Web服务器 */
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

/* 注册URI处理器 */
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

    httpd_uri_t wifi_connect_uri = {
        .uri = "/api/wifi/connect",
        .method = HTTP_POST,
        .handler = api_wifi_connect_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &wifi_connect_uri);

    // OPTIONS处理器用于CORS
    httpd_uri_t options_uri = {
        .uri = "/*",
        .method = HTTP_OPTIONS,
        .handler = api_options_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &options_uri);

    ESP_LOGI(TAG, "URI handlers registered successfully");
    return ESP_OK;
}

/* 停止Web服务器 */
esp_err_t web_server_stop(void)
{
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Web server stopped");
    }
    return ESP_OK;
}

/* 获取服务器句柄 */
httpd_handle_t web_server_get_handle(void)
{
    return server;
}
