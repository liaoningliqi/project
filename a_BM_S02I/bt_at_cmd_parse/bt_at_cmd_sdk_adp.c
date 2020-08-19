
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "bt_at_cmd.h"
#include "bt_at_cmd_parse.h"

#if defined __cplusplus
    extern "C" {
#endif

static bt_at_cmd_sdk_hook_fun_t *bt_at_sdk_cmd_adp = BT_PRIVT_NULL;
static int bt_at_sdk_cmd_adp_num = 0;

int bt_at_cmd_sdk_adp_register_hook(bt_at_cmd_sdk_hook_fun_t *sdk_cmd, const int sdk_cmd_num)
{
    if (sdk_cmd == BT_PRIVT_NULL) {
        return BT_PRIVT_ERROR;
    }

    bt_at_sdk_cmd_adp = sdk_cmd;
    bt_at_sdk_cmd_adp_num = sdk_cmd_num;

    return BT_PRIVT_OK;
}

int bt_at_cmd_sdk_adp_cmd_parse_fun(int cmd, const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    int i;
    int ret = BT_PRIVT_NO_CMD;

    if (bt_at_sdk_cmd_adp == BT_PRIVT_NULL) {
        return ret;
    }

    for (i = 0; i < bt_at_sdk_cmd_adp_num; i++) {
        if ((bt_at_sdk_cmd_adp[i].cmd == cmd) &&
            (bt_at_sdk_cmd_adp[i].fun != BT_PRIVT_NULL)) {
            ret = bt_at_sdk_cmd_adp[i].fun(para_in, para_in_len, data_out, data_out_len);
            break;
        }
    }

    return ret;
}

int bt_at_cmd_sdk_adp_set_con_intv(unsigned int connection_interval, char *str_out, int str_out_len)
{
    int ret;

    // call adk
    dbg_printf("connection interval: %u\r\n", connection_interval);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_CON_INTV_WRITE, &connection_interval, sizeof(connection_interval), BT_PRIVT_NULL, 0);
    if (ret == BT_PRIVT_TIMEOUT) {
        strncpy(str_out, BT_AT_CMD_TTM_PARSE_TIMEOUT, str_out_len);
        ret = BT_PRIVT_OK;
    }

    return ret;
}

int bt_at_cmd_sdk_adp_get_module_name(char *str_out, int str_out_len)
{
    int ret;

    // call adk
    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_MODULE_NAME_READ, BT_PRIVT_NULL, 0, str_out, str_out_len);
    dbg_printf("get module name ret %d\r\n", ret);

    return ret;
}

int bt_at_cmd_sdk_adp_set_module_name(const char *module_name, char *str_out, int str_out_len)
{
    int ret;

    // call adk
    dbg_printf("module name: %s\r\n", module_name);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_MODULE_NAME_WRITE, module_name, 0, BT_PRIVT_NULL, 0);
    if (ret == BT_PRIVT_TIMEOUT) {
        strncpy(str_out, BT_AT_CMD_TTM_PARSE_TIMEOUT, str_out_len);
        ret = BT_PRIVT_OK;
    }

    return ret;
}

int bt_at_cmd_sdk_adp_get_baud_rate(char *str_out, int str_out_len)
{
    // call adk
    dbg_printf("get baud rate\r\n");

    return bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_BAUD_RATE_READ, BT_PRIVT_NULL, 0, str_out, str_out_len);
}

int bt_at_cmd_sdk_adp_set_baud_rate(unsigned int baud_rate)
{
    int ret;

    dbg_printf("baud rate: %u\r\n", baud_rate);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_BAUD_RATE_WRITE, &baud_rate, sizeof(baud_rate), BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_get_mac_address(char *str_out, int str_out_len)
{
    // call adk
    dbg_printf("get mac address\r\n");

    return bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_MAC_ADD_READ, BT_PRIVT_NULL, 0, str_out, str_out_len);
}

int bt_at_cmd_sdk_adp_set_mac_address(const unsigned char *mac_addres, int in_len)
{
    int ret;

    dbg_printf("mac 0x%x%x_%x%x_%x%x_%x%x_%x%x_%x%x\r\n",
        mac_addres[0], mac_addres[1], mac_addres[2], mac_addres[3], mac_addres[4], mac_addres[5],
        mac_addres[6], mac_addres[7], mac_addres[8], mac_addres[9], mac_addres[10], mac_addres[11]);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_MAC_ADD_WRITE, mac_addres, in_len, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_module_reset(void)
{
    int ret;

    dbg_printf("module reset\r\n");

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_MODULE_RESET, BT_PRIVT_NULL, 0, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_adv_period(unsigned int adv_period)
{
    int ret;

    dbg_printf("adv period: %u\r\n", adv_period);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_ADV_PERIOD_WRITE, BT_PRIVT_NULL, 0, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_adv_data(const char *adv_data)
{
    int ret;

    dbg_printf("adv data: %s\r\n", adv_data);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_ADV_DATA_WRITE, BT_PRIVT_NULL, 0, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_product_id(unsigned char *product_id, int in_len)
{
    int ret;

    dbg_printf("product id: 0x%x_%x\r\n", product_id[0], product_id[1]);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_PRODUCT_ID_WRITE, product_id, in_len, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_tx_power(int tx_power)
{
    int ret;

    dbg_printf("tx power: %d\r\n", tx_power);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_TX_POW_WRITE, &tx_power, sizeof(tx_power), BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_get_rtc_time_second(uint64_t *rtc_time)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_RTC_READ_SECOND, BT_PRIVT_NULL, 0, rtc_time, sizeof(*rtc_time));

    return ret;
}

int bt_at_cmd_sdk_adp_get_rtc_time(struct tm *rtc_time)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_RTC_READ, BT_PRIVT_NULL, 0, rtc_time, sizeof(*rtc_time));

    return ret;
}

int bt_at_cmd_sdk_adp_set_rtc_time(const struct tm *rtc_time)
{
    int ret;

    dbg_printf("set rtc write y%u m%u d%u h%u m%u s%u\r\n",
        rtc_time->tm_year, rtc_time->tm_mon, rtc_time->tm_mday,
        rtc_time->tm_hour, rtc_time->tm_min, rtc_time->tm_sec);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_RTC_WRITE, rtc_time, sizeof(*rtc_time), BT_PRIVT_NULL, 0);
    g_bt_bm_module_rtc_change_should_re_calculate_event_trig_time = BT_PRIVT_TRUE;

    return ret;
}

int bt_at_cmd_sdk_adp_set_com_out_delay(unsigned int com_out_delay)
{
    int ret;

    dbg_printf("com out delay: %u\r\n", com_out_delay);

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_COM_DELAY_WRITE, BT_PRIVT_NULL, 0, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_en_up_enable(unsigned int en_flag)
{
    int ret;

    dbg_printf("enable up: %s\r\n", ((en_flag == BT_PRIVT_ENABLE) ? "on" : "off"));

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_EN_UP_WRITE, BT_PRIVT_NULL, 0, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_rssi_enable(unsigned int en_flag)
{
    int ret;

    dbg_printf("rssi: %s\r\n", ((en_flag == BT_PRIVT_ENABLE) ? "on" : "off"));

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_RSSI_EN_WRITE, &en_flag, sizeof(en_flag), BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_get_temperature(int32_t *temperature)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_TEMPERATURE_READ, BT_PRIVT_NULL, 0, temperature, sizeof(*temperature));

    return ret;
}

int bt_at_cmd_sdk_adp_get_adc_value(int adc, uint16_t *value)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_ADC_READ, &adc, sizeof(adc), value, sizeof(*value));

    return ret;
}

int bt_at_cmd_sdk_adp_get_gpio_value(uint8_t *value)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_GPIO_READ, BT_PRIVT_NULL, 0, value, sizeof(*value));

    return ret;
}

int bt_at_cmd_sdk_adp_set_gpio_value(uint8_t *value)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_GPIO_WRITE, value, sizeof(*value), BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_get_gpio_config(uint8_t *value)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_GPIO_CONFIG_READ, BT_PRIVT_NULL, 0, value, sizeof(*value));

    return ret;
}

int bt_at_cmd_sdk_adp_set_gpio_config(uint8_t *value)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_GPIO_CONFIG_WRITE, value, sizeof(*value), BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_pwm_width(const uint8_t *pwm_width, int in_len)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_PWM_WRITE, pwm_width, in_len, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_rgb_light(const uint8_t *value, int para_in_len)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_RGB_LIGHT_WRITE, value, para_in_len, BT_PRIVT_NULL, 0);

    return ret;
}

int bt_at_cmd_sdk_adp_set_uart_data(const uint8_t *value, int para_in_len)
{
    int ret;

    ret = bt_at_cmd_sdk_adp_cmd_parse_fun(BT_AT_CMD_SDK_UART_WRITE, value, para_in_len, BT_PRIVT_NULL, 0);

    return ret;
}

#if defined __cplusplus
    }
#endif

