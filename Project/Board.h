
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_H
#define __BOARD_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{ 
	WDT_S5 = 0x00,
	WDT_1S = 0x01, 
	WDT_2S = 0x02,
	WDT_3S = 0x03,
}TYPE_WDTtime;

typedef struct 
{
	INT32U	I2C_Busy:1;				//I2C忙
	INT32U	I2C_Device_Err:1;		//设备问题
}BOARD_ERR;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	Board_Init(void);
extern	void	SoftReset(void);
extern	void	WDT_Init(TYPE_WDTtime tm);
extern	void	WDT_Clear(void);
extern	void	Board_MCO_Init(void);
//extern	void	Board_I2C_Init(void);
extern	void	Board_ErrRead(BOARD_ERR* errbd);
extern	Std_ReturnType Board_I2C_RegisterWrite(INT8U DeviceAddr, INT8U* pBuffer, INT8U WriteAddr, INT16U NumByteToWrite);
extern	Std_ReturnType Board_I2C_RegisterRead(INT8U DeviceAddr, INT8U* pBuffer, INT8U ReadAddr, INT16U NumByteToRead);

#endif /* __BOARD_H */
