#ifndef __TEXT_H
#define __TEXT_H

#include "common.h"

extern	const char  CompanyName[];			//��˾���� 	3�ֽ�
extern	      char  DeviceName[];			//�豸����  15�ֽ�
extern	const char  HBMvernum[];			//����汾��  3�ֽ� 
extern	const char  AmdPassWord[];			//��������   6�ֽ�

extern	const char  part_number[];			//P/N  ���Ϻţ�11�ֽ�
extern	const char  part_name[];			//��Ʒ���ƣ�6�ֽ�
extern	const char  software_vrm[];			//VRM  �汾.����.���ģ�8�ֽ�
extern		  char  software_Date[];		//������ڣ�10�ֽ�   ��2013-08-13��

extern		INT8U	MCU_ID[];				//MCU ID�����ֽ����ȣ�	12�ֽ�
extern	const char  software_CompileDate[];	//��������
extern	const char	software_CompileTime[];	//����ʱ��

extern	Std_ReturnType	Text_Init(void);	

#endif /* __TEXT_H */
