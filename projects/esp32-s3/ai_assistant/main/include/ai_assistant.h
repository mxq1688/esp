#ifndef AI_ASSISTANT_H
#define AI_ASSISTANT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "driver/dac.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "esp_ringbuf.h"
#include "esp_psram.h"
#include "esp_mm.h"
#include "esp_lcd.h"
#include "esp_driver_cam.h"
#include "esp_driver_isp.h"
#include "esp_driver_jpeg.h"
#include "esp_driver_ppa.h"
#include "esp_driver_sdmmc.h"
#include "esp_driver_sdspi.h"
#include "esp_driver_sd_intf.h"
#include "esp_driver_spi.h"
#include "esp_driver_touch_sens.h"
#include "esp_driver_tsens.h"
#include "esp_driver_twai.h"
#include "esp_driver_usb_serial_jtag.h"
#include "esp_driver_ana_cmpr.h"
#include "esp_driver_bitscrambler.h"
#include "esp_driver_gptimer.h"
#include "esp_driver_i2c.h"
#include "esp_driver_ledc.h"
#include "esp_driver_mcpwm.h"
#include "esp_driver_parlio.h"
#include "esp_driver_pcnt.h"
#include "esp_driver_rmt.h"
#include "esp_driver_sdm.h"
#include "esp_driver_touch_sens.h"
#include "esp_driver_tsens.h"
#include "esp_driver_twai.h"
#include "esp_driver_uart.h"
#include "esp_driver_usb_serial_jtag.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_gdbstub.h"
#include "esp_hid.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_https_ota.h"
#include "esp_https_server.h"
#include "esp_hw_support.h"
#include "esp_lcd.h"
#include "esp_local_ctrl.h"
#include "esp_mm.h"
#include "esp_netif.h"
#include "esp_netif_stack.h"
#include "esp_partition.h"
#include "esp_phy.h"
#include "esp_pm.h"
#include "esp_psram.h"
#include "esp_ringbuf.h"
#include "esp_rom.h"
#include "esp_security.h"
#include "esp_system.h"
#include "esp_tee.h"
#include "esp_timer.h"
#include "esp_vfs_console.h"
#include "esp_wifi.h"
#include "esp-tls.h"
#include "espcoredump.h"
#include "esptool_py.h"
#include "fatfs.h"
#include "freertos.h"
#include "hal.h"
#include "heap.h"
#include "http_parser.h"
#include "idf_test.h"
#include "ieee802154.h"
#include "json.h"
#include "linux.h"
#include "log.h"
#include "lwip.h"
#include "mbedtls.h"
#include "mqtt.h"
#include "newlib.h"
#include "nvs_flash.h"
#include "nvs_sec_provider.h"
#include "openthread.h"
#include "partition_table.h"
#include "perfmon.h"
#include "protobuf-c.h"
#include "protocomm.h"
#include "pthread.h"
#include "riscv.h"
#include "rt.h"
#include "sdmmc.h"
#include "soc.h"
#include "spi_flash.h"
#include "spiffs.h"
#include "tcp_transport.h"
#include "touch_element.h"
#include "ulp.h"
#include "unity.h"
#include "usb.h"
#include "vfs.h"
#include "wear_levelling.h"
#include "wifi_provisioning.h"
#include "wpa_supplicant.h"
#include "xtensa.h"

// 项目版本信息
#define AI_ASSISTANT_VERSION "1.0.0"
#define AI_ASSISTANT_NAME "ESP32-S3 AI Assistant"

// 系统配置
#define MAX_WIFI_SSID_LEN 32
#define MAX_WIFI_PASSWORD_LEN 64
#define MAX_AI_RESPONSE_LEN 512
#define MAX_VOICE_COMMAND_LEN 256
#define MAX_AUDIO_BUFFER_SIZE 4096

// GPIO配置
#define LED_GPIO_PIN 2
#define MICROPHONE_GPIO_PIN 4
#define SPEAKER_GPIO_PIN 5
#define BUTTON_GPIO_PIN 0

// I2S配置
#define I2S_SAMPLE_RATE 16000
#define I2S_BITS_PER_SAMPLE 16
#define I2S_CHANNELS 1

// AI助手状态
typedef enum {
    AI_STATE_IDLE = 0,
    AI_STATE_LISTENING,
    AI_STATE_PROCESSING,
    AI_STATE_SPEAKING,
    AI_STATE_ERROR
} ai_assistant_state_t;

// 语音命令类型
typedef enum {
    VOICE_CMD_UNKNOWN = 0,
    VOICE_CMD_WEATHER,
    VOICE_CMD_TIME,
    VOICE_CMD_NEWS,
    VOICE_CMD_MUSIC,
    VOICE_CMD_LIGHT,
    VOICE_CMD_TEMPERATURE,
    VOICE_CMD_SETTINGS,
    VOICE_CMD_HELP
} voice_command_type_t;

// AI助手配置结构
typedef struct {
    char wifi_ssid[MAX_WIFI_SSID_LEN];
    char wifi_password[MAX_WIFI_PASSWORD_LEN];
    bool voice_enabled;
    bool auto_wake_up;
    uint8_t volume_level;
    uint8_t language;
    char api_key[64];
    char server_url[128];
} ai_assistant_config_t;

// 语音识别结果结构
typedef struct {
    char text[MAX_VOICE_COMMAND_LEN];
    float confidence;
    voice_command_type_t command_type;
    uint32_t timestamp;
} voice_recognition_result_t;

// AI响应结构
typedef struct {
    char text[MAX_AI_RESPONSE_LEN];
    char audio_url[256];
    bool has_audio;
    uint32_t timestamp;
} ai_response_t;

// 全局变量声明
extern ai_assistant_config_t g_ai_config;
extern ai_assistant_state_t g_ai_state;
extern QueueHandle_t voice_queue;
extern QueueHandle_t ai_response_queue;
extern EventGroupHandle_t ai_event_group;

// 事件位定义
#define AI_WIFI_CONNECTED_BIT BIT0
#define AI_VOICE_DETECTED_BIT BIT1
#define AI_PROCESSING_DONE_BIT BIT2
#define AI_SPEAKING_DONE_BIT BIT3

// 函数声明
void ai_assistant_init(void);
void ai_assistant_start(void);
void ai_assistant_stop(void);
void ai_assistant_set_config(const ai_assistant_config_t *config);
void ai_assistant_get_config(ai_assistant_config_t *config);

// 语音识别相关
esp_err_t voice_recognition_init(void);
esp_err_t voice_recognition_start(void);
esp_err_t voice_recognition_stop(void);
esp_err_t voice_recognition_process(const uint8_t *audio_data, size_t len);

// 语音合成相关
esp_err_t voice_synthesis_init(void);
esp_err_t voice_synthesis_speak(const char *text);
esp_err_t voice_synthesis_stop(void);

// AI引擎相关
esp_err_t ai_engine_init(void);
esp_err_t ai_engine_process_command(const char *command, ai_response_t *response);
esp_err_t ai_engine_set_api_key(const char *api_key);

// 自然语言处理相关
esp_err_t nlp_processor_init(void);
voice_command_type_t nlp_processor_classify_command(const char *text);
esp_err_t nlp_processor_extract_intent(const char *text, char *intent, size_t intent_len);

// 音频处理相关
esp_err_t audio_processor_init(void);
esp_err_t audio_processor_record_start(void);
esp_err_t audio_processor_record_stop(void);
esp_err_t audio_processor_play_start(const uint8_t *audio_data, size_t len);
esp_err_t audio_processor_play_stop(void);

// Web服务器相关
esp_err_t web_server_init(void);
esp_err_t web_server_start(void);
esp_err_t web_server_stop(void);

// API处理器相关
esp_err_t api_handlers_init(void);
esp_err_t api_handlers_register(httpd_handle_t server);

// 工具函数
const char* ai_assistant_get_state_string(ai_assistant_state_t state);
const char* voice_command_get_type_string(voice_command_type_t type);
void ai_assistant_log_info(const char *tag, const char *format, ...);
void ai_assistant_log_error(const char *tag, const char *format, ...);

#endif // AI_ASSISTANT_H
