
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack.h"
#include "main_app_mesh_node.h"
#include "app_gatt_service.h"

const char * device_uuid_string = "001BDC0810210B0E0A0C000B0E0A0C00";

#define MESH_BLUEKITCHEN_MODEL_ID_TEST_SERVER   0x0000u

static mesh_model_t                 mesh_vendor_model;

static mesh_model_t                 mesh_generic_on_off_server_model;
static mesh_generic_on_off_state_t  mesh_generic_on_off_state;

static char gap_name_buffer[30];
static char gap_name_prefix[] = "Mesh ";

static btstack_packet_callback_registration_t hci_event_callback_registration;

#ifdef ENABLE_MESH_GATT_BEARER
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);
    bd_addr_t addr;
    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) break;
                    // setup gap name
                    gap_local_bd_addr(addr);
                    strcpy(gap_name_buffer, gap_name_prefix);
                    strcat(gap_name_buffer, bd_addr_to_str(addr));
                    break;
                default:
                    break;
            }
            break;
    }
}

#endif

static void mesh_provisioning_message_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;

    switch(packet[0]){
        case HCI_EVENT_MESH_META:
            switch(packet[2]){
                case MESH_SUBEVENT_PB_TRANSPORT_LINK_OPEN:
                    printf("Provisioner link opened");
                    break;
                case MESH_SUBEVENT_ATTENTION_TIMER:
                    printf("Attention Timer: %u\n", mesh_subevent_attention_timer_get_attention_time(packet));
                    break;
                case MESH_SUBEVENT_PB_TRANSPORT_LINK_CLOSED:
                    printf("Provisioner link close");
                    break;
                case MESH_SUBEVENT_PB_PROV_COMPLETE:
                    printf("Provisioning complete\n");
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static void mesh_state_update_message_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;

    switch(packet[0]){
        case HCI_EVENT_MESH_META:
            switch(packet[2]){
                case MESH_SUBEVENT_STATE_UPDATE_BOOL:
                    printf("State update: model identifier 0x%08x, state identifier 0x%08x, reason %u, state %u\n",
                        mesh_subevent_state_update_bool_get_model_identifier(packet),
                        mesh_subevent_state_update_bool_get_state_identifier(packet),
                        mesh_subevent_state_update_bool_get_reason(packet),
                        mesh_subevent_state_update_bool_get_value(packet));
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static void show_usage(void){
    bd_addr_t      iut_address;
    gap_local_bd_addr(iut_address);
    printf("\n--- Bluetooth Mesh Console at %s ---\n", bd_addr_to_str(iut_address));
    printf("8      - Delete provisioning data\n");
    printf("g      - Generic ON/OFF Server Toggle Value\n");
    printf("\n");
}

static void stdin_process(char cmd){
    switch (cmd){
        case '8':
            mesh_node_reset();
            printf("Mesh Node Reset!\n");
            mesh_proxy_start_advertising_unprovisioned_device();
            break;
        case 'g':
            printf("Generic ON/OFF Server Toggle Value\n");
            mesh_generic_on_off_server_set(&mesh_generic_on_off_server_model, 1-mesh_generic_on_off_server_get(&mesh_generic_on_off_server_model), 0, 0);
            break;
        case ' ':
            show_usage();
            break;
        default:
            printf("Command: '%c' not implemented\n", cmd);
            show_usage();
            break;
    }
}

static int scan_hex_byte(const char * byte_string){
    int upper_nibble = nibble_for_char(*byte_string++);
    if (upper_nibble < 0) return -1;
    int lower_nibble = nibble_for_char(*byte_string);
    if (lower_nibble < 0) return -1;
    return (upper_nibble << 4) | lower_nibble;
}

static int btstack_parse_hex(const char * string, uint16_t len, uint8_t * buffer){
    int i;
    for (i = 0; i < len; i++) {
        int single_byte = scan_hex_byte(string);
        if (single_byte < 0) return 0;
        string += 2;
        buffer[i] = (uint8_t)single_byte;
        // don't check seperator after last byte
        if (i == len - 1) {
            return 1;
        }
        // optional seperator
        char separator = *string;
        if (separator == ':' && separator == '-' && separator == ' ') {
            string++;
        }
    }
    return 1;
}

int btstack_main(void);
int btstack_main(void)
{
#ifdef HAVE_BTSTACK_STDIN
    btstack_stdin_setup(stdin_process);
#endif

    btstack_crypto_init();

#ifdef ENABLE_MESH_GATT_BEARER
    l2cap_init();
    le_device_db_init();
    app_gatt_service_init();
    sm_init();
#endif

#ifdef ENABLE_MESH_GATT_BEARER
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
#endif

    mesh_init();

#ifdef ENABLE_MESH_GATT_BEARER
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    uint8_t adv_type = 0;   // AFV_IND
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    adv_bearer_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
#endif

    // Track Provisioning as device role
    mesh_register_provisioning_device_packet_handler(&mesh_provisioning_message_handler);

    // Loc - bottom - https://www.bluetooth.com/specifications/assigned-numbers/gatt-namespace-descriptors
    mesh_node_set_element_location(mesh_node_get_primary_element(), 0x103);

    // Setup Generic On/Off model
    mesh_generic_on_off_server_model.model_identifier = mesh_model_get_model_identifier_bluetooth_sig(MESH_SIG_MODEL_ID_GENERIC_ON_OFF_SERVER);
    mesh_generic_on_off_server_model.operations = mesh_generic_on_off_server_get_operations();
    mesh_generic_on_off_server_model.model_data = (void *) &mesh_generic_on_off_state;
    mesh_generic_on_off_server_register_packet_handler(&mesh_generic_on_off_server_model, &mesh_state_update_message_handler);
    mesh_element_add_model(mesh_node_get_primary_element(), &mesh_generic_on_off_server_model);

    // Setup our custom model
    mesh_vendor_model.model_identifier = mesh_model_get_model_identifier(BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, MESH_BLUEKITCHEN_MODEL_ID_TEST_SERVER);
    mesh_element_add_model(mesh_node_get_primary_element(), &mesh_vendor_model);

    // Enable Output OOB
    provisioning_device_set_output_oob_actions(0x08, 0x08);

    // Enable PROXY
    mesh_foundation_gatt_proxy_set(1);

#if defined(ENABLE_MESH_ADV_BEARER)
    // setup scanning when supporting ADV Bearer
    gap_set_scan_parameters(0, 0x300, 0x300);
    gap_start_scan();
#endif

    uint8_t device_uuid[16];
    btstack_parse_hex(device_uuid_string, 16, device_uuid);
    mesh_node_set_device_uuid(device_uuid);

    // turn on!
	hci_power_control(HCI_POWER_ON);

    return 0;
}
/* EXAMPLE_END */
