
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "platform_api.h"
#include "att_db.h"
#include "gap.h"
#include "btstack_event.h"

#include "att_db_util.h"

#include "../../project_common/project_common.h"
#include "../bt_at_cmd_parse/bt_at_cmd.h"
#include "sdk.h"

#define OGF_STATUS_PARAMETERS       0x05
#define OPCODE(ogf, ocf)            (ocf | ogf << 10)
#define OPCODE_READ_RSSI            OPCODE(OGF_STATUS_PARAMETERS, 0x05)

#pragma pack (push, 1)
typedef struct read_rssi_complete
{
    uint8_t  status;
    uint16_t handle;
    int8_t   rssi;
} read_rssi_complete_t;
#pragma pack (pop)

const static uint8_t scan_data[] = {
    #include "../data/scan_response.adv"
};

static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset,
                                  uint8_t * buffer, uint16_t buffer_size)
{
    return bt_bm_module_gatt_read_callback(att_handle, offset, buffer, buffer_size);
}

static btstack_packet_callback_registration_t hci_event_callback_registration;

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode,
                              uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    g_bt_bm_hci_con_handle_send = connection_handle;
    dbg_printf("att write callback connected handle 0x%x \r\n", g_bt_bm_hci_con_handle_send);
    return bt_bm_module_gatt_write_callback(att_handle, offset, buffer, buffer_size);
}

static void user_msg_handler(uint32_t msg_id, void *data, uint16_t size)
{
    uint16_t interval;
    uint16_t ce_len;

    switch (msg_id)
    {
    // need to be add
    case USER_MSG_ID_RSSI_SEND:
        gap_read_rssi(g_hci_le_conn_complete_slave_handle);
        break;
    case USER_MSG_ID_SET_INTERVAL:
        interval = size;
        ce_len = (interval << 1) - 2;
        gap_update_connection_parameters(g_hci_le_conn_complete_slave_handle, interval, interval,
            0, interval > 10 ? interval : 10, // supervisor_timeout = max(100, interval * 8)
            ce_len, ce_len);
        dbg_printf("set interval 0x%x \r\n", interval);
        break;
    case USER_MSG_ID_HEALTH_THERMOMETER_SERVICE:
    case USER_MSG_ID_UART_RX_TO_BLE_DATA_SERVICE:
    case USER_MSG_ID_ADC_VALUE_SERVICE:
    case USER_MSG_ID_GPIO_IN_SERVICE:
    default:
        att_server_request_can_send_now_event(g_bt_bm_hci_con_handle_send);
        break;
    }

    return;
}

static void user_packet_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size)
{
    le_meta_event_enh_create_conn_complete_t *conn_complete;
    read_rssi_complete_t *cmpl;
    const static ext_adv_set_en_t adv_sets_en[] = {{.handle = 0, .duration = 0, .max_events = 0}};
    uint8_t event = hci_event_packet_get_type(packet);
    const btstack_user_msg_t *p_user_msg;
    char send_data[BT_AT_CMD_MAX_LEN] = {0};

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    dbg_printf("usr pkt 0x%x\r\n", event);

    switch (event)
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
            break;

        gap_set_adv_set_random_addr(0, g_rand_mac_addres);
        gap_set_ext_adv_para(0,
                             CONNECTABLE_ADV_BIT | SCANNABLE_ADV_BIT | LEGACY_PDU_BIT,
                             0x00a1, 0x00a1,            // Primary_Advertising_Interval_Min, Primary_Advertising_Interval_Max
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
        gap_set_ext_adv_data(0, sizeof(*g_module_adv_data), (uint8_t *)g_module_adv_data);
        gap_set_ext_scan_response_data(0, sizeof(scan_data), (uint8_t*)scan_data);
        gap_set_ext_adv_enable(1, sizeof(adv_sets_en) / sizeof(adv_sets_en[0]), adv_sets_en);
        break;

    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet))
        {
        case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
            conn_complete = (le_meta_event_enh_create_conn_complete_t *)decode_hci_le_meta_event(packet, le_meta_event_enh_create_conn_complete_t);
            g_hci_le_conn_complete_slave_handle = conn_complete->handle;
            att_set_db(conn_complete->handle, att_db_util_get_address());
            dbg_printf("HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE connected handle 0x%x \r\n", conn_complete->handle);
            bt_cmd_data_uart_out_string_with_end_char("TTM:CONNECT\r\n\0");
            break;
        default:
            break;
        }

        break;

    case HCI_EVENT_COMMAND_COMPLETE:
        if (hci_event_command_complete_get_command_opcode(packet) == OPCODE_READ_RSSI)
        {
            cmpl = (read_rssi_complete_t *)hci_event_command_complete_get_return_parameters(packet);
            sprintf(send_data, "TTM:RSI%ddBm\r\n\0", cmpl->rssi);
            bt_cmd_data_uart_out_string_with_end_char(send_data);
        }
        break;

    case HCI_EVENT_CONNECTION_COMPLETE:
        break;

    case HCI_EVENT_DISCONNECTION_COMPLETE:
        // need to be add
#ifdef ING_CHIPS_PRIVATE_SERVICE
        g_bt_bm_module_health_thermometer_service_notify_enable = 0;
        g_bt_bm_module_health_thermometer_service_indicate_enable = 0;
#endif
        g_bt_bm_module_adc0_value_service_notify_enable = 0;
        g_bt_bm_module_adc1_value_service_notify_enable = 0;
        g_bt_bm_module_adc0_value_service_indicate_enable = 0;
        g_bt_bm_module_adc1_value_service_indicate_enable = 0;
        g_bt_bm_module_gpio_in_service_notify_enable = 0;
        g_bt_bm_module_gpio_in_service_indicate_enable = 0;
        gap_set_ext_adv_enable(1, sizeof(adv_sets_en) / sizeof(adv_sets_en[0]), adv_sets_en);
        bt_cmd_data_uart_out_string_with_end_char("TTM:DISCONNECT\r\n\0");
        break;

    case ATT_EVENT_CAN_SEND_NOW:
        // need to be add
        bt_bm_module_send_data();
        break;

    case BTSTACK_EVENT_USER_MSG:
        p_user_msg = hci_event_packet_get_user_msg(packet);
        user_msg_handler(p_user_msg->msg_id, p_user_msg->data, p_user_msg->len);
        break;

    default:
        break;
    }
}

uint32_t setup_profile(void *data, void *user_data)
{
    dbg_printf("setup profile\r\n");

    att_server_init(att_read_callback, att_write_callback);
    hci_event_callback_registration.callback = &user_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(&user_packet_handler);
    return 0;
}

