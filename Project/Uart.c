/*******************************************************************************
  * @file    Uart.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.05.29
  * @brief   串口工作模块
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Uart.h"
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static	INT8U	RS422_TxBuffer[RS422_TxBufferSize];
static	INT32U	RS422_TxBufferIn;
static	INT32U	RS422_TxBufferOut;
static	BOOLEAN	Flag_Sending = FALSE;		//正在发送数据标志
static	INT8U	RS422_RxBuffer[RS422_RxBufferSize];
static	INT32U	RS422_RxBufferIn;
static	INT32U	RS422_RxBufferOut;
static	RX_TIMEOUT_PROC g_fn = NULL;
static	INT32U	RS422_RxCounter;	
/* Private functions ---------------------------------------------------------*/
static	void	RS422_RxTimerOut_Working(void);
/******************************************************************************
  * @brief  RS422初始化
  * @param  BaudRate： 波特率，
  			WordLength： 字长，
  			Parity： 效验  0/1/2   No/Even/Odd
  * @retval TRUE: 初始化成功	FALSE: 初始化失败 
******************************************************************************/
Std_ReturnType	RS422_Init(INT32U BaudRate,INT8U WordLength,INT8U Parity)
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
RS422_DeInit();

	Pin_RS422_Init();
	/* Enable USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* USART configuration */
	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART */
	USART_Cmd(USART1, ENABLE);

	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_TC, ENABLE);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  	
	return TRUE;
}
/******************************************************************************
  * @brief  RS422去初始化
  * @param  none
  * @retval none 
******************************************************************************/
void	RS422_DeInit(void)
{
	USART_Cmd(USART1, DISABLE);
	USART_DeInit(USART1);
}
/******************************************************************************
  * @brief	接收中断处理，如果接收缓冲区已满，始终保持最近接收到的数据 
  * @param  None
  * @retval None
******************************************************************************/
void	RS422_ItRxCallBack(INT8U data)
{
	RS422_RxBuffer[RS422_RxBufferIn] = data;
	RS422_RxBufferIn++;
	RS422_RxCounter++;
	if(RS422_RxBufferIn >= RS422_RxBufferSize)
		RS422_RxBufferIn = 0;
	
	if(RS422_RxBufferIn == RS422_RxBufferOut)	//接收缓冲区已满,将推动输出指针前移
	{
		RS422_RxBufferOut++;
		if(RS422_RxBufferOut >= RS422_RxBufferSize)
			RS422_RxBufferOut = 0;
	}			
}
/******************************************************************************
  * @brief	读取接收缓冲区数据的数量  
  * @param  None
  * @retval 还未处理的数据数量
******************************************************************************/
INT32U	RS422_RxBufferAmount(void)
{
	INT32S amount;

	amount = RS422_RxBufferIn - RS422_RxBufferOut;
	if(amount < 0)
		amount += RS422_RxBufferSize;

	return (INT32U)amount; 	
}
/******************************************************************************
  * @brief  读取接收缓冲区的数据
  * @param  pData：读出数据存放的开始地址  
  			lData：要读出的字节个数
  * @retval TRUE: 初始化成功	FALSE: 初始化失败
******************************************************************************/
Std_ReturnType	RS422_ReadRxBuffer(INT8U* pData,INT32U lData)
{
	INT32S amount;
	INT32U i;

	amount = RS422_RxBufferIn - RS422_RxBufferOut;
	if(amount < 0)
		amount += RS422_RxBufferSize;
	if(lData > (INT32U)amount)
		return FALSE;
	else
	{
		for(i=0;i<lData;i++)
		{
			*(pData + i) = RS422_RxBuffer[RS422_RxBufferOut];
			RS422_RxBufferOut++;
			if(RS422_RxBufferOut >= RS422_RxBufferSize)
				RS422_RxBufferOut = 0;
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
Std_ReturnType RS422_RxTimerOut_Init(INT32U ms, RX_TIMEOUT_PROC fn)
{
	if((ms == 0)||(fn == NULL))
		return FALSE;	//参数有问题 
	g_fn = fn;
	return Timer_Regist(LOOP,ms,RS422_RxTimerOut_Working);
}
/******************************************************************************
  * @brief  串口接收超时工作
  * @param  None
  * @retval None
******************************************************************************/
void	RS422_RxTimerOut_Working(void)
{
	if(RS422_RxCounter > 0)
	{
		RS422_RxCounter = 0;
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
void	RS422_ItTxCallBack(void)
{
	if(RS422_TxBufferOut == RS422_TxBufferIn)
	{
		Flag_Sending = FALSE;
		Pin_RS422CON_Set(RS422CON_Receive);		//及时释放发送总线上的控制权	
	}
	else
	{
		USART_SendData(USART1,RS422_TxBuffer[RS422_TxBufferOut]);
		RS422_TxBufferOut++;
		if(RS422_TxBufferOut >= RS422_TxBufferSize)
			RS422_TxBufferOut = 0;		
	}	
}
/******************************************************************************
  * @brief	RS422发送数据到缓冲区，如果串口正在发送数据中，那么新的数据将排在
  			老数据后面，一并发送出去；如果缓冲区没有足够的空间，将返回一个错误信息 
  * @param  pData 发送的数据头指针
  			lData 要发送的数据长度
  * @retval	FALSE 没有空间了，TRUE 成功 
******************************************************************************/
Std_ReturnType	RS422_SendData(INT8U* pData,INT32U lData)
{
	INT32U i;
	INT32S surplus;

	if((lData <= 0)||(lData > RS422_TxBufferSize))
		return	FALSE;

	surplus = RS422_TxBufferOut - RS422_TxBufferIn;
	if(surplus <= 0)surplus += RS422_TxBufferSize;

	if(Flag_Sending == TRUE)//如果正在发送数据中，那么剩余空间要减 1
		surplus -= 1;
		
	if(lData > (INT32U)surplus)
		return FALSE;		//剩余空间不足	 
	else
	{
		for(i=0;i<lData;i++)
		{
			RS422_TxBuffer[RS422_TxBufferIn] = *(pData + i);
			RS422_TxBufferIn++;
			if(RS422_TxBufferIn >= RS422_TxBufferSize)
			RS422_TxBufferIn = 0;
		}
		if(Flag_Sending == FALSE)	//如果没有处于发送数据工作中，启动发送数据
		{
			Flag_Sending = TRUE;
			Pin_RS422CON_Set(RS422CON_Send);

			USART_SendData(USART1,RS422_TxBuffer[RS422_TxBufferOut]);
			RS422_TxBufferOut++;
			if(RS422_TxBufferOut >= RS422_TxBufferSize)
				RS422_TxBufferOut = 0;
		}
	}
		return TRUE;	
}

