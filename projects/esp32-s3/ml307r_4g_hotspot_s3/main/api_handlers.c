#include "api_handlers.h"
#include "ml307r_driver.h"
#include "wifi_manager.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_timer.h"

static const char *TAG = "API";

// 辅助函数：设置CORS头
static void set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
}

// 辅助函数：发送JSON响应
static esp_err_t send_json_response(httpd_req_t *req, cJSON *json)
{
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    set_cors_headers(req);
    
    esp_err_t ret = httpd_resp_send(req, json_string, strlen(json_string));
    
    free(json_string);
    return ret;
}

// 辅助函数：发送错误响应
static esp_err_t send_error_response(httpd_req_t *req, int code, const char *message)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "success", false);
    cJSON_AddNumberToObject(json, "error_code", code);
    cJSON_AddStringToObject(json, "error_message", message);
    
    esp_err_t ret = send_json_response(req, json);
    cJSON_Delete(json);
    
    return ret;
}

esp_err_t api_status_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "API: /api/status");

    cJSON *json = cJSON_CreateObject();
    cJSON *system_info = cJSON_CreateObject();
    cJSON *ml307r_info = cJSON_CreateObject();
    cJSON *wifi_info = cJSON_CreateObject();

    // 系统信息
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    cJSON_AddStringToObject(system_info, "chip_model", "ESP32-S3");
    cJSON_AddNumberToObject(system_info, "chip_cores", chip_info.cores);
    cJSON_AddNumberToObject(system_info, "chip_revision", chip_info.revision);
    cJSON_AddNumberToObject(system_info, "free_heap", esp_get_free_heap_size());
    cJSON_AddNumberToObject(system_info, "uptime", esp_timer_get_time() / 1000000);

    // ML307R状态
    ml307r_state_t ml307r_state = ml307r_get_state();
    const char *state_str = "unknown";
    switch (ml307r_state) {
        case ML307R_STATE_UNKNOWN: state_str = "unknown"; break;
        case ML307R_STATE_INIT: state_str = "initializing"; break;
        case ML307R_STATE_READY: state_str = "ready"; break;
        case ML307R_STATE_CONNECTING: state_str = "connecting"; break;
        case ML307R_STATE_CONNECTED: state_str = "connected"; break;
        case ML307R_STATE_ERROR: state_str = "error"; break;
    }
    
    cJSON_AddStringToObject(ml307r_info, "state", state_str);
    cJSON_AddBoolToObject(ml307r_info, "ready", ml307r_is_ready());
    cJSON_AddNumberToObject(ml307r_info, "signal_strength", ml307r_get_signal_strength());

    // WiFi状态
    wifi_state_t wifi_state = wifi_manager_get_state();
    const char *wifi_state_str = "disconnected";
    switch (wifi_state) {
        case WIFI_STATE_DISCONNECTED: wifi_state_str = "disconnected"; break;
        case WIFI_STATE_CONNECTING: wifi_state_str = "connecting"; break;
        case WIFI_STATE_CONNECTED: wifi_state_str = "connected"; break;
        case WIFI_STATE_AP_MODE: wifi_state_str = "ap_mode"; break;
        case WIFI_STATE_ERROR: wifi_state_str = "error"; break;
    }
    
    cJSON_AddStringToObject(wifi_info, "state", wifi_state_str);
    cJSON_AddBoolToObject(wifi_info, "connected", wifi_manager_is_connected());

    wifi_info_t wifi_details;
    if (wifi_manager_get_info(&wifi_details) == ESP_OK) {
        cJSON_AddStringToObject(wifi_info, "ssid", wifi_details.ssid);
        cJSON_AddStringToObject(wifi_info, "ip_address", wifi_details.ip_address);
        cJSON_AddNumberToObject(wifi_info, "rssi", wifi_details.rssi);
    }

    // 组装响应
    cJSON_AddBoolToObject(json, "success", true);
    cJSON_AddItemToObject(json, "system", system_info);
    cJSON_AddItemToObject(json, "ml307r", ml307r_info);
    cJSON_AddItemToObject(json, "wifi", wifi_info);

    esp_err_t ret = send_json_response(req, json);
    cJSON_Delete(json);

    return ret;
}

esp_err_t api_network_info_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "API: /api/network/info");

    ml307r_network_info_t network_info;
    esp_err_t ret = ml307r_get_network_info(&network_info);
    
    cJSON *json = cJSON_CreateObject();
    
    if (ret == ESP_OK) {
        cJSON_AddBoolToObject(json, "success", true);
        cJSON_AddStringToObject(json, "operator", network_info.operator_name);
        cJSON_AddNumberToObject(json, "signal_strength", network_info.signal_strength);
        cJSON_AddStringToObject(json, "network_type", network_info.network_type);
        cJSON_AddStringToObject(json, "ip_address", network_info.ip_address);
        cJSON_AddBoolToObject(json, "connected", network_info.is_connected);
    } else {
        cJSON_AddBoolToObject(json, "success", false);
        cJSON_AddStringToObject(json, "error", "Failed to get network info");
    }

    ret = send_json_response(req, json);
    cJSON_Delete(json);

    return ret;
}

esp_err_t api_hotspot_control_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "API: /api/hotspot/control");

    char content[512];
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);
    if (ret <= 0) {
        return send_error_response(req, 400, "Invalid request body");
    }
    content[ret] = '\0';

    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        return send_error_response(req, 400, "Invalid JSON");
    }

    cJSON *enable_item = cJSON_GetObjectItem(json, "enable");
    if (!cJSON_IsBool(enable_item)) {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing 'enable' field");
    }

    bool enable = cJSON_IsTrue(enable_item);
    esp_err_t result;

    if (enable) {
        // 启用热点 - 使用默认配置
        ml307r_hotspot_config_t config = {
            .ssid = "ESP32-ML307R-Hotspot",
            .password = "12345678",
            .max_connections = 5,
            .is_enabled = true
        };
        
        // 检查是否有自定义配置
        cJSON *ssid_item = cJSON_GetObjectItem(json, "ssid");
        cJSON *password_item = cJSON_GetObjectItem(json, "password");
        cJSON *max_conn_item = cJSON_GetObjectItem(json, "max_connections");
        
        if (cJSON_IsString(ssid_item)) {
            strncpy(config.ssid, ssid_item->valuestring, sizeof(config.ssid) - 1);
        }
        if (cJSON_IsString(password_item)) {
            strncpy(config.password, password_item->valuestring, sizeof(config.password) - 1);
        }
        if (cJSON_IsNumber(max_conn_item)) {
            config.max_connections = max_conn_item->valueint;
        }

        result = ml307r_enable_hotspot(&config);
    } else {
        result = ml307r_disable_hotspot();
    }

    cJSON_Delete(json);

    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "success", result == ESP_OK);
    if (result != ESP_OK) {
        cJSON_AddStringToObject(response, "error", "Failed to control hotspot");
    }

    ret = send_json_response(req, response);
    cJSON_Delete(response);

    return ret;
}

esp_err_t api_hotspot_config_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "API: /api/hotspot/config");

    if (req->method == HTTP_GET) {
        // 获取热点配置
        ml307r_hotspot_config_t config;
        esp_err_t ret = ml307r_get_hotspot_status(&config);
        
        cJSON *json = cJSON_CreateObject();
        
        if (ret == ESP_OK) {
            cJSON_AddBoolToObject(json, "success", true);
            cJSON_AddStringToObject(json, "ssid", config.ssid);
            cJSON_AddStringToObject(json, "password", config.password);
            cJSON_AddNumberToObject(json, "max_connections", config.max_connections);
            cJSON_AddBoolToObject(json, "enabled", config.is_enabled);
        } else {
            cJSON_AddBoolToObject(json, "success", false);
            cJSON_AddStringToObject(json, "error", "Failed to get hotspot config");
        }

        ret = send_json_response(req, json);
        cJSON_Delete(json);
        return ret;
        
    } else if (req->method == HTTP_POST) {
        // 设置热点配置
        char content[512];
        int ret = httpd_req_recv(req, content, sizeof(content) - 1);
        if (ret <= 0) {
            return send_error_response(req, 400, "Invalid request body");
        }
        content[ret] = '\0';

        cJSON *json = cJSON_Parse(content);
        if (json == NULL) {
            return send_error_response(req, 400, "Invalid JSON");
        }

        ml307r_hotspot_config_t config = {0};
        
        cJSON *ssid_item = cJSON_GetObjectItem(json, "ssid");
        cJSON *password_item = cJSON_GetObjectItem(json, "password");
        cJSON *max_conn_item = cJSON_GetObjectItem(json, "max_connections");
        
        if (!cJSON_IsString(ssid_item)) {
            cJSON_Delete(json);
            return send_error_response(req, 400, "Missing 'ssid' field");
        }
        
        strncpy(config.ssid, ssid_item->valuestring, sizeof(config.ssid) - 1);
        
        if (cJSON_IsString(password_item)) {
            strncpy(config.password, password_item->valuestring, sizeof(config.password) - 1);
        }
        
        if (cJSON_IsNumber(max_conn_item)) {
            config.max_connections = max_conn_item->valueint;
        } else {
            config.max_connections = 5; // 默认值
        }

        cJSON_Delete(json);

        // 应用配置（这里简化处理，实际应该保存到NVS）
        esp_err_t result = ml307r_enable_hotspot(&config);

        cJSON *response = cJSON_CreateObject();
        cJSON_AddBoolToObject(response, "success", result == ESP_OK);
        if (result != ESP_OK) {
            cJSON_AddStringToObject(response, "error", "Failed to set hotspot config");
        }

        ret = send_json_response(req, response);
        cJSON_Delete(response);
        return ret;
    }

    return send_error_response(req, 405, "Method not allowed");
}

esp_err_t api_ml307r_reset_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "API: /api/ml307r/reset");

    esp_err_t ret = ml307r_reset();
    
    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "success", ret == ESP_OK);
    if (ret != ESP_OK) {
        cJSON_AddStringToObject(json, "error", "Failed to reset ML307R");
    }

    ret = send_json_response(req, json);
    cJSON_Delete(json);

    return ret;
}

esp_err_t api_wifi_connect_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "API: /api/wifi/connect");

    char content[512];
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);
    if (ret <= 0) {
        return send_error_response(req, 400, "Invalid request body");
    }
    content[ret] = '\0';

    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        return send_error_response(req, 400, "Invalid JSON");
    }

    cJSON *ssid_item = cJSON_GetObjectItem(json, "ssid");
    cJSON *password_item = cJSON_GetObjectItem(json, "password");

    if (!cJSON_IsString(ssid_item)) {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing 'ssid' field");
    }

    const char *ssid = ssid_item->valuestring;
    const char *password = cJSON_IsString(password_item) ? password_item->valuestring : NULL;

    cJSON_Delete(json);

    esp_err_t result = wifi_manager_connect(ssid, password);

    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "success", result == ESP_OK);
    if (result != ESP_OK) {
        cJSON_AddStringToObject(response, "error", "Failed to connect to WiFi");
    }

    ret = send_json_response(req, response);
    cJSON_Delete(response);

    return ret;
}
