/**
 ******************************************************************************
 * 文件:u_seeting_tool_ntp.c
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
#include"tool_config.h"
#if ((TOOL_ENABLE)&&(TOOL_SETTING_ENABLE))

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include"u_dtd.h"
#include "tool.h"
#include "tool_setting.h"
#include "u_setting.h"


//----------------------------------------------------------------------------
//define s
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//global vari
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
MLBool tool_setting_write(void*args)
{
	//升级数据协议格式 
	//len(2),addr,cmd,sn,commDevAddr,devtype,subAddr,startPageIndx(2),pageNum(2),DATA(n),seed,sum
	mlu8 sn,err;
	mlu16 len;
	mlu32 i,pageIndx,pageNum;
	DeviceType devtype;
	mlu8 * rxbuf,*datptr;

	rxbuf=(mlu8*)args;


	sn=rxbuf[4];
	devtype=rxbuf[6];
	
	pageIndx=rxbuf[9];
	pageIndx=(pageIndx<<8)+rxbuf[8];

	
	pageNum=rxbuf[11];
	pageNum=(pageNum<<8)+rxbuf[10];

	
	////暂时只处理中位机属性
	if(devtype!=DEV_Middle)
	{
		return mltrue;
	}

	err=0;
	datptr=rxbuf+12;
	for (i = 0; i < pageNum; ++i)
	{
		if(!setting_page_update(pageIndx, (mlu32*)datptr, SETTING_PAGE_SIZE))
		{
			err=1;
			break;
		}
		pageIndx++;
		datptr+=SETTING_PAGE_SIZE;
	}

	//
	//回复升级数据的报文格式
	//报文格式：len(2)|addr|cmd|sn|commDevAddr(1)|devType(1)|subAddrerror(2)|startPageIndx(2)|

	mlu8 txbuf[8];
	if(txbuf==nullptr)
	{
		return mltrue;
	}

	txbuf[0]=sn;
	txbuf[1]=0;
	txbuf[2]=DEV_Middle;				//devType
	txbuf[3]=0;						//subAddr
	txbuf[4]=err;						//eror
	txbuf[5]=0;
	txbuf[6]=pageIndx;			//pack indx
	txbuf[7]=(pageIndx>>8);


	tool_tx_data(TOOL_MSG_writeAttri,0,txbuf,8);

	return mltrue;

}

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
MLBool tool_setting_read(void*args)
{
	//tool read attri 协议
	//len(2),addr,cmd,sn,commDevAddr,devtype,subAddr,startPageIndx(2),pageNum(2),seed,sum
	mlu8 commDevAddr,subAddr,sn;
	mlu16 pageIndx,pageNum,len;
	DeviceType devtype;
	mlu8 * rxbuf,*txbuf;
	mlu32 flashAddr;
	
	rxbuf=(mlu8*)args;

	sn=rxbuf[4];
	commDevAddr=rxbuf[5];
	devtype=rxbuf[6];
	subAddr=rxbuf[7];
	
	pageIndx=rxbuf[9];
	pageIndx=(pageIndx<<8)+rxbuf[8];

	
	pageNum=rxbuf[11];
	pageNum=(pageNum<<8)+rxbuf[10];

	//暂时只处理中位机属性
	if(devtype!=DEV_Middle)
	{
		return mltrue;
	}

    //回复tool的报文格式：
    //len(2)|addr|cmd|sn(1),commDevAddr(1)|devType(1)|subAddr|startPageIndx(2),pageNum(2)
    // data(n),
    //|seed|sum

	len=18+pageNum*SETTING_PAGE_SIZE;
	txbuf=tool_txbuf_malloc(len);
	if(txbuf==nullptr)
	{
		return mltrue;
	}

	//包头
	txbuf[0]=0xee;
	txbuf[1]=0xee;
	txbuf[2]=0xee;
	txbuf[3]=0xee;
	
	//长度、地址、命令字
	txbuf[4]=len-6;
	txbuf[5]=((len-6)>>8);
	txbuf[6]=0;
	txbuf[7]=TOOL_MSG_readAttri;

	
	txbuf[8]=sn;
	txbuf[9]=0;
	txbuf[10]=DEV_Middle;			//devType
	txbuf[11]=0;						//
	txbuf[12]=pageIndx;				//
	txbuf[13]=(pageIndx>>8);			//
	txbuf[14]=pageNum;				//
	txbuf[15]=(pageNum>>8);			//
	flashAddr=pageIndx;
	flashAddr=flashAddr*SETTING_PAGE_SIZE+SETTING_FLASH_ADDR;
	memcpy(txbuf+16,(mlu8*)flashAddr,pageNum*SETTING_PAGE_SIZE);

	txbuf[len-2]=0;
	txbuf[len-1]=0;
	len=len-1;
	mlu16 i;
	for(i=0;i<len;i++)
	{
		txbuf[len-1]+=txbuf[6+i];
	}
	
	return mltrue;

}


//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	 :	void
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
void tool_setting_init(void )
{

	tool_consumer_register(TOOL_MSG_readAttri,tool_setting_read); //读取设备属性
	tool_consumer_register(TOOL_MSG_writeAttri,tool_setting_write);	//写设备属性

	tool_consumer_register(TOOL_MSG_writeAttri,tool_setting_write);	//重启设备功能模块

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif



