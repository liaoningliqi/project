
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>

#include "FreeRTOS.h"
#include "timers.h"

#include "platform_api.h"
#include "profile.h"

#include "mesh_def.h"

#include "ble_mesh.h"
#include "chip_peripherals.h"
#include "ble_mesh_flash_oper.h"
#include "..\..\project_common\project_common.h"
#include "ble_mesh_light_model.h"

#if defined __cplusplus
    extern "C" {
#endif

__weak void __aeabi_assert(const char *a ,const char* b, int c)
{
    dbg_printf("assert\n");
    for (;;);
}

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

int app_main()
{
    platform_set_evt_callback(PLATFORM_CB_EVT_PUTC, (f_platform_evt_cb)cb_putc, NULL);
    chip_peripherals_init();

    fast_switch_monitor_init();

    light_power_on();

    platform_set_evt_callback(PLATFORM_CB_EVT_PROFILE_INIT, setup_profile, NULL);

    platform_config(PLATFORM_CFG_LOG_HCI,0);

    set_mesh_uart_output_func(bt_cmd_data_uart_out_data);
    mesh_trace_config(PROV_FEA | ACC_LAYER, 7); // PROV_FEA | ACC_LAYER | NET_LAYER

    return 0;
}

#if defined __cplusplus
    }
#endif

