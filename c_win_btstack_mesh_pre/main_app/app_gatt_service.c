
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "btstack.h"

#include "..\..\project_common\project_common.h"
#include "app_gatt_service.h"

#if defined __cplusplus
    extern "C" {
#endif

#define BT_BM_MODULE_GATT_HANDLE_MAX_NUM 100

typedef int (*pfun_bt_bm_module_att_read_callback)(uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
typedef int (*pfun_bt_bm_module_att_write_callback)(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);

typedef struct {
    uint16_t cmd;
    pfun_bt_bm_module_att_read_callback fun;
} bt_bm_module_att_read_callback_t;

typedef struct {
    uint16_t cmd;
    pfun_bt_bm_module_att_write_callback fun;
} bt_bm_module_att_write_callback_t;

hci_con_handle_t g_bt_bm_hci_con_handle_send;

static bt_bm_module_att_read_callback_t module_gatt_read_callback[BT_BM_MODULE_GATT_HANDLE_MAX_NUM] = {0};
static int module_gatt_read_callback_num = 0;

static bt_bm_module_att_write_callback_t module_gatt_write_callback[BT_BM_MODULE_GATT_HANDLE_MAX_NUM] = {0};
static int module_gatt_write_callback_num = 0;

static int app_gatt_service_read_callback_register(uint16_t cmd, pfun_bt_bm_module_att_read_callback fun)
{
    int cmd_index;

    if (module_gatt_read_callback_num >= BT_BM_MODULE_GATT_HANDLE_MAX_NUM) {
        dbg_printf("app_gatt_service_read_callback_register fail!\r\n");
        return BT_PRIVT_ERROR;
    }

    for (cmd_index = 0; cmd_index < module_gatt_read_callback_num; cmd_index++) {
        if (module_gatt_read_callback[cmd_index].cmd == cmd) {
            module_gatt_read_callback[cmd_index].fun = fun;
            return BT_PRIVT_OK;
        }
    }

    module_gatt_read_callback[module_gatt_read_callback_num].cmd = cmd;
    module_gatt_read_callback[module_gatt_read_callback_num].fun = fun;
    module_gatt_read_callback_num++;

    return BT_PRIVT_OK;
}

static int app_gatt_service_write_callback_register(uint16_t cmd, pfun_bt_bm_module_att_write_callback fun)
{
    int cmd_index;

    if (module_gatt_write_callback_num >= BT_BM_MODULE_GATT_HANDLE_MAX_NUM) {
        dbg_printf("app_gatt_service_write_callback_register fail!\r\n");
        return BT_PRIVT_ERROR;
    }

    for (cmd_index = 0; cmd_index < module_gatt_write_callback_num; cmd_index++) {
        if (module_gatt_write_callback[cmd_index].cmd == cmd) {
            module_gatt_write_callback[cmd_index].fun = fun;
            return BT_PRIVT_OK;
        }
    }

    module_gatt_write_callback[module_gatt_write_callback_num].cmd = cmd;
    module_gatt_write_callback[module_gatt_write_callback_num].fun = fun;
    module_gatt_write_callback_num++;

    return BT_PRIVT_OK;
}

static int app_gatt_service_read_callback(uint16_t cmd, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int cmd_index;

    dbg_printf("gatt read callback: %u\r\n", cmd);
    for (cmd_index = 0; cmd_index < module_gatt_read_callback_num; cmd_index++) {
        if ((module_gatt_read_callback[cmd_index].cmd == cmd) &&
            (module_gatt_read_callback[cmd_index].fun != BT_PRIVT_NULL)) {
            return module_gatt_read_callback[cmd_index].fun(offset, buffer, buffer_size);
        }
    }

    return BT_PRIVT_ERROR;
}

static int app_gatt_service_write_callback(uint16_t cmd, uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    int cmd_index;

    dbg_printf("gatt write callback: %u\r\n", cmd);
    for (cmd_index = 0; cmd_index < module_gatt_write_callback_num; cmd_index++) {
        if ((module_gatt_write_callback[cmd_index].cmd == cmd) &&
            (module_gatt_write_callback[cmd_index].fun != BT_PRIVT_NULL)) {
            dbg_printf("found cmd\r\n");
            return module_gatt_write_callback[cmd_index].fun(offset, buffer, buffer_size);
        }
    }

    return BT_PRIVT_ERROR;
}

static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    return app_gatt_service_read_callback(att_handle, offset, buffer, buffer_size);
}

static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    g_bt_bm_hci_con_handle_send = con_handle;
    dbg_printf("att write callback connected handle 0x%x \r\n", g_bt_bm_hci_con_handle_send);
    return app_gatt_service_write_callback(att_handle, offset, buffer, buffer_size);
}

static int bt_bm_module_mesh_prov_data_in_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

static int bt_bm_module_mesh_prov_data_in_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

static int bt_bm_module_mesh_prov_data_out_read_callback(uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

static int bt_bm_rgb_light_control_write_callback(uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    return BT_PRIVT_OK;
}

static void bt_bm_module_init_gap_service(void)
{
    static char dev_name[] = "BM_DEVICE";
    static uint8_t service_changed[] = {0x00,0x00,0x00,0x00};

    att_db_util_add_service_uuid16(ORG_BLUETOOTH_SERVICE_GENERIC_ACCESS);
    att_db_util_add_characteristic_uuid16(ORG_BLUETOOTH_CHARACTERISTIC_GAP_DEVICE_NAME,
        ATT_PROPERTY_READ,
        ATT_SECURITY_NONE, ATT_SECURITY_NONE,
        (uint8_t *)dev_name, sizeof(dev_name) - 1);

    att_db_util_add_service_uuid16(ORG_BLUETOOTH_SERVICE_GENERIC_ATTRIBUTE);
    att_db_util_add_characteristic_uuid16(ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED,
        ATT_PROPERTY_READ,
        ATT_SECURITY_NONE, ATT_SECURITY_NONE,
        service_changed, sizeof(service_changed));

    return;
}

static void bt_bm_module_init_mesh_service(void)
{
    uint16_t att_value_handle;
    uint16_t att_client_desc_value_handle = 0;
    uint8_t g_value[27] = {
        0x54, 0x65, 0x6D, 0x70, 0x65, 0x72, 0x61, 0x74, 0x75, 0x72,
        0x65, 0x20, 0x48, 0x75, 0x6D, 0x69, 0x64, 0x69, 0x74, 0x79,
        0x20, 0x53, 0x65, 0x6E, 0x73, 0x6F, 0x72};

    att_db_util_add_service_uuid16(ORG_BLUETOOTH_SERVICE_MESH_PROVISIONING);
    att_value_handle = att_db_util_add_characteristic_uuid16(ORG_BLUETOOTH_CHARACTERISTIC_MESH_PROVISIONING_DATA_IN,
        ATT_PROPERTY_WRITE_WITHOUT_RESPONSE | ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC,
        ATT_SECURITY_NONE, ATT_SECURITY_NONE,
        g_value, sizeof(g_value));
    app_gatt_service_read_callback_register(att_value_handle, bt_bm_module_mesh_prov_data_in_read_callback);
    app_gatt_service_write_callback_register(att_value_handle, bt_bm_module_mesh_prov_data_in_write_callback);

    att_value_handle = att_db_util_add_characteristic_uuid16(ORG_BLUETOOTH_CHARACTERISTIC_MESH_PROVISIONING_DATA_OUT,
        ATT_PROPERTY_READ | ATT_PROPERTY_NOTIFY,
        ATT_SECURITY_NONE, ATT_SECURITY_NONE,
        g_value, sizeof(g_value));
    att_client_desc_value_handle = att_value_handle + 1;

    dbg_printf("bt_bm_module_init_mesh_service %u \r\n", att_client_desc_value_handle);
    app_gatt_service_read_callback_register(att_value_handle, bt_bm_module_mesh_prov_data_out_read_callback);
    return;
}

static void bt_bm_module_init_private_service(void)
{
    uint16_t att_rgb_light_control_handle = 0;
    static uint8_t uuid_ing_rgb_light_service[] = {
        0x6a, 0x33, 0xa5, 0x26, 0xe0, 0x04, 0x47, 0x93, 0xa0, 0x84,
        0x8a, 0x1d, 0xc4, 0x9b, 0x84, 0xfd};
    static uint8_t uuid_ing_rgb_light_control[] = {
        0x1c, 0x19, 0x0e, 0x92, 0x37, 0xdd, 0x4a, 0xc4, 0x81, 0x54,
        0x04, 0x44, 0xc6, 0x92, 0x74, 0xc2};
    uint8_t g_rgb_light_control[] = {0x00, 0x00, 0x00};

    att_db_util_add_service_uuid128(uuid_ing_rgb_light_service);

    att_rgb_light_control_handle = att_db_util_add_characteristic_uuid128(uuid_ing_rgb_light_control,
        ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC | ATT_PROPERTY_UUID128,
        ATT_SECURITY_NONE, ATT_SECURITY_NONE,
        g_rgb_light_control, sizeof(g_rgb_light_control));
    app_gatt_service_write_callback_register(att_rgb_light_control_handle, bt_bm_rgb_light_control_write_callback);

    return;
}

int app_gatt_service_init(void)
{
    uint8_t *addr = BT_PRIVT_NULL;
    uint16_t size;

    att_db_util_init();

    bt_bm_module_init_gap_service();
    bt_bm_module_init_mesh_service();
    bt_bm_module_init_private_service();

    addr = att_db_util_get_address();
    size = att_db_util_get_size();

    att_server_init(addr, att_read_callback, att_write_callback);

    printf("LE Counter DB\n");
    printf_hexdump(addr, size);

    return BT_PRIVT_OK;
}

#if defined __cplusplus
    }
#endif

