/*******************************************************************************
  * @file    CS1237.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2015.08.24
  * @brief   ADC 模块
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "CS1237.h"
/* Private macro -------------------------------------------------------------*/
//#define AD7190_RDY_STATE       Pin_AD7195_ReadDOUT()
/* Private variables ---------------------------------------------------------*/
static	TYPE_CS1237rate g_rate;
static	INT32S	g_currADcode;
static	WAD_ERR	g_err;
static	INT32S	g_FlagNew;
static	INT32U	g_ERRCNT;		//计数值达到 1 秒钟
//
static	INT32S	g_AvgNum = 0;
static	INT32S	g_AvgCount = 0;
static	INT32S	g_AvgSum = 0;
static	WAD_AVG_PROC g_AvgBackFn = NULL;


/* Private functions ---------------------------------------------------------*/
//static	INT32U AD7195_GetRegisterValue(INT8U registerAddress,INT8U bytesNumber);
//static	void AD7195_SetRegisterValue(INT8U registerAddress,INT32U registerValue,INT8U bytesNumber);
//static	void AD7195_ChannelSelect(INT8U channel);
//static	void AD7195_Calibrate(INT8U mode, INT8U channel);
//static	void AD7195_Reset(void);
//static	void AD7195_Init(TYPE_ADCrate rate);
//static	void AD7195_WaitRdyGoLow(void);
//

static	Std_ReturnType	CS1237_Init(TYPE_CS1237rate rate);
static	void CS1237_WriteReg(INT8U adreg);
//static	INT8U CS1237_ReadReg(void);
static	Std_ReturnType CS1237_ReadAD(INT32S* ad);

static	void WAD_Working(void);
/******************************************************************************
  * @brief	ADC 初始化  
  * @param  None
  * @retval None
******************************************************************************/
Std_ReturnType	WAD_Init(TYPE_CS1237rate rate)
{
	CS1237_Init(rate);
	switch (rate)
	{
		case Rt10:
			Timer_Regist(LOOP,50,WAD_Working);
			g_ERRCNT = 20;
			break;
		case Rt40:
			Timer_Regist(LOOP,20,WAD_Working);
			g_ERRCNT = 50;
			break;
		case Rt640:
			Timer_Regist(LOOP,1,WAD_Working);
			g_ERRCNT = 1000;	
			break;
		case Rt1280:
			Timer_Regist(LOOP,1,WAD_Working);
			g_ERRCNT = 1000;
			break;
		default:	//Rt40
			Timer_Regist(LOOP,20,WAD_Working);
			g_ERRCNT = 50;
			break;
	}
	g_rate = rate;

	return TRUE;
}
/******************************************************************************
  * @brief	ADC 重新初始化  
  * @param  None
  * @retval None
******************************************************************************/
Std_ReturnType	WAD_ReInit(void)
{
	CS1237_Init(g_rate);
	return TRUE;
}
/******************************************************************************
  * @brief	检查ADC转换是否完成  
  * @param  None
  * @retval TRUE：数据准备好  FALSE：还未转换完成
******************************************************************************/
Std_ReturnType	WAD_CheckNew(void)
{
	return	g_FlagNew;		
}
/******************************************************************************
  * @brief	ADC 结果读取  
  * @param  None
  * @retval 获得的转换数据
******************************************************************************/
INT32S	WAD_Read(void)
{
	g_FlagNew = FALSE;
	return g_currADcode;
}
/******************************************************************************
  * @brief	ADC 获取平均值  
  * @param  num：求平均值的AD个数
  			fn：计算完成后的通知函数
  * @retval TRUE: 初始化成功	FALSE: 初始化失败
******************************************************************************/
Std_ReturnType	WAD_StartAvg(INT32U num,WAD_AVG_PROC fn)
{
	if((num > 0)&&(fn != NULL))
	{
		g_AvgNum = num;
		g_AvgCount = num;
		g_AvgSum = 0;
		g_AvgBackFn = fn;
		return TRUE;
	}
	return FALSE;
}
/******************************************************************************
  * @brief	读取AD模块错误标志  
  * @param  errad：返回的结构体指针
  * @retval None
******************************************************************************/
void	WAD_ErrRead(WAD_ERR* errad)
{
	errad->NoExcBit = g_err.NoExcBit;			//没有激励电压
	errad->OpenBridgeBit = g_err.OpenBridgeBit;	//桥路开路
	errad->NoAdcBit = g_err.NoAdcBit;			//多次检查没有转换完成
}
/******************************************************************************
  * @brief	发送AD值，用于调试  
  * @param  None
  * @retval None
******************************************************************************/
void WAD_send(INT32S dat)
{
	INT8U s[8] = {0xFE,0x7F};

	s[3] = dat>>24;
	s[4] = dat>>16;
	s[5] = dat>>8;
	s[6] = dat;

	RS422_SendData(s,8);
}
/******************************************************************************
  * @brief	ADC 服务工作，定时执行该函数，读取AD值，输出到全局变量 g_currTEMP  
  * @param  None
  * @retval None
******************************************************************************/
void WAD_Working(void)
{
static	 INT32U	s_errcnt = 0;
//	INT32U	dat;
	INT32S	rdat;

	if(CS1237_ReadAD(&rdat))
	{
		//g_currADcode = rdat >> 1;							//少掉最后一位，大概100万码
		rdat = Flt_PPVAvg(rdat >> 2);
		g_currADcode = Flt_MovAvg(rdat);
		//g_currADcode = Flt_MovAvg(rdat);
//WAD_send(g_currADcode);
		if((g_currADcode > 8000000)||(g_currADcode < -8000000))		//超出范围
			g_err.OpenBridgeBit = 1;
		else
			g_err.OpenBridgeBit = 0;
		g_FlagNew = TRUE;
		s_errcnt = 0;
		g_err.NoAdcBit = 0;
//AVG work----------------------------------------------------------------------
		if(g_AvgCount > 0)
		{
			g_AvgSum += g_currADcode;
			g_AvgCount--;
			if(g_AvgCount == 0)
			{
				g_AvgSum /= g_AvgNum;
				if(g_AvgBackFn != NULL)g_AvgBackFn(g_AvgSum);
				g_AvgCount = 0;
				g_AvgNum = 0;				
				g_AvgSum = 0;
				g_AvgBackFn = NULL;	
			}
		}	
	}
	else
	{	
		s_errcnt++;
		if(s_errcnt > g_ERRCNT)		// 1 秒钟没检查到转换完成
		{
			g_err.NoAdcBit = 1;
			WAD_ReInit();		//重新初始化
			s_errcnt = 0;
		}
		return;
	}
}
/******************************************************************************
  * @brief	CS1237 初始化  
  * @param  None
  * @retval None
******************************************************************************/
Std_ReturnType	CS1237_Init(TYPE_CS1237rate rate)
{

	Pin_CS1237_Init();
	switch (rate)
	{
		case Rt10:
			CS1237_WriteReg(0x0C);        //10Hz,128PGA	
			break;
		case Rt40:
			CS1237_WriteReg(0x1C);        //40Hz,128PGA	
			break;
		case Rt640:
			CS1237_WriteReg(0x2C);        //640Hz,128PGA	
			break;
		case Rt1280:
			CS1237_WriteReg(0x3C);        //1280Hz,128PGA
			break;
		default:	//Rt40
			CS1237_WriteReg(0x1C);        //40Hz,128PGA	
			break;
	}	
	return TRUE;
}
/******************************************************************************
  * @brief	CS1237 初始化  
  * @param  None
  * @retval None
******************************************************************************/
void CS1237_Clock(void)
{
    Pin_CS1237_Set(ADSCLK, PinLogic_High);
    Delay(10);
    Pin_CS1237_Set(ADSCLK, PinLogic_Low);
    Delay(10);
}
/******************************************************************************
  * @brief	CS1237 初始化  
  * @param  None
  * @retval None
******************************************************************************/
void CS1237_WriteReg(INT8U adreg)
{
	INT8U command = 0x65;
	INT32U i;
  
	for(i = 0; i < 27; i++)//clk1-clk27
    	CS1237_Clock();
	CS1237_Clock();//clk28
	CS1237_Clock();//clk29
  
	for(i = 0; i < 7; i ++) //clk30-clk36(发送读命令),7bits 数据
	{
		if(command & 0x40) //MSB -> LSB
			Pin_CS1237_Set(ADDOUT, PinLogic_High);
		else
			Pin_CS1237_Set(ADDOUT, PinLogic_Low);
    	command <<= 1;
    	CS1237_Clock();
	}
	CS1237_Clock();//clk37
  
	for(i = 0; i < 8; i++) //clk38-clk45(写入寄存器),8bits 数据
	{
		if(adreg & 0x80) //MSB -> LSB
			Pin_CS1237_Set(ADDOUT, PinLogic_High);
		else
			Pin_CS1237_Set(ADDOUT, PinLogic_Low);
		adreg <<= 1;
		CS1237_Clock();
	}
	CS1237_Clock();//clk46
}
/******************************************************************************
  * @brief	CS1237 初始化  
  * @param  None
  * @retval None
*****************************************************************************
INT8U CS1237_ReadReg(void)
{
	INT8U	command = 0x56;
	INT32U	i;
  
	for(i = 0; i < 27; i++)//clk1-clk27
    	CS1237_Clock();
	CS1237_Clock();//clk28
	CS1237_Clock();//clk29
  
	for(i = 0; i < 7; i ++) //clk30-clk36(发送读命令),7bits 数据
	{
		if(command & 0x40) //MSB -> LSB
			Pin_CS1237_Set(ADDOUT, PinLogic_High);
		else
			Pin_CS1237_Set(ADDOUT, PinLogic_Low);
		command <<= 1;
		CS1237_Clock();
	}
	CS1237_Clock();//clk37
	command = 0;
	for(i = 0; i < 8; i++) //clk38-clk45(读取寄存器),8bits 数据
	{
		command <<= 1;
		CS1237_Clock();
		if(Pin_CS1237_ReadDOUT() == PinLogic_High)
			command++;
	}
	CS1237_Clock();//clk46
	return command;
}*/
/******************************************************************************
  * @brief	CS1237 初始化  
  * @param  None
  * @retval None
******************************************************************************/
Std_ReturnType CS1237_ReadAD(INT32S* ad)
{
	INT32U i;
	INT32S addat = 0;

	Pin_CS1237_Set(ADDOUT, PinLogic_High);
  
	if(Pin_CS1237_ReadDOUT() == PinLogic_High)
		return FALSE;

	for(i = 0; i < 24; i++) //发送 24 个 CLK，接收数据
	{
		addat <<= 1;
		CS1237_Clock();
		if(Pin_CS1237_ReadDOUT() == PinLogic_High)
			addat++;
	}
	CS1237_Clock();
	CS1237_Clock();
	CS1237_Clock(); //CLK27，拉高 DRDY
	addat <<= 8;          
	addat >>= 8;         //以补码输出
	*ad = addat;
	return TRUE;
}




