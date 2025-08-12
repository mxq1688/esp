/*
 * ESP8266 WiFi LED Controller - Additional Functions
 * 
 * 包含WiFi事件处理、LED控制函数和HTTP API处理器
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/pwm.h"
#include "esp_http_server.h"
#include "cJSON.h"

extern const char *TAG;
extern EventGroupHandle_t s_wifi_event_group;
extern led_state_t current_led_state;
extern httpd_handle_t server;
extern int s_retry_num;
extern uint32_t pwm_duties[3];
extern TaskHandle_t effect_task_handle;
extern bool effect_running;

/* WiFi事件处理 */
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station join, AID=%d", event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station leave, AID=%d", event->aid);
    }
}

/* LED控制函数 */
static void led_set_color(uint16_t red, uint16_t green, uint16_t blue, uint8_t brightness)
{
    if (!current_led_state.power_on) {
        red = green = blue = 0;
    } else {
        // 应用亮度调节
        red = (red * brightness) / 100;
        green = (green * brightness) / 100;
        blue = (blue * brightness) / 100;
    }
    
    // 转换为PWM占空比 (0-1023)
    pwm_duties[0] = (red * PWM_MAX_DUTY) / 255;     // 红色
    pwm_duties[1] = (green * PWM_MAX_DUTY) / 255;   // 绿色  
    pwm_duties[2] = (blue * PWM_MAX_DUTY) / 255;    // 蓝色
    
    // 设置PWM输出
    pwm_set_duties(pwm_duties);
    pwm_start();
    
    ESP_LOGI(TAG, "LED Color: R=%d, G=%d, B=%d, Brightness=%d%%", 
             red, green, blue, brightness);
}

static void led_update_current_color()
{
    led_set_color(current_led_state.red, current_led_state.green, 
                  current_led_state.blue, current_led_state.brightness);
}

/* HSV到RGB转换 */
static void hsv_to_rgb(float h, float s, float v, uint16_t *r, uint16_t *g, uint16_t *b)
{
    int i = (int)h;
    float f = h - i;
    float p = v * (1 - s);
    float q = v * (1 - s * f);
    float t = v * (1 - s * (1 - f));
    
    switch (i % 6) {
        case 0: *r = v * 255; *g = t * 255; *b = p * 255; break;
        case 1: *r = q * 255; *g = v * 255; *b = p * 255; break;
        case 2: *r = p * 255; *g = v * 255; *b = t * 255; break;
        case 3: *r = p * 255; *g = q * 255; *b = v * 255; break;
        case 4: *r = t * 255; *g = p * 255; *b = v * 255; break;
        case 5: *r = v * 255; *g = p * 255; *b = q * 255; break;
    }
}

/* LED特效任务 */
static void led_effect_task(void *pvParameters)
{
    uint16_t hue = 0;
    uint16_t brightness_val = 10;
    int8_t brightness_dir = 1;
    uint16_t r, g, b;
    
    while (effect_running) {
        if (strcmp(current_led_state.effect_mode, "rainbow") == 0) {
            // 彩虹循环效果
            hsv_to_rgb(hue / 360.0 * 6, 1.0, 1.0, &r, &g, &b);
            led_set_color(r, g, b, current_led_state.brightness);
            hue = (hue + 2) % 360;
            vTaskDelay(50 / portTICK_PERIOD_MS);
            
        } else if (strcmp(current_led_state.effect_mode, "breathing") == 0) {
            // 呼吸灯效果
            led_set_color(current_led_state.red, current_led_state.green, 
                         current_led_state.blue, brightness_val);
            brightness_val += brightness_dir * 2;
            if (brightness_val >= 100) {
                brightness_val = 100;
                brightness_dir = -1;
            } else if (brightness_val <= 10) {
                brightness_val = 10;
                brightness_dir = 1;
            }
            vTaskDelay(30 / portTICK_PERIOD_MS);
            
        } else if (strcmp(current_led_state.effect_mode, "blink") == 0) {
            // 闪烁效果
            static bool blink_state = false;
            if (blink_state) {
                led_set_color(current_led_state.red, current_led_state.green, 
                             current_led_state.blue, current_led_state.brightness);
            } else {
                led_set_color(0, 0, 0, 0);
            }
            blink_state = !blink_state;
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            // 无效特效，停止任务
            break;
        }
    }
    
    // 恢复到静态模式
    strcpy(current_led_state.effect_mode, "static");
    led_update_current_color();
    effect_task_handle = NULL;
    vTaskDelete(NULL);
}

static void led_start_effect(const char *effect)
{
    // 停止当前特效
    if (effect_task_handle != NULL) {
        effect_running = false;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    strcpy(current_led_state.effect_mode, effect);
    
    if (strcmp(effect, "static") != 0) {
        effect_running = true;
        xTaskCreate(led_effect_task, "led_effect", 2048, NULL, 5, &effect_task_handle);
        ESP_LOGI(TAG, "Started LED effect: %s", effect);
    } else {
        led_update_current_color();
    }
}

/* HTTP处理函数 */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    const char* html_response = 
        "<!DOCTYPE html>"
        "<html><head><title>ESP8266 LED Controller</title>"
        "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<style>body{font-family:Arial;text-align:center;margin:50px;background:#f0f0f0;}"
        ".container{max-width:600px;margin:0 auto;background:white;padding:30px;border-radius:15px;box-shadow:0 4px 6px rgba(0,0,0,0.1);}"
        ".control{margin:20px;padding:20px;border:1px solid #ddd;border-radius:10px;background:#f9f9f9;}"
        "input[type='range']{width:280px;margin:10px;}button{padding:12px 24px;margin:8px;border:none;border-radius:8px;cursor:pointer;font-size:14px;}"
        ".power-on{background:#4CAF50;color:white;}.power-off{background:#f44336;color:white;}"
        ".effect-btn{background:#2196F3;color:white;}.static-btn{background:#FF9800;color:white;}"
        "#preview{width:120px;height:120px;margin:20px auto;border-radius:60px;border:3px solid #ddd;}"
        ".rgb-control{display:flex;align-items:center;margin:10px 0;}"
        ".rgb-control label{width:80px;text-align:left;margin-right:10px;}"
        ".rgb-control span{width:40px;text-align:right;margin-left:10px;font-weight:bold;}"
        "h1{color:#333;margin-bottom:30px;}h3{color:#555;margin:15px 0;}"
        "</style></head><body>"
        "<div class='container'>"
        "<h1>🌈 ESP8266 LED 控制器</h1>"
        "<div id='preview' style='background:rgb(255,255,255);'></div>"
        "<div class='control'>"
        "<h3>🎨 RGB颜色控制</h3>"
        "<div class='rgb-control'>R: <input type='range' id='r' min='0' max='255' value='255' oninput='updateColor()'><span id='r-val'>255</span></div>"
        "<div class='rgb-control'>G: <input type='range' id='g' min='0' max='255' value='255' oninput='updateColor()'><span id='g-val'>255</span></div>"
        "<div class='rgb-control'>B: <input type='range' id='b' min='0' max='255' value='255' oninput='updateColor()'><span id='b-val'>255</span></div>"
        "<div class='rgb-control'>亮度: <input type='range' id='brightness' min='0' max='100' value='50' oninput='updateColor()'><span id='brightness-val'>50%</span></div>"
        "</div>"
        "<div class='control'>"
        "<h3>⚡ 电源控制</h3>"
        "<button class='power-on' onclick='setPower(true)'>🔴 开启LED</button>"
        "<button class='power-off' onclick='setPower(false)'>⚫ 关闭LED</button>"
        "</div>"
        "<div class='control'>"
        "<h3>✨ 特效模式</h3>"
        "<button class='effect-btn' onclick='setEffect(\"rainbow\")'>🌈 彩虹循环</button>"
        "<button class='effect-btn' onclick='setEffect(\"breathing\")'>💨 呼吸灯</button>"
        "<button class='effect-btn' onclick='setEffect(\"blink\")'>⚡ 闪烁</button>"
        "<button class='static-btn' onclick='setEffect(\"static\")'>⏹️ 静态模式</button>"
        "</div>"
        "<div style='margin-top:30px;color:#666;font-size:12px;'>"
        "<p>🔧 ESP8266 WiFi LED Controller v1.0</p>"
        "<p>📡 连接状态: <span id='status'>连接中...</span></p>"
        "</div>"
        "</div>"
        "<script>"
        "function updateColor(){"
        "var r=document.getElementById('r').value;"
        "var g=document.getElementById('g').value;"
        "var b=document.getElementById('b').value;"
        "var brightness=document.getElementById('brightness').value;"
        "document.getElementById('r-val').textContent=r;"
        "document.getElementById('g-val').textContent=g;"
        "document.getElementById('b-val').textContent=b;"
        "document.getElementById('brightness-val').textContent=brightness+'%';"
        "document.getElementById('preview').style.background='rgb('+r+','+g+','+b+')';"
        "fetch('/api/color',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({red:parseInt(r),green:parseInt(g),blue:parseInt(b),brightness:parseInt(brightness)})}).then(()=>{"
        "document.getElementById('status').textContent='已连接';});"
        "}"
        "function setPower(on){"
        "fetch('/api/power',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({power:on})}).then(()=>{"
        "document.getElementById('status').textContent='已连接';"
        "});"
        "}"
        "function setEffect(effect){"
        "fetch('/api/effect',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({effect:effect})}).then(()=>{"
        "document.getElementById('status').textContent='已连接';"
        "});"
        "}"
        "setInterval(()=>{"
        "fetch('/api/status').then(r=>r.json()).then(data=>{"
        "if(data.status==='ok'){document.getElementById('status').textContent='已连接';}"
        "}).catch(()=>{document.getElementById('status').textContent='连接断开';});"
        "},3000);"
        "</script></body></html>";
    
    return httpd_resp_send(req, html_response, HTTPD_RESP_USE_STRLEN);
}