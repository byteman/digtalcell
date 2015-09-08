/*******************************************************************************
  * @file    Uart2.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2015.08.23
  * @brief   串口工作模块
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Uart2.h"
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static	INT8U	UART2_TxBuffer[UART2_TxBufferSize];
static	INT32U	UART2_TxBufferIn;
static	INT32U	UART2_TxBufferOut;
static	BOOLEAN	UART2_Flag_Sending = FALSE;		//正在发送数据标志

static	INT8U	UART2_RxBuffer[UART2_RxBufferSize];
static	INT32U	UART2_RxBufferIn;
static	INT32U	UART2_RxBufferOut;
static	RX_TIMEOUT_PROC g_fn = NULL;
static	INT32U	UART2_RxCounter;	
/* Private functions ---------------------------------------------------------*/
static	void	UART2_RxTimerOut_Working(void);
/******************************************************************************
  * @brief  UART2初始化
  * @param  BaudRate： 波特率，
  			WordLength： 字长，
  			Parity： 效验  0/1/2   No/Even/Odd
  * @retval TRUE: 初始化成功	FALSE: 初始化失败 
******************************************************************************/
Std_ReturnType	UART2_Init(INT32U BaudRate,INT8U WordLength,INT8U Parity)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	if((BaudRate % 1200 != 0)||(BaudRate == 0)||(BaudRate > 115200))
		return FALSE;
	USART_InitStructure.USART_BaudRate = BaudRate;

	switch (Parity)
	{
		case 0:
			USART_InitStructure.USART_Parity = USART_Parity_No;
			break;
		case 1:
			USART_InitStructure.USART_Parity = USART_Parity_Even;
			break;
		case 2:
			USART_InitStructure.USART_Parity = USART_Parity_Odd;
			break;
		default:
			return FALSE;
	}
	if(WordLength == 7)
	{
		if(Parity > 0)
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		else
			return FALSE;	
	}
	else if(WordLength == 8)
	{
		if(Parity > 0)
			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		else
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;	
	}
	else
		return FALSE;		
////////////////////////////////////////////////////////////////////////////////////////
	UART2_DeInit();

	Pin_UART2_Init();
	/* Enable USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	/* USART configuration */
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
	USART_Init(USART2, &USART_InitStructure);
	/* Enable USART */
	USART_Cmd(USART2, ENABLE);
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART2, USART_IT_TC, ENABLE);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  	
	return TRUE;
}
/******************************************************************************
  * @brief  UART2去初始化
  * @param  none
  * @retval none 
******************************************************************************/
void	UART2_DeInit(void)
{
	USART_Cmd(USART2, DISABLE);
	USART_DeInit(USART2);
}
/******************************************************************************
  * @brief	接收中断处理，如果接收缓冲区已满，始终保持最近接收到的数据 
  * @param  None
  * @retval None
******************************************************************************/
void	UART2_ItRxCallBack(INT8U data)
{
	UART2_RxBuffer[UART2_RxBufferIn] = data;
	UART2_RxBufferIn++;
	UART2_RxCounter++;
	if(UART2_RxBufferIn >= UART2_RxBufferSize)
		UART2_RxBufferIn = 0;
	
	if(UART2_RxBufferIn == UART2_RxBufferOut)	//接收缓冲区已满,将推动输出指针前移
	{
		UART2_RxBufferOut++;
		if(UART2_RxBufferOut >= UART2_RxBufferSize)
			UART2_RxBufferOut = 0;
	}			
}
/******************************************************************************
  * @brief	读取接收缓冲区数据的数量  
  * @param  None
  * @retval 还未处理的数据数量
******************************************************************************/
INT32U	UART2_RxBufferAmount(void)
{
	INT32S amount;

	amount = UART2_RxBufferIn - UART2_RxBufferOut;
	if(amount < 0)
		amount += UART2_RxBufferSize;

	return (INT32U)amount; 	
}
/******************************************************************************
  * @brief  读取接收缓冲区的数据
  * @param  pData：读出数据存放的开始地址  
  			lData：要读出的字节个数
  * @retval TRUE: 初始化成功	FALSE: 初始化失败
******************************************************************************/
Std_ReturnType	UART2_ReadRxBuffer(INT8U* pData,INT32U lData)
{
	INT32S amount;
	INT32U i;

	amount = UART2_RxBufferIn - UART2_RxBufferOut;
	if(amount < 0)
		amount += UART2_RxBufferSize;
	if(lData > (INT32U)amount)
		return FALSE;
	else
	{
		for(i=0;i<lData;i++)
		{
			*(pData + i) = UART2_RxBuffer[UART2_RxBufferOut];
			UART2_RxBufferOut++;
			if(UART2_RxBufferOut >= UART2_RxBufferSize)
				UART2_RxBufferOut = 0;
		}
	}
		return TRUE;
}
/******************************************************************************
  * @brief  串口接收超时初始化
  * @param  ms：超时时间  
  			fn：超时回调函数
  * @retval TRUE: 初始化成功	FALSE: 初始化失败
******************************************************************************/
Std_ReturnType UART2_RxTimerOut_Init(INT32U ms, RX_TIMEOUT_PROC fn)
{
	if((ms == 0)||(fn == NULL))
		return FALSE;	//参数有问题 
	g_fn = fn;
	return Timer_Regist(LOOP,ms,UART2_RxTimerOut_Working);
}
/******************************************************************************
  * @brief  串口接收超时工作
  * @param  None
  * @retval None
******************************************************************************/
void	UART2_RxTimerOut_Working(void)
{
	if(UART2_RxCounter > 0)
	{
		UART2_RxCounter = 0;
		return;
	}
	else
	{
		if(g_fn != NULL)g_fn();
	}
}
/******************************************************************************
  * @brief	发送中断处理，接着发送缓冲区剩下的数据，知道缓冲区空  
  * @param  None
  * @retval	None 
******************************************************************************/
void	UART2_ItTxCallBack(void)
{
	if(UART2_TxBufferOut == UART2_TxBufferIn)
	{
		UART2_Flag_Sending = FALSE;	
	}
	else
	{
		USART_SendData(USART2,UART2_TxBuffer[UART2_TxBufferOut]);
		UART2_TxBufferOut++;
		if(UART2_TxBufferOut >= UART2_TxBufferSize)
			UART2_TxBufferOut = 0;		
	}	
}
/******************************************************************************
  * @brief	UART2_发送数据到缓冲区，如果串口正在发送数据中，那么新的数据将排在
  			老数据后面，一并发送出去；如果缓冲区没有足够的空间，将返回一个错误信息 
  * @param  pData 发送的数据头指针
  			lData 要发送的数据长度
  * @retval	FALSE 没有空间了，TRUE 成功 
******************************************************************************/
Std_ReturnType	UART2_SendData(INT8U* pData,INT32U lData)
{
	INT32U i;
	INT32S surplus;

	if((lData <= 0)||(lData > UART2_TxBufferSize))
		return	FALSE;

	surplus = UART2_TxBufferOut - UART2_TxBufferIn;
	if(surplus <= 0)surplus += UART2_TxBufferSize;

	if(UART2_Flag_Sending == TRUE)//如果正在发送数据中，那么剩余空间要减 1
		surplus -= 1;
		
	if(lData > (INT32U)surplus)
		return FALSE;		//剩余空间不足	 
	else
	{
		for(i=0;i<lData;i++)
		{
			UART2_TxBuffer[UART2_TxBufferIn] = *(pData + i);
			UART2_TxBufferIn++;
			if(UART2_TxBufferIn >= UART2_TxBufferSize)
			UART2_TxBufferIn = 0;
		}
		if(UART2_Flag_Sending == FALSE)	//如果没有处于发送数据工作中，启动发送数据
		{
			UART2_Flag_Sending = TRUE;

			USART_SendData(USART2,UART2_TxBuffer[UART2_TxBufferOut]);
			UART2_TxBufferOut++;
			if(UART2_TxBufferOut >= UART2_TxBufferSize)
				UART2_TxBufferOut = 0;
		}
	}
		return TRUE;	
}

