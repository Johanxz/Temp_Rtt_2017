/*
 * File      : gpio.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-12     Johan      the first version
 */
#ifndef ADC_H__
#define ADC_H__

#include "stm32f10x.h"
#include <rtdef.h>

void rt_hw_adc_init(void);
void filter(void);
rt_uint16_t getTemp(rt_uint16_t AdValue);

#endif
