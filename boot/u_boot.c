/**
 ******************************************************************************
 * 文件:    u_boot.c
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:  
 *          
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1        2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "u_boot.h"
#include "u_led.h"
#include "ntp.h"
#include "upper_ntp.h"
#include "tool.h"
#include "mcu_flash.h"
#include "u_setting.h"
#include "tool.h"
#include "iap.h"

//----------------------------------------------------------------------------
//   define 
//---------------------------------------------------------------------------- 

//----------------------------------------------------------------------------
//global varibale 
//----------------------------------------------------------------------------

//系统应用定义
Bootloader boot;


//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void boot_init(void)
{
	__disable_irq();
	
	mcu_flash_init();
	
	setting_init();					//初始化配置文件
	
	//can 传输协议初始化
	//ctp_init(CAN_BCAST_ADDR, _500KBps_, CAN_BCAST_ADDR, _500KBps_);

	//网络传输协议初始化
	ntp_init(mySetting->myip,mySetting->netmask,mySetting->gateway,mySetting->mac);
	
	upper_ntp_init (mySetting->remoteip, mySetting->remotePort,mySetting->myPort);			//与上位机的协议实例初始化
	
	iap_init();

    led_init (100);

    tool_init ();

	__enable_irq();

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
