

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMER_H
#define __TIMER_H
/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{ 
	ONE_TIME = 0x00, 
	LOOP = 0x01,
}TIMERTYPE;
typedef void (*TIME_OUT_PROC)(void);

/* Exported constants --------------------------------------------------------*/
#define MAX_TIMER_NUM	16
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType Timer_Init(void);			//1 mS
extern	Std_ReturnType Timer_Regist(TIMERTYPE typ,INT32U ms, TIME_OUT_PROC fn);
extern	Std_ReturnType Timer_Delete(TIME_OUT_PROC fn);
extern	void Timer_Server(void);					//请在主循环中调用
/*请不要在该中断中做很多事情，否则会造成中断嵌套*/
extern	Std_ReturnType Timer6_Init(INT32U us,TIME_OUT_PROC fn);	//最小为100uS，最大为6553600uS(6.5536S)
extern	Std_ReturnType Timer6_Close(void);

extern	void Delay(INT32U nClock);					//纯软件延时

#endif /* __TIMER_H */
