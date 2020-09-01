
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <stdint.h>

#include "btstack_event.h"

#include "platform_api.h"
#include "peripheral_gpio.h"

#include "app.h"
#include "chip_peripherals.h"
#include "..\..\project_common\project_common.h"

#if defined __cplusplus
    extern "C" {
#endif

#define KB_KEY_1 GIO_GPIO_1
#define KB_KEY_2 GIO_GPIO_5
#define KB_KEY_3 GIO_GPIO_7

#define USER_UART0_IO_TX GIO_GPIO_2
#define USER_UART0_IO_RX GIO_GPIO_3
#define USER_UART1_IO_TX GIO_GPIO_7
#define USER_UART1_IO_RX GIO_GPIO_8
#define USER_UART1_IO_RTS GIO_GPIO_2
#define USER_UART1_IO_CTS GIO_GPIO_16

void kb_report_trigger_send(uint8_t key)
{
    btstack_push_user_msg(key, NULL, 0);// 发送给controller 按键trigger触发
}

void kb_state_changed(uint8_t key)
{
    if(key==1) {
        kb_report_trigger_send(USER_MSG_ID_REQUEST_SEND_KB1);
    } else {
        kb_report_trigger_send(USER_MSG_ID_REQUEST_SEND_KB2);
    }

    return;
}

void delay(int cycles)
{
    int i;
    for (i = 0; i < cycles; i++) {
        __nop();
    }
}

#define pulse()                     \
    { GIO_WriteValue(PIN_SDI, 1);   \
    delay(1);                       \
    GIO_WriteValue(PIN_SDI, 0); } while (0)

void tlc59731_write(uint32_t value)
{
    int8_t i;

    for( i = 0; i < 32; i++ ) {
        uint32_t bit = value & (0x80000000 >> i);
        pulse();

        if (bit) {
            delay(10);
            pulse();
            delay(78);
        } else {
            delay(90);
        }
    }
    delay(100 * 8);
}

void set_led_color(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t cmd = (0x3a << 24) | (b << 16) | (r << 8) | g;
    tlc59731_write(cmd);
}

uint32_t peripherals_gpio_isr(void *user_data)
{
    uint32_t current = ~GIO_ReadAll();

    if (current & (1 << KB_KEY_1)) {
        dbg_printf("KB_KEY_1_isr!!\r\n");
        kb_state_changed(1);
    } else if (current & (1 << KB_KEY_2)) {
        dbg_printf("KB_KEY_2_isr!!\r\n");
        kb_state_changed(2);
    } else {
    }

    GIO_ClearAllIntStatus();
    return 0;
}

static void peripherals_config_uart(UART_TypeDef* port, uint32_t freq, uint32_t baud)
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

static void peripherals_config_uart_user()
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

    peripherals_config_uart(USR_UART_IO_PORT, OSC_CLK_FREQ, 115200);

    return;
}

void peripherals_setup(void)
{
    peripherals_config_uart_user();

    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_GPIO) | (1 << SYSCTRL_ClkGate_APB_PinCtrl) | (1<<SYSCTRL_ClkGate_APB_PWM));
    PINCTRL_DisableAllInputs();
    PINCTRL_SetPadMux(KB_KEY_1, IO_SOURCE_GENERAL);
    PINCTRL_SetPadMux(KB_KEY_2, IO_SOURCE_GENERAL);
    GIO_SetDirection(KB_KEY_1, GIO_DIR_INPUT);
    GIO_SetDirection(KB_KEY_2, GIO_DIR_INPUT);
    GIO_ConfigIntSource(KB_KEY_1, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE, GIO_INT_EDGE);
    GIO_ConfigIntSource(KB_KEY_2, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE, GIO_INT_EDGE);
    PINCTRL_SetPadMux(PIN_SDI, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(PIN_SDI, 0);
    GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
    GIO_WriteValue(PIN_SDI, 0);
    set_led_color(50, 50, 50);

    return;
}

void chip_peripherals_init(void)
{
    platform_set_rf_clk_source(0);  	//use external crystal
    peripherals_setup();

    platform_set_irq_callback(PLATFORM_CB_IRQ_GPIO, peripherals_gpio_isr, NULL);//key isr

    return;
}

#if defined __cplusplus
    }
#endif

