
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>

#include "btstack_event.h"

#include "project_common.h"
#include "bt_cmd_parse.h"

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
    {BT_AT_CMD_TTM, bt_at_cmd_parse_temp},
};
static int cmd_root_num = sizeof(cmd_root) / sizeof(cmd_root[0]);

static bt_at_cmd_parse_fun_t cmd_ttm[] = {
    {BT_AT_CMD_TTM_CIT, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_NAM, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_REN, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_BPS, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_MAC, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_RST, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_ADP, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_ADD, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_PID, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_TPL, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_EUP, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_RSI, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_RTC, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_CDL, bt_at_cmd_parse_temp},
};
static int cmd_ttm_num = sizeof(cmd_ttm) / sizeof(cmd_ttm[0]);

static bt_at_cmd_parse_fun_t cmd_ttm_eup[] = {
    {BT_AT_CMD_TTM_EUP_ON, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_EUP_OFF, bt_at_cmd_parse_temp},
};
static int cmd_ttm_eup_num = sizeof(cmd_ttm_eup) / sizeof(cmd_ttm_eup[0]);

static bt_at_cmd_parse_fun_t cmd_ttm_rsi[] = {
    {BT_AT_CMD_TTM_RSI_ON, bt_at_cmd_parse_temp},
    {BT_AT_CMD_TTM_RSI_OFF, bt_at_cmd_parse_temp},
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

int bt_at_cmd_parse_temp(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    return BT_PRIVT_OK;
}

void bt_cmd_data_uart_data_process(const char *str_in, int str_in_len)
{
    return;
}

int bt_cmd_data_uart_cmd_parse(const char *str_in, int str_in_len, char *str_out, int str_out_len)
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

    bt_cmd_data_uart_data_process(str_in, str_in_len);

    strncpy(str_out, BT_AT_CMD_TTM_PARSE_NO_CMD, str_out_len);
    dbg_printf("output: %s\r\n", str_out);
    return BT_PRIVT_ERROR;
}

#if defined __cplusplus
    }
#endif

