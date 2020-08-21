
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "bluetooth.h"
#include "att_db.h"
#include "btstack_event.h"

#include "bt_at_cmd.h"
#include "bt_at_cmd_parse.h"
#include "bt_at_cmd_sdk_adp.h"
#include "bt_bm_module_gatt_callback.h"
#include "..\..\project_common\project_common.h"

#if defined __cplusplus
    extern "C" {
#endif

#define BT_BM_MODULE_GATT_HANDLE_MAX_NUM 100
#define BT_BM_MODULE_GATT_BLE_TO_TX_MAX_LEN 20

#define BT_BM_MODULE_EVT_PORT_ACTION_NONE 0
#define BT_BM_MODULE_EVT_PORT_ACTION_OUT_L 1
#define BT_BM_MODULE_EVT_PORT_ACTION_OUT_H 2
#define BT_BM_MODULE_EVT_PORT_ACTION_OUT_FLIP 3
#define BT_BM_MODULE_EVT_PORT_ACTION_PWM_CHG 4
#define BT_BM_MODULE_EVT_PORT_ACTION_PWM_SLOW_CHG 5

typedef union {
    uint16_t whole;
    struct {
        uint8_t byte0; // bit0
        uint8_t byte1; // bit0
    }bytes;
    struct {
        uint16_t all_en:1; // bit0
        uint16_t io_0_en:1; // bit1
        uint16_t io_1_en:1; // bit2
        uint16_t io_2_en:1; // bit3
        uint16_t io_3_en:1; // bit4
        uint16_t io_4_en:1; // bit5
        uint16_t io_5_en:1; // bit6
        uint16_t pwm_0_en:1; // bit7

        uint16_t pwm_1_en:1; // bit8
        uint16_t pwm_2_en:1; // bit9
        uint16_t pwm_3_en:1; // bit10
        uint16_t event_clr:1; // bit11
        uint16_t task_clr:1; // bit12
        uint16_t rsv:3;
    }bits;
} bt_bm_module_evt_port_config_t;

typedef union {
    uint32_t whole;
    struct {
        uint8_t byte0;
        uint8_t byte1;
        uint8_t byte2;
        uint8_t byte3;
    }bytes;
    struct {
        uint32_t event_0_en:1; // bit0
        uint32_t event_1_en:1; // bit1
        uint32_t event_2_en:1; // bit2
        uint32_t event_3_en:1; // bit3
        uint32_t event_4_en:1; // bit4
        uint32_t event_5_en:1; // bit5
        uint32_t event_6_en:1; // bit6
        uint32_t event_7_en:1; // bit7

        uint32_t event_8_en:1; // bit8
        uint32_t event_9_en:1; // bit9
        uint32_t event_10_en:1; // bit10
        uint32_t event_11_en:1; // bit11
        uint32_t event_12_en:1; // bit12
        uint32_t event_13_en:1; // bit13
        uint32_t event_14_en:1; // bit14
        uint32_t event_15_en:1; // bit15

        uint32_t event_16_en:1; // bit16
        uint32_t event_17_en:1; // bit17
        uint32_t event_18_en:1; // bit18
        uint32_t event_19_en:1; // bit19
        uint32_t event_20_en:1; // bit20
        uint32_t event_21_en:1; // bit21
        uint32_t event_22_en:1; // bit22
        uint32_t event_23_en:1; // bit23

        uint32_t event_24_en:1; // bit24
        uint32_t event_25_en:1; // bit25
        uint32_t event_26_en:1; // bit26
        uint32_t event_27_en:1; // bit27
        uint32_t event_28_en:1; // bit28
        uint32_t event_29_en:1; // bit29
        uint32_t event_30_en:1; // bit30
        uint32_t event_31_en:1; // bit31
    }bits;
} bt_bm_module_evt_single_port_event_en_t;

typedef enum {
    BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_NONE = 0,
    BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_MINUTE,
    BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_HOUR,
    BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_DAY,
    BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_MONTH,
    BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_YEAR,
} bt_bm_module_evt_cycle_trig_mode_e;

typedef struct {
    uint64_t event_trig_in_rtc_second_time;
    struct tm event_trig_time;
    uint16_t event_port_bit_map;
    uint8_t action_mode;
    uint8_t pulse_width;
    uint8_t pwm_change_time_l;
    uint8_t pwm_change_time_h;
    uint8_t is_enable;
    uint8_t is_config;
    uint8_t cycle_trig_mode;
} bt_bm_module_evt_single_event_t;

typedef struct {
    uint32_t event_bit_map;
    bt_bm_module_evt_single_event_t event[GATT_CHARACTERISTIC_EVT_MAX_EVENT_NUM];
    uint32_t port_bit_map;
    uint8_t is_enable;
} bt_bm_module_evt_event_t;

typedef struct {
    bt_bm_module_evt_single_port_event_en_t port[GATT_CHARACTERISTIC_EVT_MAX_PORT_NUM];
} bt_bm_module_evt_port_event_en_t;

static bt_bm_module_att_read_callback_t module_gatt_read_callback[BT_BM_MODULE_GATT_HANDLE_MAX_NUM] = {0};
static int module_gatt_read_callback_num = 0;

static bt_bm_module_att_write_callback_t module_gatt_write_callback[BT_BM_MODULE_GATT_HANDLE_MAX_NUM] = {0};
static int module_gatt_write_callback_num = 0;

hci_con_handle_t g_bt_bm_hci_con_handle_send;

#ifdef ING_CHIPS_PRIVATE_SERVICE
static int g_bt_bm_module_can_send_temperature = BT_PRIVT_DISNABLE;
#endif

int g_bt_bm_module_rx_to_ble_can_send_data = BT_PRIVT_DISNABLE;
static int g_bt_bm_module_can_send_adc0_value = BT_PRIVT_DISNABLE;
static int g_bt_bm_module_can_send_adc1_value = BT_PRIVT_DISNABLE;
static int g_bt_bm_module_can_send_gpio_in = BT_PRIVT_DISNABLE;

static uint8_t bt_bm_module_evt_rchannel_config = 0;
static uint8_t bt_bm_module_evt_port_index_config = 0;

static bt_bm_module_evt_event_t bt_bm_module_evt_event = {0};
static bt_bm_module_evt_port_event_en_t bt_bm_module_evt_port_event_en = {0};
static bt_bm_module_evt_port_config_t bt_bm_module_evt_port_config = {0};

uint8_t g_bt_bm_module_pwm_width[GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN] = {10, 10, 10, 10};
uint8_t g_bt_bm_module_gpio_config[GATT_CHARACTERISTIC_GPIO_CONFIG_DATA_LEN] = {0};
uint8_t g_bt_bm_module_gpio_out[GATT_CHARACTERISTIC_GPIO_OUT_DATA_LEN] = {0};

int g_bt_bm_module_rtc_change_should_re_calculate_event_trig_time = BT_PRIVT_FALSE;

int bt_bm_rgb_light_control_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return bt_at_cmd_sdk_adp_set_rgb_light(buffer, buffer_size);
}

int bt_bm_module_ble_to_tx_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t ble_to_tx_data[BT_BM_MODULE_GATT_BLE_TO_TX_MAX_LEN + 1] = {0};
    int ret;

    if (buffer_size > BT_BM_MODULE_GATT_BLE_TO_TX_MAX_LEN) {
        buffer_size = BT_BM_MODULE_GATT_BLE_TO_TX_MAX_LEN;
    }

    strncpy((char *)ble_to_tx_data, (char *)buffer, buffer_size);
    ble_to_tx_data[buffer_size] = '\0';

    dbg_printf("ble to uart %s\r\n", ble_to_tx_data);
    ret = bt_at_cmd_sdk_adp_set_uart_data(ble_to_tx_data, buffer_size);

    return ret;
}

#ifdef ING_CHIPS_PRIVATE_SERVICE
int bt_bm_thermometer_client_desc_value_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION) {
        g_bt_bm_module_health_thermometer_service_indicate_enable = BT_PRIVT_ENABLE;
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    } else if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION) {
        g_bt_bm_module_health_thermometer_service_notify_enable = BT_PRIVT_ENABLE;
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    }

    return BT_PRIVT_OK;
}

static int32_t bt_bm_module_health_thermometer_service_read_temperature()
{
    int32_t temperature = 0;

    if (bt_at_cmd_sdk_adp_get_temperature(&temperature) != BT_PRIVT_OK) {
        return 0;
    }

    return temperature;
}

static void bt_bm_module_health_thermometer_service_send_temperature()
{
    int32_t temperature;
    uint8_t temperature_value[GATT_CHARACTERISTIC_TEMPERATURE_DATA_LEN] = {0x00,0x00,0x00,0x00,0xFE};

    if (g_bt_bm_module_can_send_temperature != BT_PRIVT_ENABLE) {
        return;
    }

    temperature = bt_bm_module_health_thermometer_service_read_temperature();
    temperature_value[3] = (uint8_t)(temperature>>16);
    temperature_value[2] = (uint8_t)(temperature>>8);
    temperature_value[1] = (uint8_t)temperature;

    if (g_bt_bm_module_health_thermometer_service_notify_enable) {
        att_server_notify(g_bt_bm_hci_con_handle_send, g_bt_bm_module_att_temp_value_handle,
            (uint8_t*)temperature_value, GATT_CHARACTERISTIC_TEMPERATURE_DATA_LEN);
    }

    if (g_bt_bm_module_health_thermometer_service_indicate_enable) {
        att_server_indicate(g_bt_bm_hci_con_handle_send, g_bt_bm_module_att_temp_value_handle,
            (uint8_t*)temperature_value, GATT_CHARACTERISTIC_TEMPERATURE_DATA_LEN);
    }

    g_bt_bm_module_can_send_temperature = BT_PRIVT_DISNABLE;

    return;
}

void bt_bm_module_health_thermometer_service_timer_callback(TimerHandle_t xTimer)
{
    g_bt_bm_module_can_send_temperature = BT_PRIVT_ENABLE;
    if (g_bt_bm_module_health_thermometer_service_notify_enable | g_bt_bm_module_health_thermometer_service_indicate_enable) {
        btstack_push_user_msg(USER_MSG_ID_HEALTH_THERMOMETER_SERVICE, BT_PRIVT_NULL, 0);
    }
    return;
}

int bt_bm_thermometer_value_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int32_t temperature;
    uint8_t temperature_value[GATT_CHARACTERISTIC_TEMPERATURE_DATA_LEN] = {0x00,0x00,0x00,0x00,0xFE};

    temperature = bt_bm_module_health_thermometer_service_read_temperature();
    temperature_value[3] = (uint8_t)(temperature>>16);
    temperature_value[2] = (uint8_t)(temperature>>8);
    temperature_value[1] = (uint8_t)temperature;

    if (buffer) {
        memcpy(buffer, &temperature_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_TEMPERATURE_DATA_LEN;
    }
}
#endif

int bt_bm_module_rx_to_ble_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t ble_value[GATT_CHARACTERISTIC_RX_TO_BLE_DATA_LEN] = {0};

    if (buffer) {
        memcpy(buffer, &ble_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_RX_TO_BLE_DATA_LEN;
    }
}

int bt_bm_module_rx_to_ble_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    dbg_printf("bt_bm_module_rx_to_ble_write_callback handle %u\r\n", g_bt_bm_hci_con_handle_send);
    if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION) {
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    } else if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION) {
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    }

    return BT_PRIVT_OK;
}

static void bt_bm_module_uart_rx_to_ble_service_send_data()
{
    uint16_t have_send_len = 0;
    uint16_t this_time_send_len;
    uint16_t mtu_len = att_server_get_mtu(g_bt_bm_hci_con_handle_send) - 3;

    if (g_bt_bm_module_rx_to_ble_can_send_data != BT_PRIVT_ENABLE) {
        return;
    }

    while (g_bt_bm_module_uart_rx_to_ble_data_len > 0) {
        if (g_bt_bm_module_uart_rx_to_ble_data_len <= mtu_len) {
            this_time_send_len = g_bt_bm_module_uart_rx_to_ble_data_len;
        } else {
            this_time_send_len = mtu_len;
        }
        g_bt_bm_module_uart_rx_to_ble_data_len = g_bt_bm_module_uart_rx_to_ble_data_len - this_time_send_len;

        while (!att_server_can_send_packet_now(g_bt_bm_hci_con_handle_send)) {
        }

        att_server_notify(g_bt_bm_hci_con_handle_send, g_bt_bm_module_uart_rx_to_ble_handle,
            (uint8_t*)(&(g_bt_bm_module_uart_rx_to_ble_data[have_send_len])), this_time_send_len);
        have_send_len = have_send_len + this_time_send_len;
    }

    dbg_printf((char *)g_bt_bm_module_uart_rx_to_ble_data);

    g_bt_bm_module_rx_to_ble_can_send_data = BT_PRIVT_DISNABLE;

    return;
}

static uint16_t bt_bm_module_adc_value_service_read_adc(int adc)
{
    uint16_t value = 0;

    if (bt_at_cmd_sdk_adp_get_adc_value(adc, &value) != BT_PRIVT_OK) {
        return 0;
    }

    return value;
}

int bt_bm_module_battery_value_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t value;
    uint8_t battery_value[GATT_CHARACTERISTIC_BATTERY_VALUE_DATA_LEN] = {1,2,3,4,5};

    value = bt_bm_module_adc_value_service_read_adc(BATTERY_ADC_CHANNEL);
    battery_value[1] = (uint8_t)(value>>8);
    battery_value[0] = (uint8_t)value;

    if (buffer) {
        memcpy(buffer, &battery_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_BATTERY_VALUE_DATA_LEN;
    }
}

int bt_bm_module_pwm_out_init_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t pwm_value[GATT_CHARACTERISTIC_PWM_OUT_INIT_DATA_LEN] = {0};

    if (buffer) {
        memcpy(buffer, &pwm_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_BATTERY_VALUE_DATA_LEN;
    }
}

int bt_bm_module_pwm_out_init_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_pwm_out_width_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t pwm_value[GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN] = {0};

    if (buffer) {
        memcpy(buffer, &pwm_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN;
    }
}

int bt_bm_module_pwm_out_width_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer_size != GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    if (bt_at_cmd_sdk_adp_set_pwm_width(buffer, GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN) != BT_PRIVT_OK) {
        return BT_PRIVT_OK;
    }

    return BT_PRIVT_OK;
}

int bt_bm_module_pwm_out_freq_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t pwm_value[GATT_CHARACTERISTIC_PWM_OUT_FREQ_DATA_LEN] = {0};

    if (buffer) {
        memcpy(buffer, &pwm_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_PWM_OUT_FREQ_DATA_LEN;
    }
}

int bt_bm_module_pwm_out_freq_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_pwm_out_trans_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t pwm_value[GATT_CHARACTERISTIC_PWM_OUT_TRANS_DATA_LEN] = {0};

    if (buffer) {
        memcpy(buffer, &pwm_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_PWM_OUT_TRANS_DATA_LEN;
    }
}

int bt_bm_module_pwm_out_trans_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_adc_en_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t adc_value[GATT_CHARACTERISTIC_ADC_EN_DATA_LEN] = {0};

    if (buffer) {
        memcpy(buffer, &adc_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_ADC_EN_DATA_LEN;
    }
}

int bt_bm_module_adc_en_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer_size != GATT_CHARACTERISTIC_ADC_EN_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    switch (*buffer) {
        case 0:
            g_bt_bm_module_adc0_value_service_notify_enable = BT_PRIVT_DISNABLE;
            g_bt_bm_module_adc1_value_service_notify_enable = BT_PRIVT_DISNABLE;
            g_bt_bm_module_adc0_value_service_indicate_enable = BT_PRIVT_DISNABLE;
            g_bt_bm_module_adc1_value_service_indicate_enable = BT_PRIVT_DISNABLE;
            xTimerStop(g_adc0_value_service_timer,  portMAX_DELAY);
            xTimerStop(g_adc1_value_service_timer,  portMAX_DELAY);
            break;
        case 1:
            g_bt_bm_module_adc0_value_service_notify_enable = BT_PRIVT_ENABLE;
            g_bt_bm_module_adc0_value_service_indicate_enable = BT_PRIVT_ENABLE;
            xTimerStart(g_adc0_value_service_timer,  portMAX_DELAY);
            break;
        case 2:
            g_bt_bm_module_adc1_value_service_notify_enable = BT_PRIVT_ENABLE;
            g_bt_bm_module_adc1_value_service_indicate_enable = BT_PRIVT_ENABLE;
            xTimerStart(g_adc1_value_service_timer,  portMAX_DELAY);
            break;
        case 3:
            g_bt_bm_module_adc0_value_service_notify_enable = BT_PRIVT_ENABLE;
            g_bt_bm_module_adc1_value_service_notify_enable = BT_PRIVT_ENABLE;
            g_bt_bm_module_adc0_value_service_indicate_enable = BT_PRIVT_ENABLE;
            g_bt_bm_module_adc1_value_service_indicate_enable = BT_PRIVT_ENABLE;
            xTimerStart(g_adc0_value_service_timer,  portMAX_DELAY);
            xTimerStart(g_adc1_value_service_timer,  portMAX_DELAY);
            break;
        default:
            break;
    }

    return BT_PRIVT_OK;
}

int bt_bm_module_adc_period_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t adc_value[GATT_CHARACTERISTIC_ADC_PERIOD_DATA_LEN] = {0};

    static uint8_t temp = 0;
    adc_value[0] = 1;
    adc_value[1] = 1 + temp;
    temp++;

    if (buffer) {
        memcpy(buffer, &adc_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_ADC_PERIOD_DATA_LEN;
    }
}

int bt_bm_module_adc_period_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_adc_value0_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t value;
    uint8_t adc0_value[GATT_CHARACTERISTIC_ADC_VALUE0_DATA_LEN] = {0};

    value = bt_bm_module_adc_value_service_read_adc(BT_BM_SDK_ADC0_CHANNEL);
    adc0_value[1] = (uint8_t)(value>>8);
    adc0_value[0] = (uint8_t)value;

    if (buffer) {
        memcpy(buffer, &adc0_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_ADC_VALUE0_DATA_LEN;
    }
}

int bt_bm_module_adc_value0_notify_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION) {
        g_bt_bm_module_adc0_value_service_indicate_enable = BT_PRIVT_ENABLE;
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    } else if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION) {
        g_bt_bm_module_adc0_value_service_notify_enable = BT_PRIVT_ENABLE;
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    }

    return BT_PRIVT_OK;
}

int bt_bm_module_adc_value1_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t value;
    uint8_t adc1_value[GATT_CHARACTERISTIC_ADC_VALUE1_DATA_LEN] = {0};

    value = bt_bm_module_adc_value_service_read_adc(BT_BM_SDK_ADC1_CHANNEL);
    adc1_value[1] = (uint8_t)(value>>8);
    adc1_value[0] = (uint8_t)value;

    if (buffer) {
        memcpy(buffer, &adc1_value[offset], buffer_size);
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_ADC_VALUE1_DATA_LEN;
    }
}

int bt_bm_module_adc_value1_notify_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION) {
        g_bt_bm_module_adc1_value_service_indicate_enable = BT_PRIVT_ENABLE;
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    } else if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION) {
        g_bt_bm_module_adc1_value_service_notify_enable = BT_PRIVT_ENABLE;
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    }

    return BT_PRIVT_OK;
}

void bt_bm_module_adc0_value_service_timer_callback(TimerHandle_t xTimer)
{
    g_bt_bm_module_can_send_adc0_value = BT_PRIVT_ENABLE;
    if (g_bt_bm_module_adc0_value_service_notify_enable | g_bt_bm_module_adc0_value_service_indicate_enable) {
        btstack_push_user_msg(USER_MSG_ID_ADC_VALUE_SERVICE, BT_PRIVT_NULL, 0);
    }
    return;
}

void bt_bm_module_adc1_value_service_timer_callback(TimerHandle_t xTimer)
{
    g_bt_bm_module_can_send_adc1_value = BT_PRIVT_ENABLE;
    if (g_bt_bm_module_adc1_value_service_notify_enable | g_bt_bm_module_adc1_value_service_indicate_enable) {
        btstack_push_user_msg(USER_MSG_ID_ADC_VALUE_SERVICE, BT_PRIVT_NULL, 0);
    }
    return;
}

static void bt_bm_module_adc_value_service_send_data()
{
    uint16_t value;
    uint8_t adc0_value[GATT_CHARACTERISTIC_ADC_VALUE0_DATA_LEN] = {0};
    uint8_t adc1_value[GATT_CHARACTERISTIC_ADC_VALUE1_DATA_LEN] = {0};

    if (g_bt_bm_module_can_send_adc0_value == BT_PRIVT_ENABLE) {
        value = bt_bm_module_adc_value_service_read_adc(BT_BM_SDK_ADC0_CHANNEL);
        adc0_value[0] = (uint8_t)(value>>8);
        adc0_value[1] = (uint8_t)value;

        dbg_printf("bt_bm_module_adc_value_service_send_data0 %u \r\n", value);
        if (g_bt_bm_module_adc0_value_service_notify_enable) {
            att_server_notify(g_bt_bm_hci_con_handle_send, g_bt_bm_module_adc_value0_handle,
                adc0_value, sizeof(adc0_value));
        }
        if (g_bt_bm_module_adc0_value_service_indicate_enable) {
            att_server_indicate(g_bt_bm_hci_con_handle_send, g_bt_bm_module_adc_value0_handle,
                adc0_value, sizeof(adc0_value));
        }
        g_bt_bm_module_can_send_adc0_value = BT_PRIVT_DISNABLE;
    }

    if (g_bt_bm_module_can_send_adc1_value == BT_PRIVT_ENABLE) {
        value = bt_bm_module_adc_value_service_read_adc(BT_BM_SDK_ADC1_CHANNEL);
        adc1_value[0] = (uint8_t)(value>>8);
        adc1_value[1] = (uint8_t)value;

        dbg_printf("bt_bm_module_adc_value_service_send_data1 %u \r\n", value);
        if (g_bt_bm_module_adc1_value_service_notify_enable) {
            att_server_notify(g_bt_bm_hci_con_handle_send, g_bt_bm_module_adc_value1_handle,
                adc1_value, sizeof(adc1_value));
        }
        if (g_bt_bm_module_adc1_value_service_indicate_enable) {
            att_server_indicate(g_bt_bm_hci_con_handle_send, g_bt_bm_module_adc_value1_handle,
                adc1_value, sizeof(adc1_value));
        }
        g_bt_bm_module_can_send_adc1_value = BT_PRIVT_DISNABLE;
    }

    return;
}

static uint8_t bt_bm_module_gpio_service_read_config()
{
    uint8_t value = 0;

    if (bt_at_cmd_sdk_adp_get_gpio_config(&value) != BT_PRIVT_OK) {
        return 0;
    }

    return value;
}

int bt_bm_module_gpio_config_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t value;

    value = bt_bm_module_gpio_service_read_config();

    if (buffer) {
        *buffer = value;
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_GPIO_CONFIG_DATA_LEN;
    }
}

int bt_bm_module_gpio_config_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t gpio_config;
    if (buffer_size != GATT_CHARACTERISTIC_GPIO_CONFIG_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    gpio_config = buffer[0];
    if (bt_at_cmd_sdk_adp_set_gpio_config(&gpio_config) != BT_PRIVT_OK) {
        return BT_PRIVT_OK;
    }

    if (gpio_config == 0xFF) {
        g_bt_bm_module_gpio_in_service_notify_enable = BT_PRIVT_DISNABLE;
        g_bt_bm_module_gpio_in_service_indicate_enable = BT_PRIVT_DISNABLE;
    } else {
        g_bt_bm_module_gpio_in_service_notify_enable = BT_PRIVT_ENABLE;
        g_bt_bm_module_gpio_in_service_indicate_enable = BT_PRIVT_ENABLE;
    }

    return BT_PRIVT_OK;
}

int bt_bm_module_gpio_out_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t gpio_write;
    if (buffer_size != GATT_CHARACTERISTIC_GPIO_OUT_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    gpio_write = buffer[0];
    if (bt_at_cmd_sdk_adp_set_gpio_value(&gpio_write) != BT_PRIVT_OK) {
        return BT_PRIVT_OK;
    }

    return BT_PRIVT_OK;
}

static uint8_t bt_bm_module_gpio_service_read_in()
{
    uint8_t value = 0;

    if (bt_at_cmd_sdk_adp_get_gpio_value(&value) != BT_PRIVT_OK) {
        return 0;
    }

    return value;
}

int bt_bm_module_gpio_in_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t value;

    value = bt_bm_module_gpio_service_read_in();

    if (buffer) {
        *buffer = value;
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_GPIO_IN_DATA_LEN;
    }
}

int bt_bm_module_gpio_in_notify_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION) {
        g_bt_bm_module_gpio_in_service_indicate_enable = BT_PRIVT_ENABLE;
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    } else if (*(uint16_t *)buffer == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION) {
        g_bt_bm_module_gpio_in_service_notify_enable = BT_PRIVT_ENABLE;
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
    }

    bt_bm_module_gpio_in_service_send_msg();
    dbg_printf("btstack push gpio msg %u \r\n", *(uint16_t *)buffer);

    return BT_PRIVT_OK;
}

void bt_bm_module_gpio_in_service_send_msg(void)
{
    g_bt_bm_module_can_send_gpio_in = BT_PRIVT_ENABLE;
    if (g_bt_bm_module_gpio_in_service_notify_enable | g_bt_bm_module_gpio_in_service_indicate_enable) {
        btstack_push_user_msg(USER_MSG_ID_GPIO_IN_SERVICE, BT_PRIVT_NULL, 0);
    }
    return;
}

static void bt_bm_module_gpio_value_service_send_data()
{
    uint8_t value;

    if (g_bt_bm_module_can_send_gpio_in != BT_PRIVT_ENABLE) {
        return;
    }

    value = bt_bm_module_gpio_service_read_in();
    if (g_bt_bm_module_gpio_in_service_notify_enable) {
        att_server_notify(g_bt_bm_hci_con_handle_send, g_bt_bm_module_gpio_in_handle,
            &value, GATT_CHARACTERISTIC_GPIO_IN_DATA_LEN);
    }
    if (g_bt_bm_module_gpio_in_service_indicate_enable) {
        att_server_indicate(g_bt_bm_hci_con_handle_send, g_bt_bm_module_gpio_in_handle,
            &value, GATT_CHARACTERISTIC_GPIO_IN_DATA_LEN);
    }
    g_bt_bm_module_can_send_gpio_in = BT_PRIVT_DISNABLE;

    return;
}

int bt_bm_module_para_name_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int ret;
    char module_name[GATT_CHARACTERISTIC_MODULE_PARA_NAME_LEN + 1] = {0};

    ret = bt_at_cmd_sdk_adp_get_module_name(module_name, GATT_CHARACTERISTIC_MODULE_PARA_NAME_LEN);
    module_name[GATT_CHARACTERISTIC_MODULE_PARA_NAME_LEN] = '\0';

    dbg_printf(module_name);

    if (buffer) {
        if (ret == BT_PRIVT_OK) {
            strncpy((char *)buffer, module_name, buffer_size);
        }
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_MODULE_PARA_NAME_LEN;
    }
}

int bt_bm_module_para_name_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_evt_rtc_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int ret;
    uint8_t rtc_time[GATT_CHARACTERISTIC_EVT_RTC_LEN] = {0};
    struct tm time = {0};

    ret = bt_at_cmd_sdk_adp_get_rtc_time(&time);

    if (buffer) {
        if (ret == BT_PRIVT_OK) {
            rtc_time[6] = ((time.tm_year + STRUCT_TM_YEAR_BASE_LINE) >> 8) & 0xff;
            rtc_time[5] = (time.tm_year + STRUCT_TM_YEAR_BASE_LINE) & 0xff;
            rtc_time[4] = time.tm_mon + STRUCT_TM_MONTH_BASE_LINE;
            rtc_time[3] = time.tm_mday;
            rtc_time[2] = time.tm_hour;
            rtc_time[1] = time.tm_min;
            rtc_time[0] = time.tm_sec;
            memcpy(buffer, rtc_time, buffer_size);
        }
        return buffer_size;
    } else {
        dbg_printf("bt_bm_module_evt_rtc_read_callback get size\r\n");
        return GATT_CHARACTERISTIC_EVT_RTC_LEN;
    }
}

int bt_bm_module_evt_rtc_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    struct tm time = {0};
    uint16_t year = 0;

    if (buffer_size != GATT_CHARACTERISTIC_EVT_RTC_LEN) {
        return BT_PRIVT_OK;
    }

    year = (buffer[6] << 8) | buffer[5];

    time.tm_year = year - STRUCT_TM_YEAR_BASE_LINE;
    time.tm_mon = buffer[4] - STRUCT_TM_MONTH_BASE_LINE;
    time.tm_mday = buffer[3];
    time.tm_hour = buffer[2];
    time.tm_min = buffer[1];
    time.tm_sec = buffer[0];

    return bt_at_cmd_sdk_adp_set_rtc_time(&time);
}

int bt_bm_module_evt_rchannel_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer) {
        buffer[0] = bt_bm_module_evt_rchannel_config;
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_EVT_RCHANNEL_DATA_LEN;
    }
}

int bt_bm_module_evt_rchannel_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer_size != GATT_CHARACTERISTIC_EVT_RCHANNEL_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    bt_bm_module_evt_rchannel_config = buffer[0];

    return BT_PRIVT_OK;
}

int bt_bm_module_evt_wchannel_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer) {
        buffer[0] = bt_bm_module_evt_rchannel_config;
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_EVT_WCHANNEL_DATA_LEN;
    }
}

int bt_bm_module_get_month_days(int year, int month)
{
    int day = 31;

    if((month == 4) || (month == 6) || (month == 9) || (month == 11) ){
        day = 30;
    }

    if (month == 2) {
        if (((year % 4 == 0) && (year % 100 != 0)) || year % 400 == 0) {
            day = 29;
        } else {
            day = 28;
        }
    }

    return day;
}

int bt_bm_module_get_year_days(int year, int month)
{
    int day = 365;
    int next_year = year + 1;

    if (month <= 2) {
        if (((year % 4 == 0) && (year % 100 != 0)) || year % 400 == 0) {
            day = 366;
        }
    } else {
        if (((next_year % 4 == 0) && (next_year % 100 != 0)) || next_year % 400 == 0) {
            day = 366;
        }
    }

    return day;
}

static void bt_bm_module_evt_upgrate_event_trig_time_in_second(uint8_t event_index)
{
    int ret;
    int day;
    int i;
    struct tm time = {0};
    uint32_t bit_map_mask = 1 << event_index;

    switch (bt_bm_module_evt_event.event[event_index].cycle_trig_mode) {
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_NONE:
            bt_bm_module_evt_event.event[event_index].is_enable = BT_PRIVT_DISNABLE;
            bt_bm_module_evt_event.event_bit_map = bt_bm_module_evt_event.event_bit_map & (~bit_map_mask);
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_MINUTE:
            bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time += 60;
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_HOUR:
            bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time += (60 * 60);
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_DAY:
            bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time += (24 * 60 * 60);
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_MONTH:
            ret = bt_at_cmd_sdk_adp_get_rtc_time(&time);
            if (ret != BT_PRIVT_OK) {
                bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time += (30 * 24 * 60 * 60);
                break;
            }
            for (i = time.tm_mon; i < 12; i++) {
                day = bt_bm_module_get_month_days(time.tm_year + STRUCT_TM_YEAR_BASE_LINE, i + STRUCT_TM_MONTH_BASE_LINE);
                bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time += (day * 24 * 60 * 60);
                if (day >= bt_bm_module_evt_event.event[event_index].event_trig_time.tm_mday) {
                    break;
                }
            }
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_YEAR:
            ret = bt_at_cmd_sdk_adp_get_rtc_time(&time);
            if (ret != BT_PRIVT_OK) {
                bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time += (365 * 24 * 60 * 60);
                break;
            }

            if (bt_bm_module_evt_event.event[event_index].event_trig_time.tm_mon == 1 && bt_bm_module_evt_event.event[event_index].event_trig_time.tm_mday == 29) {
            }

            day = bt_bm_module_get_year_days(time.tm_year + STRUCT_TM_YEAR_BASE_LINE, time.tm_mon + STRUCT_TM_MONTH_BASE_LINE);
            bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time += (day * 24 * 60 * 60);
            break;
        default:
            bt_bm_module_evt_event.event[event_index].is_enable = BT_PRIVT_DISNABLE;
            break;
    }

    if (bt_bm_module_evt_event.event_bit_map == 0) {
        bt_bm_module_evt_event.is_enable = BT_PRIVT_DISNABLE;
    }

    return;
}

static int bt_bm_module_evt_calculate_event_trig_time_in_second(uint8_t event_index)
{
    int month = bt_bm_module_evt_event.event[event_index].event_trig_time.tm_mon;
    int day = bt_bm_module_evt_event.event[event_index].event_trig_time.tm_mday;
    int hour = bt_bm_module_evt_event.event[event_index].event_trig_time.tm_hour;
    int minute = bt_bm_module_evt_event.event[event_index].event_trig_time.tm_min;
    int second = bt_bm_module_evt_event.event[event_index].event_trig_time.tm_sec;
    struct tm *p_now_time = NULL;
    uint64_t now_time_second = 0;
    uint64_t set_time_second = 0;
    int set_second;
    int now_second;
    uint32_t bit_map_mask;
    int ret;
    int i;

    if (bt_bm_module_evt_event.event[event_index].is_config == BT_PRIVT_TRUE) {
        return BT_PRIVT_OK;
    }

    ret = bt_at_cmd_sdk_adp_get_rtc_time_second(&now_time_second);
    if (ret != BT_PRIVT_OK) {
        return BT_PRIVT_ERROR;
    }
    p_now_time = localtime((time_t *)(&now_time_second));

    bit_map_mask = 1 << event_index;

    switch (bt_bm_module_evt_event.event[event_index].cycle_trig_mode) {
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_MINUTE:
            set_second = second;
            now_second = p_now_time->tm_sec;
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_HOUR:
            set_second = minute*60 + second;
            now_second = p_now_time->tm_min*60 + p_now_time->tm_sec;
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_DAY:
            set_second = hour*60*60 + minute*60 + second;
            now_second = p_now_time->tm_hour*60*60 + p_now_time->tm_min*60 + p_now_time->tm_sec;
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_MONTH:
            set_second = day*24*60*60 + hour*60*60 + minute*60 + second;
            now_second = p_now_time->tm_mday*24*60*60 + p_now_time->tm_hour*60*60 + p_now_time->tm_min*60 + p_now_time->tm_sec;
            break;
        case BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_YEAR:
            set_second = day*24*60*60 + hour*60*60 + minute*60 + second;
            for (i = 0; i < month; i++) {
                set_second += bt_bm_module_get_month_days(p_now_time->tm_year + STRUCT_TM_YEAR_BASE_LINE, i + STRUCT_TM_MONTH_BASE_LINE)*24*60*60;
            }
            now_second = p_now_time->tm_mday*24*60*60 + p_now_time->tm_hour*60*60 + p_now_time->tm_min*60 + p_now_time->tm_sec;
            for (i = 0; i < p_now_time->tm_mon; i++) {
                now_second += bt_bm_module_get_month_days(p_now_time->tm_year + STRUCT_TM_YEAR_BASE_LINE, i + STRUCT_TM_MONTH_BASE_LINE)*24*60*60;
            }
            break;
        default:
            set_time_second = mktime(&(bt_bm_module_evt_event.event[event_index].event_trig_time));
            if (set_time_second < now_time_second) {
            } else {
                bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time = set_time_second;
                bt_bm_module_evt_event.event[event_index].is_enable = BT_PRIVT_ENABLE;
                bt_bm_module_evt_event.event_bit_map = bt_bm_module_evt_event.event_bit_map | bit_map_mask;
                bt_bm_module_evt_event.is_enable = BT_PRIVT_ENABLE;
            }
            return BT_PRIVT_OK;
    }

    bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time = now_time_second + set_second - now_second;
    if (set_second < now_second) {
        bt_bm_module_evt_upgrate_event_trig_time_in_second(event_index);
    }

    bt_bm_module_evt_event.event[event_index].is_enable = BT_PRIVT_ENABLE;
    bt_bm_module_evt_event.event_bit_map = bt_bm_module_evt_event.event_bit_map | bit_map_mask;
    bt_bm_module_evt_event.is_enable = BT_PRIVT_ENABLE;

    return BT_PRIVT_OK;
}

int bt_bm_module_evt_wchannel_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    uint8_t event_index;
    int year = 0;
    int ret;

    if (buffer_size != GATT_CHARACTERISTIC_EVT_WCHANNEL_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    event_index = buffer[0];
    if (event_index >= GATT_CHARACTERISTIC_EVT_MAX_EVENT_NUM) {
        event_index = GATT_CHARACTERISTIC_EVT_MAX_EVENT_NUM - 1;
    }

    year = (buffer[7] << 8) | buffer[6];
    year -= STRUCT_TM_YEAR_BASE_LINE;
    bt_bm_module_evt_event.event[event_index].event_trig_time.tm_year = year;
    bt_bm_module_evt_event.event[event_index].event_trig_time.tm_mon = buffer[5] - STRUCT_TM_MONTH_BASE_LINE;
    bt_bm_module_evt_event.event[event_index].event_trig_time.tm_mday = buffer[4];
    bt_bm_module_evt_event.event[event_index].event_trig_time.tm_hour = buffer[3];
    bt_bm_module_evt_event.event[event_index].event_trig_time.tm_min = buffer[2];
    bt_bm_module_evt_event.event[event_index].event_trig_time.tm_sec = buffer[1];

    bt_bm_module_evt_event.event[event_index].action_mode = buffer[8];
    bt_bm_module_evt_event.event[event_index].pulse_width = buffer[9];
    bt_bm_module_evt_event.event[event_index].pwm_change_time_l = buffer[10];
    bt_bm_module_evt_event.event[event_index].pwm_change_time_h = buffer[11];

    if (buffer[2] == GATT_CHARACTERISTIC_EVT_INVALID_TIME_BYTE) {
        bt_bm_module_evt_event.event[event_index].cycle_trig_mode = BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_MINUTE;
    } else if (buffer[3] == GATT_CHARACTERISTIC_EVT_INVALID_TIME_BYTE) {
        bt_bm_module_evt_event.event[event_index].cycle_trig_mode = BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_HOUR;
    } else if (buffer[4] == GATT_CHARACTERISTIC_EVT_INVALID_TIME_BYTE) {
        bt_bm_module_evt_event.event[event_index].cycle_trig_mode = BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_DAY;
    } else if (buffer[5] == GATT_CHARACTERISTIC_EVT_INVALID_TIME_BYTE) {
        bt_bm_module_evt_event.event[event_index].cycle_trig_mode = BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_MONTH;
    } else if ((buffer[6] == GATT_CHARACTERISTIC_EVT_INVALID_TIME_BYTE) && (buffer[7] == GATT_CHARACTERISTIC_EVT_INVALID_TIME_BYTE)) {
        bt_bm_module_evt_event.event[event_index].cycle_trig_mode = BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_IN_YEAR;
    } else {
        bt_bm_module_evt_event.event[event_index].cycle_trig_mode = BT_BM_MODULE_EVT_CYCLE_TRIG_MODE_NONE;
    }

    bt_bm_module_evt_event.event[event_index].is_config = BT_PRIVT_TRUE;

    ret = bt_bm_module_evt_calculate_event_trig_time_in_second(event_index);
    if (ret != BT_PRIVT_OK) {
        return BT_PRIVT_OK;
    }

    return BT_PRIVT_OK;
}

int bt_bm_module_evt_port_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer) {
        buffer[0] = bt_bm_module_evt_port_index_config;
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_EVT_PORT_INDEX_DATA_LEN;
    }
}

int bt_bm_module_evt_port_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer_size != GATT_CHARACTERISTIC_EVT_PORT_INDEX_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    bt_bm_module_evt_port_index_config = buffer[0];

    if (bt_bm_module_evt_port_index_config >= GATT_CHARACTERISTIC_EVT_MAX_PORT_NUM) {
        bt_bm_module_evt_port_index_config = GATT_CHARACTERISTIC_EVT_MAX_PORT_NUM - 1;
    }

    return BT_PRIVT_OK;
}

int bt_bm_module_evt_port_rwchannel_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer) {
        buffer[0] = bt_bm_module_evt_port_index_config;
        buffer[1] = bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].bytes.byte0;
        buffer[2] = bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].bytes.byte1;
        buffer[3] = bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].bytes.byte2;
        buffer[4] = bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].bytes.byte3;
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_EVT_PORT_RWCHANNEL_DATA_LEN;
    }
}

int bt_bm_module_evt_port_rwchannel_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t port_bit_map;
    uint32_t port_event_en_bit_map;
    int event_index;

    if (buffer_size != GATT_CHARACTERISTIC_EVT_PORT_INDEX_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    bt_bm_module_evt_port_index_config = buffer[0];

    if (bt_bm_module_evt_port_index_config >= GATT_CHARACTERISTIC_EVT_MAX_PORT_NUM) {
        bt_bm_module_evt_port_index_config = GATT_CHARACTERISTIC_EVT_MAX_PORT_NUM - 1;
    }


    bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].bytes.byte0 = buffer[1];
    bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].bytes.byte1 = buffer[2];
    bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].bytes.byte2 = buffer[3];
    bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].bytes.byte3 = buffer[4];

    port_event_en_bit_map = bt_bm_module_evt_port_event_en.port[bt_bm_module_evt_port_index_config].whole;
    port_bit_map = 1 << bt_bm_module_evt_port_index_config;

    if (port_event_en_bit_map == 0) {
        bt_bm_module_evt_event.port_bit_map = bt_bm_module_evt_event.port_bit_map & (~port_bit_map);
    } else {
        bt_bm_module_evt_event.port_bit_map = bt_bm_module_evt_event.port_bit_map | port_bit_map;
    }

    for (event_index = 0; event_index < GATT_CHARACTERISTIC_EVT_MAX_EVENT_NUM; event_index++) {
        if ((port_event_en_bit_map & (1 << event_index)) == 0) {
            bt_bm_module_evt_event.event[event_index].event_port_bit_map = bt_bm_module_evt_event.event[event_index].event_port_bit_map & (~port_bit_map);
        } else {
            bt_bm_module_evt_event.event[event_index].event_port_bit_map = bt_bm_module_evt_event.event[event_index].event_port_bit_map | port_bit_map;
        }
    }

    return BT_PRIVT_OK;
}

int bt_bm_module_evt_port_config_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer) {
        buffer[0] = bt_bm_module_evt_port_config.bytes.byte0;
        buffer[1] = bt_bm_module_evt_port_config.bytes.byte1;
        return buffer_size;
    } else {
        return GATT_CHARACTERISTIC_EVT_PORT_CONFIG_DATA_LEN;
    }
}

int bt_bm_module_evt_port_config_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer_size != GATT_CHARACTERISTIC_EVT_PORT_CONFIG_DATA_LEN) {
        return BT_PRIVT_OK;
    }

    bt_bm_module_evt_port_config.bytes.byte0 = buffer[0];
    bt_bm_module_evt_port_config.bytes.byte1 = buffer[1];

    return BT_PRIVT_OK;
}

static void bt_bm_module_evt_port_action(int event_index, int port_index)
{
    uint8_t gpio_write = g_bt_bm_module_gpio_out[0];
    uint16_t pwm_width;

    pwm_width = bt_bm_module_evt_event.event[event_index].pulse_width;
    pwm_width = (pwm_width * 100) / 255;

    switch (bt_bm_module_evt_event.event[event_index].action_mode) {
        case BT_BM_MODULE_EVT_PORT_ACTION_NONE:
            break;
        case BT_BM_MODULE_EVT_PORT_ACTION_OUT_L:
            if (port_index > 5) {
                return;
            }
            if ((g_bt_bm_module_gpio_config[0] & (1 << port_index)) == 0) {
                return;
            }
            gpio_write = gpio_write & (~(1 << port_index));
            if (bt_at_cmd_sdk_adp_set_gpio_value(&gpio_write) != BT_PRIVT_OK) {
                return;
            }
            break;
        case BT_BM_MODULE_EVT_PORT_ACTION_OUT_H:
            if (port_index > 5) {
                return;
            }
            if ((g_bt_bm_module_gpio_config[0] & (1 << port_index)) == 0) {
                return;
            }
            gpio_write = gpio_write | (1 << port_index);
            if (bt_at_cmd_sdk_adp_set_gpio_value(&gpio_write) != BT_PRIVT_OK) {
                return;
            }
            break;
        case BT_BM_MODULE_EVT_PORT_ACTION_OUT_FLIP:
            if (port_index > 5) {
                break;
            }
            if ((g_bt_bm_module_gpio_config[0] & (1 << port_index)) == 0) {
                break;
            }
            gpio_write = gpio_write ^ (1 << port_index);
            if (bt_at_cmd_sdk_adp_set_gpio_value(&gpio_write) != BT_PRIVT_OK) {
                return;
            }
            break;
        case BT_BM_MODULE_EVT_PORT_ACTION_PWM_CHG:
            if (port_index <= 5) {
                break;
            }
            g_bt_bm_module_pwm_width[port_index - 6] = pwm_width & 0xFF;
            if (bt_at_cmd_sdk_adp_set_pwm_width(g_bt_bm_module_pwm_width, GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN) != BT_PRIVT_OK) {
                return;
            }
            break;
        case BT_BM_MODULE_EVT_PORT_ACTION_PWM_SLOW_CHG:
            if (port_index <= 5) {
                break;
            }
            g_bt_bm_module_pwm_width[port_index - 6] = pwm_width & 0xFF;
            if (bt_at_cmd_sdk_adp_set_pwm_width(g_bt_bm_module_pwm_width, GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN) != BT_PRIVT_OK) {
                return;
            }
            break;
    }
    return;
}

void bt_bm_module_evt_1s_timer_task(void)
{
    uint64_t now_rtc_second = 0;
    int ret;
    int event_index;
    int port_index;

    if (g_bt_bm_module_rtc_change_should_re_calculate_event_trig_time == BT_PRIVT_TRUE) {
        g_bt_bm_module_rtc_change_should_re_calculate_event_trig_time = BT_PRIVT_FALSE;
        for (event_index = 0; event_index < GATT_CHARACTERISTIC_EVT_MAX_EVENT_NUM; event_index++) {
            if (bt_bm_module_evt_event.event[event_index].is_config != BT_PRIVT_TRUE) {
                continue;
            }
            ret = bt_bm_module_evt_calculate_event_trig_time_in_second(event_index);
            if (ret != BT_PRIVT_OK) {
                continue;
            }
        }
    }

    if ((bt_bm_module_evt_event.is_enable != BT_PRIVT_ENABLE) ||
        (bt_bm_module_evt_event.port_bit_map == 0) ||
        (bt_bm_module_evt_port_config.bits.all_en == 0)) {
        return;
    }

    ret = bt_at_cmd_sdk_adp_get_rtc_time_second(&now_rtc_second);
    if (ret != BT_PRIVT_OK) {
        return;
    }

    for (event_index = 0; event_index < GATT_CHARACTERISTIC_EVT_MAX_EVENT_NUM; event_index++) {
        if (bt_bm_module_evt_event.event[event_index].is_enable != BT_PRIVT_ENABLE) {
            continue;
        }

        if (now_rtc_second < bt_bm_module_evt_event.event[event_index].event_trig_in_rtc_second_time) {
            continue;
        }

        bt_bm_module_evt_upgrate_event_trig_time_in_second(event_index);
        if (bt_bm_module_evt_event.event[event_index].event_port_bit_map == 0) {
            continue;
        }
        for (port_index = 0; port_index < GATT_CHARACTERISTIC_EVT_MAX_PORT_NUM; port_index++) {
            if ((bt_bm_module_evt_event.event[event_index].event_port_bit_map & (1 << port_index)) == 0) {
                continue;
            }

            if ((bt_bm_module_evt_port_config.whole & (1 << (port_index + 1))) == 0) {
                continue;
            }

            // port_index event_index action
            dbg_printf("action evt %d port %d\r\n", event_index, port_index);
            bt_bm_module_evt_port_action(event_index, port_index);
        }
    }

    return;
}

int bt_bm_module_sys_id_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_sys_id_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_init_key_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_init_key_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_rssi_value_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

int bt_bm_module_rssi_value_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

void bt_bm_module_send_data()
{
#ifdef ING_CHIPS_PRIVATE_SERVICE
    bt_bm_module_health_thermometer_service_send_temperature();
#endif

    bt_bm_module_uart_rx_to_ble_service_send_data();
    bt_bm_module_adc_value_service_send_data();
    bt_bm_module_gpio_value_service_send_data();
}

int bt_bm_module_gatt_read_callback_register(uint16_t cmd, pfun_bt_bm_module_att_read_callback fun)
{
    int cmd_index;

    if (module_gatt_read_callback_num >= BT_BM_MODULE_GATT_HANDLE_MAX_NUM) {
        dbg_printf("bt_bm_module_gatt_read_callback_register fail!\r\n");
        return BT_PRIVT_ERROR;
    }

    for (cmd_index = 0; cmd_index < module_gatt_read_callback_num; cmd_index++) {
        if (module_gatt_read_callback[cmd_index].cmd == cmd) {
            module_gatt_read_callback[cmd_index].fun = fun;
            return BT_PRIVT_OK;
        }
    }

    module_gatt_read_callback[module_gatt_read_callback_num].cmd = cmd;
    module_gatt_read_callback[module_gatt_read_callback_num].fun = fun;
    module_gatt_read_callback_num++;

    return BT_PRIVT_OK;
}

int bt_bm_module_gatt_write_callback_register(uint16_t cmd, pfun_bt_bm_module_att_write_callback fun)
{
    int cmd_index;

    if (module_gatt_write_callback_num >= BT_BM_MODULE_GATT_HANDLE_MAX_NUM) {
        dbg_printf("bt_bm_module_gatt_write_callback_register fail!\r\n");
        return BT_PRIVT_ERROR;
    }

    for (cmd_index = 0; cmd_index < module_gatt_write_callback_num; cmd_index++) {
        if (module_gatt_write_callback[cmd_index].cmd == cmd) {
            module_gatt_write_callback[cmd_index].fun = fun;
            return BT_PRIVT_OK;
        }
    }

    module_gatt_write_callback[module_gatt_write_callback_num].cmd = cmd;
    module_gatt_write_callback[module_gatt_write_callback_num].fun = fun;
    module_gatt_write_callback_num++;

    return BT_PRIVT_OK;
}

int bt_bm_module_gatt_read_callback(uint16_t cmd, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int cmd_index;

    dbg_printf("gatt read callback: %u\r\n", cmd);
    for (cmd_index = 0; cmd_index < module_gatt_read_callback_num; cmd_index++) {
        if ((module_gatt_read_callback[cmd_index].cmd == cmd) &&
            (module_gatt_read_callback[cmd_index].fun != BT_PRIVT_NULL)) {
            return module_gatt_read_callback[cmd_index].fun(offset, buffer, buffer_size);
        }
    }

    return BT_PRIVT_ERROR;
}

int bt_bm_module_gatt_write_callback(uint16_t cmd, uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    int cmd_index;

    dbg_printf("gatt write callback: %u\r\n", cmd);
    for (cmd_index = 0; cmd_index < module_gatt_write_callback_num; cmd_index++) {
        if ((module_gatt_write_callback[cmd_index].cmd == cmd) &&
            (module_gatt_write_callback[cmd_index].fun != BT_PRIVT_NULL)) {
            dbg_printf("found cmd\r\n");
            return module_gatt_write_callback[cmd_index].fun(offset, buffer, buffer_size);
        }
    }

    return BT_PRIVT_ERROR;
}

#if defined __cplusplus
    }
#endif

