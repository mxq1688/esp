#include "web_server.h"
#include "api_handlers.h"
#include "web_files.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string.h>

static const char *TAG = "WebServer";

static httpd_handle_t server = NULL;

// 静态文件处理器
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Expires", "0");
    
    return httpd_resp_send(req, web_index_html_start, WEB_INDEX_HTML_SIZE);
}

static esp_err_t style_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=31536000");
    
    return httpd_resp_send(req, web_style_css_start, WEB_STYLE_CSS_SIZE);
}

static esp_err_t script_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=31536000");
    
    return httpd_resp_send(req, web_script_js_start, WEB_SCRIPT_JS_SIZE);
}

// URI处理器配置
static const httpd_uri_t uri_handlers[] = {
    // 静态文件
    {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/index.html",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/style.css",
        .method    = HTTP_GET,
        .handler   = style_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/script.js",
        .method    = HTTP_GET,
        .handler   = script_handler,
        .user_ctx  = NULL
    },
    
    // API接口
    {
        .uri       = "/api/status",
        .method    = HTTP_GET,
        .handler   = api_status_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/api/network/info",
        .method    = HTTP_GET,
        .handler   = api_network_info_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/api/hotspot/control",
        .method    = HTTP_POST,
        .handler   = api_hotspot_control_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/api/hotspot/config",
        .method    = HTTP_GET,
        .handler   = api_hotspot_config_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/api/hotspot/config",
        .method    = HTTP_POST,
        .handler   = api_hotspot_config_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/api/ml307r/reset",
        .method    = HTTP_POST,
        .handler   = api_ml307r_reset_handler,
        .user_ctx  = NULL
    },
    {
        .uri       = "/api/wifi/connect",
        .method    = HTTP_POST,
        .handler   = api_wifi_connect_handler,
        .user_ctx  = NULL
    }
};

esp_err_t web_server_start(void)
{
    if (server != NULL) {
        ESP_LOGW(TAG, "Web server already started");
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = sizeof(uri_handlers) / sizeof(uri_handlers[0]);
    config.lru_purge_enable = true;
    config.stack_size = 16384;  // 增加栈大小
    config.max_resp_headers = 8;  // 减少响应头数量
    config.max_open_sockets = 7;  // 减少最大连接数

    ESP_LOGI(TAG, "Starting web server on port %d", config.server_port);

    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start web server: %s", esp_err_to_name(ret));
        return ret;
    }

    // 注册所有URI处理器
    for (int i = 0; i < sizeof(uri_handlers) / sizeof(uri_handlers[0]); i++) {
        ret = httpd_register_uri_handler(server, &uri_handlers[i]);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register URI handler for %s: %s", 
                     uri_handlers[i].uri, esp_err_to_name(ret));
            httpd_stop(server);
            server = NULL;
            return ret;
        }
    }

    ESP_LOGI(TAG, "Web server started successfully with %d handlers", 
             sizeof(uri_handlers) / sizeof(uri_handlers[0]));
    
    return ESP_OK;
}

esp_err_t web_server_stop(void)
{
    if (server == NULL) {
        ESP_LOGW(TAG, "Web server not running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping web server...");
    
    esp_err_t ret = httpd_stop(server);
    if (ret == ESP_OK) {
        server = NULL;
        ESP_LOGI(TAG, "Web server stopped");
    } else {
        ESP_LOGE(TAG, "Failed to stop web server: %s", esp_err_to_name(ret));
    }

    return ret;
}

bool web_server_is_running(void)
{
    return server != NULL;
}
