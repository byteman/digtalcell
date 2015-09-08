/*******************************************************************************
  * @file    Weighing.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.06.14
  * @brief   ���ش���ģ��
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Weighing.h"
/* Private macro -------------------------------------------------------------*/

#define		NP_LIN    	4 		//3�����ص�
/* Private variables ---------------------------------------------------------*/
//static	INT32S	g_table_RSN[7] = {1,2,5,10,20,50,100};
static	INT32S	g_table_ZSE[5] = {0,2,5,10,20};
static	FP32	g_table_ZTR[5] = {0.0,0.5,1.0,2.0,3.0};
static	FP32	g_table_MTD[6] = {0.0,0.5,1.0,2.0,4.0,6.0};

static	INT32S	g_wet_input;
//static	FP32	g_wet_process;
static	FP32	g_fwet_zero;
static	FP32	g_Stable_input;
static	FP32	g_Stable_output;

static	INT32S	g_wet_newest;
static	INT32S	g_wet_gross;
static	INT32S	g_wet_net;

static	WET_STATE	g_wet_state;
static	WET_ERR		g_err;
///* ����������� */

static		FP32	g_FactClb_k = 1.0;
static		FP32	g_UserClb_k = 1.0;

static		FP32	g_corr_link[NP_LIN];	//���Բ���ϵ�� 

static	INT32S	g_AvgNum = 0;
static	INT32S	g_AvgCount = 0;
static	INT32S	g_AvgSum = 0;
static	WET_AVG_PROC g_AvgBackFn = NULL;

	
/* Private functions ---------------------------------------------------------*/
//static	Std_ReturnType	Wet_InitPara(void);
static	void	Wet_Working(void);
static	void	Wet_Process(INT32S adcode,FP32 temp);	
static	INT32S	Wet_Linear(INT32S wt);
//static	INT32S	Wet_StableAD(INT32S wt);
static	void	Wet_StableWt(void);
static	void	Wet_ZeroJudge(FP32	wt);
static	void	Wet_ZeroPowerUp(void);
static	void	Wet_ZeroTracking(void);
static	INT32S	Wet_IncFormat(INT32S wet);

static	INT32S	Wet_ParaIsRight(INT32S para,INT32S min,INT32S max);
/******************************************************************************
  * @brief  ���س�ʼ��
  * @param  None
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ�� 
******************************************************************************/
Std_ReturnType	Wet_Init(void)
{
	Std_ReturnType rtn;
	
	rtn = Timer_Regist(LOOP,g_Para_ICR[g_ParaUser.ICR],Wet_Working);//��ʱ���ش���
		if(rtn == FALSE)g_err.errWorkTask = 1;
		else	g_err.errWorkTask = 0;
	rtn = Timer_Regist(LOOP,100,Wet_StableWt);		//��ʱ���ȴ���		  
		if(rtn == FALSE)g_err.errStableTask = 1;
		else	g_err.errStableTask = 0;
	rtn = Timer_Regist(LOOP,100,Wet_ZeroTracking);	//��ʱ������		  
		if(rtn == FALSE)g_err.errZeroTrackTask = 1;
		else	g_err.errZeroTrackTask = 0;	
	rtn = Wet_InitPara();
		if(rtn == FALSE)g_err.errpara = 1;
		else	g_err.errpara = 0;

	if(*(INT32U*)&g_err != 0)
		return FALSE;
	else	return TRUE;	 
}
/******************************************************************************
  * @brief  ���س�ʼ������
  * @param  None
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ�� 
******************************************************************************/
Std_ReturnType	Wet_InitPara(void)
{	
	INT32U	i;
	INT32S	err = 0;

	err+= Wet_ParaIsRight(g_ParaFact.SZA,-1000000,1000000);
	err+= Wet_ParaIsRight(g_ParaFact.SFA,-1600000,1600000);
	err+= Wet_ParaIsRight(g_ParaFact.SFAdigit,-1600000,1600000);
	if(err == 0)
		g_FactClb_k = (FP32)g_ParaFact.SFAdigit /(FP32)(g_ParaFact.SFA - g_ParaFact.SZA);
	else
		g_FactClb_k = 1.0;

err = 0;
	err+= Wet_ParaIsRight(g_ParaUser.LDW,-1000000,1000000);
	err+= Wet_ParaIsRight(g_ParaUser.LWT,-1600000,1600000);
	err+= Wet_ParaIsRight(g_ParaUser.CWT,-1600000,1600000);
	if(err == 0)
		g_UserClb_k = (FP32)g_ParaUser.CWT /(FP32)(g_ParaUser.LWT - g_ParaUser.LDW);
	else
		g_UserClb_k = 1.0;
err = 0;
	g_ParaUser.LICI[0] = 0;
	g_ParaUser.LICD[0] = 0;
	g_corr_link[0]     = 1.0;
	for(i=1;i<NP_LIN-1;i++)
	{
		err+= Wet_ParaIsRight(g_ParaUser.LICI[i],g_ParaUser.LICI[i-1],g_ParaUser.LICI[i+1]);
		err+= Wet_ParaIsRight(g_ParaUser.LICD[i],g_ParaUser.LICD[i-1],g_ParaUser.LICD[i+1]);
	}
	if(err == 0)
	{
		for(i=1;i<NP_LIN;i++)
			g_corr_link[i] = (FP32)(g_ParaUser.LICD[i]-g_ParaUser.LICD[i-1]) / (FP32)(g_ParaUser.LICI[i]-g_ParaUser.LICI[i-1]);
	}
	else
	{
		for(i=1;i<NP_LIN;i++)
			g_corr_link[i] = 1.0;
	}
	g_fwet_zero = 0.0;
//-----------------------------------------------------------------------------
	if(err < 0)return FALSE;
	else	return TRUE;	
}
/******************************************************************************
  * @brief  �ж����Ͳ�����ȷ��
  * @param 			 
  * @retval 0����ȷ��-1���ﵽ����ֵ��-2��������Χ
******************************************************************************/
INT32S	Wet_ParaIsRight(INT32S para,INT32S min,INT32S max)
{
	if((para < min)||(para > max))
		return	-2;
	else if((para == min)||(para == max))
		return	-1;
	else
		return	0;
}
/******************************************************************************
  * @brief  ��ȡ����
  * @param  wt������״̬��ŵĽṹ��
  * @retval ʵʱ����ֵ
******************************************************************************/
INT32S	Wet_Read(WET_STATE* wt)
{
	if(*(INT32U*)&g_err != 0)g_wet_state.err = 1;
	else	g_wet_state.err = 0;

	wt->stableAD = g_wet_state.stableAD;	
	wt->stable = g_wet_state.stable;		
	wt->zero = g_wet_state.zero;		
	wt->overADC = g_wet_state.overADC;			
	wt->overGross = g_wet_state.overGross;		
	wt->overNET = g_wet_state.overNET;
	wt->underGross = g_wet_state.underGross;
	wt->busy = g_wet_state.busy;	
	wt->dnew = g_wet_state.dnew;
	wt->wetready = g_wet_state.wetready;
	wt->err = g_wet_state.err;
	wt->range = g_wet_state.range;
	wt->aztflg = g_wet_state.aztflg;		
	
	if(g_wet_state.wetready != 1)return	888888;
				
	g_wet_state.dnew = 0;	
	if(g_ParaUser.NOV > 0)
	{
		if(g_ParaUser.TAS == 0)
			return	g_wet_net;
		else if(g_ParaUser.TAS == 1)
			return	g_wet_gross; 
		else
			return	g_wet_newest;
	}
	else
	{
		if(g_ParaUser.TAS == 0)
			return	(FP32)g_wet_net * 5.12;		//AD103C���� 
		else if(g_ParaUser.TAS == 1)
			return	(FP32)g_wet_gross * 5.12;	//AD103C���� 
		else
			return	g_wet_newest;
	}									
}
/******************************************************************************
  * @brief	��ȡ Wet ģ������־  
  * @param  err�����صĽṹ��ָ��
  * @retval None
******************************************************************************/
void	Wet_ErrRead(WET_ERR* err)
{
	err->errpara = g_err.errpara;				//������������Χ
	err->errAD = g_err.errAD;					//ADC��ʼ��ʧ��
	err->errWorkTask = g_err.errWorkTask;		//��ʱ���������ʼ��ʧ��
	err->errStableTask = g_err.errStableTask;
	err->errZeroTrackTask = g_err.errZeroTrackTask;
}
/******************************************************************************
  * @brief	Wet ��ȡƽ��ֵ  
  * @param  num����ƽ��ֵ��AD����
  			fn��������ɺ��֪ͨ����
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ��
******************************************************************************/
Std_ReturnType	Wet_StartAvg(INT32U num,WET_AVG_PROC fn)
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
  * @brief  ���ط�������С�� AD ת�����ڵ�ʱ�����ڵ��øú���
  * @param  None
  * @retval None 
******************************************************************************/
void	Wet_Working(void)
{
	INT32S	adcode;
	FP32	temp;
	WAD_ERR	aderr;
	TEMP_ERR	tmperr;

	if(WAD_CheckNew() == TRUE)
	{
		adcode = WAD_Read();
		temp = TEMP_Read();
		Wet_Process(adcode,temp);
//AVG work----------------------------------------------------------------------
		if(g_AvgCount > 0)
		{
			g_AvgSum += g_wet_input;
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
	WAD_ErrRead(&aderr);
	TEMP_ErrRead(&tmperr);
	if((aderr.NoExcBit == 1)||(aderr.OpenBridgeBit == 1)||(aderr.NoAdcBit == 1)||
		(tmperr.NoPresenceBit == 1)||(tmperr.OverScopeBit == 1)||(tmperr.UnderScopeBit == 1))
	{
		g_wet_state.err = 1;
	}
	else	g_wet_state.err = 0;	
}
/******************************************************************************
  * @brief  ���ش���
  * @param 	adcode��ADת���ɼ���ԭʼ����
 			temp��ʵʱ�¶����� 			 
  * @retval None
******************************************************************************/
void	Wet_Process(INT32S adcode,FP32 temp)
{
static	INT32U	s_POdecrease = 20;		
static		INT32S	swet;
static		FP32	fwet;

	g_wet_state.busy = 1;

	if(s_POdecrease)s_POdecrease--;									//�ϵ���ȴ���20��
	else g_wet_state.wetready = 1;

	if((adcode > 4000000)||((adcode < -4000000)))					//ADC����
		g_wet_state.overADC = 1;
	else
		g_wet_state.overADC = 0; 	
	swet = (adcode - g_ParaFact.SZA)*g_FactClb_k;					//�����궨
//	g_wet_input = Wet_StableAD(swet);								//�ȶ�����
	g_wet_input = swet;
	 												 
	fwet = (FP32)(g_wet_input - g_ParaUser.LDW)*g_UserClb_k;		//�û��궨
	if(g_ParaUser.NOV > 0)
		fwet = fwet * g_ParaUser.NOV / 1000000;						//���̴��� 
	g_Stable_input = fwet - g_fwet_zero;							//������
	Wet_ZeroJudge(g_Stable_output);									//��λ�ж�
	if(g_ParaUser.ZSE > 0)Wet_ZeroPowerUp();						//��������	
	//if(g_ParaUser.ZTR > 0)Wet_ZeroTracking();						//������
	g_wet_newest = Wet_Linear(g_Stable_output); 					//���Դ���			
	g_wet_gross = Wet_IncFormat(g_wet_newest);						//ë��,�ֶ�ֵ��ʽ����������
	g_wet_net   = g_wet_gross - g_ParaUser.TAV;						//����

	if(g_wet_gross > (1000000 + 9*g_ParaUser.RSN))					//ë�س���
			g_wet_state.overGross = 1;
	else 	g_wet_state.overGross = 0;
	if(g_wet_gross < (0 - 20*g_ParaUser.RSN))						//ë��Ƿ��
			g_wet_state.underGross = 1;
	else	g_wet_state.underGross = 0;
	if(g_wet_net > (1000000 + 9*g_ParaUser.RSN))					//���س���
			g_wet_state.overNET = 1;
	else	g_wet_state.overNET = 0;
	if(g_ParaUser.MRA > 0)											//˫�����ж�
	{	if(g_wet_gross > g_ParaUser.MRA)g_wet_state.range = 1;
		else g_wet_state.range = 0;}
	else	g_wet_state.range = 0;
																			   
	g_wet_state.dnew = 1;
	g_wet_state.busy = 0;
}
/******************************************************************************
  * @brief  ��������
  * @param 	wt��ԭʼ����		 
  * @retval �����������
******************************************************************************/
INT32S	Wet_Linear(INT32S wt)
{
	INT32U	i;
	FP32	rwt;

	if(wt <= 0)
	{
		//wt = (FP32)wt * g_corr_link[1];				
		return (INT32S)wt;
	}
	for(i=1;i<NP_LIN;i++)
	{
		if((wt > g_ParaUser.LICI[i-1])&&(wt <= g_ParaUser.LICI[i]))
		{
			rwt = (FP32)(wt - g_ParaUser.LICI[i-1]) * g_corr_link[i];
			rwt = rwt + g_ParaUser.LICD[i-1];				
			return (INT32S)rwt;
		}						
	}
	if(wt > g_ParaUser.LICI[NP_LIN-1])
	{
		rwt = (FP32)(wt - g_ParaUser.LICI[NP_LIN-1]) * g_corr_link[NP_LIN-1];
		rwt = rwt + g_ParaUser.LICD[NP_LIN-1];				
		return (INT32S)rwt;
	}		
	return wt;		
}
///******************************************************************************
//  * @brief  �ж�ʾֵ�Ƿ��ȶ�
//  * @param 	wt��ԭʼ����		 
//  * @retval �ȶ�����
//******************************************************************************/
//#define		NUM_WTBUF    		25 		//2��
//#define		NUM_OUTSTABLE		1		//��ʱ
//#define		LIMIT_OUTSTABLE		12		//�仯����6������Ϊ���ȶ�
//#define		LIMIT_INSTABLE		10		//�仯��ΧС��4������Ϊ�ȶ�
//
//INT32S	Wet_StableAD(INT32S wt)
//{
//static	INT32U	s_flg_inited = 1;
//static	INT32S	s_wt_bufer[NUM_WTBUF];
//static	INT32U	s_wt_BufPtr = 0;
//		INT32S	wtMAX,wtMIN,wtAVG;
//static	INT32U	s_flg_Stable = 0;
//static	INT32S	s_wtLOCK;
//static	INT32U	s_AvgUp_counter = 0,s_AvgDown_counter = 0;
//
//	INT32U	i;
//	if(s_flg_inited)
//	{
//		for(i=0;i<NUM_WTBUF;i++)
//			s_wt_bufer[i] = wt;
//		s_flg_inited = 0;	
//	}
//	s_wt_bufer[s_wt_BufPtr] = wt; 
//	s_wt_BufPtr++;
//	if(s_wt_BufPtr >= NUM_WTBUF)s_wt_BufPtr=0;
//	wtAVG = 0;
//	for(i=0;i<NUM_WTBUF;i++)
//		wtAVG += s_wt_bufer[i];
//	wtAVG /= NUM_WTBUF;
//	wtMAX = s_wt_bufer[0];
//	for(i=0;i<NUM_WTBUF;i++)
//	{
//		if(s_wt_bufer[i] > wtMAX)wtMAX = s_wt_bufer[i];
//	}
//	wtMIN = s_wt_bufer[0];
//	for(i=0;i<NUM_WTBUF;i++)
//	{
//		if(s_wt_bufer[i] < wtMIN)wtMIN = s_wt_bufer[i];
//	}
//	if(s_flg_Stable)
//	{
//		if(wtAVG > s_wtLOCK)		
//		{
//			if((wtAVG - s_wtLOCK) > LIMIT_OUTSTABLE)
//			{
//				s_AvgDown_counter = NUM_OUTSTABLE;
//				if(s_AvgUp_counter)s_AvgUp_counter --;
//				else  s_flg_Stable = 0;  
//			}
//			else
//			{
//				s_AvgUp_counter = NUM_OUTSTABLE;
//				s_AvgDown_counter = NUM_OUTSTABLE;
//			}
//		}
//		else					
//		{
//			if((s_wtLOCK - wtAVG) > LIMIT_OUTSTABLE)
//			{
//				s_AvgUp_counter = NUM_OUTSTABLE;
//				if(s_AvgDown_counter)s_AvgDown_counter --;
//				else  s_flg_Stable = 0;
//			}
//			else
//			{
//				s_AvgUp_counter = NUM_OUTSTABLE;
//				s_AvgDown_counter = NUM_OUTSTABLE;
//			}
//		}
//	}
//	else
//	{
//		if((wtMAX - wtMIN) < LIMIT_INSTABLE)
//		{
//			s_flg_Stable = 1;
//	
//			s_wtLOCK = wtAVG;
//	
//			s_AvgUp_counter = NUM_OUTSTABLE;
//			s_AvgDown_counter = NUM_OUTSTABLE;
//		}
//	}
//	if(s_flg_Stable)
//	{
//		g_wet_state.stableAD = 1;
//		return	wtAVG;
//	}
//	else 
//	{
//		g_wet_state.stableAD = 0;
//		return	wt;
//	}
//}
/******************************************************************************
  * @brief  �ж�ʾֵ�Ƿ��ȶ�(0.1�봦��һ�Σ�������1��)
  * @input 		g_Stable_input,g_ParaUser.MTD,g_ParaUser.RSN		 
  * @output		g_wet_state.stable 				   
******************************************************************************/
#define		NUM_STBUF    		10 		//1��
#define		NUM_BREAK			1		//��ʱ

void	Wet_StableWt(void)
{
static	INT32U	s_flg_inited = 1;
static	FP32	s_wt_bufer[NUM_STBUF];
static	INT32U	s_wt_BufPtr = 0;
		FP32	wtMAX,wtMIN,wtAVG,linmit;
		INT32U	i;
static	INT32U	s_flg_Stable = 0;
static	FP32	s_wtLOCK;
static	INT32U	s_AvgUp_counter = 0,s_AvgDown_counter = 0;
//DebugPtf("g_Stable_input = %f\r\n",g_Stable_input);
	if(g_ParaUser.MTD == 0)
	{
		g_Stable_output = g_Stable_input;
		g_wet_state.stable = 1;
		return;
	}
	else if(g_ParaUser.MTD < 6)
		linmit = g_table_MTD[g_ParaUser.MTD] * (FP32)g_ParaUser.RSN;
	else	
		return;
			 
	if(s_flg_inited)
	{
		for(i=0;i<NUM_STBUF;i++)
			s_wt_bufer[i] = g_Stable_input;
		s_flg_inited = 0;	
	}

	s_wt_bufer[s_wt_BufPtr] = g_Stable_input; 
	s_wt_BufPtr++;
	if(s_wt_BufPtr >= NUM_STBUF)s_wt_BufPtr=0;

	wtMAX = s_wt_bufer[0];
	for(i=1;i<NUM_STBUF;i++)
		if(s_wt_bufer[i] > wtMAX)wtMAX = s_wt_bufer[i];
	wtMIN = s_wt_bufer[0];
	for(i=1;i<NUM_STBUF;i++)
		if(s_wt_bufer[i] < wtMIN)wtMIN = s_wt_bufer[i];

	wtAVG = 0;
	for(i=0;i<NUM_STBUF;i++)
		wtAVG += s_wt_bufer[i];
	wtAVG /= NUM_STBUF;

	if(s_flg_Stable)
	{
		if(wtAVG > s_wtLOCK)		
		{
			if((wtAVG - s_wtLOCK) > linmit)
			{
				s_AvgDown_counter = NUM_BREAK;
				if(s_AvgUp_counter)s_AvgUp_counter--;
				else  s_flg_Stable = 0;  
			}
			else
			{
				s_AvgUp_counter = NUM_BREAK;
				s_AvgDown_counter = NUM_BREAK;
			}
		}
		else					
		{
			if((s_wtLOCK - wtAVG) > linmit)
			{
				s_AvgUp_counter = NUM_BREAK;
				if(s_AvgDown_counter)s_AvgDown_counter --;
				else  s_flg_Stable = 0;
			}
			else
			{
				s_AvgUp_counter = NUM_BREAK;
				s_AvgDown_counter = NUM_BREAK;
			}
		}
	}
	else
	{
		if((wtMAX - wtMIN) < linmit)
		{
			s_flg_Stable = 1;	
			s_wtLOCK = wtAVG;	
			s_AvgUp_counter = NUM_BREAK;
			s_AvgDown_counter = NUM_BREAK;
		}
	}
	if(s_flg_Stable)
	{
		g_wet_state.stable = 1;
		g_Stable_output = wtAVG;
	}
	else 
	{
		g_wet_state.stable = 0;
		g_Stable_output = g_Stable_input;
	}	
}
/******************************************************************************
  * @brief  �ж�ʾֵ�Ƿ��ȶ�(0.1�봦��һ�Σ�������1��)
  * @input 		g_Stable_input,g_ParaUser.RSN,g_wet_state.stable		 
  * @output		g_wet_state.zero 				   
******************************************************************************/
void	Wet_ZeroJudge(FP32	wt)
{
	FP32	linmitMAX,linmitMIN;	

	if(g_wet_state.stable == 1)
	{
		linmitMAX = 0.25 * (FP32)g_ParaUser.RSN;
		linmitMIN = 0 - linmitMAX;
		if((wt > linmitMIN)&&(wt < linmitMAX))
			g_wet_state.zero = 1;
		else
			g_wet_state.zero = 0;
	}
	else
		g_wet_state.zero = 0;	
}
/******************************************************************************
  * @brief	Wet �������  
  * @param  none
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ��
******************************************************************************/
Std_ReturnType	Wet_Zeroing(void)
{
	if(g_wet_state.stable == 1)
	{
		if(g_ParaUser.NOV > 0)
		{
			if(g_wet_newest < (g_ParaUser.NOV /50 ))	//< 2%
			{	
				g_fwet_zero += g_Stable_input;
				return TRUE;
			}
			else
				return FALSE;	
		}
		else
		{
			g_fwet_zero += g_Stable_input;
			return TRUE;
		}
	}
	else
		return FALSE;
}
/******************************************************************************
  * @brief	Wet �����������  
  * @param  none
  * @retval none
******************************************************************************/
void	Wet_ZeroPowerUp(void)
{
	static	INT32S	s_pzflged = FALSE;
	
	if(s_pzflged == TRUE)return;
	else
	{
		if((g_wet_state.stable == 1)&&(g_wet_state.wetready == 1))
		{
			if(g_ParaUser.NOV > 0)
			{
				if(g_wet_newest < (g_ParaUser.NOV / 100 * g_table_ZSE[g_ParaUser.ZSE]))	
					g_fwet_zero += g_Stable_input;
				s_pzflged = TRUE;
			}
			else
			{
				if(g_wet_newest < (10000 * g_table_ZSE[g_ParaUser.ZSE]))	
					g_fwet_zero += g_Stable_input;
				s_pzflged = TRUE;
			}
		}
	} 
}
/******************************************************************************
  * @brief	Wet ������  
  * @param  none
  * @retval none
******************************************************************************/
#define		NUM_ZTBUF    		10 		//1��

void	Wet_ZeroTracking(void)
{
static	INT32U	s_flg_inited = 1;
static	FP32	s_wt_bufer[NUM_ZTBUF];
static	INT32U	s_wt_BufPtr = 0;
		FP32	wtMAX,wtMIN,linmit;			//,wtAVG;
		INT32U	i;

	if(g_ParaUser.ZTR == 0)
		goto pr_AZT_out;
	else if(g_ParaUser.ZTR < 5)
		linmit = g_table_ZTR[g_ParaUser.ZTR] * (FP32)g_ParaUser.RSN;
	else	
		goto pr_AZT_out;
			 
	if(s_flg_inited)
	{
		for(i=0;i<NUM_ZTBUF;i++)
			s_wt_bufer[i] = g_Stable_input;					//g_Stable_output;
		s_flg_inited = 0;	
	}

	s_wt_bufer[s_wt_BufPtr] = g_Stable_input;				//g_Stable_output; 
	s_wt_BufPtr++;
	if(s_wt_BufPtr >= NUM_ZTBUF)s_wt_BufPtr=0;

	wtMAX = s_wt_bufer[0];
	for(i=1;i<NUM_ZTBUF;i++)
		if(s_wt_bufer[i] > wtMAX)wtMAX = s_wt_bufer[i];
	wtMIN = s_wt_bufer[0];
	for(i=1;i<NUM_ZTBUF;i++)
		if(s_wt_bufer[i] < wtMIN)wtMIN = s_wt_bufer[i];

	if((wtMAX < linmit)&&(wtMIN > (0 - linmit)))
	{
		if(g_wet_state.stable == 1)
		{
			g_fwet_zero += g_Stable_input;
			g_wet_state.aztflg = 1;
			return;
		}
	}
pr_AZT_out:
	g_wet_state.aztflg = 0;
}
/******************************************************************************
  * @brief	Wet ȥƤ����  
  * @param  none
  * @retval TRUE: ��ʼ���ɹ�	FALSE: ��ʼ��ʧ��
******************************************************************************/
Std_ReturnType	Wet_Taring(void)
{
	if(g_wet_state.stable == 1)
	{
		if(g_ParaUser.NOV > 0)
		{
			if(g_wet_gross < g_ParaUser.NOV)	//< ������
			{	
				g_ParaUser.TAV = g_wet_gross;
				return TRUE;
			}
			else
				return FALSE;	
		}
		else
		{
			g_ParaUser.TAV = g_wet_gross;
			return TRUE;
		}
	}
	else
		return FALSE;
}
/******************************************************************************
  * @brief	Wet ���ֶ�ֵ����������������ֵ����  
  * @param  none
  * @retval none
******************************************************************************/
INT32S	Wet_IncFormat(INT32S wet)
{
	FP32 tempF;
	INT32S	curRSN;
	
	if((g_ParaUser.MRA > 0)&&(wet > g_ParaUser.MRA))
	{
		switch (g_ParaUser.RSN)
		{
			case 1:
				curRSN = 2;
				break;
			case 2:
				curRSN = 5;
				break;
			case 5:
				curRSN = 10;
				break;
			case 10:
				curRSN = 20;
				break;
			case 20:
				curRSN = 100;
				break;
			default:
				curRSN = g_ParaUser.RSN;
				break;
		}
	}
	else
		curRSN = g_ParaUser.RSN;

	tempF = (FP32)wet / curRSN;	
	if (tempF < 0)	tempF -= 0.5;
	else			tempF += 0.5;		

	return ((INT32S)tempF * curRSN);
}

