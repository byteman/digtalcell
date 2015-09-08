
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MYLIB_H
#define __MYLIB_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	char *Lib_LonToChr(long int i);
extern	long Lib_ChrToLon(char *chr);
extern	char *Lib_FloatToChr(float f);
extern	float Lib_ChrToFloat(char *chr);

#endif /* __MYLIB_H */
