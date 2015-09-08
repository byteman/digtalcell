/*******************************************************************************
  * @file    CS1237.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2015.08.24
  * @brief   ADC ģ��
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
static	INT32U	g_ERRCNT;		//����ֵ�ﵽ 1 ����
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
  * @brief	ADC ��ʼ��  
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
  * @brief	ADC ���³�ʼ��  
  * @param  None
  * @retval None
******************************************************************************/
Std_ReturnType	WAD_ReInit(void)
{
	CS1237_Init(g_rate);
	return TRUE;
}
/******************************************************************************
  * @brief	���ADCת���Ƿ����  
  * @param  None
  * @retval TRUE������׼����  FALSE����δת�����
******************************************************************************/
Std_ReturnType	WAD_CheckNew(void)
{
	return	g_FlagNew;		
}
/******************************************************************************
  * @brief	ADC �����ȡ  
  * @param  None
  * @retval ��õ�ת������
******************************************************************************/
INT32S	WAD_Read(void)
{
	g_FlagNew = FALSE;
	return g_currADcode;
}
/******************************************************************************
  * @brief	ADC ��ȡƽ��ֵ  
  * @param  num����ƽ��ֵ��AD����
  			fn��������ɺ��֪ͨ����
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ��
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
  * @brief	��ȡADģ������־  
  * @param  errad�����صĽṹ��ָ��
  * @retval None
******************************************************************************/
void	WAD_ErrRead(WAD_ERR* errad)
{
	errad->NoExcBit = g_err.NoExcBit;			//û�м�����ѹ
	errad->OpenBridgeBit = g_err.OpenBridgeBit;	//��·��·
	errad->NoAdcBit = g_err.NoAdcBit;			//��μ��û��ת�����
}
/******************************************************************************
  * @brief	����ADֵ�����ڵ���  
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
  * @brief	ADC ����������ʱִ�иú�������ȡADֵ�������ȫ�ֱ��� g_currTEMP  
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
		//g_currADcode = rdat >> 1;							//�ٵ����һλ�����100����
		rdat = Flt_PPVAvg(rdat >> 2);
		g_currADcode = Flt_MovAvg(rdat);
		//g_currADcode = Flt_MovAvg(rdat);
//WAD_send(g_currADcode);
		if((g_currADcode > 8000000)||(g_currADcode < -8000000))		//������Χ
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
		if(s_errcnt > g_ERRCNT)		// 1 ����û��鵽ת�����
		{
			g_err.NoAdcBit = 1;
			WAD_ReInit();		//���³�ʼ��
			s_errcnt = 0;
		}
		return;
	}
}
/******************************************************************************
  * @brief	CS1237 ��ʼ��  
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
  * @brief	CS1237 ��ʼ��  
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
  * @brief	CS1237 ��ʼ��  
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
  
	for(i = 0; i < 7; i ++) //clk30-clk36(���Ͷ�����),7bits ����
	{
		if(command & 0x40) //MSB -> LSB
			Pin_CS1237_Set(ADDOUT, PinLogic_High);
		else
			Pin_CS1237_Set(ADDOUT, PinLogic_Low);
    	command <<= 1;
    	CS1237_Clock();
	}
	CS1237_Clock();//clk37
  
	for(i = 0; i < 8; i++) //clk38-clk45(д��Ĵ���),8bits ����
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
  * @brief	CS1237 ��ʼ��  
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
  
	for(i = 0; i < 7; i ++) //clk30-clk36(���Ͷ�����),7bits ����
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
	for(i = 0; i < 8; i++) //clk38-clk45(��ȡ�Ĵ���),8bits ����
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
  * @brief	CS1237 ��ʼ��  
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

	for(i = 0; i < 24; i++) //���� 24 �� CLK����������
	{
		addat <<= 1;
		CS1237_Clock();
		if(Pin_CS1237_ReadDOUT() == PinLogic_High)
			addat++;
	}
	CS1237_Clock();
	CS1237_Clock();
	CS1237_Clock(); //CLK27������ DRDY
	addat <<= 8;          
	addat >>= 8;         //�Բ������
	*ad = addat;
	return TRUE;
}




