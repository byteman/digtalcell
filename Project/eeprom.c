/*******************************************************************************
  * @file    eeprom.c 
  * @author  yangxd houjun
  * @version V 0.01
  * @date    2013.06.08
  * @brief   EEPROM ģ��
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"
#include "stm32l1xx_flash.h"
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/******************************************************************************
  * @brief	EEPROM ģ���ʼ��  
  * @param  None
  * @retval TRUE���ɹ�  FALSE��ʧ��
******************************************************************************/
Std_ReturnType	EE_Init(void)
{
	return TRUE;
}
/******************************************************************************
  * @brief	EEPROM ���ֽڷ�ʽ��ȡ����  
  * @param  dest�����������ݴ�ŵ�ַ
  			addr����Ҫ����EE��ʼ��ַ
			numbytes��Ҫ��ȡ���ݵ��ֽ���
  * @retval TRUE���ɹ� 
******************************************************************************/
Std_ReturnType  EE_read( INT32U addr,INT8U *dest, INT32U numbytes)
{			
	INT8U *wAddr;
	  
	wAddr=(INT8U *)addr;  
	while(numbytes--){  
		*dest++=*wAddr++;  
	    }     
	return TRUE;
}
/******************************************************************************
  * @brief	EEPROM	���ֽڵķ�ʽд������  
  * @param  source��Ҫд������ݵ�ַ
			addr��EE���ݴ洢����ʼ��ַ
			numbytes��д�����ݵ��ֽ���
  * @retval TRUE:д��ɹ�   , FALSE:д��ʧ��
******************************************************************************/
Std_ReturnType  EE_write(INT32U addr,INT32U *source, INT32U numbytes)
{
	FLASH_Status sta=FLASH_ERROR_PROGRAM;
	INT32U step=4,i,wordLen;

	wordLen=(numbytes+3)/step;				//�����ж����֣�����һ���ֵİ�һ���֡�EEPROM��Ϊÿ�����ݽṹ��Ԥ���Ŀռ�Ϊ4�ֽڵ�������
__set_FAULTMASK(1);     // �ر����ж�
	DATA_EEPROM_Unlock();
	for(i=0;i<wordLen;i++)
	{			
		sta=DATA_EEPROM_ProgramWord(addr,*source);
		if(sta!=FLASH_COMPLETE)
			break;						//continue;
		source++;
		addr=addr+step;
	}
	DATA_EEPROM_Lock();
__set_FAULTMASK(0);     // �������ж�
	if(sta!=FLASH_COMPLETE)
		return FALSE;
	else
		return TRUE;
}

