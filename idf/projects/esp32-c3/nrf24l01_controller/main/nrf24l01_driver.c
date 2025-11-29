#include "nrf24l01_driver.h"

static const char *TAG = "NRF24L01";

// 全局配置
static nrf24l01_config_t g_nrf24l01_config;
static spi_device_handle_t g_spi_handle;

// SPI事务结构体 (已移除，使用局部变量)

// 私有函数声明
static esp_err_t nrf24l01_spi_init(nrf24l01_config_t *config);
static esp_err_t nrf24l01_spi_transfer(uint8_t *tx_data, uint8_t *rx_data, uint8_t length);
static void nrf24l01_ce_high(void);
static void nrf24l01_ce_low(void);
static void nrf24l01_csn_high(void);
static void nrf24l01_csn_low(void);
static esp_err_t nrf24l01_reset(void);
static esp_err_t nrf24l01_power_up(void);
static esp_err_t nrf24l01_power_down(void);

// SPI初始化
static esp_err_t nrf24l01_spi_init(nrf24l01_config_t *config)
{
    esp_err_t ret;
    
    spi_bus_config_t bus_config = {
        .miso_io_num = config->miso_pin,
        .mosi_io_num = config->mosi_pin,
        .sclk_io_num = config->sck_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    
    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 1000000,  // 1MHz
        .mode = 0,                  // SPI mode 0
        .spics_io_num = -1,         // 手动控制CS
        .queue_size = 1,
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
    };
    
    // 初始化SPI总线
    ret = spi_bus_initialize(config->spi_host, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI总线初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 添加SPI设备
    ret = spi_bus_add_device(config->spi_host, &dev_config, &g_spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI设备添加失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 配置GPIO引脚
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << config->ce_pin) | (1ULL << config->csn_pin),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    // 初始化引脚状态
    nrf24l01_ce_low();
    nrf24l01_csn_high();
    
    return ESP_OK;
}

// SPI传输
static esp_err_t nrf24l01_spi_transfer(uint8_t *tx_data, uint8_t *rx_data, uint8_t length)
{
    spi_transaction_t trans = {
        .length = length * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    
    return spi_device_transmit(g_spi_handle, &trans);
}

// CE引脚控制
static void nrf24l01_ce_high(void)
{
    gpio_set_level(g_nrf24l01_config.ce_pin, 1);
}

static void nrf24l01_ce_low(void)
{
    gpio_set_level(g_nrf24l01_config.ce_pin, 0);
}

// CSN引脚控制
static void nrf24l01_csn_high(void)
{
    gpio_set_level(g_nrf24l01_config.csn_pin, 1);
}

static void nrf24l01_csn_low(void)
{
    gpio_set_level(g_nrf24l01_config.csn_pin, 0);
}

// 复位NRF24L01
static esp_err_t nrf24l01_reset(void)
{
    esp_err_t ret;
    
    // 复位所有寄存器
    ret = nrf24l01_write_register(NRF24L01_CONFIG, 0x0C);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_EN_AA, 0x3F);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_EN_RXADDR, 0x03);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_SETUP_AW, 0x03);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_SETUP_RETR, 0x03);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RF_CH, 0x02);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RF_SETUP, 0x0E);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_STATUS, 0x70);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_OBSERVE_TX, 0x00);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_CD, 0x00);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_ADDR_P0, 0xE7);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_ADDR_P1, 0xC2);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_ADDR_P2, 0xC3);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_ADDR_P3, 0xC4);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_ADDR_P4, 0xC5);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_ADDR_P5, 0xC6);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_TX_ADDR, 0xE7);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_PW_P0, 0);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_PW_P1, 0);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_PW_P2, 0);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_PW_P3, 0);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_PW_P4, 0);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_RX_PW_P5, 0);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_FIFO_STATUS, 0x11);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_DYNPD, 0);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_write_register(NRF24L01_FEATURE, 0);
    if (ret != ESP_OK) return ret;
    
    return ESP_OK;
}

// 上电
static esp_err_t nrf24l01_power_up(void)
{
    uint8_t config = nrf24l01_read_register(NRF24L01_CONFIG);
    config |= NRF24L01_CONFIG_PWR_UP;
    return nrf24l01_write_register(NRF24L01_CONFIG, config);
}

// 下电
static esp_err_t nrf24l01_power_down(void)
{
    uint8_t config = nrf24l01_read_register(NRF24L01_CONFIG);
    config &= ~NRF24L01_CONFIG_PWR_UP;
    return nrf24l01_write_register(NRF24L01_CONFIG, config);
}

// 公共函数实现

esp_err_t nrf24l01_init(nrf24l01_config_t *config)
{
    esp_err_t ret;
    
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 复制配置
    memcpy(&g_nrf24l01_config, config, sizeof(nrf24l01_config_t));
    
    ESP_LOGI(TAG, "初始化NRF24L01...");
    
    // 初始化SPI
    ret = nrf24l01_spi_init(config);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // 等待芯片稳定
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // 复位芯片
    ret = nrf24l01_reset();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NRF24L01复位失败");
        return ret;
    }
    
    // 上电
    ret = nrf24l01_power_up();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NRF24L01上电失败");
        return ret;
    }
    
    // 等待上电完成
    vTaskDelay(pdMS_TO_TICKS(2));
    
    // 设置默认配置
    ret = nrf24l01_set_channel(config->channel);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_set_data_rate(config->data_rate);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_set_power(config->power);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_set_address(config->address);
    if (ret != ESP_OK) return ret;
    
    ret = nrf24l01_set_payload_size(config->payload_size);
    if (ret != ESP_OK) return ret;
    
    g_nrf24l01_config.is_initialized = true;
    
    ESP_LOGI(TAG, "NRF24L01初始化成功");
    return ESP_OK;
}

esp_err_t nrf24l01_deinit(void)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // 下电
    nrf24l01_power_down();
    
    // 移除SPI设备
    spi_bus_remove_device(g_spi_handle);
    
    // 释放SPI总线
    spi_bus_free(g_nrf24l01_config.spi_host);
    
    g_nrf24l01_config.is_initialized = false;
    
    ESP_LOGI(TAG, "NRF24L01已释放");
    return ESP_OK;
}

esp_err_t nrf24l01_set_mode_tx(void)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t config = nrf24l01_read_register(NRF24L01_CONFIG);
    config &= ~NRF24L01_CONFIG_PRIM_RX;
    esp_err_t ret = nrf24l01_write_register(NRF24L01_CONFIG, config);
    
    if (ret == ESP_OK) {
        nrf24l01_ce_low();
        ESP_LOGD(TAG, "设置为发送模式");
    }
    
    return ret;
}

esp_err_t nrf24l01_set_mode_rx(void)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t config = nrf24l01_read_register(NRF24L01_CONFIG);
    config |= NRF24L01_CONFIG_PRIM_RX;
    esp_err_t ret = nrf24l01_write_register(NRF24L01_CONFIG, config);
    
    if (ret == ESP_OK) {
        nrf24l01_ce_high();
        ESP_LOGD(TAG, "设置为接收模式");
    }
    
    return ret;
}

esp_err_t nrf24l01_set_channel(uint8_t channel)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (channel > 125) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nrf24l01_write_register(NRF24L01_RF_CH, channel);
    if (ret == ESP_OK) {
        g_nrf24l01_config.channel = channel;
        ESP_LOGD(TAG, "设置通道: %d", channel);
    }
    
    return ret;
}

esp_err_t nrf24l01_set_data_rate(uint8_t data_rate)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t rf_setup = nrf24l01_read_register(NRF24L01_RF_SETUP);
    rf_setup &= 0xD7; // 清除数据速率位
    rf_setup |= data_rate;
    
    esp_err_t ret = nrf24l01_write_register(NRF24L01_RF_SETUP, rf_setup);
    if (ret == ESP_OK) {
        g_nrf24l01_config.data_rate = data_rate;
        ESP_LOGD(TAG, "设置数据速率: 0x%02X", data_rate);
    }
    
    return ret;
}

esp_err_t nrf24l01_set_power(uint8_t power)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t rf_setup = nrf24l01_read_register(NRF24L01_RF_SETUP);
    rf_setup &= 0xF9; // 清除功率位
    rf_setup |= power;
    
    esp_err_t ret = nrf24l01_write_register(NRF24L01_RF_SETUP, rf_setup);
    if (ret == ESP_OK) {
        g_nrf24l01_config.power = power;
        ESP_LOGD(TAG, "设置发射功率: 0x%02X", power);
    }
    
    return ret;
}

esp_err_t nrf24l01_set_address(uint8_t *address)
{
    if (!g_nrf24l01_config.is_initialized || address == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret;
    
    // 设置接收地址P0
    ret = nrf24l01_write_bytes(NRF24L01_RX_ADDR_P0, address, 5);
    if (ret != ESP_OK) return ret;
    
    // 设置发送地址
    ret = nrf24l01_write_bytes(NRF24L01_TX_ADDR, address, 5);
    if (ret != ESP_OK) return ret;
    
    // 保存地址
    memcpy(g_nrf24l01_config.address, address, 5);
    
    ESP_LOGD(TAG, "设置地址: %02X:%02X:%02X:%02X:%02X", 
             address[0], address[1], address[2], address[3], address[4]);
    
    return ESP_OK;
}

esp_err_t nrf24l01_set_payload_size(uint8_t size)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (size > NRF24L01_MAX_PAYLOAD_SIZE) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = nrf24l01_write_register(NRF24L01_RX_PW_P0, size);
    if (ret == ESP_OK) {
        g_nrf24l01_config.payload_size = size;
        ESP_LOGD(TAG, "设置数据包大小: %d", size);
    }
    
    return ret;
}

esp_err_t nrf24l01_send_packet(uint8_t *data, uint8_t length)
{
    if (!g_nrf24l01_config.is_initialized || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (length > NRF24L01_MAX_PAYLOAD_SIZE) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret;
    
    // 设置为发送模式
    ret = nrf24l01_set_mode_tx();
    if (ret != ESP_OK) return ret;
    
    // 清空发送FIFO
    nrf24l01_flush_tx();
    
    // 写入数据
    nrf24l01_csn_low();
    uint8_t cmd = NRF24L01_CMD_W_TX_PAYLOAD;
    ret = nrf24l01_spi_transfer(&cmd, NULL, 1);
    if (ret == ESP_OK) {
        ret = nrf24l01_spi_transfer(data, NULL, length);
    }
    nrf24l01_csn_high();
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "写入发送数据失败");
        return ret;
    }
    
    // 发送数据
    nrf24l01_ce_high();
    vTaskDelay(pdMS_TO_TICKS(1)); // 至少10us
    nrf24l01_ce_low();
    
    ESP_LOGD(TAG, "发送数据包，长度: %d", length);
    
    return ESP_OK;
}

esp_err_t nrf24l01_receive_packet(nrf24l01_packet_t *packet)
{
    if (!g_nrf24l01_config.is_initialized || packet == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t status = nrf24l01_read_status();
    
    if (!(status & NRF24L01_STATUS_RX_DR)) {
        return ESP_ERR_NOT_FOUND; // 没有数据
    }
    
    // 读取数据
    nrf24l01_csn_low();
    uint8_t cmd = NRF24L01_CMD_R_RX_PAYLOAD;
    esp_err_t ret = nrf24l01_spi_transfer(&cmd, NULL, 1);
    if (ret == ESP_OK) {
        ret = nrf24l01_spi_transfer(NULL, packet->data, g_nrf24l01_config.payload_size);
    }
    nrf24l01_csn_high();
    
    if (ret == ESP_OK) {
        packet->length = g_nrf24l01_config.payload_size;
        packet->pipe = 0; // 简化处理，只使用P0
        
        // 清除接收中断
        nrf24l01_write_register(NRF24L01_STATUS, NRF24L01_STATUS_RX_DR);
        
        ESP_LOGD(TAG, "接收数据包，长度: %d", packet->length);
    }
    
    return ret;
}

esp_err_t nrf24l01_available(void)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t status = nrf24l01_read_status();
    return (status & NRF24L01_STATUS_RX_DR) ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t nrf24l01_flush_rx(void)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    nrf24l01_csn_low();
    uint8_t cmd = NRF24L01_CMD_FLUSH_RX;
    esp_err_t ret = nrf24l01_spi_transfer(&cmd, NULL, 1);
    nrf24l01_csn_high();
    
    return ret;
}

esp_err_t nrf24l01_flush_tx(void)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    nrf24l01_csn_low();
    uint8_t cmd = NRF24L01_CMD_FLUSH_TX;
    esp_err_t ret = nrf24l01_spi_transfer(&cmd, NULL, 1);
    nrf24l01_csn_high();
    
    return ret;
}

uint8_t nrf24l01_read_status(void)
{
    if (!g_nrf24l01_config.is_initialized) {
        return 0;
    }
    
    return nrf24l01_read_register(NRF24L01_STATUS);
}

esp_err_t nrf24l01_write_register(uint8_t reg, uint8_t value)
{
    if (!g_nrf24l01_config.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    nrf24l01_csn_low();
    uint8_t cmd = NRF24L01_CMD_W_REGISTER | reg;
    esp_err_t ret = nrf24l01_spi_transfer(&cmd, NULL, 1);
    if (ret == ESP_OK) {
        ret = nrf24l01_spi_transfer(&value, NULL, 1);
    }
    nrf24l01_csn_high();
    
    return ret;
}

uint8_t nrf24l01_read_register(uint8_t reg)
{
    if (!g_nrf24l01_config.is_initialized) {
        return 0;
    }
    
    uint8_t value = 0;
    
    nrf24l01_csn_low();
    uint8_t cmd = NRF24L01_CMD_R_REGISTER | reg;
    esp_err_t ret = nrf24l01_spi_transfer(&cmd, NULL, 1);
    if (ret == ESP_OK) {
        nrf24l01_spi_transfer(NULL, &value, 1);
    }
    nrf24l01_csn_high();
    
    return value;
}

esp_err_t nrf24l01_write_bytes(uint8_t reg, uint8_t *data, uint8_t length)
{
    if (!g_nrf24l01_config.is_initialized || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nrf24l01_csn_low();
    uint8_t cmd = NRF24L01_CMD_W_REGISTER | reg;
    esp_err_t ret = nrf24l01_spi_transfer(&cmd, NULL, 1);
    if (ret == ESP_OK) {
        ret = nrf24l01_spi_transfer(data, NULL, length);
    }
    nrf24l01_csn_high();
    
    return ret;
}

esp_err_t nrf24l01_read_bytes(uint8_t reg, uint8_t *data, uint8_t length)
{
    if (!g_nrf24l01_config.is_initialized || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nrf24l01_csn_low();
    uint8_t cmd = NRF24L01_CMD_R_REGISTER | reg;
    esp_err_t ret = nrf24l01_spi_transfer(&cmd, NULL, 1);
    if (ret == ESP_OK) {
        ret = nrf24l01_spi_transfer(NULL, data, length);
    }
    nrf24l01_csn_high();
    
    return ret;
}
