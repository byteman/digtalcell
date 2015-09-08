/*******************************************************************************
  * @file    Uart.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.05.29
  * @brief   ���ڹ���ģ��
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
static	BOOLEAN	Flag_Sending = FALSE;		//���ڷ������ݱ�־
static	INT8U	RS422_RxBuffer[RS422_RxBufferSize];
static	INT32U	RS422_RxBufferIn;
static	INT32U	RS422_RxBufferOut;
static	RX_TIMEOUT_PROC g_fn = NULL;
static	INT32U	RS422_RxCounter;	
/* Private functions ---------------------------------------------------------*/
static	void	RS422_RxTimerOut_Working(void);
/******************************************************************************
  * @brief  RS422��ʼ��
  * @param  BaudRate�� �����ʣ�
  			WordLength�� �ֳ���
  			Parity�� Ч��  0/1/2   No/Even/Odd
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ�� 
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
  * @brief  RS422ȥ��ʼ��
  * @param  none
  * @retval none 
******************************************************************************/
void	RS422_DeInit(void)
{
	USART_Cmd(USART1, DISABLE);
	USART_DeInit(USART1);
}
/******************************************************************************
  * @brief	�����жϴ���������ջ�����������ʼ�ձ���������յ������� 
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
	
	if(RS422_RxBufferIn == RS422_RxBufferOut)	//���ջ���������,���ƶ����ָ��ǰ��
	{
		RS422_RxBufferOut++;
		if(RS422_RxBufferOut >= RS422_RxBufferSize)
			RS422_RxBufferOut = 0;
	}			
}
/******************************************************************************
  * @brief	��ȡ���ջ��������ݵ�����  
  * @param  None
  * @retval ��δ�������������
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
  * @brief  ��ȡ���ջ�����������
  * @param  pData���������ݴ�ŵĿ�ʼ��ַ  
  			lData��Ҫ�������ֽڸ���
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ��
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
  * @brief  ���ڽ��ճ�ʱ��ʼ��
  * @param  ms����ʱʱ��  
  			fn����ʱ�ص�����
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ��
******************************************************************************/
Std_ReturnType RS422_RxTimerOut_Init(INT32U ms, RX_TIMEOUT_PROC fn)
{
	if((ms == 0)||(fn == NULL))
		return FALSE;	//���������� 
	g_fn = fn;
	return Timer_Regist(LOOP,ms,RS422_RxTimerOut_Working);
}
/******************************************************************************
  * @brief  ���ڽ��ճ�ʱ����
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
  * @brief	�����жϴ������ŷ��ͻ�����ʣ�µ����ݣ�֪����������  
  * @param  None
  * @retval	None 
******************************************************************************/
void	RS422_ItTxCallBack(void)
{
	if(RS422_TxBufferOut == RS422_TxBufferIn)
	{
		Flag_Sending = FALSE;
		Pin_RS422CON_Set(RS422CON_Receive);		//��ʱ�ͷŷ��������ϵĿ���Ȩ	
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
  * @brief	RS422�������ݵ�������������������ڷ��������У���ô�µ����ݽ�����
  			�����ݺ��棬һ�����ͳ�ȥ�����������û���㹻�Ŀռ䣬������һ��������Ϣ 
  * @param  pData ���͵�����ͷָ��
  			lData Ҫ���͵����ݳ���
  * @retval	FALSE û�пռ��ˣ�TRUE �ɹ� 
******************************************************************************/
Std_ReturnType	RS422_SendData(INT8U* pData,INT32U lData)
{
	INT32U i;
	INT32S surplus;

	if((lData <= 0)||(lData > RS422_TxBufferSize))
		return	FALSE;

	surplus = RS422_TxBufferOut - RS422_TxBufferIn;
	if(surplus <= 0)surplus += RS422_TxBufferSize;

	if(Flag_Sending == TRUE)//������ڷ��������У���ôʣ��ռ�Ҫ�� 1
		surplus -= 1;
		
	if(lData > (INT32U)surplus)
		return FALSE;		//ʣ��ռ䲻��	 
	else
	{
		for(i=0;i<lData;i++)
		{
			RS422_TxBuffer[RS422_TxBufferIn] = *(pData + i);
			RS422_TxBufferIn++;
			if(RS422_TxBufferIn >= RS422_TxBufferSize)
			RS422_TxBufferIn = 0;
		}
		if(Flag_Sending == FALSE)	//���û�д��ڷ������ݹ����У�������������
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

