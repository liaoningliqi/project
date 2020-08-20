
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "btstack_event.h"
#include "platform_api.h"

#include "iic.h"
#include "bme280.h"
#include "peripheral_adc.h"
#include "eflash.h"
#include "peripheral_rtc.h"

#include "../../project_common/project_common.h"
#include "../../bt_at_cmd_parse/bt_at_cmd.h"
#include "sdk.h"

#if defined __cplusplus
    extern "C" {
#endif

#define PIN_SDI   GIO_GPIO_4

#define PWM1_GPIO GIO_GPIO_1
#define PWM1_CHANNEL 0
#define PWM2_GPIO GIO_GPIO_0
#define PWM2_CHANNEL 2
#define PWM3_GPIO GIO_GPIO_3
#define PWM3_CHANNEL 4

#define GPIO_O6 GIO_GPIO_10
#define GPIO_O7 GIO_GPIO_11
#define GPIO_LED_ON 0
#define GPIO_LED_OFF 1

#define GPIO_IO0 GIO_GPIO_13
#define GPIO_IO5 GIO_GPIO_9

#define USER_UART0_IO_TX GIO_GPIO_2
#define USER_UART0_IO_RX GIO_GPIO_3
#define USER_UART1_IO_TX GIO_GPIO_7
#define USER_UART1_IO_RX GIO_GPIO_8
#define USER_UART1_IO_RTS GIO_GPIO_2
#define USER_UART1_IO_CTS GIO_GPIO_16

#define BATTERY_MAX_VOLT 1023
#define BATTERY_MIN_VOLT 800

#define PERA_THRESHOLD (OSC_CLK_FREQ / 1000)
#define TO_PERCENT(v) (((uint32_t)(v) * 100) >> 8)

#define PRIVATE_FLASH_DATA_START_ADD 0x00042000
#define PRIVATE_FLASH_DATA_END_ADD 0x00043FFF
#define PRIVATE_FlASH_DATA_IS_INIT 0xA55A0001

#define PRIVATE_FlASH_INIT_DEV_NAME "Tv231u-mac-addr"

typedef struct {
    uint32_t data_init_flag;
    uint32_t data_len;
    char module_name[BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN + 1];
    uint32_t module_uart_baud_rate;
    uint8_t module_mac_address[BT_AT_CMD_TTM_MAC_ADDRESS_LEN];
    uint32_t module_adv_period;
    private_module_adv_data_t module_adv_data;
    uint8_t module_product_id[2];
} private_flash_data_t;

typedef union {
    uint8_t whole_byte;
    struct {
        uint8_t bit0:1;
        uint8_t bit1:1;
        uint8_t bit2:1;
        uint8_t bit3:1;
        uint8_t bit4:1;
        uint8_t bit5:1;
        uint8_t bit6:1;
        uint8_t bit7:1;
    }bits;
} gatt_service_gpio_t;

static private_flash_data_t *p_power_off_save_data_in_flash = (private_flash_data_t *)PRIVATE_FLASH_DATA_START_ADD;
static private_flash_data_t power_off_save_data_in_ram = {
    .data_init_flag = PRIVATE_FlASH_DATA_IS_INIT,
    .data_len = sizeof(private_flash_data_t),
    .module_name = PRIVATE_FlASH_INIT_DEV_NAME,
    .module_uart_baud_rate = 115200,
    .module_mac_address = {0xCD, 0xA3, 0x28, 0x12, 0x01, 0x3f},
    .module_adv_period = 2,
    .module_adv_data = {
        .flags = {2, 0x01, 0x06},
        .local_name_len = BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN + 1,
        .local_name_handle = 0x09,
        .local_name = PRIVATE_FlASH_INIT_DEV_NAME,
    },
    .module_product_id = {0xCD, 0xA3},
};

static time_t rtc_time_in_second_base = 0;

uint8_t *g_rand_mac_addres = power_off_save_data_in_ram.module_mac_address;
private_module_adv_data_t *g_module_adv_data = &(power_off_save_data_in_ram.module_adv_data);

int sdk_uart_should_updata_baud_rate = 0;

uint16_t g_hci_le_conn_complete_slave_handle = 0;
static unsigned int sdk_rssi_1s_read_updata_enable = 0;
static int sdk_gpio_isr_cnt = 0;

#ifdef ING_CHIPS_PRIVATE_SERVICE
// CPU clok: PLL_CLK_FREQ  48000000
// 1 cycle = 21ns
// 48 cycles per us
// Tcycle = 2us --> ~100 cycles
static void sdk_delay(int cycles)
{
    int i;
    for (i = 0; i < cycles; i++) {
        __nop();
    }
    return;
}

#define pulse()                     \
    { GIO_WriteValue(PIN_SDI, 1);   \
    sdk_delay(1);                       \
    GIO_WriteValue(PIN_SDI, 0); } while (0)

static void sdk_tlc59731_write(uint32_t value)
{
    int8_t i;

    for( i = 0; i < 32; i++ ) {
        uint32_t bit = value & ( 0x80000000 >> i );
        pulse();

        if (bit) {
            sdk_delay(10);
            pulse();
            sdk_delay(78);
        } else {
            sdk_delay(90);
        }
    }
    sdk_delay(100 * 8);
    return;
}

static void sdk_set_led_color(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t cmd = (0x3a << 24) | (b << 16) | (r << 8) | g;
    sdk_tlc59731_write(cmd);
    return;
}

static void sdk_setup_rgb_led()
{
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_GPIO) | (1 << SYSCTRL_ClkGate_APB_PWM));
    PINCTRL_SetPadMux(PIN_SDI, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(PIN_SDI, 0);
    GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
    GIO_WriteValue(PIN_SDI, 0);

    sdk_set_led_color(0, 0, 0);
    return;
}

static void sdk_setup_iic_temper()
{
    static struct bme280_t bme280_data;

    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_I2C0));
    PINCTRL_SetPadMux(6, IO_SOURCE_GENERAL);
    PINCTRL_SetPadMux(7, IO_SOURCE_GENERAL);
    PINCTRL_SetPadMux(14, IO_SOURCE_I2C0_SCL_O);
    PINCTRL_SetPadMux(15, IO_SOURCE_I2C0_SDO);
    PINCTRL_SelI2cSclIn(I2C_PORT_0, 14);
    i2c_init(I2C_PORT_0);

    dbg_printf("sensor init...\r\n");
    if (bme280_init(&bme280_data)==0) {
        dbg_printf("failed\r\n");
    } else {
        dbg_printf("OK\r\n");
    }

    return;
}

static int sdk_rgb_light_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    uint8_t *rgb = (uint8_t *)para_in;

    if (para_in_len != 3) {
        return -1;
    }

    sdk_set_led_color(rgb[0], rgb[1], rgb[2]);

    return 0;
}

static int sdk_temperature_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    int32_t *temperature = (int32_t *)data_out;

    if (data_out_len != sizeof(*temperature)) {
        return -1;
    }

    *temperature = bme280_compensate_temperature_read() + 1800;

    return 0;
}
#endif

static void sdk_setup_adc()
{
    ADC_PowerCtrl(1);
    ADC_Reset();
    ADC_SetClkSel(ADC_CLK_EN | ADC_CLK_16);
    ADC_SetMode(ADC_MODE_LOOP);
    ADC_EnableChannel(BATTERY_ADC_CHANNEL, 1);
    ADC_EnableChannel(BT_BM_SDK_ADC0_CHANNEL, 1);
    ADC_EnableChannel(BT_BM_SDK_ADC1_CHANNEL, 1);
    ADC_Enable(1);
    return;
}

static void sdk_setup_channel(uint8_t channel_index)
{
    PWM_HaltCtrlEnable(channel_index, 1);
    PWM_Enable(channel_index, 0);
    PWM_SetPeraThreshold(channel_index, PERA_THRESHOLD);
    PWM_SetMultiDutyCycleCtrl(channel_index, 0);        // do not use multi duty cycles
    PWM_SetHighThreshold(channel_index, 0, PERA_THRESHOLD / 2);
    PWM_SetMode(channel_index, PWM_WORK_MODE_UP_WITHOUT_DIED_ZONE);
    PWM_SetMask(channel_index, 0, 0);
    PWM_Enable(channel_index, 1);
    PWM_HaltCtrlEnable(channel_index, 0);

    return;
}

static void set_pin_pwm_width(const uint8_t channel_index, const uint8_t high_width)
{
    PWM_SetHighThreshold(channel_index, 0, PERA_THRESHOLD / 100 * TO_PERCENT(high_width));
    return;
}

static uint32_t sdk_gpio_isr(void *user_data)
{
    sdk_gpio_isr_cnt++;
    bt_bm_module_gpio_in_service_send_msg();

    GIO_ClearAllIntStatus();
    return 0;
}

static void sdk_setup_gpio()
{
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_GPIO) | (1 << SYSCTRL_ClkGate_APB_PWM) | (1 << SYSCTRL_ClkGate_APB_PinCtrl));

    PINCTRL_SetPadMux(PWM1_GPIO, IO_SOURCE_GENERAL);
    PINCTRL_SetGeneralPadMode(PWM1_GPIO, IO_MODE_PWM, PWM1_CHANNEL, 0);
    sdk_setup_channel(PWM1_CHANNEL);
    PINCTRL_SetPadMux(PWM2_GPIO, IO_SOURCE_GENERAL);
    PINCTRL_SetGeneralPadMode(PWM2_GPIO, IO_MODE_PWM, PWM2_CHANNEL, 0);
    sdk_setup_channel(PWM2_CHANNEL);
    PINCTRL_SetPadMux(PWM3_GPIO, IO_SOURCE_GENERAL);
    PINCTRL_SetGeneralPadMode(PWM3_GPIO, IO_MODE_PWM, PWM3_CHANNEL, 0);
    sdk_setup_channel(PWM3_CHANNEL);

    PINCTRL_SetPadMux(GPIO_IO0, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(GPIO_IO0, 0);
    GIO_SetPull(GPIO_IO0, 1, GIO_PULL_DOWN);
    GIO_SetDirection(GPIO_IO0, GIO_DIR_INPUT);

    PINCTRL_SetPadMux(GPIO_IO5, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(GPIO_IO5, 0);
    GIO_SetPull(GPIO_IO5, 1, GIO_PULL_DOWN);
    GIO_SetDirection(GPIO_IO5, GIO_DIR_INPUT);

    PINCTRL_SetPadMux(GPIO_O6, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(GPIO_O6, 0);
    GIO_SetPull(GPIO_O6, 1, GIO_PULL_DOWN);
    GIO_SetDirection(GPIO_O6, GIO_DIR_INPUT);

    PINCTRL_SetPadMux(GPIO_O7, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(GPIO_O7, 0);
    GIO_SetPull(GPIO_O7, 1, GIO_PULL_DOWN);
    GIO_SetDirection(GPIO_O7, GIO_DIR_INPUT);

    return;
}

static void sdk_config_uart(UART_TypeDef* port, uint32_t freq, uint32_t baud)
{
    UART_sStateStruct config;

    config.word_length       = UART_WLEN_8_BITS;
    config.parity            = UART_PARITY_NOT_CHECK;
    config.fifo_enable       = 1;
    config.two_stop_bits     = 0;
    config.receive_en        = 1;
    config.transmit_en       = 1;
    config.UART_en           = 1;
    config.cts_en            = 0;
    config.rts_en            = 0;
    config.rxfifo_waterlevel = 1;
    config.txfifo_waterlevel = 1;
    config.ClockFrequency    = freq;
    config.BaudRate          = baud;

    apUART_Initialize(port, &config, 1 << bsUART_RECEIVE_INTENAB);

    return;
}

static void sdk_config_uart_user()
{
#if (UART_PORT == 0)
    SYSCTRL_ClearClkGateMulti(1 << SYSCTRL_ClkGate_APB_UART0);
    PINCTRL_SetPadMux(USER_UART0_IO_RX, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(USER_UART0_IO_RX, 0);
    PINCTRL_SetPadMux(USER_UART0_IO_TX, IO_SOURCE_UART0_TXD);
    GIO_SetPull(USER_UART0_IO_RX, 1, GIO_PULL_UP);
    PINCTRL_SelUartRxdIn(UART_PORT_0, USER_UART0_IO_RX);
#else
    SYSCTRL_ClearClkGateMulti(1 << SYSCTRL_ClkGate_APB_UART1);
    PINCTRL_SetPadMux(USER_UART1_IO_RX, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(USER_UART1_IO_RX, 0);
    PINCTRL_SetPadMux(USER_UART1_IO_TX, IO_SOURCE_UART1_TXD);
    PINCTRL_SetPadMux(USER_UART1_IO_RTS, IO_SOURCE_UART1_RTS);
    GIO_SetPull(USER_UART1_IO_RX, 1, GIO_PULL_UP);
    PINCTRL_SelUartCtsIn(UART_PORT_1, USER_UART1_IO_CTS);
    PINCTRL_SelUartRxdIn(UART_PORT_1, USER_UART1_IO_RX);
#endif

    sdk_config_uart(USR_UART_IO_PORT, OSC_CLK_FREQ, power_off_save_data_in_ram.module_uart_baud_rate);

    return;
}

static void sdk_private_data_write_to_flash()
{
    program_flash((uint32_t)p_power_off_save_data_in_flash, (uint8_t *)(&power_off_save_data_in_ram), sizeof(power_off_save_data_in_ram));
    write_flash((uint32_t)p_power_off_save_data_in_flash, (uint8_t *)(&power_off_save_data_in_ram), sizeof(power_off_save_data_in_ram));
    return;
}

static void sdk_init_private_data()
{
    if ((p_power_off_save_data_in_flash->data_init_flag != power_off_save_data_in_ram.data_init_flag) ||
        (p_power_off_save_data_in_flash->data_len != power_off_save_data_in_ram.data_len)) {
        sdk_private_data_write_to_flash();
    } else {
        memcpy(&power_off_save_data_in_ram, p_power_off_save_data_in_flash, sizeof(power_off_save_data_in_ram));
    }
    sprintf((char *)power_off_save_data_in_ram.module_adv_data.local_name, "Tv231u-%02x%02x%02x%02x",
        power_off_save_data_in_ram.module_mac_address[BT_AT_CMD_TTM_MAC_ADDRESS_LEN - 4],
        power_off_save_data_in_ram.module_mac_address[BT_AT_CMD_TTM_MAC_ADDRESS_LEN - 3],
        power_off_save_data_in_ram.module_mac_address[BT_AT_CMD_TTM_MAC_ADDRESS_LEN - 2],
        power_off_save_data_in_ram.module_mac_address[BT_AT_CMD_TTM_MAC_ADDRESS_LEN - 1]);

    return;
}

static int sdk_temp(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    return 0;
}

static int sdk_tx_power_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    dbg_printf("tx power: %d\r\n", *((int *)para_in));

    return 0;
}

static int sdk_rtc_read_second(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    uint64_t *p_rtc_time = (uint64_t *)data_out;

    *p_rtc_time = rtc_time_in_second_base + RTC_CurrentFull() / RTC_CLK_FREQ;

    return 0;
}

static int sdk_rtc_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    int ret;
    struct tm *p_rtc_time = (struct tm *)data_out;
    time_t rtc_time_second;

    ret = sdk_rtc_read_second(0, 0, &rtc_time_second, sizeof(rtc_time_second));
    *p_rtc_time = *(localtime(&rtc_time_second));

    return ret;
}

static int sdk_rtc_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    uint64_t rtc_time_cnt = RTC_CurrentFull();
    struct tm rtc_time = *((struct tm *)para_in);

    rtc_time_in_second_base = mktime(&rtc_time);
    rtc_time_in_second_base = rtc_time_in_second_base - rtc_time_cnt / RTC_CLK_FREQ;

    return 0;
}

static int sdk_con_intv_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    int connection_interval = *((int *)para_in);

    btstack_push_user_msg(USER_MSG_ID_SET_INTERVAL, NULL, connection_interval);

    return 0;
}

static int sdk_rssi_en_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    unsigned int *en_flag = (unsigned int *)para_in;

    sdk_rssi_1s_read_updata_enable = *en_flag;

    return 0;
}

static int sdk_uart_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    char *uart_data = (char *)para_in;

    bt_cmd_data_uart_out_string_with_end_char(uart_data);

    return 0;
}

static int sdk_product_id_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    uint8_t *product_id = (uint8_t *)para_in;

    power_off_save_data_in_ram.module_product_id[0] = product_id[0];
    power_off_save_data_in_ram.module_product_id[1] = product_id[1];

    sdk_private_data_write_to_flash();

    return 0;
}

static int sdk_baud_rate_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    char baud_rate_str[16] = {0};

    sprintf(baud_rate_str, "%u\r\n\0", power_off_save_data_in_ram.module_uart_baud_rate);
    strncpy((char *)data_out, baud_rate_str, data_out_len);

    return 0;
}

static int sdk_baud_rate_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    uint32_t baud_rate = *((uint32_t *)para_in);

    power_off_save_data_in_ram.module_uart_baud_rate = baud_rate;
    sdk_private_data_write_to_flash();

    sdk_uart_should_updata_baud_rate = 2; // after 2s updata uart baud rate

    return 0;
}

static int sdk_module_reset(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    platform_reset();
    return 0;
}

static int sdk_mac_add_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    char mac_addres[BT_AT_CMD_TTM_MAC_ADDRESS_STR_LEN + 3] = {0};
    char *mac_address_out = (char *)data_out;

    sprintf(mac_addres, "%02x%02x%02x%02x%02x%02x\r\n\0",
        power_off_save_data_in_ram.module_mac_address[0],
        power_off_save_data_in_ram.module_mac_address[1],
        power_off_save_data_in_ram.module_mac_address[2],
        power_off_save_data_in_ram.module_mac_address[3],
        power_off_save_data_in_ram.module_mac_address[4],
        power_off_save_data_in_ram.module_mac_address[5]);
    mac_addres[BT_AT_CMD_TTM_MAC_ADDRESS_STR_LEN] = '\r';
    mac_addres[BT_AT_CMD_TTM_MAC_ADDRESS_STR_LEN + 1] = '\n';
    mac_addres[BT_AT_CMD_TTM_MAC_ADDRESS_STR_LEN + 2] = '\0';

    strncpy(mac_address_out, mac_addres, data_out_len);
    mac_address_out[data_out_len] = '\0';

    return 0;
}

static int sdk_mac_add_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    int i;
    int mac_add_len;
    uint8_t *mac_add_str_in = (uint8_t *)para_in;

    mac_add_len = para_in_len / 2;
    for (i = 0; (i < mac_add_len) && (i < BT_AT_CMD_TTM_MAC_ADDRESS_LEN); i++) {
        power_off_save_data_in_ram.module_mac_address[i] = ((mac_add_str_in[i * 2] << 4) | mac_add_str_in[i * 2 + 1]);
    }
    sdk_private_data_write_to_flash();
    return 0;
}

static int sdk_module_name_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    strncpy((char *)data_out, power_off_save_data_in_ram.module_name, data_out_len);
    return 0;
}

static int sdk_module_name_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    strncpy(power_off_save_data_in_ram.module_name, para_in, BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN);
    strncpy((char *)power_off_save_data_in_ram.module_adv_data.local_name, para_in, BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN);
    power_off_save_data_in_ram.module_name[BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN] = '\0';
    sdk_private_data_write_to_flash();
    return 0;
}

static int sdk_gpio_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    gatt_service_gpio_t read;
    uint8_t *read_data = (uint8_t *)data_out;

    if (data_out_len != sizeof(read)) {
        return -1;
    }

    read.bits.bit0 = GIO_ReadValue(GPIO_IO0);
    read.bits.bit1 = 0;
    read.bits.bit2 = 0;
    read.bits.bit3 = 0;
    read.bits.bit4 = 0;
    read.bits.bit5 = GIO_ReadValue(GPIO_IO5);
    read.bits.bit6 = 0;
    read.bits.bit7 = 0;

    *read_data = read.whole_byte;
    dbg_printf("sdk_gpio_read 0x%x\r\n", read.whole_byte);

    return 0;
}

static int sdk_gpio_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    gatt_service_gpio_t write;
    uint8_t *write_data = (uint8_t *)para_in;

    if (para_in_len != sizeof(write)) {
        return -1;
    }

    write.whole_byte = *write_data;

    dbg_printf("sdk_gpio_write 0x%x\r\n", write.whole_byte);
    g_bt_bm_module_gpio_out[0] = write.whole_byte;

    if (write.bits.bit0 == 0) {
        GIO_WriteValue(GPIO_IO0, 0);
    } else {
        GIO_WriteValue(GPIO_IO0, 1);
    }

    if (write.bits.bit5 == 0) {
        GIO_WriteValue(GPIO_IO5, 0);
    } else {
        GIO_WriteValue(GPIO_IO5, 1);
    }

    if (write.bits.bit6 == 0) {
        GIO_WriteValue(GPIO_O6, 0);
    } else {
        GIO_WriteValue(GPIO_O6, 1);
    }

    if (write.bits.bit7 == 0) {
        GIO_WriteValue(GPIO_O7, 0);
    } else {
        GIO_WriteValue(GPIO_O7, 1);
    }

    return 0;
}

static int sdk_gpio_config_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    return 0;
}

static int sdk_gpio_config_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    gatt_service_gpio_t config;
    uint8_t *config_data = (uint8_t *)para_in;

    config.whole_byte = *config_data;

    if (para_in_len != sizeof(*config_data)) {
        return -1;
    }

    dbg_printf("sdk_gpio_config_write 0x%x\r\n", config.whole_byte);
    g_bt_bm_module_gpio_config[0] = config.whole_byte;

    if (config.bits.bit0 == 1) { //out
        GIO_SetDirection(GPIO_IO0, GIO_DIR_OUTPUT);
        GIO_WriteValue(GPIO_IO0, (g_bt_bm_module_gpio_out[0] & 0X01));
    } else {
        GIO_SetPull(GPIO_IO0, 0, GIO_PULL_DOWN);
        GIO_SetDirection(GPIO_IO0, GIO_DIR_INPUT);
        GIO_ConfigIntSource(GPIO_IO0, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE | GIO_INT_EN_LOGIC_HIGH_OR_RISING_EDGE, GIO_INT_EDGE);
    }

    if (config.bits.bit5 == 1) { //out
        GIO_SetDirection(GPIO_IO5, GIO_DIR_OUTPUT);
        GIO_WriteValue(GPIO_IO5, ((g_bt_bm_module_gpio_out[0] >> 5) & 0X01));
    } else {
        GIO_SetPull(GPIO_IO5, 0, GIO_PULL_DOWN);
        GIO_SetDirection(GPIO_IO5, GIO_DIR_INPUT);
        GIO_ConfigIntSource(GPIO_IO5, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE | GIO_INT_EN_LOGIC_HIGH_OR_RISING_EDGE, GIO_INT_EDGE);
    }

    if (config.bits.bit6 == 1) { //out
        GIO_SetDirection(GPIO_O6, GIO_DIR_OUTPUT);
        GIO_WriteValue(GPIO_O6, ((g_bt_bm_module_gpio_out[0] >> 6) & 0X01));
    } else {
        GIO_SetDirection(GPIO_O6, GIO_DIR_INPUT);
    }

    if (config.bits.bit7 == 1) { //out
        GIO_SetDirection(GPIO_O7, GIO_DIR_OUTPUT);
        GIO_WriteValue(GPIO_O7, ((g_bt_bm_module_gpio_out[0] >> 7) & 0X01));
    } else {
        GIO_SetDirection(GPIO_O7, GIO_DIR_INPUT);
    }

    return 0;
}

static int sdk_pwm_write(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    uint8_t *pwm_width = (uint8_t *)para_in;

    if (para_in_len != GATT_CHARACTERISTIC_PWM_OUT_WIDTH_DATA_LEN) {
        return -1;
    }

    g_bt_bm_module_pwm_width[0] = pwm_width[0];
    g_bt_bm_module_pwm_width[1] = pwm_width[1];
    g_bt_bm_module_pwm_width[2] = pwm_width[2];

    dbg_printf("sdk_pwm_write %u %u %u\r\n", g_bt_bm_module_pwm_width[0], g_bt_bm_module_pwm_width[1], g_bt_bm_module_pwm_width[2]);

    set_pin_pwm_width(PWM1_CHANNEL, g_bt_bm_module_pwm_width[0]);
    set_pin_pwm_width(PWM2_CHANNEL, g_bt_bm_module_pwm_width[1]);
    set_pin_pwm_width(PWM3_CHANNEL, g_bt_bm_module_pwm_width[2]);
    return 0;
}

static int sdk_adc_read(const void *para_in, int para_in_len, void *data_out, int data_out_len)
{
    int adc = *((int *)para_in);
    uint16_t *value = (uint16_t *)data_out;
    uint16_t voltage;

    if ((para_in_len != sizeof(adc)) || (data_out_len != sizeof(*value))) {
        return -1;
    }

    if (adc == BATTERY_ADC_CHANNEL) {
        voltage = ADC_ReadChannelData(BATTERY_ADC_CHANNEL);
        if (voltage > BATTERY_MIN_VOLT) {
            *value = (uint32_t)(voltage - BATTERY_MIN_VOLT) * 100 / (BATTERY_MAX_VOLT - BATTERY_MIN_VOLT);
        } else {
            *value = 1;
        }
    } else if (adc == BT_BM_SDK_ADC0_CHANNEL) {
        voltage = ADC_ReadChannelData(BT_BM_SDK_ADC0_CHANNEL);
        *value = voltage;
    } else if (adc == BT_BM_SDK_ADC1_CHANNEL) {
        voltage = ADC_ReadChannelData(BT_BM_SDK_ADC1_CHANNEL);
        *value = voltage;
    } else {
    }

    return 0;
}

static bt_at_cmd_sdk_hook_fun_t sdk_cmd_adp[] = {
    {BT_AT_CMD_SDK_GPIO_READ,         sdk_gpio_read},
    {BT_AT_CMD_SDK_GPIO_WRITE,        sdk_gpio_write},
    {BT_AT_CMD_SDK_GPIO_CONFIG_READ,  sdk_gpio_config_read},
    {BT_AT_CMD_SDK_GPIO_CONFIG_WRITE, sdk_gpio_config_write},
    {BT_AT_CMD_SDK_UART_READ,         sdk_temp},
    {BT_AT_CMD_SDK_UART_WRITE,        sdk_uart_write},
    {BT_AT_CMD_SDK_IIC_READ,          sdk_temp},
    {BT_AT_CMD_SDK_IIC_WRITE,         sdk_temp},
    {BT_AT_CMD_SDK_SPI_READ,          sdk_temp},
    {BT_AT_CMD_SDK_SPI_WRITE,         sdk_temp},
    {BT_AT_CMD_SDK_TIMER_READ,        sdk_temp},
    {BT_AT_CMD_SDK_TIMER_WRITE,       sdk_temp},
    {BT_AT_CMD_SDK_PWM_READ,          sdk_temp},
    {BT_AT_CMD_SDK_PWM_WRITE,         sdk_pwm_write},
    {BT_AT_CMD_SDK_ADC_READ,          sdk_adc_read},
    {BT_AT_CMD_SDK_ADC_WRITE,         sdk_temp},
    {BT_AT_CMD_SDK_RTC_READ_SECOND,   sdk_rtc_read_second},
    {BT_AT_CMD_SDK_RTC_READ,          sdk_rtc_read},
    {BT_AT_CMD_SDK_RTC_WRITE,         sdk_rtc_write},
    {BT_AT_CMD_SDK_MODULE_NAME_READ,  sdk_module_name_read},
    {BT_AT_CMD_SDK_MODULE_NAME_WRITE, sdk_module_name_write},
    {BT_AT_CMD_SDK_BAUD_RATE_READ,    sdk_baud_rate_read},
    {BT_AT_CMD_SDK_BAUD_RATE_WRITE,   sdk_baud_rate_write},
    {BT_AT_CMD_SDK_MAC_ADD_READ,      sdk_mac_add_read},
    {BT_AT_CMD_SDK_MAC_ADD_WRITE,     sdk_mac_add_write},
    {BT_AT_CMD_SDK_CON_INTV_WRITE,    sdk_con_intv_write},
    {BT_AT_CMD_SDK_MODULE_RESET,      sdk_module_reset},
    {BT_AT_CMD_SDK_ADV_PERIOD_WRITE,  sdk_temp},
    {BT_AT_CMD_SDK_ADV_DATA_WRITE,    sdk_temp},
    {BT_AT_CMD_SDK_PRODUCT_ID_WRITE,  sdk_product_id_write},
    {BT_AT_CMD_SDK_TX_POW_WRITE,      sdk_tx_power_write},
    {BT_AT_CMD_SDK_COM_DELAY_WRITE,   sdk_temp},
    {BT_AT_CMD_SDK_EN_UP_WRITE,       sdk_temp},
    {BT_AT_CMD_SDK_RSSI_EN_WRITE,     sdk_rssi_en_write},
#ifdef ING_CHIPS_PRIVATE_SERVICE
    {BT_AT_CMD_SDK_RGB_LIGHT_WRITE,   sdk_rgb_light_write},
    {BT_AT_CMD_SDK_TEMPERATURE_READ,  sdk_temperature_read},
#endif
};
static int sdk_cmd_adp_num = sizeof(sdk_cmd_adp) / sizeof(sdk_cmd_adp[0]);

void sdk_1s_timer_task(void)
{
    if (sdk_uart_should_updata_baud_rate > 0) {
        sdk_uart_should_updata_baud_rate--;
        if (sdk_uart_should_updata_baud_rate == 0) {
            sdk_config_uart(USR_UART_IO_PORT, OSC_CLK_FREQ, power_off_save_data_in_ram.module_uart_baud_rate);
        }
    }

    if (sdk_rssi_1s_read_updata_enable == 1) {
        btstack_push_user_msg(USER_MSG_ID_RSSI_SEND, NULL, 0);
    }

    dbg_printf("hi %d\r\n", sdk_gpio_isr_cnt);

    return;
}

void sdk_setup_peripherals(void)
{
    struct tm rtc_time = {
        .tm_year = 2000 - STRUCT_TM_YEAR_BASE_LINE,
        .tm_mon = 1 - STRUCT_TM_MONTH_BASE_LINE,
        .tm_mday = 1,
        .tm_hour = 0,
        .tm_min = 0,
        .tm_sec = 0,
    };
    rtc_time_in_second_base = mktime(&rtc_time);

    sdk_init_private_data();
    platform_set_irq_callback(PLATFORM_CB_IRQ_GPIO, sdk_gpio_isr, NULL);
    sdk_setup_gpio();
 #ifdef ING_CHIPS_PRIVATE_SERVICE
    sdk_setup_iic_temper();
    sdk_setup_rgb_led();
 #endif
    sdk_setup_adc();
    RTC_Enable(1);
    sdk_config_uart_user();
    bt_at_cmd_sdk_adp_register_hook(sdk_cmd_adp, sdk_cmd_adp_num);

    dbg_printf("module para: module name %s, product id 0x%2x_%2x\r\n",
        power_off_save_data_in_ram.module_name,
        power_off_save_data_in_ram.module_product_id[0],
        power_off_save_data_in_ram.module_product_id[1]);

    return;
}

#if defined __cplusplus
    }
#endif

