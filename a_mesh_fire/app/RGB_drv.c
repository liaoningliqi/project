/*  Bluetooth Mesh */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>
#include <stdio.h>
#include "RGB_drv.h"
#include <assert.h>
#include "mesh_def.h"
#include "light_model.h"

#define GPIO_R  PWM_R
#define GPIO_G  PWM_G
#define GPIO_B  PWM_B

#define GPIO_CW  PWM_COLD
#define GPIO_WW  PWM_WARM
#define MAX_PWM_NUM 12

void apPWM_Initialize (void)
{ 
    
    /*TODO :GPIO configure mapping to LED_R LED_G LED_B  */
    // PWM_R ,and PWM_G ,PWM_B should not in the same group of pwm channel.
    if((PWM_R/2== PWM_G/2) || (PWM_R/2==PWM_B/2) || (PWM_B/2== PWM_G/2)
       || PWM_R/2 == PWM_COLD/2 || (PWM_G/2==PWM_COLD/2) || (PWM_B/2== PWM_COLD/2) 
        ||(PWM_R/2== PWM_WARM/2) || (PWM_G/2==PWM_WARM/2) || (PWM_B/2== PWM_WARM/2)
        || (PWM_COLD/2== PWM_WARM/2) )
    {
        printf("PWM_R PWM_G PWM_B should not be assined from same PWM channel\n");
    }  
    light_power_on();  
}
 


