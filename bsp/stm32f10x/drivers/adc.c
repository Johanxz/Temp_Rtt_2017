#include <adc.h>
#include "math.h"


#define N 100 //????50?
extern uint8_t   ucSDiscInBuf[]  ;
extern uint8_t   ucSCoilBuf[]    ;
extern uint16_t   usSRegInBuf[]   ;
extern uint16_t   usSRegHoldBuf[] ;
vu16 AD_Value[N][5]; //????ADC????,??DMA?????
vu16 After_filter[5]; //?????????????

 rt_uint16_t getTemp(rt_uint16_t AdValue)
 {
	 rt_int16_t Temp1;
	 double R,Re,Temp;
	 
	 R = (AdValue!=0)?(4095.0 - AdValue)/(AdValue):4095;
	 
	 Re = log(R)/usSRegHoldBuf[0x20] + 1/298.15;
	 
	 Temp = (1/Re - 273.15);
	 
	 Temp = (Temp>0)?(Temp + 0.5):(Temp - 0.5);
	 
	 switch((usSRegHoldBuf[0x21]&0x0f))
	 {
		 case 0:
			 Temp1 = Temp;
			 break;
		 case 1:
			 Temp1 = 10 * Temp;
			 break;
		 case 2:
			 Temp1 = 100 * Temp;
			 break;
		 default:
			 Temp1 = Temp;
			 break;
	 }

	 return Temp1&0xffff;
 }

void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		
	DMA_DeInit(DMA1_Channel1); //?DMA???1?????????
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; //DMA??ADC???
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value; //DMA?????
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //????????????
	DMA_InitStructure.DMA_BufferSize = 5*N; //DMA???DMA?????
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //?????????
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //?????????
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //?????16?
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //?????16?
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //?????????
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA?? x??????
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //DMA??x????????????
	DMA_Init(DMA1_Channel1, &DMA_InitStructure); //??DMA_InitStruct?????????DMA???

}



void filter(void)
{
	int sum = 0;
	
	u8 count;
	for(int i=0;i<5;i++)
 
	{

	for ( count=0;count<N;count++)

	{

	sum += AD_Value[count][i];

	}

	After_filter[i]=sum/N;

	sum=0;
	}

}



void rt_hw_adc_init(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
	  ADC_InitTypeDef ADC_InitStructure;
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	   
		//配置ADC转换时钟
	  ADC_DeInit(ADC1);
		RCC_ADCCLKConfig(RCC_PCLK2_Div8); //9M
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	  ADC_InitStructure.ADC_NbrOfChannel = 5;
	  ADC_Init(ADC1,&ADC_InitStructure);

	  ADC_RegularChannelConfig(ADC1,ADC_Channel_1,1,ADC_SampleTime_55Cycles5);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_2,2,ADC_SampleTime_55Cycles5);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_3,3,ADC_SampleTime_55Cycles5);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_4,4,ADC_SampleTime_55Cycles5);
		ADC_RegularChannelConfig(ADC1,ADC_Channel_5,5,ADC_SampleTime_55Cycles5);
		ADC_DMACmd(ADC1, ENABLE);
	  ADC_Cmd(ADC1,ENABLE);
	  ADC_ResetCalibration(ADC1);               //复位校准
	  while(ADC_GetResetCalibrationStatus(ADC1));  //等待校准复位完毕
	  ADC_StartCalibration(ADC1);             //开始校准
	  while(ADC_GetCalibrationStatus(ADC1));  //等待校准完毕	
		
		DMA_Configuration();
		
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	  DMA_Cmd(DMA1_Channel1, ENABLE);
}
