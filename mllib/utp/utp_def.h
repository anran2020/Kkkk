/**
 ******************************************************************************
 * 文件:    utp_def.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    
 * 内容简介:     
 *                   
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                                                            创建该文件
 *
 *
 *
 ******************************************************************************
 **/
#ifndef  _UTP_DEF_H__
#define _UTP_DEF_H__

//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------
#include"mlos_dtd.h"




//----------------------------------------------------------------------------
//波特率
//----------------------------------------------------------------------------
typedef enum {


	_115200Bps_,

	
}UARTBaudRate;

//----------------------------------------------------------------------------
//utp 站
//----------------------------------------------------------------------------
 typedef enum {

	UTP_MASTER_STATION,
	UTP_SLAVER_STATION,
	
}UARTtpStation;





#endif

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

