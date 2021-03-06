/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#define BTSTACK_FILE__ "gatt_battery_query.c"

// *****************************************************************************
//
// BLE Client
//
// *****************************************************************************

#include "compiler.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack.h"

// gatt_battery_query.gatt contains the declaration of the provided GATT Services + Characteristics
// gatt_battery_query.h    contains the binary representation of gatt_battery_query.gatt
// it is generated by the build system by calling: $BTSTACK_ROOT/tool/compile_gatt.py gatt_battery_query.gatt gatt_battery_query.h
// it needs to be regenerated when the GATT Database declared in gatt_battery_query.gatt file is modified
#include "gatt_battery_query.h"

typedef struct advertising_report {
    uint8_t   type;
    uint8_t   event_type;
    uint8_t   address_type;
    bd_addr_t address;
    uint8_t   rssi;
    uint8_t   length;
    const uint8_t * data;
} advertising_report_t;


typedef enum {
    TC_IDLE,
    TC_W4_SCAN_RESULT,
    TC_W4_CONNECT,
    TC_W4_SERVICE_RESULT,
    TC_W4_CHARACTERISTIC_RESULT,
    TC_W4_BATTERY_DATA
} gc_state_t;

static int blacklist_index = 0;
static bd_addr_t blacklist[20];
static advertising_report_t report;

static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

static bd_addr_t cmdline_addr;
static int cmdline_addr_found = 0;

static hci_con_handle_t connection_handle;
static uint16_t battery_service_uuid = 0x180F;
static uint16_t battery_level_characteristic_uuid = 0x2a19;
static gatt_client_service_t battery_service;
static gatt_client_characteristic_t battery_level_characteristic;
    
static gc_state_t state = TC_IDLE;
static btstack_packet_callback_registration_t hci_event_callback_registration;

static gatt_client_notification_t notification_listener;
static int listener_registered;

static void printUUID(uint8_t * uuid128, uint16_t uuid16){
    if (uuid16){
        printf("%04x",uuid16);
    } else {
        printf("%s", uuid128_to_str(uuid128));
    }
}

static int blacklist_size(void){
    return sizeof(blacklist) / sizeof(bd_addr_t);
}

static int blacklist_contains(bd_addr_t addr){
    int i;
    for (i=0; i<blacklist_size(); i++){
        if (bd_addr_cmp(addr, blacklist[i]) == 0) return 1;
    }
    return 0;
}

static void add_to_blacklist(bd_addr_t addr){
    printf("%s added to blacklist (no battery service found\n", bd_addr_to_str(addr));
    bd_addr_copy(blacklist[blacklist_index], addr);
    blacklist_index = (blacklist_index + 1) % blacklist_size();
}

static void dump_advertising_report(advertising_report_t * e){
    printf("    * adv. event: evt-type %u, addr-type %u, addr %s, rssi %u, length adv %u, data: ", e->event_type,
           e->address_type, bd_addr_to_str(e->address), e->rssi, e->length);
    printf_hexdump(e->data, e->length);
    
}

static void dump_characteristic_value(uint8_t * blob, uint16_t blob_length){
    printf("    * characteristic value of length %d *** ", blob_length );
    printf_hexdump(blob , blob_length);
    printf("\n");
}

static void dump_characteristic(gatt_client_characteristic_t * characteristic){
    printf("    * characteristic: [0x%04x-0x%04x-0x%04x], properties 0x%02x, uuid ",
                            characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties);
    printUUID(characteristic->uuid128, characteristic->uuid16);
    printf("\n");
}

static void dump_service(gatt_client_service_t * service){
    printf("    * service: [0x%04x-0x%04x], uuid ", service->start_group_handle, service->end_group_handle);
    printUUID(service->uuid128, service->uuid16);
    printf("\n");
}


static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    int status;
    uint8_t battery_level;

    switch(state){
        case TC_W4_SERVICE_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_SERVICE_QUERY_RESULT:
                    gatt_event_service_query_result_get_service(packet, &battery_service);
                    dump_service(&battery_service);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("SERVICE_QUERY_RESULT - Error status %x.\n", packet[4]);
                        add_to_blacklist(report.address);
                        gap_disconnect(connection_handle);
                        break;  
                    } 
                    state = TC_W4_CHARACTERISTIC_RESULT;
                    printf("\nSearch for battery level characteristic.\n");
                    gatt_client_discover_characteristics_for_service_by_uuid16(handle_gatt_client_event, connection_handle, &battery_service, battery_level_characteristic_uuid);
                    break;
                default:
                    break;
            }
            break;
            
        case TC_W4_CHARACTERISTIC_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    gatt_event_characteristic_query_result_get_characteristic(packet, &battery_level_characteristic);
                    dump_characteristic(&battery_level_characteristic);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("CHARACTERISTIC_QUERY_RESULT - Error status %x.\n", packet[4]);
                        add_to_blacklist(report.address);
                        gap_disconnect(connection_handle);
                        break;  
                    } 

                    // register handler for notifications
                    listener_registered = 1;
                    gatt_client_listen_for_characteristic_value_updates(&notification_listener, handle_gatt_client_event, connection_handle, &battery_level_characteristic);

                    state = TC_W4_BATTERY_DATA;
                    printf("\nConfigure battery level characteristic for notify.\n");
                    status = gatt_client_write_client_characteristic_configuration(handle_gatt_client_event, connection_handle, &battery_level_characteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                    if (status != 0){
                        printf("\nNotification not supported. Query value of characteristic.\n");
                        gatt_client_read_value_of_characteristic(handle_gatt_client_event, connection_handle, &battery_level_characteristic);
                    }
                    
                    break;
                default:
                    break;
            }
            break;
        case TC_W4_BATTERY_DATA:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_NOTIFICATION:
                    printf("\nBattery Data:\n");
                    dump_characteristic_value(&packet[8], little_endian_read_16(packet, 6));
                    break;
                case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT:
                        
                        if (gatt_event_characteristic_value_query_result_get_value_length(packet) < 1) break;
                        battery_level = gatt_event_characteristic_value_query_result_get_value(packet)[0];
                        printf("Battery level %d \n", battery_level);
                    break;

                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("CHARACTERISTIC_VALUE_QUERY_RESULT - Error status %x.\n", packet[4]);
                        break;  
                    }
                    break;
                default:
                    printf("Unknown packet type %x\n", hci_event_packet_get_type(packet));
                    break;
            }
            break;

        default:
            printf("error\n");
            break;
    }
    
}

static void fill_advertising_report_from_packet(advertising_report_t * adv_report, uint8_t *packet){
    gap_event_advertising_report_get_address(packet, adv_report->address);
    adv_report->event_type = gap_event_advertising_report_get_advertising_event_type(packet);
    adv_report->address_type = gap_event_advertising_report_get_address_type(packet);
    adv_report->rssi = gap_event_advertising_report_get_rssi(packet);
    adv_report->length = gap_event_advertising_report_get_data_length(packet);
    adv_report->data = gap_event_advertising_report_get_data(packet);
}

static void hci_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    uint8_t event = hci_event_packet_get_type(packet);
	
    if (packet_type != HCI_EVENT_PACKET) return;
    
    switch (event) {
        case BTSTACK_EVENT_STATE:
            // BTstack activated, get started
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) break;
            if (cmdline_addr_found){
                printf("Connect to %s\n", bd_addr_to_str(cmdline_addr));
                state = TC_W4_CONNECT;
                gap_connect(cmdline_addr, 0);
                break;
            }
            printf("Start scanning!\n");
            state = TC_W4_SCAN_RESULT;
            gap_set_scan_parameters(0,0x0030, 0x0030);
            gap_start_scan();
            break;
        case GAP_EVENT_ADVERTISING_REPORT:
            if (state != TC_W4_SCAN_RESULT) return;
            fill_advertising_report_from_packet(&report, packet);
                
            if (blacklist_contains(report.address)) {
                break;
            }
            dump_advertising_report(&report);
            // stop scanning, and connect to the device
            state = TC_W4_CONNECT;
            gap_stop_scan();
            printf("Stop scan. Connect to device with addr %s.\n", bd_addr_to_str(report.address));
            gap_connect(report.address,report.address_type);
            
            break;
        case HCI_EVENT_LE_META:
            // wait for connection complete
            if (hci_event_le_meta_get_subevent_code(packet) !=  HCI_SUBEVENT_LE_CONNECTION_COMPLETE) break;
            if (state != TC_W4_CONNECT) return;
            connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
            // initialize gatt client context with handle, and add it to the list of active clients
            // query primary services
            printf("\nSearch for battery service.\n");
            state = TC_W4_SERVICE_RESULT;
            gatt_client_discover_primary_services_by_uuid16(handle_gatt_client_event, connection_handle, battery_service_uuid);
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            // unregister listener
            connection_handle = HCI_CON_HANDLE_INVALID;
            if (listener_registered){
                listener_registered = 0;
                gatt_client_stop_listening_for_characteristic_value_updates(&notification_listener);
            }
            
            if (cmdline_addr_found){
                printf("\nDisconnected %s\n", bd_addr_to_str(cmdline_addr));
                return;
            }

            printf("\nDisconnected %s\n", bd_addr_to_str(report.address));
            printf("Restart scan.\n");
            state = TC_W4_SCAN_RESULT;
            gap_start_scan();
            break;
        default:
            break;
    }
}

#ifdef HAVE_BTSTACK_STDIN
static void usage(const char *name){
    fprintf(stderr, "\nUsage: %s [-a|--address aa:bb:cc:dd:ee:ff]\n", name);
    fprintf(stderr, "If no argument is provided, GATT browser will start scanning and connect to the first found device.\nTo connect to a specific device use argument [-a].\n\n");
}
#endif

int btstack_main(int argc, const char * argv[]);
int btstack_main(int argc, const char * argv[]){

#ifdef HAVE_BTSTACK_STDIN
    int arg = 1;
    cmdline_addr_found = 0;
    
    while (arg < argc) {
        if(!strcmp(argv[arg], "-a") || !strcmp(argv[arg], "--address")){
            arg++;
            cmdline_addr_found = sscanf_bd_addr(argv[arg], cmdline_addr);
            arg++;
            if (!cmdline_addr_found) exit(1);
            continue;
        }
        usage(argv[0]);
        return 0;
    }
#else
    (void)argc;
    (void)argv;
#endif
    l2cap_init();

    // setup ATT server - only needed if LE Peripheral does ATT queries on its own, e.g. Android phones
    att_server_init(profile_data, NULL, NULL);    

    // GATT Client setup
    gatt_client_init();

    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);


    // turn on!
    hci_power_control(HCI_POWER_ON);

    return 0;
}




