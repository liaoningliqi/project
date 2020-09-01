
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BLE_MESH_H__
#define __BLE_MESH_H__

#include <stdint.h>

#if defined __cplusplus
    extern "C" {
#endif

extern uint32_t TM_AUTHEN_VAL_LEN;

struct bt_mesh_model * get_model_by_id(uint16_t id);
int mesh_env_init(void);
void create_mesh_task (void);
void nimble_port_init(void);

#if defined __cplusplus
    }
#endif

#endif


