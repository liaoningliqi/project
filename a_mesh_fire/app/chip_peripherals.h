
/*
** COPYRIGHT (c) 2020 by INGCHIPS
*/

#ifndef __CHIP_PERIPHERALS_H__
#define __CHIP_PERIPHERALS_H__

#if defined __cplusplus
    extern "C" {
#endif

#define PWM_R GIO_GPIO_10
#define PWM_G GIO_GPIO_7
#define PWM_B GIO_GPIO_6

#define PWM_WARM GIO_GPIO_0
#define PWM_COLD GIO_GPIO_9

void chip_peripherals_init(void);

#if defined __cplusplus
    }
#endif

#endif

