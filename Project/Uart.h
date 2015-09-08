

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_H
#define __UART_H
/* Includes ------------------------------------------------------------------*/
#include "Std_Types.h"
#include "stm32l1xx.h"
#include <stdio.h>
#include "Pin.h"
#include "Timer.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*RX_TIMEOUT_PROC)(void);
/* Exported constants --------------------------------------------------------*/
#define		RS422_TxBufferSize 		100
#define		RS422_RxBufferSize 		100
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
//extern	Std_ReturnType	RS422_Init(TYPE_RS422BaudRate BaudRate,TYPE_RS422WordLength WordLength,TYPE_RS422Parity Parity);
extern	Std_ReturnType	RS422_Init(INT32U BaudRate,INT8U WordLength,INT8U Parity);
extern	void	RS422_DeInit(void);
extern	Std_ReturnType	RS422_SendData(INT8U* pData,INT32U lData);	//数据长度不能大于发送缓冲剩余空间
extern			INT32U	RS422_RxBufferAmount(void);
extern	Std_ReturnType	RS422_RxTimerOut_Init(INT32U ms, RX_TIMEOUT_PROC fn);
extern	Std_ReturnType	RS422_ReadRxBuffer(INT8U* pData,INT32U lData);

#endif /* __UART_H */
