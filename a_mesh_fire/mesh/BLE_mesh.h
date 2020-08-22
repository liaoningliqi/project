
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BLE_MESH_H__
#define __BLE_MESH_H__

#include <stdint.h>

#if defined __cplusplus
    extern "C" {
#endif

typedef void (*mesh_at_out)(const char *str, int cnt);

extern uint32_t MYNEWT_VAL_BLE_MESH_GATT_PROXY;
extern uint32_t MYNEWT_VAL_BLE_MESH_PB_GATT;
extern uint32_t TM_AUTHEN_VAL_LEN;
extern uint8_t * ptr_aestext;
extern struct k_sem ccm_sem;
extern struct k_sem aes_sem;
extern uint32_t sleep_duration;
extern config_status f_config_status;
extern uint8_t uuid_head[16];
extern uint8_t uuid_head_len;

void mesh_set_dev_name(const char *name);
void set_mesh_uart_output_func(mesh_at_out ptrfun);
void mesh_trace_config(uint16_t sel_bits,uint16_t cla_bit);
struct bt_mesh_model * get_model_by_id(uint16_t id);
void hci_cmds_put(uint16_t cmd_id, void* arg,uint8_t len);
int mesh_env_init(void);
void create_mesh_task (void);
void init_pub(void);
uint8_t model_info_pub(void);
struct bt_mesh_comp * get_comp_of_node(void);
int Host2Mesh_msg_send(uint8_t* mesh_msg ,uint8_t len);
int mesh_flash_sys_init(void);
int8_t get_aes_result(void);
void nimble_port_init(void);

#if defined __cplusplus
    }
#endif

#endif


