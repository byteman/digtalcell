
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MMA8451_H
#define __MMA8451_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
	INT32U	ChipConnect:1;			//芯片连接问题

}ANG_ERR;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	Angle_Init(void);
extern	Std_ReturnType	Angle_CalibZero(void);
extern	FP32	Angle_Read(void);
extern	void	Angle_ErrRead(ANG_ERR* errang);

#endif /* __MMA8451_H */
