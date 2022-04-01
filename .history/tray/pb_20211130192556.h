#ifndef __PB_H__
#define __PB_H__

#include "mlos_ltimer.h"
#include "mlos_malloc.h"
#include "ntp_w5500.h"
#include "ntp_usocket.h"
#include "ntp_def.h"

//工具协议任务优先级
#define PLC_TASK_PRIO 		(2)

//协议收发缓存大小
#define PLC_TX_BUF_SIZE 		(2048)
#define PLC_RX_BUF_SIZE 		(2048)

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

    //接收回调
    mlu16 (*RxCallBack)(mlu8 *,mlu16);

    //任务
    TaskPtr pMytask;

} PbNetworkTransportProtocol, PbNettp, *PbNtpptr;


extern void Pb_init(Netcardptr pNetHardWare,mlu8 *remoteip, mlu16 remoteport,mlu16 (*rxcallback)(mlu8 *msg,mlu16 len));

extern mlu8* Pb_txbuf_malloc(mlu16 len);
extern void plc_reconn(void);

#endif



























