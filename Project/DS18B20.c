/*******************************************************************************
  * @file    DS18B20.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.06.07
  * @brief   �¶�ģ��
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "DS18B20.h"
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static	FP32	g_currTEMP;
static	TEMP_ERR	g_err;
/* Private functions ---------------------------------------------------------*/
static	void DS18B20_Working(void);

/******************************************************************************
  * @brief	�¶Ȳɼ�ģ���ʼ��  
  * @param  None
  * @retval TRUE���ɹ�  FALSE��ʧ��
******************************************************************************/
Std_ReturnType	TEMP_Init(void)
{
	Pin_DS18B20_Init();
	Timer_Regist(LOOP,100,DS18B20_Working);
	return TRUE;
}

/******************************************************************************
  * @brief	��ȡ��ǰ�¶�  
  * @param  None
  * @retval �¶�ֵ����λΪ ���϶�
******************************************************************************/
FP32	TEMP_Read(void)
{
	return g_currTEMP;
}

/******************************************************************************
  * @brief	��ȡ�¶�ģ������־  
  * @param  errtemp�����صĽṹ��ָ��
  * @retval None
******************************************************************************/
void	TEMP_ErrRead(TEMP_ERR* errtemp)
{
	errtemp->NoPresenceBit = g_err.NoPresenceBit;
	errtemp->OverScopeBit = g_err.OverScopeBit;
	errtemp->UnderScopeBit = g_err.UnderScopeBit;
}

/******************************************************************************
  * @brief	DS18B20 ��λ  
  * @param  None
  * @retval TRUE��DS18B20����	FALSE��DS18B20 ���ڣ�����
******************************************************************************/
Std_ReturnType DS18B20_Reset(void)
{
	TYPE_PinLogic result;
//__disable_irq();	
	Pin_DS18B20_Set(PinLogic_Low);	// Drives DQ low
	Delay(2600);	//Լ530us
	Pin_DS18B20_Set(PinLogic_High);	// Releases the bus
	Delay(350);		//Լ31us
	result = Pin_DS18B20_Read(); 	// Sample for presence pulse from slave
	//Delay(2050);	//Լ31us		// Complete the reset sequence recovery
//__enable_irq();
	if(result == PinLogic_Low)
		return TRUE;
	else
		return FALSE;	
}

/******************************************************************************
  * @brief	DS18B20 ����һλ  
  * @param  bit��λ�߼�
  * @retval None
******************************************************************************/
void DS18B20_WriteBit(TYPE_PinLogic bit)
{
//__disable_irq();
	if (bit == PinLogic_High)
	{
		Pin_DS18B20_Set(PinLogic_Low);	// Drives DQ low
		Delay(20);	//Լ5.7us
		Pin_DS18B20_Set(PinLogic_High);	// Releases the bus
		Delay(300);	//Լ 62us 			// Complete the time slot and 10us recovery
	}
	else
	{
		Pin_DS18B20_Set(PinLogic_Low);	// Drives DQ low
		Delay(300);	//Լ 62us
		Pin_DS18B20_Set(PinLogic_High);	// Releases the bus
		Delay(20);	//Լ5.7us
	}
//__enable_irq();
}

/******************************************************************************
  * @brief	DS18B20 ��ȡһλ  
  * @param  None
  * @retval ����λ�߼���1 �� 0��
******************************************************************************/
TYPE_PinLogic DS18B20_ReadBit(void)
{
	TYPE_PinLogic result;
//__disable_irq();	
	Pin_DS18B20_Set(PinLogic_Low);	// Drives DQ low
	Delay(5);		//Լ2.6us
	Pin_DS18B20_Set(PinLogic_High);	// Releases the bus
	Delay(35);		//Լ 10us
	result = Pin_DS18B20_Read();	// Sample the bit value from the slave
	Delay(280);		//Լ 60us 		// Complete the time slot and 10us recovery
//__enable_irq();	
	return result;
}

/******************************************************************************
  * @brief	DS18B20 ����һ���ֽ�  
  * @param  dat�����͵��ֽ�����
  * @retval None
******************************************************************************/
void DS18B20_WriteByte(INT8U dat)
{
	INT32U loop;
	
	for (loop = 0; loop < 8; loop++)
	{
		if(dat & 0x01)
			DS18B20_WriteBit(PinLogic_High);
		else
			DS18B20_WriteBit(PinLogic_Low);

		dat >>= 1;
	}
}

/******************************************************************************
  * @brief	DS18B20 ��ȡһ���ֽ�  
  * @param  None
  * @retval ��ȡ�����ֽ�����
******************************************************************************/
INT8U DS18B20_ReadByte(void)
{
	INT32U	loop;
	INT8U	result=0;

	for (loop = 0; loop < 8; loop++)
	{
		result >>= 1;
		
		if (DS18B20_ReadBit() == PinLogic_High)
		{
			result |= 0x80;
		}
	}
	return result;
}

/******************************************************************************
  * @brief	DS18B20 ����������ʱִ�иú�������ȡ�¶�ֵ�������ȫ�ֱ��� g_currTEMP  
  * @param  None
  * @retval None
******************************************************************************/
void DS18B20_Working(void)
{
static	INT32U	step = 0;
static	INT8U	TL,TH;
	INT32S	temp = 0;

	if(step == 0)
	{
		if(DS18B20_Reset() == TRUE)
		{
			step++;
			g_err.NoPresenceBit = 0;
		}
		else
		{
			step = 0;
			g_err.NoPresenceBit = 1;
		}			
	}
	else if(step == 1)
	{
		DS18B20_WriteByte(0xCC);
		step++;
	}
	else if(step == 2)
	{
		DS18B20_WriteByte(0x44);		//ת��
		step++;
	}
	else if(step == 3)
	{
		if(DS18B20_Reset() == TRUE)
		{
			step++;
			g_err.NoPresenceBit = 0;
		}
		else
		{
			step = 0;
			g_err.NoPresenceBit = 1;
		}
	}
	else if(step == 4)
	{
		DS18B20_WriteByte(0xCC);
		step++;
	}
	else if(step == 5)
	{
		DS18B20_WriteByte(0xBE);		//��ȡ
		step++;
	}
	else if(step == 6)
	{
		TL = DS18B20_ReadByte();
		step++;
	}
	else if(step == 7)
	{
		TH = DS18B20_ReadByte();
		step = 0;
//�����¶�����Ϊ���͸�ʽ
		temp = TH;
		temp <<= 8;
		temp |= TL;
		temp <<= 16;
		temp >>= 16;
//�����¶�ת��Ϊ�����ʽ
		g_currTEMP = (FP32)temp * 0.0625;
		if(g_currTEMP > 85.0)					//������Χ
			g_err.OverScopeBit = 1;
		else
			g_err.OverScopeBit = 0;
		if(g_currTEMP < -40.0)					//������Χ
			g_err.UnderScopeBit = 1;
		else
			g_err.UnderScopeBit = 0;
	}
}
/******************************************************************************
  * @brief	DS18B20 ���ù���ģʽ  
  * @param  None
  * @retval None
******************************************************************************/
//int Config_18b20(void)
//{
//		// select the device
//		if (ibtn_reset(GPIOB,GPIO_Pin_13)) // Reset the 1-Wire bus
//		{
//			return 1; // Return if no devices found
//		}
//			
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0xCC); 
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0x4E); 	
//	
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0x00); 	
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0x00); 	
//		
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0x7F); 	// 9 bit
//	
//		// select the device
//		if (ibtn_reset(GPIOB,GPIO_Pin_13)) // Reset the 1-Wire bus
//		{
//			return 1; // Return if no devices found
//		}
//		
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0xcc);  
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0x48);  	
//		
//		// select the device
//		if (ibtn_reset(GPIOB,GPIO_Pin_13)) // Reset the 1-Wire bus
//		{
//			return 1; // Return if no devices found
//		}
//			
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0xcc);  //skip rom
//		ibtn_WriteByte(GPIOB,GPIO_Pin_13, 0xb8);  	
//
//	return 0;
//}

