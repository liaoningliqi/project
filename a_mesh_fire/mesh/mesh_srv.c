
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdarg.h>
#include <stdio.h>
#include "gap.h"
#include "bt_types.h"
#include "btstack_event.h"
#include "btstack_port.h"
#include "errno.h"
#include "stdbool.h"
#include "mesh_def.h"
#include "access.h"
#include "platform_api.h"
#include "mesh_queue.h"
#include "mesh_priv.h"
#include "att_db.h"
#include "ATcmd_process.h"
#include "mesh_api.h"


#include "btstack_util.h"
#include "BLE_mesh.h"
#include "project_common.h"
#include "light_model.h"
#include "chip_peripherals.h"

#if defined __cplusplus
    extern "C" {
#endif

#define PANIC(fmt, args...) do { dbg_printf("PANIC: " fmt, ##args); for (;;);} while (false);

#define K_NO_WAIT (0)
#define K_FOREVER (-1)

#define TASK_HOST (0x06)
#define TASK_MESH (0x04)

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

#define PB_GATT_MAC_ADDR ((uint8_t[6]){0x38,0xd2,0xca,0x16,0xe0,0x010})

#define MAX_PROV_PROX_DATA_OUT_LEN (66)

#define PROV_DATA_IN_HDL 0x0003
#define PROV_DATA_OUT_HDL 0x0010
#define PROXY_DATA_IN_HDL 0x0003
#define PROXY_DATA_OUT_HDL 0x0005
#define CLIENT_CHARAC_CONF  0x0006

#define MYNEWT_VAL_BLE_MESH_DEV_UUID ((uint8_t[16]){0xA8,0x01,0x61,0x00,0x04,0x20,0x30,0x75,0x9a,0x00,0x07,0xda,0x78,0x00,0x00,0x00})

#define DISCARDED  (1)

extern void user_msg_handler(btstack_user_msg_t * usrmsg);

#ifdef PTS_TEST
static u8_t dev_uuid[16] = {0x02,0x0B,0xDC,0x08,0x10,0x21,0x0B,0x0E,0x0A,0x0C,0x00,0x0B,0x0E,0x0A,0x0B,0x96};
const static unsigned char addr[6] = {2,0,0,0,0,0};
#else
static u8_t dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);
const static unsigned char addr[6] = {1,0,0,0,0,0};
#endif

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

static uint32_t BLE_MESH_DEV_PRODUCT_ID = 5349350;
static uint8_t param[32];

static mesh_gap_event mesh_event;
static uint16_t conn_handle =INVALID_HANDLE;
static uint16_t temp_conn_handle = INVALID_HANDLE;
static uint16_t notify_write =0;

const uint8_t pb_gatt_service_data[]=
{
    // 0x0001 PRIMARY_SERVICE-MESH_PROV
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x27, 0x18,
    // 0x0002 CHARACTERISTIC-GAP_PROV_DATA_IN
    0x0d, 0x00, 0x06, 0x00, 0x02, 0x00, 0x03, 0x28, 0x04, 0x03, 0x00, 0xdb, 0x2a,
    // 0x0003 VALUE-CHARACTERISTIC-GAP_PROV_DATA_IN'
    0x23, 0x00, 0x06, 0x01, 0x03, 0x00, 0xdb, 0x2a,0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x65,0x20,0x48,0x75,0x6D,0x69,0x64,0x69,0x74,0x79,0x20,0x53,0x65,0x6E,0x73,0x6F,0x72,
    // 0x0002 CHARACTERISTIC-GAP_PROV_DATA_OUT
    0x0d, 0x00, 0x12, 0x00, 0x04, 0x00, 0x03, 0x28, 0x10, 0x05, 0x00, 0xdc, 0x2a,
    // 0x0002 VALUE-CHARACTERISTIC-GAP_PROV_DATA_OUT
    0x23, 0x00, 0x02, 0x00, 0x05, 0x00, 0xdc, 0x2a,0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x65,0x20,0x48,0x75,0x6D,0x69,0x64,0x69,0x74,0x79,0x20,0x53,0x65,0x6E,0x73,0x6F,0x72,
    //CLIENT CHARACTERISTIC Configuration
    0x0a,0x00,0x0a,0x01,0x06,0x00,0x02,0x29,0x00,0x00,
    0x00,0x00,
};

const uint8_t proxy_service_data[] =
{
    // 0x0002 PRIMARY_SERVICE-MESH_PROXY
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x28, 0x18,
    // 0x0002 CHARACTERISTIC-GAP_PROXY_DATA_IN
    0x0d, 0x00, 0x06, 0x00, 0x02, 0x00, 0x03, 0x28, 0x04, 0x03, 0x00, 0xdd, 0x2a,
    // 0x0003 VALUE-CHARACTERISTIC-GAP_PROXY_DATA_IN'
    0x23, 0x00, 0x06, 0x01, 0x03, 0x00, 0xdd, 0x2a,0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x65,0x20,0x48,0x75,0x6D,0x69,0x64,0x69,0x74,0x79,0x20,0x53,0x65,0x6E,0x73,0x6F,0x72,
    // 0x0002 CHARACTERISTIC-GAP_PROXY_DATA_OUT
    0x0d, 0x00, 0x12, 0x00, 0x04, 0x00, 0x03, 0x28, 0x10, 0x05, 0x00, 0xde, 0x2a,
    // 0x0003 VALUE-CHARACTERISTIC-GAP_PROXY_DATA_OUT'
    0x23, 0x00, 0x02, 0x00, 0x05, 0x00, 0xde, 0x2a,0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x65,0x20,0x48,0x75,0x6D,0x69,0x64,0x69,0x74,0x79,0x20,0x53,0x65,0x6E,0x73,0x6F,0x72,
    0x0a,0x00,0x0a,0x01,0x06,0x00,0x02,0x29,0x01,0x00,
    0x00,0x00,
};

const uint8_t proxy_adv_data[]={
    // Flags general discoverable
    0x02,0x01,0x06,
    //16bits UUIDs -for provisionning srvice
    0x03, 0x03, 0x28, 0x18,
    //16bits  UUIDs -for mesh proxy    service
    //0x18,0x17,0x2b,0x01,0x00,0xe3,0xd4,0xc6,0x4c,0x1e,0x29,0x21,0x7d,0x00,0x00,0x00,0x00,0x5b,0xf4,0x6b,0xbd,0x0c,0x34,0xa2,0x0e,
    0x15,0x16,0x28,0x18,0xA8,0x01,0x61,0x00,0x04,0x00,0x00,0x75,0x9a,0x00,0x07,0xda,0x78,0x00,0x00,0x00,0x00,0x00,
    //0x14, 0x16,0x28,0x18,0x01,0xc8,0x6e,0xcf,0xf3,0xf3,0x9e,0x27,0x43,0x46,0xcb,0xe1,0x49,0x21,0x20,0x1a,0x60,
};

const uint8_t pb_gatt_adv_data[]={
    // Flags general discoverable
    0x02,0x01,0x06,
    //16bits UUIDs -for provisionning srvice
    0x03, 0x03,0x27,0x18,
    //16bits  UUIDs -for mesh proxy    service
    0x15, 0x16,0x27,0x18,0xA8, 0x01, 0x61,0x00,0x04,0x00,0x00,0x75,0x9a,0x00,0x07,0xda,0x78,0x00,0x00,0x00,0x00,0x00,
};

const uint8_t mesh_srv_adv_data_len = sizeof(pb_gatt_adv_data);
const uint8_t mesh_proxy_srv_adv_data_len = sizeof(proxy_adv_data);
const uint8_t mesh_proxy_service_scan_rsp_data[]={
    // Flags general discoverable
    0x02, 0x01, 0x06,
    // Name
    0x0A, 0x09, 'I', 'N', 'G', ' ', 'C', 'H', 'I', 'P', 'S',
};

const uint8_t adv_mesh_rsp_data_len = sizeof(mesh_proxy_service_scan_rsp_data);

volatile static u16_t primary_addr;
volatile static u16_t primary_net_idx;

static btstack_packet_callback_registration_t hci_event_callback_registration;
const event_vendor_ccm_complete_t *cmpl = NULL;
uint8_t * ptr = NULL;
uint8_t pre_adv[255]={0};
uint8_t test[8]={0};

static const struct bt_mesh_prov *p_prov = NULL;
static const struct bt_mesh_comp *p_comp = NULL;

mesh_at_out uart_printf = NULL;

uint8_t mesh_at[100]={0};
uint8_t size =0;
bool recv_from_uart = false;

static char buf[200];

uint16_t fea_layer_sel =0;
uint8_t cla_flag = 0;

static void prov_complete(u16_t net_idx, u16_t addr)
{
    dbg_printf("provisioning complete for net_idx 0x%04x addr 0x%04x", net_idx, addr);
    primary_addr = addr;
    primary_net_idx = net_idx;
    light_provsioned_complete();
    set_mesh_sleep_duration(150);
    set_flag_for_adv_sent(0);

    GIO_SetDirection(PIN_SDI, GIO_DIR_OUTPUT);
    GIO_WriteValue(PIN_SDI, 0);

    if(!light_status_set) {
        set_led_color(50, 50, 50);
        light_status_set = false;
    }
}

static void prov_reset(void)
{
    bt_mesh_prov_enable((bt_mesh_prov_bearer_t)(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT));
    dbg_printf("node reset\n");
    set_flag_for_adv_sent(0);
}

int ble_gatts_find_svc(uint8_t *db, const ble_uuid_t *uuid, uint16_t *out_handle)
{
    if(((ble_uuid16_t*)uuid)->value == 0x1828) {
        *out_handle = 0x0001;
        return 0;
    }

    if(((ble_uuid16_t*)uuid)->value == 0x1827) {
        *out_handle = 0x0001;
        return 0;
    }

    return -1;
}

int ble_gatts_find_chr(uint8_t *db, uint16_t srv_handle, const ble_uuid_t *chr_uuid, uint16_t *out_att_chr)
{
   if(((ble_uuid16_t*)chr_uuid)->value == 0x2ade) { // proxy_out_handle
        *out_att_chr = 0x0005;
        return 0;
   }

   if(((ble_uuid16_t*)chr_uuid)->value == 0x2add) { // proxy_in_handle
        *out_att_chr = 0x0003;
        return 0;
   }

   if(((ble_uuid16_t*)chr_uuid)->value == 0x2adb) { // prov_in_handle
        *out_att_chr = 0x0003;
        return 0;
   }

   if(((ble_uuid16_t*)chr_uuid)->value == 0x2adc) { // prov_out_handle
        *out_att_chr = 0x0005;
        return 0;
   }

   return -1;
}

uint16_t mesh_read_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    if(!bt_mesh_is_provisioned()) {
        if (attribute_handle == CLIENT_CHARAC_CONF) {
            memcpy(buffer,&notify_write,2);
        }
        return 2;
    } else {
        if (attribute_handle == CLIENT_CHARAC_CONF) {
            memcpy(buffer,&notify_write,2);
        }
        return 2;
    }
}

static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    if (attribute_handle == CLIENT_CHARAC_CONF ) {
        return mesh_read_callback(con_handle,attribute_handle,offset,buffer,buffer_size);
    } else {
        return 0;
    }
}

uint16_t mesh_write_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if(!bt_mesh_is_provisioned()) {
       if(attribute_handle == CLIENT_CHARAC_CONF) {
           notify_write = little_endian_read_16(buffer, 0);
       }
    } else {
       if(attribute_handle == CLIENT_CHARAC_CONF) {
           notify_write = little_endian_read_16(buffer, 0);
       }
    }
    return 0;
}

static int att_write_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if(attribute_handle == PROV_DATA_IN_HDL || attribute_handle == PROXY_DATA_IN_HDL)
    {
        //sent to the queue of mesh
        memset(&mesh_event,0,sizeof(mesh_event));
        mesh_event.type = BLE_GAP_EVENT_CONNECT;   // TODO: ext adv or legal adv.
        mesh_event.connect.conn_handle= con_handle;
        mesh_event.connect.attr_handle= attribute_handle;
        uint8_t * ptr = NULL;
        DTBT_msg_t *proxy_data = btstack_memory_dtbt_msg_get();
        dbg_printf("size 0x%x\n",buffer_size);
        if(proxy_data==NULL)
        {
            dbg_printf("warning buffer full\n");
            return -1;
        }
        dbg_printf("buffer size 0x%x and mesh_event 0x%x\n",buffer_size,sizeof(mesh_event));
        ASSERT_ERR(HCI_ACL_PAYLOAD_SIZE+4 > (buffer_size+sizeof(mesh_event)));
        mesh_event.connect.data = proxy_data->data + sizeof(mesh_event);
        mesh_event.connect.length= buffer_size;
        ptr = proxy_data->data+sizeof(mesh_event);
        memcpy(proxy_data->data,&mesh_event,sizeof(mesh_event));
        memcpy(ptr,buffer,buffer_size);
        Host2Mesh_msg_send((uint8_t*)(proxy_data->data),sizeof(mesh_event)+buffer_size);
    }
    else if (attribute_handle == CLIENT_CHARAC_CONF)
    {
        mesh_write_callback(con_handle,attribute_handle,transaction_mode,offset,buffer,buffer_size);
    }
    else
    {
    }
    return 0;
}

static void user_packet_handler (uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size)
{
    uint8_t event = hci_event_packet_get_type(packet);
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (event) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                break;
            }
#if (FLASH_ENABLE)
            mesh_flash_sys_init();
#endif
            mesh_event.type = STACK_INIT_DONE;
            Host2Mesh_msg_send((uint8_t *)&mesh_event,sizeof(mesh_event));
            break;
        case HCI_EVENT_COMMAND_COMPLETE:
                switch (hci_event_command_complete_get_command_opcode(packet))
                {
                    case HCI_VENDOR_CCM:
                    {
                        cmpl = decode_hci_event_vendor_ccm_complete(packet);
                        dbg_printf("tag     = 0x%08x\n", cmpl->tag);
                        dbg_printf("out_msg = 0x%p status 0x%x\n", cmpl->out_msg,cmpl->status);
                        set_ccm_result(cmpl->status);
                        k_sem_give(&ccm_sem);
                        break;
                    }

                    case (0x2017):      //it means that hci_le_encrypt
                    {
                        if(get_aes_result()==-1) //it means that
                        {
                            set_aes_result(0);
                            reverse_128(&packet[6],ptr_aestext);
                            k_sem_give(&aes_sem);
                        }
                    }
                        break;
                    default:
                        break;
                }
            break;

        case HCI_EVENT_LE_META:
                switch (hci_event_le_meta_get_subevent_code(packet))
                {
                    case HCI_SUBEVENT_LE_EXTENDED_ADVERTISING_REPORT:
                    {
                        //need to filter the message at first, only for the mesh message
                        if(filtermeshADV(HCI_EVENT_PACKET,(uint8_t *)packet,size) == DISCARDED)
                            break;
                        //it is mesh packet ,then need to find whether its duplicated with previouse one, no need to deliver to mesh task
                        if(memcmp(pre_adv,packet,size)==0)
                        {
                            dbg_printf("this mesh packet is duplicated\r\n");
                            break;
                        }
                        const le_ext_adv_report_t *report = decode_hci_le_meta_event(packet, le_meta_event_ext_adv_report_t)->reports;
                        memset(&mesh_event,0,sizeof(mesh_event));
                        mesh_event.type = BLE_GAP_EVENT_EXT_DISC;   // TODO: ext adv or legal adv.
                        mesh_event.ext_disc.addr.type= report->addr_type;
                        mesh_event.ext_disc.props = report->evt_type;
                        memcpy(mesh_event.ext_disc.addr.val,report->address,6);
                        mesh_event.ext_disc.rssi = report->rssi;
                        mesh_event.ext_disc.legacy_event_type = report->evt_type | 0x10;
                        mesh_event.ext_disc.length_data = report->data_len;
                        //dbg_printf("add 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",report->address[0],report->address[1],report->address[2],report->address[3],report->address[4],report->address[5]);
                        if(report->data_len==0)
                        {
                        //    dbg_printf("len0\n");
                            return;
                        }
                        DTBT_msg_t *padv = btstack_memory_dtbt_msg_get();
                        //MEM_CHECK_CPY(padv->data,report->data,report->data_len);
                        if(padv==NULL)
                        {
                            dbg_printf("warning:full\n");
                            return ;
                        }
                        ASSERT_ERR(HCI_ACL_PAYLOAD_SIZE+4 > (report->data_len+sizeof(mesh_event)));
                        #define CACHE_CURRENT_ADV_PACKET(to,from,size) \
                        {\
                            memset(to,0,sizeof(to));\
                            memcpy(to,from,size);\
                        }
                        //store current packet as last received pre_adv.
                        CACHE_CURRENT_ADV_PACKET(pre_adv,packet,size);
                        mesh_event.ext_disc.data = padv->data + sizeof(mesh_event);
                        reverse_bd_addr(report->address,mesh_event.ext_disc.addr.val);  //must reverse the addr
                        mesh_event.ext_disc.addr.type = (bd_addr_type_t)report->addr_type;
                        memcpy(padv->data,&mesh_event,sizeof(mesh_event));
                        memcpy(padv->data+sizeof(mesh_event),(uint8_t*)report->data,report->data_len);
                        //dbg_printf("addr 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",report->address[0],report->address[1],report->address[2],report->address[3],report->address[4],report->address[5]);
                        Host2Mesh_msg_send((uint8_t *)padv->data,sizeof(mesh_event)+report->data_len);
                    }
                        break;

                    case HCI_SUBEVENT_LE_ADVERTISING_SET_TERMINATED:
                        if (( 7== packet[4]) && (temp_conn_handle != 0xffff))  //it is the proxy_service adv set terminated and a connection had been established, 7 means PB_GATT_ADV_HANDLE
                        {
                            //check status here to decide whether it is just connected or not
                            if(0 != packet[3])
                            {
                                //TODO: unnormal event from controller ,so need to start adv again or not£¿
                                break;
                            }
                            mesh_event.type = ADV_TERM_CON;
                            conn_handle = mesh_event.connect.conn_handle = temp_conn_handle;
                            att_server_init(att_read_callback,att_write_callback);
                            Host2Mesh_msg_send((uint8_t *)&mesh_event,sizeof(mesh_event));
                            if(!bt_mesh_is_provisioned())
                            {
                                att_set_db(conn_handle,pb_gatt_service_data);
                            }
                            else
                            {
                                att_set_db(conn_handle,proxy_service_data);
                            }
                        }
                        break;

                    case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                    case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
                        dbg_printf("le conn 0x%x 0x%x\n", MYNEWT_VAL_BLE_MESH_PB_GATT, MYNEWT_VAL_BLE_MESH_GATT_PROXY);
                        temp_conn_handle = little_endian_read_16(packet,4);
                        break;

                    default:
                        break;
                }
             break;
            case HCI_EVENT_DISCONNECTION_COMPLETE:
                if((MYNEWT_VAL(BLE_MESH_PB_GATT)) ||  (MYNEWT_VAL(BLE_MESH_GATT_PROXY)))
                {
                    dbg_printf("discon\n");
                    if (conn_handle == little_endian_read_16(packet, 3))
                    {
                        mesh_event.type = BLE_GAP_EVENT_DISCONNECT;
                        mesh_event.disconnect.reason = packet[2];
                        mesh_event.disconnect.conn.conn_handle = little_endian_read_16(packet,3);
                        Host2Mesh_msg_send((uint8_t*)&mesh_event,sizeof(mesh_event));
                    }
                }
                else
                {
                }
            break;

            case ATT_EVENT_CAN_SEND_NOW:


            break;

            case BTSTACK_EVENT_USER_MSG:
            {
                btstack_user_msg_t *p_user_msg;
                p_user_msg = (btstack_user_msg_t*)hci_event_packet_get_user_msg(packet);
                user_msg_handler(p_user_msg);
            }
            break;
        default:
            break;
    }
}


void mesh_platform_setup()
{
    memcpy(param,&BLE_MESH_DEV_PRODUCT_ID,sizeof(uint32_t));
    memcpy(param+4,&TM_AUTHEN_VAL_LEN,sizeof(uint32_t));
    memcpy(param+8,SEC,16);
    mesh_platform_config(1,PB_ADV_ADDR,param);
    mesh_platform_config(2,PB_GATT_MAC_ADDR,NULL);
}

void mesh_setup(const struct bt_mesh_prov *a_prov, const struct bt_mesh_comp *a_comp)
{
    struct bt_mesh_elem *root = a_comp->elem;
    struct bt_mesh_model *model = root->models;
    u8_t model_count;

    p_prov = a_prov;
    p_comp = a_comp;
    // setup configure client & server op
    for (model_count = 0; model_count < root->model_count; model_count++)
    {
        if (BT_MESH_MODEL_ID_CFG_SRV == model[model_count].id)
        {
            *((const struct bt_mesh_model_op **)&model[model_count].op) = bt_mesh_cfg_srv_op;
        }
        if (BT_MESH_MODEL_ID_HEALTH_SRV == model[model_count].id)
            *((const struct bt_mesh_model_op **)&model[model_count].op) = bt_mesh_health_srv_op;
#if MYNEWT_VAL(BLE_MESH_CFG_CLI)
        if (BT_MESH_MODEL_ID_CFG_CLI == model[model_count].id)
            *((const struct bt_mesh_model_op **)&model[model_count].op) = bt_mesh_cfg_cli_op;
#endif
    }
}

int mesh_init0()
{
    int err;
    dbg_printf("Bluetooth initialized\n");

#if (MYNEWT_VAL(BLE_MESH_SHELL))
    ble_mesh_shell_init();       //note: this shell init should follow PLATFORM_CB_EVT_PROFILE_INIT.
#endif

    if ((NULL == p_prov) || (NULL == p_comp)) {
        PANIC("prov/comp not initialized");
    }

    err = bt_mesh_init(1, p_prov, p_comp);
    if (err) {
        PANIC("Initializing mesh failed (err %d)\n", err);
    }

    return 0;
}

int mesh_main(int argc, char **argv)
{
    mesh_init0();

    /* Make sure we're scanning for provisioning inviations */
    /* Enable unprovisioned beacon sending */
    bt_mesh_beacon_init();
    return 0;
}

void mesh_service_trigger(uint8_t* mesh_msg,uint8_t len)
{
    memset(&mesh_event, 0, sizeof(mesh_event));
    mesh_event.type = BLE_MESH_APP_EVENT;
    DTBT_msg_t *pdata = btstack_memory_dtbt_msg_get();
    if(pdata == NULL) {
        dbg_printf("buf full\n");
        return;
    }
    ASSERT_ERR(HCI_ACL_PAYLOAD_SIZE + 4 > (len+sizeof(mesh_event)));
    mesh_event.type = BLE_MESH_APP_EVENT;
    mesh_event.mesh_app.length = len;
    mesh_event.mesh_app.data = pdata->data + sizeof(mesh_event);
    memcpy(pdata->data, &mesh_event, sizeof(mesh_event));
    memcpy(pdata->data+sizeof(mesh_event), mesh_msg,len);
    Host2Mesh_msg_send((uint8_t*)(pdata->data), sizeof(mesh_event) + len);
}

int mesh_srv_restart()
{
    dbg_printf("reset dis conn %d\n",mesh_event.connect.conn_handle);
    if (conn_handle != 0xffff) {
        hci_cmds_put(DISCONNECT,&conn_handle,2);
    } else {
        bt_mesh_gatt_config(MESH_GATT_PROV);
        mesh_start_advertising();
    }
    return 0;
}

void reset_con_handle()
{
    conn_handle = INVALID_HANDLE;
}


void gap_beacon_disable(void)
{
    if (!is_mesh_task())  {
        memset(&mesh_event,0,sizeof(mesh_event));
        mesh_event.type = BLE_MESH_BEACON_STATE;
        mesh_event.beacon_state.enable = 0;
        Host2Mesh_msg_send((uint8_t*)&mesh_event,sizeof(mesh_event));
    } else {
        bt_mesh_beacon_disable();
    }
}

void gap_beacon_enable(void)
{
    if (!is_mesh_task()) {
        memset(&mesh_event,0,sizeof(mesh_event));
        mesh_event.type = BLE_MESH_BEACON_STATE;
        mesh_event.beacon_state.enable = 1;
        Host2Mesh_msg_send((uint8_t*)&mesh_event,sizeof(mesh_event));
    } else {
        bt_mesh_beacon_enable();
    }
}

int8_t set_mesh_sleep_duration(uint32_t ms)
{
    if (ms >=20) {
        sleep_duration = ms;
        return EOK;
    } else {
        return -EPERM;
    }
}

void set_mesh_uart_output_func(mesh_at_out ptrfun)
{
    uart_printf = ptrfun;
}

void mesh_at_entry(uint8_t* mesh_msg ,uint8_t len)
{
    memcpy(mesh_at,mesh_msg,len);
    recv_from_uart = true;
}

int console_printf(const char *fmt, ...)
{
    if (uart_printf) {
        va_list args;
        int num_chars;
        int len;
        memset(buf,0,200);

        num_chars = 0;
        va_start(args, fmt);
        len = vsnprintf(buf, sizeof(buf), fmt, args);
        num_chars += len;
        if (len >= sizeof(buf)) {
            len = sizeof(buf) - 1;
        }
        uart_printf(buf, len);
        va_end(args);

        return num_chars;
    } else {
        return 0;
    }
}

void mesh_trace_config(uint16_t sel_bits,uint16_t cla_bit)
{
    fea_layer_sel =  sel_bits;
    cla_flag = cla_bit;
    return;
}

int8_t mesh_provision_start(void)
{
    //check whether the device is provisioned or not
    if (is_provisioned_poweron() || bt_mesh_is_provisioned()) {
        memset(&mesh_event,0,sizeof(mesh_event));
        mesh_event.type = DEVICE_ROLE;
        mesh_event.device_role.role = PROVISIONER_ROLE;
        Host2Mesh_msg_send((uint8_t*)&mesh_event,sizeof(mesh_event));
        return EOK;
    } else {
        return -1;
    }

}

int8_t mesh_provision_disable()
{
    memset(&mesh_event,0,sizeof(mesh_event));
    mesh_event.type = DEVICE_ROLE;
    mesh_event.device_role.role = NODE_ONLY;
    Host2Mesh_msg_send((uint8_t*)&mesh_event,sizeof(mesh_event));
    return EOK;
}

void cfg_cli_callback_set(config_status f_status)
{
    f_config_status =  f_status;
}

void mesh_clear_whitlist(void)
{
    hci_cmds_put(WHITELST_RMV,NULL,0);
    return;
}

void uuid_filter_set(const uint8_t * uuid,uint8_t len)
{
    if (len==0) {
        return;
    }
    memcpy(uuid_head,uuid,len);
    uuid_head_len = len;
    dbg_printf("set the filter uuid 0x%x 0x%x 0x%x \n",uuid_head[0],uuid_head[1],uuid_head[2]);
}

void model_init()
{
    dbg_printf("begin to setup comp\n");
    nimble_port_init();
    init_pub();
    model_info_pub();
    mesh_setup(&prov, get_comp_of_node());
    model_conf_init();
}

int mesh_env_init(void)
{
    sysSetPublicDeviceAddr(addr);
    mesh_platform_setup();
    mesh_memory_init();
    bt_mesh_handle_set(0x03, 0x05, 0x03, 0x05);//here set the handle of  proxy and prov
    mesh_feature_set(BT_MESH_FEAT_RELAY|BT_MESH_FEAT_PROXY,BT_MESH_FEAT_PROXY); //setup the feature_role

    model_init();

    att_server_init(att_read_callback, att_write_callback);
    hci_event_callback_registration.callback = &user_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(&user_packet_handler);
    mesh_main(0, NULL);

    return 0;
}

#if defined __cplusplus
    }
#endif

