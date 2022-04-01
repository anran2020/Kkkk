/**
 ******************************************************************************
 * 文件:
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1         2014-12-01     zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#if (1)

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "mlos.h"
#include "ntp.h"
#include "ntp_w5500.h"
//#include "ntp_w5100.h"


//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void ntp_init(mlu8 *myIp,mlu8 *netmask,mlu8*gateway,mlu8* mac)
{

	//默认参数
	mlu8 defaultGateway[4] =
	{ 192, 168, 0, 1 };
	mlu8 defaultNetmask[4] =
	{ 255, 255, 255, 0 };
	mlu8 defaultMac[6] =
	{ 0x04, 0x08, 0xdc, 0x11, 0x11, 0x11 };
	mlu8 defaultMyip[4] =
	{ 192, 168, 0, 3 };

	mlu32* pMyip;
	pMyip=(mlu32*)myIp;

	if((*pMyip)==0xFFFFFFFF)
	{
		//没有配置，启用默认参数
		w5500_init (defaultMyip, defaultNetmask, defaultGateway, defaultMac);
	}
	else
	{
		//网卡初始化
		//初始化网络模块
		w5500_init (myIp, netmask, gateway, mac);

	};
	
}


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif


