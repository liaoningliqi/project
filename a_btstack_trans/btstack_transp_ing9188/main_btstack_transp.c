
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#define __BTSTACK_FILE__ "main_btstack_transp.c"

#include "compiler.h"
#include <stdio.h>
#include <stdint.h>

#include "ingsoc.h"

#include "btstack_event.h"
#include "btstack_run_loop.h"
#include "btstack_run_loop_embedded.h"
#include "hci.h"
#include "bluetooth_company_id.h"
#include "btstack_memory.h"
#include "classic/btstack_link_key_db.h"

// STDOUT_FILENO and STDERR_FILENO are defined by <unistd.h> with GCC
// (this is a hack for IAR)
#ifndef STDOUT_FILENO
#define STDERR_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

// Configuration
// LED2 on PA5
// Debug: USART2, TX on PA2
// Bluetooth: USART3. TX PB10, RX PB11, CTS PB13 (in), RTS PB14 (out), N_SHUTDOWN PB15
#define GPIO_LED2 GPIO5
#define USART_CONSOLE USART2
#define GPIO_BT_N_SHUTDOWN GPIO15

#define GPIO_DEBUG_0 GPIO1
#define GPIO_DEBUG_1 GPIO2

// btstack code starts there
void btstack_main(void);

// hal_tick.h inmplementation
#include "hal_tick.h"

static void dummy_handler(void);
static void (*tick_handler)(void) = &dummy_handler;

static void dummy_handler(void){};

static btstack_packet_callback_registration_t hci_event_callback_registration;

void hal_tick_init(void){
}

int  hal_tick_get_tick_period_in_ms(void){
    return 100;
}

void hal_tick_set_handler(void (*handler)(void)){
    if (handler == NULL){
        tick_handler = &dummy_handler;
        return;
    }
    tick_handler = handler;
}

void sys_tick_handler(void){
	(*tick_handler)();
}

// hal_led.h implementation
#include "hal_led.h"
void hal_led_off(void);
void hal_led_on(void);

void hal_led_off(void){
}
void hal_led_on(void){
}
void hal_led_toggle(void){
}

// hal_cpu.h implementation
#include "hal_cpu.h"

void hal_cpu_disable_irqs(void){
	__disable_irq();
}

void hal_cpu_enable_irqs(void){
	__enable_irq();
}

void hal_cpu_enable_irqs_and_sleep(void){
	hal_led_off();
	__enable_irq();
	__asm__("wfe");	// go to sleep if event flag isn't set. if set, just clear it. IRQs set event flag

	// note: hal_uart_needed_during_sleep can be used to disable peripheral clock if it's not needed for a timer
	hal_led_on();
}

// hal_uart_dma.c implementation
#include "hal_uart_dma.h"

// handlers
static void (*rx_done_handler)(void) = dummy_handler;
static void (*tx_done_handler)(void) = dummy_handler;
static void (*cts_irq_handler)(void) = dummy_handler;

void hal_uart_dma_set_sleep(uint8_t sleep){
}

// DMA1_CHANNEL2 UART3_TX
void dma1_channel2_isr(void) {
}

// DMA1_CHANNEL2 UART3_RX
void dma1_channel3_isr(void){
}

// CTS RISING ISR
void exti15_10_isr(void){
}

void hal_uart_dma_init(void){
}
void hal_uart_dma_set_block_received( void (*the_block_handler)(void)){
    rx_done_handler = the_block_handler;
}

void hal_uart_dma_set_block_sent( void (*the_block_handler)(void)){
    tx_done_handler = the_block_handler;
}

void hal_uart_dma_set_csr_irq_handler( void (*the_irq_handler)(void)){
    cts_irq_handler = the_irq_handler;
}

int  hal_uart_dma_set_baud(uint32_t baud){
	return 0;
}

void hal_uart_dma_send_block(const uint8_t *data, uint16_t size){
}

void hal_uart_dma_receive_block(uint8_t *data, uint16_t size){
}

static void clock_setup(void){
}

static void gpio_setup(void){
}

static void debug_usart_setup(void){
}

static void bluetooth_setup(void){
}


// after HCI Reset, use 115200. Then increase baud reate to 468000.
// (on nucleo board without external crystall, running at 8 Mhz, 1 mbps was not possible)
static const hci_transport_config_uart_t config = {
	HCI_TRANSPORT_CONFIG_UART,
    115200,
    460800,
    1,
    NULL
};

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
}

int btstack_transp_init(void)
{
	clock_setup();
	gpio_setup();
	hal_tick_init();
	debug_usart_setup();
	bluetooth_setup();

	// start with BTstack init - especially configure HCI Transport
    btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_embedded_get_instance());

    // init HCI
    hci_init(hci_transport_h4_instance(btstack_uart_block_embedded_instance()), (void*) &config);
    hci_set_link_key_db(btstack_link_key_db_memory_instance());

    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

	// hand over to btstack embedded code
    btstack_main();

    // go
    btstack_run_loop_execute();

	return 0;
}
