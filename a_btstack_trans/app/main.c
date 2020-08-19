
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include "FreeRTOS.h"
#include "task.h"

#include "app_main.h"
#include "free_rtos_transp.h"
#include "btstack_transp.h"

#if defined __cplusplus
    extern "C" {
#endif

int main()
{
    free_rtos_transp_init();

    btstack_transp_init();

    app_main();

    vTaskStartScheduler(); // never return
    while (1) {
    }
}

#if defined __cplusplus
    }
#endif
