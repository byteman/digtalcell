/*******************************************************************************
  * @file    Board.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.06.15
  * @brief   小板模块，小板的一些特性管理
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Board.h"
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static	BOARD_ERR	g_err;	
/* Private functions ---------------------------------------------------------*/
static	void	Board_I2C_Init(void);
static	void	Board_I2C_CheckFlag(INT32U I2C_FLAG);
static	void	Board_I2C_CheckEvent(INT32U I2C_EVENT);
/******************************************************************************
  * @brief  小板初始化
  * @param  
  * @retval TRUE: 初始化成功	FALSE: 初始化失败 
******************************************************************************/
Std_ReturnType	Board_Init(void)
{
	Board_MCO_Init();
	Board_I2C_Init();
	return TRUE;
}
/******************************************************************************
  * @brief	读取主板模块错误标志  
  * @param  errbd：返回的结构体指针
  * @retval None
******************************************************************************/
void	Board_ErrRead(BOARD_ERR* errbd)
{
	errbd->I2C_Busy = g_err.I2C_Busy;			//I2C忙
	errbd->I2C_Device_Err = g_err.I2C_Device_Err;	//设备问题
}
/******************************************************************************
  * @brief  软件复位
  * @param 	None 
  * @retval None 
******************************************************************************/
void	SoftReset(void) 
{
	__set_FAULTMASK(1);     // 关闭所有中端
	NVIC_SystemReset();		// 复位	  
}
/******************************************************************************
  * @brief  看门狗初始化
  * @param 	tm：看门狗时间，0.5s / 1s / 2s / 3s /默认最长3.542秒 
  * @retval None 
******************************************************************************/
void	WDT_Init(TYPE_WDTtime tm)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	//关闭IWDG_PR和IWDG_RLR 的写保护
	if(tm == WDT_S5)IWDG_SetReload(578); 			//设置重装载值
	else if(tm == WDT_1S)IWDG_SetReload(1158);
	else if(tm == WDT_2S)IWDG_SetReload(2315);
	else if(tm == WDT_3S)IWDG_SetReload(3473);
	else IWDG_SetReload(0xFFF);
	IWDG_SetPrescaler(IWDG_Prescaler_32); 			//设置预分频系数为32
	//IWDG_ReloadCounter();
	IWDG_Enable(); //使能看门狗
} 
/******************************************************************************
  * @brief  喂狗，清除看门狗定时器
  * @param 	None 
  * @retval None 
******************************************************************************/
void	WDT_Clear(void)
{
	IWDG_ReloadCounter();
}
/******************************************************************************
  * @brief  初始化 I2C 驱动模块
  * @param 	None 
  * @retval None 
******************************************************************************/
#define		I2C_SPEED				400000
void	Board_I2C_Init(void)
{
	I2C_InitTypeDef  I2C_InitStructure;

	Pin_I2C_Init();

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_DeInit(I2C1);
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	
	I2C_Init(I2C1, &I2C_InitStructure);
	  
	I2C_Cmd(I2C1, ENABLE);

	I2C_AcknowledgeConfig(I2C1, ENABLE); 	//允许1字节1应答模式
} 
/******************************************************************************
  * @brief	: 写多个字节到设备寄存器.	 
  * @param	: - DeviceAddr : 设备地址
  *			  - pBuffer : 要写入数据的头指针.
  *			  - WriteAddr : 设备内部的寄存器地址.
  *			  - NumByteToWrite : 写入数据的字节数.    
  * @retval : TRUE: 写入成功	FALSE: 写入失败
******************************************************************************/
Std_ReturnType Board_I2C_RegisterWrite(INT8U DeviceAddr, INT8U* pBuffer, INT8U WriteAddr, INT16U NumByteToWrite)
{
	Board_I2C_CheckFlag(I2C_FLAG_BUSY);
    
	I2C_GenerateSTART(I2C1, ENABLE);
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT); 
	I2C_Send7bitAddress(I2C1, DeviceAddr, I2C_Direction_Transmitter);
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);     
	I2C_SendData(I2C1, WriteAddr);  
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
	while(NumByteToWrite--)  
	{
		I2C_SendData(I2C1, *pBuffer); 
		pBuffer++; 
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
	}
	I2C_GenerateSTOP(I2C1, ENABLE);

	if((g_err.I2C_Busy == 1)||(g_err.I2C_Device_Err == 1))
		return	FALSE;
	return	TRUE;
}
/******************************************************************************
  * @brief	: 从设备寄存器读多个字节..	 
  * @param	: - DeviceAddr : 设备地址
  *			  - pBuffer : 读出数据缓存的头指针. 
  *			  - ReadAddr : 设备内部的寄存器地址.
  *			  - NumByteToRead : 读出数据的字节数. 
  * @retval : TRUE: 读出成功	FALSE: 读出失败
******************************************************************************/
Std_ReturnType Board_I2C_RegisterRead(INT8U DeviceAddr, INT8U* pBuffer, INT8U ReadAddr, INT16U NumByteToRead)
{  
	Board_I2C_CheckFlag(I2C_FLAG_BUSY);  
    
	I2C_GenerateSTART(I2C1, ENABLE);
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT);	//EV5
	I2C_Send7bitAddress(I2C1, DeviceAddr, I2C_Direction_Transmitter);
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);//EV6
	I2C_Cmd(I2C1, ENABLE);/* Clear EV6 by setting again the PE bit */
	I2C_SendData(I2C1, ReadAddr);  
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED);//EV8  
	I2C_GenerateSTART(I2C1, ENABLE);
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT);//EV5
	I2C_Send7bitAddress(I2C1, DeviceAddr, I2C_Direction_Receiver);
						Board_I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);//EV6
	while(NumByteToRead)  
	{
		if(NumByteToRead == 1)
		{
			I2C_AcknowledgeConfig(I2C1, DISABLE);
			I2C_GenerateSTOP(I2C1, ENABLE);
		}
		Board_I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED);
		*pBuffer = I2C_ReceiveData(I2C1);
		pBuffer++;
		NumByteToRead--;	   
	}
	I2C_AcknowledgeConfig(I2C1, ENABLE);

	return TRUE;
}
/******************************************************************************
  * @brief  检查I2C状态，增加了超时机制
  * @param 	：- I2C_FLAG：状态名称 
  * @retval None 
******************************************************************************/
void	Board_I2C_CheckFlag(INT32U I2C_FLAG)
{
	INT32U	timeout;

	timeout = 1000;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG))
	{
		if(timeout>0)timeout--;
		else
		{
			g_err.I2C_Busy = 1;
			return;
		} 	
	}
	g_err.I2C_Busy = 0;
}
/******************************************************************************
  * @brief  检查I2C事件，增加了超时机制
  * @param 	：- I2C_EVENT：事件名称 
  * @retval None 
******************************************************************************/
void	Board_I2C_CheckEvent(INT32U I2C_EVENT)
{
	INT32U	timeout;

	timeout = 1000;
	while(!I2C_CheckEvent(I2C1, I2C_EVENT))
	{
		if(timeout>0)timeout--;
		else
		{
			g_err.I2C_Device_Err = 1;
			return;
		} 	
	}
	g_err.I2C_Device_Err = 0;
}

/******************************************************************************
  * @brief  时钟信号输出功能
  * @param 	none 
  * @retval None 
******************************************************************************/
void	Board_MCO_Init(void)
{
	/* Get 4.9152MHz on PA8 pin (MCO) */
	RCC_MCOConfig(RCC_MCOSource_SYSCLK,RCC_MCODiv_4);

	Pin_MCO_Init();
}
