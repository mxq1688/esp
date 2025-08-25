#include "bluetooth_controller.h"
#include "esp_log.h"
#include "esp_bt_defs.h"
#include "esp_gatt_common_api.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "BLUETOOTH_CTRL";

// 全局变量
bluetooth_controller_t bluetooth_ctrl = {0};

// 回调函数指针
static void (*angle_callback)(uint16_t angle) = NULL;
static void (*led_callback)(bool state) = NULL;

// 服务定义
static const uint16_t primary_service_uuid = SERVO_SERVICE_UUID;
static const uint16_t primary_char_uuid = SERVO_CHAR_UUID;

// 广播数据
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(primary_service_uuid),
    .p_service_uuid = (uint8_t *)&primary_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// 广播参数
static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// 特征属性
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;

// 服务定义
static const esp_gatts_attr_db_t servo_gatt_db[3] = {
    // Service Declaration
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, 0, 0}},

    // Characteristic Declaration
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_char_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), 0}},

    // Characteristic Value
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_char_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, 20, 0}},
};

// GAP事件处理
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(TAG, "Advertising data set complete");
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "Advertising start failed");
        } else {
            ESP_LOGI(TAG, "Advertising start successfully");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        ESP_LOGI(TAG, "Advertising stop complete");
        break;
    default:
        break;
    }
}

// GATTS事件处理
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(TAG, "GATTS register event, status = %d", param->reg.status);
        if (param->reg.status == ESP_GATT_OK) {
            bluetooth_ctrl.service_handle = param->reg.service_handle;
            esp_ble_gap_set_device_name(BLUETOOTH_DEVICE_NAME);
            esp_ble_gap_config_adv_data(&adv_data);
            esp_ble_gatts_create_service(gatts_if, &servo_gatt_db[0], 3);
        }
        break;
    case ESP_GATTS_READ_EVT:
        ESP_LOGI(TAG, "GATTS read event, conn_id = %d", param->read.conn_id);
        break;
    case ESP_GATTS_WRITE_EVT:
        ESP_LOGI(TAG, "GATTS write event, conn_id = %d, len = %d", param->write.conn_id, param->write.len);
        if (param->write.len >= 2) {
            uint8_t cmd = param->write.value[0];
            uint8_t value = param->write.value[1];
            
            switch (cmd) {
            case SERVO_CMD_SET_ANGLE:
                if (angle_callback) {
                    angle_callback(value * 180 / 255); // 将0-255映射到0-180度
                }
                ESP_LOGI(TAG, "Set servo angle: %d", value * 180 / 255);
                break;
            case SERVO_CMD_SET_LED:
                if (led_callback) {
                    led_callback(value > 0);
                }
                ESP_LOGI(TAG, "Set LED state: %s", value > 0 ? "ON" : "OFF");
                break;
            case SERVO_CMD_GET_STATUS:
                // 发送状态通知
                uint8_t status_data[3] = {SERVO_CMD_GET_STATUS, 0, 0};
                bluetooth_send_notification(status_data, 3);
                break;
            default:
                ESP_LOGW(TAG, "Unknown command: %d", cmd);
                break;
            }
        }
        break;
    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(TAG, "GATTS connect event, conn_id = %d", param->connect.conn_id);
        bluetooth_ctrl.conn_id = param->connect.conn_id;
        bluetooth_ctrl.connected = true;
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "GATTS disconnect event, conn_id = %d", param->disconnect.conn_id);
        bluetooth_ctrl.connected = false;
        bluetooth_ctrl.notify_enabled = false;
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(TAG, "GATTS MTU event, mtu = %d", param->mtu.mtu);
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(TAG, "GATTS create service event, status = %d", param->create.status);
        if (param->create.status == ESP_GATT_OK) {
            bluetooth_ctrl.char_handle = param->create.attr_handle;
            esp_ble_gatts_start_service(bluetooth_ctrl.service_handle);
        }
        break;
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(TAG, "GATTS conf event, status = %d", param->conf.status);
        break;
    default:
        break;
    }
}

// 初始化蓝牙
esp_err_t init_bluetooth(void)
{
    esp_err_t ret;

    // 初始化NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化蓝牙控制器
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "Initialize controller failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "Enable controller failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "Init bluedroid failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "Enable bluedroid failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "GATTS register callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "GAP register callback failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_ble_gatts_app_register(0);
    if (ret) {
        ESP_LOGE(TAG, "GATTS app register failed: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret) {
        ESP_LOGE(TAG, "Set local MTU failed: %s", esp_err_to_name(local_mtu_ret));
    }

    ESP_LOGI(TAG, "Bluetooth initialized successfully");
    return ESP_OK;
}

// 反初始化蓝牙
esp_err_t deinit_bluetooth(void)
{
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    return ESP_OK;
}

// 发送通知
esp_err_t bluetooth_send_notification(uint8_t *data, size_t len)
{
    if (!bluetooth_ctrl.connected || !bluetooth_ctrl.notify_enabled) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_ble_gatts_send_indicate(bluetooth_ctrl.conn_id, bluetooth_ctrl.char_handle, len, data, false);
    return ESP_OK;
}

// 设置角度回调
void bluetooth_set_angle_callback(void (*callback)(uint16_t angle))
{
    angle_callback = callback;
}

// 设置LED回调
void bluetooth_set_led_callback(void (*callback)(bool state))
{
    led_callback = callback;
}
