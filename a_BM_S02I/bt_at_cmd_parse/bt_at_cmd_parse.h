
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BT_AT_CMD_PARSE_H__
#define __BT_AT_CMD_PARSE_H__

#if defined __cplusplus
    extern "C" {
#endif

#define BT_PRIVT_OK 0
#define BT_PRIVT_ERROR -1
#define BT_PRIVT_NO_CMD -2
#define BT_PRIVT_TIMEOUT -3


#define BT_PRIVT_TRUE 1
#define BT_PRIVT_FALSE 0

#define BT_PRIVT_ENABLE 1
#define BT_PRIVT_DISNABLE 0

#define BT_PRIVT_NULL 0

#define BT_AT_CMD_TTM_PARSE_OK "OK\r\n\0"
#define BT_AT_CMD_TTM_PARSE_ERP "ERP\r\n\0"
#define BT_AT_CMD_TTM_PARSE_NO_CMD "NOCMD\r\n\0"
#define BT_AT_CMD_TTM_PARSE_TIMEOUT "TIMEOUT\r\n\0"

#define BT_AT_CMD_TTM_END_CHAR '\r'

int bt_at_cmd_parse_ttm(const char *str_in, int str_in_len, char *str_out, int str_out_len);

int bt_at_cmd_parse_ttm_cit(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_nam(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_ren(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_bps(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_mac(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_rst(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_adp(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_add(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_pid(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_tpl(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_eup(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_rsi(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_rtc(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_cdl(const char *str_in, int str_in_len, char *str_out, int str_out_len);

int bt_at_cmd_parse_ttm_eup_on(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_eup_off(const char *str_in, int str_in_len, char *str_out, int str_out_len);

int bt_at_cmd_parse_ttm_rsi_on(const char *str_in, int str_in_len, char *str_out, int str_out_len);
int bt_at_cmd_parse_ttm_rsi_off(const char *str_in, int str_in_len, char *str_out, int str_out_len);

#if defined __cplusplus
    }
#endif

#endif

