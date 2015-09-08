

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

/* Includes ------------------------------------------------------------------*/
#include "Std_Types.h"
#include "stm32l1xx.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	EE_Init(void);
extern	Std_ReturnType  EE_read( INT32U addr,INT8U *dest, INT32U numbytes);
extern	Std_ReturnType  EE_write(INT32U addr,INT32U *source, INT32U numbytes);

#endif /* __EEPROM_H */
