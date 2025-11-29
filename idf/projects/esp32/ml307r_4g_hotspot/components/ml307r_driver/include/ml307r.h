#ifndef ML307R_H
#define ML307R_H

#include "esp_err.h"
#include "driver/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

// ML307R配置
#define ML307R_UART_NUM        UART_NUM_2
#define ML307R_TXD_PIN         17
#define ML307R_RXD_PIN         16
#define ML307R_BAUD_RATE       115200
#define ML307R_BUFFER_SIZE     1024
#define ML307R_TIMEOUT_MS      5000

// AT命令定义
#define ML307R_AT_TEST         "AT\r\n"
#define ML307R_AT_ECHO_OFF     "ATE0\r\n"
#define ML307R_AT_CPIN         "AT+CPIN?\r\n"
#define ML307R_AT_CREG         "AT+CREG?\r\n"
#define ML307R_AT_CGREG        "AT+CGREG?\r\n"
#define ML307R_AT_CGACT        "AT+CGACT=1,1\r\n"
#define ML307R_AT_CGPADDR      "AT+CGPADDR=1\r\n"
#define ML307R_AT_CGDCONT      "AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n"
#define ML307R_AT_CGATT        "AT+CGATT=1\r\n"

// 响应状态
typedef enum {
    ML307R_OK = 0,
    ML307R_ERROR,
    ML307R_TIMEOUT,
    ML307R_NOT_READY
} ml307r_status_t;

// 网络状态
typedef enum {
    ML307R_NET_NOT_REGISTERED = 0,
    ML307R_NET_REGISTERED_HOME,
    ML307R_NET_SEARCHING,
    ML307R_NET_DENIED,
    ML307R_NET_UNKNOWN,
    ML307R_NET_REGISTERED_ROAMING
} ml307r_net_status_t;

// ML307R结构体
typedef struct {
    uart_port_t uart_num;
    int txd_pin;
    int rxd_pin;
    uint32_t baud_rate;
    bool initialized;
    ml307r_net_status_t net_status;
    char ip_address[16];
} ml307r_t;

/**
 * @brief 初始化ML307R模块
 * 
 * @param ml307r ML307R结构体指针
 * @return esp_err_t 
 */
esp_err_t ml307r_init(ml307r_t *ml307r);

/**
 * @brief 发送AT命令
 * 
 * @param ml307r ML307R结构体指针
 * @param command AT命令字符串
 * @return esp_err_t 
 */
esp_err_t ml307r_send_command(ml307r_t *ml307r, const char *command);

/**
 * @brief 读取响应
 * 
 * @param ml307r ML307R结构体指针
 * @param response 响应缓冲区
 * @param max_len 最大长度
 * @param timeout_ms 超时时间(毫秒)
 * @return int 实际读取的字节数
 */
int ml307r_read_response(ml307r_t *ml307r, char *response, size_t max_len, uint32_t timeout_ms);

/**
 * @brief 等待特定响应
 * 
 * @param ml307r ML307R结构体指针
 * @param expected 期望的响应字符串
 * @param timeout_ms 超时时间(毫秒)
 * @return ml307r_status_t 
 */
ml307r_status_t ml307r_wait_response(ml307r_t *ml307r, const char *expected, uint32_t timeout_ms);

/**
 * @brief 检查SIM卡状态
 * 
 * @param ml307r ML307R结构体指针
 * @return ml307r_status_t 
 */
ml307r_status_t ml307r_check_sim(ml307r_t *ml307r);

/**
 * @brief 检查网络注册状态
 * 
 * @param ml307r ML307R结构体指针
 * @return ml307r_status_t 
 */
ml307r_status_t ml307r_check_network(ml307r_t *ml307r);

/**
 * @brief 激活PDP上下文
 * 
 * @param ml307r ML307R结构体指针
 * @return ml307r_status_t 
 */
ml307r_status_t ml307r_activate_pdp(ml307r_t *ml307r);

/**
 * @brief 获取IP地址
 * 
 * @param ml307r ML307R结构体指针
 * @return ml307r_status_t 
 */
ml307r_status_t ml307r_get_ip_address(ml307r_t *ml307r);

/**
 * @brief 完整的4G连接初始化
 * 
 * @param ml307r ML307R结构体指针
 * @return ml307r_status_t 
 */
ml307r_status_t ml307r_connect_4g(ml307r_t *ml307r);

#ifdef __cplusplus
}
#endif

#endif // ML307R_H
