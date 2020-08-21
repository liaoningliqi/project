
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __BLE_MESH_H__
#define __BLE_MESH_H__

#include <stdint.h>

#if defined __cplusplus
    extern "C" {
#endif

typedef void (*mesh_at_out)(const char *str, int cnt);

void mesh_set_dev_name(const char *name);
void set_mesh_uart_output_func(mesh_at_out ptrfun);
void mesh_trace_config(uint16_t sel_bits,uint16_t cla_bit);

#if defined __cplusplus
    }
#endif

#endif


