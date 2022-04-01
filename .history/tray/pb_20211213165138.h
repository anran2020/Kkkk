#ifndef __PB_H__
#define __PB_H__
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE
#include "mlos_ltimer.h"
#include "mlos_malloc.h"
#include "ntp_w5500.h"
#include "ntp_usocket.h"
#include "ntp_def.h"

//工具协议任务优先级
#define PB_TASK_PRIO 		(2)

#define PB_PLC_MAX          (2)//针床对接控制模块的最大数，plc，

//协议收发缓存大小
#define PB_TX_BUF_SIZE 		(2048)
#define PB_RX_BUF_SIZE 		(2048)

//----------------------------------------------------------------------------
//  针床数据，结构
//----------------------------------------------------------------------------
typedef struct
{

    //plc的协议链接套接字
    USockptr plcUsock[PB_PLC_MAX];

    //通讯数据分析处理接口
    mlu16 (*datagramProcesses[PB_PLC_MAX])(mlu8 *,mlu16);

    //任务
    TaskPtr pMytask;

} PressBed, *PressBedptr;


extern void Pb_init(Netcardptr pNetHardWare,mlu8 *remoteip, mlu16 remoteport,mlu16 (*rxcallback)(mlu8 *msg,mlu16 len));

extern mlu8* Pb_txbuf_malloc(mlu16 len);
extern void plc_reconn(void);

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译
#endif



























