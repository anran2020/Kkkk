/**
 ******************************************************************************
 * 文件:iap_tool_ntp.c
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
#if ((TOOL_ENABLE)&&(TOOL_IAP_ENABLE))

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include"u_dtd.h"
#include "tool.h"
#include "tool_iap.h"
#include "iap.h"

#ifdef COMPILE_BOOT_CODE
#include"u_boot.h"
#else
#include"ctp.h"
#include"u_app.h"
#endif


//----------------------------------------------------------------------------
//define s
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//global vari
//----------------------------------------------------------------------------

#ifdef COMPILE_BOOT_CODE

mlu8 toolIapNtpTxbuf[2048];

#endif



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
void tool_iap_mc_conn_reply(void)
{

	mlu8 txbuf[6];

	//报文格式：len(2 dr|cmd|cnnmode(1)|commDevAddr(1)|devType(1)|subAddr|isRunningAp|versionp|rsv(4)
	txbuf[0]=TOOL_IAP_CONNECT;
	txbuf[1]=0;
	txbuf[2]=DEV_Middle;
	txbuf[3]=0;
#ifdef COMPILE_BOOT_CODE
	txbuf[4]=BOOT_RUNNING; 				//boot running
	txbuf[5]=BOOT_VERSION;
#else
	txbuf[4]=APP_RUNNING;					//app running
	txbuf[5]=APP_VERSION;				
#endif		
	tool_tx_data(TOOL_MSG_connect, 0,txbuf,6);
	

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
void tool_iap_mc_rerun_reply(mlu8 rerunFlag)
{

	mlu8 runningApp,version;

#ifdef COMPILE_BOOT_CODE							//在boot，解析指令
	if(rerunFlag==0)
	{
		//重启boot，已在boot，直接回复
		runningApp=0;
		version=BOOT_VERSION;
		iap_be_ready();
	}
	else
	{
		//重启app，跳转到app
		iap_run_app();
	}
#else											//在app，解析指令
	if(rerunFlag==0)
	{
		//重启boot，
		iap_run_boot();
		return ;
	}
	else
	{
		//重启app，已在App，直接回复
		runningApp=1;
		version=APP_VERSION;
	}

#endif

	mlu8 txbuf[12];
	//
	//回复报文格式
	//报文格式：len(2)|addr|cmd|commDevAddr(1)|isRunApp|devType(1)|subAddr|version|rsv(7)|

	txbuf[0]=0;
	txbuf[1]=runningApp;					// =0，run boot /=1，run app
	txbuf[2]=DEV_Middle;					//devType
	txbuf[3]=0;									//subAddr
	txbuf[4]=version;						//boot version		
	txbuf[5]=0;						
	txbuf[6]=0;			
	txbuf[7]=0;
	txbuf[8]=0;
	txbuf[9]=0;
	txbuf[10]=0;
	txbuf[11]=0;
	tool_tx_data(TOOL_MSG_iapRerun,0,txbuf,12);

}

//-----------------------------------------------------------------------------
// 以下代码被编译在boot里
//-----------------------------------------------------------------------------

#ifdef COMPILE_BOOT_CODE
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
MLBool tool_iap_connect(void*args)
{
	//tool 联机协议
	//len(2),addr,cmd,connMode,commDevAddr,devtype,subAddr,rsv(6),seed,sum
	mlu8 commDevAddr,subAddr;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;
	if(u8ptr[4]!=TOOL_IAP_CONNECT)
	{
		return mlfalse;
	}

	//回复联机报文格式
	//报文格式：len(2)|addr|cmd|cnnmode(1)|commDevAddr(1)|devType(1)|subAddr|version|isRunningApp|rsv(4)

	commDevAddr=u8ptr[5];
	devtype=u8ptr[6];
	subAddr=u8ptr[7];
	
	if(devtype!=DEV_Middle)					//在boot里，只接收中位机联机，不联机其它设备
	{
		return mltrue;
	}

	iap_be_ready();
	
	tool_iap_mc_conn_reply();


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
MLBool tool_iap_rerun(void*args)
{
	//tool rerun 协议
	//len(2),addr,cmd,commDevAddr,devtype,subAddr,rerunFlag(1),seed,sum
	mlu8 commDevAddr,subAddr,rerunFlag;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;


	//回复reboot报文格式
	//报文格式：len(2)|addr|cmd|commDevAddr(1)|devType(1)|subAddr|version

	commDevAddr=u8ptr[4];
	devtype=u8ptr[5];
	subAddr=u8ptr[6];
	rerunFlag=u8ptr[7];  //=0,rerun boot ;=1,rerun app;
	
	if(devtype!=DEV_Middle)			//在boot里，只接收中位机指令
	{
		return mltrue;
	}

	tool_iap_mc_rerun_reply(rerunFlag);			//回复

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
MLBool tool_iap_start(void*args)
{
	//tool iap start 协议
	//len(2),addr,cmd,SN(1),commDevAddr,devtype,subAddr,fileSize(4)seed,sum
	mlu8 commDevAddr,subAddr,sn;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;


	//回复start报文格式
	//报文格式：len(2)|addr|cmd|SN(1),commDevAddr(1)|devType(1)|subAddr|bootVersion|packSize(2)|

	sn=u8ptr[4];
	commDevAddr=u8ptr[5];
	devtype=u8ptr[6];
	subAddr=u8ptr[7];

	
	if(devtype!=DEV_Middle)				
	{
		return mltrue;
	}

	mlu32 binFileSize;
	//Little-Endian
	binFileSize=u8ptr[11];
	binFileSize=(binFileSize<<8)+u8ptr[10];
	binFileSize=(binFileSize<<8)+u8ptr[9];
	binFileSize=(binFileSize<<8)+u8ptr[8];

	IapError err;
	err=iap_start(binFileSize, IAP_TOOL_PACKET_SIZE);
	if (err!=e_iap_err_null)
	{
		
	}

	//回复报文格式
	//报文格式：len(2)|addr|cmd|sn(1),commDevAddr(1)|devType(1)|subAddr|bootVersion|rsv|packSize(2)|

	toolIapNtpTxbuf[0]=sn;
	toolIapNtpTxbuf[1]=0;
	toolIapNtpTxbuf[2]=DEV_Middle;				//devType
	toolIapNtpTxbuf[3]=0;							//subAddr
	toolIapNtpTxbuf[4]=BOOT_VERSION;				//boot version
	toolIapNtpTxbuf[5]=err;							//err
	toolIapNtpTxbuf[6]=(mlu8)IAP_TOOL_PACKET_SIZE;			//pack size
	toolIapNtpTxbuf[7]=(IAP_TOOL_PACKET_SIZE>>8);
	tool_tx_data(TOOL_MSG_iapStart,0,toolIapNtpTxbuf,8);

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
MLBool tool_iap_data(void*args)
{
	//升级数据协议格式 
	//len(2),addr,cmd,sn,commDevAddr,devtype,subAddr,packIndx(2),validDatLen(2),DATA(n),seed,sum
	mlu8 sn;
	mlu32 packetSeq,validBytes;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;


	//回复升级数据的报文格式
	//报文格式：len(2)|addr|cmd|sn|commDevAddr(1)|devType(1)|subAddr|error|packIndx|

	sn=u8ptr[4];
	devtype=u8ptr[6];
	
	packetSeq=u8ptr[9];
	packetSeq=(packetSeq<<8)+u8ptr[8];
	
	validBytes=u8ptr[11];
	validBytes=(validBytes<<8)+u8ptr[10];

	
	//网口并联电源
	if(devtype!=DEV_Middle)
	{
		return mltrue;
	}

	IapError err;
	err=iap_data(packetSeq, validBytes, u8ptr+12);

	//
	//回复报文格式
	//报文格式：len(2)|addr|cmd|sn(1),commDevAddr(1)|devType(1)|subAddr|error(2)|packIndx(2)|

	toolIapNtpTxbuf[0]=sn;
	toolIapNtpTxbuf[1]=0;
	toolIapNtpTxbuf[2]=DEV_Middle;				//devType
	toolIapNtpTxbuf[3]=0;						//subAddr
	toolIapNtpTxbuf[4]=err;						//eror
	toolIapNtpTxbuf[5]=0;
	toolIapNtpTxbuf[6]=packetSeq;			//pack indx
	toolIapNtpTxbuf[7]=(packetSeq>>8);


	tool_tx_data(TOOL_MSG_iapFileData,0,toolIapNtpTxbuf,8);

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
MLBool tool_iap_backup(void*args)
{
	//tool backup 协议
	//len(2),addr,cmd,sn,commDevAddr,devtype,subAddr,frameIndx(2),rsv(2),seed,sum
	mlu8 commDevAddr,subAddr,sn;
	mlu16 packetSeq,len;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;

	sn=u8ptr[4];
	commDevAddr=u8ptr[5];
	devtype=u8ptr[6];
	subAddr=u8ptr[7];
	
	packetSeq=u8ptr[9];
	packetSeq=(packetSeq<<8)+u8ptr[8];

	//网口并联电源
	if(devtype!=DEV_Middle)
	{
		return mltrue;
	}

    //回复tool的报文格式：
    //len(2)|addr|cmd|sn(1),commDevAddr(1)|devType(1)|subAddr|fileSize(4)|packIndx(2)|packSize|
    // data(n),
    //|seed|sum

	toolIapNtpTxbuf[0]=sn;
	toolIapNtpTxbuf[1]=0;
	toolIapNtpTxbuf[2]=DEV_Middle;		//devType
	toolIapNtpTxbuf[3]=0;		//
	toolIapNtpTxbuf[4]=appFileinfo.size;		//fileSize
	toolIapNtpTxbuf[5]=(appFileinfo.size>>8);		//
	toolIapNtpTxbuf[6]=(appFileinfo.size>>16);		//
	toolIapNtpTxbuf[7]=(appFileinfo.size>>24);		//
	toolIapNtpTxbuf[8]=packetSeq;		//packet seq
	toolIapNtpTxbuf[9]=(packetSeq>>8);		//
	toolIapNtpTxbuf[10]=(mlu8)IAP_TOOL_PACKET_SIZE;		//packet size
	toolIapNtpTxbuf[11]=(IAP_TOOL_PACKET_SIZE>>8);		//
	iap_backup(packetSeq, IAP_TOOL_PACKET_SIZE,(mlu32*)toolIapNtpTxbuf+12);
	

	tool_tx_data(TOOL_MSG_iapBackup,0,toolIapNtpTxbuf,IAP_TOOL_PACKET_SIZE+12);

	
	return mltrue;

}

//-----------------------------------------------------------------------------------------
//以下代码被编译在APP
//-----------------------------------------------------------------------------------------
#else

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
void tool_iap_connect_power(mlu8 CANDevAddr,DeviceType devt,mlu8 mainAddr,mlu8 subAddr)
{

	CANFrmptr ptxfrm;
	ptxfrm=ctp_txframe_malloc((CANCardID)CANDevAddr);
	if(ptxfrm!=nullptr)
	{
		//填充 搜索设备报文
		can_sfid_set(ptxfrm, mainAddr, CTP_MSG_search,0, subAddr, devt);
		ptxfrm->dataH=0;
		ptxfrm->dataL=0;
	}
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
MLBool tool_iap_power_conn_reply(void *args)
{

	CANFrmptr prxfrm;
	mlu8 txbuf[12];
	
	prxfrm=(CANFrmptr)args;
	if(prxfrm->sfRxIDbit.PP!=0)
	{
		//pp==0，标识IAP联机
		return mlfalse;
	}

	//
	//回复联机报文格式
	//报文格式：len(2)|addr|cmd|cnnmode(1)|commDevAddr(1)|devType(1)|subAddr|isRunningApp|version|rsv(6)
	txbuf[0]=TOOL_IAP_CONNECT;
	txbuf[1]=prxfrm->sfRxIDbit.CANID;
	
	txbuf[2]=prxfrm->sfRxIDbit.DATA10;		//devType
	txbuf[3]=prxfrm->sfRxIDbit.DATA9;		//subAddr
	txbuf[4]=prxfrm->u8Data8;					//app running
	txbuf[5]=prxfrm->u8Data7;					//appversion			
	txbuf[6]=prxfrm->u8Data6;			//cv borad num,vol board num
	txbuf[7]=prxfrm->u8Data5;
	txbuf[8]=prxfrm->u8Data4;
	txbuf[9]=prxfrm->u8Data3;
	txbuf[10]=prxfrm->u8Data2;
	txbuf[11]=prxfrm->u8Data1;
	
	tool_tx_data(TOOL_MSG_connect,prxfrm->sfRxIDbit.ADDR,txbuf,12);

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
MLBool tool_iap_connect(void*args)
{
	//tool 联机协议
	//len(2),addr,cmd,connMode,commDevAddr,devtype,subAddr,rsv(6),seed,sum
	mlu8 commDevAddr,subAddr;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;
	if(u8ptr[4]!=TOOL_IAP_CONNECT)
	{
		return mlfalse;
	}

	//回复联机报文格式
	//报文格式：len(2)|addr|cmd|cnnmode(1)|commDevAddr(1)|devType(1)|subAddr|version|isRunningApp|rsv(4)

	commDevAddr=u8ptr[5];
	devtype=u8ptr[6];
	subAddr=u8ptr[7];
	
	if(devtype==DEV_Middle)
	{
		//联机中位机
		tool_iap_mc_conn_reply();
	}
	else if(devtype==DEV_ParallPower||devtype==DEV_SerialPower||devtype==DEV_SerialPowerPlus||
		devtype==DEV_CVBoard||devtype==DEV_VolBoard)
	{
		//联机电源设备
		if(commDevAddr<CAN_NUM)
		{
			tool_iap_connect_power(commDevAddr, devtype, u8ptr[2],subAddr);
		}	
	}
	else if(devtype==DEV_TemBoard)
	{
		//联机温度板

	}
	else if(devtype==DEV_needbedCtrlBoard)
	{
		//联机针床控制板

	}
	else if(devtype==DEV_OCVBoard)
	{
		//联机ocv切换板

	}
	else if(devtype==DEV_toolBoard)
	{
		//联机工装下位机

	}
	else
	{

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
void tool_iap_power_rerun(mlu8 CANDevAddr,DeviceType devt,mlu8 mainAddr,mlu8 subAddr,mlu8 rerunflag)
{

	CANFrmptr ptxfrm;
	ptxfrm=ctp_txframe_malloc((CANCardID)CANDevAddr);
	if(ptxfrm!=nullptr)
	{
		//填充 搜索设备报文
		can_sfid_set(ptxfrm, mainAddr, CTP_MSG_iapRerun,rerunflag, subAddr, devt);
		ptxfrm->dataH=0;
		ptxfrm->dataL=0;
	}
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
MLBool tool_iap_power_rerun_reply(void *args)
{

	CANFrmptr prxfrm;
	mlu8 txbuf[12];
	
	prxfrm=(CANFrmptr)args;

	//
	//回复报文格式
	//报文格式：len(2)|addr|cmd|commDevAddr(1)|isRunApp|devType(1)|subAddr|version|rsv(7)|

	txbuf[0]=prxfrm->sfRxIDbit.CANID;
	txbuf[1]=prxfrm->sfRxIDbit.PP;		// run boot /run app
	txbuf[2]=prxfrm->sfRxIDbit.DATA10;		//devType
	txbuf[3]=prxfrm->sfRxIDbit.DATA9;		//subAddr
	txbuf[4]=prxfrm->u8Data8;				//boot version		
	txbuf[5]=prxfrm->u8Data7;						
	txbuf[6]=prxfrm->u8Data6;			
	txbuf[7]=prxfrm->u8Data5;
	txbuf[8]=prxfrm->u8Data4;
	txbuf[9]=prxfrm->u8Data3;
	txbuf[10]=prxfrm->u8Data2;
	txbuf[11]=prxfrm->u8Data1;
	
	tool_tx_data(TOOL_MSG_iapRerun,prxfrm->sfRxIDbit.ADDR,txbuf,12);

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
MLBool tool_iap_rerun(void*args)
{
	//tool rerun 协议
	//len(2),addr,cmd,commDevAddr,devtype,subAddr,rerunFlag(1),seed,sum
	mlu8 commDevAddr,subAddr,rerunFlag;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;


	//回复reboot报文格式
	//报文格式：len(2)|addr|cmd|commDevAddr(1)|devType(1)|subAddr|version

	commDevAddr=u8ptr[4];
	devtype=u8ptr[5];
	subAddr=u8ptr[6];
	rerunFlag=u8ptr[7];  //=0,rerun boot ;=1,rerun app;
	
	if(devtype==DEV_Middle)
	{
		//重启中位机boot、app
		tool_iap_mc_rerun_reply(rerunFlag); //当前代码运行在APP，

	}
	else if(devtype==DEV_ParallPower||devtype==DEV_SerialPower||devtype==DEV_SerialPowerPlus||
		devtype==DEV_CVBoard||devtype==DEV_VolBoard)
	{
		//重启电源设备boot/app
		if(commDevAddr<CAN_NUM)
		{
			tool_iap_power_rerun(commDevAddr, devtype, u8ptr[2],subAddr,rerunFlag);
		}	
	}
	else if(devtype==DEV_TemBoard)
	{
		//联机温度板

	}
	else if(devtype==DEV_needbedCtrlBoard)
	{
		//联机针床控制板

	}
	else if(devtype==DEV_OCVBoard)
	{
		//联机ocv切换板

	}
	else if(devtype==DEV_toolBoard)
	{
		//联机工装下位机

	}
	else
	{

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
void tool_iap_power_start(CANCardID ccid,mlu8 sn,mlu8 boxAddr,mlu8 devt,mlu8 subAddr,mlu8*fileSize)
{
	//pRxDat格式
	//len(2),addr,cmd,SN(1),commDevAddr,devtype,subAddr,fileSize(4)seed,sum

	mlu8 *buf;
	buf=ctp_txbuf_malloc(10);
	
	//数据内容格式	
	//SN,IDN,LEN,devtype,sunAddr,fileSize(4);
	buf[0]=sn;				//SN
	buf[1]=CTP_MSG_iapStart;		//IDN
	buf[2]=10;						//len
	buf[3]=0;
	buf[4]=devt;
	buf[5]=subAddr;
	buf[6]=fileSize[0];			//filesize
	buf[7]=fileSize[1];
	buf[8]=fileSize[2];
	buf[9]=fileSize[3];
	ctp_send(ccid, boxAddr,buf, 10);
	
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
MLBool tool_iap_power_start_reply(void *args)
{

	CtpMfMsgptr pMfmsg;
	
	mlu8 buf[8];
	
	pMfmsg=(CtpMfMsgptr)args;
	//接收报文数据域格式
	//SN,IDN,LEN,设备类型,从设备地址,BOOT软件版本,RSV(预留1),升级数据包大小(2)

	
	//
	//回复报文格式
	//报文格式：len(2)|addr|cmd|commDevAddr(1)|devType(1)|subAddr|bootVersion|sn(1)|packSize(2)|

	buf[0]=pMfmsg->buffer[0];
	buf[1]=pMfmsg->ccid;			//commDevAddr		
	buf[2]=pMfmsg->buffer[4];		//devType
	buf[3]=pMfmsg->buffer[5];		//subAddr
	buf[4]=pMfmsg->buffer[6];		//boot version		
	buf[5]=pMfmsg->buffer[7];		//rsv						
	buf[6]=pMfmsg->buffer[8];		//pack size		
	buf[7]=pMfmsg->buffer[9];


	tool_tx_data(TOOL_MSG_iapStart,pMfmsg->addr,buf,8);

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
MLBool tool_iap_tool_start(void*args)
{
	//tool iap start 协议
	//len(2),addr,cmd,SN(1),commDevAddr,devtype,subAddr,fileSize(4)seed,sum
	mlu8 commDevAddr,subAddr,sn;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;


	//回复start报文格式
	//报文格式：len(2)|addr|cmd|SN(1),commDevAddr(1)|devType(1)|subAddr|bootVersion|packSize(2)|

	sn=u8ptr[4];
	commDevAddr=u8ptr[5];
	devtype=u8ptr[6];
	subAddr=u8ptr[7];

	
	if(devtype==DEV_Middle)
	{
		//在app，不应答启动升级，需要跳转boot
    	iap_run_boot();
    	return mltrue;

	}
	else if(devtype==DEV_ParallPower||devtype==DEV_SerialPower||devtype==DEV_SerialPowerPlus||
		devtype==DEV_CVBoard||devtype==DEV_VolBoard)
	{
		//升级电源设备app
		if(commDevAddr<CAN_NUM)
		{
			//启动升级app
			tool_iap_power_start(commDevAddr,sn,u8ptr[2],devtype,subAddr,u8ptr+8);
		}	
	}
	else if(devtype==DEV_TemBoard)
	{
		//联机温度板

	}
	else if(devtype==DEV_needbedCtrlBoard)
	{
		//联机针床控制板

	}
	else if(devtype==DEV_OCVBoard)
	{
		//联机ocv切换板

	}
	else if(devtype==DEV_toolBoard)
	{
		//联机工装下位机

	}
	else
	{

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
void tool_iap_power_data(mlu8* pRxData)
{
	//升级数据协议格式 
	//len(2),addr,cmd,sn,commDevAddr,devtype,subAddr,packIndx(2),validDatLen(2),DATA(n),seed,sum
	mlu8 commDevAddr,boxAddr;
	mlu16 packSize;

	packSize=pRxData[1];
	packSize=(packSize<<8)+pRxData[0];
	packSize=packSize-12+10;

	boxAddr=pRxData[2];
	commDevAddr=pRxData[5];
	
	//报文格式
	//SN,IDN,LEN(2),设备类型,从设备地址,seq(数据包序号),有效数据长度(2),升级文件数据

	//重组数据包
	pRxData[2]=pRxData[4];
	pRxData[3]=CTP_MSG_iapData;
	pRxData[4]=packSize;
	pRxData[5]=(packSize>>8);

	ctp_send(commDevAddr, boxAddr,pRxData+2, packSize);
	
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
MLBool tool_iap_power_data_reply(void *args)
{


	CtpMfMsgptr pMfmsg;
	
	mlu8 buf[8];
	
	pMfmsg=(CtpMfMsgptr)args;
	//接收报文数据域格式
	//SN,IDN,LEN(2),设备类型,从设备地址,error(2),期待包序(2)

	
	//
	//回复报文格式
	//报文格式：len(2)|addr|cmd|sn(1),commDevAddr(1)|devType(1)|subAddr|error(2)|packIndx(2)|

	buf[0]=pMfmsg->buffer[0];
	buf[1]=pMfmsg->ccid;		
	buf[2]=pMfmsg->buffer[4];		//devType
	buf[3]=pMfmsg->buffer[5];		//subAddr
	buf[4]=pMfmsg->buffer[6];				//eror	
	buf[5]=pMfmsg->buffer[7];								
	buf[6]=pMfmsg->buffer[8];			//pack indx	
	buf[7]=pMfmsg->buffer[9];


	tool_tx_data(TOOL_MSG_iapFileData,pMfmsg->addr,buf,8);

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
MLBool tool_iap_data(void*args)
{
	//升级数据协议格式 
	//len(2),addr,cmd,sn,commDevAddr,devtype,subAddr,packIndx(2),validDatLen(2),DATA(n),seed,sum
	mlu8 commDevAddr,subAddr,rerunFlag;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;


	//回复升级数据的报文格式
	//报文格式：len(2)|addr|cmd|sn|commDevAddr(1)|devType(1)|subAddr|error|packIndx|

	commDevAddr=u8ptr[5];
	devtype=u8ptr[6];
	subAddr=u8ptr[7];

	if(devtype==DEV_Middle)
	{
		//升级 mc ,在app里面，不执行
		return mltrue;
	}
	else if(devtype==DEV_ParallPower||devtype==DEV_SerialPower||devtype==DEV_SerialPowerPlus||
		devtype==DEV_CVBoard||devtype==DEV_VolBoard)
	{
		//重启电源设备boot/app
		if(commDevAddr<CAN_NUM)
		{
			tool_iap_power_data(u8ptr);
		}	
	}
	else if(devtype==DEV_TemBoard)
	{
		//联机温度板

	}
	else if(devtype==DEV_needbedCtrlBoard)
	{
		//联机针床控制板

	}
	else if(devtype==DEV_OCVBoard)
	{
		//联机ocv切换板

	}
	else if(devtype==DEV_toolBoard)
	{
		//联机工装下位机

	}
	else
	{

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
void tool_iap_power_backup(mlu8* pRxData)
{
	//升级数据协议格式 
	//len(2),addr,cmd,sn,commDevAddr,devtype,subAddr,packIndx(2),rsv(2)，seed,sum
	mlu8 commDevAddr,boxAddr;


	boxAddr=pRxData[2];
	commDevAddr=pRxData[5];
	
	//报文格式
	//SN,IDN,LEN(2),设备类型,从设备地址,seq(2),rsv(2)

	//重组数据包
	pRxData[2]=pRxData[4];					//流水号
	pRxData[3]=CTP_MSG_iapBackup;			//idn
	pRxData[4]=10;							//len
	pRxData[5]=0;

	ctp_send(commDevAddr, boxAddr,pRxData+2, 10);
	
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
MLBool tool_iap_power_backup_reply(void *args)
{

	CtpMfMsgptr pMfmsg;
	mlu16 len;

	//多帧数据内容
	//SN,IDN,LEN(2),设备类型,从设备地址,文件大小(4),seq(2),备份数据包大小(2),备份数据(n)
	
	pMfmsg=(CtpMfMsgptr)args;
	
	len=pMfmsg->buffer[3];							//多帧数据包的长度
	len=(len<<8)+pMfmsg->buffer[2];
	

    //回复tool的报文格式：
    //len(2)|addr|cmd|sn(1),commDevAddr(1)|devType(1)|subAddr|fileSize(4)|packIndx(2)|packSize|
    // data(n),
    //|seed|sum
	pMfmsg->buffer[2]=pMfmsg->buffer[0];   //sn
	pMfmsg->buffer[3]=pMfmsg->ccid;			//commDevAddr ,通讯设备地址
	
	tool_tx_data(TOOL_MSG_iapBackup,pMfmsg->addr,pMfmsg->buffer+2,len-2);

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
MLBool tool_iap_backup(void*args)
{
	//tool backup 协议
	//len(2),addr,cmd,sn,commDevAddr,devtype,subAddr,frameIndx(2),rsv(2),seed,sum
	mlu8 commDevAddr,subAddr,rerunFlag;
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;


	//回复reboot报文格式
	//报文格式：len(2)|addr|cmd|commDevAddr(1)|devType(1)|subAddr|version

	commDevAddr=u8ptr[5];
	devtype=u8ptr[6];
	subAddr=u8ptr[7];
	
	if(devtype==DEV_Middle)
	{
		//在app，不执行备份
	}
	else if(devtype==DEV_ParallPower||devtype==DEV_SerialPower||devtype==DEV_SerialPowerPlus||
		devtype==DEV_CVBoard||devtype==DEV_VolBoard)
	{
		//备份程序
		if(commDevAddr<CAN_NUM)
		{
			tool_iap_power_backup(u8ptr);
		}	
	}
	else if(devtype==DEV_TemBoard)
	{
		//联机温度板

	}
	else if(devtype==DEV_needbedCtrlBoard)
	{
		//联机针床控制板

	}
	else if(devtype==DEV_OCVBoard)
	{
		//联机ocv切换板

	}
	else if(devtype==DEV_toolBoard)
	{
		//联机工装下位机

	}
	else
	{

	}

	return mltrue;

}
#endif
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
void tool_iap_init(void )
{

#ifdef COMPILE_BOOT_CODE
	tool_consumer_register(TOOL_MSG_connect,tool_iap_connect); //响应联机消息
	tool_consumer_register(TOOL_MSG_iapRerun,tool_iap_rerun);	//重启boot、app
	tool_consumer_register(TOOL_MSG_iapStart,tool_iap_start);	//启动升级
	tool_consumer_register(TOOL_MSG_iapFileData,tool_iap_data);   //升级数据
	tool_consumer_register(TOOL_MSG_iapBackup,tool_iap_backup);   //升级备份

#else

	tool_consumer_register(TOOL_MSG_connect,tool_iap_connect); //响应联机消息
	tool_consumer_register(TOOL_MSG_iapRerun,tool_iap_rerun);	//重启boot、app
	tool_consumer_register(TOOL_MSG_iapStart,tool_iap_tool_start);	//启动升级
	tool_consumer_register(TOOL_MSG_iapFileData,tool_iap_data);   //升级数据
	tool_consumer_register(TOOL_MSG_iapBackup,tool_iap_backup);   //升级备份

	//ctp 协议
	ctp_consumer_register(CAN_CARD_1, CTP_MSG_search, tool_iap_power_conn_reply);		
	ctp_consumer_register(CAN_CARD_1, CTP_MSG_iapRerun, tool_iap_power_rerun_reply);
	ctp_consumer_register(CAN_CARD_1, CTP_MSG_iapStart, tool_iap_power_start_reply);
	ctp_consumer_register(CAN_CARD_1, CTP_MSG_iapData, tool_iap_power_data_reply);
	ctp_consumer_register(CAN_CARD_1, CTP_MSG_iapBackup, tool_iap_power_backup_reply);
#endif

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


