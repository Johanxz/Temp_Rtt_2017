/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */


#include "led.h"
#include "adc.h"
#include "flash.h"
#include "user_mb_app.h"
#include "rtdevice.h"

extern uint8_t   ucSDiscInBuf[]  ;
extern uint8_t   ucSCoilBuf[]    ;
extern uint16_t   usSRegInBuf[]   ;
extern uint16_t   usSRegHoldBuf[] ;

extern vu16 After_filter[5]; //AD寄存器

ALIGN(RT_ALIGN_SIZE)
static struct rt_thread thread_led;
static struct rt_thread thread_adc;
static struct rt_thread thread_ModbusSlavePoll;
//====================操作系统各线程堆栈====================================
static rt_uint8_t thread_led_stack[256];
static rt_uint8_t thread_adc_stack[512];
static rt_uint8_t thread_ModbusSlavePoll_stack[512];



void thread_led_entry(void* parameter)
{
  unsigned int count=0;
	
    rt_hw_led_init();

	while(1)
	{
				
				 count++;
				if(count%2)
				{
					rt_hw_led_on(0);
					rt_thread_delay( RT_TICK_PER_SECOND/2);
				}
				else
				{
					rt_hw_led_off(0);
					rt_thread_delay( RT_TICK_PER_SECOND/2 );
				}
				
	}
}






void thread_adc_entry(void* parameter)
{
  rt_hw_adc_init();
	  
    while (1)
    {
				IWDG_Feed(); //feed the dog
			  filter();
			 //I状态
			  ucSDiscInBuf[0] = ((After_filter[0]>2048)?1:0)|((After_filter[1]>2048)?0x2:0)|((After_filter[2]>2048)?0x4:0)
												|((After_filter[3]>2048)?0x8:0)|((After_filter[4]>2048)?0x10:0);
				
			  ucSDiscInBuf[1] = ((After_filter[0]>3400)?1:0)|((After_filter[1]>3400)?0x2:0)|((After_filter[2]>3400)?0x4:0)
												|((After_filter[3]>3400)?0x8:0)|((After_filter[4]>3400)?0x10:0);
			
			  ucSDiscInBuf[2] = ((After_filter[0]<680)?1:0)|((After_filter[1]<680)?0x2:0)|((After_filter[2]<680)?0x4:0)
												|((After_filter[3]<680)?0x8:0)|((After_filter[4]<680)?0x10:0);
			
			  ucSDiscInBuf[3] = ucSDiscInBuf[1]|ucSDiscInBuf[2];
			 
			
			//模拟量值
			  usSRegInBuf[0x00] = 0x0100;   	//	产品码   0x100  = 温度采集模块
				usSRegInBuf[0x01] = usSRegHoldBuf[0X00];
				usSRegInBuf[0x02] = usSRegHoldBuf[0X01];
				usSRegInBuf[0x03] = usSRegHoldBuf[0X02];
			  usSRegInBuf[0x04] = 0x0100;                //版本号
			
			  usSRegInBuf[0x28] = After_filter[0]&0x0fff;
			  usSRegInBuf[0x29] = After_filter[1]&0x0fff;
			  usSRegInBuf[0x2a] = After_filter[2]&0x0fff;
			  usSRegInBuf[0x2b] = After_filter[3]&0x0fff;
			  usSRegInBuf[0x2c] = After_filter[4]&0x0fff;	
				
				usSRegInBuf[0x20] = getTemp(usSRegInBuf[0x28]);
				usSRegInBuf[0x21] = getTemp(usSRegInBuf[0x29]);
				usSRegInBuf[0x22] = getTemp(usSRegInBuf[0x2a]);
				usSRegInBuf[0x23] = getTemp(usSRegInBuf[0x2b]);
				usSRegInBuf[0x24] = getTemp(usSRegInBuf[0x2c]);
			
			if(usSRegHoldBuf[0x2f]==0x8877)
			{
				usSRegHoldBuf[0x2f] = 0x0000;
				STMFLASH_Write((u32)0x0800fc00,usSRegHoldBuf,(u16)48);  //63K以上
				rt_hw_cpu_shutdown();
			}
			rt_thread_delay(RT_TICK_PER_SECOND/10);
    }
}

//************************ Modbus从机轮训线程***************************
//函数定义: void thread_entry_ModbusSlavePoll(void* parameter)
//入口参数：无
//出口参数：无
//备    注：Editor：Armink   2013-08-02    Company: BXXJS
//******************************************************************
void thread_ModbusSlavePoll_entry(void* parameter)
{
	
	UCHAR ucSlaveAddress = 1;
	ULONG ulBaudRate = 0;
	eMBParity eParity;
	
	STMFLASH_Read((u32)0x0800fc00,usSRegHoldBuf,(u16)48);  //63K以上
	
	//站号加载
  ucSlaveAddress = ((usSRegHoldBuf[0x10]&0x00ff)!=0)?(usSRegHoldBuf[0x10]&0x00ff):1;
	//波特率加载
	switch((usSRegHoldBuf[0x10]&0xff00)>>8)
	{
		case 0:
			ulBaudRate = BAUD_RATE_2400;
			break;
		case 1: 
			ulBaudRate = BAUD_RATE_4800;
		  break;
		case 2:
			ulBaudRate = BAUD_RATE_9600;
			break;
		case 3: 
			ulBaudRate = BAUD_RATE_19200;
		  break;
		case 4:
			ulBaudRate = BAUD_RATE_38400;
			break;
		case 5: 
			ulBaudRate = BAUD_RATE_57600;
		  break;	
		case 6: 
			ulBaudRate = BAUD_RATE_115200;
		  break;			
		default:
			ulBaudRate = BAUD_RATE_9600;
			break;
	}
	
	//加载校验方式
	switch((usSRegHoldBuf[0x11]&0xff00)>>8)
	{
		case 0:
			eParity = MB_PAR_NONE;
		  break;
		case 1:
			eParity = MB_PAR_EVEN;
		  break;
		case 2:
			eParity = MB_PAR_ODD;
		  break;
		default:
			eParity = MB_PAR_NONE;
		  break;
	};
	
	eMBInit(MB_RTU, ucSlaveAddress, 1, ulBaudRate,  eParity);
	
	eMBEnable();
	while (1)
	{
		eMBPoll();
	}
}




int rt_application_init(void)
{
    rt_err_t result;

		
 result = rt_thread_init(&thread_led, 
		                         "led",
														  thread_led_entry, 
														  RT_NULL, 
															thread_led_stack,
															sizeof(thread_led_stack), 
														  12,
															50);
															
		if (result == RT_EOK)
    {
       	rt_thread_startup(&thread_led);	
    }	
		
		result = rt_thread_init(&thread_adc, 
		                         "adc",
														  thread_adc_entry, 
														  RT_NULL, 
															thread_adc_stack,
															sizeof(thread_adc_stack), 
														  11,
															50);
															
		if (result == RT_EOK)
    {
       	rt_thread_startup(&thread_adc);	
    }	

		result = rt_thread_init(&thread_ModbusSlavePoll, 
		                         "MBSlavePoll",
														  thread_ModbusSlavePoll_entry, 
														  RT_NULL, 
															thread_ModbusSlavePoll_stack,
															sizeof(thread_ModbusSlavePoll_stack), 
														  10,
															50);
															
		if (result == RT_EOK)
    {
       	rt_thread_startup(&thread_ModbusSlavePoll);	
    }																				


    return 0;
}

/*@}*/
