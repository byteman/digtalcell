

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CS1237_H
#define __CS1237_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{ 
	Rt10 = 0x00,
	Rt40 = 0x01, 
	Rt640 = 0x02,
	Rt1280 = 0x03,
}TYPE_CS1237rate;

typedef struct 
{
	INT32U	NoExcBit:1;			//没有激励电压
	INT32U	OpenBridgeBit:1;	//桥路开路
	INT32U	NoAdcBit:1;			// 1 秒钟没检查到转换完成
}WAD_ERR;

typedef void (*WAD_AVG_PROC)(INT32S);
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	WAD_Init(TYPE_CS1237rate rate);
extern	Std_ReturnType	WAD_CheckNew(void);
extern	INT32S	WAD_Read(void);
extern	void	WAD_ErrRead(WAD_ERR* errad);
extern	Std_ReturnType	WAD_StartAvg(INT32U num,WAD_AVG_PROC fn);

#endif /* __CS1237_H */
