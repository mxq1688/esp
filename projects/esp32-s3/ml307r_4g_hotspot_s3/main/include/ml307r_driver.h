#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

// ML307R UART配置
#define ML307R_UART_NUM         UART_NUM_1
#define ML307R_UART_TX_PIN      17  // ESP32-S3 GPIO17 -> ML307R RXD (开发板可用引脚)
#define ML307R_UART_RX_PIN      18  // ESP32-S3 GPIO18 -> ML307R TXD (开发板可用引脚)
#define ML307R_UART_BAUD_RATE   115200  // ML307R标准波特率
#define ML307R_UART_BUF_SIZE    2048    // 增大缓冲区

// ML307R控制引脚 (可选，如果没有硬件连接可以注释掉)
#define ML307R_POWER_PIN        -1  // 电源控制引脚 (-1表示不使用)
#define ML307R_RESET_PIN        -1  // 复位引脚 (-1表示不使用)

// AT命令超时时间
#define ML307R_AT_TIMEOUT_MS    10000   // 增加超时时间
#define ML307R_RESPONSE_BUF_SIZE 1024   // 增大响应缓冲区
#define ML307R_STARTUP_DELAY_MS 5000    // 模块启动延迟

// ML307R状态
typedef enum {
    ML307R_STATE_UNKNOWN = 0,
    ML307R_STATE_INIT,
    ML307R_STATE_READY,
    ML307R_STATE_CONNECTING,
    ML307R_STATE_CONNECTED,
    ML307R_STATE_ERROR
} ml307r_state_t;

// 网络信息结构体
typedef struct {
    char operator_name[32];
    int signal_strength;  // dBm
    char network_type[16]; // 2G/3G/4G
    char ip_address[16];
    bool is_connected;
} ml307r_network_info_t;

// 热点配置结构体
typedef struct {
    char ssid[32];
    char password[64];
    int max_connections;
    bool is_enabled;
} ml307r_hotspot_config_t;

/**
 * @brief 初始化ML307R模块
 * 
 * @return esp_err_t 
 */
esp_err_t ml307r_init(void);

/**
 * @brief 反初始化ML307R模块
 * 
 * @return esp_err_t 
 */
esp_err_t ml307r_deinit(void);

/**
 * @brief 发送AT命令
 * 
 * @param command AT命令字符串
 * @param response 响应缓冲区
 * @param response_size 响应缓冲区大小
 * @param timeout_ms 超时时间(毫秒)
 * @return esp_err_t 
 */
esp_err_t ml307r_send_at_command(const char *command, char *response, size_t response_size, uint32_t timeout_ms);

/**
 * @brief 检查模块是否就绪
 * 
 * @return true 模块就绪
 * @return false 模块未就绪
 */
bool ml307r_is_ready(void);

/**
 * @brief 获取网络信息
 * 
 * @param info 网络信息结构体指针
 * @return esp_err_t 
 */
esp_err_t ml307r_get_network_info(ml307r_network_info_t *info);

/**
 * @brief 启用4G热点
 * 
 * @param config 热点配置
 * @return esp_err_t 
 */
esp_err_t ml307r_enable_hotspot(const ml307r_hotspot_config_t *config);

/**
 * @brief 禁用4G热点
 * 
 * @return esp_err_t 
 */
esp_err_t ml307r_disable_hotspot(void);

/**
 * @brief 获取热点状态
 * 
 * @param config 热点配置结构体指针
 * @return esp_err_t 
 */
esp_err_t ml307r_get_hotspot_status(ml307r_hotspot_config_t *config);

/**
 * @brief 重启ML307R模块
 * 
 * @return esp_err_t 
 */
esp_err_t ml307r_reset(void);

/**
 * @brief 获取模块状态
 * 
 * @return ml307r_state_t 
 */
ml307r_state_t ml307r_get_state(void);

/**
 * @brief 获取信号强度
 * 
 * @return int 信号强度(dBm)，负值表示错误
 */
int ml307r_get_signal_strength(void);

/**
 * @brief 建立4G数据连接
 * 
 * @return esp_err_t 
 */
esp_err_t ml307r_establish_data_connection(void);

/**
 * @brief 断开4G数据连接
 * 
 * @return esp_err_t 
 */
esp_err_t ml307r_disconnect_data_connection(void);

#ifdef __cplusplus
}
#endif
