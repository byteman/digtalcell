/*******************************************************************************
  * @file    MMA8451.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.08.28
  * @brief   3 轴加速度模块，用于倾角检测
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "MMA8451.h"
#include "MMA845xReg.h"
#include "math.h"
/* Private macro -------------------------------------------------------------*/
//#define		MMA8451		0x38
#define		MMA8451		0x3A
/* Private variables ---------------------------------------------------------*/
static	FP32	g_Angle;
static	ANG_ERR	g_err;
static	INT32S	g_Ax,g_Ay,g_Az;

/* Private functions ---------------------------------------------------------*/
static	void	Angle_Working(void);
static	void	MMA8451_Init(void);
static	void	Angle_AvgFilter(INT32S* x,INT32S* y,INT32S* z);
/******************************************************************************
  * @brief	倾角模块初始化  
  * @param  None
  * @retval TRUE：成功  FALSE：失败
******************************************************************************/
Std_ReturnType	Angle_Init(void)
{
	Std_ReturnType rtn;

	MMA8451_Init();
	rtn = Timer_Regist(LOOP,500,Angle_Working);
	return rtn;
}
/******************************************************************************
  * @brief	倾角模块标定零点  
  * @param  None
  * @retval TRUE：成功  FALSE：失败
******************************************************************************/
Std_ReturnType	Angle_CalibZero(void)
{
	g_ParaOther.ang_clb_xzero = g_Ax;
	g_ParaOther.ang_clb_yzero = g_Ay;
	g_ParaOther.ang_clb_zzero = g_Az;

	return TRUE;
}
/******************************************************************************
  * @brief	倾角读取 
  * @param  rdAngle：倾角读出数据存放地址
  * @retval TRUE：成功  FALSE：失败
******************************************************************************/
FP32	Angle_Read(void)
{
	return g_Angle;
}
/******************************************************************************
  * @brief	读取倾角模块错误标志  
  * @param  errang：返回的结构体指针
  * @retval None
******************************************************************************/
void	Angle_ErrRead(ANG_ERR* errang)
{
	errang->ChipConnect = g_err.ChipConnect;			//芯片连接问题
}
/******************************************************************************
  * @brief  称重服务，请周期调用该函数更新倾角值
  * @param  None
  * @retval None 
******************************************************************************/
void	Angle_Working(void)
{
	INT8U	dat[6];
	BOARD_ERR	err;
	INT32S	Ax,Ay;		//Az;

	Board_I2C_RegisterRead(MMA8451,dat,0x01,6);
	Board_ErrRead(&err);
	if(err.I2C_Device_Err == 1)g_err.ChipConnect = 1;
	else g_err.ChipConnect = 0;
	 
	g_Ax = dat[0];
	g_Ax <<= 8;
	g_Ax += dat[1];
	g_Ax <<= 16;
	g_Ax >>= 18;

	g_Ay = dat[2];
	g_Ay <<= 8;
	g_Ay += dat[3];
	g_Ay <<= 16;
	g_Ay >>= 18;

	g_Az = dat[4];
	g_Az <<= 8;
	g_Az += dat[5];
	g_Az <<= 16;
	g_Az >>= 18;

	Angle_AvgFilter(&g_Ax,&g_Ay,&g_Az);

	Ax = g_Ax - g_ParaOther.ang_clb_xzero;
	Ay = g_Ay - g_ParaOther.ang_clb_yzero;
//	Az = g_Az - g_ParaOther.ang_clb_zzero;

	//g_Angle=(float)(acos((FP32)Az/4096.0))*180/3.14;
	//g_Angle=(float)(sqrt((FP32)(Ax*Ax+Ay*Ay)));
	g_Angle=(float)(asin((sqrt((FP32)(Ax*Ax+Ay*Ay)))/4096.0))*180/3.14;
}
/******************************************************************************
  * @brief	平均值滤波  
  * @param  ang：结构体指针
  * @retval None
******************************************************************************/
#define		NUM_FILTER		6
void	Angle_AvgFilter(INT32S* x,INT32S* y,INT32S* z)
{
static	INT32S	bufx[NUM_FILTER],bufy[NUM_FILTER],bufz[NUM_FILTER];
static	INT32U	p = 0;
		INT32U	i;
		INT32S	sum1=0,sum2=0,sum3=0;
static	INT32U	FlgInited = FALSE;

	if(FlgInited == FALSE)
	{	for(i=0;i<NUM_FILTER;i++)
		{	bufx[i] = *x;
			bufy[i] = *y;
			bufz[i] = *z;}
		FlgInited = TRUE;
	}
	p++;
	if(p >= NUM_FILTER)p=0;
	bufx[p] = *x;
	bufy[p] = *y;
	bufz[p] = *z;
	for(i=0;i<NUM_FILTER;i++)
	{
		sum1 +=	bufx[i];
		sum2 +=	bufy[i];
		sum3 +=	bufz[i];
	}
	*x = sum1/NUM_FILTER;
	*y = sum2/NUM_FILTER;
	*z = sum3/NUM_FILTER;
}
/******************************************************************************
  * @brief	MMA8451初始化为测量模式，并激活状态 
  * @param  None
  * @retval None
******************************************************************************/
void	MMA8451_Init(void)
{
	INT8U	dat;

	dat = 0x00; 
	Board_I2C_RegisterWrite(MMA8451, &dat, MMA845x_XYZ_DATA_CFG, 1);//2G
	Delay(1000);
	dat = 0x21;
	Board_I2C_RegisterWrite(MMA8451, &dat, MMA845x_CTRL_REG1, 1);//激活状态
	Delay(1000);	
}

