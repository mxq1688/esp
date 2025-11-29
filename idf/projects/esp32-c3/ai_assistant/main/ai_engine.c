#include "ai_engine.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"
#include "sys/time.h"
#include <string.h>

static const char *TAG = "AI_ENGINE";

static ai_personality_t current_config;
static esp_http_client_handle_t http_client = NULL;

// HTTP响应回调
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP错误");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP连接成功");
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP请求完成");
            break;
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t ai_engine_init(ai_personality_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&current_config, config, sizeof(ai_personality_t));
    
    // 初始化HTTP客户端
    esp_http_client_config_t http_config = {
        .url = "https://api.openai.com",
        .event_handler = http_event_handler,
        .timeout_ms = 10000,
    };
    
    http_client = esp_http_client_init(&http_config);
    if (http_client == NULL) {
        ESP_LOGE(TAG, "HTTP客户端初始化失败");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "AI引擎初始化完成");
    ESP_LOGI(TAG, "AI助手: %s", current_config.name);
    ESP_LOGI(TAG, "个性: %s", current_config.personality);
    
    return ESP_OK;
}

esp_err_t ai_process_command(const char *command, ai_response_t *response)
{
    if (command == NULL || response == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "处理命令: %s", command);
    
    // 首先尝试本地命令处理
    if (ai_process_local_command(command, response) == ESP_OK) {
        return ESP_OK;
    }
    
    // 如果本地处理失败，尝试API调用
    return ai_get_response_from_api(command, response);
}

esp_err_t ai_process_local_command(const char *command, ai_response_t *response)
{
    if (command == NULL || response == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 转换为小写进行比较
    char cmd_lower[256];
    strcpy(cmd_lower, command);
    for (int i = 0; cmd_lower[i]; i++) {
        cmd_lower[i] = tolower(cmd_lower[i]);
    }
    
    // 时间查询
    if (strstr(cmd_lower, "时间") || strstr(cmd_lower, "几点") || strstr(cmd_lower, "time")) {
        return ai_handle_time_query(command, response);
    }
    
    // 天气查询
    if (strstr(cmd_lower, "天气") || strstr(cmd_lower, "weather")) {
        return ai_handle_weather_query(command, response);
    }
    
    // 设备控制
    if (strstr(cmd_lower, "开灯") || strstr(cmd_lower, "关灯") || 
        strstr(cmd_lower, "turn on") || strstr(cmd_lower, "turn off")) {
        return ai_handle_device_control(command, response);
    }
    
    // 音乐控制
    if (strstr(cmd_lower, "播放") || strstr(cmd_lower, "暂停") || 
        strstr(cmd_lower, "play") || strstr(cmd_lower, "pause")) {
        return ai_handle_music_control(command, response);
    }
    
    // 问候语
    if (strstr(cmd_lower, "你好") || strstr(cmd_lower, "hello") || 
        strstr(cmd_lower, "hi") || strstr(cmd_lower, "小智")) {
        snprintf(response->text, sizeof(response->text), 
                "你好！我是%s，很高兴为您服务。有什么可以帮助您的吗？", current_config.name);
        strcpy(response->action, "greeting");
        response->confidence = 95;
        strcpy(response->emotion, "happy");
        return ESP_OK;
    }
    
    // 帮助信息
    if (strstr(cmd_lower, "帮助") || strstr(cmd_lower, "help") || 
        strstr(cmd_lower, "能做什么")) {
        snprintf(response->text, sizeof(response->text), 
                "我可以帮您：\n1. 查询时间和天气\n2. 控制智能设备\n3. 播放音乐\n4. 聊天对话\n5. 回答问题\n请告诉我您需要什么帮助？");
        strcpy(response->action, "help");
        response->confidence = 90;
        strcpy(response->emotion, "helpful");
        return ESP_OK;
    }
    
    return ESP_FAIL; // 本地无法处理
}

esp_err_t ai_handle_time_query(const char *query, ai_response_t *response)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y年%m月%d日 %H:%M:%S", &timeinfo);
    
    snprintf(response->text, sizeof(response->text), 
            "现在是%s", time_str);
    strcpy(response->action, "time_query");
    response->confidence = 100;
    strcpy(response->emotion, "informative");
    
    return ESP_OK;
}

esp_err_t ai_handle_weather_query(const char *query, ai_response_t *response)
{
    // 这里可以集成天气API，暂时返回模拟数据
    snprintf(response->text, sizeof(response->text), 
            "抱歉，天气查询功能正在开发中。目前无法获取实时天气信息。");
    strcpy(response->action, "weather_query");
    response->confidence = 80;
    strcpy(response->emotion, "apologetic");
    
    return ESP_OK;
}

esp_err_t ai_handle_device_control(const char *command, ai_response_t *response)
{
    char cmd_lower[256];
    strcpy(cmd_lower, command);
    for (int i = 0; cmd_lower[i]; i++) {
        cmd_lower[i] = tolower(cmd_lower[i]);
    }
    
    if (strstr(cmd_lower, "开灯") || strstr(cmd_lower, "turn on")) {
        snprintf(response->text, sizeof(response->text), 
                "好的，正在为您开灯");
        strcpy(response->action, "light_on");
    } else if (strstr(cmd_lower, "关灯") || strstr(cmd_lower, "turn off")) {
        snprintf(response->text, sizeof(response->text), 
                "好的，正在为您关灯");
        strcpy(response->action, "light_off");
    } else {
        snprintf(response->text, sizeof(response->text), 
                "抱歉，我没有理解您的设备控制指令");
        strcpy(response->action, "unknown");
    }
    
    response->confidence = 85;
    strcpy(response->emotion, "helpful");
    
    return ESP_OK;
}

esp_err_t ai_handle_music_control(const char *command, ai_response_t *response)
{
    char cmd_lower[256];
    strcpy(cmd_lower, command);
    for (int i = 0; cmd_lower[i]; i++) {
        cmd_lower[i] = tolower(cmd_lower[i]);
    }
    
    if (strstr(cmd_lower, "播放") || strstr(cmd_lower, "play")) {
        snprintf(response->text, sizeof(response->text), 
                "好的，正在为您播放音乐");
        strcpy(response->action, "music_play");
    } else if (strstr(cmd_lower, "暂停") || strstr(cmd_lower, "pause")) {
        snprintf(response->text, sizeof(response->text), 
                "好的，已暂停音乐播放");
        strcpy(response->action, "music_pause");
    } else {
        snprintf(response->text, sizeof(response->text), 
                "抱歉，我没有理解您的音乐控制指令");
        strcpy(response->action, "unknown");
    }
    
    response->confidence = 85;
    strcpy(response->emotion, "helpful");
    
    return ESP_OK;
}

esp_err_t ai_handle_chat(const char *message, ai_response_t *response)
{
    // 简单的聊天回复逻辑
    if (strstr(message, "谢谢") || strstr(message, "thank")) {
        snprintf(response->text, sizeof(response->text), 
                "不客气！很高兴能帮助到您");
        strcpy(response->emotion, "happy");
    } else if (strstr(message, "再见") || strstr(message, "goodbye")) {
        snprintf(response->text, sizeof(response->text), 
                "再见！有需要随时叫我");
        strcpy(response->emotion, "friendly");
    } else {
        snprintf(response->text, sizeof(response->text), 
                "我理解您说的，但可能需要更多上下文来提供更好的回答。您可以尝试问一些具体的问题。");
        strcpy(response->emotion, "thoughtful");
    }
    
    strcpy(response->action, "chat");
    response->confidence = 70;
    
    return ESP_OK;
}

esp_err_t ai_get_response_from_api(const char *query, ai_response_t *response)
{
    // 这里可以集成OpenAI API或其他AI服务
    // 暂时返回默认回复
    snprintf(response->text, sizeof(response->text), 
            "我理解您的问题，但需要连接到AI服务来提供更准确的回答。您可以尝试一些本地功能，比如查询时间或控制设备。");
    strcpy(response->action, "api_fallback");
    response->confidence = 60;
    strcpy(response->emotion, "apologetic");
    
    return ESP_OK;
}

esp_err_t ai_set_personality(ai_personality_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&current_config, config, sizeof(ai_personality_t));
    ESP_LOGI(TAG, "AI个性已更新: %s", current_config.name);
    
    return ESP_OK;
}
