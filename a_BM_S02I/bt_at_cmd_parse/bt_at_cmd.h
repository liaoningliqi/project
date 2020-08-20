
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BT_AT_CMD_H__
#define __BT_AT_CMD_H__

#include <stdint.h>

#include "freeRTOS.h"
#include "timers.h"

#include "att_db.h"

#if defined __cplusplus
    extern "C" {
#endif

#define DEBUG 1
#define UART_PORT 0
//#define ING_CHIPS_PRIVATE_SERVICE 1

#ifndef DEBUG
    #warning DEBUG should be defined 0 or 1
    #define DEBUG 0
#endif

#if (DEBUG == 1)
    #define dbg_printf(...) printf(__VA_ARGS__)
#else
    #define dbg_printf(...)
#endif

#if (UART_PORT == 0)
    #define USR_UART_IO_PORT APB_UART0
#else
    #define USR_UART_IO_PORT APB_UART1
#endif

#define STRUCT_TM_YEAR_BASE_LINE 1900
#define STRUCT_TM_MONTH_BASE_LINE 1

#define BATTERY_ADC_CHANNEL 7
#define BT_BM_SDK_ADC0_CHANNEL 0
#define BT_BM_SDK_ADC1_CHANNEL 1

#define USER_MSG_ID_HEALTH_THERMOMETER_SERVICE 1
#define USER_MSG_ID_UART_RX_TO_BLE_DATA_SERVICE 2
#define USER_MSG_ID_ADC_VALUE_SERVICE 3
#define USER_MSG_ID_GPIO_IN_SERVICE 4
#define USER_MSG_ID_RSSI_SEND 5
#define USER_MSG_ID_SET_INTERVAL 6

#define GATT_CHARACTERISTIC_BLE_TO_TX_DATA_LEN 20
#define GATT_CHARACTERISTIC_BLE_TO_TX_DATA_BUF_LEN 80
#define GATT_CHARACTERISTIC_RX_TO_BLE_DATA_LEN 20
#define GATT_CHARACTERISTIC_MAX_DATA_LEN GATT_CHARACTERISTIC_BLE_TO_TX_DATA_LEN

#define GATT_CHARACTERISTIC_TEMPERATURE_DATA_LEN        5
#define GATT_CHARACTERISTIC_BATTERY_VALUE_DATA_LEN      5
#define GATT_CHARACTERISTIC_PWM_OUT_INIT_DATA_LEN       1
#define GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN      4
#define GATT_CHARACTERISTIC_PWM_OUT_FREQ_DATA_LEN       2
#define GATT_CHARACTERISTIC_PWM_OUT_TRANS_DATA_LEN      2
#define GATT_CHARACTERISTIC_ADC_EN_DATA_LEN             1
#define GATT_CHARACTERISTIC_ADC_PERIOD_DATA_LEN         2
#define GATT_CHARACTERISTIC_ADC_VALUE0_DATA_LEN         2
#define GATT_CHARACTERISTIC_ADC_VALUE1_DATA_LEN         2
#define GATT_CHARACTERISTIC_GPIO_CONFIG_DATA_LEN        1
#define GATT_CHARACTERISTIC_GPIO_OUT_DATA_LEN           1
#define GATT_CHARACTERISTIC_GPIO_IN_DATA_LEN            1
#define GATT_CHARACTERISTIC_MODULE_PARA_NAME_LEN        16
#define GATT_CHARACTERISTIC_EVT_INVALID_TIME_BYTE       0xFF
#define GATT_CHARACTERISTIC_EVT_MAX_EVENT_NUM           32
#define GATT_CHARACTERISTIC_EVT_MAX_PORT_NUM            10
#define GATT_CHARACTERISTIC_EVT_RTC_LEN                 7
#define GATT_CHARACTERISTIC_EVT_RCHANNEL_DATA_LEN       1
#define GATT_CHARACTERISTIC_EVT_WCHANNEL_DATA_LEN       12
#define GATT_CHARACTERISTIC_EVT_PORT_INDEX_DATA_LEN     1
#define GATT_CHARACTERISTIC_EVT_PORT_RWCHANNEL_DATA_LEN 5
#define GATT_CHARACTERISTIC_EVT_PORT_CONFIG_DATA_LEN    2

#define BT_AT_CMD_MAX_LEN 30
#define BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN 16
#define BT_AT_CMD_TTM_ADV_DATA_MAX_LEN 16
#define BT_AT_CMD_TTM_PRODUCT_ID_LEN 2
#define BT_AT_CMD_TTM_RTC_TIME_LEN 14
#define BT_AT_CMD_TTM_MAC_ADDRESS_LEN 6
#define BT_AT_CMD_TTM_MAC_ADDRESS_STR_LEN (BT_AT_CMD_TTM_MAC_ADDRESS_LEN * 2)

typedef enum {
    BT_AT_CMD_SDK_GPIO_READ = 0,
    BT_AT_CMD_SDK_GPIO_WRITE,
    BT_AT_CMD_SDK_GPIO_CONFIG_READ,
    BT_AT_CMD_SDK_GPIO_CONFIG_WRITE,

    BT_AT_CMD_SDK_UART_READ,
    BT_AT_CMD_SDK_UART_WRITE,

    BT_AT_CMD_SDK_IIC_READ,
    BT_AT_CMD_SDK_IIC_WRITE,

    BT_AT_CMD_SDK_SPI_READ,
    BT_AT_CMD_SDK_SPI_WRITE,

    BT_AT_CMD_SDK_TIMER_READ,
    BT_AT_CMD_SDK_TIMER_WRITE,

    BT_AT_CMD_SDK_PWM_READ,
    BT_AT_CMD_SDK_PWM_WRITE,

    BT_AT_CMD_SDK_ADC_READ,
    BT_AT_CMD_SDK_ADC_WRITE,

    BT_AT_CMD_SDK_RTC_READ_SECOND,
    BT_AT_CMD_SDK_RTC_READ,
    BT_AT_CMD_SDK_RTC_WRITE,

    BT_AT_CMD_SDK_MODULE_NAME_READ,
    BT_AT_CMD_SDK_MODULE_NAME_WRITE,

    BT_AT_CMD_SDK_BAUD_RATE_READ,
    BT_AT_CMD_SDK_BAUD_RATE_WRITE,

    BT_AT_CMD_SDK_MAC_ADD_READ,
    BT_AT_CMD_SDK_MAC_ADD_WRITE,

    BT_AT_CMD_SDK_CON_INTV_WRITE,

    BT_AT_CMD_SDK_MODULE_RESET,

    BT_AT_CMD_SDK_ADV_PERIOD_WRITE,
    BT_AT_CMD_SDK_ADV_DATA_WRITE,

    BT_AT_CMD_SDK_PRODUCT_ID_WRITE,

    BT_AT_CMD_SDK_TX_POW_WRITE,

    BT_AT_CMD_SDK_COM_DELAY_WRITE,

    BT_AT_CMD_SDK_EN_UP_WRITE,

    BT_AT_CMD_SDK_RSSI_EN_WRITE,

    BT_AT_CMD_SDK_TEMPERATURE_READ,

    BT_AT_CMD_SDK_RGB_LIGHT_WRITE,

    BT_AT_CMD_SDK_END
} bt_at_cmd_sdk_hook_cmd_e;

typedef int (*pfunbt_at_cmd_sdk_hook)(const void *para_in, int para_in_len, void *data_out, int data_out_len);

typedef struct {
    int cmd;
    pfunbt_at_cmd_sdk_hook fun;
} bt_at_cmd_sdk_hook_fun_t;

extern hci_con_handle_t g_bt_bm_hci_con_handle_send;

#ifdef ING_CHIPS_PRIVATE_SERVICE
extern uint16_t g_bt_bm_module_att_temp_value_handle;
extern TimerHandle_t g_health_thermometer_service_timer;
extern int g_bt_bm_module_health_thermometer_service_notify_enable;
extern int g_bt_bm_module_health_thermometer_service_indicate_enable;
#endif

extern uint16_t g_bt_bm_module_uart_rx_to_ble_handle;

extern uint16_t g_bt_bm_module_adc_value0_handle;
extern uint16_t g_bt_bm_module_adc_value1_handle;
extern TimerHandle_t g_adc0_value_service_timer;
extern TimerHandle_t g_adc1_value_service_timer;
extern int g_bt_bm_module_adc0_value_service_notify_enable;
extern int g_bt_bm_module_adc0_value_service_indicate_enable;
extern int g_bt_bm_module_adc1_value_service_notify_enable;
extern int g_bt_bm_module_adc1_value_service_indicate_enable;

extern uint16_t g_bt_bm_module_gpio_in_handle;
extern int g_bt_bm_module_gpio_in_service_notify_enable;
extern int g_bt_bm_module_gpio_in_service_indicate_enable;

extern uint8_t g_bt_bm_module_pwm_width[];
extern uint8_t g_bt_bm_module_gpio_config[];
extern uint8_t g_bt_bm_module_gpio_out[];

extern int g_bt_bm_module_rtc_change_should_re_calculate_event_trig_time;

int bt_at_cmd_parse(const char *str_in, int str_in_len, char *str_out, int str_out_len);
void bt_at_cmd_uart_io_init(void);
int bt_at_cmd_sdk_adp_register_hook(bt_at_cmd_sdk_hook_fun_t *sdk_cmd, const int sdk_cmd_num);

int bt_bm_module_init_service(void);

int bt_bm_module_gatt_read_callback(uint16_t cmd, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
int bt_bm_module_gatt_write_callback(uint16_t cmd, uint16_t offset, const uint8_t * buffer, uint16_t buffer_size);
void bt_bm_module_send_data(void);
void bt_bm_module_gpio_in_service_send_msg(void);

int bt_at_cmd_uart_out_data(const unsigned char *data, int data_len);

void bt_at_cmd_parse_send_data_by_ble(const char *str_in, int str_in_len);

void bt_bm_module_evt_1s_timer_task(void);
void bt_at_cmd_parse_init(void);

#if defined __cplusplus
    }
#endif

#endif

