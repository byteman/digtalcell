/*******************************************************************************
  * @file    main.c 
  * @author  Aspire Electric Technology ( Chang Zhou )Co.,Ltd.
  * @version V 0.01
  * @date    2013.05.26
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void test(void);
void test1(void);
/******************************************************************************
  * @brief  Main program.
  * @param  None
  * @retval 
******************************************************************************/
int main(void)
{
static __IO INT32U s_loopcnt;

	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, (INT32U)0x08004000);
	Delay(100);
	Init_PowerOn();
	Delay(100);
	/* test */
	//Timer_Regist(LOOP,100,test1);
	//Timer6_Init(500000,test1);
	//WDT_Init(WDT_2S);		//开启看门狗  2秒

	/* Infinite loop */	
	while(1)
	{
		Inter_Working();

		Timer_Server();

		if(s_loopcnt)s_loopcnt--;
		else
		{
			s_loopcnt = 50000;			//该值请经测试后确定,一般为 1 秒
			WDT_Clear();			
			WorkLED_Flash();
		}
	}
}
/******************************************************************************
  * @brief  定时处理测试
  * @param  None
  * @retval None
******************************************************************************/
void test(void)
{

}
void test1(void)
{
//char*	c = "BLEtest";
	INT8U  a;
//
//	//printf("test3().\r\n");
//	UART2_SendData(c,7);	
	while (UART2_ReadRxBuffer(&a,1))
	{
		RS422_SendData(&a,1);
	}
	while (RS422_ReadRxBuffer(&a,1))
	{
		UART2_SendData(&a,1);
	}
}
/******************************************************************************
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
******************************************************************************/
int fputc(int ch, FILE *f)
{
	INT8U c;
  	c = (INT8U)ch;
	while (!RS422_SendData(&c,1)){}
	return ch;
}

#ifdef  USE_FULL_ASSERT
/******************************************************************************
  * @brief  Reports the name of the source file and the source line number
  *         where the assert error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert error line source number
  * @retval None
******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
	while(1);
}
#endif
/************************ (C) COPYRIGHT Aspire ****************END OF FILE****/
