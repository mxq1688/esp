/*
 * API Handlers for ESP32-C3 LED Controller
 */

#include "api_handlers.h"
#include "web_server.h"
#include "wifi_manager.h"
#include "led_controller.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "API_HANDLERS";

/* OPTIONS预检请求处理器 */
esp_err_t api_options_handler(httpd_req_t *req)
{
    web_server_set_cors_headers(req);
    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/* 根页面处理器 - 现代化Web界面 */
esp_err_t api_root_handler(httpd_req_t *req)
{
    web_server_set_cors_headers(req);
    httpd_resp_set_type(req, "text/html");
    
    const char* html = 
        "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32-C3 LED控制器</title>"
        "<style>body{font-family:Arial;background:#f0f2f5;margin:0;padding:20px;box-sizing:border-box;}"
        ".container{max-width:400px;margin:0 auto;background:white;padding:30px;border-radius:15px;box-shadow:0 4px 12px rgba(0,0,0,0.1);}"
        "h1{text-align:center;color:#333;margin-bottom:30px;font-size:24px;}"
        ".control{margin:20px 0;padding:15px;background:#f8f9fa;border-radius:10px;}"
        ".slider{width:100%;margin:10px 0;}"
        "input[type='range']{width:100%;height:6px;border-radius:3px;outline:none;}"
        ".btn{padding:10px 20px;margin:5px;border:none;border-radius:8px;cursor:pointer;font-weight:bold;width:calc(50% - 10px);box-sizing:border-box;}"
        ".btn-power{background:#28a745;color:white;}"
        ".btn-effect{background:#007bff;color:white;}"
        "#preview{width:60px;height:60px;border-radius:50%;margin:15px auto;border:2px solid #ddd;}"
        ".value{float:right;font-weight:bold;color:#007bff;}"
        "@media (max-width: 600px) {"
        "body{padding:10px;}"
        ".container{padding:20px;border-radius:10px;}"
        "h1{font-size:20px;margin-bottom:20px;}"
        ".btn{width:calc(100% - 10px);display:block;margin:10px auto;}"
        "}"
        "</style></head><body>"
        "<div class='container'><h1>🌈 ESP32-C3 LED控制器</h1>"
        "<div id='preview' style='background:rgb(255,255,255);'></div>"
        "<div class='control'><h3>颜色控制</h3>"
        "<label>红色 <span class='value' id='r-val'>255</span></label>"
        "<input type='range' id='r' min='0' max='255' value='255' oninput='updateColor()'><br>"
        "<label>绿色 <span class='value' id='g-val'>255</span></label>"
        "<input type='range' id='g' min='0' max='255' value='255' oninput='updateColor()'><br>"
        "<label>蓝色 <span class='value' id='b-val'>255</span></label>"
        "<input type='range' id='b' min='0' max='255' value='255' oninput='updateColor()'><br>"
        "<label>亮度 <span class='value' id='brightness-val'>50%</span></label>"
        "<input type='range' id='brightness' min='0' max='100' value='50' oninput='updateColor()'>"
        "</div>"
        "<div class='control'><h3>控制</h3>"
        "<button class='btn btn-power' onclick='togglePower()' id='power-btn'>开启LED</button><br>"
        "<button class='btn btn-effect' onclick='setEffect(\"rainbow\")'>彩虹</button>"
        "<button class='btn btn-effect' onclick='setEffect(\"breathing\")'>呼吸</button>"
        "<button class='btn btn-effect' onclick='setEffect(\"blink\")'>闪烁</button>"
        "<button class='btn btn-effect' onclick='setEffect(\"static\")'>静态</button>"
        "</div></div>"
        "<script>"
        "let powerState=false;"
        "function updateColor(){"
        "const r=document.getElementById('r').value;"
        "const g=document.getElementById('g').value;"
        "const b=document.getElementById('b').value;"
        "const brightness=document.getElementById('brightness').value;"
        "document.getElementById('r-val').textContent=r;"
        "document.getElementById('g-val').textContent=g;"
        "document.getElementById('b-val').textContent=b;"
        "document.getElementById('brightness-val').textContent=brightness+'%';"
        "document.getElementById('preview').style.background=`rgb(${r},${g},${b})`;"
        "fetch('/api/led/color',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({red:parseInt(r),green:parseInt(g),blue:parseInt(b),brightness:parseInt(brightness)})});"
        "}"
        "function togglePower(){"
        "powerState=!powerState;"
        "document.getElementById('power-btn').textContent=powerState?'关闭LED':'开启LED';"
        "fetch('/api/led/power',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({power:powerState})});"
        "}"
        "function setEffect(effect){"
        "fetch('/api/led/effect',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({effect:effect,speed:50})});"
        "}"
        "</script></body></html>";
    
    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}

/* 系统状态API处理器 */
esp_err_t api_status_handler(httpd_req_t *req)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "ok");
    cJSON_AddStringToObject(json, "device", "ESP32-C3");
    cJSON_AddNumberToObject(json, "uptime", esp_timer_get_time() / 1000000);
    cJSON_AddNumberToObject(json, "free_heap", esp_get_free_heap_size());
    
    esp_err_t ret = web_server_send_json_response(req, json);
    cJSON_Delete(json);
    return ret;
}

/* LED颜色控制API */
esp_err_t api_led_color_handler(httpd_req_t *req)
{
    char buffer[200];
    cJSON *json = web_server_parse_json_body(req, buffer, sizeof(buffer));
    
    if (json == NULL) {
        return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }
    
    cJSON *red = cJSON_GetObjectItem(json, "red");
    cJSON *green = cJSON_GetObjectItem(json, "green");
    cJSON *blue = cJSON_GetObjectItem(json, "blue");
    cJSON *brightness = cJSON_GetObjectItem(json, "brightness");
    
    if (red && green && blue) {
        led_set_rgb(red->valueint, green->valueint, blue->valueint);
        if (brightness) {
            led_set_brightness(brightness->valueint);
        }
    }
    
    cJSON_Delete(json);
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    esp_err_t ret = web_server_send_json_response(req, response);
    cJSON_Delete(response);
    
    return ret;
}

/* LED电源控制API */
esp_err_t api_led_power_handler(httpd_req_t *req)
{
    char buffer[100];
    cJSON *json = web_server_parse_json_body(req, buffer, sizeof(buffer));
    
    if (json == NULL) {
        return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }
    
    cJSON *power = cJSON_GetObjectItem(json, "power");
    if (power && cJSON_IsBool(power)) {
        led_set_power(cJSON_IsTrue(power));
    }
    
    cJSON_Delete(json);
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    esp_err_t ret = web_server_send_json_response(req, response);
    cJSON_Delete(response);
    
    return ret;
}

/* LED特效控制API */
esp_err_t api_led_effect_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Received request for /api/led/effect, Method: %d", req->method);
    char buffer[150];
    cJSON *json = web_server_parse_json_body(req, buffer, sizeof(buffer));
    
    if (json == NULL) {
        return web_server_send_error_response(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }
    
    cJSON *effect = cJSON_GetObjectItem(json, "effect");
    cJSON *speed = cJSON_GetObjectItem(json, "speed");
    
    if (effect && cJSON_IsString(effect)) {
        const char* effect_name = effect->valuestring;
        uint16_t effect_speed = speed ? speed->valueint : 50;
        
        led_effect_type_t effect_type = LED_EFFECT_STATIC;
        if (strcmp(effect_name, "rainbow") == 0) {
            effect_type = LED_EFFECT_RAINBOW;
        } else if (strcmp(effect_name, "breathing") == 0) {
            effect_type = LED_EFFECT_BREATHING;
        } else if (strcmp(effect_name, "blink") == 0) {
            effect_type = LED_EFFECT_BLINK;
        }
        
        led_start_effect(effect_type, effect_speed);
    }
    
    cJSON_Delete(json);
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    esp_err_t ret = web_server_send_json_response(req, response);
    cJSON_Delete(response);
    
    return ret;
}



/* 辅助函数 */
cJSON* api_create_success_response(cJSON* data)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "success");
    if (data) cJSON_AddItemToObject(json, "data", data);
    return json;
}

cJSON* api_create_error_response(api_error_code_t error_code, const char* message)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "error");
    cJSON_AddNumberToObject(json, "error_code", error_code);
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