
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BLE_MESH_LIGHT_MODEL_H__
#define __BLE_MESH_LIGHT_MODEL_H__

#include "mesh_def.h"

#if defined __cplusplus
    extern "C" {
#endif

extern bool g_ble_mesh_light_model_status_set;
extern u8_t g_ble_mesh_light_model_onoff_state;

int ble_mesh_light_model_power_on(void);
int ble_mesh_light_model_unbind_mode_run(uint32_t duration);
int ble_mesh_light_model_provsioned_complete(void);
int ble_mesh_light_model_conf_init(void);

#if defined __cplusplus
    }
#endif

#endif
