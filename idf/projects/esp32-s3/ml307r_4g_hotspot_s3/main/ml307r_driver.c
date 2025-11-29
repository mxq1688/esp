#include "ml307r_driver.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "ML307R";

// 全局变量
static bool ml307r_initialized = false;
static ml307r_state_t ml307r_current_state = ML307R_STATE_UNKNOWN;
static SemaphoreHandle_t uart_mutex = NULL;

// 私有函数声明
static esp_err_t ml307r_uart_init(void);
static esp_err_t ml307r_gpio_init(void);
static esp_err_t ml307r_wait_response(char *response, size_t response_size, uint32_t timeout_ms);
static bool ml307r_check_response_ok(const char *response);

esp_err_t ml307r_init(void)
{
    if (ml307r_initialized) {
        ESP_LOGW(TAG, "ML307R already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing ML307R module...");

    // 创建UART互斥锁
    uart_mutex = xSemaphoreCreateMutex();
    if (uart_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create UART mutex");
        return ESP_ERR_NO_MEM;
    }

    // 初始化GPIO (仅在有有效引脚时)
    if (ML307R_POWER_PIN >= 0 || ML307R_RESET_PIN >= 0) {
        esp_err_t ret = ml307r_gpio_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize GPIO: %s", esp_err_to_name(ret));
            return ret;
        }
    }

    // 初始化UART
    esp_err_t ret = ml307r_uart_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize UART: %s", esp_err_to_name(ret));
        return ret;
    }

    // 上电复位ML307R (仅在有控制引脚时)
    if (ML307R_POWER_PIN >= 0 || ML307R_RESET_PIN >= 0) {
        ESP_LOGI(TAG, "Powering on ML307R...");
        if (ML307R_POWER_PIN >= 0) {
            gpio_set_level(ML307R_POWER_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        if (ML307R_RESET_PIN >= 0) {
            gpio_set_level(ML307R_RESET_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(ML307R_RESET_PIN, 1);
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // 等待模块启动
    } else {
        ESP_LOGI(TAG, "No power/reset control pins, assuming ML307R is already powered");
        ESP_LOGI(TAG, "Waiting for ML307R module to start...");
        vTaskDelay(pdMS_TO_TICKS(ML307R_STARTUP_DELAY_MS)); // 给模块足够启动时间
    }

    ml307r_current_state = ML307R_STATE_INIT;

    // 清空UART缓冲区
    uart_flush_input(ML307R_UART_NUM);
    vTaskDelay(pdMS_TO_TICKS(100));

    // 测试AT命令 - 使用更标准的方法
    char response[ML307R_RESPONSE_BUF_SIZE];
    ESP_LOGI(TAG, "Testing ML307R communication with AT commands...");
    
    // 根据串口工具测试，ML307R当前是自动波特率检测模式(IPR=0)
    // 先尝试设置固定波特率，然后测试通信
    const uint32_t test_baud_rates[] = {115200, 9600, 19200, 38400, 57600, 230400};
    bool found_correct_baud = false;
    
    for (int b = 0; b < 6; b++) {
        ESP_LOGI(TAG, "Testing baud rate: %lu", test_baud_rates[b]);
        
        // 重新配置UART波特率
        uart_set_baudrate(ML307R_UART_NUM, test_baud_rates[b]);
        vTaskDelay(pdMS_TO_TICKS(100)); // 等待波特率设置生效
        
        // 清空缓冲区
        uart_flush_input(ML307R_UART_NUM);
        
        // 发送AT命令测试
        ret = ml307r_send_at_command("AT", response, sizeof(response), 2000);
        if (ret == ESP_OK && ml307r_check_response_ok(response)) {
            ESP_LOGI(TAG, "✅ Found correct baud rate: %lu, response: %s", test_baud_rates[b], response);
            
            // 如果找到正确的波特率，尝试设置ML307R为固定波特率
            ESP_LOGI(TAG, "Setting ML307R to fixed baud rate %lu...", test_baud_rates[b]);
            char baud_cmd[32];
            snprintf(baud_cmd, sizeof(baud_cmd), "AT+IPR=%lu", test_baud_rates[b]);
            ret = ml307r_send_at_command(baud_cmd, response, sizeof(response), 3000);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "✅ ML307R set to fixed baud rate %lu: %s", test_baud_rates[b], response);
            } else {
                ESP_LOGW(TAG, "❌ Failed to set fixed baud rate %lu: %s", test_baud_rates[b], esp_err_to_name(ret));
            }
            
            found_correct_baud = true;
            break;
        } else {
            ESP_LOGW(TAG, "❌ Baud rate %lu failed: %s, response: %s", test_baud_rates[b], esp_err_to_name(ret), response);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    if (!found_correct_baud) {
        ESP_LOGE(TAG, "❌ No correct baud rate found, using default 115200");
        uart_set_baudrate(ML307R_UART_NUM, 115200);
    }
    
    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "AT test attempt %d/10", i + 1);
        
        // 发送简单的AT命令
        ESP_LOGI(TAG, "About to call ml307r_send_at_command with 'AT'");
        ret = ml307r_send_at_command("AT", response, sizeof(response), ML307R_AT_TIMEOUT_MS);
        ESP_LOGI(TAG, "ml307r_send_at_command returned: %s", esp_err_to_name(ret));
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Received response: %s", response);
            if (ml307r_check_response_ok(response)) {
                ESP_LOGI(TAG, "ML307R module is ready");
                ml307r_current_state = ML307R_STATE_READY;
                ml307r_initialized = true;
                return ESP_OK;
            }
        } else {
            ESP_LOGW(TAG, "AT command failed: %s", esp_err_to_name(ret));
        }
        
        // 尝试不同的AT命令
        if (i == 3) {
            ESP_LOGI(TAG, "Trying ATE0 command (disable echo)...");
            ml307r_send_at_command("ATE0", response, sizeof(response), ML307R_AT_TIMEOUT_MS);
        } else if (i == 5) {
            ESP_LOGI(TAG, "Trying ATI command (product info)...");
            ml307r_send_at_command("ATI", response, sizeof(response), ML307R_AT_TIMEOUT_MS);
        } else if (i == 7) {
            ESP_LOGI(TAG, "Trying AT+CGMI command (manufacturer info)...");
            ml307r_send_at_command("AT+CGMI", response, sizeof(response), ML307R_AT_TIMEOUT_MS);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000)); // 增加重试间隔
    }

    ESP_LOGE(TAG, "ML307R initialization failed");
    ml307r_current_state = ML307R_STATE_ERROR;
    return ESP_ERR_TIMEOUT;
}

esp_err_t ml307r_deinit(void)
{
    if (!ml307r_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing ML307R module...");

    // 断电
    gpio_set_level(ML307R_POWER_PIN, 0);
    
    // 删除UART驱动
    uart_driver_delete(ML307R_UART_NUM);
    
    // 删除互斥锁
    if (uart_mutex != NULL) {
        vSemaphoreDelete(uart_mutex);
        uart_mutex = NULL;
    }

    ml307r_initialized = false;
    ml307r_current_state = ML307R_STATE_UNKNOWN;

    ESP_LOGI(TAG, "ML307R deinitialized");
    return ESP_OK;
}

esp_err_t ml307r_send_at_command(const char *command, char *response, size_t response_size, uint32_t timeout_ms)
{
    ESP_LOGI(TAG, "ml307r_send_at_command called with command: %s", command ? command : "NULL");
    
    if (command == NULL || response == NULL) {
        ESP_LOGE(TAG, "Invalid arguments: command=%p, response=%p", command, response);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Taking UART mutex...");
    if (xSemaphoreTake(uart_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take UART mutex");
        return ESP_ERR_TIMEOUT;
    }
    ESP_LOGI(TAG, "UART mutex taken successfully");

    esp_err_t ret = ESP_OK;

    // 清空接收缓冲区
    uart_flush_input(ML307R_UART_NUM);

    // 发送AT命令 - 根据串口工具配置，命令以\r\n结束
    char cmd_with_crlf[256];
    snprintf(cmd_with_crlf, sizeof(cmd_with_crlf), "%s\r\n", command);
    
    int len = uart_write_bytes(ML307R_UART_NUM, cmd_with_crlf, strlen(cmd_with_crlf));
    if (len < 0) {
        ESP_LOGE(TAG, "Failed to send AT command: %s", command);
        ret = ESP_FAIL;
        goto exit;
    }

    ESP_LOGI(TAG, "Sent AT command: %s (length: %d)", command, (int)strlen(cmd_with_crlf));
    ESP_LOGI(TAG, "Raw command bytes: ");
    for (int i = 0; i < strlen(cmd_with_crlf); i++) {
        ESP_LOGI(TAG, "  [%d] = 0x%02X ('%c')", i, (unsigned char)cmd_with_crlf[i], 
                 (cmd_with_crlf[i] >= 32 && cmd_with_crlf[i] <= 126) ? cmd_with_crlf[i] : '.');
    }

    // 等待响应
    ret = ml307r_wait_response(response, response_size, timeout_ms);
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Received response: %s", response);
    } else {
        ESP_LOGW(TAG, "No response for command: %s", command);
    }

exit:
    xSemaphoreGive(uart_mutex);
    return ret;
}

bool ml307r_is_ready(void)
{
    return ml307r_initialized && (ml307r_current_state == ML307R_STATE_READY || 
                                  ml307r_current_state == ML307R_STATE_CONNECTED);
}

esp_err_t ml307r_get_network_info(ml307r_network_info_t *info)
{
    if (!ml307r_is_ready() || info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char response[ML307R_RESPONSE_BUF_SIZE];
    esp_err_t ret;

    // 清空信息结构体
    memset(info, 0, sizeof(ml307r_network_info_t));

    // 获取运营商信息
    ret = ml307r_send_at_command("AT+COPS?", response, sizeof(response), 5000);
    if (ret == ESP_OK) {
        // 解析运营商名称 (简化版本)
        char *start = strstr(response, "\"");
        if (start) {
            start++;
            char *end = strstr(start, "\"");
            if (end) {
                size_t len = end - start;
                if (len < sizeof(info->operator_name)) {
                    strncpy(info->operator_name, start, len);
                    info->operator_name[len] = '\0';
                }
            }
        }
    }

    // 获取信号强度
    ret = ml307r_send_at_command("AT+CSQ", response, sizeof(response), 3000);
    if (ret == ESP_OK) {
        int rssi;
        if (sscanf(response, "+CSQ: %d", &rssi) == 1) {
            // 转换为dBm
            if (rssi >= 0 && rssi <= 31) {
                info->signal_strength = -113 + rssi * 2;
            } else {
                info->signal_strength = -113; // 未知或无信号
            }
        }
    }

    // 获取网络注册状态
    ret = ml307r_send_at_command("AT+CREG?", response, sizeof(response), 3000);
    if (ret == ESP_OK) {
        int status;
        if (sscanf(response, "+CREG: %*d,%d", &status) == 1) {
            info->is_connected = (status == 1 || status == 5); // 1=本地注册, 5=漫游注册
        }
    }

    // 获取IP地址 (如果已连接)
    if (info->is_connected) {
        ret = ml307r_send_at_command("AT+CGPADDR=1", response, sizeof(response), 5000);
        if (ret == ESP_OK) {
            char *ip_start = strstr(response, "\"");
            if (ip_start) {
                ip_start++;
                char *ip_end = strstr(ip_start, "\"");
                if (ip_end) {
                    size_t len = ip_end - ip_start;
                    if (len < sizeof(info->ip_address)) {
                        strncpy(info->ip_address, ip_start, len);
                        info->ip_address[len] = '\0';
                    }
                }
            }
        }
    }

    // 设置网络类型 (简化为4G)
    strcpy(info->network_type, "4G");

    return ESP_OK;
}

esp_err_t ml307r_enable_hotspot(const ml307r_hotspot_config_t *config)
{
    if (!ml307r_is_ready() || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char command[256];
    char response[ML307R_RESPONSE_BUF_SIZE];
    esp_err_t ret;

    ESP_LOGI(TAG, "Enabling 4G hotspot: %s", config->ssid);

    // 配置WiFi热点参数
    snprintf(command, sizeof(command), "AT+WIFIAP=\"%s\",\"%s\",%d", 
             config->ssid, config->password, config->max_connections);
    
    ret = ml307r_send_at_command(command, response, sizeof(response), 10000);
    if (ret != ESP_OK || !ml307r_check_response_ok(response)) {
        ESP_LOGE(TAG, "Failed to configure WiFi AP");
        return ESP_FAIL;
    }

    // 启用WiFi热点
    ret = ml307r_send_at_command("AT+WIFIAPEN=1", response, sizeof(response), 10000);
    if (ret != ESP_OK || !ml307r_check_response_ok(response)) {
        ESP_LOGE(TAG, "Failed to enable WiFi AP");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "4G hotspot enabled successfully");
    return ESP_OK;
}

esp_err_t ml307r_disable_hotspot(void)
{
    if (!ml307r_is_ready()) {
        return ESP_ERR_INVALID_ARG;
    }

    char response[ML307R_RESPONSE_BUF_SIZE];
    esp_err_t ret = ml307r_send_at_command("AT+WIFIAPEN=0", response, sizeof(response), 5000);
    
    if (ret == ESP_OK && ml307r_check_response_ok(response)) {
        ESP_LOGI(TAG, "4G hotspot disabled");
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to disable 4G hotspot");
    return ESP_FAIL;
}

esp_err_t ml307r_get_hotspot_status(ml307r_hotspot_config_t *config)
{
    if (!ml307r_is_ready() || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char response[ML307R_RESPONSE_BUF_SIZE];
    esp_err_t ret = ml307r_send_at_command("AT+WIFIAPEN?", response, sizeof(response), 3000);
    
    if (ret == ESP_OK) {
        int enabled;
        if (sscanf(response, "+WIFIAPEN: %d", &enabled) == 1) {
            config->is_enabled = (enabled == 1);
        }
    }

    return ret;
}

esp_err_t ml307r_reset(void)
{
    ESP_LOGI(TAG, "Resetting ML307R module...");
    
    gpio_set_level(ML307R_RESET_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(ML307R_RESET_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ml307r_current_state = ML307R_STATE_INIT;
    
    // 重新测试连接
    char response[ML307R_RESPONSE_BUF_SIZE];
    esp_err_t ret = ml307r_send_at_command("AT", response, sizeof(response), 3000);
    
    if (ret == ESP_OK && ml307r_check_response_ok(response)) {
        ml307r_current_state = ML307R_STATE_READY;
        ESP_LOGI(TAG, "ML307R reset successfully");
        return ESP_OK;
    }

    ml307r_current_state = ML307R_STATE_ERROR;
    ESP_LOGE(TAG, "ML307R reset failed");
    return ESP_FAIL;
}

ml307r_state_t ml307r_get_state(void)
{
    return ml307r_current_state;
}

int ml307r_get_signal_strength(void)
{
    if (!ml307r_is_ready()) {
        return -999;
    }

    char response[ML307R_RESPONSE_BUF_SIZE];
    esp_err_t ret = ml307r_send_at_command("AT+CSQ", response, sizeof(response), 3000);
    
    if (ret == ESP_OK) {
        int rssi;
        if (sscanf(response, "+CSQ: %d", &rssi) == 1) {
            if (rssi >= 0 && rssi <= 31) {
                return -113 + rssi * 2; // 转换为dBm
            }
        }
    }

    return -113; // 无信号
}

// 建立4G数据连接
esp_err_t ml307r_establish_data_connection(void)
{
    // 即使模块状态不是READY也尝试建立连接
    ESP_LOGI(TAG, "ML307R current state: %d", ml307r_current_state);

    char response[ML307R_RESPONSE_BUF_SIZE];
    esp_err_t ret;

    ESP_LOGI(TAG, "Establishing 4G data connection...");

    // 1. 检查网络注册状态
    ret = ml307r_send_at_command("AT+CREG?", response, sizeof(response), 5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to check network registration: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Network registration status: %s", response);

    // 2. 检查GPRS附着状态
    ret = ml307r_send_at_command("AT+CGATT?", response, sizeof(response), 5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to check GPRS attachment: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "GPRS attachment status: %s", response);

    // 3. 设置PDP上下文
    ret = ml307r_send_at_command("AT+CGDCONT=1,\"IP\",\"cmnet\"", response, sizeof(response), 5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set PDP context: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "PDP context set: %s", response);

    // 4. 激活PDP上下文
    ret = ml307r_send_at_command("AT+CGACT=1,1", response, sizeof(response), 10000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to activate PDP context: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "PDP context activated: %s", response);

    // 5. 获取IP地址
    ret = ml307r_send_at_command("AT+CGPADDR=1", response, sizeof(response), 5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get IP address: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "IP address: %s", response);

    ml307r_current_state = ML307R_STATE_CONNECTED;
    ESP_LOGI(TAG, "✅ 4G data connection established successfully");
    
    return ESP_OK;
}

// 断开4G数据连接
esp_err_t ml307r_disconnect_data_connection(void)
{
    if (!ml307r_is_ready()) {
        return ESP_ERR_INVALID_STATE;
    }

    char response[ML307R_RESPONSE_BUF_SIZE];
    esp_err_t ret;

    ESP_LOGI(TAG, "Disconnecting 4G data connection...");

    // 去激活PDP上下文
    ret = ml307r_send_at_command("AT+CGACT=0,1", response, sizeof(response), 5000);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to deactivate PDP context: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "PDP context deactivated: %s", response);
    }

    ml307r_current_state = ML307R_STATE_READY;
    ESP_LOGI(TAG, "4G data connection disconnected");
    
    return ESP_OK;
}

// 私有函数实现
static esp_err_t ml307r_uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = ML307R_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t ret = uart_driver_install(ML307R_UART_NUM, ML307R_UART_BUF_SIZE, ML307R_UART_BUF_SIZE, 0, NULL, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = uart_param_config(ML307R_UART_NUM, &uart_config);
    if (ret != ESP_OK) {
        uart_driver_delete(ML307R_UART_NUM);
        return ret;
    }

    ret = uart_set_pin(ML307R_UART_NUM, ML307R_UART_TX_PIN, ML307R_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        uart_driver_delete(ML307R_UART_NUM);
        return ret;
    }

    ESP_LOGI(TAG, "UART initialized: TX=%d, RX=%d, Baud=%d", 
             ML307R_UART_TX_PIN, ML307R_UART_RX_PIN, ML307R_UART_BAUD_RATE);
    
    return ESP_OK;
}

static esp_err_t ml307r_gpio_init(void)
{
    uint64_t pin_mask = 0;
    
    // 只配置有效的引脚
    if (ML307R_POWER_PIN >= 0 && ML307R_POWER_PIN < 64) {
        pin_mask |= (1ULL << (uint32_t)ML307R_POWER_PIN);
    }
    if (ML307R_RESET_PIN >= 0 && ML307R_RESET_PIN < 64) {
        pin_mask |= (1ULL << (uint32_t)ML307R_RESET_PIN);
    }
    
    if (pin_mask == 0) {
        ESP_LOGI(TAG, "No GPIO pins to configure");
        return ESP_OK;
    }

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = pin_mask,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        return ret;
    }

    // 初始状态：断电，复位拉高
    if (ML307R_POWER_PIN >= 0) {
        gpio_set_level(ML307R_POWER_PIN, 0);
    }
    if (ML307R_RESET_PIN >= 0) {
        gpio_set_level(ML307R_RESET_PIN, 1);
    }

    ESP_LOGI(TAG, "GPIO initialized: POWER=%d, RESET=%d", ML307R_POWER_PIN, ML307R_RESET_PIN);
    
    return ESP_OK;
}

static esp_err_t ml307r_wait_response(char *response, size_t response_size, uint32_t timeout_ms)
{
    if (response == NULL || response_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(response, 0, response_size);
    
    TickType_t start_time = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);
    size_t pos = 0;
    bool got_data = false;

    ESP_LOGI(TAG, "Waiting for response (timeout: %lu ms)...", timeout_ms);

    while ((xTaskGetTickCount() - start_time) < timeout_ticks) {
        uint8_t data;
        int len = uart_read_bytes(ML307R_UART_NUM, &data, 1, pdMS_TO_TICKS(50));
        
        if (len > 0) {
            got_data = true;
            ESP_LOGI(TAG, "Received byte: 0x%02X ('%c')", data, 
                     (data >= 32 && data <= 126) ? data : '.');
            if (pos < response_size - 1) {
                // 过滤掉不可打印字符，但保留\r\n
                if (data >= 32 || data == '\r' || data == '\n') {
                    response[pos++] = data;
                    response[pos] = '\0';
                    ESP_LOGI(TAG, "Current response buffer: '%s'", response);
                }
                
                // 检查是否收到完整响应 - 根据ML307R响应格式
                if (strstr(response, "OK") || strstr(response, "ERROR") || 
                    strstr(response, "+CME ERROR") || strstr(response, "+CMS ERROR") ||
                    strstr(response, "+CIS ERROR")) {
                    
                    // 等待一点时间确保完整接收
                    vTaskDelay(pdMS_TO_TICKS(100));
                    
                    // 继续读取剩余数据
                    while (uart_read_bytes(ML307R_UART_NUM, &data, 1, pdMS_TO_TICKS(10)) > 0) {
                        if (pos < response_size - 1 && (data >= 32 || data == '\r' || data == '\n')) {
                            response[pos++] = data;
                            response[pos] = '\0';
                        }
                    }
                    
                    ESP_LOGD(TAG, "Complete response received: %s", response);
                    return ESP_OK;
                }
            }
        } else if (got_data && pos > 0) {
            // 如果已经收到一些数据但没有更多数据，等待一下
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    if (pos > 0) {
        ESP_LOGW(TAG, "Partial response received: %s", response);
        return ESP_OK; // 返回部分响应
    }

    ESP_LOGW(TAG, "No response received within timeout");
    return ESP_ERR_TIMEOUT;
}

static bool ml307r_check_response_ok(const char *response)
{
    if (response == NULL) {
        return false;
    }
    
    return (strstr(response, "OK") != NULL) && 
           (strstr(response, "ERROR") == NULL);
}
