/**
 ******************************************************************************
 * 文件:    ctp.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    MSG_LEA_IDN
 * 内容简介:    lc mc 通讯协议相关数据类型定义，功能函数
 *                    接口导出
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
#ifndef  _CTP_H__
#define _CTP_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos.h"
#include "ctp_config.h"
#include "ctp_dtd.h"
#include "ctp_can_driver.h"
#include "ctp_msg_idn.h"




//----------------------------------------------------------------------------
//define s
//----------------------------------------------------------------------------

#define CTPSN 				mlu8 			//流水号

//----------------------------------------------------------------------------
//  can 卡 master  station 定义
//----------------------------------------------------------------------------
typedef struct{

	mlu8 take;//绑定的cancard
	


}CANtpMasterStation,*CtpMstStationptr;//主站

//----------------------------------------------------------------------------
//  can 卡 slaver station  定义
//----------------------------------------------------------------------------

typedef struct{

	mlu8 connSta;//联机状态,slaver与master的联机状态
	//通信监测定时器,监测联机状态
	LTimerPtr pConnTimer;

	//发送联机报文接口，根据slaver的设备类型，实现相应的接口,
	void (*connect)(void);

}CANtpSlaverStation,*CtpSlvStationptr;//从站

//----------------------------------------------------------------------------
//  can 卡 定义
//----------------------------------------------------------------------------
typedef struct{

	mlu8 id;								//can 卡的  id
	mlu8 station;							//can 卡站属性，主站或从站		
	mlu8 mfSeriNmber;						//多帧流水号流水号
	mlu8 sfSeriNmber;						//单帧流水号
		
	Queptr pRxQue;							//收队列,链式队列
	Queptr pTxQue;							//发队列，链式队列
	CtpSlvStationptr pMySlvStation;			//从站属性
	
	MsgPumpptr pMymsgpump;					//消息泵，每个can卡，有一个消息泵

	CtpMfMsgptr pMfmsg;						//多帧报文消息
	mlu32 mfRxError;
	
	mlu8 (*tx)(CANCardID,CANFrmptr);		//can卡的报文发送驱动接口
	
}CANCard,*CANCardptr;

//----------------------------------------------------------------------------
//  通讯传输协议数据类型定义
//----------------------------------------------------------------------------
 typedef struct{

 	
 	Taskptr pMytask;						//协议任务
	CANCard canCard[CAN_NUM];				//can 卡定义
	LTimerPtr pTestTimer;					//测试定时器

	mlu8 txbuf[];							//发送缓冲区
	
}CanTransmitProtocol,CANtp,CTP;


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

//初始化
void ctp_init(mlu8 CAN1Addr,CANBaudRate baudrate1,mlu8 CAN2Addr,CANBaudRate baudrate2);


//订阅消息
void ctp_consumer_register(CANCardID ccid,CANtpMsgIDN idn, MsgCallbackFunction callback);

//设置从站联机接口
void ctp_conn_port_set(CANCardID cid,void (*txConnectMsgPort)(void));
void ctp_connected(CANCardID ccid);												//反馈从站联机成功，来自业务逻辑判定的联机成功反馈

//发送单帧数据
CANFrmptr ctp_txframe_malloc(CANCardID ccid);

//发送多帧数据
MLBool ctp_send(CANCardID ccid,mlu8 addr,mlu8*pBuf,mlu16 len);
mlu8* ctp_txbuf_malloc(mlu16 len);


//流水号
CtpSerialNumber ctp_sn_generate(CANCardID cid);

#endif

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

