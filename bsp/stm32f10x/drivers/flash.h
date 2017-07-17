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
#ifndef FLASH_H__
#define FLASH_H__

#include "stm32f10x.h"

void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);

#endif
