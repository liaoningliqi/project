
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

int light_model_gen_onoff_get(struct bt_mesh_model *model, u8_t *state);

int light_model_gen_onoff_set(struct bt_mesh_model *model, u8_t state);

int light_model_gen_level_get(struct bt_mesh_model *model, s16_t *level);

int light_model_gen_level_set(struct bt_mesh_model *model, s16_t level);
int light_model_light_lightness_get(struct bt_mesh_model *model, u16_t *p_lightness,u16_t *lightness,u8_t* remain);
int light_model_light_lightness_set(struct bt_mesh_model *model, u16_t lightness);
void light_gpio_init(void);

int light_model_light_HSL_get(struct bt_mesh_model *model, uint16_t *hue, uint16_t *sa, uint16_t *lightness,uint8_t *trans);
int light_model_light_HSL_set(struct bt_mesh_model *model,hsl_val_t* val);

int light_model_ctl_temp_get(struct bt_mesh_model *model,uint16_t *temp , s16_t* deluv, uint16_t* tar_temp, s16_t* tar_deluv, uint8_t* time);
int light_model_ctl_temp_set(struct bt_mesh_model *model,uint16_t temp ,  s16_t deluv, uint8_t TID, uint8_t transit, uint8_t delay);

int light_model_ctl_get(struct bt_mesh_model *model,uint16_t *lightness,uint16_t *temp , uint16_t* tar_lightness, uint16_t* tar_temp, uint8_t* time);
int light_model_ctl_set(struct bt_mesh_model *model,uint16_t lightness , uint16_t temp, s16_t deluv,uint8_t TID, uint8_t transit, uint8_t delay);

int unbind_light_mode_run(uint32_t duration);

int light_provsioned_complete(void);

void update_light_state(void);

uint32_t PWM_CONFIG(uint8_t VAL,uint8_t PIN);

int ble_mesh_light_model_conf_init(void);

void light_ctl_adjust(uint8_t WW, uint8_t CW,uint16_t light);

void HSL_PWM_LED(uint16_t hue,uint16_t sa, uint16_t light);

int light_reset(uint16_t light, uint8_t cw);

int light_power_on(void);
void update_light_state(void);

#if defined __cplusplus
    }
#endif

#endif
