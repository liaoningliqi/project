/*
 * Copyright (c) 2018 INGCHIPS MESH
 */

#include <stdio.h>
#include "ble_mesh_light_model.h"
#include "state_binding.h"
#include "device_composition.h"
#include "transition.h"
#include "eflash.h"
#include "mesh_api.h"
#include "kv_storage.h"
#include "profile.h"

#include "chip_peripherals.h"
#include "..\..\project_common\project_common.h"

#if defined __cplusplus
    extern "C" {
#endif

#define MAX_TEMP 0x4E20
#define MIN_TEMP 0x0320
#define DIS_TEMP (MAX_TEMP - MIN_TEMP)
#define MID_TEMP ((MAX_TEMP + MIN_TEMP) / 2)

#define HIGH_LEVEL_WORK      (0)
#define LIGHT_TEMP_CIRCUIT   (1)

#define LED_R  PWM_R
#define LED_G  PWM_G
#define LED_B  PWM_B

#define PERA_THRESHOLD 0x100

u8_t g_ble_mesh_light_model_onoff_state;
bool g_ble_mesh_light_model_status_set = BT_PRIVT_FALSE;

static bool last_light_status = BT_PRIVT_FALSE;
static s16_t gen_level_state;
static float R,G,B;
static uint8_t Rv, Gv,Bv;
static uint32_t GWW,GCW;
static u16_t cur_ctl_light = 0;
static u16_t due_light =0;
static s16_t due_deluv = 0;
static u16_t cur_ctl_temp = MID_TEMP;
static u16_t due_temp =0;
static struct k_delayed_work breath_mode;
static struct k_delayed_work breath_work;
static struct k_delayed_work flash_work;
static struct cached_data_t cust_mem;

static uint16_t curr_hue = 0;
static uint16_t curr_sa = 0;
static uint16_t curr_lightness =0;
static uint8_t remain;

static volatile bool breath_mode_is_run = BT_PRIVT_FALSE;

void light2save(struct ble_npl_event *ev)
{
    dbg_printf("flash write light param\n");
    kv_put(CUST_CONF_FLASH_INDEX,(uint8_t*)&cust_mem,sizeof(cust_mem));
}

void update_light_state(void)
{
    u32_t power, color;
    uint32_t cach_value =0;
    power = lightness;
    uint8_t ratio = 0;
    uint8_t set_lightness = 0;
    color = 100 * ((float)(temperature + 32768) / 65535);
    GWW = color;
    GCW = 100-color;

    if (power>0) {
        g_ble_mesh_light_model_onoff_state = 1;
    } else {
        g_ble_mesh_light_model_onoff_state = 0;
    }

    dbg_printf("Light state: onoff=%d lightness=0x%04x CW= 0x%x\n", g_ble_mesh_light_model_onoff_state, (u16_t)power,GCW);
    if (1 == g_ble_mesh_light_model_onoff_state) {
        ratio = (uint32_t)(lightness *100) >> 16;
        set_lightness =  ((uint32_t)(ratio << 8)) / 100;
        Rv = set_lightness;
        Gv = set_lightness;
        Bv = set_lightness;
        GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
        GIO_WriteValue(PIN_SDI, 0);
        set_led_color(set_lightness, set_lightness, set_lightness);
        if(last_lightness == lightness) {
            goto light_save;
        }
        return;
    } else {
        GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
        GIO_WriteValue(PIN_SDI, 0);
        set_led_color(0, 0, 0);
        goto light_save;
    }

light_save:
#if (FLASH_ENABLE)
        cach_value = last_lightness << 16 | GCW << 8 | GWW;
        dbg_printf("write light 0x%x\n", cach_value);
        cust_mem.lightness = last_lightness;
        cust_mem.temperature = (GCW << 8 | GWW);
        cust_mem.R = Rv;
        cust_mem.G = Gv;
        cust_mem.B = Bv;
        cust_mem.status = g_ble_mesh_light_model_onoff_state;
        //cached to flash
        k_delayed_work_submit(&flash_work, 500);
        return;
#endif
}

int ble_mesh_light_model_gen_onoff_get(struct bt_mesh_model *model, u8_t *state)
{
    *state = g_ble_mesh_light_model_onoff_state;
    return 0;
}

int ble_mesh_light_model_gen_onoff_set(struct bt_mesh_model *model, u8_t state)
{
    g_ble_mesh_light_model_onoff_state = state;
    update_light_state();
    return 0;
}

int ble_mesh_light_model_gen_level_get(struct bt_mesh_model *model, s16_t *level)
{
    *level = gen_level_state;
    dbg_printf("the curren level %d %d\n", gen_level_state, *level);
    return 0;
}

int ble_mesh_light_model_gen_level_set(struct bt_mesh_model *model, s16_t level)
{
    uint8_t lightratio;
    uint16_t light = level + 32768;
    gen_level_state = level;
    uint32_t loc_r,loc_g,loc_b;
    uint32_t CW,WW;
    CW = GCW;
    WW = GWW;

    dbg_printf("the level is %d\n",level);
    if ((u16_t)light > 0x0000) {
#if (LIGHT_TEMP_CIRCUIT == 0)
        CW = (GCW == 0) ? 10 : GCW;
        WW = (GWW == 0) ? 10 : GWW;
#endif
        g_ble_mesh_light_model_onoff_state = 1;
        if (light == 65535) {
            lightratio = 100;
        } else {
            lightratio = light * 100 / 65535;
            uint16_t temp = (light * 100) & 0xffff;
            if (temp > 32767) {
                lightratio += 1;
            }
        }
        dbg_printf("lightratio is 0x%x\n",lightratio);
        //configure the PWM input
        loc_r =  255 * lightratio / 100;
        loc_g =  255 * lightratio / 100;
        loc_b =  255 * lightratio / 100;
#if (LIGHT_TEMP_CIRCUIT == 0)
        CW = (CW * loc_r*100 / Rv) / 100;
        te = (CW * loc_r*100 / Rv) % 100;
        if (te >= 50) {
            CW += 1;
        }
        WW = (WW * loc_r * 100 / Rv) / 100;
        te = (WW * loc_r * 100 / Rv) % 100;
        if (te > 50) {
            WW += 1;
        }
        dbg_printf("CW %d  WW %d \n", CW, WW);
        if (CW > 100) {
            CW =100;
        }
        if (WW > 100) {
            WW =100;
        }
#endif
        PWM_CONFIG(loc_r, LED_R);
        PWM_CONFIG(loc_g, LED_G);
        PWM_CONFIG(loc_b, LED_B);
#if (LIGHT_TEMP_CIRCUIT)
        light_ctl_adjust(WW, CW, light);
#else
        PWM_CONFIG(WW, PWM_WARM);
        PWM_CONFIG(CW, PWM_COLD);
#endif
        Rv = loc_r;
        Gv = loc_g;
        Bv = loc_b;
        GWW = WW;
        GCW = CW;

    } else if ((u16_t)gen_level_state == 0x0000) {
        g_ble_mesh_light_model_onoff_state = 0;
        Rv = Gv = Bv = 0;
        update_light_state();
    }

    return 0;
}

int ble_mesh_light_model_HSL_hue_set(struct bt_mesh_model *model, uint16_t level)
{
    gen_level_state = level;
    if ((u16_t)gen_level_state > 0x0000) {
        g_ble_mesh_light_model_onoff_state = 1;
    }
    if ((u16_t)gen_level_state == 0x0000) {
        g_ble_mesh_light_model_onoff_state = 0;
    }
    //call PWM to adjust the lightness
    update_light_state();
    return 0;
}

int ble_mesh_light_model_light_lightness_get(struct bt_mesh_model *model, u16_t *p_lightness,u16_t *lightness,u8_t *remain)
{
    s16_t temp = 0;
    temp = gen_level_state + 32768;
    *lightness = htole16(temp);
    *p_lightness = *lightness;
    *remain =0;
    return 0;
}

int ble_mesh_light_model_light_lightness_set(struct bt_mesh_model *model, u16_t light)
{
    s16_t light1 = light - 32768;
    return ble_mesh_light_model_gen_level_set(model, light1);
}

int ble_mesh_light_model_light_HSL_get(struct bt_mesh_model *model, uint16_t *hue, uint16_t *sa, uint16_t *lightness,uint8_t *trans)
{
    *hue = curr_hue;
    *sa = curr_sa;
    *lightness = curr_lightness;
    *trans =  remain;
    return 0;
}

int ble_mesh_light_model_light_HSL_set(struct bt_mesh_model *model,hsl_val_t* val)
{
    //set the value of HSL to notify the driven
    printk("start\n");
    HSL_PWM_LED(val->hue, val->sa, val->lightness);
    return 0;
}

int light_rgb_set(uint8_t rv, uint8_t gv, uint8_t bv )
{
    uint16_t rl, gl, bl;
    printk("uint8_t %d %d %d\n", rl, gl, bl);
    PWM_CONFIG(rv, LED_R);
    PWM_CONFIG(gv, LED_G);
    PWM_CONFIG(bv, LED_B);
    return 0;
}

int lingtness_set(uint16_t light)
{
    uint16_t light1 = 0;
    light1 = light * 100 >> 16;
    PWM_CONFIG(light1, LED_R);
    PWM_CONFIG(light1, LED_G);
    PWM_CONFIG(light1, LED_B);
    return 0;
}

float Hue_2_RGB(float v1, float v2, float vH) //Function Hue_2_RGB
{
    printk("Hue_2_RGB: v1 %f v2 %f vH %f\n", v1, v2, vH);
    if (vH < 0) {
        vH += 1;
    }
    if (vH > 1) {
        vH -= 1;
    }
    if ((6.0 * vH) < 1) {
        return (v1 + (v2 - v1) * 6.0 * vH);
    }
    if ((2.0 * vH) < 1) {
        return (v2);
    }
    if ((3.0 * vH) < 2) {
        return (v1 + (v2 - v1) * ((2.0 / 3.0) - vH) * 6.0);
    }
    return ( v1 );
}

typedef struct RGB_VAL
{
    uint8_t R_VAL;
    uint8_t G_VAL;
    uint8_t B_VAL;
}rgb_val_t;

void HSL_2_RGB(uint16_t hue, uint16_t sa, uint16_t light)
{
    float H, S, L, var_2, var_1;

    H = hue / 65535.0;
    S = sa / 65535.0;
    L = light / 65535.0;
    printk("input H %f S %f L %f\n",H,S,L);
    if (S == 0) {
        R = L;
        G = L;
        B = L;
    } else {
        if (L < 0.5) {
            var_2 = L * ( 1 + S );
        } else {
            var_2 = ( L + S ) - ( S * L );
        }
        var_1 = 2 * L - var_2;
        printk("var_2 %f var_1 %f\n", var_2, var_1);
        R = Hue_2_RGB(var_1, var_2, H + (1.0 / 3.0));
        G = Hue_2_RGB(var_1, var_2, H );
        B = Hue_2_RGB(var_1, var_2, H - (1.0 / 3.0));
    }
    printk("HSL: R: %f G:%f B:%f", R, G, B);
}

void HSL_PWM_LED(uint16_t hue, uint16_t sa, uint16_t light)
{
    HSL_2_RGB(hue, sa, light);
    Rv = (R * 255.0 + 0.5);
    printk("RV %d\n", Rv);
    Gv = G * 255.0 + 0.5;
    Bv = B * 255.0 + 0.5;
    light_rgb_set(Rv, Gv, Bv);
}
uint32_t pwm_value_update(uint8_t VAL, uint8_t PIN)
{
    uint32_t VAL2;
    uint32_t period = PERA_THRESHOLD;

#if (HIGH_LEVEL_WORK)
     if (0 == (PIN % 2)) {
        VAL2 = period - VAL * period / 100;
     } else {
        VAL2 = VAL * period / 100;
     }
#else
     if (0 == (PIN % 2)) {
        VAL2 = VAL * period / 100;
     } else {
        VAL2 = period - VAL * period / 100L;
     }
#endif
     if (VAL2 == 256) {
         return 255;
     } else {
        return VAL2;
     }
}
uint32_t PWM_CONFIG(uint8_t VAL, uint8_t PIN)
{
    uint32_t VAL2;
    uint32_t period = PERA_THRESHOLD;

    //one PWM is mapped to 2 opsition way's signal.
    if ((PIN == PWM_WARM ) || (PIN == PWM_COLD)) { // this 2 PIN, represent 0-100
#if (HIGH_LEVEL_WORK)
        if (0 == PIN % 2) {
            VAL2 = period - VAL * period / 100; // low is lighten, high is off,
        } else {
            VAL2 = VAL * period / 100;
        }
#else
        if (0 == PIN % 2) {
            VAL2 = VAL * period / 100; // low is lighten, high is off,
        } else {
            VAL2 = period - VAL * period / 100;
        }
#endif
        PWM_SetHighThreshold(PIN >> 1, 0, VAL2);
    } else { // represent RGB : 0---256)
#if (HIGH_LEVEL_WORK)
        if (0 == (PIN % 2)) {
            VAL2 = 255 - VAL;
        } else {
            VAL2 = VAL;
        }
#else
        if (0 == (PIN % 2)) {
            VAL2 = VAL;
        } else {
            VAL2 = 255 - VAL;
        }
#endif
        if (VAL2==255) {
            PWM_SetHighThreshold(PIN   >> 1, 0, PERA_THRESHOLD);
        } else {
            PWM_SetHighThreshold(PIN   >> 1, 0, VAL2);
        }
    }
    return 0;
}

int ble_mesh_light_model_ctl_temp_get(struct bt_mesh_model *model, uint16_t *temp, s16_t* deluv, uint16_t* tar_temp, s16_t* tar_deluv, uint8_t* time)
{
    return 0;
}

int ble_mesh_light_model_ctl_temp_set(struct bt_mesh_model *model, uint16_t temp, s16_t deluv, uint8_t TID, uint8_t transit, uint8_t delay)
{
    return 0;
}

void light_ctl_adjust(uint8_t WW, uint8_t CW, uint16_t light)
{
    uint32_t mulrat = 0;
    uint32_t re = light *100;

    if (WW != 0 || CW != 0) {
        GWW = WW;
        GCW = CW;
    }

    if (light == 65535) {
        mulrat = 100;
    } else {
        mulrat = re >> 16;
        if((re & 0xffff) > 32768)
        mulrat += 1;
    }
#if (LIGHT_TEMP_CIRCUIT)
    uint32_t temp_rat = 0;
    uint32_t me = CW * 100;
    if(CW == 0 && WW == 0 && light == 0) { //light all off;
        temp_rat =0;
        mulrat =0;
    } else {
        temp_rat = me / (CW + WW);
        if(me % 100 > 50)
            temp_rat += 1;
    }
    dbg_printf("mulrat 0x%x,temp_rat 0x%x\n", mulrat, temp_rat);
    PWM_CONFIG(mulrat, PWM_COLD); // CW for LIGHTNESS
    PWM_CONFIG(temp_rat, PWM_WARM); // WW for temperatue;
#else
    PWM_CONFIG(temp1 = WW * mulrat / 100, PWM_WARM);
    PWM_CONFIG(temp2 = CW * mulrat / 100, PWM_COLD);
    dbg_printf("final state WW %d, CW %d\n", temp1, temp2);
#endif
}

void update_light_ctl(uint16_t light, uint16_t temp, s16_t delua)
{
    uint8_t CW = 0;
    uint8_t WW = 0;
    uint32_t te = 0;

    dbg_printf("Light ctl state: lightness =%d temp =%d deluv=%d\n", light, temp, delua);

    CW = (temp - MIN_TEMP) * 100 / DIS_TEMP;
    te = ((temp - MIN_TEMP) * 100) % DIS_TEMP;
    if (te > (DIS_TEMP / 2)) {
        CW += 1;
    }
    WW = (DIS_TEMP - temp + MIN_TEMP) * 100 / DIS_TEMP;
    te = (DIS_TEMP - temp + MIN_TEMP) * 100 % DIS_TEMP;
    if (te > (DIS_TEMP / 2)) {
        WW += 1;
    }
    dbg_printf("updated CW %d WW %d\n", CW, WW);
    cur_ctl_temp = temp;
    cur_ctl_light = light;
    gen_level_state = (s16_t)(light - 32768);
    PWM_CONFIG(Rv, LED_R);
    PWM_CONFIG(Gv, LED_G);
    PWM_CONFIG(Bv, LED_B);
    light_ctl_adjust(WW, CW, light);
}

int ble_mesh_light_model_ctl_get(struct bt_mesh_model *model,uint16_t *lightness,uint16_t *temp , uint16_t* tar_lightness, uint16_t* tar_temp, uint8_t* time)
{
    *lightness = cur_ctl_light;
    *temp = cur_ctl_temp;
    *tar_temp = cur_ctl_temp;
    *tar_lightness = cur_ctl_light;
    *time = 0;
    return 0;
}

int ble_mesh_light_model_ctl_set(struct bt_mesh_model *model,uint16_t light , uint16_t temp, s16_t deluv,uint8_t TID, uint8_t transit, uint8_t delay)
{
    due_light = light;
    due_temp = temp;
    due_deluv = deluv;
    update_light_ctl(due_light, due_temp, due_deluv);
    return 0;
}

int light_power_on()
{
    uint32_t lightstatus = 0;

    g_ble_mesh_light_model_onoff_state = 1;
    uint32_t count_m;
    uint32_t count_n;
    uint8_t cw=0;
#if (FLASH_ENABLE)
    struct cached_data_t ctword={0};
    int16_t len;
    uint8_t* db = NULL;
    uint16_t pack_cw = 5;

    db = kv_get(CUST_CONF_FLASH_INDEX, &len);
    if (db && (len >0)) {
        memcpy(&ctword,(uint8_t*)db,sizeof(cust_mem));
    }
    memcpy(&lightstatus, &ctword, 4);

    if (lightstatus == 0 || lightstatus == 0xffffffff) {
        GCW = 50;
        GWW = 50;
        gen_level_state = 32767;

        uint16_t pack_t = (65535 - lightstatus >> 16) / 2000;
        if (pack_t == 0) {
            pack_t = 1;
        }
        for(count_n=0;count_n<2000;count_n++) {
            cw = pack_cw * count_n / 200;
            light_reset(65535, cw);
            for(count_m = 0; count_m < 7062; count_m++) {
            }
        }
        //revert the lightness and temperature from flash.
        last_lightness =  0xffff;
        temperature = 0;
    } else {
        gen_level_state = (s16_t)((lightstatus & 0xffff) - 32768);
        dbg_printf("last light status 0x%x \n",ctword.status);
        if (ctword.status) {
            set_led_color(50, 50, 50);
        } else {
            set_led_color(0, 0, 0);
        }
        g_ble_mesh_light_model_status_set = BT_PRIVT_TRUE;
        last_light_status = ctword.status;
    }
#endif
    return 0;
}

int light_reset(uint16_t light, uint8_t cw)
{
    uint32_t re = light * 100;
    uint32_t mulrat = 0;
    uint8_t val = 0;

    if(light == 0) {
        mulrat = 0;
    } else {
        if (light == 65535) {
            mulrat = 100;
        } else {
            mulrat = re >> 16;
            if ((re & 0xffff) > 32768) {
                mulrat += 1;
            }
        }
    }
    val = pwm_value_update(mulrat, PIN_SDI);
    set_led_color(val, val, val);
    return 0;
}

void light_default_var_init(void)
{
    gen_def_trans_time_srv_user_data.tt = 0x00;

    gen_power_onoff_srv_user_data.onpowerup = STATE_RESTORE;

    light_lightness_srv_user_data.light_range_min = LIGHTNESS_MIN;
    light_lightness_srv_user_data.light_range_max = LIGHTNESS_MAX;
    light_lightness_srv_user_data.last = LIGHTNESS_MAX;
    light_lightness_srv_user_data.def = LIGHTNESS_MAX;

    light_ctl_srv_user_data.temp_range_min = TEMP_MIN;
    light_ctl_srv_user_data.temp_range_max = TEMP_MAX;

    light_ctl_srv_user_data.temp_def = TEMP_MIN;
    light_ctl_srv_user_data.lightness_temp_last = (uint32_t)LIGHTNESS_MAX << 16 | TEMP_MIN;
}

void light_default_status_init(void)
{
    uint32_t t1 = ((gen_level_state + 32768));
    uint32_t t2 = (MIN_TEMP +  DIS_TEMP * (100 - GCW) / 100);

    last_lightness = t1;
    light_ctl_srv_user_data.lightness_temp_def = light_ctl_srv_user_data.lightness_temp_last = (t1 << 16) | t2;
    lightness = (u16_t)(light_ctl_srv_user_data.lightness_temp_last >> 16);

    dbg_printf("t1 0x%x t2 0x%x 3 0x%x\n", t1, t2, light_ctl_srv_user_data.lightness_temp_last);

    if (last_light_status) {
        gen_onoff_srv_root_user_data.onoff = STATE_ON;
        gen_onoff_srv_root_user_data.target_onoff = STATE_ON; // added by dengyiyn, for first status check
    } else {
        gen_onoff_srv_root_user_data.onoff = STATE_OFF;
        gen_onoff_srv_root_user_data.target_onoff = STATE_OFF; // added by dengyiyun, for first status check
    }

    if (light_ctl_srv_user_data.lightness_temp_def) {
        light_ctl_srv_user_data.lightness_def = (u16_t)(light_ctl_srv_user_data.lightness_temp_def >> 16);
        light_ctl_srv_user_data.temp_def = (u16_t)(light_ctl_srv_user_data.lightness_temp_def);
    }

    light_lightness_srv_user_data.def = light_ctl_srv_user_data.lightness_def;
    light_ctl_srv_user_data.temp = light_ctl_srv_user_data.temp_def;

    if (light_lightness_srv_user_data.lightness_range) {
        light_lightness_srv_user_data.light_range_max = (u16_t)(light_lightness_srv_user_data.lightness_range >> 16);
        light_lightness_srv_user_data.light_range_min = (u16_t)(light_lightness_srv_user_data.lightness_range);
    }

    if (light_ctl_srv_user_data.temperature_range) {
        light_ctl_srv_user_data.temp_range_max = (u16_t)(light_ctl_srv_user_data.temperature_range >> 16);
        light_ctl_srv_user_data.temp_range_min = (u16_t)(light_ctl_srv_user_data.temperature_range);
    }

    switch (gen_power_onoff_srv_user_data.onpowerup) {
        case STATE_OFF:
            gen_onoff_srv_root_user_data.onoff = STATE_OFF;
            state_binding(ONOFF, ONOFF_TEMP);
            break;
        case STATE_DEFAULT:
            gen_onoff_srv_root_user_data.onoff = STATE_ON;
            state_binding(ONOFF, ONOFF_TEMP);
            break;
        case STATE_RESTORE:
            light_lightness_srv_user_data.last = (u16_t)(light_ctl_srv_user_data.lightness_temp_last >> 16);
            gen_level_srv_root_user_data.level = light_lightness_srv_user_data.last - 32768;
            light_ctl_srv_user_data.temp = (u16_t)(light_ctl_srv_user_data.lightness_temp_last);
            light_lightness_srv_user_data.actual = light_lightness_srv_user_data.last;
            state_binding(ONPOWERUP, ONOFF_TEMP);
            break;
    }

    default_tt = gen_def_trans_time_srv_user_data.tt;
}

int ble_mesh_light_model_conf_init()
{
    light_default_var_init();
    transition_timers_init();
    light_default_status_init();

    k_delayed_work_init(&flash_work, light2save);
    return 0;
}

int model_breath_mode()
{
    uint8_t cw = 0;
    uint16_t light = 0;
    uint32_t count_m, count_n = 0;
    uint16_t pack_t = 65535 / 2000;
    uint16_t pack_cw = 5;

    GCW = 50;
    GWW = 50;
    gen_level_state = 32767;

    //for lightness to darkness
    if (65535 % 2000 > 2000) {
        pack_t += 1;
    }

    for (count_n = 0; count_n < 2000; count_n++) {
        if (count_n == 1999) {
            light = 0;
            cw = 0;
        } else {
            uint32_t m = pack_t * count_n;
            uint32_t n = pack_cw * (count_n / 200);
            light = (65535> m) ? 65535- m : 0;
            cw = (50 > n) ? 50 - n : 0;
            cw = (light == 0) ? 0 : 50 - n;
            if (cw > GCW) {
                cw = GCW;
            }
        }
        light_reset(light, cw);
        for (count_m = 0; count_m < 1500; count_m++) {
        }
    }

    for (count_n = 0; count_n < 2000; count_n++) {
        if (count_n == 1999) {
            light = 65535;
            cw = 50;
        } else {
            uint32_t m = pack_t * count_n;
            uint32_t n = pack_cw * (count_n / 200);
            light =  (m > 65535) ? 65535 : m;
            cw = (n > 50) ? 50 : n;
            if (cw > GCW) {
                cw = GCW;
            }
        }
        light_reset(light, cw);
        for(count_m = 0; count_m < 1500; count_m++) {
        }
    }
    return 0;
}

void breath_mode_expire(struct ble_npl_event *work)
{
    breath_mode_is_run = BT_PRIVT_FALSE;
    dbg_printf("stop breath mode\n");
    return;
}

bool IS_BREATH_RUN()
{
    return breath_mode_is_run;
}

void breath_mode_work(struct ble_npl_event *work)
{
    if (IS_BREATH_RUN()) {
        k_delayed_work_submit(&breath_work, 2000);
        model_breath_mode();
    } else {
        dbg_printf("unbind mode finish\n");
    }
}

int unbind_light_mode_run(uint32_t duration)
{
    k_delayed_work_init(&breath_mode, breath_mode_expire);
    k_delayed_work_init(&breath_work, breath_mode_work);
    k_delayed_work_submit(&breath_mode, duration);
    k_delayed_work_submit(&breath_work, 2000);
    breath_mode_is_run = BT_PRIVT_TRUE;
    dbg_printf("unbind mode start\n");
    return 1;

}

int light_provsioned_complete()
{
    if (IS_BREATH_RUN()) {
        breath_mode_is_run = BT_PRIVT_FALSE;
        k_delayed_work_cancel(&breath_mode);
        dbg_printf("breath mode stopped\n");
    }
    return 0;
}

#if defined __cplusplus
    }
#endif

