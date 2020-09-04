
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

static bt_at_cmd_parse_fun_t *cmd_root = BT_PRIVT_NULL;
static int cmd_root_num = 0;
static pfun_bt_cmd_data_uart_data_process_t bt_cmd_data_uart_data_process_fun = BT_PRIVT_NULL;


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

void bt_cmd_data_uart_data_process(const char *str_in, int str_in_len)
{
    if (bt_cmd_data_uart_data_process_fun == BT_PRIVT_NULL) {
        return;
    }

    bt_cmd_data_uart_data_process_fun(str_in, str_in_len);
    dbg_printf("uart data process\r\n");

    return;
}

int bt_cmd_data_uart_cmd_parse(const char *str_in, int str_in_len, char *str_out, int str_out_len)
{
    int cmd_index;

    if ((str_in == BT_PRIVT_NULL) || (str_out == BT_PRIVT_NULL) || (cmd_root == BT_PRIVT_NULL)) {
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

int bt_cmd_data_uart_cmd_parse_register(bt_at_cmd_parse_fun_t *cmd, int cmd_num, pfun_bt_cmd_data_uart_data_process_t fun)
{
    if (cmd == BT_PRIVT_NULL || cmd_num == 0) {
        return BT_PRIVT_ERROR;
    }

    cmd_root = cmd;
    cmd_root_num = cmd_num;
    bt_cmd_data_uart_data_process_fun = fun;

    return BT_PRIVT_OK;
}

#if defined __cplusplus
    }
#endif

