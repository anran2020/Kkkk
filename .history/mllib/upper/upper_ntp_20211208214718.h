/**
 ******************************************************************************
 * 文件:ntp_upper.h
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

#ifndef  _NTP_UPPER_H__
#define  _NTP_UPPER_H__

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------

#if (1)

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_ltimer.h"
#include "mlos_malloc.h"
#include "upper_ntp_idn.h"
#include "ntp_usocket.h"
#include "ntp_def.h"

//----------------------------------------------------------------------------
// define configs
//----------------------------------------------------------------------------

//工具协议任务优先级
#define UPPER_TASK_PRIO 		(2)

//协议收发缓存大小
#define UPPER_TX_BUF_SIZE 		(2048)
#define UPPER_RX_BUF_SIZE 		(2048)


//----------------------------------------------------------------------------
//  上位机 通讯协议
//----------------------------------------------------------------------------
typedef struct
{

    //状态
    mlu8 state;

    mlu16 sn;

    //协议链接套接字
    USockptr usock;

    //消息泵
    MsgPumpptr pMyMpump;

    //任务
    TaskPtr pMytask;

} UpperNetworkTransportProtocol, UpperNtp, *UpperNtpptr;

//----------------------------------------------------------------------------
//export vari
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//export def fun
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------

//工具传输协议初始化
void upper_ntp_init(mlu8 *remoteip, mlu16 remoteport,mlu16 myport);


//工具的消息，消费者登记
void upper_consumer_register(UpperNtpMsgIDN idn, MsgCallbackFunction callback);

//获取发送缓冲区
mlu8* upper_txbuf_malloc(mlu16 len);

mlu8 upper_data_send(mlu8 *msg, mlu16 msgLen);

//超时重连
void upper_keepalive_reconn(void); 

void upper_datagram_process_port_set((mlu8*)(*datProcessPort)(mlu8*,mlu8*,mlu16 ));

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif

