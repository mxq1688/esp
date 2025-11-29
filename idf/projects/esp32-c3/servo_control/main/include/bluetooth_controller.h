#ifndef BLUETOOTH_CONTROLLER_H
#define BLUETOOTH_CONTROLLER_H

#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatt_defs.h"

// 蓝牙服务UUID
#define SERVO_SERVICE_UUID    0x180F  // 使用标准服务UUID作为基础
#define SERVO_CHAR_UUID       0x2A19  // 使用标准特征UUID作为基础

// 蓝牙设备名称
#define BLUETOOTH_DEVICE_NAME "ESP32-Servo"

// 控制命令定义
typedef enum {
    SERVO_CMD_SET_ANGLE = 0x01,    // 设置舵机角度
    SERVO_CMD_SET_LED = 0x02,      // 设置LED状态
    SERVO_CMD_GET_STATUS = 0x03,   // 获取状态
} servo_ble_cmd_t;

// 蓝牙控制器结构体
typedef struct {
    uint16_t conn_id;
    uint16_t service_handle;
    uint16_t char_handle;
    bool connected;
    bool notify_enabled;
} bluetooth_controller_t;

// 函数声明
esp_err_t init_bluetooth(void);
esp_err_t deinit_bluetooth(void);
esp_err_t bluetooth_send_notification(uint8_t *data, size_t len);
void bluetooth_set_angle_callback(void (*callback)(uint16_t angle));
void bluetooth_set_led_callback(void (*callback)(bool state));

// 外部变量
extern bluetooth_controller_t bluetooth_ctrl;

#endif // BLUETOOTH_CONTROLLER_H
