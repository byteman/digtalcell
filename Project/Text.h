#ifndef __TEXT_H
#define __TEXT_H

#include "common.h"

extern	const char  CompanyName[];			//公司名称 	3字节
extern	      char  DeviceName[];			//设备名称  15字节
extern	const char  HBMvernum[];			//软件版本号  3字节 
extern	const char  AmdPassWord[];			//超级密码   6字节

extern	const char  part_number[];			//P/N  物料号，11字节
extern	const char  part_name[];			//产品名称，6字节
extern	const char  software_vrm[];			//VRM  版本.发行.更改，8字节
extern		  char  software_Date[];		//软件日期，10字节   “2013-08-13”

extern		INT8U	MCU_ID[];				//MCU ID，低字节在先，	12字节
extern	const char  software_CompileDate[];	//编译日期
extern	const char	software_CompileTime[];	//编译时间

extern	Std_ReturnType	Text_Init(void);	

#endif /* __TEXT_H */
