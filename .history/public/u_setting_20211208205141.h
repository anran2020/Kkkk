/**
 ******************************************************************************
 * 文件:  u_setting.h
 * 作者:
 * 版本:   V0.01
 * 日期:
 * 内容简介:     设备运行环境，设备配置，参数
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

#ifndef _U_SETTING_H__
#define _U_SETTING_H__

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include "mlos.h"
//#include "u_led.h"
//#include "ntp.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

//配置存储，数据flash
//
#define SETTING_FLASH_ADDR (0x08000000)
#define SETTING_FLASH_SIZE (1024 * 8) // 8k
#define SETTING_FLASH_BLOCK_NUM (128)
#define SETTING_FLASH_BLOCK_SIZE (64) // 64B

//分128页，每页大小64byte;			配置文件，按页操作

#define SETTING_PAGE_SIZE SETTING_FLASH_BLOCK_SIZE
#define SETTING_PAGE_NUM SETTING_FLASH_BLOCK_NUM

//----------------------------------------------------------------------------
// 网络 配置页
//----------------------------------------------------------------------------
typedef union
{

	mlu8 page1Data[SETTING_PAGE_SIZE]; //页大小64B
	struct
	{
		mlu8 magicNumber[4]; //识别配置文件存在的魔术数字
		mlu8 myip[4];
		mlu8 remoteip[4];
		mlu8 netmask[4];
		mlu8 gateway[4];
		mlu16 myPort;
		mlu16 remotePort;
		mlu8 mac[6];   // mac地址
		mlu16 plcPort; // plc的ip信息
		mlu8 plcip[4];
		mlu32 pag1Rsv[7];
	};

} SettingPage1, SettingHead, NetworkConfigurationPage, NetCfgPage,
	*SettingHeadPtr, *NetCfgPagePtr;

//----------------------------------------------------------------------------
//   数据类型
//----------------------------------------------------------------------------
//#pragma anon_unions
typedef struct
{
	// 配置文件大小，8k，分128page，每页64Byte，
	// page 1
	mlu8 magicNumber[4]; //识别配置文件存在的魔术数字
	mlu8 myip[4];
	mlu8 remoteip[4];
	mlu8 netmask[4];
	mlu8 gateway[4];
	mlu16 myPort;
	mlu16 remotePort;
	mlu8 mac[6];   // mac地址
	mlu16 plcPort; // plc的ip信息
	mlu8 plcip[4];
	mlu16 plc2Port; // plc的ip信息
	mlu8 plc2ip[4];

	mlu8 pag1u8Rsv[2]; //预留后续扩展使用，4字节对齐
	mlu32 pag1u32Rsv[5]; //预留后续扩展使用
	
	// page2,64-u8，16-u32
	mlu8 version;		//配置文件的版本号，后续更改配置文件，做版本兼容
	mlu8 appType; 		//配置文件的应用类型，0，标识托盘应用，
	mlu8 traypwrBoxType;	   //电源箱类型，0,并联，1，串联，2，极简串联
	mlu8 trayNum;		 //托盘个数，即有效托盘数，<=4,

	mlu8 traypwrBoxNum;		 //托盘的电源箱数,
	mlu8 trayPwrBoxChnlNum;	   //电源箱通道数
	mlu8 trayChnlMdlNum;	   //电源箱通道的硬件模块数
	mlu8 trayChnlCellNum;	   //通道数的电芯数

	mlu8 pwrBoxStartAddr;  //电源箱起始箱号
	mlu8 CNA1PwrBoxNum;	   // can1连接的电源箱数,剩下的箱体连接CAN2
	mlu8 tmprBoxCellChnlNum;	/*温度板用于电芯的通道数量,不含预留*/
	mlu8 volBoxPerPwrBox;	   /*每个电源b，电压采样板数量,目前极简串联有*/

	mlu32 pwrChnlVolRange; //通道电压量程
	mlu32 pwrChnlCurRange; //通道电流量程
	
	
	mlu8 tmprBoxUartIdx;  //温度箱的串口通讯线，即uart1，uart2
	mlu8 tmprBoxNum;	   //温度箱的个数
	mlu8 tmprBoxStartAddr; //温度箱的起始箱号
	mlu8 tmprBoxChnlTotal;   //温度箱的通道总数

	mlu8 tmprChnlPerCell;  //每个电芯的温度通道数
	mlu8 tmprChnlPerLoction;//每个库位的温度通道数
	mlu8 tmprLocChnlStart;//库位的温度通道，基地址，即从哪个通道开始是库位的温度通道
	mlu8 tmprLocChnlNUm;	//库位温度通道数

	
	//plc配置
	mlu8 toolingUartIdx ;//工装的串口，即uart1，uart2
	mlu8 plcNum;		//plc的数量
	mlu8 plcMapTrayNum;			//plc映射的托盘数，即一个plc对应几个托盘
	mlu8 plcMapTrayStartAddr;	//plc对应的托盘的起始地址

	mlu32 plcReadBaseAddr[4];
	mlu32 plcReadMaxAddr;

	mlu32 endFlag1;//结束标识，增加内容需要往后移动，到最后
	mlu32 endFlag2;

	mlu32 page2U32Rsv[3];//预留后续扩展使用，4字节对齐



	// page2

	// page 128

} SystemSetting, *SystemSettingptr;

//----------------------------------------------------------------------------
// exprot variable
//----------------------------------------------------------------------------
extern SystemSettingptr mySetting;

//----------------------------------------------------------------------------
//  exproted  macro
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// exproted  fucation declare
//----------------------------------------------------------------------------

void setting_init(void);
MLBool setting_page_update(mlu8 pageIndx, mlu32 *pSettingData, mlu32 size);
MLBool setting_in_being(void);

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif
