
#include "FreeRTOS.h"
#include "timers.h"

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

#include "BLE_mesh.h"
#include "chip_peripherals.h"
#include "mesh_flash_oper.h"
#include "..\..\project_common\project_common.h"

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

#define PB_GATT_MAC_ADDR                       ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x010})

extern uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
extern btstack_packet_callback_registration_t hci_event_callback_registration;
extern int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode,  uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);
extern void user_packet_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size);
extern const unsigned char addr[6];
extern int mesh_env_init(void);
extern uint32_t TM_AUTHEN_VAL_LEN;
extern void create_mesh_task (void);
extern void console_out(const char *str, int cnt);

__weak void __aeabi_assert(const char *a ,const char* b, int c)
{
    printf("assert\n");
    for (;;);
}

void delay(int cycles)
{
    int i;
    for (i = 0; i < cycles; i++)
    {
        __nop();
    }
}

/*below for TIANMAO configuration*/
static uint32_t BLE_MESH_DEV_PRODUCT_ID = 5349350;
static uint8_t mesh_bt_dev_name[8] = {'I','n','g','c','h','i','p','\0'};
static uint8_t param[32];

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
    create_mesh_task();
    return 0;
}

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

    kv_init(db_mesh_write_to_flash,read_mesh_from_flash);  // attention !! : must set the init func here prior to func fast_switch_monitor.
    fast_switch_monitor();
    sysSetPublicDeviceAddr(addr);
    apPWM_Initialize();

    platform_set_evt_callback(PLATFORM_CB_EVT_PROFILE_INIT, setup_profile, NULL);

    platform_config(PLATFORM_CFG_LOG_HCI,0);

    set_mesh_uart_output_func(console_out);
    mesh_trace_config(PROV_FEA|ACC_LAYER,7);//|PROV_FEA|ACC_LAYER|NET_LAYER
    return 0;
}

