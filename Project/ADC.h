

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VMADC_H
#define __VMADC_H
/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
	INT32U	TimeouteBit:1;			//ADת����ʱ��־λ
	INT32U	OutrangeBit:1;			//ADת��������
	INT32U	ErrTimerBit:1;			//��ѹ������ʱ������
	INT32U	VolLowBit:1;			//��ѹ��

}VM_ERR;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

#define		THRESHOLD_LOW    	6.0 	//�͵�ѹ�жϷ�ֵ
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType VM_Init(void);	
extern	FP32	VM_Read(void);
extern	void	VM_ErrRead(VM_ERR* errtemp);

#endif /* __VMADC_H */
