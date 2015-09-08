/*******************************************************************************
  * @file    Pin.c 
  * @author  yangxd
  * @version V 0.01
  * @date    2013.05.27
  * @brief   引脚管理模块
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Filter.h"

/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/******************************************************************************
  * @brief  
  * @param  
  * @retval 
******************************************************************************/
INT32S	Flt_PPVAvg(INT32S data)
{
static int data_MAX,data_MIN,data_BF,data_CRU;
static int cntup = 0;
static int cntdw = 0;

        if(data >= data_MAX)
        {
                data_MAX = data;
                cntup++;
                if(cntup > 6)goto OUTFN;
                cntdw = 0;
        }
        else if(data <= data_MIN)
        {
                data_MIN = data;
                cntdw++;
                if(cntdw > 6)goto OUTFN;
                cntup = 0;
        }
        else
        {
                if((cntup > 0)||(cntdw > 0))
                {
                        data_CRU =(data_MAX + data_MIN)/2;
                        cntup = 0;
                        cntdw = 0;
                }
                if(data < data_BF)data_MIN = data;
                else if(data > data_BF)data_MAX = data;
        }
        data_BF = data;
        return data_CRU;

OUTFN:
        cntup = 0;
        cntdw = 0;
        data_MAX = data;
        data_MIN = data;
        data_CRU = data;
        return data;
}
/******************************************************************************
  * @brief  
  * @param  
  * @retval 
******************************************************************************/
#define		BUF_LENG	(INT32U)200

INT32S	Flt_MovAvg(INT32S newdat)
{
static  INT32S data_buf[BUF_LENG];
static  INT32S data_point=0;
static  INT32S data_sum;
        INT32S data_tmp;

		data_point++;
		if(data_point >= g_Para_ASF[g_ParaUser.ASF])data_point=0;
		data_sum -= data_buf[data_point];
		data_buf[data_point]= newdat;
		data_sum += data_buf[data_point];
        data_tmp = data_sum/g_Para_ASF[g_ParaUser.ASF];

        return data_tmp;
}

/******************************************************************************
  * @brief  全部用到的引脚初始化
  * @param  None
  * @retval TRUE: 初始化成功	FALSE: 初始化失败 
*****************************************************************************
#define		ORDER		(INT32U)25
#define		OUTAD		(INT32S)30		

INT32S	Flt_MovAvg(INT32S newdat)
{
static  INT32S data_buf[ORDER];
static  INT32S data_point=0;
static  INT32S data_sum;
        INT32U i;

	if(((newdat - data_sum) > OUTAD)||((data_sum - newdat) < (0 - OUTAD)))
	{
		for(i=0;i<ORDER;i++)
			data_buf[i] = newdat;
		data_sum = newdat;
	}
	else
	{
		data_point++;
		if(data_point >= ORDER)data_point=0;
		data_buf[data_point]= newdat;
		data_sum=0;
		for(i=0;i<ORDER;i++)
			data_sum += data_buf[i];
        data_sum = data_sum/ORDER;
	}
        return data_sum;
} */
/******************************************************************************
  * @brief  
  * @param  
  * @retval 
*****************************************************************************
INT32S	Flt_PPAvg(FLT_PPAVG* para,INT32S outdat,INT32S newdat)
{
	if(((newdat - para->data_CRU) > outdat)||((para->data_CRU - newdat) < (0 - outdat)))
	{
		para->data_MAX = newdat;
        para->data_MIN = newdat;
        para->data_CRU = newdat;
	}
	else
	{
		if(newdat >= para->data_MAX)
			para->data_MAX = newdat;
        else if(newdat <= para->data_MIN)
			para->data_MIN = newdat;
        else
        {
			para->data_CRU = (para->data_MAX + para->data_MIN)/2;
			if(newdat < para->data_BF)para->data_MIN = newdat;
			else if(newdat > para->data_BF)para->data_MAX = newdat;
        }
	}
	para->data_BF = newdat;
	return para->data_CRU;
}
*/

/******************************************************************************/
/* Filter Solutions Version 10.0                 Nuhertz Technologies, L.L.C. */
/*                                                            www.nuhertz.com */
/*                                                            +1 602-206-7781 */
/* 10th Order Low Pass Butterworth                                            */
/* Bilinear Transformation with Prewarping                                    */
/* Sample Frequency = 100.0 Hz                                                */
/* Standard Form                                                              */
/* Arithmetic Precision = 4 Digits                                            */
/*                                                                            */
/* Pass Band Frequency = 5.000 Rad/Sec                                        */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Input Variable Definitions:                                                */
/* Inputs:                                                                    */
/*   invar    float       The input to the filter                             */
/*   initvar  float       The initial value of the filter                     */
/*   setic    int         1 to initialize the filter to the value of initvar  */
/*                                                                            */
/* There is no requirement to ever initialize the filter.                     */
/* The default initialization is zero when the filter is first called         */
/*                                                                            */
/*****************************************************************************
float DigFil(float invar, float initval, int setic)
{
    float sumnum, sumden;  int i;
    static float delay[11] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
    static float znum[11] = {
        8.145e-17,
        8.145e-16,
        3.665e-15,
        9.774e-15,
        1.71e-14,
        2.053e-14,
        1.71e-14,
        9.774e-15,
        3.665e-15,
        8.145e-16,
        8.145e-17
    };
    static float zden[10] = {
        0.7264,
        -7.496,
        34.81,
        -95.83,
        173.1,
        -214.5,
        184.5,
        -108.9,
        42.17,
        -9.68
    };
    if (setic==1){
        for (i=0;i<=10;i++) delay[i] = 1.199e+13*initval;
        return initval;
    }
    else{
        sumden=0.0;
        sumnum=0.0;
        for (i=0;i<=9;i++){
            delay[i] = delay[i+1];
            sumden += delay[i]*zden[i];
            sumnum += delay[i]*znum[i];
        }
        delay[10] = invar-sumden;
        sumnum += delay[10]*znum[10];
        return sumnum;
    }
}
*/




