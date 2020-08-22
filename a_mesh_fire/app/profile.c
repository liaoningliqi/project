
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdio.h>
#include "mesh_def.h"
#include "mesh_api.h"
#include "access.h"
#include "bt_types.h"
#include "gap.h"
#include "btstack_event.h"
#include "btstack_util.h"
#include "platform_api.h"
#include "ota_service.h"
#include "light_model.h"
#include "device_composition.h"
#include "att_db_util.h"
#include "gatt_client.h"
#include "att_db.h"
#include "profile.h"
#include "eflash.h"

#include "app.h"
#include "..\mesh\BLE_mesh.h"
#include "..\..\project_common\project_common.h"

#if defined __cplusplus
    extern "C" {
#endif

static uint8_t mesh_bt_dev_name[8] = {'I','n','g','c','h','i','p','\0'};

#define OTA_ADV_HANDLE (0x05) // Attention: must not use the handle > 5
/*
 * Server Configuration Declaration
 */

volatile static u16_t primary_addr;
volatile static u16_t primary_net_idx;
extern void set_led_color(uint8_t r, uint8_t g, uint8_t b);

#define INVALID_HANDLE (0xffff)
#define AT_CMD_ID      (0xfffe)
static uint16_t OTA_CONN_HANDLE = INVALID_HANDLE;
static uint16_t temp_OTA_CONN_HANDLE = INVALID_HANDLE;

void kb_report_trigger_send(uint8_t keynum);

#ifdef USE_OOB
static int output_number(bt_mesh_output_action_t action, u32_t number)
{
    printf("OOB Number %u", number);
    return 0;
}

static int output_string(const char *str)
{
    printf("OOB String %s", str);
    return 0;
}
#endif
uint8_t PTS_LT2[6] ={0x00,0x1B,0xDC,0xF2,0x1c,0xc7};
uint8_t PTS_LT1[6] ={0x00,0x1b,0xdc,0xf2,0x1c,0xc6};
#define PIN_SDI GIO_GPIO_0
static void prov_complete(u16_t net_idx, u16_t addr)
{
    printf("provisioning complete for net_idx 0x%04x addr 0x%04x",
                net_idx, addr);
    primary_addr = addr;
    primary_net_idx = net_idx;
		//to notify the light model
    light_provsioned_complete();
		set_mesh_sleep_duration(150);
    set_flag_for_adv_sent(0); 	// 0£ºstart   1£ºstop


    GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
    GIO_WriteValue(PIN_SDI, 0);

    //if not set the light status ,set here
extern bool light_status_set;
    if(!light_status_set)
    {
        set_led_color(50, 50, 50);
        light_status_set = false;
    }
}

static void prov_reset(void)
{
    bt_mesh_prov_enable((bt_mesh_prov_bearer_t)(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT));
		printf("node reset\n");
    set_flag_for_adv_sent(0);
}

#define MYNEWT_VAL_BLE_MESH_DEV_UUID             	((uint8_t[16]){0xA8,0x01,0x61,0x00,0x04,0x20,0x30,0x75,0x9a,0x00,0x07,0xda,0x78,0x00,0x00,0x00})

#ifdef PTS_TEST
static u8_t dev_uuid[16] = {0x02,0x0B,0xDC,0x08,0x10,0x21,0x0B,0x0E,0x0A,0x0C,0x00,0x0B,0x0E,0x0A,0x0B,0x96};
const static unsigned char addr[6] = {2,0,0,0,0,0};
#else
static u8_t dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);
const static unsigned char addr[6] = {1,0,0,0,0,0};
#endif

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

void model_init()
{
    extern void nimble_port_init(void);
    nimble_port_init();
    init_pub();
    printf("begin to setup comp\n");
    model_info_pub();
    mesh_setup(&prov, get_comp_of_node());
    model_conf_init();
}

uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    return ota_read_callback(att_handle, offset, buffer, buffer_size);
}


hci_con_handle_t handle_send;
btstack_packet_callback_registration_t hci_event_callback_registration;

int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode,
                              uint16_t offset,const uint8_t *buffer, uint16_t buffer_size)
{
    handle_send = connection_handle;
    return ota_write_callback(att_handle, transaction_mode, offset, (uint8_t*)buffer, buffer_size);
}


const uint8_t adv_data[] = {
    // Flags general discoverable
    0x02, 0x01, 0x06,

    //Tx Power
    0x02, 0x0a,0x08,

    //32bits complete service UUIDs
    0x08, 0x0b,'I','N','G','-','O','T','A',
};

//#ifndef DEV_BOARD
//#endif

uint8_t addr2[8]={0x01,0x01,0x01,0x04,0x05,0x06};
uint8_t att_db_storage[800];
uint8_t vnd_msg[4]={0xcf,0x09,0xF0,0x23};

extern u8_t gen_onoff_state;
static void user_msg_handler(uint32_t msg_id, void *data, uint16_t size)
{
    uint8_t  key_status = 0;
    switch (msg_id)
    {
//	#ifndef DEV_BOARD
        case USER_MSG_ID_REQUEST_SEND_KB1:
            if(service_is_ready(0))   //here use app_idx =0;
            {
                extern struct bt_mesh_model root_models[];
                uint8_t msg[3]={0x00,0x00,0x00};
                app_request_t pmsg ={0};
                pmsg.model = get_model_by_id(BT_MESH_MODEL_ID_GEN_ONOFF_SRV);
                if(!pmsg.model)
                {
                    printf("model not exist\n");
                    return;
                }
                pmsg.app_idx = 0;
                pmsg.dst = 0x015e;	//the genie addr ID  X1=00DA   R1=015E
                pmsg.opcode = BT_MESH_MODEL_OP_2(0x82, 0x04);// Generic OnOff Status  opcode 0x8204
                if(1 == gen_onoff_state)
                {
                    key_status=0;
                    gen_onoff_state=0;
                    set_led_color(0,0,0);
                }
                else
                {
                    gen_onoff_state=1;
                    key_status=1;
                    set_led_color(50,50,50);
                }
                memset(msg,key_status,sizeof(msg));
                memcpy(pmsg.msg,msg,3);
                pmsg.len =3;
                pmsg.bear =1;
                mesh_service_trigger((uint8_t*)&pmsg,sizeof(app_request_t));
                printf("mesh service trigger 0x8204 light status !!\r\n");
             }
				break;

        case USER_MSG_ID_REQUEST_SEND_KB2:
            if(service_is_ready(0))    //here use app_idx =0;
            {
                extern struct bt_mesh_model vnd_models[];
                //Attr name:0x09f0    ¸´Î»event 0x23
                app_request_t pmsg ={0};
                pmsg.model = get_model_by_id(0x01A8);						//&alibaba_models[0];
                if(!pmsg.model)
                {
                    printf("model not exist\n");
                    return;
                }
                pmsg.app_idx = 0;
                pmsg.dst = 0xF000;    //the address of genie
                pmsg.opcode = BT_MESH_MODEL_OP_3(0xD4, 0x01A8);
                vnd_msg[0]++;
                memcpy(pmsg.msg,vnd_msg,4);
                pmsg.len  =4;
                pmsg.bear =1;
                mesh_service_trigger((uint8_t*)&pmsg,sizeof(app_request_t));
                printf("mesh service trigger Hardrest event 0x23!!\r\n");
            }
                break;

			case USER_MSG_MESH_INIT_DONE:
					//to do something here after mesh initialized.
					//setup mesh task sleep duration after adv package sent to HOST.here to setup 20MS.
				//set_mesh_sleep_duration(20);
				break;
			default:
					;
    }
}
#define BREATH_MODE_DURATION  10000

void user_packet_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size)
{
    uint8_t event = hci_event_packet_get_type(packet);
    const btstack_user_msg_t *p_user_msg;
    uint8_t addr[] = {6, 5, 4, 0, 0, 0};
    if (packet_type != HCI_EVENT_PACKET) return;
    switch (event)
    {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
                    break;
            if(!is_provisioned_poweron())
            {
                unbind_light_mode_run(BREATH_MODE_DURATION); //	breath mode once not provisoned.
                set_flag_for_adv_sent(0);
                set_mesh_sleep_duration(80);
            }
            gap_set_random_device_address(addr);
            gap_set_adv_set_random_addr(OTA_ADV_HANDLE, addr2);
            gap_set_ext_adv_para(OTA_ADV_HANDLE,                            // ota use ADV_SET 5
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
                    ext_adv_set_en_t adv_set;
            adv_set.handle = OTA_ADV_HANDLE;
            adv_set.duration = 0;
            adv_set.max_events=0;
            gap_set_ext_adv_enable(1,1,&adv_set);

            if(service_is_ready(0))
            {
                extern struct bt_mesh_model vnd_models[];
                uint8_t msg[2]={0x01,0x02};
                app_request_t pmsg ={0};
                pmsg.model = &vnd_models[0];
                pmsg.app_idx = 0;
                pmsg.dst = 0x00da;
                pmsg.opcode = BT_MESH_MODEL_OP_3(0x02, 0x05C3);
                memcpy(pmsg.msg,msg,20);
                pmsg.len =20;
                pmsg.bear =1;
                mesh_service_trigger((uint8_t*)&pmsg,sizeof(app_request_t));
            }
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet))
            {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                    temp_OTA_CONN_HANDLE = little_endian_read_16(packet,4);  //may be the connection is for other service ,not for OTA ,so it is decided by  ADV set terminated events
                    break;
                case HCI_SUBEVENT_LE_ADVERTISING_SET_TERMINATED:
                    if ((OTA_ADV_HANDLE == packet[4]) && (temp_OTA_CONN_HANDLE != INVALID_HANDLE))
                    {
                        OTA_CONN_HANDLE = temp_OTA_CONN_HANDLE;
                        att_set_db(OTA_CONN_HANDLE,att_db_storage);
                        att_server_init(att_read_callback,att_write_callback);
                        printf("OTA_SERVICE connected\n");
                    }
                    break;
                default:
                    break;
            }
                break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            //if OTA connection abort, not start the OTA downlaod process ,then need to restart the OTA service advertising
            if(OTA_CONN_HANDLE == little_endian_read_16(packet, 3))
            {
                //start adv again.
                ext_adv_set_en_t advset;
                advset.handle = OTA_ADV_HANDLE;
                advset.duration =0;
                advset.max_events =0;
                gap_set_ext_adv_enable(1,1,&advset);
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

#ifdef V2
ota_ver_t this_version = {
    .app = {.major = 1, .minor = 2, .patch = 0}
};
unsigned char pub_addr[] = {6,5,4,2,2,2,};
#else
ota_ver_t this_version = {
    .app = {.major = 1, .minor = 1, .patch = 0}
};
unsigned char pub_addr[] = {6,5,4,1,1,1,};
#endif

kb_report_t report =
{
    .modifier = 0,
    .reserved = 0,
    .codes = {0}
};

#define gatt_service_generic_attribute                  0x1801

 uint8_t *init_service()
{
    att_db_util_init(att_db_storage, sizeof(att_db_storage));
    ota_init_service(&this_version);

    return att_db_storage;
}

void update_led_command(remote_control_adv_t remote_control_adv_t, uint8_t length)
{
    return;
}

int flash_erase_and_write(uint8_t *flash_area, uint32_t off, uint32_t *src,uint32_t len)
{
    return program_flash((uint32_t)flash_area,(uint8_t*)src,len);
}

uint32_t setup_profile(void *data, void *user_data)
{
    dbg_printf("setup profile\r\n");
    sysSetPublicDeviceAddr(addr);

    init_service();
    att_server_init(att_read_callback, att_write_callback);
    hci_event_callback_registration.callback = &user_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(&user_packet_handler);

    mesh_env_init();
    mesh_set_dev_name((char*)mesh_bt_dev_name);
    create_mesh_task();

    return 0;
}

#if defined __cplusplus
    }
#endif

