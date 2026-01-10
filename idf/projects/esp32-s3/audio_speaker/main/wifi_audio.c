/**
 * @file wifi_audio.c
 * @brief WiFi 网络音频播放器实现
 * 
 * 支持从网络下载 WAV 音频并播放
 */

#include "include/wifi_audio.h"
#include "include/audio_player.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_client.h"

static const char *TAG = "wifi_audio";

// WiFi 事件组
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// 重试计数
static int s_retry_num = 0;
#define MAX_RETRY 5

/**
 * @brief WiFi 事件处理器
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "重试连接 WiFi... (%d/%d)", s_retry_num, MAX_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "WiFi 连接断开");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "获取到 IP 地址: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief 初始化 WiFi
 */
esp_err_t wifi_audio_init_wifi(void)
{
    ESP_LOGI(TAG, "初始化 WiFi...");
    
    // 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 创建事件组
    s_wifi_event_group = xEventGroupCreate();
    
    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    // WiFi 初始化
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册事件处理器
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    // 配置 WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi 初始化完成，正在连接到: %s", WIFI_SSID);
    
    return ESP_OK;
}

/**
 * @brief 等待 WiFi 连接
 */
esp_err_t wifi_audio_wait_connected(uint32_t timeout_ms)
{
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(timeout_ms));
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi 连接成功!");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "WiFi 连接失败!");
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "WiFi 连接超时!");
        return ESP_ERR_TIMEOUT;
    }
}

// WAV 文件头结构
typedef struct __attribute__((packed)) {
    char riff[4];           // "RIFF"
    uint32_t file_size;     // 文件大小 - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmt_size;      // fmt 块大小
    uint16_t audio_format;  // 音频格式 (1 = PCM)
    uint16_t num_channels;  // 声道数
    uint32_t sample_rate;   // 采样率
    uint32_t byte_rate;     // 字节率
    uint16_t block_align;   // 块对齐
    uint16_t bits_per_sample; // 位深度
} wav_header_t;

/**
 * @brief HTTP 事件处理器
 */
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP 错误");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP 已连接");
            break;
        case HTTP_EVENT_ON_DATA:
            // 数据在主函数中处理
            break;
        default:
            break;
    }
    return ESP_OK;
}

/**
 * @brief 从网络播放 WAV 音频
 */
esp_err_t wifi_audio_play_url(const char *url)
{
    ESP_LOGI(TAG, "开始下载音频: %s", url);
    
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .buffer_size = 4096,
        .timeout_ms = 10000,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "HTTP 客户端初始化失败");
        return ESP_FAIL;
    }
    
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP 连接失败: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }
    
    int content_length = esp_http_client_fetch_headers(client);
    ESP_LOGI(TAG, "文件大小: %d 字节", content_length);
    
    // 读取 WAV 头
    wav_header_t wav_header;
    int read_len = esp_http_client_read(client, (char *)&wav_header, sizeof(wav_header));
    if (read_len != sizeof(wav_header)) {
        ESP_LOGE(TAG, "读取 WAV 头失败");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    // 验证 WAV 格式
    if (strncmp(wav_header.riff, "RIFF", 4) != 0 || strncmp(wav_header.wave, "WAVE", 4) != 0) {
        ESP_LOGE(TAG, "不是有效的 WAV 文件");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "WAV 信息: %d Hz, %d 声道, %d 位",
             (int)wav_header.sample_rate,
             wav_header.num_channels,
             wav_header.bits_per_sample);
    
    // 根据 WAV 采样率重新配置 I2S
    esp_err_t ret = audio_player_set_sample_rate(wav_header.sample_rate);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置采样率失败");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ret;
    }
    
    // 跳过可能存在的额外头部数据
    char chunk_id[4];
    uint32_t chunk_size;
    while (1) {
        read_len = esp_http_client_read(client, chunk_id, 4);
        if (read_len != 4) break;
        
        read_len = esp_http_client_read(client, (char *)&chunk_size, 4);
        if (read_len != 4) break;
        
        if (strncmp(chunk_id, "data", 4) == 0) {
            ESP_LOGI(TAG, "音频数据大小: %lu 字节", (unsigned long)chunk_size);
            break;
        }
        
        // 跳过非 data 块
        char skip_buf[256];
        uint32_t remaining = chunk_size;
        while (remaining > 0) {
            int to_read = (remaining > sizeof(skip_buf)) ? sizeof(skip_buf) : remaining;
            read_len = esp_http_client_read(client, skip_buf, to_read);
            if (read_len <= 0) break;
            remaining -= read_len;
        }
    }
    
    // 分配音频缓冲区
    const int buffer_size = 4096;
    int16_t *audio_buffer = (int16_t *)malloc(buffer_size);
    if (audio_buffer == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_NO_MEM;
    }
    
    // 流式播放音频
    ESP_LOGI(TAG, "开始播放音频...");
    int total_read = 0;
    
    while (1) {
        read_len = esp_http_client_read(client, (char *)audio_buffer, buffer_size);
        if (read_len <= 0) {
            break;
        }
        
        total_read += read_len;
        
        // 播放音频数据
        // 如果是单声道，需要转换为立体声
        if (wav_header.num_channels == 1) {
            // 单声道数据
            int16_t *stereo_buf = (int16_t *)malloc(read_len * 2);
            if (stereo_buf) {
                int sample_count = read_len / sizeof(int16_t);
                for (int i = 0; i < sample_count; i++) {
                    stereo_buf[i * 2] = audio_buffer[i];
                    stereo_buf[i * 2 + 1] = audio_buffer[i];
                }
                audio_player_play(audio_buffer, read_len, 1000);
                free(stereo_buf);
            }
        } else {
            // 立体声数据，直接发送
            size_t bytes_written = 0;
            extern esp_err_t audio_player_play_stereo(const int16_t *data, size_t len, uint32_t wait_ms);
            audio_player_play_stereo(audio_buffer, read_len, 1000);
        }
        
        // 显示进度
        if (content_length > 0 && (total_read % (buffer_size * 10)) == 0) {
            ESP_LOGI(TAG, "播放进度: %d%%", (total_read * 100) / content_length);
        }
    }
    
    ESP_LOGI(TAG, "音频播放完成，共播放 %d 字节", total_read);
    
    free(audio_buffer);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    
    return ESP_OK;
}

/**
 * @brief 播放网络测试音频
 */
esp_err_t wifi_audio_play_test(void)
{
    // NASA 公开的音频 - "Houston, we have a problem"
    // PCM WAV 格式
    const char *test_url = "http://soundbible.com/grab.php?id=1817&type=wav";
    
    ESP_LOGI(TAG, "播放网络音频...");
    return wifi_audio_play_url(test_url);
}

