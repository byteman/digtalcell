/*******************************************************************************
  * @file    eeprom.c 
  * @author  yangxd houjun
  * @version V 0.01
  * @date    2013.06.08
  * @brief   EEPROM 模块
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
  * @brief	EEPROM 模块初始化  
  * @param  None
  * @retval TRUE：成功  FALSE：失败
******************************************************************************/
Std_ReturnType	EE_Init(void)
{
	return TRUE;
}
/******************************************************************************
  * @brief	EEPROM 按字节方式读取数据  
  * @param  dest：读出的数据存放地址
  			addr：需要读的EE起始地址
			numbytes：要读取数据的字节数
  * @retval TRUE：成功 
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
  * @brief	EEPROM	按字节的方式写入数据  
  * @param  source：要写入的数据地址
			addr：EE数据存储的起始地址
			numbytes：写入数据的字节数
  * @retval TRUE:写入成功   , FALSE:写入失败
******************************************************************************/
Std_ReturnType  EE_write(INT32U addr,INT32U *source, INT32U numbytes)
{
	FLASH_Status sta=FLASH_ERROR_PROGRAM;
	INT32U step=4,i,wordLen;

	wordLen=(numbytes+3)/step;				//计算有多少字，不足一个字的按一个字。EEPROM中为每个数据结构体预留的空间为4字节的整数倍
__set_FAULTMASK(1);     // 关闭总中端
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
__set_FAULTMASK(0);     // 开启总中端
	if(sta!=FLASH_COMPLETE)
		return FALSE;
	else
		return TRUE;
}

