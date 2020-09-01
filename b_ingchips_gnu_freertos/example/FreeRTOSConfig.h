
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "ingsoc.h"

#define configUSE_PREEMPTION        1
#define configUSE_IDLE_HOOK         0
#define configUSE_TICK_HOOK         0
#ifdef POWER_SAVING
#define configUSE_TICKLESS_IDLE     1
#endif
#define configSYSTICK_CLOCK_HZ      32768
#if TARGET_FPGA
#define configCPU_CLOCK_HZ          ( ( unsigned long ) 32000000 )
#else
#define configCPU_CLOCK_HZ          ( ( unsigned long ) 48000000 )
#endif
#define configTICK_RATE_HZ          ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES        ( 15 )
#define configMINIMAL_STACK_SIZE    ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE       ( ( size_t ) ( 30 * 1024 ) )
#define configMAX_TASK_NAME_LEN     ( 16 )
#define configUSE_TRACE_FACILITY    0
#define configUSE_16_BIT_TICKS      0
#define configIDLE_SHOULD_YIELD     1
#define configUSE_QUEUE_SETS        1
#define configSUPPORT_STATIC_ALLOCATION 0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero to exclude the API function. */

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1


/* Cortex-M specific definitions. */
#define configPRIO_BITS              __NVIC_PRIO_BITS


/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY       ((1 << configPRIO_BITS) - 1)
/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    0x01
/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY        ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY   ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

void platform_raise_assertion(const char *file_name, int line_no);

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
//#define configASSERT( x ) if( ( x ) == 0 ) { platform_raise_assertion(__MODULE__, __LINE__); }
#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#ifdef POWER_SAVING

#define configPRE_SUPPRESS_TICKS_AND_SLEEP_PROCESSING(xExpectedIdleTime)    \
        do {    extern TickType_t sysPreSuppressTicksAndSleepProcessing(TickType_t expectedTicks);  \
                xExpectedIdleTime = sysPreSuppressTicksAndSleepProcessing(xExpectedIdleTime);       \
        } while (0)

#define configPRE_SLEEP_PROCESSING(xModifiableIdleTime)                             \
		do {    extern void sysPreSleepProcessing(TickType_t idleTime);             \
                sysPreSleepProcessing(xModifiableIdleTime);                         \
        } while (0)

#define configPOST_SLEEP_PROCESSING( xExpectedIdleTime )                            \
   		do {    extern void sysPostSleepProcessing(TickType_t idleTime);            \
                sysPostSleepProcessing(xModifiableIdleTime);                        \
        } while (0)
#else
#define configPRE_SUPPRESS_TICKS_AND_SLEEP_PROCESSING(xExpectedIdleTime) \
        do {                                 \
                xExpectedIdleTime = 0;       \
        } while (0)
#endif

#define configUSE_TIMERS				1
#define configTIMER_TASK_PRIORITY		( 2 )
#define configTIMER_QUEUE_LENGTH		5
#define configTIMER_TASK_STACK_DEPTH	( 80 )

#define configUSE_MUTEXES				1
#define configSUPPORT_DYNAMIC_ALLOCATION		1

#endif /* FREERTOS_CONFIG_H */

