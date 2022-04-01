/**
 ******************************************************************************
 * 文件:ntp_usocket.h
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

#ifndef  _NTP_USOCKET_H__
#define  _NTP_USOCKET_H__

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#if (1)

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"
#include "mlos_malloc.h"
#include "mlos_list.h"
#include "mlos_ltimer.h"
#include "mlos_task.h"
#include "ntp_def.h"

//----------------------------------------------------------------------------
//	define config
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//	自定义 ML 套接字,协议模式
//----------------------------------------------------------------------------

typedef enum
{

    e_sck_md_tcp = 1, // net tcp
    e_sck_md_udp, // net udp

} USocketMode;

//----------------------------------------------------------------------------
//  自定义ML网络套接字状态
//----------------------------------------------------------------------------
typedef enum
{

    e_sck_sta_null = 0,
	e_sck_sta_opening, //打开
    e_sck_sta_connecting, //连接
    e_sck_sta_working, //工作
    e_sck_sta_closed, //关闭

} USocketState;

typedef enum
{

    e_sck_opNull = 0, e_sck_opClose, e_sck_opClear, //打开

} USocketOperation;

//---------------------------------------------------------
//mlsocket连接状态
//---------------------------------------------------------
typedef enum
{

    e_sck_cnn_none = 0, //没有连接
    e_sck_cnn_established, //连接到服务器了 
    e_sck_cnn_closing, //正在关闭连接
    e_sck_cnn_failed, //连接失败
    e_sck_cnn_closed, //连接关闭
    e_sck_cnn_timeout, //连接超时

} USockConnSta;

//----------------------------------------------------------------------------
//  自定义网络套接字发送状态
//----------------------------------------------------------------------------
typedef enum
{

    e_sck_rxNull = 0, //未接收到数据
    e_sck_rxTimeout,
    e_sck_rxData, //接收数据

} USockRxState;

typedef enum
{

    e_sck_txFinished = 0, //发送完成
    e_sck_txIng, //发送中
    e_sck_txTimeout, //发送超时

} USockTxState;

//----------------------------------------------------------------------------
//  用户自定义 ML网络套接字 
//----------------------------------------------------------------------------
//#pragma anon_unions
typedef struct USocketNode
{

    //指向下一个网络套接字
    struct USocketNode *pnext;

    mlu8 sn; //socket number
    mlu8 mode;
    mlu8 haveUDPHead;

    mlu8 timeOut; //connection or termination or data transmission 超时
    mlu8 rxTimeoutCnt; //收发错误计数
    mlu8 txFailedCnt;
    mls8 error; //错误信息

    //状态
    mlu8 sta; //
    mlu8 txsta; //收发状态
    mlu8 rxsta;
    mlu8 connSta; //连接状态
    mlu8 op;

    //要发送的数据长度
    mlu16 txDataLen;
    mlu16 rxDataLen;
    mlu16 curtxlen; //当前发送长度
    mlu16 curRxlen;

    //收发定时器
    LTimerPtr pRxtimer;
    LTimerPtr pTxtimer;

    //收发缓冲区
    mlu16 txSize;
    mlu16 rxSize;
    mlu8 *pRxBuf;
    mlu8 *pTxBuf;

    //驱动任务
    SubtaskPtr pMyDriveTask;

    mlu8 remoteip[4];
    mlu16 remoteport;
    mlu16 myport;

    mlu32 rxBaseAddr;

	//所属网卡
	Netcardptr pMynetcard;
	
} USocket, *USockptr;

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
_inline void usocket_receive(USockptr pusck)
{
	
	pusck->pMynetcard->usocket_drive(pusck);

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
//功能描述：socket 发送数据
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//----------------------------------------------------------------------------- 
_inline void usocket_send(USockptr pusck)
{
	
	pusck->pMynetcard->usocket_drive(pusck);

}

//----------------------------------------------------------------------------
//export function
//----------------------------------------------------------------------------

USockptr usocket_create(Netcardptr pNetcard,USocketMode md, mlu16 txsize, mlu16 rxsize);

void usocket_reset(USockptr pusck);

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif

