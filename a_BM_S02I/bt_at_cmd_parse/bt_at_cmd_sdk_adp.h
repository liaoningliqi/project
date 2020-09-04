
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BT_AT_CMD_SDK_ADP_H__
#define __BT_AT_CMD_SDK_ADP_H__

#include <stdint.h>
#include <time.h>

#if defined __cplusplus
    extern "C" {
#endif

int bt_at_cmd_sdk_adp_set_con_intv(unsigned int connection_interval, char *str_out, int str_out_len);
int bt_at_cmd_sdk_adp_get_module_name(char *str_out, int str_out_len);
int bt_at_cmd_sdk_adp_set_module_name(const char *module_name, char *str_out, int str_out_len);
int bt_at_cmd_sdk_adp_get_baud_rate(char *str_out, int str_out_len);
int bt_at_cmd_sdk_adp_set_baud_rate(unsigned int baud_rate);
int bt_at_cmd_sdk_adp_get_mac_address(char *str_out, int str_out_len);
int bt_at_cmd_sdk_adp_set_mac_address(const unsigned char *mac_addres, int in_len);
int bt_at_cmd_sdk_adp_module_reset(void);
int bt_at_cmd_sdk_adp_set_adv_period(unsigned int adv_period);
int bt_at_cmd_sdk_adp_set_adv_data(const char *adv_data);
int bt_at_cmd_sdk_adp_set_product_id(unsigned char *product_id, int in_len);
int bt_at_cmd_sdk_adp_set_tx_power(int tx_power);
int bt_at_cmd_sdk_adp_get_rtc_time_second(uint64_t *rtc_time);
int bt_at_cmd_sdk_adp_get_rtc_time(struct tm *rtc_time);
int bt_at_cmd_sdk_adp_set_rtc_time(const struct tm *rtc_time);
int bt_at_cmd_sdk_adp_set_com_out_delay(unsigned int com_out_delay);
int bt_at_cmd_sdk_adp_set_en_up_enable(unsigned int en_flag);
int bt_at_cmd_sdk_adp_set_rssi_enable(unsigned int en_flag);
int bt_at_cmd_sdk_adp_get_temperature(int32_t *temperature);
int bt_at_cmd_sdk_adp_get_adc_value(int adc, uint16_t *value);
int bt_at_cmd_sdk_adp_get_gpio_value(uint8_t *value);
int bt_at_cmd_sdk_adp_set_gpio_value(uint8_t *value);
int bt_at_cmd_sdk_adp_get_gpio_config(uint8_t *value);
int bt_at_cmd_sdk_adp_set_gpio_config(uint8_t *value);
int bt_at_cmd_sdk_adp_set_pwm_width(const uint8_t *pwm_width, int in_len);
int bt_at_cmd_sdk_adp_set_rgb_light(const uint8_t *value, int para_in_len);
int bt_at_cmd_sdk_adp_set_uart_data(const uint8_t *value, int para_in_len);

#if defined __cplusplus
    }
#endif

#endif

