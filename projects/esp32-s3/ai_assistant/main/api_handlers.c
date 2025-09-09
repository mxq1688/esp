/**
 * @file api_handlers.c
 * @brief API Handlers Implementation for ESP32-S3 AI Assistant
 * @author AI Assistant Team
 * @version 1.0.0
 */

#include "api_handlers.h"
#include "ai_assistant.h"
#include "ai_engine.h"
#include "voice_recognition.h"
#include "voice_synthesis.h"
#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "cJSON.h"

static const char *TAG = "API_HANDLERS";

/**
 * @brief 初始化API处理器
 */
esp_err_t api_handlers_init(void)
{
    ai_assistant_log_info(TAG, "Initializing API Handlers");
    ai_assistant_log_info(TAG, "API Handlers initialized successfully");
    return ESP_OK;
}

/**
 * @brief 注册API处理器
 */
esp_err_t api_handlers_register(httpd_handle_t server)
{
    if (server == NULL) {
        ai_assistant_log_error(TAG, "Server handle cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Registering API handlers");
    
    // 系统状态API
    httpd_uri_t status_uri = {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = api_handler_get_status,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &status_uri);
    
    // 配置API
    httpd_uri_t get_config_uri = {
        .uri = "/api/config",
        .method = HTTP_GET,
        .handler = api_handler_get_config,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &get_config_uri);
    
    httpd_uri_t set_config_uri = {
        .uri = "/api/config",
        .method = HTTP_POST,
        .handler = api_handler_set_config,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &set_config_uri);
    
    // 语音命令API
    httpd_uri_t voice_command_uri = {
        .uri = "/api/voice/command",
        .method = HTTP_POST,
        .handler = api_handler_voice_command,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &voice_command_uri);
    
    // AI聊天API
    httpd_uri_t ai_chat_uri = {
        .uri = "/api/ai/chat",
        .method = HTTP_POST,
        .handler = api_handler_ai_chat,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &ai_chat_uri);
    
    // 语音上传API
    httpd_uri_t voice_upload_uri = {
        .uri = "/api/voice/upload",
        .method = HTTP_POST,
        .handler = api_handler_voice_upload,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &voice_upload_uri);
    
    // 语音下载API
    httpd_uri_t voice_download_uri = {
        .uri = "/api/voice/download",
        .method = HTTP_GET,
        .handler = api_handler_voice_download,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &voice_download_uri);
    
    // 系统信息API
    httpd_uri_t system_info_uri = {
        .uri = "/api/system/info",
        .method = HTTP_GET,
        .handler = api_handler_system_info,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &system_info_uri);
    
    // 重启API
    httpd_uri_t restart_uri = {
        .uri = "/api/system/restart",
        .method = HTTP_POST,
        .handler = api_handler_restart,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &restart_uri);
    
    // 恢复出厂设置API
    httpd_uri_t factory_reset_uri = {
        .uri = "/api/system/factory_reset",
        .method = HTTP_POST,
        .handler = api_handler_factory_reset,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &factory_reset_uri);
    
    ai_assistant_log_info(TAG, "API handlers registered successfully");
    return ESP_OK;
}

/**
 * @brief 注销API处理器
 */
esp_err_t api_handlers_unregister(httpd_handle_t server)
{
    if (server == NULL) {
        ai_assistant_log_error(TAG, "Server handle cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Unregistering API handlers");
    
    // 注销所有API处理器
    httpd_unregister_uri_handler(server, "/api/status", HTTP_GET);
    httpd_unregister_uri_handler(server, "/api/config", HTTP_GET);
    httpd_unregister_uri_handler(server, "/api/config", HTTP_POST);
    httpd_unregister_uri_handler(server, "/api/voice/command", HTTP_POST);
    httpd_unregister_uri_handler(server, "/api/ai/chat", HTTP_POST);
    httpd_unregister_uri_handler(server, "/api/voice/upload", HTTP_POST);
    httpd_unregister_uri_handler(server, "/api/voice/download", HTTP_GET);
    httpd_unregister_uri_handler(server, "/api/system/info", HTTP_GET);
    httpd_unregister_uri_handler(server, "/api/system/restart", HTTP_POST);
    httpd_unregister_uri_handler(server, "/api/system/factory_reset", HTTP_POST);
    
    ai_assistant_log_info(TAG, "API handlers unregistered successfully");
    return ESP_OK;
}

/**
 * @brief 获取系统状态API
 */
esp_err_t api_handler_get_status(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling get status request");
    
    // 创建响应JSON
    cJSON *json = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();
    
    // 系统状态
    cJSON_AddStringToObject(data, "ai_state", ai_assistant_get_state_string(g_ai_state));
    cJSON_AddStringToObject(data, "voice_recognition_state", "IDLE");
    cJSON_AddStringToObject(data, "voice_synthesis_state", "IDLE");
    cJSON_AddStringToObject(data, "wifi_state", "CONNECTED");
    
    // 系统信息
    cJSON_AddNumberToObject(data, "free_heap", esp_get_free_heap_size());
    cJSON_AddNumberToObject(data, "uptime", esp_timer_get_time() / 1000000);
    cJSON_AddStringToObject(data, "version", AI_ASSISTANT_VERSION);
    
    // 配置信息
    cJSON_AddBoolToObject(data, "voice_enabled", g_ai_config.voice_enabled);
    cJSON_AddBoolToObject(data, "auto_wake_up", g_ai_config.auto_wake_up);
    cJSON_AddNumberToObject(data, "volume_level", g_ai_config.volume_level);
    cJSON_AddNumberToObject(data, "language", g_ai_config.language);
    
    api_response_t response = {
        .error_code = API_ERROR_NONE,
        .message = "Success",
        .data = data,
        .timestamp = esp_timer_get_time()
    };
    
    esp_err_t ret = api_handlers_send_json_response(req, &response);
    
    cJSON_Delete(json);
    return ret;
}

/**
 * @brief 获取配置API
 */
esp_err_t api_handler_get_config(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling get config request");
    
    cJSON *data = cJSON_CreateObject();
    
    // WiFi配置（不包含密码）
    cJSON_AddStringToObject(data, "wifi_ssid", g_ai_config.wifi_ssid);
    cJSON_AddBoolToObject(data, "voice_enabled", g_ai_config.voice_enabled);
    cJSON_AddBoolToObject(data, "auto_wake_up", g_ai_config.auto_wake_up);
    cJSON_AddNumberToObject(data, "volume_level", g_ai_config.volume_level);
    cJSON_AddNumberToObject(data, "language", g_ai_config.language);
    cJSON_AddStringToObject(data, "server_url", g_ai_config.server_url);
    
    api_response_t response = {
        .error_code = API_ERROR_NONE,
        .message = "Success",
        .data = data,
        .timestamp = esp_timer_get_time()
    };
    
    return api_handlers_send_json_response(req, &response);
}

/**
 * @brief 设置配置API
 */
esp_err_t api_handler_set_config(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling set config request");
    
    cJSON *json = NULL;
    esp_err_t ret = api_handlers_parse_json_request(req, &json);
    if (ret != ESP_OK) {
        return api_handlers_send_error_response(req, API_ERROR_INVALID_REQUEST, "Invalid JSON");
    }
    
    // 更新配置
    cJSON *item;
    
    item = cJSON_GetObjectItem(json, "wifi_ssid");
    if (item && cJSON_IsString(item)) {
        strncpy(g_ai_config.wifi_ssid, item->valuestring, sizeof(g_ai_config.wifi_ssid) - 1);
    }
    
    item = cJSON_GetObjectItem(json, "wifi_password");
    if (item && cJSON_IsString(item)) {
        strncpy(g_ai_config.wifi_password, item->valuestring, sizeof(g_ai_config.wifi_password) - 1);
    }
    
    item = cJSON_GetObjectItem(json, "voice_enabled");
    if (item && cJSON_IsBool(item)) {
        g_ai_config.voice_enabled = cJSON_IsTrue(item);
    }
    
    item = cJSON_GetObjectItem(json, "auto_wake_up");
    if (item && cJSON_IsBool(item)) {
        g_ai_config.auto_wake_up = cJSON_IsTrue(item);
    }
    
    item = cJSON_GetObjectItem(json, "volume_level");
    if (item && cJSON_IsNumber(item)) {
        g_ai_config.volume_level = (uint8_t)item->valueint;
    }
    
    item = cJSON_GetObjectItem(json, "language");
    if (item && cJSON_IsNumber(item)) {
        g_ai_config.language = (uint8_t)item->valueint;
    }
    
    item = cJSON_GetObjectItem(json, "api_key");
    if (item && cJSON_IsString(item)) {
        strncpy(g_ai_config.api_key, item->valuestring, sizeof(g_ai_config.api_key) - 1);
    }
    
    item = cJSON_GetObjectItem(json, "server_url");
    if (item && cJSON_IsString(item)) {
        strncpy(g_ai_config.server_url, item->valuestring, sizeof(g_ai_config.server_url) - 1);
    }
    
    cJSON_Delete(json);
    
    // 创建响应
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "message", "Configuration updated successfully");
    
    api_response_t response = {
        .error_code = API_ERROR_NONE,
        .message = "Success",
        .data = data,
        .timestamp = esp_timer_get_time()
    };
    
    return api_handlers_send_json_response(req, &response);
}

/**
 * @brief 语音命令API
 */
esp_err_t api_handler_voice_command(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling voice command request");
    
    cJSON *json = NULL;
    esp_err_t ret = api_handlers_parse_json_request(req, &json);
    if (ret != ESP_OK) {
        return api_handlers_send_error_response(req, API_ERROR_INVALID_REQUEST, "Invalid JSON");
    }
    
    cJSON *command_item = cJSON_GetObjectItem(json, "command");
    if (!command_item || !cJSON_IsString(command_item)) {
        cJSON_Delete(json);
        return api_handlers_send_error_response(req, API_ERROR_INVALID_PARAMETER, "Missing command parameter");
    }
    
    const char *command = command_item->valuestring;
    ai_assistant_log_info(TAG, "Processing voice command: %s", command);
    
    // 处理语音命令
    ai_response_t ai_response;
    ret = ai_engine_process_command(command, &ai_response);
    
    cJSON_Delete(json);
    
    if (ret != ESP_OK) {
        return api_handlers_send_error_response(req, API_ERROR_INTERNAL_ERROR, "Failed to process command");
    }
    
    // 创建响应
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "response", ai_response.text);
    cJSON_AddBoolToObject(data, "has_audio", ai_response.has_audio);
    if (ai_response.has_audio) {
        cJSON_AddStringToObject(data, "audio_url", ai_response.audio_url);
    }
    
    api_response_t response = {
        .error_code = API_ERROR_NONE,
        .message = "Success",
        .data = data,
        .timestamp = esp_timer_get_time()
    };
    
    return api_handlers_send_json_response(req, &response);
}

/**
 * @brief AI聊天API
 */
esp_err_t api_handler_ai_chat(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling AI chat request");
    
    cJSON *json = NULL;
    esp_err_t ret = api_handlers_parse_json_request(req, &json);
    if (ret != ESP_OK) {
        return api_handlers_send_error_response(req, API_ERROR_INVALID_REQUEST, "Invalid JSON");
    }
    
    cJSON *message_item = cJSON_GetObjectItem(json, "message");
    if (!message_item || !cJSON_IsString(message_item)) {
        cJSON_Delete(json);
        return api_handlers_send_error_response(req, API_ERROR_INVALID_PARAMETER, "Missing message parameter");
    }
    
    const char *message = message_item->valuestring;
    ai_assistant_log_info(TAG, "Processing AI chat: %s", message);
    
    // 处理AI聊天
    ai_response_t ai_response;
    ret = ai_engine_process_command(message, &ai_response);
    
    cJSON_Delete(json);
    
    if (ret != ESP_OK) {
        return api_handlers_send_error_response(req, API_ERROR_INTERNAL_ERROR, "Failed to process message");
    }
    
    // 创建响应
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "response", ai_response.text);
    
    api_response_t response = {
        .error_code = API_ERROR_NONE,
        .message = "Success",
        .data = data,
        .timestamp = esp_timer_get_time()
    };
    
    return api_handlers_send_json_response(req, &response);
}

/**
 * @brief 语音上传API
 */
esp_err_t api_handler_voice_upload(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling voice upload request");
    
    // 这里应该实现音频文件上传处理
    // 目前返回未实现错误
    
    return api_handlers_send_error_response(req, API_ERROR_NOT_FOUND, "Voice upload not implemented");
}

/**
 * @brief 语音下载API
 */
esp_err_t api_handler_voice_download(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling voice download request");
    
    // 这里应该实现音频文件下载处理
    // 目前返回未实现错误
    
    return api_handlers_send_error_response(req, API_ERROR_NOT_FOUND, "Voice download not implemented");
}

/**
 * @brief 系统信息API
 */
esp_err_t api_handler_system_info(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling system info request");
    
    cJSON *data = cJSON_CreateObject();
    
    // 系统信息
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    cJSON_AddStringToObject(data, "chip_model", "ESP32-S3");
    cJSON_AddNumberToObject(data, "chip_revision", chip_info.revision);
    cJSON_AddNumberToObject(data, "cpu_cores", chip_info.cores);
    cJSON_AddNumberToObject(data, "free_heap", esp_get_free_heap_size());
    cJSON_AddNumberToObject(data, "minimum_free_heap", esp_get_minimum_free_heap_size());
    cJSON_AddNumberToObject(data, "uptime", esp_timer_get_time() / 1000000);
    
    // 软件信息
    cJSON_AddStringToObject(data, "idf_version", esp_get_idf_version());
    cJSON_AddStringToObject(data, "app_version", AI_ASSISTANT_VERSION);
    cJSON_AddStringToObject(data, "compile_date", __DATE__);
    cJSON_AddStringToObject(data, "compile_time", __TIME__);
    
    // WiFi信息
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        cJSON_AddStringToObject(data, "wifi_ssid", (char*)ap_info.ssid);
        cJSON_AddNumberToObject(data, "wifi_rssi", ap_info.rssi);
    }
    
    api_response_t response = {
        .error_code = API_ERROR_NONE,
        .message = "Success",
        .data = data,
        .timestamp = esp_timer_get_time()
    };
    
    return api_handlers_send_json_response(req, &response);
}

/**
 * @brief 重启系统API
 */
esp_err_t api_handler_restart(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling restart request");
    
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "message", "System will restart in 3 seconds");
    
    api_response_t response = {
        .error_code = API_ERROR_NONE,
        .message = "Success",
        .data = data,
        .timestamp = esp_timer_get_time()
    };
    
    esp_err_t ret = api_handlers_send_json_response(req, &response);
    
    // 延迟3秒后重启
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
    
    return ret;
}

/**
 * @brief 恢复出厂设置API
 */
esp_err_t api_handler_factory_reset(httpd_req_t *req)
{
    ai_assistant_log_info(TAG, "Handling factory reset request");
    
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "message", "Factory reset initiated");
    
    api_response_t response = {
        .error_code = API_ERROR_NONE,
        .message = "Success",
        .data = data,
        .timestamp = esp_timer_get_time()
    };
    
    esp_err_t ret = api_handlers_send_json_response(req, &response);
    
    // 清除NVS存储
    nvs_flash_erase();
    
    // 延迟3秒后重启
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
    
    return ret;
}

/**
 * @brief 发送JSON响应
 */
esp_err_t api_handlers_send_json_response(httpd_req_t *req, const api_response_t *response)
{
    if (req == NULL || response == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 创建响应JSON
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "error_code", response->error_code);
    cJSON_AddStringToObject(json, "message", response->message);
    cJSON_AddNumberToObject(json, "timestamp", response->timestamp);
    
    if (response->data != NULL) {
        cJSON_AddItemToObject(json, "data", cJSON_Duplicate(response->data, true));
    }
    
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        cJSON_Delete(json);
        return ESP_ERR_NO_MEM;
    }
    
    // 设置响应头
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    // 发送响应
    esp_err_t ret = httpd_resp_send(req, json_string, strlen(json_string));
    
    free(json_string);
    cJSON_Delete(json);
    
    return ret;
}

/**
 * @brief 发送错误响应
 */
esp_err_t api_handlers_send_error_response(httpd_req_t *req, api_error_code_t error_code, const char *message)
{
    api_response_t response = {
        .error_code = error_code,
        .message = {0},
        .data = NULL,
        .timestamp = esp_timer_get_time()
    };
    
    strncpy(response.message, message ? message : api_handlers_get_error_message(error_code), sizeof(response.message) - 1);
    
    return api_handlers_send_json_response(req, &response);
}

/**
 * @brief 解析JSON请求
 */
esp_err_t api_handlers_parse_json_request(httpd_req_t *req, cJSON **json)
{
    if (req == NULL || json == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 获取内容长度
    size_t content_len = req->content_len;
    if (content_len == 0 || content_len > API_MAX_JSON_LEN) {
        ai_assistant_log_error(TAG, "Invalid content length: %d", content_len);
        return ESP_ERR_INVALID_SIZE;
    }
    
    // 分配缓冲区
    char *buffer = malloc(content_len + 1);
    if (buffer == NULL) {
        ai_assistant_log_error(TAG, "Failed to allocate buffer");
        return ESP_ERR_NO_MEM;
    }
    
    // 读取请求体
    int ret = httpd_req_recv(req, buffer, content_len);
    if (ret <= 0) {
        ai_assistant_log_error(TAG, "Failed to receive request body");
        free(buffer);
        return ESP_FAIL;
    }
    
    buffer[content_len] = '\0';
    
    // 解析JSON
    *json = cJSON_Parse(buffer);
    free(buffer);
    
    if (*json == NULL) {
        ai_assistant_log_error(TAG, "Failed to parse JSON");
        return ESP_ERR_INVALID_ARG;
    }
    
    return ESP_OK;
}

/**
 * @brief 验证请求参数
 */
esp_err_t api_handlers_validate_request(httpd_req_t *req, const char *required_fields[], size_t field_count)
{
    if (req == NULL || required_fields == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    cJSON *json = NULL;
    esp_err_t ret = api_handlers_parse_json_request(req, &json);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 检查必需字段
    for (size_t i = 0; i < field_count; i++) {
        cJSON *item = cJSON_GetObjectItem(json, required_fields[i]);
        if (item == NULL) {
            ai_assistant_log_error(TAG, "Missing required field: %s", required_fields[i]);
            cJSON_Delete(json);
            return ESP_ERR_INVALID_ARG;
        }
    }
    
    cJSON_Delete(json);
    return ESP_OK;
}

/**
 * @brief 获取错误消息
 */
const char* api_handlers_get_error_message(api_error_code_t error_code)
{
    switch (error_code) {
        case API_ERROR_NONE: return "Success";
        case API_ERROR_INVALID_REQUEST: return "Invalid request";
        case API_ERROR_INVALID_PARAMETER: return "Invalid parameter";
        case API_ERROR_NOT_FOUND: return "Not found";
        case API_ERROR_INTERNAL_ERROR: return "Internal error";
        case API_ERROR_UNAUTHORIZED: return "Unauthorized";
        case API_ERROR_FORBIDDEN: return "Forbidden";
        case API_ERROR_TIMEOUT: return "Timeout";
        case API_ERROR_SERVICE_UNAVAILABLE: return "Service unavailable";
        default: return "Unknown error";
    }
}
