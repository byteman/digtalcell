/*******************************************************************************
  * @file    Pin.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.05.27
  * @brief   引脚管理模块
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Pin.h"
/* Private macro -------------------------------------------------------------*/
#define 	PIN_WORKLED		GPIO_Pin_12
#define 	PORT_WORKLED	GPIOA

#define 	PIN_I2C_SCL		GPIO_Pin_6
#define		Sour_I2C_SCL	GPIO_PinSource6
#define 	PORT_I2C_SCL	GPIOB
#define 	PIN_I2C_SDA		GPIO_Pin_7
#define		Sour_I2C_SDA	GPIO_PinSource7
#define 	PORT_I2C_SDA	GPIOB

#define 	PIN_RS422RX		GPIO_Pin_10
#define		Sour_RS422RX	GPIO_PinSource10
#define 	PORT_RS422RX	GPIOA
#define 	PIN_RS422TX		GPIO_Pin_9
#define		Sour_RS422TX	GPIO_PinSource9
#define 	PORT_RS422TX	GPIOA
#define 	PIN_RS422CON	GPIO_Pin_15
#define 	PORT_RS422CON	GPIOB

#define 	PIN_UART2RX		GPIO_Pin_3
#define		Sour_UART2RX	GPIO_PinSource3
#define 	PORT_UART2RX	GPIOA
#define 	PIN_UART2TX		GPIO_Pin_2
#define		Sour_UART2TX	GPIO_PinSource2
#define 	PORT_UART2TX	GPIOA

#define 	PIN_AD7195CS	GPIO_Pin_4
#define 	PORT_AD7195CS	GPIOA
#define 	PIN_AD7195SCLK	GPIO_Pin_3
#define 	PORT_AD7195SCLK	GPIOA
#define 	PIN_AD7195DIN	GPIO_Pin_6
#define 	PORT_AD7195DIN	GPIOA
#define 	PIN_AD7195DOUT	GPIO_Pin_5
#define 	PORT_AD7195DOUT	GPIOA
#define 	PIN_AD7195SYNC	GPIO_Pin_7
#define 	PORT_AD7195SYNC	GPIOA

#define 	PIN_CS1237SCLK	GPIO_Pin_8
#define 	PORT_CS1237SCLK	GPIOB
#define 	PIN_CS1237DOUT	GPIO_Pin_9
#define 	PORT_CS1237DOUT	GPIOB

#define 	PIN_DS18B20DQ	GPIO_Pin_12
#define 	PORT_DS18B20DQ	GPIOA

#define 	PIN_PwrVol		GPIO_Pin_2
#define 	PORT_PwrVol		GPIOA

/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void	WorkLED_Init(void);

/******************************************************************************
  * @brief  全部用到的引脚初始化
  * @param  None
  * @retval TRUE: 初始化成功	FALSE: 初始化失败 
******************************************************************************/
Std_ReturnType	Pin_Init(void)
{
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	WorkLED_Init();
	return TRUE;
}
/******************************************************************************
  * @brief  I2C 引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	Pin_I2C_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure I2C pins: SCL and SDA */
	GPIO_InitStructure.GPIO_Pin   =  PIN_I2C_SCL;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//漏极开路;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(PORT_I2C_SCL, &GPIO_InitStructure);

	/* Configure I2C pins: SCL and SDA */
	GPIO_InitStructure.GPIO_Pin   =  PIN_I2C_SDA;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//漏极开路;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(PORT_I2C_SDA, &GPIO_InitStructure);

	/* Connect PXx to I2C_SCL */
	GPIO_PinAFConfig(PORT_I2C_SCL, Sour_I2C_SCL, GPIO_AF_I2C1);

	/* Connect PXx to I2C_SDA */
	GPIO_PinAFConfig(PORT_I2C_SDA, Sour_I2C_SDA, GPIO_AF_I2C1);
}
/******************************************************************************
  * @brief  RS422引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	Pin_RS422_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable GPIO clock */
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	/* Connect PA9 to USARTx_Tx */
	GPIO_PinAFConfig(PORT_RS422TX, Sour_RS422TX, GPIO_AF_USART1);
	/* Connect PA10 to USARTx_Rx */
	GPIO_PinAFConfig(PORT_RS422RX, Sour_RS422RX, GPIO_AF_USART1);
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = PIN_RS422TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		//GPIO_OType_PP;GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//GPIO_PuPd_UP;
	GPIO_Init(PORT_RS422TX, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = PIN_RS422RX;
	GPIO_Init(PORT_RS422RX, &GPIO_InitStructure);

	/* Configure PB12(RS422_CON) in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin 	= PIN_RS422CON;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;		//GPIO_OType_PP;GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;		//GPIO_PuPd_UP;
	GPIO_Init(PORT_RS422CON, &GPIO_InitStructure);

	Pin_RS422CON_Set(RS422CON_Receive);
}
/******************************************************************************
  * @brief  RS422CON控制
  * @param  cnt：接收还是发送？
  * @retval None 
******************************************************************************/
void	Pin_RS422CON_Set(TYPE_RS422CON cnt)
{
	if(cnt == RS422CON_Receive)
	{
		GPIO_ResetBits(PORT_RS422CON, PIN_RS422CON);
	}
	else if(cnt == RS422CON_Send)
	{
		GPIO_SetBits(PORT_RS422CON, PIN_RS422CON);
	}
}
/******************************************************************************
  * @brief  UART2引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	Pin_UART2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable GPIO clock */

	/* Connect Pin to USARTx_Tx */
	GPIO_PinAFConfig(PORT_UART2TX, Sour_UART2TX, GPIO_AF_USART2);
	/* Connect Pin to USARTx_Rx */
	GPIO_PinAFConfig(PORT_UART2RX, Sour_UART2RX, GPIO_AF_USART2);
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = PIN_UART2TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		//GPIO_OType_PP;GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//GPIO_PuPd_UP;
	GPIO_Init(PORT_UART2TX, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = PIN_UART2RX;
	GPIO_Init(PORT_UART2RX, &GPIO_InitStructure);
}
/******************************************************************************
  * @brief  AD7195引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	Pin_AD7195_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin 	= PIN_AD7195SYNC;			//SYNC
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;//;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_Init(PORT_AD7195SYNC, &GPIO_InitStructure);
	GPIO_SetBits(PORT_AD7195SYNC, PIN_AD7195SYNC);

	GPIO_InitStructure.GPIO_Pin 	= PIN_AD7195CS;				//CS
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;//;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_Init(PORT_AD7195CS, &GPIO_InitStructure);
	GPIO_SetBits(PORT_AD7195CS, PIN_AD7195CS);

	GPIO_InitStructure.GPIO_Pin 	= PIN_AD7195SCLK;			//SCLK
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;//;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_Init(PORT_AD7195SCLK, &GPIO_InitStructure);
	GPIO_SetBits(PORT_AD7195SCLK, PIN_AD7195SCLK);

	GPIO_InitStructure.GPIO_Pin 	= PIN_AD7195DIN;			//DIN
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;//;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_Init(PORT_AD7195DIN, &GPIO_InitStructure);
	GPIO_SetBits(PORT_AD7195DIN, PIN_AD7195DIN);

	GPIO_InitStructure.GPIO_Pin 	= PIN_AD7195DOUT;			//DOUT
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;//;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_Init(PORT_AD7195DOUT, &GPIO_InitStructure);
}
/******************************************************************************
  * @brief  AD7195引脚设置（置高或置低）
  * @param  pin：引脚名称
  			cnt：引脚逻辑
  * @retval None 
******************************************************************************/
void	Pin_AD7195_Set(AD7195_PIN pin,TYPE_PinLogic cnt)
{
 	if(pin == SYNC)
	{
		if(cnt == PinLogic_High)GPIO_SetBits(PORT_AD7195SYNC, PIN_AD7195SYNC);
		else GPIO_ResetBits(PORT_AD7195SYNC, PIN_AD7195SYNC);
	}
	else if(pin == SCLK)
	{
		if(cnt == PinLogic_High)GPIO_SetBits(PORT_AD7195SCLK, PIN_AD7195SCLK);
		else GPIO_ResetBits(PORT_AD7195SCLK, PIN_AD7195SCLK);
	}
	else if(pin == DIN)
	{
		if(cnt == PinLogic_High)GPIO_SetBits(PORT_AD7195DIN, PIN_AD7195DIN);
		else GPIO_ResetBits(PORT_AD7195DIN, PIN_AD7195DIN);
	}
	else if(pin == CS)
	{
		if(cnt == PinLogic_High)GPIO_SetBits(PORT_AD7195CS, PIN_AD7195CS);
		else GPIO_ResetBits(PORT_AD7195CS, PIN_AD7195CS);
	}
}
/******************************************************************************
  * @brief  AD7195读取引脚状态（高或低）
  * @param  None
  * @retval 引脚逻辑 
******************************************************************************/
TYPE_PinLogic	Pin_AD7195_ReadDOUT(void)
{
	if(GPIO_ReadInputDataBit(PORT_AD7195DOUT, PIN_AD7195DOUT) == Bit_RESET)
		return PinLogic_Low;
	else 
		return PinLogic_High;
}
/******************************************************************************
  * @brief  CS1237引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	Pin_CS1237_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin 	= PIN_CS1237SCLK;			//SCLK
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;//;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_Init(PORT_CS1237SCLK, &GPIO_InitStructure);
	GPIO_ResetBits(PORT_CS1237SCLK, PIN_CS1237SCLK);

	GPIO_InitStructure.GPIO_Pin 	= PIN_CS1237DOUT;			//DOUT
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;//;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_Init(PORT_CS1237DOUT, &GPIO_InitStructure);
	GPIO_SetBits(PORT_CS1237DOUT, PIN_CS1237DOUT);
}
/******************************************************************************
  * @brief  CS1237引脚设置（置高或置低）
  * @param  pin：引脚名称
  			cnt：引脚逻辑
  * @retval None 
******************************************************************************/
void	Pin_CS1237_Set(CS1237_PIN pin,TYPE_PinLogic cnt)
{
	if(pin == ADSCLK)
	{
		if(cnt == PinLogic_High)GPIO_SetBits(PORT_CS1237SCLK, PIN_CS1237SCLK);
		else GPIO_ResetBits(PORT_CS1237SCLK, PIN_CS1237SCLK);
	}
	else if(pin == ADDOUT)
	{
		if(cnt == PinLogic_High)GPIO_SetBits(PORT_CS1237DOUT, PIN_CS1237DOUT);
		else GPIO_ResetBits(PORT_CS1237DOUT, PIN_CS1237DOUT);
	}
}
/******************************************************************************
  * @brief  CS1237读取引脚状态（高或低）
  * @param  None
  * @retval 引脚逻辑 
******************************************************************************/
TYPE_PinLogic	Pin_CS1237_ReadDOUT(void)
{
	if(GPIO_ReadInputDataBit(PORT_CS1237DOUT, PIN_CS1237DOUT) == Bit_RESET)
		return PinLogic_Low;
	else 
		return PinLogic_High;
}
/******************************************************************************
  * @brief  DS18B20 一线总线引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	Pin_DS18B20_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin 	= PIN_DS18B20DQ;		
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;		//漏极开路;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_Init(PORT_DS18B20DQ, &GPIO_InitStructure);
	GPIO_SetBits(PORT_DS18B20DQ, PIN_DS18B20DQ);	
}
/******************************************************************************
  * @brief  DS18B20 DQ 引脚控制
  * @param  cnt：
  * @retval None 
******************************************************************************/
void	Pin_DS18B20_Set(TYPE_PinLogic cnt)
{
	if(cnt == PinLogic_Low)
	{
		GPIO_ResetBits(PORT_DS18B20DQ, PIN_DS18B20DQ);
	}
	else if(cnt == PinLogic_High)
	{
		GPIO_SetBits(PORT_DS18B20DQ, PIN_DS18B20DQ);
	}
}
/******************************************************************************
  * @brief  DS18B20 读取 DQ 引脚状态（高或低）
  * @param  None
  * @retval 引脚逻辑 
******************************************************************************/
TYPE_PinLogic	Pin_DS18B20_Read(void)
{
	if(GPIO_ReadInputDataBit(PORT_DS18B20DQ, PIN_DS18B20DQ) == Bit_RESET)
		return PinLogic_Low;
	else 
		return PinLogic_High;
}
/******************************************************************************
  * @brief  工作电压测量引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	Pin_VMADC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
                     
	GPIO_InitStructure.GPIO_Pin = PIN_PwrVol;		//PA2  ADC_IN2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(PORT_PwrVol, &GPIO_InitStructure);
}
/******************************************************************************
  * @brief  MCO引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	Pin_MCO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	/* Connect PA8 to MCO */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		//GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/******************************************************************************
  * @brief  WorkLED引脚初始化
  * @param  None
  * @retval None 
******************************************************************************/
void	WorkLED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin 	= PIN_WORKLED;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;//;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_Init(PORT_WORKLED, &GPIO_InitStructure);

	GPIO_SetBits(PORT_WORKLED, PIN_WORKLED);
}
/******************************************************************************
  * @brief  WorkLED控制
  * @param  cnt：灯亮还是灯灭
  * @retval None 
******************************************************************************/
void	WorkLED_Set(WorkLED_Cntrl cnt)
{
	if(cnt == WorkLED_On)
	{
		GPIO_ResetBits(PORT_WORKLED, PIN_WORKLED);
	}
	else if(cnt == WorkLED_Off)
	{
		GPIO_SetBits(PORT_WORKLED, PIN_WORKLED);
	}
}
/******************************************************************************
  * @brief  WorkLED闪烁，状态取反
  * @param  None
  * @retval None 
******************************************************************************/
void	WorkLED_Flash(void)
{
static	TYPE_PinLogic	s_LED_State = PinLogic_Low;

	if(s_LED_State == PinLogic_Low)
	{
		WorkLED_Set(WorkLED_On);
		s_LED_State = PinLogic_High;
	}
	else
	{
		WorkLED_Set(WorkLED_Off);
		s_LED_State = PinLogic_Low;
	}
}
