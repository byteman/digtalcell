
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PARAMETER_H
#define __PARAMETER_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
//typedef __packed struct
typedef struct
{
	INT8U 	serialnum[7+1];
	INT32S	SZA;
	INT32S	SFA;
	INT32S	SFAdigit;
	//INT8U AddrChr[2+1];

	INT8U 	checksum;	
}PARA_TACTORY;
//typedef __packed struct
typedef struct
{
	INT8U devicename[15+1];
	INT8U password[7+1];
	INT8U dLCAddr;
	INT8U AddrChr[2+1];		
	INT32U baudRate;
	INT8U wordLength;
	INT8U parityBit;

	INT8U	ASS;
	INT8U	HSM;
	INT8U	FMD;
	INT8U	ASF;
	INT8U	ICR;

	INT32S	NOV;
	INT32S	CWT;
	INT32S	LDW;
	INT32S	LWT;

	INT8U 	ENU[4+1];
	INT32S	MRA;
	INT32S	RSN;
	INT8U	DPT;
	INT8U	ZSE;
	INT8U	ZTR;
	INT8U	MTD;
	INT32S	CRCh;
	INT8U	LFT;
	INT32S	GCA;
	INT32S	GDE;

	INT32S	LIC[4];
	INT32S	LICI[4];
	INT32S	LICD[4];

	INT8U	TAS;
	INT32S	TAV;

	INT8U	COF;
	INT8U	CSM;

	INT8U	DZTtime;
	INT8U	DZTband;

	INT8U checksum;	
}PARA_USER;
typedef struct
{
	INT32U	key;			//
	INT32S	ang_clb_xzero;		//
	INT32S	ang_clb_yzero;		//
	INT32S	ang_clb_zzero;		//

	INT8U 	checksum;	
}PARA_OTHER;
/* Exported macro ------------------------------------------------------------*/
#define EE_BASE_ADDR    0x08080000					//     
#define EE_BYTE_SIZE    	0x0EFF					//4k - 256   
#define EE_ABOUT_SN		0x08080F08					//

#define EE_ADDR_TACTORY				EE_BASE_ADDR + 0
#define EE_ADDR_USER				EE_ADDR_TACTORY + ((sizeof(PARA_TACTORY)+3)/4*4) + 4			//所留空间圆整为4的倍数,再在两段空间空4字节			
#define EE_ADDR_OTHER				EE_ADDR_USER + ((sizeof(PARA_USER)+3)/4*4) + 4

/* Exported constants --------------------------------------------------------*/
extern	const	PARA_TACTORY	g_Default_FactPara;
extern	const	PARA_USER		g_Default_UserPara;
extern	const 	PARA_OTHER		g_Default_OtherPara;
extern			PARA_TACTORY	g_ParaFact;
extern			PARA_USER		g_ParaUser;
extern			PARA_OTHER		g_ParaOther;

extern	const	INT32S	g_Para_ASF[11];
extern	const	INT32U	g_Para_ICR[8];
/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType Para_Init(void);
extern	Std_ReturnType Para_Read_block(INT32U eeAddr,INT8U * destStr,INT32U numByte);
extern	Std_ReturnType Para_Save_block(INT32U eeAddr,INT8U * sourceStr,INT32U numByte);

#endif /* __PARAMETER_H */
