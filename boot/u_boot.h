/**
 ******************************************************************************
 * 文件:  u_boot.h  
 * 作者:   
 * 版本:   V0.01
 * 日期:    
 * 内容简介:    
 *          
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                           创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef  _U_BOOT_H__
#define _U_BOOT_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------

#include "mlos_dtd.h"
//#include "u_led.h"
//#include "ntp.h"

//----------------------------------------------------------------------------
// define
//---------------------------------------------------------------------------- 

//版本号
#define BOOT_VERSION  	1

//----------------------------------------------------------------------------
// 系统属性
//---------------------------------------------------------------------------- 

//----------------------------------------------------------------------------
//   
//---------------------------------------------------------------------------- 
//#pragma anon_unions
typedef struct
{

    //设备地址
    mlu8 devAddr;

	//mlu32 VTOR;
	
} Bootloader;

//----------------------------------------------------------------------------
// exprot variable
//----------------------------------------------------------------------------            
extern Bootloader boot;


//----------------------------------------------------------------------------
// exprot variable
//----------------------------------------------------------------------------            

//----------------------------------------------------------------------------
//  exproted  macro
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// exproted  fucation declare
//----------------------------------------------------------------------------

void boot_init(void);

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif
