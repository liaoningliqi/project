
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "ingsoc.h"
#include "mesh_api.h"
#include "eflash.h"
#include "profile.h"
#include "kv_storage.h"

#include "..\..\project_common\project_common.h"

#ifdef __cplusplus
    #extern "C" {
#endif

#define INVALID_VAL (0xffffffff)
#define PAGE_SIZE (0x2000)

#define GET_LOWER_16BITS(x)  (x&0xffff)
#define ALIGNLEN4(len)  (((len+3)>>2)<<2)

static uint8_t counter = 0;
static struct k_delayed_work switch_onff;
static uint8_t  reset_count =4;

static int8_t after_power_on()
{
    counter = 0;
    kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(counter));
    return 1;
}

static uint8_t initial_fac_conf()
{
    extern uint8_t counter;
    kv_remove_all();
    kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(uint8_t));
    write_control_word2mirror(0xA0A0A0A0);
    kv_commit();
    return 0;
}

static int db_mesh_write_to_flash(const void *db, const int size)
{
    program_flash(MESH_FLASH_ADDRESS, db, size);
    dbg_printf("db_mesh_write_to_flash **********************\r\n");
    return KV_OK;
}

static int read_mesh_from_flash(void *db, const int max_size)
{
    memcpy((uint8_t*)db,(uint8_t*)(MESH_FLASH_ADDRESS), max_size);
    return 0;
}

static int8_t fast_switch_monitor()
{
    int16_t len = 0;
    uint8_t *db = NULL;

    k_delayed_work_init(&switch_onff,(ble_npl_event_fn *)after_power_on);

    db = kv_get(FAST_POWER_ON_FLASH_INDEX,&len);
    if (!db)
    {
        counter =1;
        kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(uint8_t));
    }
    else
    {
        counter = *((uint8_t*)db);
        counter++;
        dbg_printf("counter 0x%x\n",counter-1);
    }
    if(counter>=reset_count)
    {
           dbg_printf("reset start\n");
           counter = 0;
           initial_fac_conf();
           return 1;
    }
    else
    {
        kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(uint8_t));
        kv_commit();
    }
    dbg_printf("5s start\n");
    k_delayed_work_submit(&switch_onff,5000);
    return 0;
}

void fast_switch_monitor_init(void)
{
    kv_init(db_mesh_write_to_flash,read_mesh_from_flash);  // attention !! : must set the init func here prior to func fast_switch_monitor.
    fast_switch_monitor();

    return;
}

#ifdef __cplusplus
    }
#endif

