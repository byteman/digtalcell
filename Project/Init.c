/*******************************************************************************
  * @file    Init.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.07.07
  * @brief   ³õÊ¼»¯Ä£¿é
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Init.h"
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static	INIT_ERR	g_err;
/* Private functions ---------------------------------------------------------*/

/******************************************************************************
  * @brief	
  * @param  None
  * @retval None
******************************************************************************/
void	Init_PowerOn(void)
{
Std_ReturnType rtn;

	rtn = Text_Init();
							if(rtn == FALSE)g_err.ModuleText = 1;
							else			g_err.ModuleText = 0;	
	rtn = Pin_Init();
							if(rtn == FALSE)g_err.ModulePin = 1;
							else			g_err.ModulePin = 0;
	rtn = Board_Init();
							if(rtn == FALSE)g_err.ModuleBoard = 1;
							else			g_err.ModuleBoard = 0;
	rtn = Para_Init();
							if(rtn == FALSE)g_err.ModulePara = 1;
							else			g_err.ModulePara = 0;
	rtn = Timer_Init();
							if(rtn == FALSE)g_err.ModuleTimer = 1;
							else			g_err.ModuleTimer = 0;
	rtn = Inter_Init();
							if(rtn == FALSE)g_err.ModulePtcl = 1;
							else			g_err.ModulePtcl = 0;
//	rtn = TEMP_Init();
							if(rtn == FALSE)g_err.ModuleTEMP = 1;
							else			g_err.ModuleTEMP = 0;
//	rtn = Angle_Init();
							if(rtn == FALSE)g_err.ModuleAngle = 1;
							else			g_err.ModuleAngle = 0;
//	rtn = VM_Init();
							if(rtn == FALSE)g_err.ModuleVM = 1;
							else			g_err.ModuleVM = 0;
	rtn = WAD_Init(Rt40);
							if(rtn == FALSE)g_err.ModuleAD = 1;
							else			g_err.ModuleAD = 0;
	rtn = Wet_Init();
							if(rtn == FALSE)g_err.ModuleWet = 1;
							else			g_err.ModuleWet = 0;
}
/******************************************************************************
  * @brief	
  * @param  None
  * @retval None
******************************************************************************/
void	Init_ErrRead(INIT_ERR* errinit)
{
	errinit->ModuleText = g_err.ModuleText;
	errinit->ModulePin = g_err.ModulePin;
	errinit->ModuleBoard = g_err.ModuleBoard;
	errinit->ModulePara = g_err.ModulePara;
	errinit->ModuleTimer = g_err.ModuleTimer;
	errinit->ModulePtcl = g_err.ModulePtcl;
	errinit->ModuleTEMP = g_err.ModuleTEMP;
	errinit->ModuleAngle = g_err.ModuleAngle;
	errinit->ModuleVM = g_err.ModuleVM;
	errinit->ModuleWet = g_err.ModuleWet;
}
/******************************************************************************
  * @brief	
  * @param  None
  * @retval None
******************************************************************************/
void	Init_ParaUpdata(void)
{
}


