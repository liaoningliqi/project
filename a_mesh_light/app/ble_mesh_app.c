
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <stdint.h>

#include "mesh_api.h"
#include "mesh_def.h"
#include "device_composition.h"

#include "project_common.h"
#include "BLE_mesh.h"
#include "chip_peripherals.h"
#include "ble_mesh_light_model.h"

#if defined __cplusplus
    extern "C" {
#endif

#define LIGHT_SEC_ALI

#ifdef LIGHT_SEC_ALI
#define SEC ((uint8_t[16]){0x64,0xc9,0x41,0x14,0xca,0x07,0x24,0x40,0xa3,0xc1,0xbb,0x2b,0x22,0xb5,0x24,0x5e})
#define PB_ADV_ADDR ((uint8_t[6]){0x28,0xfa,0x7a,0xa3,0xcd,0xfc})
#endif

#ifdef LIGHT_SEC4
#define SEC ((uint8_t[16]){0x39,0xd6,0x96,0xff,0x64,0xfd,0xbf,0xe2,0xbb,0xfe,0x27,0xc0,0x5a,0xb3,0x26,0x5f})
#define PB_ADV_ADDR ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x2f})
#endif

#ifdef LIGHT_SEC3
#define SEC ((uint8_t[16]){0x59,0x06,0x2b,0x94,0x04,0xdf,0x29,0xfc,0xfd,0x2f,0x75,0x11,0x77,0xff,0x68,0x1a})
#define PB_ADV_ADDR ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x02})
#endif

#ifdef LIGHT_SEC2
#define SEC ((uint8_t[16]){0x12,0xfc,0x31,0x28,0x73,0x0f,0x5b,0x84,0xe9,0x6c,0x1a,0x2c,0xac,0x5f,0xb8,0x93})
#define PB_ADV_ADDR ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x01})
#endif

#ifdef LIGHT_SEC1
#define SEC ((uint8_t[16]){0xde,0x9d,0x7a,0xdb,0xa7,0xef,0x12,0x67,0x00,0xec,0xa6,0x56,0x68,0x93,0x89,0x4c})
#define PB_ADV_ADDR ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x00})
#endif

#define PB_GATT_MAC_ADDR ((uint8_t[6]){0x38, 0x05, 0x33, 0xD5, 0x38, 0x78})
#define MYNEWT_VAL_BLE_MESH_DEV_UUID ((uint8_t[16]){0xA8,0x01,0x61,0x00,0x04,0x20,0x30,0x75,0x9a,0x00,0x07,0xda,0x78,0x00,0x00,0x00})

static uint32_t BLE_MESH_DEV_PRODUCT_ID = 5349350;
static uint8_t param[32];

static u8_t dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);
static const unsigned char addr[6] = {1,0,0,0,0,0};

static void prov_complete(u16_t net_idx, u16_t addr);
static void prov_reset(void);
static const struct bt_mesh_prov prov = {
    .uuid = dev_uuid,
#if USE_OOB
    .output_size = 6,
    .output_actions = (BT_MESH_DISPLAY_NUMBER | BT_MESH_DISPLAY_STRING),
    .output_number = output_number,
    .output_string = output_string,
#else
    .output_size = 0,
    .output_actions = 0,
    .output_number = 0,
    .output_string = 0,
#endif
    .complete = prov_complete,
    .reset = prov_reset,
};

volatile static u16_t primary_addr;
volatile static u16_t primary_net_idx;

static void prov_complete(u16_t net_idx, u16_t addr)
{
    dbg_printf("provisioning complete for net_idx 0x%04x addr 0x%04x", net_idx, addr);
    primary_addr = addr;
    primary_net_idx = net_idx;
    ble_mesh_light_model_provsioned_complete();
    set_mesh_sleep_duration(150);
    set_flag_for_adv_sent(0);

    GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
    GIO_WriteValue(PIN_SDI, 0);

    if(!g_ble_mesh_light_model_status_set) {
        set_led_color(50, 50, 50);
        g_ble_mesh_light_model_status_set = false;
    }
}

static void prov_reset(void)
{
    bt_mesh_prov_enable((bt_mesh_prov_bearer_t)(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT));
    dbg_printf("node reset\n");
    set_flag_for_adv_sent(0);
}

void mesh_platform_setup()
{
    memcpy(param, &BLE_MESH_DEV_PRODUCT_ID, sizeof(uint32_t));
    memcpy(param + 4, &TM_AUTHEN_VAL_LEN, sizeof(uint32_t));
    memcpy(param + 8, SEC, 16);
    mesh_platform_config(1, PB_ADV_ADDR, param);
    mesh_platform_config(2, PB_GATT_MAC_ADDR, NULL);
}

void model_init()
{
    dbg_printf("begin to setup comp\n");
    nimble_port_init();
    init_pub();
    model_info_pub();
    mesh_setup(&prov, get_comp_of_node());
    ble_mesh_light_model_conf_init();
}

#if defined __cplusplus
    }
#endif

