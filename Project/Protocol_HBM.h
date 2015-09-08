
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PROTOCOL_HBM_H
#define __PROTOCOL_HBM_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	Ptl_HBM_Init(void);
extern	void	Ptl_HBM_analysis(INT8U  inbyte);

#endif /* __PROTOCOL_HBM_H */
