/*******************************************************************************
  * @file    Board.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.06.15
  * @brief   С��ģ�飬С���һЩ���Թ���
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
  * @brief  С���ʼ��
  * @param  
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ�� 
******************************************************************************/
Std_ReturnType	Board_Init(void)
{
	Board_MCO_Init();
	Board_I2C_Init();
	return TRUE;
}
/******************************************************************************
  * @brief	��ȡ����ģ������־  
  * @param  errbd�����صĽṹ��ָ��
  * @retval None
******************************************************************************/
void	Board_ErrRead(BOARD_ERR* errbd)
{
	errbd->I2C_Busy = g_err.I2C_Busy;			//I2Cæ
	errbd->I2C_Device_Err = g_err.I2C_Device_Err;	//�豸����
}
/******************************************************************************
  * @brief  �����λ
  * @param 	None 
  * @retval None 
******************************************************************************/
void	SoftReset(void) 
{
	__set_FAULTMASK(1);     // �ر������ж�
	NVIC_SystemReset();		// ��λ	  
}
/******************************************************************************
  * @brief  ���Ź���ʼ��
  * @param 	tm�����Ź�ʱ�䣬0.5s / 1s / 2s / 3s /Ĭ���3.542�� 
  * @retval None 
******************************************************************************/
void	WDT_Init(TYPE_WDTtime tm)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	//�ر�IWDG_PR��IWDG_RLR ��д����
	if(tm == WDT_S5)IWDG_SetReload(578); 			//������װ��ֵ
	else if(tm == WDT_1S)IWDG_SetReload(1158);
	else if(tm == WDT_2S)IWDG_SetReload(2315);
	else if(tm == WDT_3S)IWDG_SetReload(3473);
	else IWDG_SetReload(0xFFF);
	IWDG_SetPrescaler(IWDG_Prescaler_32); 			//����Ԥ��Ƶϵ��Ϊ32
	//IWDG_ReloadCounter();
	IWDG_Enable(); //ʹ�ܿ��Ź�
} 
/******************************************************************************
  * @brief  ι����������Ź���ʱ��
  * @param 	None 
  * @retval None 
******************************************************************************/
void	WDT_Clear(void)
{
	IWDG_ReloadCounter();
}
/******************************************************************************
  * @brief  ��ʼ�� I2C ����ģ��
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

	I2C_AcknowledgeConfig(I2C1, ENABLE); 	//����1�ֽ�1Ӧ��ģʽ
} 
/******************************************************************************
  * @brief	: д����ֽڵ��豸�Ĵ���.	 
  * @param	: - DeviceAddr : �豸��ַ
  *			  - pBuffer : Ҫд�����ݵ�ͷָ��.
  *			  - WriteAddr : �豸�ڲ��ļĴ�����ַ.
  *			  - NumByteToWrite : д�����ݵ��ֽ���.    
  * @retval : TRUE: д��ɹ�	FALSE: д��ʧ��
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
  * @brief	: ���豸�Ĵ���������ֽ�..	 
  * @param	: - DeviceAddr : �豸��ַ
  *			  - pBuffer : �������ݻ����ͷָ��. 
  *			  - ReadAddr : �豸�ڲ��ļĴ�����ַ.
  *			  - NumByteToRead : �������ݵ��ֽ���. 
  * @retval : TRUE: �����ɹ�	FALSE: ����ʧ��
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
  * @brief  ���I2C״̬�������˳�ʱ����
  * @param 	��- I2C_FLAG��״̬���� 
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
  * @brief  ���I2C�¼��������˳�ʱ����
  * @param 	��- I2C_EVENT���¼����� 
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
  * @brief  ʱ���ź��������
  * @param 	none 
  * @retval None 
******************************************************************************/
void	Board_MCO_Init(void)
{
	/* Get 4.9152MHz on PA8 pin (MCO) */
	RCC_MCOConfig(RCC_MCOSource_SYSCLK,RCC_MCODiv_4);

	Pin_MCO_Init();
}
