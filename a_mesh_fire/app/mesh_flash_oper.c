
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

#ifdef __cplusplus
}
#endif

