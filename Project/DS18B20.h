

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DS18B20_H
#define __DS18B20_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
	INT32U	NoPresenceBit:1;		//连接不上温度传感器标志位
	INT32U	OverScopeBit:1;			//温度高于正常范围  > 85度
	INT32U	UnderScopeBit:1;		//温度低于正常范围  > -40度
}TEMP_ERR;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	TEMP_Init(void);
extern	FP32	TEMP_Read(void);
extern	void	TEMP_ErrRead(TEMP_ERR* errtemp);

#endif /* __DS18B20_H */

