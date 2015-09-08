
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INTERFACE_H
#define __INTERFACE_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define 	HBM_ACTIV
//#define 	FLINTEC_ACTIV
//#define 	MODBUS_ACTIV

/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	Inter_Init(void);
extern	void	Inter_Working(void);
extern	Std_ReturnType	Inter_Send(INT8U* pData,INT32U lData);

#endif /* __INTERFACE_H */
