
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>
#include "../bt_at_cmd_parse/bt_at_cmd.h"

#if defined __cplusplus
    extern "C" {
#endif

typedef struct {
    char *input;
    char *output;
} bt_at_cmd_parse_test_case_t;

static bt_at_cmd_parse_test_case_t test_case[] = {
    {"TTM:RST-SYSTESETT", "TTM:ERP\r\n\0"},
    {"TTM:MAC-123456789ab\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:RST-SYSTEMRESET", "TTM:OK\r\n\0"},
    {"TTM:RST-SYSTEMRESET\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-20ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-30ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-50ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-100ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-200ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-300ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-400ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-500ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-1000ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-1500ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-2000ms", "TTM:OK\r\n\0"},
    {"TTM:CIT-2500ms", "TTM:ERP\r\n\0"},
    {"TTM:NAM-?", "TTM:NAM-my_module\r\n\0"},
    {"TTM:NAM-??", "TTM:ERP\r\n\0"},
    {"TTM:REN-0123456789abcde", "TTM:OK\r\n\0"},
    {"TTM:REN-0123456789abcdef", "TTM:ERP\r\n\0"},
    {"TTM:BPS-?", "TTM:BPS-115200\r\n\0"},
    {"TTM:BPS-??", "TTM:ERP\r\n\0"},
    {"TTM:BPS-4800", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-9600", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-19200", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-38400", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-57600", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-115200", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-1152000", "TTM:ERP\r\n\0"},
    {"TTM:MAC-?", "TTM:MAC-0123456789AB\r\n\0"},
    {"TTM:MAC-??", "TTM:ERP\r\n\0"},
    {"TTM:MAC-123456789abc", "TTM:OK\r\n\0"},
    {"TTM:MAC-123456789ab", "TTM:ERP\r\n\0"},
    {"TTM:MAC-123456789ab\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:MAC-123456789abcd", "TTM:ERP\r\n\0"},
    {"TTM:MAC-123456789ABC", "TTM:OK\r\n\0"},
    {"TTM:MAC-123456789AB", "TTM:ERP\r\n\0"},
    {"TTM:MAC-123456789AB\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:MAC-123456789ABCD", "TTM:ERP\r\n\0"},
    {"TTM:RST-SYSTEMRESET", "TTM:OK\r\n\0"},
    {"TTM:RST-SYSTEMRESET\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:RST-SYSTEMRESETT", "TTM:ERP\r\n\0"},
    {"TTM:RST-SYSTEMRESTT", "TTM:ERP\r\n\0"},
    {"TTM:ADP-2", "TTM:OK\r\n\0"},
    {"TTM:ADP-5", "TTM:OK\r\n\0"},
    {"TTM:ADP-10", "TTM:OK\r\n\0"},
    {"TTM:ADP-15", "TTM:OK\r\n\0"},
    {"TTM:ADP-20", "TTM:OK\r\n\0"},
    {"TTM:ADP-25", "TTM:OK\r\n\0"},
    {"TTM:ADP-30", "TTM:OK\r\n\0"},
    {"TTM:ADP-40", "TTM:OK\r\n\0"},
    {"TTM:ADP-50", "TTM:OK\r\n\0"},
    {"TTM:ADP-500", "TTM:ERP\r\n\0"},
    {"TTM:ADD-0123456789abcdef", "TTM:OK\r\n\0"},
    {"TTM:ADD-0123456789abcdef0", "TTM:ERP\r\n\0"},
    {"TTM:PID-01", "TTM:OK\r\n\0"},
    {"TTM:PID-012", "TTM:ERP\r\n\0"},
    {"TTM:TPL-(+4)", "TTM:OK\r\n\0"},
    {"TTM:TPL-(0)", "TTM:OK\r\n\0"},
    {"TTM:TPL-(-6)", "TTM:OK\r\n\0"},
    {"TTM:TPL-(-23)", "TTM:OK\r\n\0"},
    {"TTM:TPL-(-230)", "TTM:ERP\r\n\0"},
    {"TTM:EUP-ON", "TTM:OK\r\n\0"},
    {"TTM:EUP-OFF", "TTM:OK\r\n\0"},
    {"TTM:EUP-ONF", "TTM:ERP\r\n\0"},
    {"TTM:EUP-OFFN", "TTM:ERP\r\n\0"},
    {"TTM:RSI-ON", "TTM:OK\r\n\0"},
    {"TTM:RSI-OFF", "TTM:OK\r\n\0"},
    {"TTM:RSI-ONF", "TTM:ERP\r\n\0"},
    {"TTM:RSI-OFFN", "TTM:ERP\r\n\0"},
    {"TTM:RTC-?", "TTM:RTC-20200102030405\r\n\0"},
    {"TTM:RTC-??", "TTM:ERP\r\n\0"},
    {"TTM:RTC-20200102030405", "TTM:OK\r\n\0"},
    {"TTM:RTC-2a200102030405", "TTM:ERP\r\n\0"},
    {"TTM:RTC-2020010203040506", "TTM:ERP\r\n\0"},
    {"TTM:CDL-0ms", "TTM:OK\r\n\0"},
    {"TTM:CDL-2ms", "TTM:OK\r\n\0"},
    {"TTM:CDL-5ms", "TTM:OK\r\n\0"},
    {"TTM:CDL-10ms", "TTM:OK\r\n\0"},
    {"TTM:CDL-15ms", "TTM:OK\r\n\0"},
    {"TTM:CDL-20ms", "TTM:OK\r\n\0"},
    {"TTM:CDL-25ms", "TTM:OK\r\n\0"},
    {"TTM:CDL-250ms", "TTM:ERP\r\n\0"},
    {"TTM:CIT-20ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-30ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-50ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-100ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-200ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-300ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-400ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-500ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-1000ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-1500ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-2000ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CIT-2500ms\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:NAM-?\r\n\0", "TTM:NAM-my_module\r\n\0"},
    {"TTM:NAM-??\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:REN-0123456789abcde\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:REN-0123456789abcdef\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:BPS-?\r\n\0", "TTM:BPS-115200\r\n\0"},
    {"TTM:BPS-??\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:BPS-4800\r\n\0", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-9600\r\n\0", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-19200\r\n\0", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-38400\r\n\0", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-57600\r\n\0", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-115200\r\n\0", "TTM:BPS SET AFTER 2S...\r\n\0"},
    {"TTM:BPS-1152000\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:MAC-?\r\n\0", "TTM:MAC-0123456789AB\r\n\0"},
    {"TTM:MAC-??\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:MAC-123456789abc\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:MAC-123456789abcd\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:RST-SYSTEMRESET\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-2\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-5\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-10\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-15\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-20\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-25\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-30\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-40\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-50\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADP-500\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:ADD-0123456789abcdef\0\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:ADD-0123456789abcdef0\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:PID-01\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:PID-012\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:TPL-(+4)\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:TPL-(0)\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:TPL-(-6)\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:TPL-(-23)\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:TPL-(-230)\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:EUP-ON\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:EUP-OFF\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:EUP-ONF\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:RSI-ON\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:RSI-OFF\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:RSI-ONF\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:RTC-?\r\n\0", "TTM:RTC-20200102030405\r\n\0"},
    {"TTM:RTC-??\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:RTC-20200102030405\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:RTC-2020010203040506\r\n\0", "TTM:ERP\r\n\0"},
    {"TTM:CDL-0ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CDL-2ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CDL-5ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CDL-10ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CDL-15ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CDL-20ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CDL-25ms\r\n\0", "TTM:OK\r\n\0"},
    {"TTM:CDL-250ms\r\n\0", "TTM:ERP\r\n\0"},
};
static int test_case_num = sizeof(test_case) / sizeof(test_case[0]);

static int sdk_gpio_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    printf("hello888888888888888888888888888888888888888888888888888\r\n");
    return 0;
}

static bt_at_cmd_sdk_hook_fun_t sdk_cmd_adp[] = {
    {BT_AT_CMD_SDK_GPIO_READ,         sdk_gpio_read},
    {BT_AT_CMD_SDK_GPIO_WRITE,        sdk_gpio_read},

    {BT_AT_CMD_SDK_UART_READ,         sdk_gpio_read},
    {BT_AT_CMD_SDK_UART_WRITE,        sdk_gpio_read},

    {BT_AT_CMD_SDK_IIC_READ,          sdk_gpio_read},
    {BT_AT_CMD_SDK_IIC_WRITE,         sdk_gpio_read},

    {BT_AT_CMD_SDK_SPI_READ,          sdk_gpio_read},
    {BT_AT_CMD_SDK_SPI_WRITE,         sdk_gpio_read},

    {BT_AT_CMD_SDK_TIMER_READ,        sdk_gpio_read},
    {BT_AT_CMD_SDK_TIMER_WRITE,       sdk_gpio_read},

    {BT_AT_CMD_SDK_PWM_READ,          sdk_gpio_read},
    {BT_AT_CMD_SDK_PWM_WRITE,         sdk_gpio_read},

    {BT_AT_CMD_SDK_ADC_READ,          sdk_gpio_read},
    {BT_AT_CMD_SDK_ADC_WRITE,         sdk_gpio_read},

    {BT_AT_CMD_SDK_RTC_READ,          sdk_gpio_read},
    {BT_AT_CMD_SDK_RTC_WRITE,         sdk_gpio_read},

    {BT_AT_CMD_SDK_MODULE_NAME_READ,  sdk_gpio_read},
    {BT_AT_CMD_SDK_MODULE_NAME_WRITE, sdk_gpio_read},

    {BT_AT_CMD_SDK_BAUD_RATE_READ,    sdk_gpio_read},
    {BT_AT_CMD_SDK_BAUD_RATE_WRITE,   sdk_gpio_read},

    {BT_AT_CMD_SDK_MAC_ADD_READ,      sdk_gpio_read},
    {BT_AT_CMD_SDK_MAC_ADD_WRITE,     sdk_gpio_read},

    {BT_AT_CMD_SDK_CON_INTV_WRITE,    sdk_gpio_read},

    {BT_AT_CMD_SDK_MODULE_RESET,      sdk_gpio_read},

    {BT_AT_CMD_SDK_ADV_PERIOD_WRITE,  sdk_gpio_read},
    {BT_AT_CMD_SDK_ADV_DATA_WRITE,    sdk_gpio_read},

    {BT_AT_CMD_SDK_PRODUCT_ID_WRITE,  sdk_gpio_read},

    {BT_AT_CMD_SDK_TX_POW_WRITE,      sdk_gpio_read},

    {BT_AT_CMD_SDK_COM_DELAY_WRITE,   sdk_gpio_read},

    {BT_AT_CMD_SDK_EN_UP_WRITE,       sdk_gpio_read},

    {BT_AT_CMD_SDK_RSSI_EN_WRITE,     sdk_gpio_read},
};
static int sdk_cmd_adp_num = sizeof(sdk_cmd_adp) / sizeof(sdk_cmd_adp[0]);

int bt_at_cmd_parse_test_case()
{
    int test_case_index;
    char out[64] = {0};
    int ret = 0;
    char *in;
    int in_len;

    bt_at_cmd_sdk_adp_register_hook(&sdk_cmd_adp, sdk_cmd_adp_num);

    for (test_case_index = 0; test_case_index < test_case_num; test_case_index++) {
        in = test_case[test_case_index].input;
        in_len = strlen(in);
        bt_at_cmd_parse(in, in_len, out, sizeof(out));
        if (strcmp(test_case[test_case_index].output, out) == 0) {
        } else {
            ret = 1;
            printf("num %d test case %s failed, out %s.\r\n", test_case_index, test_case[test_case_index].input, out);
        }
    }

    if (ret == 0) {
        printf("test pass.");
    }
    return ret;
}

#if defined __cplusplus
    }
#endif

