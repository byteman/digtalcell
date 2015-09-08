
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WEIGHING_H
#define __WEIGHING_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
	INT32U			stableAD:1;		//AD�ȶ�
	INT32U			stable:1;		//�ȶ�
	INT32U			zero:1;			//��λ
	INT32U			overADC:1;		//ADC����
	INT32U			overGross:1;	//ë�س���
	INT32U			overNET:1;		//���س���
	INT32U			underGross:1;	//ë��Ƿ��
	INT32U			dnew:1;			//������
	INT32U			err:1;			//����
	INT32U			busy:1;			// æ
	INT32U			wetready:1;		//����ģ�����
	INT32U			range:1;		//����ָʾ��0=range1��1=range2
	INT32U			aztflg:1;		//�����ָʾ��1=activated 
}WET_STATE;

typedef struct 
{
	INT32U			errpara:1;				// ������������Χ
	INT32U			errAD:1;				// ADC��ʼ��ʧ��
	INT32U			errWorkTask:1;			// ��ʱ���������ʼ��ʧ��
	INT32U			errStableTask:1;		// ��ʱ�ȶ������ʼ��ʧ��
	INT32U			errZeroTrackTask:1;		// ��ʱ��������ʼ��ʧ��
}WET_ERR;

typedef void (*WET_AVG_PROC)(INT32S);
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	Wet_Init(void);
extern	Std_ReturnType	Wet_InitPara(void);
extern	INT32S			Wet_Read(WET_STATE* wt);
extern	void			Wet_ErrRead(WET_ERR* err);
extern	Std_ReturnType	Wet_StartAvg(INT32U num,WET_AVG_PROC fn);

extern	Std_ReturnType	Wet_Zeroing(void);
extern	Std_ReturnType	Wet_Taring(void);
//extern	Std_ReturnType	Wet_SetTare(INT32S tare);
//extern	INT32S			Wet_ReadTare(void);
//extern	Std_ReturnType	Wet_SetGorN(INT32S gorn);
//extern	INT32S			Wet_ReadGorN(void);

#endif /* __WEIGHING_H */
