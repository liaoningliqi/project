
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "platform_api.h"
#include "ingsoc.h"

#include "..\..\project_common\project_common.h"
#include "..\..\bt_at_cmd_parse\bt_at_cmd.h"
#include "profile.h"
#include "sdk.h"

uint32_t cb_putc(char *c, void *dummy)
{
    while (apUART_Check_TXFIFO_FULL(USR_UART_IO_PORT) == 1);
    UART_SendData(USR_UART_IO_PORT, (uint8_t)*c);
    return 0;
}

int fputc(int ch, FILE *f)
{
    cb_putc((char *)&ch, NULL);
    return ch;
}

static void main_1s_timer_task(TimerHandle_t xTimer)
{
    sdk_1s_timer_task();
    bt_bm_module_evt_1s_timer_task();

    return;
}

int app_main()
{
    TimerHandle_t main_1s_timer = 0;
    // If there are *three* crystals on board, *uncomment* below line.
    // Otherwise, below line should be kept commented out.
    // platform_set_rf_clk_source(0);
    platform_set_evt_callback(PLATFORM_CB_EVT_PUTC, (f_platform_evt_cb)cb_putc, NULL);
    platform_config(PLATFORM_CFG_32K_CLK, PLATFORM_32K_RC);

    sysSetPublicDeviceAddr((uint8_t *)0x2a100);

    sdk_setup_peripherals();
    bt_at_cmd_parse_init();
    bt_cmd_data_uart_io_init();
    bt_bm_module_init_service();

    platform_set_evt_callback(PLATFORM_CB_EVT_PROFILE_INIT, setup_profile, NULL);

    main_1s_timer = xTimerCreate("main_1s_task", pdMS_TO_TICKS(1000), pdTRUE, NULL, main_1s_timer_task);
    xTimerStart(main_1s_timer, portMAX_DELAY);

    return 0;
}



