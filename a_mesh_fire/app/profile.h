
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __PROFILE_H__
#define __PROFILE_H__

#if defined __cplusplus
    extern "C" {
#endif

#define FAST_POWER_ON_FLASH_INDEX  (10)
#define CUST_CONF_FLASH_INDEX (9)

#define INGCHIPS_COMP_ID           (0x06AC)
#define ALIbaba_COMP_ID            (0x01A8)
#define MESH_FLASH_ADDRESS         (0x4000+ 0x28*0x2000)

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

uint32_t setup_profile(void *data, void *user_data);
void update_led_command(remote_control_adv_t remote_control_adv_t, uint8_t length);

#if defined __cplusplus
    }
#endif

#endif

