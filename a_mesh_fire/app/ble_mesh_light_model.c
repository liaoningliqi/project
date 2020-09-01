
/*
 * Copyright (c) 2018 INGCHIPS MESH
 */

#include <stdio.h>
#include <stdint.h>

#include "mesh_def.h"
#include "kv_storage.h"
#include "device_composition.h"
#include "state_binding.h"
#include "transition.h"

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

#define PERA_THRESHOLD 0x100

#define CUST_CONF_FLASH_INDEX (9)

struct cached_data_t
{
    uint16_t lightness;
    uint16_t temperature;
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t status;
};

u8_t g_ble_mesh_light_model_onoff_state;
bool g_ble_mesh_light_model_status_set = BT_PRIVT_FALSE;

static bool last_light_status = BT_PRIVT_FALSE;
static s16_t gen_level_state;
static uint8_t Rv, Gv,Bv;
static uint32_t GWW,GCW;
static struct k_delayed_work breath_mode;
static struct k_delayed_work breath_work;
static struct k_delayed_work flash_work;
static struct cached_data_t cust_mem;

static volatile bool breath_mode_is_run = BT_PRIVT_FALSE;

static uint32_t ble_mesh_light_model_pwm_value_update(uint8_t VAL, uint8_t PIN)
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

static int ble_mesh_light_model_light_reset(uint16_t light, uint8_t cw)
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
    val = ble_mesh_light_model_pwm_value_update(mulrat, PIN_SDI);
    set_led_color(val, val, val);
    return 0;
}

static void ble_mesh_light_model_default_var_init(void)
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

static void ble_mesh_light_model_default_status_init(void)
{
    uint32_t t1 = ((gen_level_state + 32768));
    uint32_t t2 = (MIN_TEMP + DIS_TEMP * (100 - GCW) / 100);

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

static int ble_mesh_light_model_breath_mode()
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
        ble_mesh_light_model_light_reset(light, cw);
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
        ble_mesh_light_model_light_reset(light, cw);
        for(count_m = 0; count_m < 1500; count_m++) {
        }
    }
    return 0;
}

static void ble_mesh_light_model_breath_mode_expire(struct ble_npl_event *work)
{
    breath_mode_is_run = BT_PRIVT_FALSE;
    dbg_printf("stop breath mode\n");
    return;
}

static bool ble_mesh_light_model_is_breath_run()
{
    return breath_mode_is_run;
}

static void ble_mesh_light_model_breath_mode_work(struct ble_npl_event *work)
{
    if (ble_mesh_light_model_is_breath_run()) {
        k_delayed_work_submit(&breath_work, 2000);
        ble_mesh_light_model_breath_mode();
    } else {
        dbg_printf("unbind mode finish\n");
    }
}

static void ble_mesh_light_model_light_to_save(struct ble_npl_event *ev)
{
    dbg_printf("flash write light param\n");
    kv_put(CUST_CONF_FLASH_INDEX, (uint8_t*)&cust_mem, sizeof(cust_mem));
}

int ble_mesh_light_model_conf_init()
{
    ble_mesh_light_model_default_var_init();
    transition_timers_init();
    ble_mesh_light_model_default_status_init();

    k_delayed_work_init(&flash_work, ble_mesh_light_model_light_to_save);
    return 0;
}

int ble_mesh_light_model_unbind_mode_run(uint32_t duration)
{
    k_delayed_work_init(&breath_mode, ble_mesh_light_model_breath_mode_expire);
    k_delayed_work_init(&breath_work, ble_mesh_light_model_breath_mode_work);
    k_delayed_work_submit(&breath_mode, duration);
    k_delayed_work_submit(&breath_work, 2000);
    breath_mode_is_run = BT_PRIVT_TRUE;
    dbg_printf("unbind mode start\n");
    return 1;

}

int ble_mesh_light_model_provsioned_complete()
{
    if (ble_mesh_light_model_is_breath_run()) {
        breath_mode_is_run = BT_PRIVT_FALSE;
        k_delayed_work_cancel(&breath_mode);
        dbg_printf("breath mode stopped\n");
    }
    return 0;
}

int ble_mesh_light_model_power_on()
{
    uint32_t lightstatus = 0;
    uint32_t count_m;
    uint32_t count_n;
    uint8_t cw=0;

    g_ble_mesh_light_model_onoff_state = 1;

#if (FLASH_ENABLE)
    struct cached_data_t ctword={0};
    int16_t len;
    uint8_t* db = NULL;
    uint16_t pack_cw = 5;

    db = kv_get(CUST_CONF_FLASH_INDEX, &len);
    if (db && (len > 0)) {
        memcpy(&ctword, (uint8_t*)db, sizeof(cust_mem));
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
        for(count_n = 0; count_n < 2000; count_n++) {
            cw = pack_cw * count_n / 200;
            ble_mesh_light_model_light_reset(65535, cw);
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

void update_light_state(void)
{
    u32_t power, color;
    uint32_t cach_value =0;
    power = lightness;
    uint8_t ratio = 0;
    uint8_t set_lightness = 0;
    color = 100 * ((float)(temperature + 32768) / 65535);
    GWW = color;
    GCW = 100 - color;

    if (power > 0) {
        g_ble_mesh_light_model_onoff_state = 1;
    } else {
        g_ble_mesh_light_model_onoff_state = 0;
    }

    dbg_printf("Light state: onoff = %d lightness = 0x%04x CW = 0x%x\n", g_ble_mesh_light_model_onoff_state, (u16_t)power, GCW);
    if (1 == g_ble_mesh_light_model_onoff_state) {
        ratio = (uint32_t)(lightness * 100) >> 16;
        set_lightness = ((uint32_t)(ratio << 8)) / 100;
        Rv = set_lightness;
        Gv = set_lightness;
        Bv = set_lightness;
        GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
        GIO_WriteValue(PIN_SDI, 0);
        set_led_color(set_lightness, set_lightness, set_lightness);
        if (last_lightness == lightness) {
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

#if defined __cplusplus
    }
#endif

