/*******************************************************************************
  * @file    Parameter.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.11.16
  * @brief   参数模块
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Parameter.h"
#include "eeprom.h"
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
const PARA_TACTORY	g_Default_FactPara =
{
		"0123456",							//serialnum[7+1];
		0,									//SZA 		零点
		1000000,							//SFA		标定采样值
		1000000,							//SFAdigit	标定目标值
		//"31",								//AddrChr[2+1];
		
		0									// checksum;
};
const PARA_USER	g_Default_UserPara =
{
		"               ",		//devicename[15+1];
		"8888888",				//password[7+1];
		'A',					//dLCAddr;
		"31",					//AddrChr[2+1];
		9600,					//baudRate;
		8, 						//wordLength;
		1,						//parityBit;

		2,						//INT8U		ASS;(0 = 0mV/V,1 = 2mV/V,2 = sensor,3 = 2mV/V in)
		0,						//INT8U		HSM;(0 = 100 M/s   1 = 200 M/s)(0 = 600 M/s   1 = 1200 M/s)
		0,						//INT8U		FMD;(0 = IIR2,1 = FIR32,2 = IIR8,3 = IIR4FT,4 = FIR64,5 = FIR64+ MA)
		5,						//INT8U		ASF;(0 = OFF,...10)
		3,						//INT8U		ICR;(0...7, 600\300\150\75\38\19\9\4)

		0,						//INT32S	NOV;(0...1599999,Nominal Value)
		1000000,				//INT32S	CWT;(1000000,20%...120%NOV)
		0,						//INT32S	LDW;(0...+-1599999,zero point)
		1000000,				//INT32S	LWT;(0...+-1599999,full scale)

		"    ",					//INT8U 	ENU[4+1];(kg,g,lb...)
		0,						//INT32S	MRA;(Multirange switch point)
		1,						//INT32S	RSN;(1, 2, 5, 10, 20, 50, 100 )
		0,						//INT8U		DPT;(0...6,1 = xxxxxx.x,2 = xxxxx.xx,3 = xxxx.xxx)
		0,						//INT8U		ZSE;(0 = deactivated,1 = 2%NOV,2 = 5%,3 = 10%,4 = 20%)
		0,						//INT8U		ZTR;(0 = deactivated,1 = 0.5d,2 = 1.0d,3 = 2.0d,4 = 3.0d)
		0,						//INT8U		MTD;(0 = deactivated,1 = 0.25d/s, 0.5 , 1 , 2 , 3)
		0,						//INT32S	CRCh;(Checksum)
		0,						//INT8U		LFT;(0 = standard,1 = OIML,2 = NTEP)
		981029,					//INT32S	GCA;(Gravitational acceleration at the place of adjustment)
		981029,					//INT32S	GDE;(Gravitational acceleration at the place of installation)

		{0,1000000,0,0},		//INT32S	LIC[4];
		{0,500,2000,10000},		//INT32S	LICI[3];
		{0,500,2000,10000},		//INT32S	LICD[3];

		1,						//INT8U		TAS;(0 = net, 1 = gross)
		0,						//INT32S	TAV;(Tare value)

		8,						//INT8U 	COF;(= 0...143)
		0,						//INT8U		CSM;(= 0...3)

		0,						//INT8U		DZTtime;(0...100 s,0 =deactivated)
		0,						//INT8U		DZTband;(0...2,+-1d,+-2d,+-3d)

		0						// checksum;
};
const PARA_OTHER	g_Default_OtherPara =
{
		0,						//INT32U	key;			//
		0,						//INT32S	ang_clb_xzero;		//
		0,						//INT32S	ang_clb_yzero;		//
		0,						//INT32S	ang_clb_zzero;		//

		0						//INT8U 	checksum;		
};
//const	INT32S	g_Para_ASF[11]={1,10,20,40,80,160,200,200,200,200,200};
const	INT32S	g_Para_ASF[11]={1,2,4,8,16,30,40,80,160,200,200};
//const	INT32U	g_Para_ICR[8] = {5,5,7,14,27,54,111,250};	//(0...7, 600\300\150\75\38\19\9\4)	
const	INT32U	g_Para_ICR[8] = {5,5,7,14,27,54,111,250};

PARA_TACTORY	g_ParaFact;
PARA_USER		g_ParaUser;
PARA_OTHER		g_ParaOther;
/* Private functions ---------------------------------------------------------*/
static	INT8U Para_Calc_checksum(INT8U *str,INT32U len);

/******************************************************************************
  * @brief  参数模块初始化
  * @param  
  * @retval TRUE: 初始化成功	FALSE: 初始化失败 
******************************************************************************/
Std_ReturnType Para_Init(void)
{
	Std_ReturnType	rtn1,rtn2,rtn3;

	rtn1 = Para_Read_block(EE_ADDR_TACTORY,(INT8U*)&g_ParaFact,sizeof(PARA_TACTORY));
	if(rtn1 == FALSE)
		memcpy((INT8U*)&g_ParaFact,(INT8U*)&g_Default_FactPara, sizeof(PARA_TACTORY));

	rtn2 = Para_Read_block(EE_ADDR_USER,(INT8U*)&g_ParaUser,sizeof(PARA_USER));
	if(rtn2 == FALSE)
		memcpy((INT8U*)&g_ParaUser,(INT8U*)&g_Default_UserPara, sizeof(PARA_USER));

	rtn3 = Para_Read_block(EE_ADDR_OTHER,(INT8U*)&g_ParaOther,sizeof(PARA_OTHER));
	if(rtn3 == FALSE)
		memcpy((INT8U*)&g_ParaOther,(INT8U*)&g_Default_OtherPara, sizeof(PARA_OTHER));

	if((rtn1 == FALSE)||(rtn2 == FALSE)||(rtn3 == FALSE))
		return 	FALSE;
	else
		return	TRUE;
}
/******************************************************************************
  * @brief  计算效验和
  * @param  str:计算效验和的首字节地址
  			len:计算效验和的字节数目
  * @retval 效验和字节 
******************************************************************************/
INT8U Para_Calc_checksum(INT8U *str,INT32U len)
{
	INT32U i;
	INT8U chksum = 0xAA;

	for(i=0; i<len; i++)
	{	
		chksum += *str;
		str++;
	}
	return chksum;	
}
/******************************************************************************
  * @brief  参数模块从eeprom中读取
  * @param  eeAddr：EE中存储参数的首地址
  			destStr：读出参数存放的首地址，这里按照字节读取
			numByte：读取参数的字节长度
  * @retval TRUE：成功  FALSE：失败 
******************************************************************************/
Std_ReturnType Para_Read_block(INT32U eeAddr,INT8U * destStr,INT32U numByte)
{
	memset(destStr,0, numByte);
	EE_read(eeAddr,(INT8U*)destStr,numByte);
	if(*(destStr + numByte -1) == Para_Calc_checksum(destStr,numByte-1))
		return TRUE;
	else
		return FALSE;
}
/******************************************************************************
  * @brief  参数模块写入eeprom中
  * @param  eeAddr：EE中存储参数的首地址
  			destStr：写入参数存放的首地址，这里按照字节写入
			numByte：写入参数的字节长度
  * @retval TRUE：成功  FALSE：失败
******************************************************************************/
Std_ReturnType Para_Save_block(INT32U eeAddr,INT8U * sourceStr,INT32U numByte)
{		
	Std_ReturnType sta;
	INT8U	chk,echk;

	chk = Para_Calc_checksum(sourceStr,numByte-1);
	*(sourceStr + numByte -1) = chk;
	sta = EE_write(eeAddr,(INT32U*)sourceStr,numByte);
	if(sta != TRUE)
		return FALSE;
	if(EE_read((eeAddr+numByte-1),&echk,1)==TRUE)
	{
		if(chk==echk)
			return TRUE;
		else
			return FALSE;	
	}	
	else
		return FALSE;
}
