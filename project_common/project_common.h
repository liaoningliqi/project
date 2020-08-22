
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __PROJECT_COMMON_H__
#define __PROJECT_COMMON_H__

#if defined __cplusplus
    extern "C" {
#endif

#define DEBUG 1
#define UART_PORT 0

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

#define CMD_IO_PORT_BUF_LEN 80

#define BT_PRIVT_OK 0
#define BT_PRIVT_ERROR -1
#define BT_PRIVT_NO_CMD -2
#define BT_PRIVT_TIMEOUT -3

#define BT_PRIVT_TRUE 1
#define BT_PRIVT_FALSE 0

#define BT_PRIVT_ENABLE 1
#define BT_PRIVT_DISNABLE 0

#define BT_PRIVT_NULL 0

#define INVALID_HANDLE (0xffff)

typedef int (*pfunbt_at_cmd_parse)(const char *str_in, int str_in_len, char *str_out, int str_out_len);
typedef int (*pfun_bt_cmd_data_uart_data_process_t)(const char *str_in, int str_in_len);

typedef struct {
    char *cmd;
    pfunbt_at_cmd_parse fun;
} bt_at_cmd_parse_fun_t;

void bt_cmd_data_uart_io_init(void);
unsigned int bt_at_cmd_parse_is_match(const char *str_in, int str_in_len, char *str_in_cmd);
int bt_cmd_data_uart_out_string_with_end_char(const char *data);
void bt_cmd_data_uart_out_data(const char *data, int data_len);
int bt_cmd_data_uart_cmd_parse_register(bt_at_cmd_parse_fun_t *cmd, int cmd_num, pfun_bt_cmd_data_uart_data_process_t fun);

#if defined __cplusplus
    }
#endif

#endif

