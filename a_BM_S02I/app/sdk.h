
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BT_AT_SDK_H__
#define __BT_AT_SDK_H__

#if defined __cplusplus
    extern "C" {
#endif

typedef struct {
    uint8_t flags[3];
    uint8_t local_name_len;
    uint8_t local_name_handle;
    uint8_t local_name[BT_AT_CMD_TTM_MODULE_NAME_MAX_LEN];
} private_module_adv_data_t;

extern uint8_t *g_rand_mac_addres;
extern private_module_adv_data_t *g_module_adv_data;
extern uint16_t g_hci_le_conn_complete_slave_handle;

void sdk_setup_peripherals(void);
void sdk_1s_timer_task(void);


#if defined __cplusplus
    }
#endif

#endif

