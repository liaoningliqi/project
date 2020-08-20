/*  Bluetooth Mesh */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 #ifndef RGB_DRV_H
 #define RGB_DRV_H
 #include <stdint.h>
#include "ingsoc.h"
 
 #define PWM_GPIO_0                 0
 #define PWM_GPIO_1                 1
 #define PWM_GPIO_2                 2
 #define PWM_GPIO_3                 3
 #define PWM_GPIO_4                 4
 #define PWM_GPIO_5                 5
 #define PWM_GPIO_6                 6
 #define PWM_GPIO_7                 7
 #define PWM_GPIO_8                 8
 #define PWM_GPIO_9                 9
 #define PWM_GPIO_10                10
 #define PWM_GPIO_11                11

 #define PWM_R   PWM_GPIO_10
 #define PWM_G   PWM_GPIO_7
 #define PWM_B   PWM_GPIO_6
 
 #define PWM_IO_SEL_REG_OFFSET   0x68
 
 #define PWM_WARM  PWM_GPIO_0
 #define PWM_COLD  PWM_GPIO_9



/////////////////////////////////////
#define KEY_1 0x1e // Keyboard 1 and !
#define KEY_2 0x1f // Keyboard 2 and @
#define KEY_3 0x20 // Keyboard 3 and #
#define KEY_4 0x21 // Keyboard 4 and $
#define KEY_5 0x22 // Keyboard 5 and %
#define KEY_6 0x23 // Keyboard 6 and ^
#define KEY_7 0x24 // Keyboard 7 and &
#define KEY_8 0x25 // Keyboard 8 and *
#define KEY_9 0x26 // Keyboard 9 and (
#define KEY_0 0x27 // Keyboard 0 and )

 

 void  apPWM_Initialize (void);
 #endif

