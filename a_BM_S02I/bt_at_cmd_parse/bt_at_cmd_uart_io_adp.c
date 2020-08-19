
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "platform_api.h"

#include "bt_at_cmd.h"
#include "bt_at_cmd_parse.h"

#include "ingsoc.h"

#if defined __cplusplus
    extern "C" {
#endif

#define CMD_IO_PORT_BUF_LEN (GATT_CHARACTERISTIC_BLE_TO_TX_DATA_BUF_LEN + 1)

typedef struct
{
    int buf_index;
    unsigned char buf[CMD_IO_PORT_BUF_LEN];
} bt_at_cmd_port_buf_t;

static bt_at_cmd_port_buf_t input = {0};
static bt_at_cmd_port_buf_t output = {0};

static TaskHandle_t bt_at_cmd_handle_task_uart_data;
static SemaphoreHandle_t bt_at_cmd_uart_rx_semaphore = NULL;

static void bt_at_cmd_uart_rx_data_fun(char data)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    if (input.buf_index == 0) {
        xSemaphoreGiveFromISR(bt_at_cmd_uart_rx_semaphore, &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }

    if (input.buf_index < sizeof(input.buf)) {
        input.buf[input.buf_index] = data;
        input.buf_index++;
    } else {
        input.buf_index = 0;
    }

    if (input.buf_index >= GATT_CHARACTERISTIC_BLE_TO_TX_DATA_BUF_LEN) {
        bt_at_cmd_parse_send_data_by_ble((char *)input.buf, input.buf_index);
        input.buf_index = 0;
    }

    return;
}

static uint32_t bt_at_cmd_uart_isr(void *user_data)
{
    uint32_t status;

    while(1) {
        status = apUART_Get_all_raw_int_stat(USR_UART_IO_PORT);
        if (status == 0) {
            break;
        }

        USR_UART_IO_PORT->IntClear = status;

        // rx int
        if (status & (1 << bsUART_RECEIVE_INTENAB)) {
            while (apUART_Check_RXFIFO_EMPTY(USR_UART_IO_PORT) != 1) {
                char c = USR_UART_IO_PORT->DataRead;
                bt_at_cmd_uart_rx_data_fun(c);
            }
        }
    }

    return BT_PRIVT_OK;
}

int bt_at_cmd_uart_out_string_with_end_char(const char *data)
{
    int i;

    if (data == BT_PRIVT_NULL) {
        return BT_PRIVT_ERROR;
    }

    for (i = 0; i < CMD_IO_PORT_BUF_LEN; i++) {
        while (apUART_Check_TXFIFO_FULL(USR_UART_IO_PORT) == 1);
        UART_SendData(USR_UART_IO_PORT, data[i]);
        if (data[i] == '\0') {
            break;
        }
    }

    return BT_PRIVT_OK;
}

int bt_at_cmd_uart_out_data(const unsigned char *data, int data_len)
{
    if (data == BT_PRIVT_NULL || data_len >= CMD_IO_PORT_BUF_LEN) {
        return BT_PRIVT_ERROR;
    }

    return BT_PRIVT_OK;
}

static void bt_at_cmd_uart_in_data_handle_task(void *pdata)
{
    int last_in_data_len = 0;

    while (xSemaphoreTake(bt_at_cmd_uart_rx_semaphore, portMAX_DELAY)) {
        last_in_data_len = input.buf_index;
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(100));
            if (input.buf_index == 0) {
                break;
            } else if (input.buf_index != last_in_data_len) {
                last_in_data_len = input.buf_index;
            } else {
                input.buf[input.buf_index] = '\0';
                if (bt_at_cmd_parse((char *)input.buf, input.buf_index, (char *)output.buf, CMD_IO_PORT_BUF_LEN) == BT_PRIVT_OK) {
                    bt_at_cmd_uart_out_string_with_end_char((char *)output.buf);
                }
                input.buf_index = 0;
                last_in_data_len = 0;
                break;
            }
        }
    }
}

void bt_at_cmd_uart_io_init(void)
{
    bt_at_cmd_uart_rx_semaphore = xSemaphoreCreateBinary();
    xTaskCreate(bt_at_cmd_uart_in_data_handle_task, "task_uart_data", 1024, NULL, (1), &bt_at_cmd_handle_task_uart_data);

    platform_irq_callback_type_t irq_type = (USR_UART_IO_PORT == APB_UART0) ? PLATFORM_CB_IRQ_UART0 : PLATFORM_CB_IRQ_UART1;
    platform_set_irq_callback(irq_type, bt_at_cmd_uart_isr, NULL);

    return;
}

#if defined __cplusplus
    }
#endif

