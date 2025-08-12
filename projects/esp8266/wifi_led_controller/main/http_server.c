/*
 * ESP8266 WiFi LED Controller - HTTP Server & API Handlers
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"

extern const char *TAG;
extern led_state_t current_led_state;
extern void led_start_effect(const char *effect);
extern void led_update_current_color();

/* API状态处理器 */
static esp_err_t api_status_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    cJSON *json = cJSON_CreateObject();
    cJSON *status = cJSON_CreateString("ok");
    cJSON *color = cJSON_CreateObject();
    cJSON *red = cJSON_CreateNumber(current_led_state.red);
    cJSON *green = cJSON_CreateNumber(current_led_state.green);
    cJSON *blue = cJSON_CreateNumber(current_led_state.blue);
    cJSON *brightness = cJSON_CreateNumber(current_led_state.brightness);
    cJSON *power = cJSON_CreateBool(current_led_state.power_on);
    cJSON *effect = cJSON_CreateString(current_led_state.effect_mode);
    
    cJSON_AddItemToObject(color, "red", red);
    cJSON_AddItemToObject(color, "green", green);
    cJSON_AddItemToObject(color, "blue", blue);
    cJSON_AddItemToObject(color, "brightness", brightness);
    
    cJSON_AddItemToObject(json, "status", status);
    cJSON_AddItemToObject(json, "color", color);
    cJSON_AddItemToObject(json, "power", power);
    cJSON_AddItemToObject(json, "effect", effect);
    
    const char *json_string = cJSON_Print(json);
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    
    free((void *)json_string);
    cJSON_Delete(json);
    return ESP_OK;
}

/* API颜色控制处理器 */
static esp_err_t api_color_post_handler(httpd_req_t *req)
{
    char buf[200];
    int ret, remaining = req->content_len;
    
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Content too long");
        return ESP_FAIL;
    }
    
    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    cJSON *json = cJSON_Parse(buf);
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    cJSON *red = cJSON_GetObjectItem(json, "red");
    cJSON *green = cJSON_GetObjectItem(json, "green");
    cJSON *blue = cJSON_GetObjectItem(json, "blue");
    cJSON *brightness = cJSON_GetObjectItem(json, "brightness");
    
    if (red && cJSON_IsNumber(red)) current_led_state.red = red->valueint;
    if (green && cJSON_IsNumber(green)) current_led_state.green = green->valueint;
    if (blue && cJSON_IsNumber(blue)) current_led_state.blue = blue->valueint;
    if (brightness && cJSON_IsNumber(brightness)) current_led_state.brightness = brightness->valueint;
    
    // 停止特效并更新颜色
    led_start_effect("static");
    
    cJSON_Delete(json);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

/* API电源控制处理器 */
static esp_err_t api_power_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;
    
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Content too long");
        return ESP_FAIL;
    }
    
    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    cJSON *json = cJSON_Parse(buf);
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    cJSON *power = cJSON_GetObjectItem(json, "power");
    if (power && cJSON_IsBool(power)) {
        current_led_state.power_on = cJSON_IsTrue(power);
        led_update_current_color();
        ESP_LOGI(TAG, "LED Power: %s", current_led_state.power_on ? "ON" : "OFF");
    }
    
    cJSON_Delete(json);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

/* API特效控制处理器 */
static esp_err_t api_effect_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;
    
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Content too long");
        return ESP_FAIL;
    }
    
    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    cJSON *json = cJSON_Parse(buf);
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    cJSON *effect = cJSON_GetObjectItem(json, "effect");
    if (effect && cJSON_IsString(effect)) {
        led_start_effect(effect->valuestring);
        ESP_LOGI(TAG, "LED Effect: %s", effect->valuestring);
    }
    
    cJSON_Delete(json);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

/* HTTP服务器启动 */
httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.server_port = 80;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        
        // 外部声明的根处理器
        extern esp_err_t root_get_handler(httpd_req_t *req);
        
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler
        };
        httpd_register_uri_handler(server, &root_uri);
        
        httpd_uri_t api_status_uri = {
            .uri       = "/api/status",
            .method    = HTTP_GET,
            .handler   = api_status_get_handler
        };
        httpd_register_uri_handler(server, &api_status_uri);
        
        httpd_uri_t api_color_uri = {
            .uri       = "/api/color",
            .method    = HTTP_POST,
            .handler   = api_color_post_handler
        };
        httpd_register_uri_handler(server, &api_color_uri);
        
        httpd_uri_t api_power_uri = {
            .uri       = "/api/power",
            .method    = HTTP_POST,
            .handler   = api_power_post_handler
        };
        httpd_register_uri_handler(server, &api_power_uri);
        
        httpd_uri_t api_effect_uri = {
            .uri       = "/api/effect",
            .method    = HTTP_POST,
            .handler   = api_effect_post_handler
        };
        httpd_register_uri_handler(server, &api_effect_uri);
        
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}