

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART2_H
#define __UART2_H
/* Includes ------------------------------------------------------------------*/
#include "Std_Types.h"
#include "stm32l1xx.h"
#include <stdio.h>
#include "Pin.h"
#include "Timer.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*RX_TIMEOUT_PROC)(void);
/* Exported constants --------------------------------------------------------*/
#define		UART2_TxBufferSize 		100
#define		UART2_RxBufferSize 		100
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	UART2_Init(INT32U BaudRate,INT8U WordLength,INT8U Parity);
extern			void	UART2_DeInit(void);
extern	Std_ReturnType	UART2_SendData(INT8U* pData,INT32U lData);	//数据长度不能大于发送缓冲剩余空间

extern	Std_ReturnType	UART2_RxTimerOut_Init(INT32U ms, RX_TIMEOUT_PROC fn);
extern	Std_ReturnType	UART2_ReadRxBuffer(INT8U* pData,INT32U lData);
extern			INT32U	UART2_RxBufferAmount(void);

#endif /* __UART2_H */
