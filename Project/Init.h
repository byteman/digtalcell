

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INIT_H
#define __INIT_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
	INT32U	ModuleText:1;			//
	INT32U	ModulePin:1;			//
	INT32U	ModuleBoard:1;			//
	INT32U	ModulePara:1;			//
	INT32U	ModuleTimer:1;			//
	INT32U	ModulePtcl:1;			//
	INT32U	ModuleTEMP:1;			//
	INT32U	ModuleAngle:1;			//
	INT32U	ModuleVM:1;				//
	INT32U	ModuleAD:1;				//
	INT32U	ModuleWet:1;			//
}INIT_ERR;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
extern	void	Init_PowerOn(void);
extern	void	Init_ParaUpdata(void);
extern	void	Init_ErrRead(INIT_ERR* errinit);

#endif /* __INIT_H */
