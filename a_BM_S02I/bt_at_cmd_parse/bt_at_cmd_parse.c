
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>

#include "btstack_event.h"

#include "bt_at_cmd.h"
#include "bt_at_cmd_parse.h"
#include "bt_at_cmd_sdk_adp.h"
#include "bt_bm_module_gatt_callback.h"

#if defined __cplusplus
    extern "C" {
#endif

#define BT_AT_CMD_TTM "TTM:"

#define BT_AT_CMD_TTM_CIT "CIT-"
#define BT_AT_CMD_TTM_NAM "NAM-"
#define BT_AT_CMD_TTM_REN "REN-"
#define BT_AT_CMD_TTM_BPS "BPS-"
#define BT_AT_CMD_TTM_MAC "MAC-"
#define BT_AT_CMD_TTM_RST "RST-"
#define BT_AT_CMD_TTM_ADP "ADP-"
#define BT_AT_CMD_TTM_ADD "ADD-"
#define BT_AT_CMD_TTM_PID "PID-"
#define BT_AT_CMD_TTM_TPL "TPL-"
#define BT_AT_CMD_TTM_EUP "EUP-"
#define BT_AT_CMD_TTM_RSI "RSI-"
#define BT_AT_CMD_TTM_RTC "RTC-"
#define BT_AT_CMD_TTM_CDL "CDL-"

#define BT_AT_CMD_TTM_RST_SYS "SYSTEMRESET"

#define BT_AT_CMD_TTM_EUP_ON "ON"
#define BT_AT_CMD_TTM_EUP_OFF "OFF"

#define BT_AT_CMD_TTM_RSI_ON "ON"
#define BT_AT_CMD_TTM_RSI_OFF "OFF"

#define BT_AT_CMD_TTM_BPS_SET "BPS SET AFTER 2S...\r\n\0"

static bt_at_cmd_parse_fun_t cmd_root[] = {
    {BT_AT_CMD_TTM, bt_at_cmd_parse_ttm},
};
static int cmd_root_num = sizeof(cmd_root) / sizeof(cmd_root[0]);

static bt_at_cmd_parse_fun_t cmd_ttm[] = {
    {BT_AT_CMD_TTM_CIT, bt_at_cmd_parse_ttm_cit},
    {BT_AT_CMD_TTM_NAM, bt_at_cmd_parse_ttm_nam},
    {BT_AT_CMD_TTM_REN, bt_at_cmd_parse_ttm_ren},
    {BT_AT_CMD_TTM_BPS, bt_at_cmd_parse_ttm_bps},
    {BT_AT_CMD_TTM_MAC, bt_at_cmd_parse_ttm_mac},
    {BT_AT_CMD_TTM_RST, bt_at_cmd_parse_ttm_rst},
    {BT_AT_CMD_TTM_ADP, bt_at_cmd_parse_ttm_adp},
    {BT_AT_CMD_TTM_ADD, bt_at_cmd_parse_ttm_add},
    {BT_AT_CMD_TTM_PID, bt_at_cmd_parse_ttm_pid},
    {BT_AT_CMD_TTM_TPL, bt_at_cmd_parse_ttm_tpl},
    {BT_AT_CMD_TTM_EUP, bt_at_cmd_parse_ttm_eup},
    {BT_AT_CMD_TTM_RSI, bt_at_cmd_parse_ttm_rsi},
    {BT_AT_CMD_TTM_RTC, bt_at_cmd_parse_ttm_rtc},
    {BT_AT_CMD_TTM_CDL, bt_at_cmd_parse_ttm_cdl},
};
static int cmd_ttm_num = sizeof(cmd_ttm) / sizeof(cmd_ttm[0]);

static bt_at_cmd_parse_fun_t cmd_ttm_eup[] = {
    {BT_AT_CMD_TTM_EUP_ON, bt_at_cmd_parse_ttm_eup_on},
    {BT_AT_CMD_TTM_EUP_OFF, bt_at_cmd_parse_ttm_eup_off},
};
static int cmd_ttm_eup_num = sizeof(cmd_ttm_eup) / sizeof(cmd_ttm_eup[0]);

static bt_at_cmd_parse_fun_t cmd_ttm_rsi[] = {
    {BT_AT_CMD_TTM_RSI_ON, bt_at_cmd_parse_ttm_rsi_on},
    {BT_AT_CMD_TTM_RSI_OFF, bt_at_cmd_parse_ttm_rsi_off},
};
static int cmd_ttm_rsi_num = sizeof(cmd_ttm_rsi) / sizeof(cmd_ttm_rsi[0]);

unsigned int bt_at_cmd_parse_is_match(const char *str_in, int str_in_len, char *str_in_cmd)
{
    int char_index = 0;

    while ((str_in[char_index] != '\0') && (str_in_cmd[char_index] != '\0')) {
        if (char_index >= str_in_len) {
            break;
        }
        if (str_in[char_index] != str_in_cmd[char_index]) {
            break;
        }
        char_index++;
    }

    if (str_in_cmd[char_index] == '\0') {
        return BT_PRIVT_TRUE;
    }

    return BT_PRIVT_FALSE;
}

int bt_at_cmd_parse_ttm(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int cmd_index;
    int str_in_index = sizeof(BT_AT_CMD_TTM) - 1;
    int str_out_index = sizeof(BT_AT_CMD_TTM) - 1;
    int in_len;
    int out_len;
    int ret;

    strncpy((char *)str_out, BT_AT_CMD_TTM, str_out_len);
    in_len = str_in_len - str_in_index;
    out_len = str_out_len - str_out_index;
    if ((in_len < 0) || (out_len < 0)) {
        return BT_PRIVT_ERROR;
    }

    for (cmd_index = 0; cmd_index < cmd_ttm_num; cmd_index++) {
        if (bt_at_cmd_parse_is_match(&(str_in[str_in_index]), in_len, cmd_ttm[cmd_index].cmd) &&
            (cmd_ttm[cmd_index].fun != BT_PRIVT_NULL)) {
            strncpy(&(str_out[str_out_index]), BT_AT_CMD_TTM_PARSE_OK, out_len);
            ret = cmd_ttm[cmd_index].fun(&(str_in[str_in_index]), in_len, &(str_out[str_out_index]), out_len);
            if (ret != BT_PRIVT_OK) {
                strncpy(&(str_out[str_out_index]), BT_AT_CMD_TTM_PARSE_ERP, out_len);
            }
            return BT_PRIVT_OK;
        }
    }

    strncpy(&(str_out[str_out_index]), BT_AT_CMD_TTM_PARSE_NO_CMD, out_len);
    return BT_PRIVT_ERROR;
}

int bt_at_cmd_parse_ttm_ms_str(const char *str_in, int str_in_len, unsigned int *out)
{
    unsigned int ms_uint = 0;
    unsigned int i;
    unsigned int m_is_found = BT_PRIVT_FALSE;

    for (i = 0; i < str_in_len; i++) {
        if ((str_in[i] >= '0') && (str_in[i] <= '9')) {
            ms_uint = (ms_uint * 10) + (str_in[i] - '0');
        } else if (str_in[i] == 'm') {
            m_is_found = BT_PRIVT_TRUE;
            i++;
            break;
        } else {
            break;
        }
    }

    if ((m_is_found == BT_PRIVT_TRUE) && (i < str_in_len)) {
        if (str_in[i] == 's') {
            *out = ms_uint;
            return BT_PRIVT_OK;
        }
    }

    return BT_PRIVT_ERROR;
}

int bt_at_cmd_parse_ttm_cit(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_CIT) - 1;
    int in_len;
    unsigned int connection_interval = 0;
    int ret;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    ret = bt_at_cmd_parse_ttm_ms_str(&(str_in[str_in_index]), in_len, &connection_interval);
    if (ret == BT_PRIVT_OK) {
        if ((connection_interval == 20) ||
            (connection_interval == 30) ||
            (connection_interval == 50) ||
            (connection_interval == 100) ||
            (connection_interval == 200) ||
            (connection_interval == 300) ||
            (connection_interval == 400) ||
            (connection_interval == 500) ||
            (connection_interval == 1000) ||
            (connection_interval == 1500) ||
            (connection_interval == 2000)) {
            // set BLE connection interval
            ret = bt_at_cmd_sdk_adp_set_con_intv(connection_interval, str_out, str_out_len);
        } else {
            ret = BT_PRIVT_ERROR;
        }
    }

    return ret;
}

int bt_at_cmd_parse_ttm_nam(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_index = sizeof(BT_AT_CMD_TTM_NAM) - 1;
    int in_len;
    int out_len;

    in_len = str_in_len - str_index;
    out_len = str_out_len - str_index;
    if ((in_len < 0) || (out_len < 0)) {
        return BT_PRIVT_ERROR;
    }

    if (str_in[str_index] == '?') {
        if ((in_len == 1) || (str_in[str_index + 1] <= BT_AT_CMD_TTM_END_CHAR)) {
            // get module name
            strncpy(str_out, BT_AT_CMD_TTM_NAM, str_out_len);
            return bt_at_cmd_sdk_adp_get_module_name((&str_out[str_index]), out_len);
        }
    }

    return BT_PRIVT_ERROR;
}

unsigned int bt_at_cmd_parse_ttm_module_name_is_good(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int i;

    for (i = 0; i < str_out_len; i++) {
        if (i >= str_in_len) {
            str_out[i] = '\0';
            return BT_PRIVT_TRUE;
        }

        str_out[i] = str_in[i];

        if (str_in[i] <= BT_AT_CMD_TTM_END_CHAR) {
            str_out[i] = '\0';
            return BT_PRIVT_TRUE;
        }
    }

    if (i == str_out_len) {
        if (i == str_in_len) {
            return BT_PRIVT_TRUE;
        } else if (str_in[i] <= BT_AT_CMD_TTM_END_CHAR) {
            return BT_PRIVT_TRUE;
        } else {
        }
    }

    return BT_PRIVT_FALSE;
}

int bt_at_cmd_parse_ttm_ren(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_REN) - 1;
    char module_name[BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN + 1] = {0};
    int in_len;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if (bt_at_cmd_parse_ttm_module_name_is_good((&str_in[str_in_index]), in_len, module_name, BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN) == BT_PRIVT_TRUE) {
        // change module name
        return bt_at_cmd_sdk_adp_set_module_name(module_name, str_out, str_out_len);
    }

    return BT_PRIVT_ERROR;
}

unsigned int bt_at_cmd_parse_ttm_baud_rate_is_good(const char *str_in, int str_in_len, unsigned int *baud_rate)
{
    unsigned int rate_uint = 0;
    unsigned int i;
    unsigned int baud_rate_is_found = BT_PRIVT_FALSE;

    for (i = 0; i < 7; i++) { // max baud rate is 115200 which take 6 + 1 = 7 DEC bit
        if (i >= str_in_len) {
            baud_rate_is_found = BT_PRIVT_TRUE;
            break;
        }

        if ((str_in[i] >= '0') && (str_in[i] <= '9')) {
            rate_uint = (rate_uint * 10) + (str_in[i] - '0');
        } else if (str_in[i] <= BT_AT_CMD_TTM_END_CHAR) {
            baud_rate_is_found = BT_PRIVT_TRUE;
            break;
        } else {
            break;
        }
    }

    if (baud_rate_is_found == BT_PRIVT_TRUE) {
        if ((rate_uint == 4800) ||
            (rate_uint == 9600) ||
            (rate_uint == 19200) ||
            (rate_uint == 38400) ||
            (rate_uint == 57600) ||
            (rate_uint == 115200)) {
            *baud_rate = rate_uint;
            return BT_PRIVT_TRUE;
        }
    }

    return BT_PRIVT_FALSE;
}

int bt_at_cmd_parse_ttm_bps(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_index = sizeof(BT_AT_CMD_TTM_BPS) - 1;
    unsigned int baud_rate = 0;
    int in_len;
    int out_len;

    in_len = str_in_len - str_index;
    out_len = str_out_len - str_index;
    if ((in_len < 0) || (out_len < 0)) {
        return BT_PRIVT_ERROR;
    }

    if (str_in[str_index] == '?') {
        if ((in_len == 1) || (str_in[str_index + 1] <= BT_AT_CMD_TTM_END_CHAR)) {
            // get baud rate
            strncpy(str_out, BT_AT_CMD_TTM_BPS, str_out_len);
            return bt_at_cmd_sdk_adp_get_baud_rate((&str_out[str_index]), out_len);
        }
    } else if (bt_at_cmd_parse_ttm_baud_rate_is_good((&str_in[str_index]), in_len, &baud_rate) == BT_PRIVT_TRUE) {
        // set baud rate
        strncpy(str_out, BT_AT_CMD_TTM_BPS_SET, str_out_len);
        return bt_at_cmd_sdk_adp_set_baud_rate(baud_rate);
    } else {
    }

    return BT_PRIVT_ERROR;
}

unsigned int bt_at_cmd_parse_ttm_mac_address_is_good(const char *str_in, int str_in_len, unsigned char *mac_address, int out_len)
{
    int i;

    // string "0123456789AB" to HEX 0x01233456789AB
    if (str_in_len < out_len) {
        return BT_PRIVT_FALSE;
    } else if (str_in_len > out_len) {
        if (str_in[out_len] > BT_AT_CMD_TTM_END_CHAR) {
            return BT_PRIVT_FALSE;
        }
    } else {
    }

    for (i = 0; i < out_len; i++) {
        if ((str_in[i] >= '0') && (str_in[i] <= '9')) {
            mac_address[i] = str_in[i] - '0';
        } else if ((str_in[i] >= 'a') && (str_in[i] <= 'f')) {
            mac_address[i] = str_in[i] - 'a' + 10;
        } else if ((str_in[i] >= 'A') && (str_in[i] <= 'F')) {
            mac_address[i] = str_in[i] - 'A' + 10;
        } else {
            return BT_PRIVT_FALSE;
        }
    }

    return BT_PRIVT_TRUE;
}

int bt_at_cmd_parse_ttm_mac(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_index = sizeof(BT_AT_CMD_TTM_MAC) - 1;
    int in_len;
    int out_len;
    unsigned char mac_addres[BT_AT_CMD_TTM_MAC_ADDRESS_STR_LEN] = {0};

    in_len = str_in_len - str_index;
    out_len = str_out_len - str_index;
    if ((in_len < 0) || (out_len < 0)) {
        return BT_PRIVT_ERROR;
    }

    if (str_in[str_index] == '?') {
        if ((in_len == 1) || (str_in[str_index + 1] <= BT_AT_CMD_TTM_END_CHAR)) {
            // get mac address
            strncpy(str_out, BT_AT_CMD_TTM_MAC, str_out_len);
            return bt_at_cmd_sdk_adp_get_mac_address((&str_out[str_index]), out_len);
        }
    } else if (bt_at_cmd_parse_ttm_mac_address_is_good((&str_in[str_index]), in_len, mac_addres, BT_AT_CMD_TTM_MAC_ADDRESS_STR_LEN) == BT_PRIVT_TRUE) {
        // set mac address
        return bt_at_cmd_sdk_adp_set_mac_address(mac_addres, BT_AT_CMD_TTM_MAC_ADDRESS_STR_LEN);
    } else {
    }

    return BT_PRIVT_ERROR;
}

int bt_at_cmd_parse_ttm_rst(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_RST) - 1;
    int in_len;
    char cmd_in[sizeof(BT_AT_CMD_TTM_RST_SYS)] = {0};
    int cmd_len = sizeof(BT_AT_CMD_TTM_RST_SYS) - 1;

    in_len = str_in_len - str_in_index;
    if (in_len < cmd_len) {
        return BT_PRIVT_ERROR;
    } else if (in_len > cmd_len) {
        if (str_in[str_in_index + cmd_len] > BT_AT_CMD_TTM_END_CHAR) {
            return BT_PRIVT_ERROR;
        }
    } else {
    }

    strncpy(cmd_in, &str_in[str_in_index], cmd_len);
    cmd_in[cmd_len] = '\0';
    if (strcmp(cmd_in, BT_AT_CMD_TTM_RST_SYS) == 0) {
        return bt_at_cmd_sdk_adp_module_reset();
    }

    return BT_PRIVT_ERROR;
}

unsigned int bt_at_cmd_parse_ttm_adv_period_is_good(const char *str_in, int str_in_len, unsigned int *adv_period)
{
    unsigned int period_uint = 0;
    unsigned int i;
    unsigned int adv_period_is_found = BT_PRIVT_FALSE;

    for (i = 0; i < 3; i++) { // max adv period is 50 * 100ms which take 2 + 1 = 3 DEC bit
        if (i >= str_in_len) {
            adv_period_is_found = BT_PRIVT_TRUE;
            break;
        }

        if ((str_in[i] >= '0') && (str_in[i] <= '9')) {
            period_uint = (period_uint * 10) + (str_in[i] - '0');
        } else if (str_in[i] <= BT_AT_CMD_TTM_END_CHAR) {
            adv_period_is_found = BT_PRIVT_TRUE;
            break;
        } else {
            break;
        }
    }

    if (adv_period_is_found == BT_PRIVT_TRUE) {
        if ((period_uint == 2) ||
            (period_uint == 5) ||
            (period_uint == 10) ||
            (period_uint == 15) ||
            (period_uint == 20) ||
            (period_uint == 25) ||
            (period_uint == 30) ||
            (period_uint == 40) ||
            (period_uint == 50)) {
            *adv_period = period_uint;
            return BT_PRIVT_TRUE;
        }
    }

    return BT_PRIVT_FALSE;
}

int bt_at_cmd_parse_ttm_adp(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_ADP) - 1;
    unsigned int adv_period = 0;
    int in_len;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if (bt_at_cmd_parse_ttm_adv_period_is_good((&str_in[str_in_index]), in_len, &adv_period) == BT_PRIVT_TRUE) {
        // set adv period
        return bt_at_cmd_sdk_adp_set_adv_period(adv_period);
    }

    return BT_PRIVT_ERROR;
}

unsigned int bt_at_cmd_parse_ttm_adv_data_is_good(const char *str_in, int str_in_len, char *adv_data, int str_out_len)
{
    unsigned int i;
    unsigned int adv_data_is_found = BT_PRIVT_FALSE;
    unsigned int data_is_all_zero = BT_PRIVT_TRUE;

    for (i = 0; i <= str_out_len; i++) {
        if (i >= str_in_len) {
            adv_data_is_found = BT_PRIVT_TRUE;
            break;
        }

        adv_data[i] = str_in[i];
        if (str_in[i] == '\0') {
            adv_data_is_found = BT_PRIVT_TRUE;
            break;
        } else {
            data_is_all_zero = BT_PRIVT_FALSE;
        }
    }

    if (data_is_all_zero == BT_PRIVT_TRUE) {
        adv_data_is_found = BT_PRIVT_FALSE;
    }

    adv_data[i] = '\0';
    return adv_data_is_found;
}

int bt_at_cmd_parse_ttm_add(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_ADD) - 1;
    char adv_data[BT_AT_CMD_TTM_ADV_DATA_MAX_LEN + 1] = {0};
    int in_len;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if (bt_at_cmd_parse_ttm_adv_data_is_good((&str_in[str_in_index]), in_len, adv_data, BT_AT_CMD_TTM_ADV_DATA_MAX_LEN) == BT_PRIVT_TRUE) {
        return bt_at_cmd_sdk_adp_set_adv_data(adv_data);
    }

    return BT_PRIVT_ERROR;
}

unsigned int bt_at_cmd_parse_ttm_product_id_is_good(const char *str_in, int str_in_len, unsigned char *product_id, int str_out_len)
{
    unsigned int i;
    unsigned int product_id_is_found = BT_PRIVT_FALSE;

    if (str_in_len < BT_AT_CMD_TTM_PRODUCT_ID_LEN) {
        return product_id_is_found;
    }

    for (i = 0; i <= str_out_len; i++) {
        if (i >= str_in_len) {
            break;
        }

        product_id[i] = str_in[i];
    }

    if (str_in[BT_AT_CMD_TTM_PRODUCT_ID_LEN] <= BT_AT_CMD_TTM_END_CHAR) {
        product_id_is_found = BT_PRIVT_TRUE;
    }

    return product_id_is_found;
}

int bt_at_cmd_parse_ttm_pid(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_PID) - 1;
    int in_len;
    unsigned char product_id[BT_AT_CMD_TTM_PRODUCT_ID_LEN] = {0};

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if (bt_at_cmd_parse_ttm_product_id_is_good((&str_in[str_in_index]), in_len, product_id, BT_AT_CMD_TTM_PRODUCT_ID_LEN) == BT_PRIVT_TRUE) {
        return bt_at_cmd_sdk_adp_set_product_id(product_id, BT_AT_CMD_TTM_PRODUCT_ID_LEN);
    }
    return BT_PRIVT_ERROR;
}

unsigned int bt_at_cmd_parse_ttm_tx_power_is_good(const char *str_in, int str_in_len, int *tx_power)
{
    int power = 0;
    int sign_of_power = 0;
    unsigned int i;
    unsigned int tx_power_is_found = BT_PRIVT_FALSE;

    if ((str_in[0] != '(') || (str_in[1] == ')')) {
        return BT_PRIVT_FALSE;
    }

    if (str_in[1] == '-') {
        i = 2;
        sign_of_power = -1;
    } else if (str_in[1] == '+') {
        i = 2;
        sign_of_power = 1;
    } else {
        i = 1;
        sign_of_power = 1;
    }

    for (; i < 6; i++) { // (-23) take 5 + 1 = 6 DEC bit
        if (i >= str_in_len) {
            break;
        }

        if ((str_in[i] >= '0') && (str_in[i] <= '9')) {
            power = (power * 10) + (str_in[i] - '0');
        } else if (str_in[i] == ')') {
            tx_power_is_found = BT_PRIVT_TRUE;
            break;
        } else {
            break;
        }
    }

    if (tx_power_is_found == BT_PRIVT_TRUE) {
        power = power * sign_of_power;
        if ((power == 4) ||
            (power == 0) ||
            (power == -6) ||
            (power == -23)) {
            *tx_power = power;
            return BT_PRIVT_TRUE;
        }
    }

    return BT_PRIVT_FALSE;
}

int bt_at_cmd_parse_ttm_tpl(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_TPL) - 1;
    int in_len;
    int tx_power = 0;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if (bt_at_cmd_parse_ttm_tx_power_is_good((&str_in[str_in_index]), in_len, &tx_power) == BT_PRIVT_TRUE) {
        return bt_at_cmd_sdk_adp_set_tx_power(tx_power);
    }

    return BT_PRIVT_ERROR;
}

int bt_at_cmd_parse_ttm_eup(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int cmd_index;
    int str_in_index = sizeof(BT_AT_CMD_TTM_EUP) - 1;
    int in_len;
    int ret;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    for (cmd_index = 0; cmd_index < cmd_ttm_eup_num; cmd_index++) {
        if (bt_at_cmd_parse_is_match(&(str_in[str_in_index]), in_len, cmd_ttm_eup[cmd_index].cmd) &&
            (cmd_ttm_eup[cmd_index].fun != BT_PRIVT_NULL)) {
            ret = cmd_ttm_eup[cmd_index].fun(&(str_in[str_in_index]), in_len, str_out, str_out_len);
            return ret;
        }
    }

    return BT_PRIVT_ERROR;
}

int bt_at_cmd_parse_ttm_rsi(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int cmd_index;
    int str_in_index = sizeof(BT_AT_CMD_TTM_RSI) - 1;
    int in_len;
    int ret;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    for (cmd_index = 0; cmd_index < cmd_ttm_rsi_num; cmd_index++) {
        if (bt_at_cmd_parse_is_match(&(str_in[str_in_index]), in_len, cmd_ttm_rsi[cmd_index].cmd) &&
            (cmd_ttm_rsi[cmd_index].fun != BT_PRIVT_NULL)) {
            ret = cmd_ttm_rsi[cmd_index].fun(&(str_in[str_in_index]), in_len, str_out, str_out_len);
            return ret;
        }
    }
    return BT_PRIVT_ERROR;
}

unsigned int bt_at_cmd_parse_ttm_rtc_time_is_good(const char *str_in, int str_in_len, struct tm *time)
{
    struct tm time_tmp;
    int i;

    if (str_in_len < BT_AT_CMD_TTM_RTC_TIME_LEN) {
        return BT_PRIVT_FALSE;
    } else if (str_in[BT_AT_CMD_TTM_RTC_TIME_LEN] > BT_AT_CMD_TTM_END_CHAR) {
        return BT_PRIVT_FALSE;
    } else {
    }

    for (i = 0; i < BT_AT_CMD_TTM_RTC_TIME_LEN; i++) {
        if ((str_in[i] < '0') || (str_in[i] > '9')) {
            return BT_PRIVT_FALSE;
        }
    }

    time_tmp.tm_year =   (str_in[0] - '0') * 1000 + (str_in[1] - '0') * 100 + (str_in[2] - '0') * 10 + (str_in[3] - '0') - STRUCT_TM_YEAR_BASE_LINE;
    time_tmp.tm_mon =  (str_in[4] - '0') * 10 + (str_in[5] - '0') - STRUCT_TM_MONTH_BASE_LINE;
    time_tmp.tm_mday =    (str_in[6] - '0') * 10 + (str_in[7] - '0');
    time_tmp.tm_hour =   (str_in[8] - '0') * 10 + (str_in[9] - '0');
    time_tmp.tm_min = (str_in[10] - '0') * 10 + (str_in[11] - '0');
    time_tmp.tm_sec = (str_in[12] - '0') * 10 + (str_in[13] - '0');

    if ((time_tmp.tm_year < 9999) && (time_tmp.tm_year > 0) &&
        (time_tmp.tm_mon < 12) && (time_tmp.tm_mon >= 0) &&
        (time_tmp.tm_mday < 32) && (time_tmp.tm_mday > 0) &&
        (time_tmp.tm_hour < 24) && (time_tmp.tm_hour >= 0) &&
        (time_tmp.tm_min < 60) && (time_tmp.tm_min >= 0) &&
        (time_tmp.tm_sec < 60) && (time_tmp.tm_sec >= 0)) {
        *time = time_tmp;
        return BT_PRIVT_TRUE;
    }

    return BT_PRIVT_FALSE;
}

int bt_at_cmd_parse_ttm_rtc(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_index = sizeof(BT_AT_CMD_TTM_RTC) - 1;
    char rtc_time[20] = {0};
    struct tm time = {0};
    int in_len;
    int out_len;
    int ret;

    in_len = str_in_len - str_index;
    out_len = str_out_len - str_index;
    if ((in_len < 0) || (out_len < 0)) {
        return BT_PRIVT_ERROR;
    }

    if (str_in[str_index] == '?') {
        if ((in_len == 1) || (str_in[str_index + 1] <= BT_AT_CMD_TTM_END_CHAR)) {
            strncpy(str_out, BT_AT_CMD_TTM_RTC, str_out_len);
            ret = bt_at_cmd_sdk_adp_get_rtc_time(&time);
            sprintf(rtc_time, "%04u%02u%02u%02u%02u%02u\r\n\0",
                time.tm_year + STRUCT_TM_YEAR_BASE_LINE, time.tm_mon + STRUCT_TM_MONTH_BASE_LINE, time.tm_mday,
                time.tm_hour, time.tm_min, time.tm_sec);
            strncpy(&str_out[str_index], rtc_time, out_len);
            return ret;
        }
    } else if (bt_at_cmd_parse_ttm_rtc_time_is_good((&str_in[str_index]), in_len, &time) == BT_PRIVT_TRUE) {
        return bt_at_cmd_sdk_adp_set_rtc_time(&time);
    } else {
    }

    return BT_PRIVT_ERROR;
}

int bt_at_cmd_parse_ttm_cdl(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_CDL) - 1;
    int in_len;
    unsigned int com_out_delay = 0;
    int ret;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    ret = bt_at_cmd_parse_ttm_ms_str(&(str_in[str_in_index]), in_len, &com_out_delay);
    if (ret == BT_PRIVT_OK) {
        if ((com_out_delay == 0) ||
            (com_out_delay == 2) ||
            (com_out_delay == 5) ||
            (com_out_delay == 10) ||
            (com_out_delay == 15) ||
            (com_out_delay == 20) ||
            (com_out_delay == 25)) {
            // set delay between BCTS low level and com out
            ret = bt_at_cmd_sdk_adp_set_com_out_delay(com_out_delay);
        } else {
            ret = BT_PRIVT_ERROR;
        }
    }

    return ret;
}

int bt_at_cmd_parse_ttm_eup_on(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_EUP_ON) - 1;
    int in_len;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if ((in_len != 0) && (str_in[str_in_index] > BT_AT_CMD_TTM_END_CHAR)) {
        return BT_PRIVT_ERROR;
    }

    return bt_at_cmd_sdk_adp_set_en_up_enable(BT_PRIVT_ENABLE);
}

int bt_at_cmd_parse_ttm_eup_off(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_EUP_OFF) - 1;
    int in_len;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if ((in_len != 0) && (str_in[str_in_index] > BT_AT_CMD_TTM_END_CHAR)) {
        return BT_PRIVT_ERROR;
    }

    return bt_at_cmd_sdk_adp_set_en_up_enable(BT_PRIVT_DISNABLE);
}

int bt_at_cmd_parse_ttm_rsi_on(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_RSI_ON) - 1;
    int in_len;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if ((in_len != 0) && (str_in[str_in_index] > BT_AT_CMD_TTM_END_CHAR)) {
        return BT_PRIVT_ERROR;
    }

    return bt_at_cmd_sdk_adp_set_rssi_enable(BT_PRIVT_ENABLE);
}

int bt_at_cmd_parse_ttm_rsi_off(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int str_in_index = sizeof(BT_AT_CMD_TTM_RSI_OFF) - 1;
    int in_len;

    in_len = str_in_len - str_in_index;
    if (in_len < 0) {
        return BT_PRIVT_ERROR;
    }

    if ((in_len != 0) && (str_in[str_in_index] > BT_AT_CMD_TTM_END_CHAR)) {
        return BT_PRIVT_ERROR;
    }

    return bt_at_cmd_sdk_adp_set_rssi_enable(BT_PRIVT_DISNABLE);
}

void bt_at_cmd_parse_send_data_by_ble(const char *str_in, int str_in_len)
{
    if (str_in_len > GATT_CHARACTERISTIC_BLE_TO_TX_DATA_BUF_LEN) {
        str_in_len = GATT_CHARACTERISTIC_BLE_TO_TX_DATA_BUF_LEN;
    }

    memcpy(g_bt_bm_module_uart_rx_to_ble_data, str_in, str_in_len);
    g_bt_bm_module_uart_rx_to_ble_data[str_in_len] = '\0';
    g_bt_bm_module_uart_rx_to_ble_data_len = str_in_len;
    g_bt_bm_module_rx_to_ble_can_send_data = BT_PRIVT_ENABLE;

    btstack_push_user_msg(USER_MSG_ID_UART_RX_TO_BLE_DATA_SERVICE, BT_PRIVT_NULL, 0);

    return;
}

int bt_at_cmd_parse(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int cmd_index;

    if ((str_in == BT_PRIVT_NULL) || (str_out == BT_PRIVT_NULL)) {
        return BT_PRIVT_ERROR;
    }

    if ((str_in_len < 0) || (str_out_len < 0)) {
        return BT_PRIVT_ERROR;
    }

    dbg_printf("input cmd: %s len: %d\r\n", str_in, str_in_len);
    for (cmd_index = 0; cmd_index < cmd_root_num; cmd_index++) {
        if (bt_at_cmd_parse_is_match(str_in, str_in_len, cmd_root[cmd_index].cmd) &&
            (cmd_root[cmd_index].fun != BT_PRIVT_NULL)) {
            cmd_root[cmd_index].fun(str_in, str_in_len, str_out, str_out_len);
            dbg_printf("output: %s\r\n", str_out);
            return BT_PRIVT_OK;
        }
    }

    bt_at_cmd_parse_send_data_by_ble(str_in, str_in_len);

    strncpy(str_out, BT_AT_CMD_TTM_PARSE_NO_CMD, str_out_len);
    dbg_printf("output: %s\r\n", str_out);
    return BT_PRIVT_ERROR;
}

#if defined __cplusplus
    }
#endif

