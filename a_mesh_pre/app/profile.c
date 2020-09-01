
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include <stdint.h>

#include "btstack_event.h"
#include "att_db.h"
#include "att_db_util.h"

#include "mesh_def.h"
#include "mesh_api.h"
#include "ota_service.h"
#include "access.h"
#include "device_composition.h"

#include "eflash.h"

#include "app.h"
#include "chip_peripherals.h"
#include "ble_mesh_light_model.h"
#include "ble_mesh.h"
#include "profile.h"
#include "..\..\project_common\project_common.h"

#if defined __cplusplus
    extern "C" {
#endif

#define OTA_ADV_HANDLE (0x05) // Attention: must not use the handle > 5
#define BREATH_MODE_DURATION  10000


static uint16_t OTA_CONN_HANDLE = INVALID_HANDLE;
static uint16_t temp_OTA_CONN_HANDLE = INVALID_HANDLE;

static btstack_packet_callback_registration_t hci_event_callback_registration;

static const uint8_t adv_data[] = {
    // Flags general discoverable
    0x02, 0x01, 0x06,
    //Tx Power
    0x02, 0x0a, 0x08,
    //32bits complete service UUIDs
    0x08, 0x0b, 'I', 'N', 'G', '-', 'O', 'T', 'A',
};

#ifdef V2
static ota_ver_t this_version = {
    .app = {.major = 1, .minor = 2, .patch = 0}
};
static unsigned char pub_addr[] = {6, 5, 4, 2, 2, 2};
#else
static ota_ver_t this_version = {
    .app = {.major = 1, .minor = 1, .patch = 0}
};
static unsigned char pub_addr[] = {6, 5, 4, 1, 1, 1};
#endif

static uint8_t addr2[8] = {0x01, 0x01, 0x01, 0x04, 0x05, 0x06};
static uint8_t att_db_storage[800];
static uint8_t vnd_msg[4] = {0xcf, 0x09, 0xF0, 0x23};

#ifdef USE_OOB
static int output_number(bt_mesh_output_action_t action, u32_t number)
{
    dbg_printf("OOB Number %u", number);
    return 0;
}

static int output_string(const char *str)
{
    dbg_printf("OOB String %s", str);
    return 0;
}
#endif

static uint16_t att_ota_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    return ota_read_callback(att_handle, offset, buffer, buffer_size);
}

static int att_ota_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset,const uint8_t *buffer, uint16_t buffer_size)
{
    return ota_write_callback(att_handle, transaction_mode, offset, (uint8_t*)buffer, buffer_size);
}

void user_packet_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size)
{
    uint8_t event = hci_event_packet_get_type(packet);
    const btstack_user_msg_t *p_user_msg;
    uint8_t addr[] = {6, 5, 4, 0, 0, 0};
    ext_adv_set_en_t adv_set;
    uint8_t msg[2] = {0x01, 0x02};
    app_request_t pmsg = {0};

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    switch (event) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                break;
            }

            if(!is_provisioned_poweron()) {
                ble_mesh_light_model_unbind_mode_run(BREATH_MODE_DURATION); //	breath mode once not provisoned.
                set_flag_for_adv_sent(0);
                set_mesh_sleep_duration(80);
            }
            gap_set_random_device_address(addr);
            gap_set_adv_set_random_addr(OTA_ADV_HANDLE, addr2);
            gap_set_ext_adv_para(OTA_ADV_HANDLE,            // ota use ADV_SET 5
                                 CONNECTABLE_ADV_BIT | SCANNABLE_ADV_BIT | LEGACY_PDU_BIT,
                                 0x00f1, 0x00f1,            // Primary_Advertising_Interval_Min, Primary_Advertising_Interval_Max
                                 PRIMARY_ADV_ALL_CHANNELS,  // Primary_Advertising_Channel_Map
                                 BD_ADDR_TYPE_LE_RANDOM,    // Own_Address_Type
                                 BD_ADDR_TYPE_LE_PUBLIC,    // Peer_Address_Type (ignore)
                                 NULL,                      // Peer_Address      (ignore)
                                 ADV_FILTER_ALLOW_ALL,      // Advertising_Filter_Policy
                                 0x00,                      // Advertising_Tx_Power
                                 PHY_1M,                    // Primary_Advertising_PHY
                                 0,                         // Secondary_Advertising_Max_Skip
                                 PHY_1M,                    // Secondary_Advertising_PHY
                                 0x00,                      // Advertising_SID
                                 0x00);                     // Scan_Request_Notification_Enable
            gap_set_ext_adv_data(OTA_ADV_HANDLE, sizeof(adv_data), (uint8_t*)adv_data);
            gap_set_ext_scan_response_data(OTA_ADV_HANDLE, sizeof(adv_data), (uint8_t*)adv_data);
            adv_set.handle = OTA_ADV_HANDLE;
            adv_set.duration = 0;
            adv_set.max_events = 0;
            gap_set_ext_adv_enable(1, 1, &adv_set);

            if (service_is_ready(0)) {
                pmsg.model = &vnd_models[0];
                pmsg.app_idx = 0;
                pmsg.dst = 0x00da;
                pmsg.opcode = BT_MESH_MODEL_OP_3(0x02, 0x05C3);
                memcpy(pmsg.msg, msg, 20);
                pmsg.len = 20;
                pmsg.bear = 1;
                mesh_service_trigger((uint8_t*)&pmsg, sizeof(app_request_t));
            }
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                    temp_OTA_CONN_HANDLE = little_endian_read_16(packet, 4);  //may be the connection is for other service ,not for OTA ,so it is decided by  ADV set terminated events
                    break;
                case HCI_SUBEVENT_LE_ADVERTISING_SET_TERMINATED:
                    if ((OTA_ADV_HANDLE == packet[4]) && (temp_OTA_CONN_HANDLE != INVALID_HANDLE)) {
                        OTA_CONN_HANDLE = temp_OTA_CONN_HANDLE;
                        att_set_db(OTA_CONN_HANDLE, att_db_storage);
                        att_server_init(att_ota_read_callback,att_ota_write_callback);
                        dbg_printf("OTA_SERVICE connected\n");
                    }
                    break;
                default:
                    break;
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            if(OTA_CONN_HANDLE == little_endian_read_16(packet, 3)) {
                ext_adv_set_en_t advset;
                advset.handle = OTA_ADV_HANDLE;
                advset.duration = 0;
                advset.max_events = 0;
                gap_set_ext_adv_enable(1, 1, &advset);
            }
            break;

        case ATT_EVENT_CAN_SEND_NOW:
            break;

        case BTSTACK_EVENT_USER_MSG:
            p_user_msg = hci_event_packet_get_user_msg(packet);
            user_msg_handler(p_user_msg->msg_id, p_user_msg->data, p_user_msg->len);
            break;

        default:
            break;
    }
}

static void init_ota_service()
{
    ota_init_service(&this_version);

    return;
}

void update_led_command(remote_control_adv_t remote_control_adv_t, uint8_t length)
{
    return;
}

int flash_erase_and_write(uint8_t *flash_area, uint32_t off, uint32_t *src, uint32_t len)
{
    return program_flash((uint32_t)flash_area, (uint8_t*)src,len);
}

uint32_t setup_profile(void *data, void *user_data)
{
    dbg_printf("setup profile\r\n");
    att_db_util_init(att_db_storage, sizeof(att_db_storage));

    init_ota_service();
    att_server_init(att_ota_read_callback, att_ota_write_callback);

    hci_event_callback_registration.callback = &user_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(&user_packet_handler);

    ble_mesh_profile_env_init();

    return 0;
}

#if defined __cplusplus
    }
#endif

