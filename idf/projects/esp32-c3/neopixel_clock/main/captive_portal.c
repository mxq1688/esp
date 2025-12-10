/**
 * @file captive_portal.c
 * @brief Captive Portal for WiFi configuration via web interface
 */

#include "captive_portal.h"
#include "light_show.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lwip/inet.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static const char *TAG = "captive_portal";

// NVS namespace for WiFi credentials
#define NVS_NAMESPACE "wifi_config"
#define NVS_KEY_SSID "ssid"
#define NVS_KEY_PASSWORD "password"

// State
static bool s_portal_running = false;
static bool s_new_config_available = false;
static char s_configured_ssid[33] = {0};
static char s_configured_password[65] = {0};
static httpd_handle_t s_server = NULL;
static esp_netif_t *s_ap_netif = NULL;

// HTML 页面
static const char *HTML_PAGE = 
"<!DOCTYPE html>"
"<html><head>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<title>NeoPixel Clock WiFi 配置</title>"
"<style>"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;"
"background:linear-gradient(135deg,#1a1a2e 0%%,#16213e 50%%,#0f3460 100%%);"
"min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px}"
".card{background:rgba(255,255,255,0.95);border-radius:20px;padding:40px;width:100%%;"
"max-width:400px;box-shadow:0 25px 50px rgba(0,0,0,0.3)}"
"h1{color:#1a1a2e;font-size:24px;margin-bottom:8px;text-align:center}"
".subtitle{color:#666;font-size:14px;text-align:center;margin-bottom:30px}"
".form-group{margin-bottom:20px}"
"label{display:block;color:#333;font-size:14px;font-weight:500;margin-bottom:8px}"
"input{width:100%%;padding:14px 16px;border:2px solid #e0e0e0;border-radius:12px;"
"font-size:16px;transition:all 0.3s}"
"input:focus{outline:none;border-color:#0f3460;box-shadow:0 0 0 3px rgba(15,52,96,0.1)}"
"button{width:100%%;padding:16px;background:linear-gradient(135deg,#0f3460,#1a1a2e);"
"color:white;border:none;border-radius:12px;font-size:16px;font-weight:600;"
"cursor:pointer;transition:transform 0.2s,box-shadow 0.2s}"
"button:hover{transform:translateY(-2px);box-shadow:0 10px 20px rgba(0,0,0,0.2)}"
"button:active{transform:translateY(0)}"
".icon{font-size:48px;text-align:center;margin-bottom:20px}"
".success{background:#d4edda;color:#155724;padding:15px;border-radius:12px;"
"text-align:center;margin-bottom:20px}"
".error{background:#f8d7da;color:#721c24;padding:15px;border-radius:12px;"
"text-align:center;margin-bottom:20px}"
".networks{margin-bottom:20px;max-height:200px;overflow-y:auto}"
".network{padding:12px;border:2px solid #e0e0e0;border-radius:10px;margin-bottom:8px;"
"cursor:pointer;transition:all 0.2s;display:flex;justify-content:space-between;align-items:center}"
".network:hover{border-color:#0f3460;background:#f8f9fa}"
".network.selected{border-color:#0f3460;background:#e8f0fe}"
".signal{color:#666;font-size:12px}"
"</style></head><body>"
"<div class='card'>"
"<div class='icon'>⏰</div>"
"<h1>NeoPixel Clock</h1>"
"<p class='subtitle'>WiFi 网络配置</p>"
"<div id='msg'></div>"
"<form id='form' action='/save' method='POST'>"
"<div class='form-group'>"
"<label>WiFi 名称 (SSID)</label>"
"<input type='text' name='ssid' id='ssid' placeholder='输入或选择WiFi名称' required maxlength='32'>"
"</div>"
"<div class='form-group'>"
"<label>WiFi 密码</label>"
"<input type='password' name='password' id='password' placeholder='输入WiFi密码' maxlength='64'>"
"</div>"
"<button type='submit'>保存并连接</button>"
"</form>"
"<div id='networks' class='networks' style='margin-top:20px'></div>"
"</div>"
"<script>"
"fetch('/scan').then(r=>r.json()).then(d=>{"
"let h='<label style=\"margin-bottom:10px;display:block\">可用网络:</label>';"
"d.forEach(n=>{"
"h+='<div class=\"network\" onclick=\"document.getElementById(\\'ssid\\').value=\\''+n.ssid+'\\'\">'"
"+n.ssid+'<span class=\"signal\">'+n.rssi+' dBm</span></div>'});"
"document.getElementById('networks').innerHTML=h});"
"document.getElementById('form').onsubmit=function(e){"
"e.preventDefault();"
"fetch('/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},"
"body:'ssid='+encodeURIComponent(document.getElementById('ssid').value)"
"+'&password='+encodeURIComponent(document.getElementById('password').value)})"
".then(r=>r.json()).then(d=>{"
"if(d.success){document.getElementById('msg').innerHTML="
"'<div class=\"success\">配置已保存！设备将重启连接到新网络...</div>';"
"setTimeout(()=>location.reload(),5000)}"
"else{document.getElementById('msg').innerHTML='<div class=\"error\">'+d.error+'</div>'}})}"
"</script></body></html>";


// URL decode helper
static void url_decode(char *dst, const char *src, size_t dst_size) {
    char a, b;
    size_t i = 0;
    while (*src && i < dst_size - 1) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            dst[i++] = 16*a + b;
            src += 3;
        } else if (*src == '+') {
            dst[i++] = ' ';
            src++;
        } else {
            dst[i++] = *src++;
        }
    }
    dst[i] = '\0';
}

// Parse form data
static esp_err_t parse_form_data(const char *data, char *ssid, size_t ssid_len, char *password, size_t password_len) {
    char *ssid_start = strstr(data, "ssid=");
    char *password_start = strstr(data, "password=");
    
    if (!ssid_start) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ssid_start += 5;
    char *ssid_end = strchr(ssid_start, '&');
    size_t ssid_data_len = ssid_end ? (size_t)(ssid_end - ssid_start) : strlen(ssid_start);
    
    char encoded_ssid[128] = {0};
    if (ssid_data_len >= sizeof(encoded_ssid)) ssid_data_len = sizeof(encoded_ssid) - 1;
    strncpy(encoded_ssid, ssid_start, ssid_data_len);
    url_decode(ssid, encoded_ssid, ssid_len);
    
    if (password_start) {
        password_start += 9;
        char *password_end = strchr(password_start, '&');
        size_t password_data_len = password_end ? (size_t)(password_end - password_start) : strlen(password_start);
        
        char encoded_password[256] = {0};
        if (password_data_len >= sizeof(encoded_password)) password_data_len = sizeof(encoded_password) - 1;
        strncpy(encoded_password, password_start, password_data_len);
        url_decode(password, encoded_password, password_len);
    } else {
        password[0] = '\0';
    }
    
    return ESP_OK;
}

// HTTP handlers
static esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t scan_handler(httpd_req_t *req) {
    wifi_scan_config_t scan_config = {
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300,
    };
    
    esp_wifi_scan_start(&scan_config, true);
    
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    
    wifi_ap_record_t *ap_records = NULL;
    if (ap_count > 0) {
        if (ap_count > 20) ap_count = 20;
        ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
        esp_wifi_scan_get_ap_records(&ap_count, ap_records);
    }
    
    char *json = malloc(2048);
    strcpy(json, "[");
    
    for (int i = 0; i < ap_count; i++) {
        char entry[128];
        snprintf(entry, sizeof(entry), "%s{\"ssid\":\"%s\",\"rssi\":%d}",
                 i > 0 ? "," : "", ap_records[i].ssid, ap_records[i].rssi);
        strcat(json, entry);
    }
    strcat(json, "]");
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    
    free(json);
    if (ap_records) free(ap_records);
    
    return ESP_OK;
}

static esp_err_t save_handler(httpd_req_t *req) {
    char buf[256] = {0};
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    
    if (ret <= 0) {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"success\":false,\"error\":\"No data received\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    
    char ssid[33] = {0};
    char password[65] = {0};
    
    if (parse_form_data(buf, ssid, sizeof(ssid), password, sizeof(password)) != ESP_OK || strlen(ssid) == 0) {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"success\":false,\"error\":\"Invalid SSID\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Received WiFi config - SSID: %s", ssid);
    
    // Save to NVS
    nvs_handle_t nvs;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (err == ESP_OK) {
        nvs_set_str(nvs, NVS_KEY_SSID, ssid);
        nvs_set_str(nvs, NVS_KEY_PASSWORD, password);
        nvs_commit(nvs);
        nvs_close(nvs);
        ESP_LOGI(TAG, "WiFi credentials saved to NVS");
    }
    
    // Store in memory
    strncpy(s_configured_ssid, ssid, sizeof(s_configured_ssid) - 1);
    strncpy(s_configured_password, password, sizeof(s_configured_password) - 1);
    s_new_config_available = true;
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"success\":true}", HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

// Captive portal redirect handler
static esp_err_t redirect_handler(httpd_req_t *req) {
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// Start HTTP server
static esp_err_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    config.stack_size = 8192;
    config.lru_purge_enable = true;
    
    if (httpd_start(&s_server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return ESP_FAIL;
    }
    
    // Register handlers
    httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = root_handler};
    httpd_uri_t scan = {.uri = "/scan", .method = HTTP_GET, .handler = scan_handler};
    httpd_uri_t save = {.uri = "/save", .method = HTTP_POST, .handler = save_handler};
    httpd_uri_t redirect = {.uri = "/generate_204", .method = HTTP_GET, .handler = redirect_handler};
    httpd_uri_t redirect2 = {.uri = "/hotspot-detect.html", .method = HTTP_GET, .handler = redirect_handler};
    
    httpd_register_uri_handler(s_server, &root);
    httpd_register_uri_handler(s_server, &scan);
    httpd_register_uri_handler(s_server, &save);
    httpd_register_uri_handler(s_server, &redirect);
    httpd_register_uri_handler(s_server, &redirect2);
    
    ESP_LOGI(TAG, "HTTP server started");
    return ESP_OK;
}

esp_err_t captive_portal_start(void) {
    if (s_portal_running) {
        ESP_LOGW(TAG, "Captive portal already running");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Starting captive portal...");
    
    // Create AP netif if not exists
    if (s_ap_netif == NULL) {
        s_ap_netif = esp_netif_create_default_wifi_ap();
    }
    
    // Configure AP
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .channel = AP_CHANNEL,
            .password = AP_PASSWORD,
            .max_connection = AP_MAX_CONN,
            .authmode = strlen(AP_PASSWORD) > 0 ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    
    ESP_LOGI(TAG, "AP started - SSID: %s", AP_SSID);
    
    // Start web server
    start_webserver();
    
    s_portal_running = true;
    return ESP_OK;
}

esp_err_t captive_portal_stop(void) {
    if (!s_portal_running) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Stopping captive portal...");
    
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
    }
    
    // Switch to STA mode only
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    s_portal_running = false;
    ESP_LOGI(TAG, "Captive portal stopped");
    return ESP_OK;
}

bool captive_portal_is_running(void) {
    return s_portal_running;
}

bool captive_portal_has_new_config(void) {
    return s_new_config_available;
}

esp_err_t captive_portal_get_ssid(char *ssid, size_t len) {
    if (!ssid || len < 1) return ESP_ERR_INVALID_ARG;
    strncpy(ssid, s_configured_ssid, len - 1);
    ssid[len - 1] = '\0';
    return ESP_OK;
}

esp_err_t captive_portal_get_password(char *password, size_t len) {
    if (!password || len < 1) return ESP_ERR_INVALID_ARG;
    strncpy(password, s_configured_password, len - 1);
    password[len - 1] = '\0';
    return ESP_OK;
}

void captive_portal_clear_new_config(void) {
    s_new_config_available = false;
}

// Load saved WiFi credentials from NVS
esp_err_t captive_portal_load_config(char *ssid, size_t ssid_len, char *password, size_t password_len) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No saved WiFi config found");
        return ESP_ERR_NOT_FOUND;
    }
    
    size_t len = ssid_len;
    err = nvs_get_str(nvs, NVS_KEY_SSID, ssid, &len);
    if (err != ESP_OK) {
        nvs_close(nvs);
        return ESP_ERR_NOT_FOUND;
    }
    
    len = password_len;
    err = nvs_get_str(nvs, NVS_KEY_PASSWORD, password, &len);
    if (err != ESP_OK) {
        password[0] = '\0';
    }
    
    nvs_close(nvs);
    ESP_LOGI(TAG, "Loaded WiFi config - SSID: %s", ssid);
    return ESP_OK;
}

// Clear saved WiFi credentials
esp_err_t captive_portal_clear_config(void) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (err == ESP_OK) {
        nvs_erase_all(nvs);
        nvs_commit(nvs);
        nvs_close(nvs);
        ESP_LOGI(TAG, "WiFi config cleared");
    }
    return err;
}

// Start web server in STA mode for reconfiguration
static httpd_handle_t s_sta_server = NULL;

// STA 模式下的完整控制页面 - 全新霓虹赛博朋克风格
static const char *STA_HTML_PAGE = 
"<!DOCTYPE html><html><head>"
"<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'>"
"<title>⏰ NeoPixel Clock</title>"
"<link href='https://fonts.googleapis.com/css2?family=Rajdhani:wght@400;600;700&family=Audiowide&display=swap' rel='stylesheet'>"
"<style>"
":root{--neon-cyan:#00f5ff;--neon-pink:#ff6bcb;--neon-purple:#c084fc;--neon-yellow:#fde047;"
"--dark-bg:#0f172a;--card-bg:rgba(30,41,59,0.95)}"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{font-family:'Rajdhani',sans-serif;min-height:100vh;background:var(--dark-bg);color:#f1f5f9;"
"overflow-x:hidden}"
".universe{position:fixed;inset:0;z-index:-2;overflow:hidden;"
"background:linear-gradient(135deg,#0f172a 0%%,#1e293b 50%%,#334155 100%%)}"
".stars{position:absolute;width:100%%;height:100%%;background:transparent;"
"background-image:radial-gradient(2px 2px at 20px 30px,#fff,transparent),"
"radial-gradient(2px 2px at 40px 70px,rgba(255,255,255,0.9),transparent),"
"radial-gradient(2px 2px at 90px 40px,#fff,transparent),"
"radial-gradient(2px 2px at 160px 120px,rgba(255,255,255,0.95),transparent),"
"radial-gradient(2px 2px at 230px 80px,#fff,transparent),"
"radial-gradient(2px 2px at 300px 150px,rgba(255,255,255,0.85),transparent);"
"background-size:350px 200px;animation:twinkle 5s ease-in-out infinite}"
"@keyframes twinkle{0%%,100%%{opacity:1}50%%{opacity:0.7}}"
".nebula{position:absolute;inset:0;"
"background:radial-gradient(ellipse at 10%% 90%%,rgba(192,132,252,0.25) 0%%,transparent 50%%),"
"radial-gradient(ellipse at 90%% 10%%,rgba(0,245,255,0.25) 0%%,transparent 50%%),"
"radial-gradient(ellipse at 50%% 50%%,rgba(255,107,203,0.15) 0%%,transparent 60%%)}"
".scanline{position:fixed;top:0;left:0;right:0;height:4px;background:linear-gradient(90deg,"
"transparent,var(--neon-cyan),transparent);animation:scan 3s linear infinite;opacity:0.3;z-index:100}"
"@keyframes scan{0%%{top:0}100%%{top:100vh}}"
".container{max-width:440px;margin:0 auto;padding:16px;position:relative}"
".header{text-align:center;padding:24px 0 20px;position:relative}"
".logo-wrap{position:relative;width:100px;height:100px;margin:0 auto 16px}"
".logo-ring{position:absolute;inset:0;border:2px solid var(--neon-cyan);border-radius:50%%;"
"animation:rotate 10s linear infinite}"
".logo-ring::before{content:'';position:absolute;top:-4px;left:50%%;width:8px;height:8px;"
"background:var(--neon-cyan);border-radius:50%%;box-shadow:0 0 15px var(--neon-cyan)}"
"@keyframes rotate{to{transform:rotate(360deg)}}"
".logo-ring2{position:absolute;inset:8px;border:1px solid var(--neon-pink);border-radius:50%%;"
"animation:rotate 8s linear infinite reverse}"
".logo-inner{position:absolute;inset:16px;background:radial-gradient(circle,rgba(0,245,255,0.1),transparent);"
"border-radius:50%%;display:flex;align-items:center;justify-content:center}"
".logo-clock{font-size:36px;animation:glow 2s ease-in-out infinite}"
"@keyframes glow{0%%,100%%{filter:drop-shadow(0 0 5px var(--neon-cyan))}50%%{filter:drop-shadow(0 0 20px var(--neon-cyan))}}"
"h1{font-family:'Audiowide',sans-serif;font-size:26px;letter-spacing:6px;"
"background:linear-gradient(90deg,var(--neon-cyan),var(--neon-pink),var(--neon-purple),var(--neon-cyan));"
"background-size:300%%;-webkit-background-clip:text;-webkit-text-fill-color:transparent;"
"animation:shine 4s linear infinite;text-shadow:0 0 30px rgba(0,245,255,0.3)}"
"@keyframes shine{to{background-position:300%% 0}}"
".tagline{color:#94a3b8;font-size:11px;letter-spacing:4px;margin-top:6px;text-transform:uppercase}"
".status-bar{display:flex;justify-content:center;gap:20px;margin:16px 0;font-size:10px;color:#94a3b8}"
".status-item{display:flex;align-items:center;gap:6px}"
".status-dot{width:6px;height:6px;border-radius:50%%;background:var(--neon-cyan);"
"box-shadow:0 0 8px var(--neon-cyan);animation:blink 1.5s ease-in-out infinite}"
"@keyframes blink{0%%,100%%{opacity:1}50%%{opacity:0.3}}"
".tabs{display:flex;gap:4px;margin-bottom:16px}"
".tab{flex:1;padding:14px 8px;background:transparent;border:none;border-bottom:2px solid #475569;"
"color:#94a3b8;font-family:inherit;font-size:12px;font-weight:600;letter-spacing:2px;"
"cursor:pointer;transition:all 0.4s;position:relative}"
".tab::after{content:'';position:absolute;bottom:-2px;left:0;width:0;height:2px;"
"background:linear-gradient(90deg,var(--neon-cyan),var(--neon-pink));transition:width 0.4s}"
".tab:hover{color:#e2e8f0}.tab:hover::after{width:100%%}"
".tab.active{color:var(--neon-cyan)}.tab.active::after{width:100%%}"
".card{background:var(--card-bg);border:1px solid #475569;border-radius:8px;padding:20px;"
"position:relative;backdrop-filter:blur(10px);box-shadow:0 4px 20px rgba(0,0,0,0.3)}"
".card::before{content:'';position:absolute;top:0;left:20px;right:20px;height:2px;"
"background:linear-gradient(90deg,transparent,var(--neon-cyan),transparent)}"
".card::after{content:'';position:absolute;inset:0;border-radius:8px;padding:1px;"
"background:linear-gradient(135deg,rgba(0,245,255,0.2),transparent,rgba(255,107,203,0.2));"
"-webkit-mask:linear-gradient(#fff 0 0) content-box,linear-gradient(#fff 0 0);"
"-webkit-mask-composite:xor;pointer-events:none}"
".section-title{font-size:11px;color:var(--neon-cyan);letter-spacing:3px;margin-bottom:16px;"
"display:flex;align-items:center;gap:8px;text-transform:uppercase}"
".section-title::before{content:'◆';font-size:8px}"
".modes{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}"
".mode{aspect-ratio:1;display:flex;flex-direction:column;align-items:center;justify-content:center;"
"background:linear-gradient(135deg,rgba(255,255,255,0.08),rgba(255,255,255,0.03));"
"border:1px solid #475569;border-radius:6px;cursor:pointer;transition:all 0.3s;position:relative}"
".mode::before{content:'';position:absolute;inset:0;border-radius:6px;"
"background:radial-gradient(circle at center,rgba(0,245,255,0.2),transparent);opacity:0;transition:opacity 0.3s}"
".mode:hover{border-color:#64748b;transform:translateY(-2px);background:rgba(255,255,255,0.1)}.mode:hover::before{opacity:1}"
".mode.active{border-color:var(--neon-cyan);box-shadow:0 0 25px rgba(0,245,255,0.35),"
"inset 0 0 25px rgba(0,245,255,0.1);background:rgba(0,245,255,0.1)}"
".mode.active::before{opacity:1}"
".mode-icon{font-size:24px;margin-bottom:4px;transition:transform 0.3s}"
".mode:hover .mode-icon{transform:scale(1.15)}"
".mode-name{font-size:10px;color:#94a3b8;letter-spacing:1px;transition:color 0.3s;font-weight:600}"
".mode.active .mode-name{color:var(--neon-cyan)}"
".custom-panel{margin-top:20px;padding-top:20px;border-top:1px solid #475569;display:none}"
".custom-panel.show{display:block;animation:fadeIn 0.3s}"
"@keyframes fadeIn{from{opacity:0;transform:translateY(-10px)}to{opacity:1;transform:translateY(0)}}"
".color-section{margin-bottom:20px}"
".color-row{display:flex;gap:10px;margin-bottom:12px}"
".color-box{flex:1;position:relative}"
".color-label{font-size:10px;color:#cbd5e1;letter-spacing:1px;margin-bottom:6px;display:block;font-weight:500}"
".color-input{width:100%%;height:44px;border:2px solid #475569;background:#1e293b;cursor:pointer;"
"border-radius:4px;transition:all 0.3s}"
".color-input:hover{border-color:var(--neon-cyan);box-shadow:0 0 15px rgba(0,245,255,0.3)}"
".presets{display:flex;gap:8px;flex-wrap:wrap;margin-top:12px}"
".preset-btn{width:40px;height:32px;border:2px solid #475569;border-radius:4px;cursor:pointer;"
"transition:all 0.3s;position:relative;overflow:hidden}"
".preset-btn::after{content:'';position:absolute;inset:0;background:rgba(255,255,255,0.15);opacity:0;transition:opacity 0.3s}"
".preset-btn:hover{transform:scale(1.1);border-color:var(--neon-cyan);box-shadow:0 0 15px rgba(0,245,255,0.5)}"
".preset-btn:hover::after{opacity:1}"
".control-group{margin-bottom:18px}"
".control-label{font-size:11px;color:#cbd5e1;letter-spacing:1px;margin-bottom:8px;display:flex;"
"justify-content:space-between;align-items:center;font-weight:500}"
".control-value{color:var(--neon-cyan);font-weight:600}"
".slider{width:100%%;height:8px;background:#334155;border-radius:4px;appearance:none;cursor:pointer}"
".slider::-webkit-slider-thumb{appearance:none;width:20px;height:20px;background:var(--neon-cyan);"
"border:none;border-radius:50%%;cursor:pointer;transition:all 0.3s;"
"box-shadow:0 0 15px var(--neon-cyan)}"
".slider::-webkit-slider-thumb:hover{transform:scale(1.15);box-shadow:0 0 25px var(--neon-cyan)}"
".select-wrap{position:relative}"
".select-wrap::after{content:'▼';position:absolute;right:14px;top:50%%;transform:translateY(-50%%);"
"color:var(--neon-cyan);font-size:10px;pointer-events:none}"
"select{width:100%%;padding:14px;background:#1e293b;border:2px solid #475569;border-radius:4px;"
"color:#f1f5f9;font-family:inherit;font-size:13px;appearance:none;cursor:pointer;transition:all 0.3s}"
"select:hover,select:focus{border-color:var(--neon-cyan);outline:none;box-shadow:0 0 15px rgba(0,245,255,0.3)}"
".btn{width:100%%;padding:16px;background:linear-gradient(135deg,rgba(0,245,255,0.15),rgba(255,107,203,0.15));"
"border:2px solid var(--neon-cyan);color:var(--neon-cyan);"
"font-family:inherit;font-size:13px;font-weight:700;letter-spacing:3px;cursor:pointer;"
"transition:all 0.3s;position:relative;overflow:hidden;border-radius:6px;margin-top:16px}"
".btn::before{content:'';position:absolute;inset:0;background:linear-gradient(90deg,var(--neon-cyan),var(--neon-pink));"
"opacity:0;transition:opacity 0.3s}"
".btn:hover{color:#0f172a;text-shadow:none;transform:translateY(-2px);box-shadow:0 5px 20px rgba(0,245,255,0.4)}.btn:hover::before{opacity:1}"
".btn span{position:relative;z-index:1}"
".wifi-card{display:none}"
".wifi-card.show{display:block}"
".input-group{margin-bottom:16px;position:relative}"
".input-label{font-size:10px;color:#cbd5e1;letter-spacing:1px;margin-bottom:8px;display:block;font-weight:500}"
"input[type=text],input[type=password]{width:100%%;padding:14px;background:#1e293b;"
"border:2px solid #475569;border-radius:4px;color:#f1f5f9;font-family:inherit;font-size:14px;"
"transition:all 0.3s}"
"input:focus{outline:none;border-color:var(--neon-cyan);box-shadow:0 0 20px rgba(0,245,255,0.25)}"
"input::placeholder{color:#64748b}"
".toast{position:fixed;bottom:20px;left:50%%;transform:translateX(-50%%);padding:12px 24px;"
"border-radius:4px;font-size:12px;letter-spacing:1px;opacity:0;transition:all 0.4s;z-index:200}"
".toast.show{opacity:1;transform:translateX(-50%%) translateY(-10px)}"
".toast.ok{background:rgba(0,255,136,0.9);color:#000;border:1px solid #0f8}"
".toast.err{background:rgba(255,68,68,0.9);color:#fff;border:1px solid #f44}"
".footer{text-align:center;padding:24px;color:#64748b;font-size:10px;letter-spacing:3px}"
".glitch{position:relative}.glitch::before,.glitch::after{content:attr(data-text);position:absolute;"
"top:0;left:0;width:100%%;height:100%%}"
".glitch::before{left:2px;text-shadow:-2px 0 var(--neon-pink);animation:glitch1 2s infinite linear alternate-reverse}"
".glitch::after{left:-2px;text-shadow:2px 0 var(--neon-cyan);animation:glitch2 3s infinite linear alternate-reverse}"
"@keyframes glitch1{0%%,100%%{clip-path:inset(20%% 0 60%% 0)}25%%{clip-path:inset(60%% 0 10%% 0)}"
"50%%{clip-path:inset(10%% 0 80%% 0)}75%%{clip-path:inset(80%% 0 5%% 0)}}"
"@keyframes glitch2{0%%,100%%{clip-path:inset(80%% 0 5%% 0)}25%%{clip-path:inset(10%% 0 70%% 0)}"
"50%%{clip-path:inset(50%% 0 30%% 0)}75%%{clip-path:inset(30%% 0 50%% 0)}}"
"</style></head><body>"
"<div class='universe'><div class='stars'></div><div class='nebula'></div></div>"
"<div class='scanline'></div>"
"<div class='container'>"
"<header class='header'>"
"<div class='logo-wrap'><div class='logo-ring'></div><div class='logo-ring2'></div>"
"<div class='logo-inner'><span class='logo-clock'>⏰</span></div></div>"
"<h1 class='glitch' data-text='NEOPIXEL'>NEOPIXEL</h1>"
"<p class='tagline'>智能时钟控制台</p>"
"</header>"
"<div class='status-bar'>"
"<div class='status-item'><span class='status-dot'></span>在线</div>"
"<div class='status-item'>v2.0</div></div>"
"<div class='tabs'>"
"<button class='tab active' onclick='showTab(0)'>💡 灯光秀</button>"
"<button class='tab' onclick='showTab(1)'>📶 网络</button></div>"
"<div id='lightCard' class='card'>"
"<div class='section-title'>模式选择</div>"
"<div class='modes' id='modes'></div>"
"<div class='custom-panel' id='customPanel'>"
"<div class='section-title'>自定义调色</div>"
"<div class='color-section'>"
"<div class='color-row'>"
"<div class='color-box'><span class='color-label'>主色调</span>"
"<input type='color' class='color-input' id='color1' value='#00f5ff' onchange='liveUpdate()'></div>"
"<div class='color-box'><span class='color-label'>辅助色</span>"
"<input type='color' class='color-input' id='color2' value='#ff00aa' onchange='liveUpdate()'></div>"
"<div class='color-box'><span class='color-label'>点缀色</span>"
"<input type='color' class='color-input' id='color3' value='#ffe600' onchange='liveUpdate()'></div></div>"
"<div class='color-label'>快速配色</div>"
"<div class='presets'>"
"<button class='preset-btn' onclick='setPreset(\"#ff0000\",\"#00ff00\",\"#0000ff\")' style='background:linear-gradient(135deg,#f00,#0f0,#00f)'></button>"
"<button class='preset-btn' onclick='setPreset(\"#ff00ff\",\"#00ffff\",\"#ffff00\")' style='background:linear-gradient(135deg,#f0f,#0ff,#ff0)'></button>"
"<button class='preset-btn' onclick='setPreset(\"#ff6b6b\",\"#feca57\",\"#48dbfb\")' style='background:linear-gradient(135deg,#ff6b6b,#feca57,#48dbfb)'></button>"
"<button class='preset-btn' onclick='setPreset(\"#a29bfe\",\"#fd79a8\",\"#00b894\")' style='background:linear-gradient(135deg,#a29bfe,#fd79a8,#00b894)'></button>"
"<button class='preset-btn' onclick='setPreset(\"#e17055\",\"#fdcb6e\",\"#00cec9\")' style='background:linear-gradient(135deg,#e17055,#fdcb6e,#00cec9)'></button>"
"<button class='preset-btn' onclick='setPreset(\"#6c5ce7\",\"#fd79a8\",\"#ffeaa7\")' style='background:linear-gradient(135deg,#6c5ce7,#fd79a8,#ffeaa7)'></button>"
"<button class='preset-btn' onclick='setPreset(\"#00f5ff\",\"#bf00ff\",\"#ff00aa\")' style='background:linear-gradient(135deg,#00f5ff,#bf00ff,#ff00aa)'></button>"
"<button class='preset-btn' onclick='setPreset(\"#f39c12\",\"#e74c3c\",\"#9b59b6\")' style='background:linear-gradient(135deg,#f39c12,#e74c3c,#9b59b6)'></button>"
"</div></div>"
"<div class='control-group'><div class='select-wrap'>"
"<select id='effect' onchange='liveUpdate()'>"
"<option value='0'>⬤ 纯色填充</option><option value='1'>◐ 双色渐变</option>"
"<option value='2'>◑ 三色渐变</option><option value='3'>🌈 彩虹旋转</option>"
"<option value='4'>💫 呼吸闪烁</option><option value='5'>〰 双色流水</option>"
"<option value='6'>☄ 彗星拖尾</option><option value='7'>🌊 波浪起伏</option>"
"<option value='8'>✦ 随机闪烁</option><option value='9'>🔮 渐变呼吸</option></select></div></div>"
"<div class='control-group'>"
"<div class='control-label'><span>动画速度</span><span class='control-value' id='speedVal'>5</span></div>"
"<input type='range' class='slider' id='speed' min='1' max='10' value='5' "
"oninput='document.getElementById(\"speedVal\").textContent=this.value;liveUpdate()'></div>"
"<div class='control-group'>"
"<div class='control-label'><span>亮度</span><span class='control-value'><span id='brightVal'>80</span>%%</span></div>"
"<input type='range' class='slider' id='brightness' min='5' max='100' value='80' "
"oninput='document.getElementById(\"brightVal\").textContent=this.value;liveUpdate()'></div>"
"<div class='control-group'>"
"<div class='control-label'><span>尾迹长度</span><span class='control-value' id='tailVal'>10</span></div>"
"<input type='range' class='slider' id='tail' min='3' max='30' value='10' "
"oninput='document.getElementById(\"tailVal\").textContent=this.value;liveUpdate()'></div>"
"<div class='control-group'><div class='select-wrap'>"
"<select id='direction' onchange='liveUpdate()'>"
"<option value='0'>↻ 顺时针</option><option value='1'>↺ 逆时针</option>"
"<option value='2'>↔ 双向扩散</option><option value='3'>↔ 双向收缩</option></select></div></div>"
"<button class='btn' onclick='applyCustom()'><span>✨ 应用效果</span></button></div></div>"
"<div id='wifiCard' class='card wifi-card'>"
"<div class='section-title'>网络配置</div>"
"<div class='input-group'><span class='input-label'>WiFi 名称</span>"
"<input type='text' id='ssid' placeholder='输入网络名称'></div>"
"<div class='input-group'><span class='input-label'>密码</span>"
"<input type='password' id='password' placeholder='输入密码'></div>"
"<button class='btn' onclick='saveWifi()'><span>🔗 连接网络</span></button></div>"
"<footer class='footer'>NEOPIXEL CLOCK © 2024</footer></div>"
"<div class='toast' id='toast'></div>"
"<script>"
"const modes=[{id:0,icon:'⏰',name:'时钟'},{id:1,icon:'🌈',name:'彩虹'},"
"{id:2,icon:'💫',name:'呼吸'},{id:3,icon:'⚡',name:'追逐'},"
"{id:4,icon:'✨',name:'星光'},{id:5,icon:'🔥',name:'火焰'},"
"{id:6,icon:'🌊',name:'海洋'},{id:7,icon:'☄️',name:'流星'},"
"{id:8,icon:'🚨',name:'警灯'},{id:9,icon:'🍬',name:'糖果'},"
"{id:10,icon:'🌌',name:'极光'},{id:11,icon:'💗',name:'心跳'},"
"{id:12,icon:'🎨',name:'自定义'}];"
"let currentMode=0;"
"function toast(msg,type){const t=document.getElementById('toast');t.textContent=msg;"
"t.className='toast '+type+' show';setTimeout(()=>t.classList.remove('show'),2500)}"
"function renderModes(){"
"let h='';modes.forEach(m=>{"
"h+='<div class=\"mode'+(m.id===currentMode?' active':'')+'\" onclick=\"setMode('+m.id+')\">';"
"h+='<span class=\"mode-icon\">'+m.icon+'</span><span class=\"mode-name\">'+m.name+'</span></div>'});"
"document.getElementById('modes').innerHTML=h;"
"document.getElementById('customPanel').classList.toggle('show',currentMode===12)}"
"function setMode(id){currentMode=id;renderModes();toast(modes[id].name+' 已激活','ok');"
"fetch('/api/light?mode='+id).then(r=>r.json())}"
"function setPreset(c1,c2,c3){document.getElementById('color1').value=c1;"
"document.getElementById('color2').value=c2;document.getElementById('color3').value=c3;liveUpdate()}"
"function liveUpdate(){if(currentMode===12)applyCustom()}"
"function applyCustom(){"
"const c1=document.getElementById('color1').value;"
"const c2=document.getElementById('color2').value;"
"const c3=document.getElementById('color3').value;"
"const params='mode=12&r1='+parseInt(c1.slice(1,3),16)+'&g1='+parseInt(c1.slice(3,5),16)+"
"'&b1='+parseInt(c1.slice(5,7),16)+'&r2='+parseInt(c2.slice(1,3),16)+"
"'&g2='+parseInt(c2.slice(3,5),16)+'&b2='+parseInt(c2.slice(5,7),16)+"
"'&r3='+parseInt(c3.slice(1,3),16)+'&g3='+parseInt(c3.slice(3,5),16)+"
"'&b3='+parseInt(c3.slice(5,7),16)+"
"'&speed='+document.getElementById('speed').value+"
"'&brightness='+document.getElementById('brightness').value+"
"'&effect='+document.getElementById('effect').value+"
"'&tail='+document.getElementById('tail').value+"
"'&dir='+document.getElementById('direction').value;"
"fetch('/api/light?'+params).then(r=>r.json()).then(()=>toast('效果已更新','ok'))}"
"function showTab(n){document.querySelectorAll('.tab').forEach((t,i)=>t.classList.toggle('active',i===n));"
"document.getElementById('lightCard').style.display=n===0?'block':'none';"
"document.getElementById('wifiCard').classList.toggle('show',n===1)}"
"function saveWifi(){"
"fetch('/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},"
"body:'ssid='+encodeURIComponent(document.getElementById('ssid').value)+"
"'&password='+encodeURIComponent(document.getElementById('password').value)})"
".then(r=>r.json()).then(d=>{toast(d.success?'配置已保存，正在重启...':'错误: '+d.error,d.success?'ok':'err')})}"
"fetch('/api/light').then(r=>r.json()).then(d=>{currentMode=d.mode;renderModes()});"
"</script></body></html>";

// 灯光秀 API handler
static esp_err_t light_api_handler(httpd_req_t *req) {
    char buf[256] = {0};
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    
    if (buf_len > 1 && buf_len < sizeof(buf)) {
        httpd_req_get_url_query_str(req, buf, buf_len);
        
        // 解析 mode 参数
        char param[32];
        if (httpd_query_key_value(buf, "mode", param, sizeof(param)) == ESP_OK) {
            int mode = atoi(param);
            light_show_set_mode((light_show_mode_t)mode);
            
            // 如果是自定义模式，解析更多参数
            if (mode == LIGHT_SHOW_CUSTOM) {
                custom_params_t params;
                light_show_get_custom_params(&params);
                
                if (httpd_query_key_value(buf, "r1", param, sizeof(param)) == ESP_OK)
                    params.color1.r = atoi(param);
                if (httpd_query_key_value(buf, "g1", param, sizeof(param)) == ESP_OK)
                    params.color1.g = atoi(param);
                if (httpd_query_key_value(buf, "b1", param, sizeof(param)) == ESP_OK)
                    params.color1.b = atoi(param);
                if (httpd_query_key_value(buf, "r2", param, sizeof(param)) == ESP_OK)
                    params.color2.r = atoi(param);
                if (httpd_query_key_value(buf, "g2", param, sizeof(param)) == ESP_OK)
                    params.color2.g = atoi(param);
                if (httpd_query_key_value(buf, "b2", param, sizeof(param)) == ESP_OK)
                    params.color2.b = atoi(param);
                if (httpd_query_key_value(buf, "r3", param, sizeof(param)) == ESP_OK)
                    params.color3.r = atoi(param);
                if (httpd_query_key_value(buf, "g3", param, sizeof(param)) == ESP_OK)
                    params.color3.g = atoi(param);
                if (httpd_query_key_value(buf, "b3", param, sizeof(param)) == ESP_OK)
                    params.color3.b = atoi(param);
                if (httpd_query_key_value(buf, "speed", param, sizeof(param)) == ESP_OK)
                    params.speed = atoi(param);
                if (httpd_query_key_value(buf, "brightness", param, sizeof(param)) == ESP_OK)
                    params.brightness = atoi(param);
                if (httpd_query_key_value(buf, "effect", param, sizeof(param)) == ESP_OK)
                    params.effect = atoi(param);
                if (httpd_query_key_value(buf, "tail", param, sizeof(param)) == ESP_OK)
                    params.tail_length = atoi(param);
                if (httpd_query_key_value(buf, "dir", param, sizeof(param)) == ESP_OK)
                    params.direction = atoi(param);
                
                light_show_set_custom_params(&params);
            }
        }
    }
    
    // 返回当前状态
    char json[128];
    snprintf(json, sizeof(json), "{\"mode\":%d,\"active\":%s}",
             light_show_get_mode(),
             light_show_is_active() ? "true" : "false");
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// STA 模式主页 handler
static esp_err_t sta_root_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, STA_HTML_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t captive_portal_start_sta_server(void) {
    if (s_sta_server != NULL) {
        return ESP_OK;
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;
    config.stack_size = 8192;
    
    if (httpd_start(&s_sta_server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start STA HTTP server");
        return ESP_FAIL;
    }
    
    // Register handlers
    httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = sta_root_handler};
    httpd_uri_t scan = {.uri = "/scan", .method = HTTP_GET, .handler = scan_handler};
    httpd_uri_t save = {.uri = "/save", .method = HTTP_POST, .handler = save_handler};
    httpd_uri_t light_api = {.uri = "/api/light", .method = HTTP_GET, .handler = light_api_handler};
    
    httpd_register_uri_handler(s_sta_server, &root);
    httpd_register_uri_handler(s_sta_server, &scan);
    httpd_register_uri_handler(s_sta_server, &save);
    httpd_register_uri_handler(s_sta_server, &light_api);
    
    ESP_LOGI(TAG, "STA HTTP server started with light show control");
    return ESP_OK;
}

esp_err_t captive_portal_stop_sta_server(void) {
    if (s_sta_server) {
        httpd_stop(s_sta_server);
        s_sta_server = NULL;
    }
    return ESP_OK;
}

