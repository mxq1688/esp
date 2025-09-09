/**
 * @file ai_engine.c
 * @brief AI Engine Implementation for ESP32-S3 AI Assistant
 * @author AI Assistant Team
 * @version 1.0.0
 */

#include "ai_engine.h"
#include "ai_assistant.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "cJSON.h"

static const char *TAG = "AI_ENGINE";

// AI引擎配置
static ai_engine_config_t s_ai_config = {
    .type = AI_ENGINE_OPENAI,
    .api_key = "",
    .server_url = "https://api.openai.com/v1/chat/completions",
    .timeout_ms = 30000,
    .enable_streaming = false
};

// HTTP响应缓冲区
static char s_response_buffer[AI_ENGINE_MAX_RESPONSE_LEN];
static size_t s_response_len = 0;

/**
 * @brief HTTP事件处理器
 */
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ai_assistant_log_error(TAG, "HTTP error occurred");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ai_assistant_log_info(TAG, "HTTP connected");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ai_assistant_log_info(TAG, "HTTP headers sent");
            break;
        case HTTP_EVENT_ON_HEADER:
            ai_assistant_log_info(TAG, "HTTP header: %.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // 复制响应数据
                if (s_response_len + evt->data_len < AI_ENGINE_MAX_RESPONSE_LEN - 1) {
                    memcpy(s_response_buffer + s_response_len, evt->data, evt->data_len);
                    s_response_len += evt->data_len;
                    s_response_buffer[s_response_len] = '\0';
                }
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ai_assistant_log_info(TAG, "HTTP request finished");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ai_assistant_log_info(TAG, "HTTP disconnected");
            break;
        default:
            break;
    }
    return ESP_OK;
}

/**
 * @brief 初始化AI引擎
 */
esp_err_t ai_engine_init(void)
{
    ai_assistant_log_info(TAG, "Initializing AI Engine");
    
    // 清空响应缓冲区
    memset(s_response_buffer, 0, sizeof(s_response_buffer));
    s_response_len = 0;
    
    ai_assistant_log_info(TAG, "AI Engine initialized successfully");
    return ESP_OK;
}

/**
 * @brief 设置AI引擎配置
 */
esp_err_t ai_engine_set_config(const ai_engine_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_ai_config, config, sizeof(ai_engine_config_t));
    ai_assistant_log_info(TAG, "AI Engine configuration updated");
    return ESP_OK;
}

/**
 * @brief 获取AI引擎配置
 */
esp_err_t ai_engine_get_config(ai_engine_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_ai_config, sizeof(ai_engine_config_t));
    return ESP_OK;
}

/**
 * @brief 处理AI命令
 */
esp_err_t ai_engine_process_command(const char *command, ai_response_t *response)
{
    if (command == NULL || response == NULL) {
        ai_assistant_log_error(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Processing AI command: %s", command);
    
    esp_err_t ret = ESP_OK;
    
    // 根据AI引擎类型处理
    switch (s_ai_config.type) {
        case AI_ENGINE_OPENAI:
            ret = ai_engine_handle_openai(command, response);
            break;
        case AI_ENGINE_BAIDU:
            ret = ai_engine_handle_baidu(command, response);
            break;
        case AI_ENGINE_TENCENT:
            ret = ai_engine_handle_tencent(command, response);
            break;
        case AI_ENGINE_CUSTOM:
            // 自定义AI引擎处理
            ai_assistant_log_info(TAG, "Custom AI engine not implemented");
            ret = ESP_ERR_NOT_SUPPORTED;
            break;
        default:
            ai_assistant_log_error(TAG, "Unknown AI engine type: %d", s_ai_config.type);
            ret = ESP_ERR_INVALID_ARG;
            break;
    }
    
    if (ret == ESP_OK) {
        response->timestamp = esp_timer_get_time();
        ai_assistant_log_info(TAG, "AI command processed successfully");
    } else {
        ai_assistant_log_error(TAG, "Failed to process AI command");
    }
    
    return ret;
}

/**
 * @brief 设置API密钥
 */
esp_err_t ai_engine_set_api_key(const char *api_key)
{
    if (api_key == NULL) {
        ai_assistant_log_error(TAG, "API key cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(s_ai_config.api_key, api_key, sizeof(s_ai_config.api_key) - 1);
    s_ai_config.api_key[sizeof(s_ai_config.api_key) - 1] = '\0';
    
    ai_assistant_log_info(TAG, "API key updated");
    return ESP_OK;
}

/**
 * @brief 设置服务器URL
 */
esp_err_t ai_engine_set_server_url(const char *url)
{
    if (url == NULL) {
        ai_assistant_log_error(TAG, "Server URL cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(s_ai_config.server_url, url, sizeof(s_ai_config.server_url) - 1);
    s_ai_config.server_url[sizeof(s_ai_config.server_url) - 1] = '\0';
    
    ai_assistant_log_info(TAG, "Server URL updated: %s", s_ai_config.server_url);
    return ESP_OK;
}

/**
 * @brief 测试连接
 */
esp_err_t ai_engine_test_connection(void)
{
    ai_assistant_log_info(TAG, "Testing AI engine connection");
    
    // 发送简单的测试请求
    ai_response_t test_response;
    esp_err_t ret = ai_engine_process_command("Hello", &test_response);
    
    if (ret == ESP_OK) {
        ai_assistant_log_info(TAG, "AI engine connection test successful");
    } else {
        ai_assistant_log_error(TAG, "AI engine connection test failed");
    }
    
    return ret;
}

/**
 * @brief 发送HTTP请求
 */
static esp_err_t ai_engine_send_request(const char *command, char *response, size_t response_len)
{
    if (command == NULL || response == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 清空响应缓冲区
    memset(s_response_buffer, 0, sizeof(s_response_buffer));
    s_response_len = 0;
    
    // 配置HTTP客户端
    esp_http_client_config_t config = {
        .url = s_ai_config.server_url,
        .event_handler = http_event_handler,
        .timeout_ms = s_ai_config.timeout_ms,
        .buffer_size = AI_ENGINE_MAX_RESPONSE_LEN,
        .buffer_size_tx = AI_ENGINE_MAX_REQUEST_LEN,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ai_assistant_log_error(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    
    // 设置请求头
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    
    // 设置Authorization头（如果有API密钥）
    if (strlen(s_ai_config.api_key) > 0) {
        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Bearer %s", s_ai_config.api_key);
        esp_http_client_set_header(client, "Authorization", auth_header);
    }
    
    // 构建请求JSON
    cJSON *json = cJSON_CreateObject();
    cJSON *model = cJSON_CreateString("gpt-3.5-turbo");
    cJSON *messages = cJSON_CreateArray();
    cJSON *message = cJSON_CreateObject();
    cJSON *role = cJSON_CreateString("user");
    cJSON *content = cJSON_CreateString(command);
    
    cJSON_AddItemToObject(message, "role", role);
    cJSON_AddItemToObject(message, "content", content);
    cJSON_AddItemToArray(messages, message);
    cJSON_AddItemToObject(json, "model", model);
    cJSON_AddItemToObject(json, "messages", messages);
    
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        ai_assistant_log_error(TAG, "Failed to create JSON request");
        cJSON_Delete(json);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    // 发送请求
    esp_err_t err = esp_http_client_open(client, strlen(json_string));
    if (err != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        free(json_string);
        cJSON_Delete(json);
        esp_http_client_cleanup(client);
        return err;
    }
    
    int wlen = esp_http_client_write(client, json_string, strlen(json_string));
    if (wlen < 0) {
        ai_assistant_log_error(TAG, "Failed to write HTTP request");
        free(json_string);
        cJSON_Delete(json);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    // 读取响应
    int content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0) {
        ai_assistant_log_error(TAG, "HTTP client fetch headers failed");
        free(json_string);
        cJSON_Delete(json);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    int status_code = esp_http_client_get_status_code(client);
    if (status_code != 200) {
        ai_assistant_log_error(TAG, "HTTP request failed with status code: %d", status_code);
        free(json_string);
        cJSON_Delete(json);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    // 复制响应数据
    if (s_response_len > 0 && s_response_len < response_len) {
        strncpy(response, s_response_buffer, response_len - 1);
        response[response_len - 1] = '\0';
    }
    
    // 清理资源
    free(json_string);
    cJSON_Delete(json);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    
    return ESP_OK;
}

/**
 * @brief 解析AI响应
 */
static esp_err_t ai_engine_parse_response(const char *json_response, ai_response_t *response)
{
    if (json_response == NULL || response == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    cJSON *json = cJSON_Parse(json_response);
    if (json == NULL) {
        ai_assistant_log_error(TAG, "Failed to parse JSON response");
        return ESP_FAIL;
    }
    
    // 解析OpenAI格式的响应
    cJSON *choices = cJSON_GetObjectItem(json, "choices");
    if (choices != NULL && cJSON_IsArray(choices)) {
        cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
        if (first_choice != NULL) {
            cJSON *message = cJSON_GetObjectItem(first_choice, "message");
            if (message != NULL) {
                cJSON *content = cJSON_GetObjectItem(message, "content");
                if (content != NULL && cJSON_IsString(content)) {
                    strncpy(response->text, content->valuestring, sizeof(response->text) - 1);
                    response->text[sizeof(response->text) - 1] = '\0';
                    response->has_audio = false;
                    response->audio_url[0] = '\0';
                    
                    cJSON_Delete(json);
                    return ESP_OK;
                }
            }
        }
    }
    
    // 如果解析失败，设置默认响应
    strcpy(response->text, "抱歉，我无法理解您的问题。");
    response->has_audio = false;
    response->audio_url[0] = '\0';
    
    cJSON_Delete(json);
    return ESP_FAIL;
}

/**
 * @brief 处理OpenAI请求
 */
static esp_err_t ai_engine_handle_openai(const char *command, ai_response_t *response)
{
    char http_response[AI_ENGINE_MAX_RESPONSE_LEN];
    
    esp_err_t ret = ai_engine_send_request(command, http_response, sizeof(http_response));
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to send request to OpenAI");
        return ret;
    }
    
    return ai_engine_parse_response(http_response, response);
}

/**
 * @brief 处理百度AI请求
 */
static esp_err_t ai_engine_handle_baidu(const char *command, ai_response_t *response)
{
    // 百度AI接口实现
    ai_assistant_log_info(TAG, "Baidu AI engine not fully implemented");
    
    // 模拟响应
    snprintf(response->text, sizeof(response->text), "百度AI回复：%s", command);
    response->has_audio = false;
    response->audio_url[0] = '\0';
    
    return ESP_OK;
}

/**
 * @brief 处理腾讯AI请求
 */
static esp_err_t ai_engine_handle_tencent(const char *command, ai_response_t *response)
{
    // 腾讯AI接口实现
    ai_assistant_log_info(TAG, "Tencent AI engine not fully implemented");
    
    // 模拟响应
    snprintf(response->text, sizeof(response->text), "腾讯AI回复：%s", command);
    response->has_audio = false;
    response->audio_url[0] = '\0';
    
    return ESP_OK;
}
