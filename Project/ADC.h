

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VMADC_H
#define __VMADC_H
/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
	INT32U	TimeouteBit:1;			//AD转换超时标志位
	INT32U	OutrangeBit:1;			//AD转换超量程
	INT32U	ErrTimerBit:1;			//电压测量定时器错误
	INT32U	VolLowBit:1;			//电压低

}VM_ERR;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

#define		THRESHOLD_LOW    	6.0 	//低电压判断阀值
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType VM_Init(void);	
extern	FP32	VM_Read(void);
extern	void	VM_ErrRead(VM_ERR* errtemp);

#endif /* __VMADC_H */
