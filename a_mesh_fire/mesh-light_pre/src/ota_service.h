/*
* Copyright (C) INGCHIPS. All rights reserved.
* This code is INGCHIPS proprietary and confidential.
* Any use of the code for whatever purpose is subject to
* specific written permission of INGCHIPS.
*/

#ifndef _ota_service_h
#define _ota_service_h

//#include "basic_type_definitions.h"
#include <stdint.h>

#define INGCHIPS_UUID_PART1         0x33,0x45,0xc2
#define INGCHIPS_UUID_PART2         0x6f,0x36,0x45,0xc5,0x85,0x41,0x92,0xf5,0x67,0x28,0xd5,0xf3

#define INGCHIPS_UUID_OTA_SERVICE {INGCHIPS_UUID_PART1,0xf0,INGCHIPS_UUID_PART2}

#define INGCHIPS_UUID_OTA_VER     {INGCHIPS_UUID_PART1,0xf1,INGCHIPS_UUID_PART2}
#define INGCHIPS_UUID_OTA_CTRL    {INGCHIPS_UUID_PART1,0xf2,INGCHIPS_UUID_PART2}
#define INGCHIPS_UUID_OTA_DATA    {INGCHIPS_UUID_PART1,0xf3,INGCHIPS_UUID_PART2}

typedef struct prog_ver
{
    short major;
    char  minor;
    char  patch;
} prog_ver_t;

typedef struct ota_ver
{
    prog_ver_t platform;
    prog_ver_t app;
} ota_ver_t;

#define OTA_CTRL_START              0xAA // param: no
#define OTA_CTRL_PAGE_BEGIN         0xB0 // param: page address (32 bit), following DATA contains the data
#define OTA_CTRL_PAGE_END           0xB1 // param: size (16bit), crc (16bit)
#define OTA_CTRL_READ_PAGE          0xC0 // param: page address, following DATA reading contains the data
#define OTA_CTRL_REBOOT             0xFF // param: no

#define OTA_STATUS_DISABLED         0
#define OTA_STATUS_OK               1
#define OTA_STATUS_ERROR            2

void ota_init_service(void);

int ota_write_callback(uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int ota_read_callback(uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);

#endif
