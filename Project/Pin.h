

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PIN_H
#define __PIN_H

/* Includes ------------------------------------------------------------------*/
#include "Std_Types.h"
#include "stm32l1xx.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
typedef enum
{ 
	PinLogic_Low = 0x00, 
	PinLogic_High = 0x01,
}TYPE_PinLogic;

typedef enum
{ 
	RS422CON_Receive = 0x00, 
	RS422CON_Send = 0x01,
}TYPE_RS422CON;

typedef enum
{ 
	WorkLED_On = 0x00, 
	WorkLED_Off = 0x01,
}WorkLED_Cntrl;

typedef enum
{ 
	CS = 0x00, 
	SCLK = 0x01,
	DIN = 0x02,
	SYNC = 0x03,
	DOUT = 0x04,
}AD7195_PIN;

typedef enum
{ 
	ADSCLK = 0x00, 
	ADDOUT = 0x01,
}CS1237_PIN;

typedef enum
{ 
	CSB = 0x00, 
	SCK = 0x01,
	MOSI = 0x02,
	MISO = 0x03,
	XRESET = 0x04,	
}SCA3060_PIN;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	Pin_Init(void);

extern	void	Pin_RS422_Init(void);
extern	void	Pin_RS422CON_Set(TYPE_RS422CON cnt);

extern	void	Pin_UART2_Init(void);

extern	void	Pin_AD7195_Init(void);
extern	void	Pin_AD7195_Set(AD7195_PIN pin,TYPE_PinLogic cnt);
extern	TYPE_PinLogic	Pin_AD7195_ReadDOUT(void);

extern	void	Pin_CS1237_Init(void);
extern	void	Pin_CS1237_Set(CS1237_PIN pin,TYPE_PinLogic cnt);
extern	TYPE_PinLogic	Pin_CS1237_ReadDOUT(void);

extern	void	Pin_DS18B20_Init(void);
extern	void	Pin_DS18B20_Set(TYPE_PinLogic cnt);
extern	TYPE_PinLogic	Pin_DS18B20_Read(void);

extern	void	Pin_VMADC_Init(void);
extern	void	Pin_I2C_Init(void);
extern	void	Pin_MCO_Init(void);

extern	void	WorkLED_Set(WorkLED_Cntrl cnt);
extern	void	WorkLED_Flash(void);

#endif /* __PIN_H */

