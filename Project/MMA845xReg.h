/***********************************************************************************************\
* Freescale MMA8450Q Driver
*
* Filename: mma845x.h
*
*
* (c) Copyright 2009, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
\***********************************************************************************************/
#ifndef _MMA845XREG_H_
#define _MMA845XREG_H_

/***********************************************************************************************\
* Public macros
\***********************************************************************************************/

#define MMA845x_IIC_ADDRESS       0x38

/***********************************************************************************************
**
**  MMA845xQ Sensor Internal Registers
*/
enum
{
  MMA845x_STATUS = 0,			// 0x00
  MMA845x_OUT_X_MSB,			// 0x01
  MMA845x_OUT_X_LSB,			// 0x02
  MMA845x_OUT_Y_MSB,			// 0x03
  MMA845x_OUT_Y_LSB,			// 0x04
  MMA845x_OUT_Z_MSB,			// 0x05
  MMA845x_OUT_Z_LSB,			// 0x06
  MMA845x_NA_1,								// 0x07
  MMA845x_NA_2,								// 0x08
  MMA845x_F_SETUP,				// 0x09
  MMA845x_TRIG_CFG,				// 0x0A
  MMA845x_SYSMOD,				// 0x0B
  MMA845x_INT_SOURCE,			// 0x0C
  MMA845x_WHO_AM_I,				// 0x0D
  MMA845x_XYZ_DATA_CFG,			// 0x0E
  MMA845x_HP_FILTER_CUTOFF,		// 0x0F
  MMA845x_PL_STATUS,			// 0x10
  MMA845x_PL_CFG,				// 0x11
  MMA845x_PL_COUNT,				// 0x12
  MMA845x_PL_BF_ZCOMP,			// 0x13
  MMA845x_P_L_THS_REG,			// 0x14
  MMA845x_FF_MT_CFG,			// 0x15
  MMA845x_FF_MT_SRC,			// 0x16
  MMA845x_FF_MT_THS,			// 0x17
  MMA845x_FF_MT_COUNT,			// 0x18
  MMA845x_NA_3,								// 0x19
  MMA845x_NA_4,								// 0x1A
  MMA845x_NA_5,								// 0x1B
  MMA845x_NA_6,								// 0x1C
  MMA845x_TRANSIENT_CFG,		// 0x1D
  MMA845x_TRANSIENT_SRC,		// 0x1E
  MMA845x_TRANSIENT_THS,		// 0x1F
  MMA845x_TRANSIENT_COUNT,		// 0x20
  MMA845x_PULSE_CFG,			// 0x21
  MMA845x_PULSE_SRC,			// 0x22
  MMA845x_PULSE_THSX,			// 0x23
  MMA845x_PULSE_THSY,			// 0x24
  MMA845x_PULSE_THSZ,			// 0x25
  MMA845x_PULSE_TMLT,			// 0x26
  MMA845x_PULSE_LTCY,			// 0x27
  MMA845x_PULSE_WIND,			// 0x28
  MMA845x_ASLP_COUNT,			// 0x29
  MMA845x_CTRL_REG1,			// 0x2A
  MMA845x_CTRL_REG2,			// 0x2B
  MMA845x_CTRL_REG3,			// 0x2C
  MMA845x_CTRL_REG4,			// 0x2D
  MMA845x_CTRL_REG5,			// 0x2E
  MMA845x_OFF_X,				// 0x2F
  MMA845x_OFF_Y,				// 0x30
  MMA845x_OFF_Z,				// 0x31
};

/*
**  STATUS Registers
*/
#define STATUS_00_REG         0x00
#define STATUS_04_REG         0x04
#define STATUS_0B_REG         0x0B
//
#define ZYXOW_BIT             Bit._7
#define ZOW_BIT               Bit._6
#define YOR_BIT               Bit._5
#define XOR_BIT               Bit._4
#define ZYXDR_BIT             Bit._3
#define ZDR_BIT               Bit._2
#define YDR_BIT               Bit._1
#define XDR_BIT               Bit._0
//
#define ZYXOW_MASK            0x80
#define ZOW_MASK              0x40
#define YOR_MASK              0x20
#define XOR_MASK              0x10
#define ZYXDR_MASK            0x08
#define ZDR_MASK              0x04
#define YDR_MASK              0x02
#define XDR_MASK              0x01

/*
**  8-Bit XYZ Data Registers
*/
#define OUT_X_MSB8_REG        0x01
#define OUT_Y_MSB8_REG        0x02
#define OUT_Z_MSB8_REG        0x03

/*
**  12-bit XYZ Data Registers
*/
#define OUT_X_LSB_REG         0x05
#define OUT_X_MSB_REG         0x06
#define OUT_Y_LSB_REG         0x07
#define OUT_Y_MSB_REG         0x08
#define OUT_Z_LSB_REG         0x09
#define OUT_Z_MSB_REG         0x0A

/*
**  XYZ Delta AC Data Registers
*/
#define OUT_X_DELTA_REG       0x0C
#define OUT_Y_DELTA_REG       0x0D
#define OUT_Z_DELTA_REG       0x0E

/*
**  WHO_AM_I Device ID Register
*/
#define WHO_AM_I_REG          0x0F
#define MMA845x_ID_r10        0xC4
#define MMA845x_ID_r15        0xC6

/*
**  F_STATUS FIFO Status Register
*/
#define F_STATUS_REG          0x10
//
#define F_OVF_BIT             Bit._7
#define F_WMRK_FLAG_BIT       Bit._6
#define F_CNT5_BIT            Bit._5
#define F_CNT4_BIT            Bit._4
#define F_CNT3_BIT            Bit._3
#define F_CNT2_BIT            Bit._2
#define F_CNT1_BIT            Bit._1
#define F_CNT0_BIT            Bit._0
//
#define F_OVF_MASK            0x80
#define F_WMRK_FLAG_MASK      0x40
#define F_CNT5_MASK           0x20
#define F_CNT4_MASK           0x10
#define F_CNT3_MASK           0x08
#define F_CNT2_MASK           0x04
#define F_CNT1_MASK           0x02
#define F_CNT0_MASK           0x01
#define F_CNT_MASK            0x3F

/*
**  F_8DATA 8-Bit FIFO Data Register
*/
#define F_8DATA_REG           0x11

/*
**  F_12DATA 12-bit FIFO Data Register
*/
#define F_12DATA_REG          0x12

/*
**  F_SETUP FIFO Setup Register
*/
#define F_SETUP_REG           0x13
//
#define F_MODE1_BIT           Bit._7
#define F_MODE0_BIT           Bit._6
#define F_WMRK5_BIT           Bit._5
#define F_WMRK4_BIT           Bit._4
#define F_WMRK3_BIT           Bit._3
#define F_WMRK2_BIT           Bit._2
#define F_WMRK1_BIT           Bit._1
#define F_WMRK0_BIT           Bit._0
//
#define F_MODE1_MASK          0x80
#define F_MODE0_MASK          0x40
#define F_WMRK5_MASK          0x20
#define F_WMRK4_MASK          0x10
#define F_WMRK3_MASK          0x08
#define F_WMRK2_MASK          0x04
#define F_WMRK1_MASK          0x02
#define F_WMRK0_MASK          0x01
#define F_MODE_MASK           0xC0
#define F_WMRK_MASK           0x3F

/*
**  SYSMOD System Mode Register
*/
#define SYSMOD_REG            0x14
//
#define PERR_BIT              Bit._7
#define FGERR_BIT             Bit._6
#define SYSMOD1_BIT           Bit._1
#define SYSMOD0_BIT           Bit._0
//
#define PERR_MASK             0x80
#define FGERR_MASK            0x40
#define SYSMOD1_MASK          0x02
#define SYSMOD0_MASK          0x01
#define SYSMOD_MASK           0x03

/*
**  INT_SOURCE System Interrupt Status Register
*/
#define INT_SOURCE_REG        0x15
//
#define SRC_ASLP_BIT          Bit._7
#define SRC_FIFO_BIT          Bit._6
#define SRC_TRANS_BIT         Bit._5
#define SRC_LNDPRT_BIT        Bit._4
#define SRC_PULSE_BIT         Bit._3
#define SRC_FF_MT_1_BIT       Bit._2
#define SRC_FF_MT_2_BIT       Bit._1
#define SRC_DRDY_BIT          Bit._0
//
#define SRC_ASLP_MASK         0x80
#define SRC_FIFO_MASK         0x40
#define SRC_TRANS_MASK        0x20
#define SRC_LNDPRT_MASK       0x10
#define SRC_PULSE_MASK        0x08
#define SRC_FF_MT_1_MASK      0x04
#define SRC_FF_MT_2_MASK      0x02
#define SRC_DRDY_MASK         0x01

/*
**  XYZ_DATA_CFG Sensor Data Configuration Register
*/
#define XYZ_DATA_CFG_REG      0x16
//
#define FDE_BIT               Bit._7
#define DREM_BIT              Bit._3
#define ZDEFE_BIT             Bit._2
#define YDEFE_BIT             Bit._1
#define XDEFE_BIT             Bit._0
//
#define FDE_MASK              0x80
#define DREM_MASK             0x08
#define ZDEFE_MASK            0x04
#define YDEFE_MASK            0x02
#define XDEFE_MASK            0x01

/*
**  HP_FILTER_CUTOFF High Pass Filter Register
*/
#define HP_FILTER_CUTOFF_REG  0x17
//
#define SEL1_BIT              Bit._1
#define SEL0_BIT              Bit._0
//
#define SEL1_MASK             0x02
#define SEL0_MASK             0x01
#define SEL_MASK              0x03

/*
**  PL_STATUS Portrait/Landscape Status Register
**  PL_PRE_STATUS Portrait/Landscape Previous Data Status Register
*/
#define PL_STATUS_REG         0x18
#define PL_PRE_STATUS_REG     0x19
//
#define NEWLP_BIT             Bit._7
#define LO_BIT                Bit._6
#define LAPO2_BIT             Bit._4
#define LAPO1_BIT             Bit._3
#define LAPO0_BIT             Bit._2
#define BAFRO1_BIT            Bit._1
#define BAFRO0_BIT            Bit._0
//
#define NEWLP_MASK            0x80
#define LO_MASK               0x40
#define LAPO2_MASK            0x10
#define LAPO1_MASK            0x08
#define LAPO0_MASK            0x04
#define BAFRO1_MASK           0x02
#define BAFRO0_MASK           0x01
#define LAPO_MASK             0x1C
#define BAFRO_MASK            0x03

/*
**  PL_CFG Portrait/Landscape Configuration Register
*/
#define PL_CFG_REG            0x1A
//
#define DBCNTM_BIT            Bit._7
#define PL_EN_BIT             Bit._6
#define BF_EN_BIT             Bit._5
#define GOFF2_BIT             Bit._2
#define GOFF1_BIT             Bit._1
#define GOFF0_BIT             Bit._0
//
#define DBCNTM_MASK           0x80
#define PL_EN_MASK            0x40
#define BF_EN_MASK            0x20
#define GOFF2_MASK            0x04
#define GOFF1_MASK            0x02
#define GOFF0_MASK            0x01
#define GOFF_MASK             0x07

/*
**  PL_COUNT Portrait/Landscape Debounce Register
*/
#define PL_COUNT_REG          0x1B

/*
**  PL_BF_ZCOMP Back/Front and Z Compensation Register
*/
#define PL_BF_ZCOMP_REG       0x1C
//
#define BKFR1_BIT             Bit._7
#define BKFR0_BIT             Bit._6
#define ZLOCK2_BIT            Bit._2
#define ZLOCK1_BIT            Bit._1
#define ZLOCK0_BIT            Bit._0
//
#define BKFR1_MASK            0x80
#define BKFR0_MASK            0x40
#define ZLOCK2_MASK           0x04
#define ZLOCK1_MASK           0x02
#define ZLOCK0_MASK           0x01
#define BKFR_MASK             0xC0
#define ZLOCK_MASK            0x07

/*
**  PL_P_L_THS Portrait to Landscape Threshold Registers
*/
#define PL_P_L_THS_REG1       0x1D
#define PL_P_L_THS_REG2       0x1E
#define PL_P_L_THS_REG3       0x1F

/*
**  PL_L_P_THS Landscape to Portrait Threshold Registers
*/
#define PL_L_P_THS_REG1       0x20
#define PL_L_P_THS_REG2       0x21
#define PL_L_P_THS_REG3       0x22

/*
**  FF_MT_CFG Freefall and Motion Configuration Registers
*/
#define FF_MT_CFG_1_REG       0x23
#define FF_MT_CFG_2_REG       0x27
//
#define ELE_BIT               Bit._7
#define OAE_BIT               Bit._6
#define ZHEFE_BIT             Bit._5
#define ZLEFE_BIT             Bit._4
#define YHEFE_BIT             Bit._3
#define YLEFE_BIT             Bit._2
#define XHEFE_BIT             Bit._1
#define XLEFE_BIT             Bit._0
//
#define ELE_MASK              0x80
#define OAE_MASK              0x40
#define ZHEFE_MASK            0x20
#define ZLEFE_MASK            0x10
#define YHEFE_MASK            0x08
#define YLEFE_MASK            0x04
#define XHEFE_MASK            0x02
#define XLEFE_MASK            0x01

/*
**  FF_MT_SRC Freefall and Motion Source Registers
*/
#define FF_MT_SRC_1_REG       0x24
#define FF_MT_SRC_2_REG       0x28
//
#define EA_BIT                Bit._6
#define ZHE_BIT               Bit._5
#define ZLE_BIT               Bit._4
#define YHE_BIT               Bit._3
#define YLE_BIT               Bit._2
#define XHE_BIT               Bit._1
#define XLE_BIT               Bit._0
//
#define EA_MASK               0x40
#define ZHE_MASK              0x20
#define ZLE_MASK              0x10
#define YHE_MASK              0x08
#define YLE_MASK              0x04
#define XHE_MASK              0x02
#define XLE_MASK              0x01

/*
**  FF_MT_THS Freefall and Motion Threshold Registers
**  TRANSIENT_THS Transient Threshold Register
*/
#define FT_MT_THS_1_REG       0x25
#define FT_MT_THS_2_REG       0x29
#define TRANSIENT_THS_REG     0x2D
//
#define DBCNTM_BIT            Bit._7
#define THS6_BIT              Bit._6
#define THS5_BIT              Bit._5
#define THS4_BIT              Bit._4
#define THS3_BIT              Bit._3
#define THS2_BIT              Bit._2
#define THS1_BIT              Bit._1
#define THS0_BIT              Bit._0
//
#define DBCNTM_MASK           0x80
#define THS6_MASK             0x40
#define THS5_MASK             0x20
#define THS4_MASK             0x10
#define THS3_MASK             0x08
#define THS2_MASK             0x04
#define TXS1_MASK             0x02
#define THS0_MASK             0x01
#define THS_MASK              0x7F

/*
**  FF_MT_COUNT Freefall Motion Count Registers
*/
#define FF_MT_COUNT_1_REG     0x26
#define FF_MT_COUNT_2_REG     0x2A

/*
**  TRANSIENT_CFG Transient Configuration Register
*/
#define TRANSIENT_CFG_REG     0x2B
//
#define TELE_BIT              Bit._3
#define ZTEFE_BIT             Bit._2
#define YTEFE_BIT             Bit._1
#define XTEFE_BIT             Bit._0
//
#define TELE_MASK             0x08
#define ZTEFE_MASK            0x04
#define YTEFE_MASK            0x02
#define XTEFE_MASK            0x01

/*
**  TRANSIENT_SRC Transient Source Register
*/
#define TRANSIENT_SRC_REG     0x2C
//
#define TEA_BIT               Bit._3
#define ZTRANSE_BIT           Bit._2
#define YTRANSE_BIT           Bit._1
#define XTRANSE_BIT           Bit._0
//
#define TEA_MASK              0x08
#define ZTRANSE_MASK          0x04
#define YTRANSE_MASK          0x02
#define XTRANSE_MASK          0x01

/*
**  TRANSIENT_COUNT Transient Debounce Register
*/
#define TRANSIENT_COUNT_REG   0x2E

/*
**  PULSE_CFG Pulse Configuration Register
*/
#define PULSE_CFG_REG         0x2F
//
#define DPA_BIT               Bit._7
#define PELE_BIT              Bit._6
#define ZDPEFE_BIT            Bit._5
#define ZSPEFE_BIT            Bit._4
#define YDPEFE_BIT            Bit._3
#define YSPEFE_BIT            Bit._2
#define XDPEFE_BIT            Bit._1
#define XSPEFE_BIT            Bit._0
//
#define DPA_MASK              0x80
#define PELE_MASK             0x40
#define ZDPEFE_MASK           0x20
#define ZSPEFE_MASK           0x10
#define YDPEFE_MASK           0x08
#define YSPEFE_MASK           0x04
#define XDPEFE_MASK           0x02
#define XSPEFE_MASK           0x01

/*
**  PULSE_SRC Pulse Source Register
*/
#define PULSE_SRC_REG         0x30
//
#define PEA_BIT               Bit._6
#define ZDPE_BIT              Bit._5
#define ZSPE_BIT              Bit._4
#define YDPE_BIT              Bit._3
#define YSPE_BIT              Bit._2
#define XDPE_BIT              Bit._1
#define XSPE_BIT              Bit._0
//
#define PEA_MASK              0x40
#define ZDPE_MASK             0x20
#define ZSPE_MASK             0x10
#define YDPE_MASK             0x08
#define YSPE_MASK             0x04
#define XDPE_MASK             0x02
#define XSPE_MASK             0x01

/*
**  PULSE_THS XYZ Pulse Threshold Registers
*/
#define PULSE_THSX_REG        0x31
#define PULSE_THSY_REG        0x32
#define PULSE_THSZ_REG        0x33
//
#define PTHS_MASK             0x1F

/*
**  PULSE_TMLT Pulse Time Window Register
*/
#define PULSE_TMLT_REG        0x34

/*
**  PULSE_LTCY Pulse Latency Timer Register
*/
#define PULSE_LTCY_REG        0x35

/*
**  PULSE_WIND Second Pulse Time Window Register
*/
#define PULSE_WIND_REG        0x36

/*
**  ASLP_COUNT Auto Sleep Inactivity Timer Register
*/
#define ASLP_COUNT_REG        0x37

/*
**  CTRL_REG1 System Control 1 Register
*/
#define CTRL_REG1             0x38
//
#define ASLP_RATE1_BIT        Bit._7
#define ASLP_RATE0_BIT        Bit._6
#define DR2_BIT               Bit._4
#define DR1_BIT               Bit._3
#define DR0_BIT               Bit._2
#define FS1_BIT               Bit._1
#define FS0_BIT               Bit._0
//
#define ASLP_RATE1_MASK       0x80
#define ASLP_RATE0_MASK       0x40
#define DR2_MASK              0x10
#define DR1_MASK              0x08
#define DR0_MASK              0x04
#define FS1_MASK              0x02
#define FS0_MASK              0x01
#define ASLP_RATE_MASK        0xC0
#define DR_MASK               0x1C
#define FS_MASK               0x03
//
#define ASLP_RATE_20MS        0x00
#define ASLP_RATE_40MS        ASLP_RATE0_MASK
#define ASLP_RATE_80MS        ASLP_RATE1_MASK
#define ASLP_RATE_640MS       ASLP_RATE1_MASK+ASLP_RATE0_MASK
//
#define DATA_RATE_2500US      0x00
#define DATA_RATE_5MS         DR0_MASK
#define DATA_RATE_10MS        DR1_MASK
#define DATA_RATE_20MS        DR1_MASK+DR0_MASK
#define DATA_RATE_80MS        DR2_MASK
#define DATA_RATE_640MS       DR2_MASK+DR0_MASK
//
#define FULL_SCALE_STANDBY    0x00
#define FULL_SCALE_2G         FS0_MASK
#define FULL_SCALE_4G         FS1_MASK
#define FULL_SCALE_8G         FS1_MASK+FS0_MASK

/*
**  CTRL_REG2 System Control 2 Register
*/
#define CTRL_REG2             0x39
//
#define ST_BIT                Bit._7
#define BOOT_BIT              Bit._6
#define SLPE_BIT              Bit._1
#define MODS_BIT              Bit._0
//
#define ST_MASK               0x80
#define BOOT_MASK             0x40
#define SLPE_MASK             0x02
#define MODS_MASK             0x01

/*
**  CTRL_REG3 Interrupt Control Register
*/
#define CTRL_REG3             0x3A
//
#define FIFO_GATE_BIT         Bit._7
#define WAKE_TRANS_BIT        Bit._6
#define WAKE_LNDPRT_BIT       Bit._5
#define WAKE_PULSE_BIT        Bit._4
#define WAKE_FF_MT_1_BIT      Bit._3
#define WAKE_FF_MT_2_BIT      Bit._2
#define IPOL_BIT              Bit._1
#define PP_OD_BIT             Bit._0
//
#define FIFO_GATE_MASK        0x80
#define WAKE_TRANS_MASK       0x40
#define WAKE_LNDPRT_MASK      0x20
#define WAKE_PULSE_MASK       0x10
#define WAKE_FF_MT_1_MASK     0x08
#define WAKE_FF_MT_2_MASK     0x04
#define IPOL_MASK             0x02
#define PP_OD_MASK            0x01

/*
**  CTRL_REG4 Interrupt Enable Register
*/
#define CTRL_REG4             0x3B
//
#define INT_EN_ASLP_BIT       Bit._7
#define INT_EN_FIFO_BIT       Bit._6
#define INT_EN_TRANS_BIT      Bit._5
#define INT_EN_LNDPRT_BIT     Bit._4
#define INT_EN_PULSE_BIT      Bit._3
#define INT_EN_FF_MT_1_BIT    Bit._2
#define INT_EN_FF_MT_2_BIT    Bit._1
#define INT_EN_DRDY_BIT       Bit._0
//
#define INT_EN_ASLP_MASK      0x80
#define INT_EN_FIFO_MASK      0x40
#define INT_EN_TRANS_MASK     0x20
#define INT_EN_LNDPRT_MASK    0x10
#define INT_EN_PULSE_MASK     0x08
#define INT_EN_FF_MT_1_MASK   0x04
#define INT_EN_FF_MT_2_MASK   0x02
#define INT_EN_DRDY_MASK      0x01

/*
**  CTRL_REG5 Interrupt Configuration Register
*/
#define CTRL_REG5             0x3C
//
#define INT_CFG_ASLP_BIT      Bit._7
#define INT_CFG_FIFO_BIT      Bit._6
#define INT_CFG_TRANS_BIT     Bit._5
#define INT_CFG_LNDPRT_BIT    Bit._4
#define INT_CFG_PULSE_BIT     Bit._3
#define INT_CFG_FF_MT_1_BIT   Bit._2
#define INT_CFG_FF_MT_2_BIT   Bit._1
#define INT_CFG_DRDY_BIT      Bit._0
//
#define INT_CFG_ASLP_MASK     0x80
#define INT_CFG_FIFO_MASK     0x40
#define INT_CFG_TRANS_MASK    0x20
#define INT_CFG_LNDPRT_MASK   0x10
#define INT_CFG_PULSE_MASK    0x08
#define INT_CFG_FF_MT_1_MASK  0x04
#define INT_CFG_FF_MT_2_MASK  0x02
#define INT_CFG_DRDY_MASK     0x01

/*
**  XYZ Offset Correction Registers
*/
#define OFF_X_REG             0x3D
#define OFF_Y_REG             0x3E
#define OFF_Z_REG             0x3F


#endif  /* _MMA845XREG_H_ */
