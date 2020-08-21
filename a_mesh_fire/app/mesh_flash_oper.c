
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "ingsoc.h"
#include "mesh_api.h"
#include "eflash.h"
#include "profile.h"
#include "kv_storage.h"

#ifdef __cplusplus
#extern "C" {
#endif

#define INVALID_VAL              (0xffffffff)
#define PAGE_SIZE                (0x2000)

#define GET_LOWER_16BITS(x)  (x&0xffff)
#define ALIGNLEN4(len)  (((len+3)>>2)<<2)

static uint8_t counter = 0;
static struct k_delayed_work switch_onff;
static uint8_t  reset_count =4;


int8_t after_power_on()
{
    counter = 0;
    kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(counter));
    return 1;
}

int8_t fast_switch_monitor()
{
//	EflashCacheBypass();
//	EflashProgramEnable();
// 	EraseEFlashPage(0x28);
// 	EraseEFlashPage(0x29);
// 	EflashProgramDisable();
// 	EflashCacheEna();
// 	EflashCacheFlush();
    int16_t len = 0;
    uint8_t *db = NULL;
    k_delayed_work_init(&switch_onff,(ble_npl_event_fn *)after_power_on);
    //read from flash ,to get the state of last power off;
    db = kv_get(FAST_POWER_ON_FLASH_INDEX,&len);
    if (!db)
    {
        counter =1;
        kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(uint8_t));
    }
    else
    {
        //get the current value;
        counter = *((uint8_t*)db);
        counter++;
        printf("counter 0x%x\n",counter-1);
    }
    if(counter>=reset_count)
    {
           printf("reset start\n");
           counter = 0;
           initial_fac_conf();
           return 1;
    }
    else
    {
        kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(uint8_t));
        kv_commit();
    }
    platform_printf("5s start\n");
    k_delayed_work_submit(&switch_onff,5000);
    return 0;
}

/**
* @brief  to get the last valid flash address for mesh configuration
*
* @param[in] db_size     the memory block size to be written into flash
*
* @param[ou] last_addr   last valid flash start address entry to save configuration
*
* @return    the address of a flash to which curren memory block could be written
*
* @note      if could not get a consecutive flash memory of db_size bytes in one flash page ,then return NULL.
*/
uint8_t* mesh_flashpage_valid_check(uint16_t db_size, uint32_t* last_addr)
{
    //the block should be aligned with 4 bytes ,so need to adjust the search length.
    //to position the next flash address.
    uint8_t* ptr = (uint8_t*) MESH_FLASH_ADDRESS;
    uint32_t len = *((uint32_t*)MESH_FLASH_ADDRESS);
    if(len==INVALID_VAL)
        return ptr;
    else
    {
        *last_addr = (uint32_t) ptr;
        ptr += ALIGNLEN4(GET_LOWER_16BITS(len)+2); //2 means the size element in db takes up 2 bytes.
    }
    while((len = *(uint32_t*)ptr) != INVALID_VAL && (((uint32_t)ptr - MESH_FLASH_ADDRESS)<PAGE_SIZE))
    {
        *last_addr = (uint32_t)ptr;
        ptr += ALIGNLEN4(GET_LOWER_16BITS(len)+2);
    }
    if (((uint32_t)ptr-MESH_FLASH_ADDRESS)>PAGE_SIZE)
    {
        //need to erase the page
        return NULL;
    }
    else if ((uint32_t)ptr + db_size +2 > (PAGE_SIZE + MESH_FLASH_ADDRESS))
    {
        return NULL;
    }
    else
        return ptr;
}

/**
* @brief  to reset the node to a unprovisioned device. remove the provisioned environm
*
* @note   write a 0xA0A0A0A0 to mark it is reset operation. remove all other network key ,appkey etc. related to provisioned environment.
*/
uint8_t initial_fac_conf()
{
    extern uint8_t counter;
    kv_remove_all();
    kv_put(FAST_POWER_ON_FLASH_INDEX,&counter,sizeof(uint8_t));
    write_control_word2mirror(0xA0A0A0A0);    //better not use 0xffffffff
    kv_commit();
    return 0;
}

int db_mesh_write_to_flash(const void *db, const int size)
{
    program_flash(MESH_FLASH_ADDRESS, db, size);
    return KV_OK;
}

int read_mesh_from_flash(void *db, const int max_size)
{
    memcpy((uint8_t*)db,(uint8_t*)(MESH_FLASH_ADDRESS), max_size);
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

