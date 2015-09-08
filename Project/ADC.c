/*******************************************************************************
  * @file    ADC.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.08.12
  * @brief   电压测量模块
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ADC.h"
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static	FP32	g_currVoltage;
static	VM_ERR	g_err;
/* Private functions ---------------------------------------------------------*/
static	void	VM_Working(void);
static	INT32U	VM_Fltavg(INT32U dat);
/******************************************************************************
  * @brief  初始化电压测量模块
  * @param  None
  * @retval TRUE
******************************************************************************/
Std_ReturnType VM_Init(void)
{
	Std_ReturnType rtn;

	ADC_InitTypeDef ADC_InitStructure;
	/* Enable ADC1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_AHBPeriph_GPIOA, ENABLE);
	Pin_VMADC_Init();

	/* Enable HSI clock for ADC clock */
	RCC_HSICmd(ENABLE);
	/* Wait till HSI is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET){};

	/* de-initialize ADC ---------------------------------------------------------*/
	ADC_DeInit(ADC1);

    /* ADC Configuration */
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel2 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_384Cycles);

	ADC_DelaySelectionConfig(ADC1, ADC_DelayLength_Freeze);

	ADC_PowerDownCmd(ADC1, ADC_PowerDown_Idle_Delay, ENABLE);
  
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
  
	/* Wait until ADC1 ON status */
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET){};

	/* Start ADC1 Conversion using Software trigger */
	ADC_SoftwareStartConv(ADC1);

	rtn = Timer_Regist(LOOP,100,VM_Working);
		if(rtn == FALSE)g_err.ErrTimerBit = 1;
		else	g_err.ErrTimerBit = 0;

	g_err.TimeouteBit = 0;		
	g_err.OutrangeBit = 0;		
	g_err.VolLowBit = 0;		

	if(g_err.ErrTimerBit == 1)return FALSE;
	else					  return TRUE;
}
/******************************************************************************
  * @brief	电压测量模块服务工作，定时执行该函数，读取温度值，输出到全局变量   
  * @param  None
  * @retval None
******************************************************************************/
void	VM_Working(void)
{
	INT32U	 adcvalue;

	if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != RESET)
	{
		adcvalue = ADC_GetConversionValue(ADC1);
		if((adcvalue > 4000)||(adcvalue < 10))
			g_err.OutrangeBit =	1;
		else
			g_err.OutrangeBit =	0;	
		adcvalue = VM_Fltavg(adcvalue);
		g_currVoltage = adcvalue*56.1/4096 + 0.286;	//参考电压为 3.3V 分压比例为 1/17
		g_err.TimeouteBit =	0;
		if(g_currVoltage < THRESHOLD_LOW)
			g_err.VolLowBit = 1;
		else
			g_err.VolLowBit = 0; 
	}
	else
		g_err.TimeouteBit =	1; 
}
/******************************************************************************
  * @brief  初始化电压测量模块
  * @param  None
  * @retval TRUE
******************************************************************************/
FP32	VM_Read(void)
{
	return	g_currVoltage;
}
/******************************************************************************
  * @brief	读取电压测量模块错误标志  
  * @param  errtemp：返回的结构体指针
  * @retval None
******************************************************************************/
void	VM_ErrRead(VM_ERR* errtemp)
{
	errtemp->TimeouteBit = g_err.TimeouteBit;
	errtemp->OutrangeBit = g_err.OutrangeBit;
	errtemp->ErrTimerBit = g_err.ErrTimerBit;
	errtemp->VolLowBit = g_err.VolLowBit;
}
/******************************************************************************
  * @brief  
  * @param  None
  * @retval TRUE
******************************************************************************/
#define		NUM_DATBUF    	10
INT32U	VM_Fltavg(INT32U dat)
{
	INT32U datsum,i;
static INT32U	DatBuf[NUM_DATBUF];
static INT32U pDatBuf = 0;

	DatBuf[pDatBuf] = dat;
	pDatBuf++;
	if(pDatBuf >= NUM_DATBUF)pDatBuf = 0;
	datsum = 0;
	for(i=0;i<NUM_DATBUF;i++)datsum += DatBuf[i];
	datsum = datsum/NUM_DATBUF;
	return datsum;
}



