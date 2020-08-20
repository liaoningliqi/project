/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BT_MESH_LIGHT_MODEL_H
#define __BT_MESH_LIGHT_MODEL_H

#include "mesh_def.h"

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


/** 
 * @brief callback for the gen_onoff models status check.
 *
 * @param[in] model :  struct bt_mesh_model typed model
 *
 * @param[ou] status : current light on/off status
 *
 * @return 0 : EOK
 */
int light_model_gen_onoff_get(struct bt_mesh_model *model, u8_t *state);

/** 
 * @brief callback to set the current light on/off status
 *
 * @param[in] model :  struct bt_mesh_model typed model
 *
 * @param[in] status : expect to the status of light on/off
 *
 * @return 0 : EOK
 */
int light_model_gen_onoff_set(struct bt_mesh_model *model, u8_t state);

/** 
 * @brief callback to set get the lightness level
 *
 * @param[in] model :  struct bt_mesh_model typed model
 *
 * @param[ou] level :  get the lightness level
 *
 * @return 0 : EOK
 */
int light_model_gen_level_get(struct bt_mesh_model *model, s16_t *level);

/** 
 * @brief callback to set get the lightness level
 *
 * @param[in] model :  struct bt_mesh_model typed model
 *
 * @param[in] level :  set the lightness level for the light model
 *
 * @return 0 : EOK
 */
int light_model_gen_level_set(struct bt_mesh_model *model, s16_t level);
int light_model_light_lightness_get(struct bt_mesh_model *model, u16_t *p_lightness,u16_t *lightness,u8_t* remain);
int light_model_light_lightness_set(struct bt_mesh_model *model, u16_t lightness);
void light_gpio_init(void);

int light_model_light_HSL_get(struct bt_mesh_model *model, uint16_t *hue, uint16_t *sa, uint16_t *lightness,uint8_t *trans);
int light_model_light_HSL_set(struct bt_mesh_model *model,hsl_val_t* val);

//light ctl temperature
int light_model_ctl_temp_get(struct bt_mesh_model *model,uint16_t *temp , s16_t* deluv, uint16_t* tar_temp, s16_t* tar_deluv, uint8_t* time);
int light_model_ctl_temp_set(struct bt_mesh_model *model,uint16_t temp ,  s16_t deluv, uint8_t TID, uint8_t transit, uint8_t delay);

//light ctl
int light_model_ctl_get(struct bt_mesh_model *model,uint16_t *lightness,uint16_t *temp , uint16_t* tar_lightness, uint16_t* tar_temp, uint8_t* time);
int light_model_ctl_set(struct bt_mesh_model *model,uint16_t lightness , uint16_t temp, s16_t deluv,uint8_t TID, uint8_t transit, uint8_t delay);

int unbind_light_mode_run(uint32_t duration);

/**
 * @brief use this API could do some operations related to light model status, once the device has been provisioned 
 *
 * @return 0 : EOK
 */
int light_provsioned_complete(void);

/**
 * @brief callback by light model to update the status.
 *
 * @return 0 : EOK
 */
void update_light_state(void);

/**
 * @brief configue the PWM parameters according to PIN.
 *
 * @param VAL  PWM value scope in [0,255]
 *
 * @param PIN  GPIO number
 *
 * @return 0: EOK
 *
 * @note this API is used to modify the PWM value according to the actual customer GPIO design connected to peripherial.
 */
uint32_t PWM_CONFIG(uint8_t VAL,uint8_t PIN);

/**
 * @brief light model initialization
 *
 * @return 0: EOK
 *
 */
int model_conf_init(void);

void light_ctl_adjust(uint8_t WW, uint8_t CW,uint16_t light);

void HSL_PWM_LED(uint16_t hue,uint16_t sa, uint16_t light);

int light_reset(uint16_t light, uint8_t cw);

/**
 * @brief use this API to configure the light status once power on.
 *
 * @return 0 : EOK
 *
 * @note  customer could get the last light on/off status ,lightness etc parameters from flash , and configure them to the light ,after power on.
 */
int light_power_on(void);
#endif
