/*******************************************************************************
  * @file    Interface.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.11.16
  * @brief   交互处理（通讯、IO、）
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Interface.h"
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static enum
{	WUSART = 0x00,
	WBTV40 = 0x01, 
}g_WorkInter;	
/* Private functions ---------------------------------------------------------*/


/******************************************************************************
  * @brief  通讯接口初始化
  * @param  None
  * @retval TRUE: 初始化成功	FALSE: 初始化失败 
******************************************************************************/
Std_ReturnType	Inter_Init(void)
{
Std_ReturnType	rtn;
	
	rtn = RS422_Init(g_ParaUser.baudRate,8,g_ParaUser.parityBit);
	//rtn = RS422_Init(115200,8,0);
	UART2_Init(9600,8,0);
	rtn = Ptl_HBM_Init();
	return rtn;
}
/******************************************************************************
  * @brief  交互接口处理（注意要及时处理、优先级最高）
  * @param  none
  * @retval none 
******************************************************************************/
void	Inter_Working(void)
{
	INT8U temp;
	
//	while(RS422_RxBufferAmount())
//	{
//		if(RS422_ReadRxBuffer(&temp,1)==TRUE)
//		{
//#ifdef HBM_ACTIV
//			Ptl_HBM_analysis(temp);
//#endif
//#ifdef FLINTEC_ACTIV
//			Ptl_FLINTEC_analysis(temp);
//#endif
//#ifdef MODBUS_ACTIV
//			Ptl_MODBUS_analysis(temp);
//#endif
//		}
//	}
	while(RS422_RxBufferAmount())
	{
		if(RS422_ReadRxBuffer(&temp,1)==TRUE)
			Ptl_HBM_analysis(temp);
		g_WorkInter = WUSART;
	}

	while(UART2_RxBufferAmount()) 
	{
		if(UART2_ReadRxBuffer(&temp,1)==TRUE)
			Ptl_HBM_analysis(temp);
		g_WorkInter = WBTV40;
	}
}
/******************************************************************************
  * @brief  Inter_Send
  * @param  none
  * @retval none 
******************************************************************************/
Std_ReturnType	Inter_Send(INT8U* pData,INT32U lData)
{
	if(g_WorkInter == WUSART)
		RS422_SendData(pData,lData);
	else if(g_WorkInter == WBTV40)
		UART2_SendData(pData,lData);
	return TRUE;
}

