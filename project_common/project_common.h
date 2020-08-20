
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

#if defined __cplusplus
    }
#endif

#endif

