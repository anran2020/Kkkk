/**
 ******************************************************************************
 * 文件:  utp_msg_idn.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    
 * 内容简介: 
 *                   
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                创建该文件
 *
 *
 *
 ******************************************************************************
 **/
#ifndef  _UTP_MSG_IDN_H__
#define _UTP_MSG_IDN_H__

//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//消息 iD 定义
//----------------------------------------------------------------------------

typedef enum {

	UTP_MSG_ack=0,
	UTP_MSG_connect, //联机命令
	UTP_MSG_sample, //主通道采样数据
	UTP_MSG_disconnect, //广播脱机

	
}UARTtpMsgIDN;





//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif

