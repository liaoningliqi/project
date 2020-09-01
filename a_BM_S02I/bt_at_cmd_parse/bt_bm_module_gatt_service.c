
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>

#include "freeRTOS.h"
#include "timers.h"

#include "att_db.h"
#include "att_db_util.h"
#include "btstack_event.h"

#include "bt_at_cmd.h"
#include "bt_at_cmd_parse.h"
#include "bt_bm_module_gatt_service.h"
#include "bt_bm_module_gatt_callback.h"
#include "..\..\project_common\project_common.h"

#if defined __cplusplus
    extern "C" {
#endif

#define GATT_SERVICE_GENERIC_ATTRIBUTE                  0x1801

#define GATT_SERVICE_HEALTH_THERMOMETER                 0x1809
#define GATT_CHARACTERISTIC_TEMPERATURE_MEASUREMENT     0x2a1c
#define GATT_CHARACTERISTIC_TEMPERATURE_TYPE            0x2a1d

#define TEMPERATURE_TYPE_BODY                           2

#define GATT_SERVICE_BLE_TO_TX_UUID                     0xFFE5
#define GATT_CHARACTERISTIC_BLE_TO_TX_UUID              0xFFE9

#define GATT_SERVICE_RX_TO_BLE_UUID                     0xFFE0
#define GATT_CHARACTERISTIC_RX_TO_BLE_UUID              0xFFE4

#define GATT_SERVICE_PWM_UUID                           0xFFB0
#define GATT_CHARACTERISTIC_PWM_OUT_INIT_UUID           0xFFB1
#define GATT_CHARACTERISTIC_PWM_OUT_WIDTH_UUID          0xFFB2
#define GATT_CHARACTERISTIC_PWM_OUT_FREQ_UUID           0xFFB3
#define GATT_CHARACTERISTIC_PWM_OUT_TRANS_UUID          0xFFB4

#define GATT_SERVICE_ADC_UUID                           0xFFD0
#define GATT_CHARACTERISTIC_ADC_EN_UUID                 0xFFD1
#define GATT_CHARACTERISTIC_ADC_PERIOD_UUID             0xFFD2
#define GATT_CHARACTERISTIC_ADC_VALUE0_UUID             0xFFD3
#define GATT_CHARACTERISTIC_ADC_VALUE1_UUID             0xFFD4

#define GATT_SERVICE_GPIO_UUID                          0xFFF0
#define GATT_CHARACTERISTIC_GPIO_CONFIG_UUID            0xFFF1
#define GATT_CHARACTERISTIC_GPIO_OUT_UUID               0xFFF2
#define GATT_CHARACTERISTIC_GPIO_IN_UUID                0xFFF3

#define GATT_CHARACTERISTIC_GPIO6_FLIP_A_UUID           0xFFF4
#define GATT_CHARACTERISTIC_GPIO6_FLIP_B_UUID           0xFFF5
#define GATT_CHARACTERISTIC_GPIO7_FLIP_A_UUID           0xFFF6
#define GATT_CHARACTERISTIC_GPIO7_FLIP_B_UUID           0xFFF7

#define GATT_CHARACTERISTIC_GPIO4_H_TIME_UUID           0xFFF8
#define GATT_CHARACTERISTIC_GPIO5_LH_TIME_UUID          0xFFF9

#define GATT_SERVICE_KEY_UUID                           0xFFC0
#define GATT_CHARACTERISTIC_PSWD_SET_UUID               0xFFC1
#define GATT_CHARACTERISTIC_PSWD_EVENT_UUID             0xFFC2

#define GATT_SERVICE_BATTERY_UUID                       0x180F
#define GATT_CHARACTERISTIC_BATTERY_VALUE_UUID          0x2A19

#define GATT_SERVICE_RSSI_UUID                          0xFFA0
#define GATT_CHARACTERISTIC_RSSI_VALUE_UUID             0xFFA1
#define GATT_CHARACTERISTIC_RSSI_PERIOD_UUID            0xFFA2

#define GATT_SERVICE_MODULE_PARA_UUID                   0xFF90
#define GATT_CHARACTERISTIC_MODULE_NAME_UUID            0xFF91
#define GATT_CHARACTERISTIC_MODULE_CIT_UUID             0xFF92
#define GATT_CHARACTERISTIC_MODULE_BAUD_UUID            0xFF93
#define GATT_CHARACTERISTIC_MODULE_RST_UUID             0xFF94
#define GATT_CHARACTERISTIC_MODULE_ADV_PERIOD_UUID      0xFF95
#define GATT_CHARACTERISTIC_MODULE_PID_UUID             0xFF96
#define GATT_CHARACTERISTIC_MODULE_TX_POWER_UUID        0xFF97
#define GATT_CHARACTERISTIC_MODULE_ADV_DATA_UUID        0xFF98
#define GATT_CHARACTERISTIC_MODULE_REMOT_CNTL_UUID      0xFF99
#define GATT_CHARACTERISTIC_MODULE_SYS_FUNC_UUID        0xFF9A

#define GATT_SERVICE_DEV_INFO_UUID                      0x180A
#define GATT_CHARACTERISTIC_SYS_ID_UUID                 0xFF99
#define GATT_CHARACTERISTIC_SOFT_VER_UUID               0xFF9A

#define GATT_SERVICE_EVT_UUID                           0xFE00
#define GATT_CHARACTERISTIC_EVT_RTC_UUID                0xFE01
#define GATT_CHARACTERISTIC_EVT_READ_HANDLE_UUID        0xFE02
#define GATT_CHARACTERISTIC_EVT_RW_CHL_UUID             0xFE03
#define GATT_CHARACTERISTIC_EVT_PORT_HANDLE_UUID        0xFE04
#define GATT_CHARACTERISTIC_EVT_PORT_RW_CHL_UUID        0xFE05
#define GATT_CHARACTERISTIC_EVT_PORT_CONFIG_UUID        0xFE06

static uint8_t att_db_storage[2048];

#ifdef ING_CHIPS_PRIVATE_SERVICE
uint16_t g_bt_bm_module_att_temp_value_handle = 0;
TimerHandle_t g_health_thermometer_service_timer = 0;
int g_bt_bm_module_health_thermometer_service_notify_enable = 0;
int g_bt_bm_module_health_thermometer_service_indicate_enable = 0;

// Service INGChips RGB Lighting Service: {6a33a526-e004-4793-a084-8a1dc49b84fd}
static uint8_t uuid_ing_rgb_light_service[] = {0x6a, 0x33, 0xa5, 0x26, 0xe0, 0x04, 0x47, 0x93, 0xa0, 0x84, 0x8a, 0x1d, 0xc4, 0x9b, 0x84, 0xfd};
// Characteristic RGB Lighting Control: {1c190e92-37dd-4ac4-8154-0444c69274c2} c2:74:92:c6:44:04:54:81:c4:4a:dd:37:92:0e:19:1c
static uint8_t uuid_ing_rgb_light_control[] = {0x1c, 0x19, 0x0e, 0x92, 0x37, 0xdd, 0x4a, 0xc4, 0x81, 0x54, 0x04, 0x44, 0xc6, 0x92, 0x74, 0xc2};

uint8_t g_rgb_light_control[] = {0x00, 0x00, 0x00};
#endif

uint16_t g_bt_bm_module_uart_rx_to_ble_handle = 0;
uint8_t g_bt_bm_module_uart_rx_to_ble_data[GATT_CHARACTERISTIC_BLE_TO_TX_DATA_BUF_LEN + 1] = {0};
int g_bt_bm_module_uart_rx_to_ble_data_len = 0;

uint16_t g_bt_bm_module_adc_value0_handle = 0;
uint16_t g_bt_bm_module_adc_value1_handle = 0;
TimerHandle_t g_adc0_value_service_timer = 0;
TimerHandle_t g_adc1_value_service_timer = 0;
int g_bt_bm_module_adc0_value_service_notify_enable = 0;
int g_bt_bm_module_adc0_value_service_indicate_enable = 0;
int g_bt_bm_module_adc1_value_service_notify_enable = 0;
int g_bt_bm_module_adc1_value_service_indicate_enable = 0;

uint16_t g_bt_bm_module_gpio_in_handle = 0;
int g_bt_bm_module_gpio_in_service_notify_enable = 0;
int g_bt_bm_module_gpio_in_service_indicate_enable = 0;

static void bt_bm_module_init_gap_service(void)
{
    static char dev_name[] = "BM_DEVICE";
    static uint8_t service_changed[] = {0x00,0x00,0x00,0x00};

    att_db_util_add_service_uuid16(GAP_SERVICE_UUID);
    att_db_util_add_characteristic_uuid16(GAP_DEVICE_NAME_UUID, ATT_PROPERTY_READ, (uint8_t *)dev_name, sizeof(dev_name) - 1);

    att_db_util_add_service_uuid16(GATT_SERVICE_GENERIC_ATTRIBUTE);
    att_db_util_add_characteristic_uuid16(GAP_SERVICE_CHANGED, ATT_PROPERTY_READ, service_changed, sizeof(service_changed));

    return;
}

#ifdef ING_CHIPS_PRIVATE_SERVICE
static void bt_bm_module_init_health_thermometer_service(void)
{
    uint16_t att_client_desc_value_handle = 0;
    static uint8_t temp_value_type = TEMPERATURE_TYPE_BODY;
    uint8_t temperature_value[GATT_CHARACTERISTIC_TEMPERATURE_DATA_LEN] = {0x00, 0x00, 0x00, 0x00, 0xFE};

    att_db_util_add_service_uuid16(GATT_SERVICE_HEALTH_THERMOMETER);

    g_bt_bm_module_att_temp_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_TEMPERATURE_MEASUREMENT,
        ATT_PROPERTY_INDICATE | ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC,
        temperature_value, sizeof(temperature_value));
    att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_TEMPERATURE_TYPE,
        ATT_PROPERTY_READ, &temp_value_type, sizeof(temp_value_type));

    att_client_desc_value_handle = g_bt_bm_module_att_temp_value_handle + 1;
    bt_bm_module_gatt_read_callback_register(g_bt_bm_module_att_temp_value_handle, bt_bm_thermometer_value_callback);
    bt_bm_module_gatt_write_callback_register(att_client_desc_value_handle, bt_bm_thermometer_client_desc_value_callback);

    g_health_thermometer_service_timer = xTimerCreate("health_thermometer_service_timer",
        pdMS_TO_TICKS(2000), BT_PRIVT_TRUE, BT_PRIVT_NULL, bt_bm_module_health_thermometer_service_timer_callback);

    return;
}

static void bt_bm_module_init_ing_rgb_light_service(void)
{
    uint16_t att_rgb_light_control_handle = 0;
    att_db_util_add_service_uuid128(uuid_ing_rgb_light_service);

    att_rgb_light_control_handle = att_db_util_add_characteristic_uuid128(uuid_ing_rgb_light_control,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC | ATT_PROPERTY_UUID128,
        g_rgb_light_control, sizeof(g_rgb_light_control));
    bt_bm_module_gatt_write_callback_register(att_rgb_light_control_handle, bt_bm_rgb_light_control_callback);

    return;
}
#endif

static void bt_bm_module_init_data_transmit_service(void)
{
    uint16_t att_value_handle;
    uint16_t att_client_desc_value_handle = 0;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // 蓝牙数据通道 服务 UUID 0xFFE5
    att_db_util_add_service_uuid16(GATT_SERVICE_BLE_TO_TX_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_BLE_TO_TX_UUID,
        ATT_PROPERTY_DYNAMIC | ATT_PROPERTY_WRITE,
        g_value, GATT_CHARACTERISTIC_BLE_TO_TX_DATA_LEN);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_ble_to_tx_callback);

    // 串口数据通道 服务 UUID 0xFFE0
    att_db_util_add_service_uuid16(GATT_SERVICE_RX_TO_BLE_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_RX_TO_BLE_UUID,
        ATT_PROPERTY_INDICATE | ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    att_client_desc_value_handle = att_value_handle + 1;
    g_bt_bm_module_uart_rx_to_ble_handle = att_value_handle;

    dbg_printf("bt_bm_module_init_data_transmit_service %u \r\n", att_value_handle);
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_rx_to_ble_read_callback);
    bt_bm_module_gatt_write_callback_register(att_client_desc_value_handle, bt_bm_module_rx_to_ble_write_callback);

    return;
}

static void bt_bm_module_init_pwm_service(void)
{
    uint16_t att_value_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // PWM 输出(4 路) 服务 UUID 0xFFB0
    att_db_util_add_service_uuid16(GATT_SERVICE_PWM_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_PWM_OUT_INIT_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_PWM_OUT_INIT_DATA_LEN);
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_pwm_out_init_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_pwm_out_init_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_PWM_OUT_WIDTH_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN);
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_pwm_out_width_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_pwm_out_width_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_PWM_OUT_FREQ_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_PWM_OUT_FREQ_DATA_LEN);
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_pwm_out_freq_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_pwm_out_freq_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_PWM_OUT_TRANS_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_PWM_OUT_TRANS_DATA_LEN);
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_pwm_out_trans_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_pwm_out_trans_write_callback);

    return;
}

static void bt_bm_module_init_adc_service(void)
{
    uint16_t att_value_handle;
    uint16_t att_value_notify_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // ADC 输入(2 路) 服务 UUID 0xFFD0
    att_db_util_add_service_uuid16(GATT_SERVICE_ADC_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_ADC_EN_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_ADC_EN_DATA_LEN);
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_adc_en_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_adc_en_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_ADC_PERIOD_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_ADC_PERIOD_DATA_LEN);
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_adc_period_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_adc_period_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_ADC_VALUE0_UUID,
        ATT_PROPERTY_INDICATE | ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_ADC_VALUE0_DATA_LEN);
    att_value_notify_handle = att_value_handle + 1;
    g_bt_bm_module_adc_value0_handle = att_value_handle;
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_adc_value0_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_notify_handle, bt_bm_module_adc_value0_notify_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_ADC_VALUE1_UUID,
        ATT_PROPERTY_INDICATE | ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_ADC_VALUE1_DATA_LEN);
    att_value_notify_handle = att_value_handle + 1;
    g_bt_bm_module_adc_value1_handle = att_value_handle;
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_adc_value1_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_notify_handle, bt_bm_module_adc_value1_notify_write_callback);

    g_adc0_value_service_timer = xTimerCreate("adc0_value_service_timer",
        pdMS_TO_TICKS(1000), BT_PRIVT_TRUE, BT_PRIVT_NULL, bt_bm_module_adc0_value_service_timer_callback);
    g_adc1_value_service_timer = xTimerCreate("adc1_value_service_timer",
        pdMS_TO_TICKS(1000), BT_PRIVT_TRUE, BT_PRIVT_NULL, bt_bm_module_adc1_value_service_timer_callback);

    return;
}

static void bt_bm_module_init_gpio_service(void)
{
    uint16_t att_value_handle;
    uint16_t att_value_notify_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // 可编程 IO (8 路) 服务 UUID 0xFFF0
    att_db_util_add_service_uuid16(GATT_SERVICE_GPIO_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO_CONFIG_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_gpio_config_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_gpio_config_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO_OUT_UUID,
        ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_gpio_out_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO_IN_UUID,
        ATT_PROPERTY_INDICATE | ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    att_value_notify_handle = att_value_handle + 1;
    g_bt_bm_module_gpio_in_handle = att_value_handle;
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_gpio_in_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_notify_handle, bt_bm_module_gpio_in_notify_write_callback);

    // 定时翻转输出 (2 路) 服务 UUID 0xFFF0
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO6_FLIP_A_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO6_FLIP_B_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO7_FLIP_A_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO7_FLIP_B_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    // 电平脉宽计数 (2 路) 服务 UUID 0xFFF0
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO4_H_TIME_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_GPIO5_LH_TIME_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    return;
}

static void bt_bm_module_init_key_service(void)
{
    uint16_t att_value_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // 防劫持密钥 服务 UUID 0xFFC0
    att_db_util_add_service_uuid16(GATT_SERVICE_KEY_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_PSWD_SET_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_init_key_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_init_key_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_PSWD_EVENT_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    return;
}

static void bt_bm_module_init_battery_service(void)
{
    uint16_t att_value_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // 电池电量报告 服务 UUID 0x180F
    att_db_util_add_service_uuid16(GATT_SERVICE_BATTERY_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_BATTERY_VALUE_UUID,
        ATT_PROPERTY_INDICATE | ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC,
        g_value, GATT_CHARACTERISTIC_BATTERY_VALUE_DATA_LEN);
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_battery_value_read_callback);

    return;
}

static void bt_bm_module_init_rssi_service(void)
{
    uint16_t att_value_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // RSSI 报告 服务 UUID 0xFFA0
    att_db_util_add_service_uuid16(GATT_SERVICE_RSSI_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_RSSI_VALUE_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_rssi_value_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_rssi_value_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_RSSI_PERIOD_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    return;
}

static void bt_bm_module_init_module_para_service(void)
{
    uint16_t att_value_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // 模块参数设置 服务 UUID 0xFF90
    att_db_util_add_service_uuid16(GATT_SERVICE_MODULE_PARA_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_NAME_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_para_name_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_para_name_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_CIT_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_BAUD_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_RST_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_ADV_PERIOD_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_PID_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_TX_POWER_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_ADV_DATA_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_REMOT_CNTL_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_MODULE_SYS_FUNC_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    return;
}

static void bt_bm_module_init_dev_info_service(void)
{
    uint16_t att_value_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // 设备信息 服务 UUID 0x180A
    att_db_util_add_service_uuid16(GATT_SERVICE_DEV_INFO_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_SYS_ID_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_sys_id_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_sys_id_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_SOFT_VER_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));

    return;
}

static void bt_bm_module_init_evt_service(void)
{
    uint16_t att_value_handle;
    uint8_t g_value[GATT_CHARACTERISTIC_MAX_DATA_LEN]={0};

    // 端口定时事件配置 服务 UUID 0xFE00
    att_db_util_add_service_uuid16(GATT_SERVICE_EVT_UUID);
    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_EVT_RTC_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_evt_rtc_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_evt_rtc_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_EVT_READ_HANDLE_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_evt_rchannel_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_evt_rchannel_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_EVT_RW_CHL_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_evt_wchannel_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_evt_wchannel_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_EVT_PORT_HANDLE_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_evt_port_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_evt_port_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_EVT_PORT_RW_CHL_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_evt_port_rwchannel_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_evt_port_rwchannel_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(GATT_CHARACTERISTIC_EVT_PORT_CONFIG_UUID,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC,
        g_value, sizeof(g_value));
    bt_bm_module_gatt_read_callback_register(att_value_handle, bt_bm_module_evt_port_config_read_callback);
    bt_bm_module_gatt_write_callback_register(att_value_handle, bt_bm_module_evt_port_config_write_callback);

    return;
}

static void bt_bm_module_init_private_service(void)
{
    bt_bm_module_init_data_transmit_service();
    bt_bm_module_init_pwm_service();
    bt_bm_module_init_adc_service();
    bt_bm_module_init_gpio_service();
    bt_bm_module_init_key_service();
    bt_bm_module_init_battery_service();
    bt_bm_module_init_rssi_service();
    bt_bm_module_init_module_para_service();
    bt_bm_module_init_dev_info_service();
    bt_bm_module_init_evt_service();

    return;
}

int bt_bm_module_init_service()
{
    att_db_util_init(att_db_storage, sizeof(att_db_storage));

    bt_bm_module_init_gap_service();
    bt_bm_module_init_private_service();

#ifdef ING_CHIPS_PRIVATE_SERVICE
    bt_bm_module_init_health_thermometer_service();
    bt_bm_module_init_ing_rgb_light_service();
#endif

    return BT_PRIVT_OK;
}

#if defined __cplusplus
    }
#endif

