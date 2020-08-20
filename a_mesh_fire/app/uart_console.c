/*
 * Copyright (c) 2018 INGCHIPS MESH
 */


/*
 *  uart_console.c
 *
 *  
 *  
 *
 */
/*for PTS test*/ 
#include <stdint.h>
#include "ingsoc.h"
#include "btstack_event.h"
#include "mesh_api.h"
#include <stdarg.h>
#define AT_CMD_LEN (200)
typedef struct
{
    uint16_t size;
    char buf[AT_CMD_LEN];
} str_buf_t;

str_buf_t input = {0};
str_buf_t output = {0};

static void append_data(str_buf_t *buf, const char *d, const uint16_t len)
{
    if (buf->size + len > sizeof(buf->buf))
        buf->size = 0;
    
    if (buf->size + len <= sizeof(buf->buf))
    {
        memcpy(buf->buf + buf->size, d, len);
        buf->size += len;
    }
}

void console_rx_data(const char *d, uint8_t len)
{
    
    if (0 == input.size)
    {
        while ((len > 0) && ((*d == '\r') || (*d == '\n')))
        {
            d++;
            len--;
        }
    }
    if (len == 0) return;
    
    append_data(&input, d, len);

    if ((input.size > 0) && 
        ((input.buf[input.size - 1] == '\r') || (input.buf[input.size - 1] == '\r')))
    {
        int16_t t = input.size - 2;
        while ((t > 0) && ((input.buf[t] == '\r') || (input.buf[t] == '\n'))) t--;
        input.buf[t + 1] = '\0';        
        mesh_at_entry((uint8_t*)input.buf,t+2);
        input.size = 0;
    }
}
extern void console_out(const char *str, int cnt);
uint32_t uart1_isr(void *user_data)
{
    uint32_t status;
    while(1)
    {
        status = apUART_Get_all_raw_int_stat(APB_UART1);
        if (status == 0)
            break;

        APB_UART1->IntClear = status;

        // rx int
        if (status & (1 << bsUART_RECEIVE_INTENAB))
        {
            apUART_Clr_RECEIVE_INT(APB_UART1);
            while (apUART_Check_RXFIFO_EMPTY(APB_UART1) != 1)
            {
                char c = APB_UART1->DataRead;             
                console_rx_data(&c, 1);
            }
        }
    }
    return 0;     
}

void console_out(const char *str, int cnt)
{
    uint16_t txsize =0;
    uint16_t txPointer =0;
    txsize =  cnt;
    txPointer = 0;	
  	while(txPointer<txsize)
    {
			if (!apUART_Check_TXFIFO_FULL(APB_UART1))
			{
					UART_SendData(APB_UART1,str[txPointer++]);
			}
	}
}
