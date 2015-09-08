
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FILTER_H
#define __FILTER_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
//typedef struct 
//{
//	INT32S	data_MAX;			//
//	INT32S	data_MIN;			//
//	INT32S	data_BF;			// 
//	INT32S	data_CRU;			// 
//	//INT32S	dif_OUT;
//}FLT_PPAVG;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

extern	INT32S	Flt_PPVAvg(INT32S newdat);
extern	INT32S	Flt_MovAvg(INT32S newdat);
//extern	INT32S	Flt_PPAvg(FLT_PPAVG* para,INT32S outdat,INT32S newdat);

//extern	float DigFil(float invar, float initval, int setic);


#endif /* __FILTER_H */
