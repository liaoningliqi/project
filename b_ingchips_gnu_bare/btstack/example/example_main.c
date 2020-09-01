
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/
#include "stdio.h"

#include "ingsoc.h"
#include "peripheral_gpio.h"
#include "peripheral_pinctrl.h"
#include "peripheral_sysctrl.h"
#include "peripheral_uart.h"

#if defined __cplusplus
    extern "C" {
#endif

static void config_uart(uint32_t freq, uint32_t baud)
{
    UART_sStateStruct UART_0;

    UART_0.word_length       = UART_WLEN_8_BITS;
    UART_0.parity            = UART_PARITY_NOT_CHECK;
    UART_0.fifo_enable       = 1;
    UART_0.two_stop_bits     = 0;   //used 2 stop bit ,    0
    UART_0.receive_en        = 1;
    UART_0.transmit_en       = 1;
    UART_0.UART_en           = 1;
    UART_0.cts_en            = 0;
    UART_0.rts_en            = 0;
    UART_0.rxfifo_waterlevel = 1;    //UART_FIFO_ONE_SECOND;
    UART_0.txfifo_waterlevel = 3;    //UART_FIFO_ONE_SECOND;
    UART_0.ClockFrequency    = freq;
    UART_0.BaudRate          = baud;

    apUART_Initialize(APB_UART0, &UART_0, 0);
}

static void setup_peripherals(void)
{
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_GPIO) | (1 << SYSCTRL_ClkGate_APB_PWM) | (1 << SYSCTRL_ClkGate_APB_PinCtrl));

    PINCTRL_SetPadMux(GIO_GPIO_6, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(GIO_GPIO_6, 0);
    GIO_SetPull(GIO_GPIO_6, 1, GIO_PULL_DOWN);
    GIO_SetDirection(GIO_GPIO_6, GIO_DIR_OUTPUT);
    GIO_WriteValue(GIO_GPIO_6, 0);

    PINCTRL_DisableAllInputs();
    PINCTRL_SelUartRxdIn(UART_PORT_0, 3);

    config_uart(OSC_CLK_FREQ, 115200);
}

static void delay1(void)
{
    volatile int i;
    volatile int j;
    volatile int k;
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 100; j++) {
            k++;
        }
    }
    return;
}

static void delay2(void)
{
    volatile int i;
    volatile int j;
    volatile int k;
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            k++;
        }
    }
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            k++;
        }
    }
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            k++;
        }
    }
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            k++;
        }
    }
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            k++;
        }
    }
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            k++;
        }
    }
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            k++;
        }
    }
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            k++;
        }
    }
    return;
}

int _write(int fd, char *pBuffer, int size)
{
    for (int i = 0; i < size; i++) {
        while (apUART_Check_TXFIFO_FULL(APB_UART0) == 1);
        UART_SendData(APB_UART0, pBuffer[i]);
    }
    return size;
}

int example_main()
{
    setup_peripherals();

    printf("hi\n");
    fflush(stdout);

    while (1) {
        GIO_WriteValue(GIO_GPIO_6, 0);
        delay1();
        GIO_WriteValue(GIO_GPIO_6, 1);
        delay2();
    }
}

#if defined __cplusplus
    }
#endif
