
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <stdint.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "platform_api.h"
#include "ingsoc.h"
#include "peripheral_uart.h"

#include "../../project_common/project_common.h"

#if defined __cplusplus
    extern "C" {
#endif

typedef struct platform_evt_cb_info
{
    f_platform_evt_cb f;
    void *user_data;
} platform_evt_cb_info_t;

typedef struct platform_irq_cb_info
{
    f_platform_irq_cb f;
    void *user_data;
} platform_irq_cb_info_t;

static platform_evt_cb_info_t platform_evt_callbacks[PLATFORM_CB_EVT_MAX] = {0};

static assertion_info_t  assertion_info = {0};

const IRQn_Type irq_mapping[PLATFORM_CB_IRQ_MAX] =
{
    n02_RTC_M0_IRQn,    //    PLATFORM_CB_IRQ_RTC0,
    n04_TMR0_IRQn,      //    PLATFORM_CB_IRQ_TIMER0,
    n05_TMR1_IRQn,      //    PLATFORM_CB_IRQ_TIMER1,
    n06_TMR2_IRQn,      //    PLATFORM_CB_IRQ_TIMER2,
    n09_GPIO_IRQn,      //    PLATFORM_CB_IRQ_GPIO,
    n14_SPI0_IRQn,      //    PLATFORM_CB_IRQ_SPI0,
    n15_SPI1_IRQn,      //    PLATFORM_CB_IRQ_SPI1,
    n16_URT0_IRQn,      //    PLATFORM_CB_IRQ_UART0,
    n17_URT1_IRQn,      //    PLATFORM_CB_IRQ_UART1,
    n18_I2C_IRQn,       //    PLATFORM_CB_IRQ_I2C0,
    n26_I2C_IRQn,       //    PLATFORM_CB_IRQ_I2C1,
};

static uint32_t invoke_evt_cb(platform_evt_callback_type_t type, void *data, uint8_t lock)
{
    platform_evt_cb_info_t *info = &platform_evt_callbacks[type];
    if (lock) __disable_irq();

    if (info->f) {
        return info->f(data, info->user_data);
    }

    if (0 == lock) {
        return 0;
    }

    switch (type)
    {
        case PLATFORM_CB_HEAP_OOM:
            dbg_printf("oom %ld", (uint32_t)(data));
            break;
        case PLATFORM_CB_EVT_ASSERTION:
            dbg_printf("[ERR]@%s:%d", assertion_info.file_name, assertion_info.line_no);
            break;
        default:
            break;
    }

    while (1) {};
}

static void NVIC_SetVectorTable(uint32_t NVIC_VectTab, uint32_t Offset)
{
    /* Check the parameters */
    SCB->VTOR = NVIC_VectTab | (Offset & (uint32_t)0x1FFFFF80);
}

static uint32_t cb_putc(char *c, void *dummy)
{
    uint8_t ch = (uint8_t)*c;

    while (apUART_Check_TXFIFO_FULL(LOG_UART) == 1);
    UART_SendData(LOG_UART, ch);

    return 0;
}

static void irq_setup()
{
    uint8_t i;

    NVIC_SetPriority(n00_BB0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    NVIC_SetPriority(n01_BB1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    for (i = n02_RTC_M0_IRQn; i <= n28_URT1_IRQn; i++)
    {
        NVIC_SetPriority((IRQn_Type)i, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2);
        NVIC_DisableIRQ((IRQn_Type)i);
    }
}

void platform_raise_assertion(const char *file_name, int line_no)
{
    assertion_info.file_name = file_name;
    assertion_info.line_no = line_no;

    invoke_evt_cb(PLATFORM_CB_EVT_ASSERTION, &assertion_info, 1);
}

void platform_set_evt_callback(platform_evt_callback_type_t type, f_platform_evt_cb f, void *user_data)
{
    platform_evt_callbacks[type].f = f;
    platform_evt_callbacks[type].user_data = user_data;
}

void transp_freertos_init(void)
{
    __disable_irq();
    NVIC_SetVectorTable(0x00, 0x4000);
    irq_setup();
    platform_set_evt_callback(PLATFORM_CB_EVT_PUTC, (f_platform_evt_cb)cb_putc, NULL);

    return;
}

#if defined __cplusplus
    }
#endif
