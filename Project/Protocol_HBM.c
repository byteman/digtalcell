/*******************************************************************************
  * @file    Protocol_HBM.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.11.16
  * @brief   HBM 通讯协议处理
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Protocol_HBM.h"

#include "eeprom.h"
/* Private macro -------------------------------------------------------------*/
#define		NUM_RECBUF		30
#define		NUM_SENDBUF		50

typedef enum
{	Sno = 0, 	//do not executes commands, do not responds.
	Sxx = 1,	//executes all the commands and responds. 
	S98 = 2,	//All AEDs execute all commands, but do not respond
	S99 = 3,	//All AEDs execute all commands and respond
}WORKSTAT;
/* Private variables ---------------------------------------------------------*/
static	INT8U	g_Reciv_Bufer[NUM_RECBUF];		//接收通讯字节缓冲区
static	INT32U	g_Reciv_Pt = 0;					//通讯接收缓冲区指针
static	char	g_Send_Bufer[NUM_SENDBUF];		//发送数据缓冲区
static	char	g_Tmp_Send_Bufer[NUM_SENDBUF+3];		//发送数据缓冲区

static	INT32U	g_Send_Num = 0;					//发送数据长度

static	WORKSTAT	g_Flg_Work = Sxx;			//通讯选择状态
static	INT32S	g_Flg_Unlocked = FALSE;			//减除锁定状态

static	INT32U	g_Num_MSV = 0;					//连续发送数据个数
/* Private functions ---------------------------------------------------------*/
static	void	Ptl_TimeOut(void);
static	void	Ptl_Process(void);
static	void	Ptl_Executes(void);
static	void	Ptl_Responds(void);
/******************************************************************************
	以下为命令执行函数 
******************************************************************************/

int build_resp(const char* cmd,char *buf, int size)
{
	int len = strlen(cmd);
	strcpy(g_Tmp_Send_Bufer,cmd);
	memcpy(g_Tmp_Send_Bufer+len,buf,size);
	memcpy(g_Send_Bufer,g_Tmp_Send_Bufer,size+len);
	return size+len;
}

/*Interface settings(ADR, BDR, COF, CSM, TEX, STR)***************************************/
/*ADR*/
void	Ptl_CMD_ADR(void)
{
	INT32U	i;
	INT8U	serNumber[7+1];

	if(g_Reciv_Bufer[3] == '?')
	{		
		memcpy(g_Send_Bufer,g_ParaUser.AddrChr,2);
		sprintf(g_Send_Bufer+2,"\r\n");
		g_Send_Num = 4;
	}
	else
	{
		if(g_Reciv_Pt >= 14)
		{
			Para_Read_block(EE_ABOUT_SN,serNumber,8);
			for(i=0;i<7;i++)
			{
				if(g_Reciv_Bufer[i+7] != serNumber[i])
				{
					g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
					return;
				}
			}
		}
		if((g_Reciv_Bufer[4] == '\0')||(g_Reciv_Bufer[4] == ','))
		{
			g_ParaUser.AddrChr[0] = '0';
			g_ParaUser.AddrChr[1] = g_Reciv_Bufer[3];
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else if((g_Reciv_Bufer[5] == '\0')||(g_Reciv_Bufer[5] == ','))
		{
			g_ParaUser.AddrChr[0] = g_Reciv_Bufer[3];
			g_ParaUser.AddrChr[1] = g_Reciv_Bufer[4];
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
}
Std_ReturnType Ptl_Save(void)
{
	Std_ReturnType rtn = Para_Save_block(EE_ADDR_USER,(INT8U*)&g_ParaUser,sizeof(PARA_USER));
	if(rtn == TRUE)
	{
		Wet_InitPara();
	}
	return rtn;

}
/*BDR*/
void	Ptl_CMD_BDR(void)
{
	INT32U	i = 0,err = 0,baud;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%d,%1d\r\n",g_ParaUser.baudRate,g_ParaUser.parityBit);
		return;
	}
	else if(g_Reciv_Bufer[3] == ',')
	{
		if((g_Reciv_Bufer[4] == '0')||(g_Reciv_Bufer[4] == '1'))
			g_ParaUser.parityBit = g_Reciv_Bufer[4]-'0';
		else
			err++;
	}
	else
	{
		for(i=3;i<NUM_RECBUF;i++)
		{
			if(g_Reciv_Bufer[i] == '\0')
			{
				g_ParaUser.parityBit = 0;
				break;
			}
			else if(g_Reciv_Bufer[i] == ',')
			{
				g_Reciv_Bufer[i] = '\0';
				if((g_Reciv_Bufer[i+1] == '0')||(g_Reciv_Bufer[i+1] == '1'))
					g_ParaUser.parityBit = g_Reciv_Bufer[i+1]-'0';
				else
					err++;	
				break;
			}
		}
		baud = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
		if((baud > 0)&&(baud % 1200 == 0))
			g_ParaUser.baudRate = baud;
		else
			err++;
	}	
	if(err > 0)
		g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	else
	{
		RS422_Init(g_ParaUser.baudRate,8,g_ParaUser.parityBit);
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	}
}


/*Factory characteristic curve(SZA, SFA)***************************************/
void	Ptl_back_SZA(INT32S avg)
{
	Std_ReturnType	rtn;

	if((avg > -100000)&&(avg < 100000))
	{
		g_ParaFact.SZA = avg;
		g_ParaUser.LDW = 0;
		g_ParaUser.LWT = 1000000;
		g_ParaUser.CWT = 1000000;
		Wet_InitPara();

		rtn = Para_Save_block(EE_ADDR_TACTORY,(INT8U*)&g_ParaFact,sizeof(PARA_TACTORY));
		if(rtn == TRUE)
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	Ptl_Responds();
}
/*SZA*/
void	Ptl_CMD_SZA(void)
{
	INT32S	tmp;
	Std_ReturnType	rtn;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%7d\r\n",g_ParaFact.SZA);	
	}
	else if(g_Reciv_Bufer[3] == '\0')
	{
		if(WAD_StartAvg(30,Ptl_back_SZA) == FALSE)
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");	
	}
	else
	{
		if((g_Reciv_Pt > 4)&&(g_Reciv_Pt < 13))			//参数加符号最多8个字符
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
			if((tmp > -100000)&&(tmp < 100000))
			{
				g_ParaFact.SZA = tmp;
				g_ParaUser.LDW = 0;
				g_ParaUser.LWT = 1000000;
				g_ParaUser.CWT = 1000000;
				Wet_InitPara();

				rtn = Para_Save_block(EE_ADDR_TACTORY,(INT8U*)&g_ParaFact,sizeof(PARA_TACTORY));
				if(rtn == TRUE)
					g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
				else
					g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
			}
			else
				g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
}
void	Ptl_back_SFA(INT32S avg)
{
	Std_ReturnType	rtn;

	if(((avg > -1600000)&&(avg < -600000))||((avg > 600000)&&(avg < 1600000)))
	{
		g_ParaFact.SFA = avg;
		g_ParaFact.SFAdigit	= 1000000;
		g_ParaUser.LDW = 0;
		g_ParaUser.LWT = 1000000;
		g_ParaUser.CWT = 1000000;
		Wet_InitPara();

		rtn = Para_Save_block(EE_ADDR_TACTORY,(INT8U*)&g_ParaFact,sizeof(PARA_TACTORY));
		if(rtn == TRUE)
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	Ptl_Responds();
}
/*SFA*/
void	Ptl_CMD_SFA(void)
{
	INT32S	tmp;
	Std_ReturnType	rtn;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%7d\r\n",g_ParaFact.SFA);	
	}
	else if(g_Reciv_Bufer[3] == '\0')
	{
		if(WAD_StartAvg(30,Ptl_back_SFA) == FALSE)
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
	else
	{
		if((g_Reciv_Pt > 4)&&(g_Reciv_Pt < 13))			//参数加符号最多8个字符
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
			if(((tmp > -1600000)&&(tmp < -600000))||((tmp > 600000)&&(tmp < 1600000)))
			{
				g_ParaFact.SFA = tmp;
				g_ParaFact.SFAdigit	= 1000000;
				g_ParaUser.LDW = 0;
				g_ParaUser.LWT = 1000000;
				g_ParaUser.CWT = 1000000;
				Wet_InitPara();

				rtn = Para_Save_block(EE_ADDR_TACTORY,(INT8U*)&g_ParaFact,sizeof(PARA_TACTORY));
				if(rtn == TRUE)
					g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
				else
					g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
			}
			else
				g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
}

void Ptl_back_CLK(INT32S avg)
{
	if((avg > -1000000)&&(avg < 1000000))
	{
		g_ParaUser.LDW = avg;
		Wet_InitPara();
		if(Ptl_Save())
			g_Send_Num = sprintf(g_Send_Bufer,"CLK:0:%d",g_ParaUser.LDW); //设置参数的返回
		else 
			g_Send_Num = sprintf(g_Send_Bufer,"CKL:2"); //设置参数的返回
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"CKL:3");
	Ptl_Responds();
}

void Ptl_CMD_CLK()
{
	
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"CLK?%d",g_ParaUser.LWT);	
	}
	else if(g_Reciv_Bufer[3] == '\0')
	{
		if(Wet_StartAvg(30,Ptl_back_CLK) == FALSE)
			g_Send_Num = sprintf(g_Send_Bufer,"CLK:1");	
	}
	else if(g_Reciv_Bufer[3] == ':')//一条命令中包含了cwt重量.
	{
		if((g_Reciv_Pt > 5)&&(g_Reciv_Pt < 14))			//参数加符号最多8个字符
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[4]);
			if((tmp > -100000)&&(tmp < 100000))
			{
				g_ParaUser.CWT= tmp;
				if(Wet_StartAvg(30,Ptl_back_CLK) == FALSE)	
					g_Send_Num = sprintf(g_Send_Bufer,"CLK:1");	
			}
			else
				g_Send_Num = sprintf(g_Send_Bufer,"CLK:2");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"CLK:3");
	}

}
void	Ptl_CMD_CLW(void)
{
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?') //查询标定重量.
	{
		g_Send_Num = sprintf(g_Send_Bufer,"CLW?%8d",g_ParaUser.CWT);	
	}
	else
	{
		if((g_Reciv_Pt > 5)&&(g_Reciv_Pt < 14))			//参数加符号最多8个字符
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[4]);

			if(tmp >= 200000)
			{
				g_ParaUser.CWT = tmp;
				g_Send_Num = sprintf(g_Send_Bufer,"CLW:0");
			}
			else
				g_Send_Num = sprintf(g_Send_Bufer,"CLW:1");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"CLW:2");
	}
}
void	Ptl_back_CLZ(INT32S avg)
{
	if((avg > -1000000)&&(avg < 1000000))
	{
		g_ParaUser.LDW = avg;
		Wet_InitPara();
		g_Send_Num = sprintf(g_Send_Bufer,"CLZ:0:%ld",avg);
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"CLZ:2");
	Ptl_Responds();
}
//标定零点
void	Ptl_CMD_CLZ(void)
{

	if(g_Reciv_Bufer[3] == '?') //查询零点
	{
		g_Send_Num = sprintf(g_Send_Bufer,"CLZ?%7d",g_ParaUser.LDW);	
	}
	else if(g_Reciv_Bufer[3] == '\0') //标定零点
	{
		if(Wet_StartAvg(30,Ptl_back_CLZ) == FALSE)
			g_Send_Num = sprintf(g_Send_Bufer,"CLZ:1");	
	}
	
	
}


/*Application characteristic curve and output formatting (CWT, LDW, LWT, NOV, RSN, ENU, DPT, LIC; MRA, GCA, GDE)*/
/*CWT*/
void	Ptl_CMD_CWT(void)
{
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%8d,%8d\r\n",g_ParaUser.CWT,g_ParaUser.CWT);	
	}
	else
	{
		if((g_Reciv_Pt > 4)&&(g_Reciv_Pt < 13))			//参数加符号最多8个字符
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
//			if(g_ParaUser.NOV == 0)
//			{
				if(tmp >= 200000)
				{
					g_ParaUser.CWT = tmp;
					g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
				}
				else
					g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
//			}
//			else
//			{
//				if(tmp >= (g_ParaUser.NOV/5))	//20%
//				{
//					g_ParaUser.CWT = tmp;
//					g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
//				}
//				else
//					g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
//			}
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	}
}
/*LDW*/
void	Ptl_back_LDW(INT32S avg)
{
	if((avg > -1000000)&&(avg < 1000000))
	{
		g_ParaUser.LDW = avg;
		Wet_InitPara();
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	Ptl_Responds();
}




void	Ptl_back_ZER(INT32S avg)
{
	if((avg > -1000000)&&(avg < 1000000))
	{
		g_ParaUser.LDW = avg;
		Wet_InitPara();
		g_Send_Num = sprintf(g_Send_Bufer,"ZER:0");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"ZER:1");
	Ptl_Responds();
}

void	Ptl_CMD_LDW(void)
{
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%7d\r\n",g_ParaUser.LDW);	
	}
	else if(g_Reciv_Bufer[3] == '\0')
	{
		if(Wet_StartAvg(30,Ptl_back_LDW) == FALSE)
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");	
	}
	else
	{
		if((g_Reciv_Pt > 4)&&(g_Reciv_Pt < 13))			//参数加符号最多8个字符
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
			if((tmp > -100000)&&(tmp < 100000))
			{
				g_ParaUser.LDW = tmp;
				Wet_InitPara(); //在里面会重新计算k值.
				g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
			}
			else
				g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
}
/*LWT*/
void	Ptl_back_LWT(INT32S avg)
{
	Std_ReturnType	rtn;
	//if(((avg > -1600000)&&(avg < -600000))||((avg > 600000)&&(avg < 1600000)))
	if(avg > g_ParaUser.LDW)
	{
		g_ParaUser.LWT = avg;
		Wet_InitPara();
		rtn = Para_Save_block(EE_ADDR_USER,(INT8U*)&g_ParaUser,sizeof(PARA_USER));
		if(rtn == TRUE)
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");			
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	Ptl_Responds();
}
void	Ptl_CMD_LWT(void)
{
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%7d\r\n",g_ParaUser.LWT);	
	}
	else if(g_Reciv_Bufer[3] == '\0')
	{
		if(Wet_StartAvg(30,Ptl_back_LWT) == FALSE)
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");	
	}
	else
	{
		if((g_Reciv_Pt > 4)&&(g_Reciv_Pt < 13))			//参数加符号最多8个字符
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
			if(((tmp > -1600000)&&(tmp < -600000))||((tmp > 600000)&&(tmp < 1600000)))
			{
				g_ParaUser.LWT = tmp;
				Wet_InitPara();
				g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
			}
			else
				g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
}

/*RSN*/
void	Ptl_CMD_RSN(void)
{
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%d\r\n",g_ParaUser.RSN);	
	}
	else
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
		if((tmp == 1)||(tmp == 2)||(tmp == 5)||(tmp == 10)||(tmp == 20)||(tmp == 50)||(tmp == 100))
		{
			g_ParaUser.RSN = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");	
	}
}
/*LIC*/
void	Ptl_CMD_LIC(void)
{
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%d,%d,%d,%d\r\n",g_ParaUser.LIC[0],g_ParaUser.LIC[1],g_ParaUser.LIC[2],g_ParaUser.LIC[3]);	
	}
	//else if((g_Reciv_Bufer[3] > '/')&&(g_Reciv_Bufer[3] < '4'))		//LIC0--3
	else if((g_Reciv_Bufer[3] == '0')&&(g_Reciv_Bufer[4] == ','))
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[5]);
		g_ParaUser.LIC[0] = tmp;
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	}
	else if((g_Reciv_Bufer[3] == '1')&&(g_Reciv_Bufer[4] == ','))
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[5]);
		g_ParaUser.LIC[1] = tmp;
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	}
	else if((g_Reciv_Bufer[3] == '2')&&(g_Reciv_Bufer[4] == ','))
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[5]);
		g_ParaUser.LIC[2] = tmp;
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	}
	else if((g_Reciv_Bufer[3] == '3')&&(g_Reciv_Bufer[4] == ','))
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[5]);
		g_ParaUser.LIC[3] = tmp;
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	}
	else if((g_Reciv_Bufer[3] == 'I')||(g_Reciv_Bufer[3] == 'i'))				//多段线性修正原始值
	{
		if(g_Reciv_Bufer[4] == '?')
		{
			g_Send_Num = sprintf(g_Send_Bufer,"%d,%d,%d\r\n",g_ParaUser.LICI[1],g_ParaUser.LICI[2],g_ParaUser.LICI[3]);
		}
		else if((g_Reciv_Bufer[4] == '1')&&(g_Reciv_Bufer[5] == ','))
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[6]);
			g_ParaUser.LICI[1] = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else if((g_Reciv_Bufer[4] == '2')&&(g_Reciv_Bufer[5] == ','))
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[6]);
			g_ParaUser.LICI[2] = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else if((g_Reciv_Bufer[4] == '3')&&(g_Reciv_Bufer[5] == ','))
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[6]);
			g_ParaUser.LICI[3] = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
	else if((g_Reciv_Bufer[3] == 'D')||(g_Reciv_Bufer[3] == 'd'))				//多段线性修正目标值
	{
		if(g_Reciv_Bufer[4] == '?')
		{
			g_Send_Num = sprintf(g_Send_Bufer,"%d,%d,%d\r\n",g_ParaUser.LICD[1],g_ParaUser.LICD[2],g_ParaUser.LICD[3]);
		}
		else if((g_Reciv_Bufer[4] == '1')&&(g_Reciv_Bufer[5] == ','))
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[6]);
			g_ParaUser.LICD[1] = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else if((g_Reciv_Bufer[4] == '2')&&(g_Reciv_Bufer[5] == ','))
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[6]);
			g_ParaUser.LICD[2] = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else if((g_Reciv_Bufer[4] == '3')&&(g_Reciv_Bufer[5] == ','))
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[6]);
			g_ParaUser.LICD[3] = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
}
/*Settings for measuring mode (HSM, FMD, ASF, NTF, ICR, MTD, ZSE, ZTR)*******************/

/*DZT*/
void	Ptl_CMD_DZT(void)
{
	INT8U	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%3d,%1d\r\n",g_ParaUser.DZTtime,g_ParaUser.DZTband);	
	}
	else if(g_Reciv_Bufer[3] == ',')
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[4]);
		if(tmp <= 2)
		{
			g_ParaUser.DZTband = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	}
	else
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
		if(tmp <= 100)
		{
			g_ParaUser.DZTtime = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	}
}
/*Commands for measuring mode (MSV, TAR, TAS, TAV, CDL)**********************************/
/*STP*/
void	Ptl_CMD_STP(void)
{
	g_Num_MSV = 0;
}
/*构建发送数据帧*/
INT32U	Ptl_Create_Frame(void)
{
	WET_STATE	wf;
	INT32S		wt;
	INT8U		st = 0x00;
	INT32U		frameleng = 0;

	wt = Wet_Read(&wf);
	if(wf.err == 1)st |= 0x80;
	if(wf.stable == 1)st |= 0x08;
	if(wf.overADC == 1)st |= 0x04;
	if(wf.overGross == 1)st |= 0x02;
	if(wf.overNET == 1)st |= 0x01;

	if((g_ParaUser.COF & 0x01) == 0x00)	//Binary
	{
		if((g_ParaUser.COF & 0x08) == 0x08)	//status
		{
			if(g_ParaUser.CSM == 0)
			{
				st = st;
			}
			else if(g_ParaUser.CSM == 1)
			{
				st = ((INT8U)(wt>>16))^((INT8U)(wt>>8))^((INT8U)wt);
			}
//			else if(g_ParaUser.CSM == 2)
//			{
//			}
//			else if(g_ParaUser.CSM == 3)
//			{
//			}
		}
		switch(g_ParaUser.COF & 0x06)
		{
			case 0x00:					//4 byte, MSB
				g_Send_Bufer[0] = wt>>16;
				g_Send_Bufer[1] = wt>>8;
				g_Send_Bufer[2] = wt;
				g_Send_Bufer[3] = st;
				frameleng += 4;
				break;
			case 0x02:					//2 byte, MSB
				g_Send_Bufer[0] = wt>>8;
				g_Send_Bufer[1] = wt;
				frameleng += 2;
				break;
			case 0x04:					//4 byte, LSB
				g_Send_Bufer[0] = st;
				g_Send_Bufer[1] = wt;
				g_Send_Bufer[2] = wt>>8;
				g_Send_Bufer[3] = wt>>16;
				frameleng += 4;
				break;
			case 0x06:					//2 byte, LSB 
				g_Send_Bufer[0] = wt;
				g_Send_Bufer[1] = wt>>8;
				frameleng += 2;
				break;
			default:
				break;
		}	
	}
	else								//ASCII								
	{
		if((g_ParaUser.COF & 0x02) == 0x02)//no addr
		{
			frameleng = sprintf(g_Send_Bufer,"%8d",wt);
		}
		else							//addr
		{
			frameleng = sprintf(g_Send_Bufer,"%8d",wt);
			g_Send_Bufer[frameleng] = ',';
			frameleng++;
			g_Send_Bufer[frameleng] = g_ParaUser.AddrChr[0];
			frameleng++;
			g_Send_Bufer[frameleng] = g_ParaUser.AddrChr[1];
			frameleng++;	
		}
		if((g_ParaUser.COF & 0x08) == 0x08)
		{
			g_Send_Bufer[frameleng] = ',';
			frameleng++;
			sprintf(g_Send_Bufer+frameleng,"%3d",st);
			frameleng += 3;
		}
	}
	//build_resp("MSV?"g_Send_Bufer,
	return	frameleng;
}
/*MSV*/
void	Ptl_back_MSV(void)
{
	INT32U		frameleng = 0;

	frameleng = Ptl_Create_Frame();
	if(g_Num_MSV > 0)g_Num_MSV--;
	if(g_Num_MSV == 0)
	{
		sprintf(g_Send_Bufer+frameleng,"\r\n");
		g_Send_Num = frameleng+2;
		if(g_Flg_Work == Sxx)Ptl_Responds();
	}
	else
	{
		g_Send_Num = frameleng;
		if(g_Flg_Work == Sxx)Ptl_Responds();
		Timer_Regist(ONE_TIME,g_Para_ICR[g_ParaUser.ICR],Ptl_back_MSV);
	}
}
void	Ptl_CMD_MSV(void)
{
	INT32S		wt;
	INT32U		frameleng = 0;

	if(g_Reciv_Bufer[3] == '?')
	{
		if((g_Reciv_Bufer[4] > '/')&&(g_Reciv_Bufer[4] < ':'))
		{
			wt = Lib_ChrToLon((char*)&g_Reciv_Bufer[4]);	
			if((wt > 0)&&(wt < 10000))
			{
				g_Num_MSV = wt;
				Ptl_back_MSV();
			}
			else
				g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");	
		}
		else
		{
			frameleng = Ptl_Create_Frame();
			sprintf(g_Send_Bufer+frameleng,"\r\n");
			g_Send_Num = frameleng+2;
		}
	}
}
void	Ptl_CMD_AD(void)
{
	
	INT32U		frameleng = 0;

	if(g_Reciv_Bufer[3] == '?')
	{
	
			frameleng = Ptl_Create_Frame();
			g_Send_Num = build_resp("ADV:",g_Send_Bufer,frameleng);		
	}
}
/*TAR*/
void	Ptl_CMD_TAR(void)
{
	Std_ReturnType	rtn;

	rtn = Wet_Taring();
	if(rtn == TRUE)
	{
		g_ParaUser.TAS = 0;		//net
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");	
}

/*TAV*/
void	Ptl_CMD_TAV(void)
{
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%7d\r\n",g_ParaUser.TAV);	
	}
	else
	{
		if((g_Reciv_Pt > 4)&&(g_Reciv_Pt < 13))			//参数加符号最多8个字符
		{
			tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
			if(g_ParaUser.NOV > 0)
			{
				if(tmp < g_ParaUser.NOV)
				{
					g_ParaUser.TAV = tmp;
					g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
				}
				else
					g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
			}
			else
			{
				if(tmp < 1000000)
				{
					g_ParaUser.TAV = tmp;
					g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
				}
				else
					g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
			}				
		}
	}
}
/*CDL*/
void	Ptl_CMD_CDL(void)
{
	Std_ReturnType	rtn;

	rtn = Wet_Zeroing();
	if(rtn == TRUE)
	{
		g_ParaUser.TAS = 1;
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");			
}
/*Special commands/functions (TDD, RES, DPW, SPW, IDN, ASS)******************************/
/*TDD*/
void	Ptl_CMD_TDD(void)
{
	Std_ReturnType	rtn;
	INT8U AddrChr[2+1];
	INT32U baudRate;
	INT8U wordLength;
	INT8U parityBit;

	if((g_Reciv_Bufer[3] == '0')&&(g_Flg_Unlocked == TRUE))	//Restoring the factory setting
	{
		AddrChr[0] = g_ParaUser.AddrChr[0];
		AddrChr[1] = g_ParaUser.AddrChr[1];
		baudRate = g_ParaUser.baudRate;
		wordLength = g_ParaUser.wordLength;
		parityBit = g_ParaUser.parityBit;
	memcpy((INT8U*)&g_ParaUser,(INT8U*)&g_Default_UserPara, sizeof(PARA_USER));
		g_ParaUser.AddrChr[0] = AddrChr[0];
		g_ParaUser.AddrChr[1] = AddrChr[1];
		g_ParaUser.baudRate = baudRate;
		g_ParaUser.wordLength = wordLength;
		g_ParaUser.parityBit = parityBit;
	Wet_InitPara();
		rtn = Para_Save_block(EE_ADDR_USER,(INT8U*)&g_ParaUser,sizeof(PARA_USER));
		if(rtn == TRUE)
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	}
	else if(g_Reciv_Bufer[3] == '1')		//Saving current parameters to the EEPROM
	{
		rtn = Para_Save_block(EE_ADDR_USER,(INT8U*)&g_ParaUser,sizeof(PARA_USER));
		if(rtn == TRUE)
		{
			Wet_InitPara();
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	}
	else if(g_Reciv_Bufer[3] == '2')		//Loading parameters from EEPROM to RAM
	{
		rtn = Para_Read_block(EE_ADDR_USER,(INT8U*)&g_ParaUser,sizeof(PARA_USER));
		if(rtn == TRUE)
		{
			Wet_InitPara();
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n"); 
	}
}

/*RES*/
void	Ptl_CMD_RES(void)
{
	SoftReset();
}
/*SPW*/
void	Ptl_CMD_SPW(void)
{
	INT32U	i;
	INT32U	unequ1=0,unequ2=0;

	if((g_Reciv_Pt >= 5)&&(g_Reciv_Bufer[3] == '"'))
	{
		for(i=0;i<7;i++)
		{
			if(AmdPassWord[i] == '\0')
				break;
			if(g_Reciv_Bufer[i+4] != AmdPassWord[i])
				unequ1++;			
		}
		for(i=0;i<7;i++)
		{
			if(g_ParaUser.password[i] == '\0')
				break;
			if(g_Reciv_Bufer[i+4] != g_ParaUser.password[i])
				unequ2++;			
		}
		if((unequ1==0)||(unequ2==0))
		{
			g_Flg_Unlocked = TRUE;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");			
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
}
/*DPW*/
void	Ptl_CMD_DPW(void)
{
	INT32U	i;

	if(g_Reciv_Bufer[3] == '?')
	{
		i = sprintf(g_Send_Bufer,"%s",(char *)g_ParaUser.password);
		i += sprintf(g_Send_Bufer+i,"\r\n");
		g_Send_Num = i;
	}
	else if((g_Reciv_Pt >= 5)&&(g_Reciv_Bufer[3] == '"'))
	{
		for(i=0;i<7;i++)
		{
			if(g_Reciv_Bufer[i+4] != '"')
				g_ParaUser.password[i] = g_Reciv_Bufer[i+4];
			else
				for(;i<7;i++)g_ParaUser.password[i] = '\0';			
		}
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");			
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
}
/*IDN*/
void	Ptl_CMD_IDN(void)
{
	INT8U	serNumber[7+1];		//
	INT32U	i;

	if(g_Reciv_Bufer[3] == '?')
	{
		Para_Read_block(EE_ABOUT_SN,serNumber,8);

		sprintf(g_Send_Bufer,"%3s,",CompanyName);
	   	if(g_ParaUser.devicename[0] == ' ')
			memcpy(g_Send_Bufer+4,DeviceName,15);			
		else
			memcpy(g_Send_Bufer+4,g_ParaUser.devicename,15);	
		g_Send_Bufer[19] = ',';
		//memcpy(g_Send_Bufer+20,g_ParaFact.serialnum,7);
		memcpy(g_Send_Bufer+20,serNumber,7);
		g_Send_Bufer[27] = ',';
		sprintf(g_Send_Bufer+28,"%3s\r\n",HBMvernum);
		g_Send_Num = 33;
	}
	else if((g_Reciv_Pt >= 5)&&(g_Reciv_Bufer[3] == '"'))
	{
		if(g_Flg_Unlocked == TRUE)
		{
			for(i=0;i<15;i++)
			{
				if(g_Reciv_Bufer[i+4] != '"')
					g_ParaUser.devicename[i] = g_Reciv_Bufer[i+4];
				else
					for(;i<15;i++)g_ParaUser.devicename[i] = ' ';
			}
			g_ParaUser.devicename[15] = '\0';

			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
}

/*Error messages (ESR, AOV, SOV)*********************************************************/
/*ESR*/
void	Ptl_CMD_ESR(void)
{
}
/*Commands for legal for trade applications	(LFT, TCR, CRC)******************************/


/*Commands without function (COR, CAL, ACL, STR, ASS)************************************/

/*commands for signal processing(RIO)****************************************************/
/*RIO*/
void	Ptl_CMD_RIO(void)
{
	WET_STATE	wf;
	INT32U		stat = 3;

	if(g_Reciv_Bufer[3] == '?')
	{
		Wet_Read(&wf);
		if(wf.zero == 1)stat |= 0x0200;
		if(wf.stable == 1)stat |= 0x0400;
		if(wf.range == 1)stat |= 0x0800;
		if(g_ParaUser.TAS == 1)stat |= 0x1000;
//		if((wf.overADC == 1)||(wf.overGross == 1)||(wf.overNET == 1)||(wf.underGross == 1))
//			stat |= 0x2000;
		if(g_ParaUser.LFT > 0)
		{	if((wf.overGross == 1)||(wf.overNET == 1)||(wf.underGross == 1))
				stat |= 0x4000;}			
		g_Send_Num = sprintf(g_Send_Bufer,"%5d\r\n",stat);
	}
}
/*TST*///--------------------------------------------------------------------------------------
void	Ptl_CMD_TEP(void)
{
	FP32	temp;
	if(g_Reciv_Bufer[3] == '?')
	{
		temp = TEMP_Read();
		g_Send_Num = sprintf(g_Send_Bufer,"%3.2f ℃\r\n",temp);	//A1 E6
	}
}
void	Ptl_CMD_VOL(void)
{
	FP32	vol;
	if(g_Reciv_Bufer[3] == '?')
	{
		vol = VM_Read();
		g_Send_Num = sprintf(g_Send_Bufer,"%3.3f V\r\n",vol);	//
	}
}
void	Ptl_CMD_AGL(void)
{
	FP32	vol;
	if(g_Reciv_Bufer[3] == '?')
	{
		vol = Angle_Read();
		g_Send_Num = sprintf(g_Send_Bufer,"%3.2f °\r\n",vol);	//
	}
}
void	Ptl_CMD_AGC(void)
{
	Std_ReturnType	rtn;

	rtn = Angle_CalibZero();
	if(rtn == TRUE)
	{
		rtn = Para_Save_block(EE_ADDR_OTHER,(INT8U*)&g_ParaOther,sizeof(PARA_OTHER));
		if(rtn == TRUE)
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
}
void	Ptl_CMD_COT(void)
{
	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%s %s\r\n",software_Date,software_CompileTime);	//
	}
}
void	Ptl_CMD_TSS(void)
{
INT8U	serNumber[7+1];		//

	if(g_Reciv_Bufer[3] == '?')
	{
		Para_Read_block(EE_ABOUT_SN,serNumber,8);
		g_Send_Num = sprintf(g_Send_Bufer,"%s\r\n",serNumber);	
	}
}
//重量标定相应
void Ptl_Load()
{
	
}
//零点标定响应
void Ptl_Zero()
{
	if(Wet_StartAvg(30,Ptl_back_ZER) == FALSE)
	{
		
	}
}
void	Ptl_Para_wr()
{
	if(g_Reciv_Bufer[3] == '?') //查询所有参数
	{
		int i = 4;
		strcpy(g_Send_Bufer,"PAR?");//查询参数的返回
		memcpy(g_Send_Bufer+i,&g_ParaUser.NOV,4);
		i+=4;
		g_Send_Bufer[i++] = g_ParaUser.MTD;
		g_Send_Bufer[i++] = g_ParaUser.ZTR;
		g_Send_Bufer[i++] = g_ParaUser.ZSE;
		g_Send_Bufer[i++] = g_ParaUser.RSN;
		g_Send_Bufer[i++] = g_ParaUser.DPT;	
		memcpy(g_Send_Bufer+i,g_ParaUser.ENU,4);
		i+=4;
	
		g_Send_Num = i;
	}
	else // 写入所有参数
	{
		int i = 4;
		unsigned char tmp = 0;
		g_ParaUser.NOV = *((INT32S*)(g_Reciv_Bufer+i)); //normal value.
		i+=4;
		tmp = g_Reciv_Bufer[i++];
		g_ParaUser.MTD = (tmp&0x7);
		g_ParaUser.ZTR = (tmp>>3);
		
		//g_ParaUser.MTD = g_Reciv_Bufer[i++]; //stand still monitor
		//g_ParaUser.ZTR = g_Reciv_Bufer[i++]; //zero track
		tmp = g_Reciv_Bufer[i++];
		g_ParaUser.ZSE = (tmp&0x7); //zero start up
		g_ParaUser.RSN = (tmp>>3); //resolution 
		
		g_ParaUser.DPT = g_Reciv_Bufer[i++]; //digtal point num
		memcpy(g_ParaUser.ENU,g_Reciv_Bufer+i,2);
		i+=2;
		g_ParaUser.ENU[4] = 0;
		if(Ptl_Save())
			g_Send_Num = sprintf(g_Send_Bufer,"PAR:0"); //设置参数的返回
		else 
			g_Send_Num = sprintf(g_Send_Bufer,"PAR:1"); //设置参数的返回
	}

}

void	Ptl_Para32s_wr(INT32S* para,INT32S min,INT32S max)
{
	INT32S	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%d\r\n",*para);	
	}
	else
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
		if((tmp >= min)&&(tmp <= max))
		{
			*para = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");	
	}
}
void	Ptl_Para8u_wr(INT8U* para,INT8U min,INT8U max)
{
	INT8U	tmp;

	if(g_Reciv_Bufer[3] == '?')
	{
		g_Send_Num = sprintf(g_Send_Bufer,"%d\r\n",*para);	
	}
	else
	{
		tmp = Lib_ChrToLon((char*)&g_Reciv_Bufer[3]);
		if((tmp >= min)&&(tmp <= max))
		{
			*para = tmp;
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");	
	}
}
void	Ptl_ParaChr_wr(INT8U* para,INT32U len)
{
	INT32U	i;

	if(g_Reciv_Bufer[3] == '?')
	{
		memcpy(g_Send_Bufer,para,len);
		sprintf(g_Send_Bufer+len,"\r\n");
		g_Send_Num = len + 2;
	}
	else if(g_Reciv_Bufer[3] == 0x22)
	{
		if(g_Reciv_Pt >= 4)
		{
			for(i=0;i<len;i++)
				para[i] = g_Reciv_Bufer[i+4];
			g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");			
		}
		else
			g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
	}
	else
		g_Send_Num = sprintf(g_Send_Bufer,"?\r\n");
}
/******************************************************************************
  * @brief	HBM 协议模块初始化  
  * @param  None
  * @retval TRUE：成功  FALSE：失败
******************************************************************************/
Std_ReturnType	Ptl_HBM_Init(void)
{
	return RS422_RxTimerOut_Init(100,Ptl_TimeOut);		//波特率最小1200/e，一个字节最多9.167ms
}
/******************************************************************************
  * @brief	协议超时模块  
  * @param  None
  * @retval None
******************************************************************************/
void	Ptl_TimeOut(void)
{
	g_Reciv_Pt = 0;		
}
/******************************************************************************
  * @brief  HBM 协议分析，识别一条正确的命令
  * @param  inpt:输入的一个字节数据
  * @retval none 
******************************************************************************/
void	Ptl_HBM_analysis(INT8U  inbyte)
{
INT32U	i;

	if(inbyte == ';')						//命令以';'结束
	{
		g_Reciv_Bufer[g_Reciv_Pt] = '\0';
		g_Reciv_Pt++;
		if(g_Reciv_Pt > 3)					//小于3个字节的命令丢弃
			Ptl_Process();
		g_Reciv_Pt = 0;
		for(i=0;i<NUM_RECBUF;i++)			//清空接收缓冲数据
			g_Reciv_Bufer[i] = 0;
	}
	else if(g_Reciv_Pt == 0)
	{
		if((inbyte < 'A')||(inbyte > 'z'))	//命令以字母开头
		{
			g_Reciv_Pt = 0;
			return;
		}
		else
		{
			g_Reciv_Bufer[g_Reciv_Pt] = inbyte;
			g_Reciv_Pt++;
		}
	}
	else if(g_Reciv_Pt < 3)
	{
		if((inbyte < '0')||(inbyte > 'z'))	//命令2、3字节必须是字母和数字
		{
			g_Reciv_Pt = 0;
			return;
		}
		else
		{
			g_Reciv_Bufer[g_Reciv_Pt] = inbyte;
			g_Reciv_Pt++;
		}
	}
	else if(inbyte == '\n')					//排除自己发送的响应数据
	{
		if(g_Reciv_Bufer[g_Reciv_Pt-1] == '\r')
		{
			g_Reciv_Pt = 0;
			for(i=0;i<NUM_RECBUF;i++)		//清空接收缓冲数据
				g_Reciv_Bufer[i] = 0;
		}
	}
	else
	{
		g_Reciv_Bufer[g_Reciv_Pt] = inbyte;
		g_Reciv_Pt++;
		if(g_Reciv_Pt >= NUM_RECBUF)		//如果接收缓冲溢出，只保存最新收到的数据
		{
			for(i=0;i<NUM_RECBUF-2;i++)
				g_Reciv_Bufer[i]=g_Reciv_Bufer[i+1];	
			g_Reciv_Pt -= 1;
		}
	}
}
/******************************************************************************
  * @brief  命令处理
  * @param  none
  * @retval none 
******************************************************************************/
void	Ptl_Process(void)
{
	INT32U	i;

	for(i=0;i<3;i++)		//命令部分小写字母转大写
	{
		if((g_Reciv_Bufer[i] > 0x60)&&(g_Reciv_Bufer[i] < 0x7B))
			g_Reciv_Bufer[i] -= 0x20;
	}
//选择性命令-------------------------------------------------------------------
	if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] < ':')&&(g_Reciv_Bufer[2] < ':'))
	{
		if((g_Reciv_Bufer[1] == '9')&&(g_Reciv_Bufer[2] == '8'))		//广播
		{	
			g_Flg_Work = S98;
		}
		else if((g_Reciv_Bufer[1] == g_ParaUser.AddrChr[0])&&(g_Reciv_Bufer[2] == g_ParaUser.AddrChr[1]))
		{
			if(g_Flg_Work == S98)
			{
				Ptl_Responds();
//				g_Flg_Work = Sno;				
			}
//			else //if(g_Flg_Work == Sno)
				g_Flg_Work = Sxx;			
//			return;
		}
		else
		{
			if(g_Flg_Work == Sxx)
				g_Flg_Work = Sno;	
		}
	}
//执行性命令-------------------------------------------------------------
	else
	{
		switch (g_Flg_Work)
		{
			case Sxx:
						Ptl_Executes();
						Ptl_Responds();
						break;
			case S98:
						Ptl_Executes();
						break;
			case Sno:
						break;
			default:
						break;
		}
	} 
}
/******************************************************************************
  * @brief  命令执行，
  * @param  none
  * @retval none 
******************************************************************************/
void	Ptl_Executes(void)
{
	if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'S'))Ptl_CMD_TSS();		//测试
	/*Interface settings(ADR, BDR, COF, CSM, TEX, STR)*/
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'P'))Ptl_CMD_STP();
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'R'))Ptl_CMD_ADR();
	else if((g_Reciv_Bufer[0] == 'B')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'R'))Ptl_CMD_BDR();
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'O')&&(g_Reciv_Bufer[2] == 'F'))
		Ptl_Para8u_wr(&g_ParaUser.COF,0,143);	
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'M'))	/*CSM*/
		Ptl_Para8u_wr(&g_ParaUser.CSM,0,3);
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'E')&&(g_Reciv_Bufer[2] == 'X')&&(g_Reciv_Bufer[3] == '?'))	/*TEX*/
		g_Send_Num = sprintf(g_Send_Bufer,"172\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'R')&&(g_Reciv_Bufer[3] == '?'))	/*STR*/
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");

	/*Settings for measuring mode(HSM, FMD, ASF, NTF, ICR >>> MAC)*/
	else if((g_Reciv_Bufer[0] == 'H')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'M'))	/*HSM*/
		Ptl_Para8u_wr(&g_ParaUser.HSM,0,1);	
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'M')&&(g_Reciv_Bufer[2] == 'D'))	/*FMD*/
		Ptl_Para8u_wr(&g_ParaUser.FMD,0,5);	
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'F'))	/*ASF*/
		Ptl_Para8u_wr(&g_ParaUser.ASF,0,10);
	else if((g_Reciv_Bufer[0] == 'N')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'F')&&(g_Reciv_Bufer[3] == '?'))	/*NTF*/
		g_Send_Num = sprintf(g_Send_Bufer,"00,00\r\n");
	else if((g_Reciv_Bufer[0] == 'I')&&(g_Reciv_Bufer[1] == 'C')&&(g_Reciv_Bufer[2] == 'R'))	/*ICR*/
		Ptl_Para8u_wr(&g_ParaUser.ICR,0,7);	
	else if((g_Reciv_Bufer[0] == 'M')&&(g_Reciv_Bufer[1] == 'A')&&(g_Reciv_Bufer[2] == 'C')&&(g_Reciv_Bufer[3] == '?'))	/*MAC*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");	
	/*Commands for measuring mode(MSV, TAR, TAS, TAV, CDL)*/
	else if((g_Reciv_Bufer[0] == 'M')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'V'))Ptl_CMD_MSV();
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'A')&&(g_Reciv_Bufer[2] == 'R'))Ptl_CMD_TAR();
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'A')&&(g_Reciv_Bufer[2] == 'S'))	/*TAS*/
		Ptl_Para8u_wr(&g_ParaUser.TAS,0,1);
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'A')&&(g_Reciv_Bufer[2] == 'V'))Ptl_CMD_TAV();	/*TAV*/
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'L'))Ptl_CMD_CDL();
	/*Special commands/functions(TDD, RES, SPW, IDN, ASS >>> TYP)*/
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'D'))Ptl_CMD_TDD();
	else if((g_Reciv_Bufer[0] == 'R')&&(g_Reciv_Bufer[1] == 'E')&&(g_Reciv_Bufer[2] == 'S'))Ptl_CMD_RES();
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'P')&&(g_Reciv_Bufer[2] == 'W'))Ptl_CMD_SPW();
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'P')&&(g_Reciv_Bufer[2] == 'W'))Ptl_CMD_DPW();
	else if((g_Reciv_Bufer[0] == 'I')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'N'))Ptl_CMD_IDN();
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'S'))	/*ASS*/
		Ptl_Para8u_wr(&g_ParaUser.ASS,0,3);	

	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'Y')&&(g_Reciv_Bufer[2] == 'P')&&(g_Reciv_Bufer[3] == '?'))	/*TYP*/
		g_Send_Num = sprintf(g_Send_Bufer,"000\r\n");
	/*Error messages(ESR, AOV, SOV)*/
	else if((g_Reciv_Bufer[0] == 'E')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'R'))Ptl_CMD_ESR();/*ESR*/
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'O')&&(g_Reciv_Bufer[2] == 'V')&&(g_Reciv_Bufer[3] == '?'))	/*AOV*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'O')&&(g_Reciv_Bufer[2] == 'V')&&(g_Reciv_Bufer[3] == '?'))	/*SOV*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");

	/*Commands for legal for trade applications(TCR)*/
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'T'))	/*LFT*/
		Ptl_Para8u_wr(&g_ParaUser.LFT,0,2);	
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'C')&&(g_Reciv_Bufer[2] == 'R')&&(g_Reciv_Bufer[3] == '?'))	/*TCR*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	/*Commands without function(COR, CAL, ACL, STR, ASS)*/
	
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'C')&&(g_Reciv_Bufer[3] == '?'))	/*TRC*/
		g_Send_Num = sprintf(g_Send_Bufer,"0,0, 0000000,00,00\r\n");
//	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'I')&&(g_Reciv_Bufer[2] == 'V')&&(g_Reciv_Bufer[3] == '?'))	/*LIV*/
//		g_Send_Num = sprintf(g_Send_Bufer,"0,0, 0000000,00,00\r\n");
	else if((g_Reciv_Bufer[0] == 'I')&&(g_Reciv_Bufer[1] == 'M')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*IMD*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");

	/*commands for signal processing(RIO)*/
	else if((g_Reciv_Bufer[0] == 'R')&&(g_Reciv_Bufer[1] == 'I')&&(g_Reciv_Bufer[2] == 'O'))Ptl_CMD_RIO();	/*RIO*/

	/*--------------------------------------------------*/
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'E')&&(g_Reciv_Bufer[2] == 'P'))Ptl_CMD_TEP();
	else if((g_Reciv_Bufer[0] == 'V')&&(g_Reciv_Bufer[1] == 'O')&&(g_Reciv_Bufer[2] == 'L'))Ptl_CMD_VOL();
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'G')&&(g_Reciv_Bufer[2] == 'L'))Ptl_CMD_AGL();
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'G')&&(g_Reciv_Bufer[2] == 'C'))Ptl_CMD_AGC();
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'O')&&(g_Reciv_Bufer[2] == 'T'))Ptl_CMD_COT();	
	//特殊参数设置命令
	else if((g_Reciv_Bufer[0] == 'P')&&(g_Reciv_Bufer[1] == 'A')&&(g_Reciv_Bufer[2] == 'R'))	/*CRC*/
		Ptl_Para_wr();
	
	//特殊置零命令
	else if((g_Reciv_Bufer[0] == 'Z')&&(g_Reciv_Bufer[1] == 'E')&&(g_Reciv_Bufer[2] == 'R'))	/*CRC*/
		Ptl_Zero();
	
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'O')&&(g_Reciv_Bufer[2] == 'D'))	/*CRC*/
		Ptl_Load();
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'V'))
		Ptl_CMD_AD();
/*密码保护命令******************************************************************************************/
	if((g_Flg_Unlocked == TRUE)||(g_Reciv_Bufer[3] == '?'))
	{	 
		if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'P')&&(g_Reciv_Bufer[2] == 'W'))Ptl_CMD_DPW();
		/*Factory characteristic curve(SZA, SFA)*/
		else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'Z')&&(g_Reciv_Bufer[2] == 'A'))Ptl_CMD_SZA();
		else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'A'))Ptl_CMD_SFA();
		/*Application characteristic curve and output formatting(CWT, LDW, LWT, NOV, RSN, ENU, DPT, LIC; MRA)*/
		else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'W')&&(g_Reciv_Bufer[2] == 'T'))Ptl_CMD_CWT();
		else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'W'))Ptl_CMD_LDW();
		else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'W')&&(g_Reciv_Bufer[2] == 'T'))Ptl_CMD_LWT();
		
		else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'L')&&(g_Reciv_Bufer[2] == 'Z'))Ptl_CMD_CLZ();
		else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'L')&&(g_Reciv_Bufer[2] == 'W'))Ptl_CMD_CLW();
		else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'L')&&(g_Reciv_Bufer[2] == 'K'))Ptl_CMD_CLK();
		
		else if((g_Reciv_Bufer[0] == 'N')&&(g_Reciv_Bufer[1] == 'O')&&(g_Reciv_Bufer[2] == 'V'))	/*NOV*/
			Ptl_Para32s_wr(&g_ParaUser.NOV,0,1599999);
		else if((g_Reciv_Bufer[0] == 'R')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'N'))Ptl_CMD_RSN();	/*RSN*/
		else if((g_Reciv_Bufer[0] == 'E')&&(g_Reciv_Bufer[1] == 'N')&&(g_Reciv_Bufer[2] == 'U'))	/*ENU*/
			Ptl_ParaChr_wr(g_ParaUser.ENU,4);
		else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'P')&&(g_Reciv_Bufer[2] == 'T'))	/*DPT*/
			Ptl_Para8u_wr(&g_ParaUser.DPT,0,6);
		else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'I')&&(g_Reciv_Bufer[2] == 'C'))Ptl_CMD_LIC();
		else if((g_Reciv_Bufer[0] == 'M')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'A'))	/*MRA*/
			Ptl_Para32s_wr(&g_ParaUser.MRA,0,1599999);
		/*Application characteristic curve and output formatting(GCA, GDE)*/
		else if((g_Reciv_Bufer[0] == 'G')&&(g_Reciv_Bufer[1] == 'C')&&(g_Reciv_Bufer[2] == 'A'))	/*GCA*/
			Ptl_Para32s_wr(&g_ParaUser.GCA,970000,990000);
		else if((g_Reciv_Bufer[0] == 'G')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'E'))	/*GDE*/
			Ptl_Para32s_wr(&g_ParaUser.GDE,970000,990000);		

		/*Settings for measuring mode(MTD, ZSE, ZTR >>> DZT)*/
		else if((g_Reciv_Bufer[0] == 'M')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'D'))	/*MTD*/
			Ptl_Para8u_wr(&g_ParaUser.MTD,0,5);
		else if((g_Reciv_Bufer[0] == 'Z')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'E'))	/*ZSE*/
			Ptl_Para8u_wr(&g_ParaUser.ZSE,0,4);
		else if((g_Reciv_Bufer[0] == 'Z')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'R'))	/*ZTR*/
			Ptl_Para8u_wr(&g_ParaUser.ZTR,0,4);
		else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'Z')&&(g_Reciv_Bufer[2] == 'T'))Ptl_CMD_DZT();	/*DZT*/
			
		/*Commands for legal for trade applications(LFT, CRC)*/
	//	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'T'))	/*LFT*/
	//		Ptl_Para8u_wr(&g_ParaUser.LFT,0,2);
		else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'C'))	/*CRC*/
			Ptl_Para32s_wr(&g_ParaUser.CRCh,-8388607,8388607);
		
		
	}	

/*未实现命令  ******************************************************************************************/
	if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'I')&&(g_Reciv_Bufer[2] == 'V')&&(g_Reciv_Bufer[3] == '?')&&(g_Reciv_Bufer[4] == '1'))	/*LIV*/
		g_Send_Num = sprintf(g_Send_Bufer,"1,0,0, 0000000, 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'I')&&(g_Reciv_Bufer[2] == 'V')&&(g_Reciv_Bufer[3] == '?')&&(g_Reciv_Bufer[4] == '2'))	/*LIV*/
		g_Send_Num = sprintf(g_Send_Bufer,"2,0,0, 0000000, 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'I')&&(g_Reciv_Bufer[2] == 'V')&&(g_Reciv_Bufer[3] == '?')&&(g_Reciv_Bufer[4] == '3'))	/*LIV*/
		g_Send_Num = sprintf(g_Send_Bufer,"3,0,0, 0000000, 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'I')&&(g_Reciv_Bufer[2] == 'V')&&(g_Reciv_Bufer[3] == '?')&&(g_Reciv_Bufer[4] == '4'))	/*LIV*/
		g_Send_Num = sprintf(g_Send_Bufer,"4,0,0, 0000000, 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == '1')&&(g_Reciv_Bufer[3] == '?'))	/*DT1*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == '2')&&(g_Reciv_Bufer[3] == '?'))	/*DT2*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == '3')&&(g_Reciv_Bufer[3] == '?'))	/*DT3*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == '4')&&(g_Reciv_Bufer[3] == '?'))	/*DT4*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == '1')&&(g_Reciv_Bufer[3] == '?'))	/*AT1*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == '2')&&(g_Reciv_Bufer[3] == '?'))	/*AT2*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == '3')&&(g_Reciv_Bufer[3] == '?'))	/*AT3*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == '4')&&(g_Reciv_Bufer[3] == '?'))	/*AT4*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'M')&&(g_Reciv_Bufer[3] == '?'))	/*TRM*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*TRS*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'N')&&(g_Reciv_Bufer[3] == '?'))	/*TRN*/
		g_Send_Num = sprintf(g_Send_Bufer," 00000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'F')&&(g_Reciv_Bufer[3] == '?'))	/*TRF*/
		g_Send_Num = sprintf(g_Send_Bufer,"1000000\r\n");
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*CDT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'M')&&(g_Reciv_Bufer[1] == 'V')&&(g_Reciv_Bufer[2] == 'C')&&(g_Reciv_Bufer[3] == '?'))	/*MVC*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");
	else if((g_Reciv_Bufer[0] == 'R')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'B')&&(g_Reciv_Bufer[3] == '?'))	/*RTB*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'L')&&(g_Reciv_Bufer[3] == '?'))	/*TSL*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*TST*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'V')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*TVT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'P')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*PTD*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");
	else if((g_Reciv_Bufer[0] == 'P')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'N')&&(g_Reciv_Bufer[3] == '?'))	/*PTN*/
		g_Send_Num = sprintf(g_Send_Bufer,"15\r\n");
	else if((g_Reciv_Bufer[0] == 'P')&&(g_Reciv_Bufer[1] == 'V')&&(g_Reciv_Bufer[2] == 'A')&&(g_Reciv_Bufer[3] == '?'))	/*PVA*/
		g_Send_Num = sprintf(g_Send_Bufer," 1638400,-1638400\r\n");
	else if((g_Reciv_Bufer[0] == 'P')&&(g_Reciv_Bufer[1] == 'V')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*PVS*/
		g_Send_Num = sprintf(g_Send_Bufer,"0.1\r\n");
	else if((g_Reciv_Bufer[0] == 'H')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'N')&&(g_Reciv_Bufer[3] == '?'))	/*HRN*/
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'W')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*FWT*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*CFD*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*FFD*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'L')&&(g_Reciv_Bufer[3] == '?'))	/*LTL*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'U')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'L')&&(g_Reciv_Bufer[3] == '?'))	/*UTL*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'B')&&(g_Reciv_Bufer[2] == 'K')&&(g_Reciv_Bufer[3] == '?'))	/*CBK*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'B')&&(g_Reciv_Bufer[2] == 'K')&&(g_Reciv_Bufer[3] == '?'))	/*FBK*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'Y')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*SYD*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'M')&&(g_Reciv_Bufer[3] == '?'))	/*FFM*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'E')&&(g_Reciv_Bufer[1] == 'W')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*EWT*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'M')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'W')&&(g_Reciv_Bufer[3] == '?'))	/*MSW*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'A')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*TAD*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'L')&&(g_Reciv_Bufer[3] == '?'))	/*FFL*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'C')&&(g_Reciv_Bufer[3] == '?'))	/*LTC*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'F')&&(g_Reciv_Bufer[3] == '?'))	/*LTF*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'R')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*RFT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*STT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'E')&&(g_Reciv_Bufer[1] == 'P')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*EPT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'B')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*CBT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'B')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*FBT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'M')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*MDT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'L')&&(g_Reciv_Bufer[2] == '1')&&(g_Reciv_Bufer[3] == '?'))	/*DL1*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'L')&&(g_Reciv_Bufer[2] == '2')&&(g_Reciv_Bufer[3] == '?'))	/*DL2*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'M')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*TMD*/
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	else if((g_Reciv_Bufer[0] == 'O')&&(g_Reciv_Bufer[1] == 'M')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*OMD*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");
	else if((g_Reciv_Bufer[0] == 'O')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'N')&&(g_Reciv_Bufer[3] == '?'))	/*OSN*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");
	else if((g_Reciv_Bufer[0] == 'R')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*RDS*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");
	else if((g_Reciv_Bufer[0] == 'V')&&(g_Reciv_Bufer[1] == 'C')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*VCT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'F')&&(g_Reciv_Bufer[3] == '?'))	/*SDF*/
		g_Send_Num = sprintf(g_Send_Bufer,"002\r\n");
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'M')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*DMD*/
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	else if((g_Reciv_Bufer[0] == 'E')&&(g_Reciv_Bufer[1] == 'M')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*EMD*/
		g_Send_Num = sprintf(g_Send_Bufer,"0\r\n");
	else if((g_Reciv_Bufer[0] == 'R')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'P')&&(g_Reciv_Bufer[3] == '?'))	/*RDP*/
		g_Send_Num = sprintf(g_Send_Bufer,"00\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'R')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*FRS*/
		g_Send_Num = sprintf(g_Send_Bufer,"0000\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'O')&&(g_Reciv_Bufer[3] == '?'))	/*SDO*/
		g_Send_Num = sprintf(g_Send_Bufer,"000\r\n");
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*CFD*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'D')&&(g_Reciv_Bufer[3] == '?'))	/*FFD*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'U')&&(g_Reciv_Bufer[2] == 'M')&&(g_Reciv_Bufer[3] == '?'))	/*SUM*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000000\r\n");
	else if((g_Reciv_Bufer[0] == 'N')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*NDS*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'M')&&(g_Reciv_Bufer[3] == '?'))	/*SDM*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'S')&&(g_Reciv_Bufer[1] == 'D')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*SDS*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'D')&&(g_Reciv_Bufer[1] == 'S')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*DST*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*CFT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'F')&&(g_Reciv_Bufer[2] == 'T')&&(g_Reciv_Bufer[3] == '?'))	/*FFT*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*CTS*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'C')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*TCS*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000, 0000000, 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'C')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'M')&&(g_Reciv_Bufer[3] == '?'))	/*CTM*/
		g_Send_Num = sprintf(g_Send_Bufer,"00000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'C')&&(g_Reciv_Bufer[2] == 'M')&&(g_Reciv_Bufer[3] == '?'))	/*TCM*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000, 0000000, 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'B')&&(g_Reciv_Bufer[1] == 'M')&&(g_Reciv_Bufer[2] == 'W')&&(g_Reciv_Bufer[3] == '?'))	/*BMW*/
		g_Send_Num = sprintf(g_Send_Bufer,"1\r\n");
	else if((g_Reciv_Bufer[0] == 'A')&&(g_Reciv_Bufer[1] == 'L')&&(g_Reciv_Bufer[2] == 'V')&&(g_Reciv_Bufer[3] == '?'))	/*ALV*/
		g_Send_Num = sprintf(g_Send_Bufer,"7000000\r\n");
	else if((g_Reciv_Bufer[0] == 'F')&&(g_Reciv_Bufer[1] == 'T')&&(g_Reciv_Bufer[2] == 'L')&&(g_Reciv_Bufer[3] == '?'))	/*FTL*/
		g_Send_Num = sprintf(g_Send_Bufer,"00010\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'C')&&(g_Reciv_Bufer[2] == 'Z')&&(g_Reciv_Bufer[3] == '?'))	/*TCZ*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000, 0000000, 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'T')&&(g_Reciv_Bufer[1] == 'C')&&(g_Reciv_Bufer[2] == 'C')&&(g_Reciv_Bufer[3] == '?'))	/*TCC*/
		g_Send_Num = sprintf(g_Send_Bufer," 0100000, 0000000, 0000000\r\n");
	else if((g_Reciv_Bufer[0] == 'L')&&(g_Reciv_Bufer[1] == 'I')&&(g_Reciv_Bufer[2] == 'S')&&(g_Reciv_Bufer[3] == '?'))	/*LIS*/
		g_Send_Num = sprintf(g_Send_Bufer," 0000000, 0100000, 0000000, 0000000\r\n");
}
/******************************************************************************
  * @brief  命令响应，发送数据
  * @param  none
  * @retval none 
******************************************************************************/
void	Ptl_Responds(void)
{
	INT32U	i=0;
//
//	while ((g_Send_Bufer[i] != 0)&&(i<NUM_SENDBUF))
//		i++;
	Inter_Send((INT8U*)g_Send_Bufer,g_Send_Num);
	for(i=0;i<NUM_SENDBUF;i++)			//清空发送缓冲数据
		g_Send_Bufer[i] = 0;
	g_Send_Num = 0;
}


