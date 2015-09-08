/*******************************************************************************
  * @file    AD7195.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.06.03
  * @brief   ADC 模块
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "AD7195.h"
/* Private macro -------------------------------------------------------------*/
#define AD7190_RDY_STATE       Pin_AD7195_ReadDOUT()
/* Private variables ---------------------------------------------------------*/
static	TYPE_ADCrate g_rate;
static	INT32S	g_currADcode;
static	AD_ERR	g_err;
static	INT32S	g_FlagNew;
static	INT32U	g_ERRCNT;		//计数值达到 1 秒钟

static	INT32S	g_AvgNum = 0;
static	INT32S	g_AvgCount = 0;
static	INT32S	g_AvgSum = 0;
static	AD_AVG_PROC g_AvgBackFn = NULL;
/* Private functions ---------------------------------------------------------*/
static	INT32U AD7195_GetRegisterValue(INT8U registerAddress,INT8U bytesNumber);
static	void AD7195_SetRegisterValue(INT8U registerAddress,INT32U registerValue,INT8U bytesNumber);
static	void AD7195_ChannelSelect(INT8U channel);
static	void AD7195_Calibrate(INT8U mode, INT8U channel);
static	void AD7195_Reset(void);
static	void AD7195_Init(TYPE_ADCrate rate);
static	void AD7195_WaitRdyGoLow(void);

static	void AD_Working(void);
/******************************************************************************
  * @brief	ADC 初始化  
  * @param  None
  * @retval None
******************************************************************************/
Std_ReturnType	AD_Init(TYPE_ADCrate rate)
{
	AD7195_Init(rate);

	switch (rate)
	{
		case R10:	
			Timer_Regist(LOOP,50,AD_Working);
			g_ERRCNT = 20;
			break;
		case R50:	
			Timer_Regist(LOOP,10,AD_Working);
			g_ERRCNT = 100;
			break;
		case R100:	
			Timer_Regist(LOOP,5,AD_Working);
			g_ERRCNT = 200;
			break;
		case R200:	
			Timer_Regist(LOOP,2,AD_Working);
			g_ERRCNT = 500;
			break;
		default:	
			Timer_Regist(LOOP,50,AD_Working);		//默认 10Hz
			g_ERRCNT = 20;
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
Std_ReturnType	AD_ReInit(void)
{
	AD7195_Init(g_rate);
	return TRUE;
}
/******************************************************************************
  * @brief	检查ADC转换是否完成  
  * @param  None
  * @retval TRUE：数据准备好  FALSE：还未转换完成
******************************************************************************/
Std_ReturnType	AD_CheckNew(void)
{
	return	g_FlagNew;		
}
/******************************************************************************
  * @brief	ADC 结果读取  
  * @param  None
  * @retval 获得的转换数据
******************************************************************************/
INT32S	AD_Read(void)
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
Std_ReturnType	AD_StartAvg(INT32U num,AD_AVG_PROC fn)
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
void	AD_ErrRead(AD_ERR* errad)
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
void AD_send(INT32S dat)
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
void AD_Working(void)
{
static	 INT32U	s_errcnt = 0;
	INT32U	dat;
	INT32S	rdat;

	if(Pin_AD7195_ReadDOUT() == PinLogic_Low)
	{
		dat = AD7195_GetRegisterValue(AD7195_REG_DATA,4);
		if(dat & AD7195_STAT_ERR)g_err.NoExcBit = 1;		//一般参考电压低会职位该位
		else	g_err.NoExcBit = 0;	
		rdat = dat>>8;
		rdat = rdat - 0x800000;								//转换为有符号整型数
		//g_currADcode = rdat >> 1;							//少掉最后一位，大概100万码
		rdat = Flt_PPVAvg(rdat >> 1);
		g_currADcode = Flt_MovAvg(rdat);
//AD_send(g_currADcode);
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
			AD_ReInit();		//重新初始化
			s_errcnt = 0;
		}
		return;
	}
}
/******************************************************************************
  * @brief	AD7195 读写一个字节  
  * @param  wbt：要写入的字节
  * @retval 读出的字节
******************************************************************************/
INT8U	AD7195_WRByte(INT8U wbt)
{
	INT32U j;
	INT8U rbt = 0;	

	for(j=0; j<8; j++)
	{
		Pin_AD7195_Set(SCLK,PinLogic_Low);			
		Delay(1);
//--------------------------------------------------------
		if(0x80 == (wbt & 0x80))
			Pin_AD7195_Set(DIN,PinLogic_High);
		else
			Pin_AD7195_Set(DIN,PinLogic_Low);
		wbt <<= 1;
//--------------------------------------------------------
		rbt <<= 1;
		if(Pin_AD7195_ReadDOUT() == PinLogic_High)
			rbt += 1;
//--------------------------------------------------------
		Pin_AD7195_Set(SCLK,PinLogic_High);
		Delay(1);
	}
	return rbt;
}
/******************************************************************************
  * @brief	AD7195 读寄存器  
  * @param  registerAddress：寄存器地址
  			bytesNumber：寄存器长度，字节数
  * @retval 读出的寄存器值
******************************************************************************/
INT32U AD7195_GetRegisterValue(INT8U registerAddress,INT8U bytesNumber)
{
	INT8U	cmd = 0; 
    INT32U	buffer	= 0,i;
     
	cmd = AD7195_COMM_READ | AD7195_COMM_ADDR(registerAddress);

    AD7195_WRByte(cmd);
    for(i=0;i<bytesNumber;i++) 
    {		
		buffer = (buffer << 8);
		buffer += AD7195_WRByte(0xFF);
    }    
    return buffer;
}
/******************************************************************************
  * @brief	AD7195 写寄存器  
  * @param  registerAddress：寄存器地址
  			registerValue：写入的数据，低字节对齐
  			bytesNumber：寄存器长度，字节数
  * @retval None
******************************************************************************/
void AD7195_SetRegisterValue(INT8U registerAddress,INT32U registerValue,INT8U bytesNumber)
{
    INT8U writeCommand[5] = {0, 0, 0, 0, 0};
    INT8U bytesNr         = bytesNumber;
	INT32U	i;
    
    writeCommand[0] = AD7195_COMM_WRITE | AD7195_COMM_ADDR(registerAddress);
    while(bytesNr > 0)
    {
        writeCommand[bytesNr] = (INT8U)registerValue;
        registerValue >>= 8;
        bytesNr --;
    }
	for(i=0;i<(bytesNumber+1);i++)
	{
		AD7195_WRByte(writeCommand[i]);
	} 
}
/******************************************************************************
  * @brief	AD7195 通道选择  
  * @param  channel：选择的通道
  * @retval None
******************************************************************************/
void AD7195_ChannelSelect(INT8U channel)
{
    INT32U oldRegValue = 0x0;
    INT32U newRegValue = 0x0;   
     
    oldRegValue = AD7195_GetRegisterValue(AD7195_REG_CONF, 3);
    oldRegValue &= ~(AD7195_CONF_CHAN(0x3F));
    newRegValue = oldRegValue | AD7195_CONF_CHAN(1 << channel);   
    AD7195_SetRegisterValue(AD7195_REG_CONF, newRegValue, 3);
}
/******************************************************************************
  * @brief	AD7195 标定  
  * @param  mode：标定模式
  			channel：标定的通道
  * @retval None
******************************************************************************/
void AD7195_Calibrate(INT8U mode, INT8U channel)
{
    INT32U oldRegValue = 0x0;
    INT32U newRegValue = 0x0;
    
    AD7195_ChannelSelect(channel);
    oldRegValue = AD7195_GetRegisterValue(AD7195_REG_MODE, 3);
    oldRegValue &= ~AD7195_MODE_SEL(0x7);
    newRegValue = oldRegValue | AD7195_MODE_SEL(mode);
    //ADI_PART_CS_LOW; 
    AD7195_SetRegisterValue(AD7195_REG_MODE, newRegValue, 3); // CS is not modified.
    AD7195_WaitRdyGoLow();
    //ADI_PART_CS_HIGH;
}
/******************************************************************************
  * @brief	AD7195 复位  
  * @param  None
  * @retval None
******************************************************************************/
void	AD7195_Reset(void)
{
	//Pin_AD7195_Set(CS,PinLogic_Low);
	AD7195_WRByte(0x01);
	AD7195_WRByte(0xFF);
	AD7195_WRByte(0xFF);
	AD7195_WRByte(0xFF);
	AD7195_WRByte(0xFF);
	AD7195_WRByte(0xFF);
	AD7195_WRByte(0xFF);
	//Pin_AD7195_Set(CS,PinLogic_High);
}
/******************************************************************************
  * @brief	AD7195 初始化  
  * @param  None
  * @retval None
******************************************************************************/
void	AD7195_Init(TYPE_ADCrate rate)
{
//    INT32U oldRegValue = 0x0;
    INT32U newRegValue = 0x0;
	INT32U fs;

	Pin_AD7195_Init();
	Pin_AD7195_Set(CS,PinLogic_High);
	Delay(10);
	Pin_AD7195_Set(CS,PinLogic_Low);
	Delay(100);
	AD7195_Reset();
	Delay(1000);			//600us

	switch (rate)
	{
		case R10:
			fs = 480;	
			break;
		case R50:
			fs = 96;	
			break;
		case R100:
			fs = 48;	
			break;
		case R200:
			fs = 24;	
			break;
		default:
			fs = 480;	
			break;
	}

	AD7195_Calibrate(AD7195_MODE_CAL_INT_ZERO, AD7195_CH_AIN3P_AIN4M);
//	AD7195_Calibrate(AD7195_MODE_CAL_INT_FULL, AD7195_CH_AIN1P_AIN2M);
	//oldRegValue = AD7195_GetRegisterValue(AD7195_REG_MODE,3);
	newRegValue = 	AD7195_MODE_SEL(AD7195_MODE_CONT) | 
					AD7195_MODE_DAT_STA |
					AD7195_MODE_CLKSRC(AD7195_CLK_EXT_MCLK2) |
					//AD7195_MODE_CLKSRC(AD7195_CLK_INT) |
					AD7195_MODE_ENPAR |
					AD7195_MODE_RATE(fs);
	AD7195_SetRegisterValue(AD7195_REG_MODE,newRegValue,3);
	//oldRegValue = AD7195_GetRegisterValue(AD7195_REG_CONF,3);		
	newRegValue = 	//AD7195_CONF_ACX |
					AD7195_CONF_CHAN(1 << AD7195_CH_AIN3P_AIN4M) |			  
					AD7195_CONF_BUF |
					AD7195_CONF_GAIN(AD7195_CONF_GAIN_128);
	AD7195_SetRegisterValue(AD7195_REG_CONF,newRegValue,3);
	//
//	Delay(1000);			//600us
}
/******************************************************************************
  * @brief	AD7195 等待工作完成，等待准备好  
  * @param  None
  * @retval None
******************************************************************************/
void AD7195_WaitRdyGoLow(void)
{
    INT32U timeOutCnt = 0xFFFFF;
    
    while((Pin_AD7195_ReadDOUT() == PinLogic_High) && timeOutCnt--)
    {
        ;
    }
}

