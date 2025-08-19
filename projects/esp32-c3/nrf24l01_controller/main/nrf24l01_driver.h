#ifndef NRF24L01_DRIVER_H
#define NRF24L01_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

// NRF24L01 寄存器地址
#define NRF24L01_CONFIG      0x00
#define NRF24L01_EN_AA       0x01
#define NRF24L01_EN_RXADDR   0x02
#define NRF24L01_SETUP_AW    0x03
#define NRF24L01_SETUP_RETR  0x04
#define NRF24L01_RF_CH       0x05
#define NRF24L01_RF_SETUP    0x06
#define NRF24L01_STATUS      0x07
#define NRF24L01_OBSERVE_TX  0x08
#define NRF24L01_CD          0x09
#define NRF24L01_RX_ADDR_P0  0x0A
#define NRF24L01_RX_ADDR_P1  0x0B
#define NRF24L01_RX_ADDR_P2  0x0C
#define NRF24L01_RX_ADDR_P3  0x0D
#define NRF24L01_RX_ADDR_P4  0x0E
#define NRF24L01_RX_ADDR_P5  0x0F
#define NRF24L01_TX_ADDR     0x10
#define NRF24L01_RX_PW_P0    0x11
#define NRF24L01_RX_PW_P1    0x12
#define NRF24L01_RX_PW_P2    0x13
#define NRF24L01_RX_PW_P3    0x14
#define NRF24L01_RX_PW_P4    0x15
#define NRF24L01_RX_PW_P5    0x16
#define NRF24L01_FIFO_STATUS 0x17
#define NRF24L01_DYNPD       0x1C
#define NRF24L01_FEATURE     0x1D

// NRF24L01 命令
#define NRF24L01_CMD_R_REGISTER    0x00
#define NRF24L01_CMD_W_REGISTER    0x20
#define NRF24L01_CMD_R_RX_PAYLOAD  0x61
#define NRF24L01_CMD_W_TX_PAYLOAD  0xA0
#define NRF24L01_CMD_FLUSH_TX      0xE1
#define NRF24L01_CMD_FLUSH_RX      0xE2
#define NRF24L01_CMD_REUSE_TX_PL   0xE3
#define NRF24L01_CMD_NOP           0xFF

// 状态位
#define NRF24L01_STATUS_RX_DR      0x40
#define NRF24L01_STATUS_TX_DS      0x20
#define NRF24L01_STATUS_MAX_RT     0x10

// 配置位
#define NRF24L01_CONFIG_MASK_RX_DR 0x40
#define NRF24L01_CONFIG_MASK_TX_DS 0x20
#define NRF24L01_CONFIG_MASK_MAX_RT 0x10
#define NRF24L01_CONFIG_EN_CRC     0x08
#define NRF24L01_CONFIG_CRCO       0x04
#define NRF24L01_CONFIG_PWR_UP     0x02
#define NRF24L01_CONFIG_PRIM_RX    0x01

// 数据速率
#define NRF24L01_RF_DR_250KBPS     0x20
#define NRF24L01_RF_DR_1MBPS       0x00
#define NRF24L01_RF_DR_2MBPS       0x08

// 发射功率
#define NRF24L01_RF_PWR_18DBM      0x00
#define NRF24L01_RF_PWR_12DBM      0x02
#define NRF24L01_RF_PWR_6DBM       0x04
#define NRF24L01_RF_PWR_0DBM       0x06

// 默认地址
#define NRF24L01_DEFAULT_ADDR      {0x01, 0x02, 0x03, 0x04, 0x05}

// 最大数据包大小
#define NRF24L01_MAX_PAYLOAD_SIZE  32

// NRF24L01 配置结构体
typedef struct {
    spi_host_device_t spi_host;
    gpio_num_t ce_pin;
    gpio_num_t csn_pin;
    gpio_num_t miso_pin;
    gpio_num_t mosi_pin;
    gpio_num_t sck_pin;
    uint8_t channel;
    uint8_t data_rate;
    uint8_t power;
    uint8_t payload_size;
    uint8_t address[5];
    bool is_initialized;
} nrf24l01_config_t;

// 数据包结构体
typedef struct {
    uint8_t data[NRF24L01_MAX_PAYLOAD_SIZE];
    uint8_t length;
    uint8_t pipe;
} nrf24l01_packet_t;

// 函数声明
esp_err_t nrf24l01_init(nrf24l01_config_t *config);
esp_err_t nrf24l01_deinit(void);
esp_err_t nrf24l01_set_mode_tx(void);
esp_err_t nrf24l01_set_mode_rx(void);
esp_err_t nrf24l01_set_channel(uint8_t channel);
esp_err_t nrf24l01_set_data_rate(uint8_t data_rate);
esp_err_t nrf24l01_set_power(uint8_t power);
esp_err_t nrf24l01_set_address(uint8_t *address);
esp_err_t nrf24l01_set_payload_size(uint8_t size);
esp_err_t nrf24l01_send_packet(uint8_t *data, uint8_t length);
esp_err_t nrf24l01_receive_packet(nrf24l01_packet_t *packet);
esp_err_t nrf24l01_available(void);
esp_err_t nrf24l01_flush_rx(void);
esp_err_t nrf24l01_flush_tx(void);
uint8_t nrf24l01_read_status(void);
esp_err_t nrf24l01_write_register(uint8_t reg, uint8_t value);
uint8_t nrf24l01_read_register(uint8_t reg);
esp_err_t nrf24l01_write_bytes(uint8_t reg, uint8_t *data, uint8_t length);
esp_err_t nrf24l01_read_bytes(uint8_t reg, uint8_t *data, uint8_t length);

#endif // NRF24L01_DRIVER_H
