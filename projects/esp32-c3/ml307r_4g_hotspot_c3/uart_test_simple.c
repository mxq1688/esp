/*
 * 简单的ESP32-C3 UART测试程序
 * 测试GPIO4/5的UART通信
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define UART_NUM        UART_NUM_1
#define TXD_PIN         4
#define RXD_PIN         5
#define UART_BAUD_RATE  115200
#define BUF_SIZE        1024

static const char *TAG = "UART_TEST";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C3 UART测试开始");
    
    // 配置UART
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // 安装UART驱动
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    ESP_LOGI(TAG, "UART1配置完成: GPIO%d(TX) -> GPIO%d(RX)", TXD_PIN, RXD_PIN);
    
    // 测试循环
    char test_str[] = "AT\r\n";
    uint8_t data[BUF_SIZE];
    
    while (1) {
        // 发送AT命令
        ESP_LOGI(TAG, "发送: %s", test_str);
        uart_write_bytes(UART_NUM, test_str, strlen(test_str));
        
        // 等待并读取响应
        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, 2000 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = 0;
            ESP_LOGI(TAG, "收到响应: %s", (char*)data);
        } else {
            ESP_LOGW(TAG, "无响应");
        }
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
