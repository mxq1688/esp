#ifndef NRF24L01_CONFIG_H
#define NRF24L01_CONFIG_H

#include "nrf24l01_driver.h"

// NRF24L01 引脚配置 (ESP32-C3)
#define NRF24L01_CE_PIN      GPIO_NUM_2    // CE引脚
#define NRF24L01_CSN_PIN     GPIO_NUM_3    // CSN引脚
#define NRF24L01_MISO_PIN    GPIO_NUM_4    // MISO引脚
#define NRF24L01_MOSI_PIN    GPIO_NUM_5    // MOSI引脚
#define NRF24L01_SCK_PIN     GPIO_NUM_6    // SCK引脚

// 默认配置参数
#define NRF24L01_DEFAULT_CHANNEL      76    // 默认通道 (2.476GHz)
#define NRF24L01_DEFAULT_DATA_RATE    NRF24L01_RF_DR_1MBPS  // 默认数据速率 1Mbps
#define NRF24L01_DEFAULT_POWER        NRF24L01_RF_PWR_0DBM  // 默认发射功率 0dBm
#define NRF24L01_DEFAULT_PAYLOAD_SIZE 32    // 默认数据包大小

// 默认地址 (5字节)
#define NRF24L01_DEFAULT_ADDRESS      {0x01, 0x02, 0x03, 0x04, 0x05}

// 应用模式定义
typedef enum {
    NRF24L01_MODE_SENDER = 0,    // 发送模式
    NRF24L01_MODE_RECEIVER,      // 接收模式
    NRF24L01_MODE_BOTH           // 双向模式
} nrf24l01_mode_t;

// 应用配置结构体
typedef struct {
    nrf24l01_mode_t mode;        // 工作模式
    uint8_t channel;             // 通道号
    uint8_t data_rate;           // 数据速率
    uint8_t power;               // 发射功率
    uint8_t payload_size;        // 数据包大小
    uint8_t address[5];          // 地址
    uint32_t send_interval_ms;   // 发送间隔(ms)
    bool auto_ack;               // 自动应答
} nrf24l01_app_config_t;

// 函数声明
esp_err_t nrf24l01_get_default_config(nrf24l01_config_t *config);
esp_err_t nrf24l01_get_default_app_config(nrf24l01_app_config_t *app_config);
void nrf24l01_print_config(nrf24l01_config_t *config);
void nrf24l01_print_app_config(nrf24l01_app_config_t *app_config);

#endif // NRF24L01_CONFIG_H
