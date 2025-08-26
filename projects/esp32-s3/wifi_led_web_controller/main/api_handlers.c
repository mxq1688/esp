/*
 * API Handlers Implementation for ESP32-S3 LED Controller
 */

#include "api_handlers.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "cJSON.h"
#include "wifi_manager.h"
#include "led_controller.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "API_HANDLERS";

/* 设置CORS头 */
static void set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
}

/* 根路径处理器 - 返回简单HTML页面 */
esp_err_t api_root_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    httpd_resp_set_type(req, "text/html");
    
    const char* html_content = "<html><body><h1>ESP32-S3 LED Controller</h1><p>API working!</p></body></html>";
    httpd_resp_send(req, html_content, strlen(html_content));
    
    return ESP_OK;
}

/* 状态API处理器 */
esp_err_t api_status_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    // WiFi状态
    cJSON *wifi = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "wifi", wifi);
    cJSON_AddItemToObject(wifi, "connected", cJSON_CreateBool(wifi_is_connected()));
    cJSON_AddItemToObject(wifi, "ap_mode", cJSON_CreateBool(wifi_is_ap_mode()));
    cJSON_AddItemToObject(wifi, "ip", cJSON_CreateString(wifi_get_ip_string()));
    
    // LED状态
    cJSON *led = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "led", led);
    rgb_color_t color = led_get_current_color();
    cJSON_AddItemToObject(led, "power", cJSON_CreateBool(led_get_power_state()));
    cJSON_AddItemToObject(led, "r", cJSON_CreateNumber(color.r));
    cJSON_AddItemToObject(led, "g", cJSON_CreateNumber(color.g));
    cJSON_AddItemToObject(led, "b", cJSON_CreateNumber(color.b));
    cJSON_AddItemToObject(led, "brightness", cJSON_CreateNumber(color.brightness));
    
    // 系统状态
    cJSON_AddItemToObject(root, "uptime", cJSON_CreateNumber(esp_timer_get_time() / 1000000));
    cJSON_AddItemToObject(root, "free_heap", cJSON_CreateNumber(esp_get_free_heap_size()));
    
    char *json_string = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_string, strlen(json_string));
    
    free(json_string);
    cJSON_Delete(root);
    
    return ESP_OK;
}

/* LED颜色API处理器 */
esp_err_t api_led_color_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    
    char content[100];
    int recv_len = httpd_req_recv(req, content, sizeof(content) - 1);
    if (recv_len <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_len] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    rgb_color_t color = {0};
    cJSON *r = cJSON_GetObjectItem(json, "r");
    cJSON *g = cJSON_GetObjectItem(json, "g");
    cJSON *b = cJSON_GetObjectItem(json, "b");
    cJSON *brightness = cJSON_GetObjectItem(json, "brightness");
    
    if (r && g && b && brightness) {
        color.r = r->valueint;
        color.g = g->valueint;
        color.b = b->valueint;
        color.brightness = brightness->valueint;
        
        esp_err_t ret = led_set_color(&color);
        cJSON_Delete(json);
        
        if (ret == ESP_OK) {
            httpd_resp_send(req, "{\"status\":\"ok\"}", strlen("{\"status\":\"ok\"}"));
            return ESP_OK;
        } else {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
    }
    
    cJSON_Delete(json);
    httpd_resp_send_500(req);
    return ESP_FAIL;
}

/* LED电源API处理器 */
esp_err_t api_led_power_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    
    char content[100];
    int recv_len = httpd_req_recv(req, content, sizeof(content) - 1);
    if (recv_len <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_len] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    cJSON *power = cJSON_GetObjectItem(json, "power");
    if (power) {
        esp_err_t ret = led_set_power(power->valueint);
        cJSON_Delete(json);
        
        if (ret == ESP_OK) {
            httpd_resp_send(req, "{\"status\":\"ok\"}", strlen("{\"status\":\"ok\"}"));
            return ESP_OK;
        } else {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
    }
    
    cJSON_Delete(json);
    httpd_resp_send_500(req);
    return ESP_FAIL;
}

/* LED特效API处理器 */
esp_err_t api_led_effect_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    
    char content[100];
    int recv_len = httpd_req_recv(req, content, sizeof(content) - 1);
    if (recv_len <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_len] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    cJSON *effect = cJSON_GetObjectItem(json, "effect");
    if (effect && effect->valuestring) {
        led_effect_t led_effect = LED_EFFECT_NONE;
        
        if (strcmp(effect->valuestring, "rainbow") == 0) {
            led_effect = LED_EFFECT_RAINBOW;
        } else if (strcmp(effect->valuestring, "breath") == 0) {
            led_effect = LED_EFFECT_BREATH;
        } else if (strcmp(effect->valuestring, "blink") == 0) {
            led_effect = LED_EFFECT_BLINK;
        } else if (strcmp(effect->valuestring, "fade") == 0) {
            led_effect = LED_EFFECT_FADE;
        }
        
        esp_err_t ret = led_set_effect(led_effect);
        cJSON_Delete(json);
        
        if (ret == ESP_OK) {
            httpd_resp_send(req, "{\"status\":\"ok\"}", strlen("{\"status\":\"ok\"}"));
            return ESP_OK;
        } else {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
    }
    
    cJSON_Delete(json);
    httpd_resp_send_500(req);
    return ESP_FAIL;
}

/* WiFi连接API处理器 */
esp_err_t api_wifi_connect_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    
    char content[200];
    int recv_len = httpd_req_recv(req, content, sizeof(content) - 1);
    if (recv_len <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_len] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
    cJSON *password = cJSON_GetObjectItem(json, "password");
    
    if (ssid && password && ssid->valuestring && password->valuestring) {
        esp_err_t ret = wifi_connect_sta(ssid->valuestring, password->valuestring);
        cJSON_Delete(json);
        
        if (ret == ESP_OK) {
            httpd_resp_send(req, "{\"status\":\"ok\"}", strlen("{\"status\":\"ok\"}"));
            return ESP_OK;
        } else {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
    }
    
    cJSON_Delete(json);
    httpd_resp_send_500(req);
    return ESP_FAIL;
}

/* OPTIONS请求处理器 */
esp_err_t api_options_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    httpd_resp_send(req, "", 0);
    return ESP_OK;
}
