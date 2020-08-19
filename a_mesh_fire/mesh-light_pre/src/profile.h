#ifndef PROFILE_H
#define PROFILE_H

#include <stdio.h>
#include "mesh_def.h"

#ifndef TRUE
#define TRUE 1
#define FALSE (!TRUE)
#endif

/**
 * define the customer defined FLASH key ID
 */
#define CUST_CONF_FLASH_INDEX (9)

/**
 * define the last power on status of FLASH key ID
 */
#define FAST_POWER_ON_FLASH_INDEX  (10)

#define INGCHIPS_COMP_ID           (0x06AC)
#define ALIbaba_COMP_ID            (0x01A8)
#define MESH_FLASH_ADDRESS         (0x4000+ 0x28*0x2000)


void model_init(void);

uint32_t setup_profile(void *data, void *user_data);

uint8_t *init_service(void);

#define INPUT_REPORT_KEYS_MAX   6

typedef struct remote_control_adv
{
		uint16_t ing_id;
    uint8_t control_type;
    uint8_t r;
    uint8_t g;
    uint8_t b;
		uint16_t syn_cnt;
} remote_control_adv_t;

typedef struct kb_report
{
    uint8_t modifier;
    uint8_t reserved;
    uint8_t codes[INPUT_REPORT_KEYS_MAX];
} kb_report_t;

void update_led_command(remote_control_adv_t remote_control_adv_t, uint8_t length);
#endif

