/*******************************************************************************
  * @file    Text.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.09.04
  * @brief   一些软件信息处理。
  ******************************************************************************
  * @attention
  *			注意：这个文件在软件发布时一定要编译一下，否则时间和日期是不准的
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Text.h"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const char  CompanyName[]  = "MSE";				//公司名称 	3字节
      char  DeviceName[]  = "ADC101B-150101 ";	//设备名称  15字节
//const char  HBMvernum[]    = "P27";			//AD104
const char  HBMvernum[]    = "P78";				//AD103
const char  AmdPassWord[]  = "MSExby";			//超级密码   6字节

const char  part_number[]  = "40801-10001";		//P/N  物料号，			12字节
const char  part_name[]    = "AS6D-1";			//产品名称，			7字节
const char  software_vrm[] = "01.01.01";		//VRM  版本.发行.更改，	9字节
const char  software_CompileDate[]=__DATE__;	//编译日期，			12字节  “Aug 27 2013”
const char	software_CompileTime[]=__TIME__;	//编译时间，			9字节   “19:56:59”
		char  software_Date[11];				//软件日期，			10字节   “2013-08-13”
		
		INT8U MCU_ID[12];						//MCU ID，低字节在先，	12字节 

/* Private functions ---------------------------------------------------------*/
static	void	Text_SoftDate(char*	dat);
static	void	Text_ReadMCUID(INT8U* id);
/******************************************************************************
  * @brief  称重初始化
  * @param  None
  * @retval TRUE: 初始化成功	FALSE: 初始化失败 
******************************************************************************/
Std_ReturnType	Text_Init(void)
{
	Text_SoftDate(software_Date);
	Text_ReadMCUID(MCU_ID);

	DeviceName[8] = software_Date[2];
	DeviceName[9] = software_Date[3];
	DeviceName[10]= software_Date[5];
	DeviceName[11]= software_Date[6];
	DeviceName[12]= software_Date[8];
	DeviceName[13]= software_Date[9];

	return	TRUE;	 
}
/******************************************************************************
  * @brief  读取编译日期
  * @param 	dat：编译日期ASCII数组地址，共6个字节 
  * @retval None 
******************************************************************************/
void	Text_SoftDate(char*	dat)
{
const char str_months[12][3]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char num_months[12][2]={"01","02","03","04","05","06","07","08","09","10","11","12"};
	INT32U	i;

	for(i=0;i<12;i++)
	{
		if((software_CompileDate[0]==str_months[i][0])&&(software_CompileDate[1]==str_months[i][1])&&(software_CompileDate[2]==str_months[i][2]))
		{
		dat[0]=software_CompileDate[7];
		dat[1]=software_CompileDate[8];
		dat[2]=software_CompileDate[9];
		dat[3]=software_CompileDate[10];
		dat[4]='-';
		dat[5]=num_months[i][0];
		dat[6]=num_months[i][1];
		dat[7]='-';
		dat[8]=software_CompileDate[4];
		if(dat[8]==' ')dat[8]='0';
		dat[9]=software_CompileDate[5];
		dat[10]='\0';
		return;
		}		
	}
}
/******************************************************************************
  * @brief  读取 MCU ID	码
  * @param 	id：读出ID 存放的数组地址，共12个字节 
  * @retval None 
******************************************************************************/
void	Text_ReadMCUID(INT8U*	id)
{
	INT32U	CPU_ID0,CPU_ID1,CPU_ID2;

	CPU_ID0 = *(__IO INT32U*)(0x1FF80050);   
	CPU_ID1 = *(__IO INT32U*)(0x1FF80050 + 0x04);   
	CPU_ID2 = *(__IO INT32U*)(0x1FF80050 + 0x14);

	id[0] = CPU_ID2 >> 24;id[1] = CPU_ID2 >> 16;id[2] = CPU_ID2 >> 8;id[3] = CPU_ID2;
	id[4] = CPU_ID1 >> 24;id[5] = CPU_ID1 >> 16;id[6] = CPU_ID1 >> 8;id[7] = CPU_ID1;
	id[8] = CPU_ID0 >> 24;id[9] = CPU_ID0 >> 16;id[10] = CPU_ID0 >> 8;id[11] = CPU_ID0;
}

	


