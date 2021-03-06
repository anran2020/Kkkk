/**
 ******************************************************************************
 * 文件:    ctp_msg_idn.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    
 * 内容简介:     通讯协议 消息 id 定义生声明
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
#ifndef  _CTP_MSG_IDN_H__
#define _CTP_MSG_IDN_H__

//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//消息 iD 定义
//----------------------------------------------------------------------------

typedef enum {

	CTP_MSG_ack=0,
	CTP_MSG_connect,
	CTP_MSG_sample,
	CTP_MSG_control,
	CTP_MSG_search,
	CTP_MSG_iapRerun,
	CTP_MSG_iapStart,
	CTP_MSG_iapData,
	CTP_MSG_iapBackup,
	//add new msg id here 
	
	CTP_MSG_autoConnect=27;
	CTP_MSG_cmbSample=
	CTP_MSG_cmbCtrl

	CTP_MSG_commException,					//CAN离线，通讯异常
	CTP_MSG_MAX,						//消息id 最大值
	
}CANtpMsgIDN;





//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif

