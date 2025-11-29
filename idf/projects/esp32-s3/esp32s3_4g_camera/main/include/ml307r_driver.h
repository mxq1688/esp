#ifndef ML307R_DRIVER_H
#define ML307R_DRIVER_H

#include "esp_err.h"
#include "driver/uart.h"
#include <stdbool.h>

// ML307R UART配置
#define ML307R_UART_NUM         UART_NUM_1
#define ML307R_UART_TX_PIN      17
#define ML307R_UART_RX_PIN      18
#define ML307R_UART_BAUD_RATE   115200
#define ML307R_UART_BUF_SIZE    2048

// ML307R 控制引脚 (可选，如果硬件没有引出则设为-1)
#define ML307R_POWER_PIN        -1
#define ML307R_RESET_PIN        -1

// AT命令超时和缓冲区
#define ML307R_AT_TIMEOUT_MS       5000
#define ML307R_RESPONSE_BUF_SIZE   512
#define ML307R_STARTUP_DELAY_MS    5000

// ML307R状态
typedef enum {
    ML307R_STATE_UNKNOWN = 0,
    ML307R_STATE_INIT,
    ML307R_STATE_READY,
    ML307R_STATE_CONNECTED,
    ML307R_STATE_ERROR
} ml307r_state_t;

// 网络信息
typedef struct {
    char operator_name[32];    // 运营商名称
    char network_type[16];     // 网络类型 (2G/3G/4G)
    int signal_strength;       // 信号强度 (dBm)
    bool is_connected;         // 是否已连接
    char ip_address[16];       // IP地址
} ml307r_network_info_t;

// 热点配置
typedef struct {
    char ssid[32];             // 热点SSID
    char password[64];         // 热点密码
    uint8_t max_connections;   // 最大连接数
    bool is_enabled;           // 是否已启用
} ml307r_hotspot_config_t;

/**
 * @brief 初始化ML307R模块
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_init(void);

/**
 * @brief 反初始化ML307R模块
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_deinit(void);

/**
 * @brief 发送AT命令
 * 
 * @param command AT命令
 * @param response 响应缓冲区
 * @param response_size 缓冲区大小
 * @param timeout_ms 超时时间(毫秒)
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_send_at_command(const char *command, char *response, 
                                  size_t response_size, uint32_t timeout_ms);

/**
 * @brief 检查ML307R是否就绪
 * 
 * @return true 就绪, false 未就绪
 */
bool ml307r_is_ready(void);

/**
 * @brief 获取网络信息
 * 
 * @param info 保存网络信息的指针
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_get_network_info(ml307r_network_info_t *info);

/**
 * @brief 启用4G热点
 * 
 * @param config 热点配置
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_enable_hotspot(const ml307r_hotspot_config_t *config);

/**
 * @brief 禁用4G热点
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_disable_hotspot(void);

/**
 * @brief 获取热点状态
 * 
 * @param config 保存热点配置的指针
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_get_hotspot_status(ml307r_hotspot_config_t *config);

/**
 * @brief 重启ML307R模块
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_reset(void);

/**
 * @brief 获取ML307R状态
 * 
 * @return ml307r_state_t 当前状态
 */
ml307r_state_t ml307r_get_state(void);

/**
 * @brief 获取信号强度
 * 
 * @return int 信号强度(dBm)
 */
int ml307r_get_signal_strength(void);

/**
 * @brief 建立4G数据连接
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_establish_data_connection(void);

/**
 * @brief 断开4G数据连接
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t ml307r_disconnect_data_connection(void);

#endif // ML307R_DRIVER_H

