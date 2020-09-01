
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BT_CMD_PARSE_H__
#define __BT_CMD_PARSE_H__

#if defined __cplusplus
    extern "C" {
#endif

#define BT_AT_CMD_TTM_PARSE_OK "OK\r\n\0"
#define BT_AT_CMD_TTM_PARSE_ERP "ERP\r\n\0"
#define BT_AT_CMD_TTM_PARSE_NO_CMD "NOCMD\r\n\0"
#define BT_AT_CMD_TTM_PARSE_TIMEOUT "TIMEOUT\r\n\0"

#define BT_AT_CMD_TTM_END_CHAR '\r'

int bt_at_cmd_parse_temp(const char *str_in, int str_in_len, char *str_out, int str_out_len);

int bt_cmd_data_uart_cmd_parse(const char *str_in, int str_in_len, char *str_out, int str_out_len);
void bt_cmd_data_uart_data_process(const char *str_in, int str_in_len);

#if defined __cplusplus
    }
#endif

#endif

