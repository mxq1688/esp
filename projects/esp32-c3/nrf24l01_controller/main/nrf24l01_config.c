#include "nrf24l01_config.h"
#include "esp_log.h"

static const char *TAG = "NRF24L01_CONFIG";

esp_err_t nrf24l01_get_default_config(nrf24l01_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 设置默认引脚配置
    config->spi_host = SPI2_HOST;
    config->ce_pin = NRF24L01_CE_PIN;
    config->csn_pin = NRF24L01_CSN_PIN;
    config->miso_pin = NRF24L01_MISO_PIN;
    config->mosi_pin = NRF24L01_MOSI_PIN;
    config->sck_pin = NRF24L01_SCK_PIN;
    
    // 设置默认参数
    config->channel = NRF24L01_DEFAULT_CHANNEL;
    config->data_rate = NRF24L01_DEFAULT_DATA_RATE;
    config->power = NRF24L01_DEFAULT_POWER;
    config->payload_size = NRF24L01_DEFAULT_PAYLOAD_SIZE;
    
    // 设置默认地址
    uint8_t default_addr[5] = NRF24L01_DEFAULT_ADDRESS;
    memcpy(config->address, default_addr, 5);
    
    config->is_initialized = false;
    
    return ESP_OK;
}

esp_err_t nrf24l01_get_default_app_config(nrf24l01_app_config_t *app_config)
{
    if (app_config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 设置默认应用配置
    app_config->mode = NRF24L01_MODE_SENDER;
    app_config->channel = NRF24L01_DEFAULT_CHANNEL;
    app_config->data_rate = NRF24L01_DEFAULT_DATA_RATE;
    app_config->power = NRF24L01_DEFAULT_POWER;
    app_config->payload_size = NRF24L01_DEFAULT_PAYLOAD_SIZE;
    app_config->send_interval_ms = 1000; // 1秒发送间隔
    app_config->auto_ack = true;
    
    // 设置默认地址
    uint8_t default_addr[5] = NRF24L01_DEFAULT_ADDRESS;
    memcpy(app_config->address, default_addr, 5);
    
    return ESP_OK;
}

void nrf24l01_print_config(nrf24l01_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置为空");
        return;
    }
    
    ESP_LOGI(TAG, "=== NRF24L01 硬件配置 ===");
    ESP_LOGI(TAG, "SPI主机: %d", config->spi_host);
    ESP_LOGI(TAG, "CE引脚: GPIO%d", config->ce_pin);
    ESP_LOGI(TAG, "CSN引脚: GPIO%d", config->csn_pin);
    ESP_LOGI(TAG, "MISO引脚: GPIO%d", config->miso_pin);
    ESP_LOGI(TAG, "MOSI引脚: GPIO%d", config->mosi_pin);
    ESP_LOGI(TAG, "SCK引脚: GPIO%d", config->sck_pin);
    ESP_LOGI(TAG, "通道: %d", config->channel);
    ESP_LOGI(TAG, "数据速率: 0x%02X", config->data_rate);
    ESP_LOGI(TAG, "发射功率: 0x%02X", config->power);
    ESP_LOGI(TAG, "数据包大小: %d", config->payload_size);
    ESP_LOGI(TAG, "地址: %02X:%02X:%02X:%02X:%02X", 
             config->address[0], config->address[1], config->address[2], 
             config->address[3], config->address[4]);
    ESP_LOGI(TAG, "已初始化: %s", config->is_initialized ? "是" : "否");
}

void nrf24l01_print_app_config(nrf24l01_app_config_t *app_config)
{
    if (app_config == NULL) {
        ESP_LOGE(TAG, "应用配置为空");
        return;
    }
    
    const char *mode_str[] = {"发送模式", "接收模式", "双向模式"};
    
    ESP_LOGI(TAG, "=== NRF24L01 应用配置 ===");
    ESP_LOGI(TAG, "工作模式: %s", mode_str[app_config->mode]);
    ESP_LOGI(TAG, "通道: %d", app_config->channel);
    ESP_LOGI(TAG, "数据速率: 0x%02X", app_config->data_rate);
    ESP_LOGI(TAG, "发射功率: 0x%02X", app_config->power);
    ESP_LOGI(TAG, "数据包大小: %d", app_config->payload_size);
    ESP_LOGI(TAG, "地址: %02X:%02X:%02X:%02X:%02X", 
             app_config->address[0], app_config->address[1], app_config->address[2], 
             app_config->address[3], app_config->address[4]);
    ESP_LOGI(TAG, "发送间隔: %d ms", app_config->send_interval_ms);
    ESP_LOGI(TAG, "自动应答: %s", app_config->auto_ack ? "启用" : "禁用");
}
