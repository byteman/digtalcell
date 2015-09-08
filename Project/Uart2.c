/*******************************************************************************
  * @file    Uart2.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2015.08.23
  * @brief   ���ڹ���ģ��
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
static	BOOLEAN	UART2_Flag_Sending = FALSE;		//���ڷ������ݱ�־

static	INT8U	UART2_RxBuffer[UART2_RxBufferSize];
static	INT32U	UART2_RxBufferIn;
static	INT32U	UART2_RxBufferOut;
static	RX_TIMEOUT_PROC g_fn = NULL;
static	INT32U	UART2_RxCounter;	
/* Private functions ---------------------------------------------------------*/
static	void	UART2_RxTimerOut_Working(void);
/******************************************************************************
  * @brief  UART2��ʼ��
  * @param  BaudRate�� �����ʣ�
  			WordLength�� �ֳ���
  			Parity�� Ч��  0/1/2   No/Even/Odd
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ�� 
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
  * @brief  UART2ȥ��ʼ��
  * @param  none
  * @retval none 
******************************************************************************/
void	UART2_DeInit(void)
{
	USART_Cmd(USART2, DISABLE);
	USART_DeInit(USART2);
}
/******************************************************************************
  * @brief	�����жϴ���������ջ�����������ʼ�ձ���������յ������� 
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
	
	if(UART2_RxBufferIn == UART2_RxBufferOut)	//���ջ���������,���ƶ����ָ��ǰ��
	{
		UART2_RxBufferOut++;
		if(UART2_RxBufferOut >= UART2_RxBufferSize)
			UART2_RxBufferOut = 0;
	}			
}
/******************************************************************************
  * @brief	��ȡ���ջ��������ݵ�����  
  * @param  None
  * @retval ��δ�������������
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
  * @brief  ��ȡ���ջ�����������
  * @param  pData���������ݴ�ŵĿ�ʼ��ַ  
  			lData��Ҫ�������ֽڸ���
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ��
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
  * @brief  ���ڽ��ճ�ʱ��ʼ��
  * @param  ms����ʱʱ��  
  			fn����ʱ�ص�����
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ��
******************************************************************************/
Std_ReturnType UART2_RxTimerOut_Init(INT32U ms, RX_TIMEOUT_PROC fn)
{
	if((ms == 0)||(fn == NULL))
		return FALSE;	//���������� 
	g_fn = fn;
	return Timer_Regist(LOOP,ms,UART2_RxTimerOut_Working);
}
/******************************************************************************
  * @brief  ���ڽ��ճ�ʱ����
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
  * @brief	�����жϴ������ŷ��ͻ�����ʣ�µ����ݣ�֪����������  
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
  * @brief	UART2_�������ݵ�������������������ڷ��������У���ô�µ����ݽ�����
  			�����ݺ��棬һ�����ͳ�ȥ�����������û���㹻�Ŀռ䣬������һ��������Ϣ 
  * @param  pData ���͵�����ͷָ��
  			lData Ҫ���͵����ݳ���
  * @retval	FALSE û�пռ��ˣ�TRUE �ɹ� 
******************************************************************************/
Std_ReturnType	UART2_SendData(INT8U* pData,INT32U lData)
{
	INT32U i;
	INT32S surplus;

	if((lData <= 0)||(lData > UART2_TxBufferSize))
		return	FALSE;

	surplus = UART2_TxBufferOut - UART2_TxBufferIn;
	if(surplus <= 0)surplus += UART2_TxBufferSize;

	if(UART2_Flag_Sending == TRUE)//������ڷ��������У���ôʣ��ռ�Ҫ�� 1
		surplus -= 1;
		
	if(lData > (INT32U)surplus)
		return FALSE;		//ʣ��ռ䲻��	 
	else
	{
		for(i=0;i<lData;i++)
		{
			UART2_TxBuffer[UART2_TxBufferIn] = *(pData + i);
			UART2_TxBufferIn++;
			if(UART2_TxBufferIn >= UART2_TxBufferSize)
			UART2_TxBufferIn = 0;
		}
		if(UART2_Flag_Sending == FALSE)	//���û�д��ڷ������ݹ����У�������������
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

