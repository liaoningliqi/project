
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BT_BM_MODULE_GATT_CALLBACK__
#define __BT_BM_MODULE_GATT_CALLBACK__

#include <stdint.h>

#include "freeRTOS.h"
#include "timers.h"

#if defined __cplusplus
    extern "C" {
#endif

typedef int (*pfun_bt_bm_module_att_read_callback)(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
typedef int (*pfun_bt_bm_module_att_write_callback)(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);

typedef struct {
    uint16_t cmd;
    pfun_bt_bm_module_att_read_callback fun;
} bt_bm_module_att_read_callback_t;

typedef struct {
    uint16_t cmd;
    pfun_bt_bm_module_att_write_callback fun;
} bt_bm_module_att_write_callback_t;

extern uint8_t g_bt_bm_module_uart_rx_to_ble_data[];
extern int g_bt_bm_module_uart_rx_to_ble_data_len;
extern int g_bt_bm_module_rx_to_ble_can_send_data;

int bt_bm_module_gatt_read_callback_register(uint16_t cmd, pfun_bt_bm_module_att_read_callback fun);
int bt_bm_module_gatt_write_callback_register(uint16_t cmd, pfun_bt_bm_module_att_write_callback fun);

void bt_bm_module_health_thermometer_service_timer_callback(TimerHandle_t xTimer);

int bt_bm_rgb_light_control_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_ble_to_tx_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_thermometer_client_desc_value_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_thermometer_value_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_rx_to_ble_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_rx_to_ble_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_battery_value_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_pwm_out_init_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_pwm_out_init_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_pwm_out_width_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_pwm_out_width_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_pwm_out_freq_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_pwm_out_freq_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_pwm_out_trans_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_pwm_out_trans_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_adc_en_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_adc_en_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_adc_period_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_adc_period_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_adc_value0_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_adc_value0_notify_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_adc_value1_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_adc_value1_notify_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
void bt_bm_module_adc0_value_service_timer_callback(TimerHandle_t xTimer);
void bt_bm_module_adc1_value_service_timer_callback(TimerHandle_t xTimer);
int bt_bm_module_gpio_config_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_gpio_config_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_gpio_out_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_gpio_in_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_gpio_in_notify_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_para_name_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_para_name_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_rtc_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_rtc_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_rchannel_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_rchannel_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_wchannel_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_wchannel_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_port_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_port_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_port_rwchannel_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_port_rwchannel_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_port_config_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_evt_port_config_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_sys_id_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_sys_id_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_init_key_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_init_key_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_rssi_value_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int bt_bm_module_rssi_value_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);

#if defined __cplusplus
    }
#endif

#endif

