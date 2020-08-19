
#include "ingsoc.h"
#include "platform_api.h"
#include "peripheral_pwm.h"
#include "bluetooth.h"
#include "bt_types.h"
#include "att_db.h"
#include "gap.h"
#include "mesh_api.h"
#include "light_model.h"
#include "RGB_drv.h"
#include "platform_api.h"
#include "profile.h"
#include "kv_storage.h"
#include "eflash.h"
uint8_t* mesh_flash_mirror_addr = NULL;
struct k_delayed_work switch_onff;
struct k_delayed_work switch_onff_1phase;
//static uint16_t duration = 3000;
static uint8_t  reset_count =4;
uint32_t finalword =0;
uint32_t off=0;


//#define LIGHT_SEC3  3
#define KB_KEY_1        GIO_GPIO_1
#define KB_KEY_2        GIO_GPIO_5
#define KB_KEY_3        GIO_GPIO_7

#define KEY_MASK        ((1 << KB_KEY_1) | (1 << KB_KEY_2) | (1 << KB_KEY_3))

//#define UART1_USED
#define UART1_IN   PWM_GPIO_7
#define UATR1_OU   PWM_GPIO_8
#define w32(a,b) *(volatile uint32_t*)(a) = (uint32_t)(b);
#define r32(a)   *(volatile uint32_t*)(a)

#define INVALID_VALUE        (0xffffffff)


int8_t after_power_on()
{
    extern uint8_t counter;
    counter = 0;
    kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(counter));
    return 1;
}

extern uint8_t initial_fac_conf(void);
uint8_t counter = 0;
int8_t fast_switch_monitor()
{
//	EflashCacheBypass();
//	EflashProgramEnable();
// 	EraseEFlashPage(0x28);
// 	EraseEFlashPage(0x29);
// 	EflashProgramDisable();
// 	EflashCacheEna();
// 	EflashCacheFlush();
    int16_t len = 0;
    uint8_t *db = NULL;
    k_delayed_work_init(&switch_onff,(ble_npl_event_fn *)after_power_on);
    //read from flash ,to get the state of last power off;
    db = kv_get(FAST_POWER_ON_FLASH_INDEX,&len);
    if (!db)
    {
        counter =1;
        kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(uint8_t));
    }
    else
    {
        //get the current value;
        counter = *((uint8_t*)db);
        counter++;
        printf("counter 0x%x\n",counter-1);
    }
    if(counter>=reset_count)
    {
           printf("reset start\n");
           counter = 0;
           initial_fac_conf();
           return 1;
    }
    else
    {
        kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(uint8_t));
        kv_commit();
    }
    platform_printf("5s start\n");
    k_delayed_work_submit(&switch_onff,5000);
    return 0;
}

#define PRINT_UART    APB_UART0
#define PRINT_UART1   APB_UART1
uint32_t cb_putc(char *c, void *dummy)
{
    while (apUART_Check_TXFIFO_FULL(PRINT_UART) == 1);
    UART_SendData(PRINT_UART, (uint8_t)*c);
    return 0;
}

int fputc(int ch, FILE *f)
{
    cb_putc((char *)&ch, NULL);
    return ch;
}

__weak void __aeabi_assert(const char *a ,const char* b, int c)
{
    printf("assert\n");
    for (;;);
}


void config_uart(uint32_t freq, uint32_t baud)
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
    UART_0.rxfifo_waterlevel = 1;   //UART_FIFO_ONE_SECOND;
    UART_0.txfifo_waterlevel = 1;   //UART_FIFO_ONE_SECOND;
    UART_0.ClockFrequency    = freq;
    UART_0.BaudRate          = baud;

    apUART_Initialize(PRINT_UART, &UART_0, 1<<bsUART_RECEIVE_INTENAB);
    apUART_Initialize(PRINT_UART1,&UART_0, 1<<bsUART_RECEIVE_INTENAB);
}


#ifdef BOARD_V2

#define PIN_SDI   GIO_GPIO_0
// CPU clok: PLL_CLK_FREQ  48000000
// 1 cycle = 21ns
// 48 cycles per us
// Tcycle = 2us --> ~100 cycles
void delay(int cycles)
{
    int i;
    for (i = 0; i < cycles; i++)
    {
        __nop();
    }
}

void tlc59731_write(uint32_t value)
{
    int8_t i;

    #define pulse()                     \
        { GIO_WriteValue(PIN_SDI, 1);   \
        delay(1);                       \
        GIO_WriteValue(PIN_SDI, 0); } while (0)

    for( i = 0; i < 32; i++ )
    {
        uint32_t bit = value & ( 0x80000000 >> i );
        pulse();

        if (bit)
        {
            delay(10);
            pulse();
            delay(78);
        }
        else
            delay(90);
    }
    delay(100 * 8);
}

void set_led_color(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t cmd = (0x3a << 24) | (b << 16) | (r << 8) | g;
    tlc59731_write(cmd);
}

void setup_peripherals(void)
{
#ifdef UART1_USED
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_UART1));
    PINCTRL_SetPadMux(UATR1_OU, IO_SOURCE_UART1_TXD);
    PINCTRL_SelUartRxdIn(UART_PORT_1,UART1_IN);
#endif
    config_uart(OSC_CLK_FREQ, 115200);
//	setup timer 2: 20Hz

    // setup GPIOs for keys
    SYSCTRL_ClearClkGateMulti(  (1 << SYSCTRL_ClkGate_APB_GPIO)
                              | (1 << SYSCTRL_ClkGate_APB_PinCtrl)
                              |  (1<<SYSCTRL_ClkGate_APB_PWM));
    PINCTRL_DisableAllInputs();
	PINCTRL_SetPadMux(KB_KEY_1, IO_SOURCE_GENERAL);
    PINCTRL_SetPadMux(KB_KEY_2, IO_SOURCE_GENERAL);
	GIO_SetDirection(KB_KEY_1, GIO_DIR_INPUT);
    GIO_SetDirection(KB_KEY_2, GIO_DIR_INPUT);
    GIO_ConfigIntSource(KB_KEY_1, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE ,
                        GIO_INT_EDGE);
    GIO_ConfigIntSource(KB_KEY_2, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE ,
                        GIO_INT_EDGE);
    PINCTRL_SetPadMux(PIN_SDI, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(PIN_SDI, 0);
    GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
    GIO_WriteValue(PIN_SDI, 0);
    set_led_color(50,50,50);
}

#else
#define CHANNEL_RED     10
#define CHANNEL_GREEN   8
#define CHANNEL_BLUE    6

#define CHANNEL_WARM  0
#define CHANNEL_COLD  4

#define UART1_RX      6
#define UART1_TX      7
#define PERA_THRESHOLD 0x10000//(OSC_CLK_FREQ / 1000)

void set_led_color(uint8_t r, uint8_t g, uint8_t b)
{
#define TO_PERCENT(v) (((uint32_t)(v) * 100) >> 8)
    PWM_SetHighThreshold(CHANNEL_RED   >> 1, 0, PERA_THRESHOLD / 100 * TO_PERCENT(r));
    PWM_SetHighThreshold(CHANNEL_GREEN >> 1, 0, PERA_THRESHOLD / 100 * TO_PERCENT(g >> 1));  // GREEN & BLUE led seems too bright
    PWM_SetHighThreshold(CHANNEL_BLUE  >> 1, 0, PERA_THRESHOLD / 100 * TO_PERCENT(b >> 1));
}

static void setup_channel(uint8_t channel_index)
{
    PWM_HaltCtrlEnable(channel_index, 1);
    PWM_Enable(channel_index, 0);
    PWM_SetPeraThreshold(channel_index, PERA_THRESHOLD);
    PWM_SetMultiDutyCycleCtrl(channel_index, 0);        // do not use multi duty cycles
    PWM_SetHighThreshold(channel_index, 0, PERA_THRESHOLD / 2);
    PWM_SetMode(channel_index, PWM_WORK_MODE_UP_WITHOUT_DIED_ZONE);
    PWM_SetMask(channel_index, 0, 0);
    PWM_Enable(channel_index, 1);
    PWM_HaltCtrlEnable(channel_index, 0);
}

void setup_peripherals(void)
{
#ifdef UART1_USED
//	w32(0x40070058,0xffffffff);
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_UART1));
    PINCTRL_SetPadMux(UATR1_OU, IO_SOURCE_UART1_TXD);
    PINCTRL_SelUartRxdIn(UART_PORT_1,UART1_IN);
#endif
    config_uart(OSC_CLK_FREQ, 115200);
#ifndef UART1_USED
    PINCTRL_SetPadMux(CHANNEL_RED, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(CHANNEL_RED, 1);
    PINCTRL_SetPadMux(CHANNEL_GREEN, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(CHANNEL_GREEN, 1);
    PINCTRL_SetPadMux(CHANNEL_BLUE, IO_SOURCE_GENERAL);
    PINCTRL_SetPadPwmSel(CHANNEL_BLUE, 1);
#endif
// setup GPIOs for keys
    SYSCTRL_ClearClkGateMulti(  (1 << SYSCTRL_ClkGate_APB_GPIO)
                              | (1 << SYSCTRL_ClkGate_APB_PinCtrl));
    PINCTRL_DisableAllInputs();
    PINCTRL_SetPadMux(KB_KEY_1, IO_SOURCE_GENERAL);
    PINCTRL_SetPadMux(KB_KEY_2, IO_SOURCE_GENERAL);
    PINCTRL_SetPadMux(KB_KEY_3, IO_SOURCE_GENERAL);
    GIO_SetDirection(KB_KEY_1, GIO_DIR_INPUT);
 	GIO_SetDirection(KB_KEY_2, GIO_DIR_INPUT);
 	GIO_SetDirection(KB_KEY_3, GIO_DIR_INPUT);
    GIO_ConfigIntSource(KB_KEY_1, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE | GIO_INT_EN_LOGIC_HIGH_OR_RISING_EDGE,
                        GIO_INT_EDGE);
 	GIO_ConfigIntSource(KB_KEY_2, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE | GIO_INT_EN_LOGIC_HIGH_OR_RISING_EDGE,
                         GIO_INT_EDGE);
 	GIO_ConfigIntSource(KB_KEY_3, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE | GIO_INT_EN_LOGIC_HIGH_OR_RISING_EDGE,
                         GIO_INT_EDGE);
//#endif
}
#endif

extern uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
extern btstack_packet_callback_registration_t hci_event_callback_registration;
extern int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode,
                              uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
extern const uint8_t adv_data[][];
extern void user_packet_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size);
extern const unsigned char addr[6];
extern int mesh_env_init(void);
/*below for TIANMAO configuration*/
uint32_t BLE_MESH_DEV_PRODUCT_ID = 5349350;

#define LIGHT_SEC_ALI

#ifdef LIGHT_SEC_ALI
#define SEC	 ((uint8_t[16]){0x64,0xc9,0x41,0x14,0xca,0x07,0x24,0x40,0xa3,0xc1,0xbb,0x2b,0x22,0xb5,0x24,0x5e})
#define PB_ADV_ADDR   ((uint8_t[6]){0x28,0xfa,0x7a,0xa3,0xcd,0xfc})
#endif

#ifdef LIGHT_SEC4
#define SEC	 ((uint8_t[16]){0x39,0xd6,0x96,0xff,0x64,0xfd,0xbf,0xe2,0xbb,0xfe,0x27,0xc0,0x5a,0xb3,0x26,0x5f})
#define PB_ADV_ADDR   ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x2f})
#endif

#ifdef LIGHT_SEC3
#define SEC	 ((uint8_t[16]){0x59,0x06,0x2b,0x94,0x04,0xdf,0x29,0xfc,0xfd,0x2f,0x75,0x11,0x77,0xff,0x68,0x1a})
#define PB_ADV_ADDR   ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x02})
#endif

#ifdef LIGHT_SEC2
#define SEC	 ((uint8_t[16]){0x12,0xfc,0x31,0x28,0x73,0x0f,0x5b,0x84,0xe9,0x6c,0x1a,0x2c,0xac,0x5f,0xb8,0x93})
#define PB_ADV_ADDR   ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x01})
#endif

#ifdef LIGHT_SEC1
#define SEC	 ((uint8_t[16]){0xde,0x9d,0x7a,0xdb,0xa7,0xef,0x12,0x67,0x00,0xec,0xa6,0x56,0x68,0x93,0x89,0x4c})
#define PB_ADV_ADDR   ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x00})
#endif

uint8_t mesh_bt_dev_name[8] = {'I','n','g','c','h','i','p','\0'};
extern uint32_t TM_AUTHEN_VAL_LEN;

#define PB_GATT_MAC_ADDR                       ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x010})
#define OTA_MAC_ADDR                           ((uint8_t[6]){0x01,0x01,0x01,0x04,0x05,0x06})

uint8_t param[32];

void mesh_platform_setup()
{
    memcpy(param,&BLE_MESH_DEV_PRODUCT_ID,sizeof(uint32_t));
    memcpy(param+4,&TM_AUTHEN_VAL_LEN,sizeof(uint32_t));
    memcpy(param+8,SEC,16);
    mesh_platform_config(1,PB_ADV_ADDR,param);
    mesh_platform_config(2,PB_GATT_MAC_ADDR,NULL);
}

uint32_t setup_profile(void *data, void *user_data)
{
    mesh_env_init();
    init_service();
    att_server_init(att_read_callback, att_write_callback);
    hci_event_callback_registration.callback = &user_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(&user_packet_handler);
    mesh_set_dev_name((char*)mesh_bt_dev_name);
extern void create_mesh_task (void);
    create_mesh_task();
    return 0;
}

uint8_t mesh_toggle =0;
extern uint8_t* mesh_flashpage_valid_check(uint16_t db_size, uint32_t* last_addr);
#ifndef DB_CAPACITY_SIZE
    #define DB_CAPACITY_SIZE 1021
#endif
/**
 * @brief The whole data. size is the total size of data (size itself excluded)
 */
typedef struct kv_db
{
    uint16_t size;
    uint8_t  data[DB_CAPACITY_SIZE + 2 + 1];
} kv_db_t;

extern int flash_erase_and_write(uint8_t *flash_area, uint32_t off, uint32_t *src,uint32_t len);
int db_mesh_write_to_flash(const void *db, const int size)
{
    program_flash(MESH_FLASH_ADDRESS, db, size);
    return KV_OK;
}

int read_mesh_from_flash(void *db, const int max_size)
{
    memcpy((uint8_t*)db,(uint8_t*)(MESH_FLASH_ADDRESS), max_size);
    return 0;
}
extern uint32_t uart1_isr(void *user_data);

//#ifndef DEV_BOARD
extern kb_report_trigger_send(uint8_t keynum);
extern void kb_state_changed(uint8_t key);

extern kb_report_t report;

uint32_t gpio_isr(void *user_data)
{
    uint32_t current = ~GIO_ReadAll();

    // report which keys are pressed
    if (current & (1 << KB_KEY_1))
    {
        printf("KB_KEY_1_isr!!\r\n");
        kb_state_changed(1);
    }
    else if (current & (1 << KB_KEY_2))
    {
        printf("KB_KEY_2_isr!!\r\n");
        kb_state_changed(2);
    }
    else;
    GIO_ClearAllIntStatus();
    return 0;
}
//#endif

int app_main()
{
    platform_set_rf_clk_source(0);  	//use external crystal

    platform_set_irq_callback(PLATFORM_CB_IRQ_GPIO, gpio_isr, NULL);//key isr

    setup_peripherals();
    kv_init(db_mesh_write_to_flash,read_mesh_from_flash);  // attention !! : must set the init func here prior to func fast_switch_monitor.
    fast_switch_monitor();
    sysSetPublicDeviceAddr(addr);
    apPWM_Initialize();
    //support uart for mesh verification.
    platform_set_irq_callback(PLATFORM_CB_IRQ_UART0, NULL, NULL);
    //platform_set_irq_callback(PLATFORM_CB_IRQ_UART1, uart1_isr, NULL);
    // setup putc handle
    platform_set_evt_callback(PLATFORM_CB_EVT_PUTC, (f_platform_evt_cb)cb_putc, NULL);

    platform_set_evt_callback(PLATFORM_CB_EVT_PROFILE_INIT, setup_profile, NULL);

    platform_config(PLATFORM_CFG_LOG_HCI,0);

    extern void console_out(const char *str, int cnt);
    set_mesh_uart_output_func(console_out);
    mesh_trace_config(PROV_FEA|ACC_LAYER,7);//|PROV_FEA|ACC_LAYER|NET_LAYER
    return 0;
}

