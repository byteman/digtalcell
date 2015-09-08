

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AD7195_H
#define __AD7195_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{ 
	R10 = 0x00,
	R50 = 0x01, 
	R100 = 0x02,
	R200 = 0x03,
}TYPE_ADCrate;

typedef struct 
{
	INT32U	NoExcBit:1;			//没有激励电压
	INT32U	OpenBridgeBit:1;	//桥路开路
	INT32U	NoAdcBit:1;			// 1 秒钟没检查到转换完成
}AD_ERR;

typedef void (*AD_AVG_PROC)(INT32S);
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* AD7195 Register Map */
#define AD7195_REG_COMM         0 // Communications Register (WO, 8-bit) 
#define AD7195_REG_STAT         0 // Status Register         (RO, 8-bit) 
#define AD7195_REG_MODE         1 // Mode Register           (RW, 24-bit 
#define AD7195_REG_CONF         2 // Configuration Register  (RW, 24-bit)
#define AD7195_REG_DATA         3 // Data Register           (RO, 24/32-bit) 
#define AD7195_REG_ID           4 // ID Register             (RO, 8-bit) 
#define AD7195_REG_GPOCON       5 // GPOCON Register         (RW, 8-bit) 
#define AD7195_REG_OFFSET       6 // Offset Register         (RW, 24-bit 
#define AD7195_REG_FULLSCALE    7 // Full-Scale Register     (RW, 24-bit)

/* Communications Register Bit Designations (AD7195_REG_COMM) */
#define AD7195_COMM_WEN         (1 << 7)           // Write Enable. 
#define AD7195_COMM_WRITE       (0 << 6)           // Write Operation.
#define AD7195_COMM_READ        (1 << 6)           // Read Operation. 
#define AD7195_COMM_ADDR(x)     (((x) & 0x7) << 3) // Register Address. 
#define AD7195_COMM_CREAD       (1 << 2)           // Continuous Read of Data Register.

/* Status Register Bit Designations (AD7195_REG_STAT) */
#define AD7195_STAT_RDY         (1 << 7) // Ready.
#define AD7195_STAT_ERR         (1 << 6) // ADC error bit.
#define AD7195_STAT_NOREF       (1 << 5) // Error no external reference. 
#define AD7195_STAT_PARITY      (1 << 4) // Parity check of the data register. 
#define AD7195_STAT_CH2         (1 << 2) // Channel 2. 
#define AD7195_STAT_CH1         (1 << 1) // Channel 1. 
#define AD7195_STAT_CH0         (1 << 0) // Channel 0. 

/* Mode Register Bit Designations (AD7195_REG_MODE) */
#define AD7195_MODE_SEL(x)      (((unsigned long)(x) & 0x7) << 21) // Operation Mode Select.
#define AD7195_MODE_DAT_STA     ((unsigned long)1 << 20)           // Status Register transmission.
#define AD7195_MODE_CLKSRC(x)   (((unsigned long)(x) & 0x3) << 18) // Clock Source Select.
#define AD7195_MODE_SINC3       (1 << 15)                          // SINC3 Filter Select.
#define AD7195_MODE_ENPAR       (1 << 13)                          // Parity Enable.
#define AD7195_MODE_SCYCLE      (1 << 11)                          // Single cycle conversion.
#define AD7195_MODE_REJ60       (1 << 10)                          // 50/60Hz notch filter.
#define AD7195_MODE_RATE(x)     ((x) & 0x3FF)                      // Filter Update Rate Select.

/* Mode Register: AD7195_MODE_SEL(x) options */
#define AD7195_MODE_CONT                0 // Continuous Conversion Mode.
#define AD7195_MODE_SINGLE              1 // Single Conversion Mode.
#define AD7195_MODE_IDLE                2 // Idle Mode.
#define AD7195_MODE_PWRDN               3 // Power-Down Mode.
#define AD7195_MODE_CAL_INT_ZERO        4 // Internal Zero-Scale Calibration.
#define AD7195_MODE_CAL_INT_FULL        5 // Internal Full-Scale Calibration.
#define AD7195_MODE_CAL_SYS_ZERO        6 // System Zero-Scale Calibration.
#define AD7195_MODE_CAL_SYS_FULL        7 // System Full-Scale Calibration.

/* Mode Register: AD7195_MODE_CLKSRC(x) options */
#define AD7195_CLK_EXT_MCLK1_2          0 // External crystal. The external crystal
                                          // is connected from MCLK1 to MCLK2.
#define AD7195_CLK_EXT_MCLK2            1 // External Clock applied to MCLK2 
#define AD7195_CLK_INT                  2 // Internal 4.92 MHz clock. 
                                          // Pin MCLK2 is tristated.
#define AD7195_CLK_INT_CO               3 // Internal 4.92 MHz clock. The internal
                                          // clock is available on MCLK2.

/* Configuration Register Bit Designations (AD7195_REG_CONF) */
#define AD7195_CONF_CHOP        ((unsigned long)1 << 23)            // CHOP enable.
#define AD7195_CONF_ACX		    ((unsigned long)1 << 22)            // AC excitation enable.
#define AD7195_CONF_CHAN(x)     ((unsigned long)((x) & 0x3FF) << 8) // Channel select.
#define AD7195_CONF_BURN        (1 << 7)                            // Burnout current enable.
#define AD7195_CONF_REFDET      (1 << 6)                            // Reference detect enable.
#define AD7195_CONF_BUF         (1 << 4)                            // Buffered Mode Enable.
#define AD7195_CONF_UNIPOLAR    (1 << 3)                            // Unipolar/Bipolar Enable.
#define AD7195_CONF_GAIN(x)     ((x) & 0x7)                         // Gain Select.

/* Configuration Register: AD7195_CONF_CHAN(x) options */
#define AD7195_CH_AIN1P_AIN2M      0 // AIN1(+) - AIN2(-)       
#define AD7195_CH_AIN3P_AIN4M      1 // AIN3(+) - AIN4(-)       
#define AD7195_CH_TEMP_SENSOR      2 // Temperature sensor       
#define AD7195_CH_AIN2P_AIN2M      3 // AIN2(+) - AIN2(-)       
#define AD7195_CH_AIN1P_AINCOM     4 // AIN1(+) - AINCOM       
#define AD7195_CH_AIN2P_AINCOM     5 // AIN2(+) - AINCOM       
#define AD7195_CH_AIN3P_AINCOM     6 // AIN3(+) - AINCOM       
#define AD7195_CH_AIN4P_AINCOM     7 // AIN4(+) - AINCOM

/* Configuration Register: AD7195_CONF_GAIN(x) options */
//                                             ADC Input Range (5 V Reference)
#define AD7195_CONF_GAIN_1		0 // Gain 1    +-5 V
#define AD7195_CONF_GAIN_8		3 // Gain 8    +-625 mV
#define AD7195_CONF_GAIN_16		4 // Gain 16   +-312.5 mV
#define AD7195_CONF_GAIN_32		5 // Gain 32   +-156.2 mV
#define AD7195_CONF_GAIN_64		6 // Gain 64   +-78.125 mV
#define AD7195_CONF_GAIN_128	7 // Gain 128  +-39.06 mV

/* ID Register Bit Designations (AD7195_REG_ID) */
#define ID_AD7195               0xA6

/* Exported functions ------------------------------------------------------- */
extern	Std_ReturnType	AD_Init(TYPE_ADCrate rate);
extern	Std_ReturnType	AD_CheckNew(void);
extern	INT32S	AD_Read(void);
extern	void	AD_ErrRead(AD_ERR* errad);
extern	Std_ReturnType	AD_StartAvg(INT32U num,AD_AVG_PROC fn);

#endif /* __AD7195_H */

