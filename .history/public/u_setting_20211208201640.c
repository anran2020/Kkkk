/**
 ******************************************************************************
 * 文件:    u_setting.c
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:  
 *          
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                       2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------

#include "hal_data.h"
#include "mlos.h"
#include "u_setting.h"
#include "tool.h"
#include "mcu_flash.h"
#include "u_dtd.h"

//----------------------------------------------------------------------------
//  config define 
//---------------------------------------------------------------------------- 



//----------------------------------------------------------------------------
//global varibale 
//----------------------------------------------------------------------------

//系统配置
SystemSettingptr mySetting=(SystemSettingptr)SETTING_FLASH_ADDR;


#if 0
void tray_setting_load(void )
{
    u8 i,addr;
    u16 u16temp;
    //判断配置文件是否存在
    if (!setting_in_being())
    {
        return ;
    }

    gTrayBoxType =mySetting->traypwrBoxType ;
    gTmpTrayAmt =mySetting->trayNum;
    gTmpTrayBoxAmt = mySetting->traypwrBoxNum;      //托盘的箱数
    gTmpBoxChnAmt = mySetting->trayPwrBoxChnlNum;   //箱的通道数
    gTmpChnCellAmt = mySetting->trayChnlCellNum;    //通道的电芯数
    gChnModuAmt = mySetting->trayChnlMdlNum;            //通道的模块数

     //gBoxAddr[32] = { 0,1,2,3,4,5,6,7 }; /*所有box的addr都必须列出*/
    //gCan0BoxIdx[32] = { 0,1,2,3 }; /*dev-box-idx*/
    u16temp=gTmpTrayAmt;
    u16temp*=gTmpTrayBoxAmt;
    addr=mySetting->pwrBoxStartAddr;
    for (i = 0; i < u16temp; i++)
    {
       gBoxAddr[i]=addr++;
    }
    for (i = 0; i < mySetting->CNA1PwrBoxNum; i++)
    {
    	gCan0BoxIdx[i]=gBoxAddr[i];
    }
   
    gVoltageMax =mySetting->pwrChnlVolRange;
    gCurrentMax = mySetting->pwrChnlCurRange;

    gTmpTmprSmplCellAmt = mySetting->tmprBoxCellChnlNum; /*温度板用于电芯的通道数量,不含预留*/
    gBoxVolSmplBoardAmt = mySetting->volBoxPerPwrBox; /*单box电压采样板数量,目前极简串联有*/    


    gTmpFixtUartIdx = mySetting->toolingUartIdx;//工装的串口号

    gTmpTmprSmplAmt = mySetting->tmprBoxNum;  /*温度采样板数量*/
   //gTmprSmplAddr[10] = { 0,1 }; //所有的addr都必须列出
   addr=mySetting->tmprBoxStartAddr;
    for (i = 0; i < mySetting->tmprBoxNum; i++)
    {
        gTmprSmplAddr[i]= addr++;
    }

    gTmpTmprSmplChnAmt = mySetting->tmprBoxChnlTotal;  /*温度盒通道总数量,含预留*/

    gTmpTmprSmplCellBase = 0;
    gTmpTmprSmplLocAmt = mySetting->tmprLocChnlNUm;  /*用于库温的通道数量,不含预留*/
    gTmpTmprSmplLocBase = mySetting->tmprLocChnlStart;
    gTmprAmtPerCell = mySetting->tmprChnlPerCell;  /*每个电芯的温度个数*/
    gTmprAmtPerSlot = mySetting->tmprChnlPerLoction;  /*每个库位的库温个数*/
    gTmpTmprSmplUartIdx = mySetting->tmprBoxUartIdx;

}
//加载配置信息
void plc_setting_load(void )
{
    u8 i,addr;
    //判断配置文件是否存在
    if (!setting_in_being())
    {
        return ;
    }
    gPlcAmt=mySetting->plcNum;
    //gPlcTrayIdx[] = { 0,1 };
    addr=mySetting->plcMapTrayStartAddr;
    for (i = 0; i < mySetting->plcMapTrayNum; i++)
    {
       gPlcTrayIdx[i]=addr++;
    }
    gPlcTrayAddrBase[0]=mySetting->plcReadStartAddr;
    gPlcTrayAddrBase[1]=mySetting->plcReadEndAddr;
    gPlcReadAmtMax=mySetting->plcReadMaxAddr;

    gPlcPort=mySetting->plcPort;

    for (i = 0; i < 4; i++)
    {
        gPlcIp[i]=mySetting->plcip[i];
    }
}

#endif
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
MLBool setting_update(mlu32* pSettingData,mlu32 size)
{

	if(mcu_flash_erase(SETTING_FLASH_ADDR, SETTING_FLASH_BLOCK_NUM))
	{
			return mcu_flash_write(SETTING_FLASH_ADDR, pSettingData, size);
	}
	
	return mlfalse;
}


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
MLBool setting_page_update(mlu8 pageIndx,mlu32* pSettingData,mlu32 size)
{
	mlu32 addr;
	addr=pageIndx;
	addr=addr*SETTING_PAGE_SIZE+SETTING_FLASH_ADDR;
	if(mcu_flash_erase(addr, 1))
	{
			return mcu_flash_write(addr, pSettingData, size);
	}
	
	return mlfalse;
}

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
MLBool setting_in_being(void)
{

	if(mySetting->endFlag1==123456&&
	mySetting->endFlag2==654321)
	{
		return mltrue;
	}


	return mlfalse;
}


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
void setting_init(void)
{
	mlu8 i;
	mlu8 setFileMagicNumber[4]={'c','n','f','g'};
	mlu8 pageData[SETTING_PAGE_SIZE];
	SettingHeadPtr headptr;
	
	mlu8 gateway[4] =
	{ 192, 168, 0, 1 };
	mlu8 netmask[4] =
	{ 255, 255, 255, 0 };
	mlu8 mac[6] =
	{ 0x04, 0x08, 0xdc, 0x11, 0x11, 0x11 };
	
	mlu8 myIp[4] =
	{ 192, 168, 0, 3 };
	//mlu16 myport = 5000;
	//tcp
	mlu8 servIp[4] =
	{ 192, 168, 0, 30 };
	mlu16 servport = 8686;
	
	//配置文件是否存在 <mSetting>
	if(memcmp(mySetting->magicNumber,setFileMagicNumber,4))
	{
		for (i = 0; i <SETTING_PAGE_SIZE; ++i)
		{
			pageData[i]=0xff;
		}
		
		headptr=(SettingHeadPtr)pageData;
		for (i = 0; i <4; ++i)
		{
			headptr->magicNumber[i]=setFileMagicNumber[i];
		}
	
		
		for (i = 0; i < 4; ++i)
		{
			headptr->myip[i]=myIp[i];
			headptr->remoteip[i]=servIp[i];
			headptr->gateway[i]=gateway[i];
			headptr->netmask[i]=netmask[i];
			headptr->mac[i]=mac[i];
		}
		headptr->mac[4]=mac[4];
		headptr->mac[5]=mac[5];
		headptr->myPort=5000;
		headptr->remotePort=6000;
		setting_page_update(0,(mlu32*)(headptr),SETTING_PAGE_SIZE);
	
	}

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------



