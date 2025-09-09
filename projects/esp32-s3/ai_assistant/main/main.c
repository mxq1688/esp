/**
 * @file main.c
 * @brief ESP32-S3 AI Assistant Main Application
 * @author AI Assistant Team
 * @version 1.0.0
 * @date 2024
 * 
 * 智能语音助手主程序
 * 支持语音识别、自然语言处理、语音合成和Web交互
 */

#include "ai_assistant.h"
#include "wifi_manager.h"
#include "ai_engine.h"
#include "voice_recognition.h"
#include "voice_synthesis.h"
#include "web_server.h"
#include "api_handlers.h"
#include "audio_processor.h"
#include "nlp_processor.h"

static const char *TAG = "AI_ASSISTANT_MAIN";

// 全局变量定义
ai_assistant_config_t g_ai_config = {
    .wifi_ssid = "AI_Assistant",
    .wifi_password = "12345678",
    .voice_enabled = true,
    .auto_wake_up = true,
    .volume_level = 80,
    .language = 0, // 0=中文, 1=英文
    .api_key = "",
    .server_url = "https://api.openai.com/v1"
};

ai_assistant_state_t g_ai_state = AI_STATE_IDLE;
QueueHandle_t voice_queue = NULL;
QueueHandle_t ai_response_queue = NULL;
EventGroupHandle_t ai_event_group = NULL;

// 任务句柄
static TaskHandle_t voice_task_handle = NULL;
static TaskHandle_t ai_processing_task_handle = NULL;
static TaskHandle_t audio_task_handle = NULL;

/**
 * @brief 语音处理任务
 * @param pvParameters 任务参数
 */
static void voice_processing_task(void *pvParameters)
{
    voice_recognition_result_t voice_result;
    ai_response_t ai_response;
    
    ai_assistant_log_info(TAG, "Voice processing task started");
    
    while (1) {
        // 等待语音识别结果
        if (xQueueReceive(voice_queue, &voice_result, portMAX_DELAY) == pdTRUE) {
            ai_assistant_log_info(TAG, "Received voice command: %s (confidence: %.2f)", 
                                voice_result.text, voice_result.confidence);
            
            // 更新状态为处理中
            g_ai_state = AI_STATE_PROCESSING;
            xEventGroupSetBits(ai_event_group, AI_PROCESSING_DONE_BIT);
            
            // 处理AI命令
            if (ai_engine_process_command(voice_result.text, &ai_response) == ESP_OK) {
                ai_assistant_log_info(TAG, "AI response: %s", ai_response.text);
                
                // 发送AI响应到队列
                if (xQueueSend(ai_response_queue, &ai_response, portMAX_DELAY) != pdTRUE) {
                    ai_assistant_log_error(TAG, "Failed to send AI response to queue");
                }
            } else {
                ai_assistant_log_error(TAG, "Failed to process AI command");
            }
        }
    }
}

/**
 * @brief AI响应处理任务
 * @param pvParameters 任务参数
 */
static void ai_response_task(void *pvParameters)
{
    ai_response_t ai_response;
    
    ai_assistant_log_info(TAG, "AI response task started");
    
    while (1) {
        // 等待AI响应
        if (xQueueReceive(ai_response_queue, &ai_response, portMAX_DELAY) == pdTRUE) {
            ai_assistant_log_info(TAG, "Processing AI response: %s", ai_response.text);
            
            // 更新状态为说话中
            g_ai_state = AI_STATE_SPEAKING;
            
            // 语音合成
            if (g_ai_config.voice_enabled) {
                if (voice_synthesis_speak(ai_response.text) == ESP_OK) {
                    ai_assistant_log_info(TAG, "Voice synthesis completed");
                } else {
                    ai_assistant_log_error(TAG, "Voice synthesis failed");
                }
            }
            
            // 更新状态为空闲
            g_ai_state = AI_STATE_IDLE;
            xEventGroupSetBits(ai_event_group, AI_SPEAKING_DONE_BIT);
        }
    }
}

/**
 * @brief 音频处理任务
 * @param pvParameters 任务参数
 */
static void audio_processing_task(void *pvParameters)
{
    uint8_t audio_buffer[MAX_AUDIO_BUFFER_SIZE];
    size_t bytes_read;
    
    ai_assistant_log_info(TAG, "Audio processing task started");
    
    while (1) {
        // 检查是否在监听状态
        if (g_ai_state == AI_STATE_LISTENING) {
            // 读取音频数据
            if (audio_processor_record_start() == ESP_OK) {
                // 这里应该实现实际的音频数据读取
                // 为了演示，我们使用模拟数据
                vTaskDelay(pdMS_TO_TICKS(100));
                
                // 模拟语音识别
                voice_recognition_result_t voice_result = {
                    .text = "你好小智，今天天气怎么样？",
                    .confidence = 0.95f,
                    .command_type = VOICE_CMD_WEATHER,
                    .timestamp = esp_timer_get_time()
                };
                
                // 发送到语音处理队列
                if (xQueueSend(voice_queue, &voice_result, 0) != pdTRUE) {
                    ai_assistant_log_error(TAG, "Failed to send voice result to queue");
                }
                
                audio_processor_record_stop();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief 初始化AI助手系统
 */
void ai_assistant_init(void)
{
    ai_assistant_log_info(TAG, "Initializing AI Assistant v%s", AI_ASSISTANT_VERSION);
    
    // 创建事件组
    ai_event_group = xEventGroupCreate();
    if (ai_event_group == NULL) {
        ai_assistant_log_error(TAG, "Failed to create event group");
        return;
    }
    
    // 创建队列
    voice_queue = xQueueCreate(5, sizeof(voice_recognition_result_t));
    if (voice_queue == NULL) {
        ai_assistant_log_error(TAG, "Failed to create voice queue");
        return;
    }
    
    ai_response_queue = xQueueCreate(5, sizeof(ai_response_t));
    if (ai_response_queue == NULL) {
        ai_assistant_log_error(TAG, "Failed to create AI response queue");
        return;
    }
    
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // 初始化各个组件
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_ERROR_CHECK(ai_engine_init());
    ESP_ERROR_CHECK(voice_recognition_init());
    ESP_ERROR_CHECK(voice_synthesis_init());
    ESP_ERROR_CHECK(audio_processor_init());
    ESP_ERROR_CHECK(nlp_processor_init());
    ESP_ERROR_CHECK(web_server_init());
    ESP_ERROR_CHECK(api_handlers_init());
    
    ai_assistant_log_info(TAG, "AI Assistant initialization completed");
}

/**
 * @brief 启动AI助手
 */
void ai_assistant_start(void)
{
    ai_assistant_log_info(TAG, "Starting AI Assistant");
    
    // 启动WiFi
    wifi_manager_start();
    
    // 等待WiFi连接
    xEventGroupWaitBits(ai_event_group, AI_WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    
    // 启动Web服务器
    web_server_start();
    
    // 创建任务
    xTaskCreatePinnedToCore(voice_processing_task, "voice_task", 4096, NULL, 5, &voice_task_handle, 1);
    xTaskCreatePinnedToCore(ai_response_task, "ai_response_task", 4096, NULL, 5, &ai_processing_task_handle, 1);
    xTaskCreatePinnedToCore(audio_processing_task, "audio_task", 4096, NULL, 4, &audio_task_handle, 0);
    
    // 启动语音识别
    if (g_ai_config.voice_enabled) {
        voice_recognition_start();
        g_ai_state = AI_STATE_LISTENING;
    }
    
    ai_assistant_log_info(TAG, "AI Assistant started successfully");
}

/**
 * @brief 停止AI助手
 */
void ai_assistant_stop(void)
{
    ai_assistant_log_info(TAG, "Stopping AI Assistant");
    
    // 停止语音识别
    voice_recognition_stop();
    
    // 停止语音合成
    voice_synthesis_stop();
    
    // 停止Web服务器
    web_server_stop();
    
    // 删除任务
    if (voice_task_handle) {
        vTaskDelete(voice_task_handle);
        voice_task_handle = NULL;
    }
    
    if (ai_processing_task_handle) {
        vTaskDelete(ai_processing_task_handle);
        ai_processing_task_handle = NULL;
    }
    
    if (audio_task_handle) {
        vTaskDelete(audio_task_handle);
        audio_task_handle = NULL;
    }
    
    g_ai_state = AI_STATE_IDLE;
    ai_assistant_log_info(TAG, "AI Assistant stopped");
}

/**
 * @brief 设置AI助手配置
 * @param config 配置结构指针
 */
void ai_assistant_set_config(const ai_assistant_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Invalid config pointer");
        return;
    }
    
    memcpy(&g_ai_config, config, sizeof(ai_assistant_config_t));
    ai_assistant_log_info(TAG, "AI Assistant config updated");
}

/**
 * @brief 获取AI助手配置
 * @param config 配置结构指针
 */
void ai_assistant_get_config(ai_assistant_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Invalid config pointer");
        return;
    }
    
    memcpy(config, &g_ai_config, sizeof(ai_assistant_config_t));
}

/**
 * @brief 获取状态字符串
 * @param state AI助手状态
 * @return 状态字符串
 */
const char* ai_assistant_get_state_string(ai_assistant_state_t state)
{
    switch (state) {
        case AI_STATE_IDLE: return "IDLE";
        case AI_STATE_LISTENING: return "LISTENING";
        case AI_STATE_PROCESSING: return "PROCESSING";
        case AI_STATE_SPEAKING: return "SPEAKING";
        case AI_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/**
 * @brief 获取语音命令类型字符串
 * @param type 语音命令类型
 * @return 类型字符串
 */
const char* voice_command_get_type_string(voice_command_type_t type)
{
    switch (type) {
        case VOICE_CMD_UNKNOWN: return "UNKNOWN";
        case VOICE_CMD_WEATHER: return "WEATHER";
        case VOICE_CMD_TIME: return "TIME";
        case VOICE_CMD_NEWS: return "NEWS";
        case VOICE_CMD_MUSIC: return "MUSIC";
        case VOICE_CMD_LIGHT: return "LIGHT";
        case VOICE_CMD_TEMPERATURE: return "TEMPERATURE";
        case VOICE_CMD_SETTINGS: return "SETTINGS";
        case VOICE_CMD_HELP: return "HELP";
        default: return "UNKNOWN";
    }
}

/**
 * @brief 日志信息函数
 * @param tag 标签
 * @param format 格式字符串
 * @param ... 可变参数
 */
void ai_assistant_log_info(const char *tag, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_INFO, tag, format, args);
    va_end(args);
}

/**
 * @brief 日志错误函数
 * @param tag 标签
 * @param format 格式字符串
 * @param ... 可变参数
 */
void ai_assistant_log_error(const char *tag, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_ERROR, tag, format, args);
    va_end(args);
}

/**
 * @brief 主函数
 */
void app_main(void)
{
    ai_assistant_log_info(TAG, "=== ESP32-S3 AI Assistant Starting ===");
    ai_assistant_log_info(TAG, "Version: %s", AI_ASSISTANT_VERSION);
    ai_assistant_log_info(TAG, "Build Date: %s %s", __DATE__, __TIME__);
    
    // 初始化AI助手
    ai_assistant_init();
    
    // 启动AI助手
    ai_assistant_start();
    
    ai_assistant_log_info(TAG, "=== AI Assistant Ready ===");
    ai_assistant_log_info(TAG, "Current state: %s", ai_assistant_get_state_string(g_ai_state));
    
    // 主循环
    while (1) {
        // 检查系统状态
        if (g_ai_state == AI_STATE_ERROR) {
            ai_assistant_log_error(TAG, "AI Assistant in error state, attempting recovery");
            // 这里可以实现错误恢复逻辑
            g_ai_state = AI_STATE_IDLE;
        }
        
        // 定期打印状态信息
        static uint32_t last_status_time = 0;
        uint32_t current_time = esp_timer_get_time() / 1000000; // 转换为秒
        
        if (current_time - last_status_time >= 30) { // 每30秒打印一次状态
            ai_assistant_log_info(TAG, "Status: %s, Free heap: %d bytes", 
                                ai_assistant_get_state_string(g_ai_state), 
                                esp_get_free_heap_size());
            last_status_time = current_time;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
