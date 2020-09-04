
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

static struct k_delayed_work flash_work;
static struct cached_data_t cust_mem;

static void ble_mesh_light_model_default_var_init(void)
{
}

static void ble_mesh_light_model_default_status_init(void)
{
}

static void ble_mesh_light_model_light_to_save(struct ble_npl_event *ev)
{
    dbg_printf("flash write light param\n");
    kv_put(CUST_CONF_FLASH_INDEX, (uint8_t*)&cust_mem, sizeof(cust_mem));
}

int ble_mesh_light_model_conf_init()
{
    ble_mesh_light_model_default_var_init();
    ble_mesh_light_model_default_status_init();

    k_delayed_work_init(&flash_work, ble_mesh_light_model_light_to_save);
    return 0;
}

int ble_mesh_light_model_unbind_mode_run(uint32_t duration)
{
    return 1;

}

int ble_mesh_light_model_provsioned_complete()
{
    return 0;
}

void update_light_state(void)
{
    //cached to flash
    //k_delayed_work_submit(&flash_work, 500);
    return;
}

#if defined __cplusplus
    }
#endif

