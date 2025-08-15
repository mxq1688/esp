/*
 * API Handlers for ESP32-C3 LED Controller
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "led_controller.h"
#include "web_files.h"
#include "web_server.h"

static const char *TAG = "API_HANDLERS";

// 移除SPIFFS相关代码，直接使用嵌入的文件
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "identity");
    httpd_resp_send(req, (const char *)index_html, get_index_html_size());
    return ESP_OK;
}

static esp_err_t style_css_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Content-Encoding", "identity");
    httpd_resp_send(req, (const char *)style_css, get_style_css_size());
    return ESP_OK;
}

static esp_err_t script_js_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_set_hdr(req, "Content-Encoding", "identity");
    httpd_resp_send(req, (const char *)script_js, get_script_js_size());
    return ESP_OK;
}

static esp_err_t manifest_json_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Content-Encoding", "identity");
    httpd_resp_send(req, (const char *)manifest_json, get_manifest_json_size());
    return ESP_OK;
}

static esp_err_t sw_js_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_set_hdr(req, "Content-Encoding", "identity");
    httpd_resp_send(req, (const char *)sw_js, get_sw_js_size());
    return ESP_OK;
}

// CORS预检请求处理器
esp_err_t api_options_handler(httpd_req_t *req)
{
    web_server_set_cors_headers(req);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// 系统状态API
esp_err_t api_status_handler(httpd_req_t *req)
{
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return ESP_FAIL;
    }

    // 添加系统信息
    cJSON_AddStringToObject(json, "status", "ok");
    cJSON_AddNumberToObject(json, "uptime", esp_timer_get_time() / 1000000);
    cJSON_AddStringToObject(json, "version", "2.0.0");

    esp_err_t ret = web_server_send_json_response(req, json);
    cJSON_Delete(json);
    return ret;
}

// LED颜色控制API
esp_err_t api_led_color_handler(httpd_req_t *req)
{
    char buffer[256];
    cJSON *json = (cJSON*)web_server_parse_json_body(req, buffer, sizeof(buffer));
    if (!json) {
        return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    cJSON *r = cJSON_GetObjectItem(json, "r");
    cJSON *g = cJSON_GetObjectItem(json, "g");
    cJSON *b = cJSON_GetObjectItem(json, "b");

    if (r && g && b && cJSON_IsNumber(r) && cJSON_IsNumber(g) && cJSON_IsNumber(b)) {
        uint8_t red = (uint8_t)r->valueint;
        uint8_t green = (uint8_t)g->valueint;
        uint8_t blue = (uint8_t)b->valueint;
        
        led_set_rgb(red, green, blue);
        
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddStringToObject(response, "message", "Color updated");
        
        esp_err_t ret = web_server_send_json_response(req, response);
        cJSON_Delete(response);
        cJSON_Delete(json);
        return ret;
    }

    cJSON_Delete(json);
    return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid color values");
}

// LED电源控制API
esp_err_t api_led_power_handler(httpd_req_t *req)
{
    char buffer[256];
    cJSON *json = (cJSON*)web_server_parse_json_body(req, buffer, sizeof(buffer));
    if (!json) {
        return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    cJSON *power = cJSON_GetObjectItem(json, "power");
    if (power && cJSON_IsBool(power)) {
        bool is_on = cJSON_IsTrue(power);
        led_set_power(is_on);
        
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddStringToObject(response, "message", is_on ? "LED turned on" : "LED turned off");
        
        esp_err_t ret = web_server_send_json_response(req, response);
        cJSON_Delete(response);
        cJSON_Delete(json);
        return ret;
    }

    cJSON_Delete(json);
    return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid power value");
}

// LED特效控制API
esp_err_t api_led_effect_handler(httpd_req_t *req)
{
    char buffer[256];
    cJSON *json = (cJSON*)web_server_parse_json_body(req, buffer, sizeof(buffer));
    if (!json) {
        return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    cJSON *effect = cJSON_GetObjectItem(json, "effect");
    if (effect && cJSON_IsString(effect)) {
        const char *effect_name = effect->valuestring;
        
        // 根据效果名称设置特效
        led_effect_type_t effect_type = LED_EFFECT_STATIC;
        if (strcmp(effect_name, "rainbow") == 0) {
            effect_type = LED_EFFECT_RAINBOW;
        } else if (strcmp(effect_name, "breathing") == 0) {
            effect_type = LED_EFFECT_BREATHING;
        } else if (strcmp(effect_name, "blink") == 0) {
            effect_type = LED_EFFECT_BLINK;
        }
        
        led_start_effect(effect_type, 50); // 默认速度50
        
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddStringToObject(response, "message", "Effect updated");
        
        esp_err_t ret = web_server_send_json_response(req, response);
        cJSON_Delete(response);
        cJSON_Delete(json);
        return ret;
    }

    cJSON_Delete(json);
    return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid effect value");
}

// 根路径处理器
esp_err_t api_root_handler(httpd_req_t *req)
{
    return root_get_handler(req);
}

// CSS文件处理器
esp_err_t api_style_handler(httpd_req_t *req)
{
    return style_css_get_handler(req);
}

// JavaScript文件处理器
esp_err_t api_script_handler(httpd_req_t *req)
{
    return script_js_get_handler(req);
}

// Manifest文件处理器
esp_err_t api_manifest_handler(httpd_req_t *req)
{
    return manifest_json_get_handler(req);
}

// Service Worker文件处理器
esp_err_t api_sw_handler(httpd_req_t *req)
{
    return sw_js_get_handler(req);
}

// 错误响应创建函数
cJSON* api_create_error_response(api_error_code_t error_code, const char* message)
{
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    cJSON_AddStringToObject(json, "status", "error");
    cJSON_AddNumberToObject(json, "code", error_code);
    cJSON_AddStringToObject(json, "message", message);

    return json;
}

void api_log_request(httpd_req_t *req, const char* api_name)
{
    ESP_LOGI(TAG, "API [%s] called", api_name);
}

esp_err_t api_get_client_ip(httpd_req_t *req, char* ip_str, size_t max_len)
{
    strncpy(ip_str, "0.0.0.0", max_len);
    return ESP_OK;
}

/* 简化存根函数 */
esp_err_t api_system_info_handler(httpd_req_t *req) { return api_status_handler(req); }
esp_err_t api_system_reset_handler(httpd_req_t *req) { esp_restart(); return ESP_OK; }
esp_err_t api_wifi_status_handler(httpd_req_t *req) { return api_status_handler(req); }
esp_err_t api_wifi_scan_handler(httpd_req_t *req) { return api_status_handler(req); }
esp_err_t api_wifi_connect_handler(httpd_req_t *req) { return api_status_handler(req); }
esp_err_t api_led_status_handler(httpd_req_t *req) { return api_status_handler(req); }
esp_err_t api_led_brightness_handler(httpd_req_t *req) { return api_status_handler(req); }
esp_err_t api_websocket_handler(httpd_req_t *req) { return ESP_OK; }
bool api_validate_json_params(cJSON* json, const char* required_fields[], int field_count) { return true; }
bool api_check_authorization(httpd_req_t *req) { return true; }