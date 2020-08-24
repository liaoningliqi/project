
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BLE_MESH_LIGHT_MODEL_H__
#define __BLE_MESH_LIGHT_MODEL_H__

#include "mesh_def.h"

#if defined __cplusplus
    extern "C" {
#endif

typedef struct HSL_VAL
{
    uint16_t lightness;
    uint16_t hue;
    uint16_t sa;
    uint8_t  TID;
    uint8_t  transtime;
    uint8_t  delay;
}hsl_val_t;

struct cached_data_t
{
    uint16_t lightness;
    uint16_t temperature;
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t status;
};

extern bool g_ble_mesh_light_model_status_set;
extern u8_t g_ble_mesh_light_model_onoff_state;

int light_power_on(void);
uint32_t PWM_CONFIG(uint8_t VAL,uint8_t PIN);
void light_ctl_adjust(uint8_t WW, uint8_t CW,uint16_t light);
int unbind_light_mode_run(uint32_t duration);
void HSL_PWM_LED(uint16_t hue,uint16_t sa, uint16_t light);
int light_reset(uint16_t light, uint8_t cw);
int light_provsioned_complete(void);
int ble_mesh_light_model_conf_init(void);

#if defined __cplusplus
    }
#endif

#endif
