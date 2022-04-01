
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE
#include "pb.h"

//----------------------------------------------------------------------------
// global variable
//----------------------------------------------------------------------------

//上位机传输协议服务
PbNetworkTransportProtocol PbNtp;

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
mlu8* Pb_txbuf_malloc(mlu16 len)
{
    mlu8 *u8ptr=nullptr;

    if (PbNtp.usock->sta != e_sck_sta_working)
    {
    	PbNtp.usock->txDataLen = 0;
        return nullptr;
    }

    if((PbNtp.usock->txDataLen+len)<=PbNtp.usock->txSize)
    {
        u8ptr = PbNtp.usock->pTxBuf + PbNtp.usock->txDataLen;
        PbNtp.usock->txDataLen += len;
    }

    return u8ptr;

}

void Pb_Txmsg(mlu8 *msg,mlu16 datalen)
{
	mlu8 *ptr = NULL;

	ptr = Pb_txbuf_malloc(datalen);

	if(ptr != NULL)
	{
		memcpy(ptr,msg,datalen);
	}
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
void Pb_Rxmsg(mlu8 *msg,mlu16 datalen)
{
	
}
void plc_reconn()
{
    if (e_sck_sta_working == PbNtp.usock->sta)
    {
        PbNtp.usock->txDataLen = 0;
        PbNtp.usock->rxDataLen = 0;
        PbNtp.usock->sta = e_sck_sta_closed;
    }

    return;
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
Taskstate Pb_task(void *args)
{
    task_declaration();			//任务声明
    mlu16 i, len, byteTotal;
    mlu8 *pMsgPacket;

    task_begin(PbNtp.pMytask);//任务开始

    //驱动通讯套接字,获取数据
	usocket_receive( PbNtp.usock);

    //提取消息数据包
	if(PbNtp.RxCallBack != NULL && PbNtp.usock->rxDataLen != 0)
	{
		PbNtp.usock->rxDataLen = PbNtp.RxCallBack(PbNtp.usock->pRxBuf,PbNtp.usock->rxDataLen);
	}

    //再次驱动通讯套接字,发送数据
    usocket_send( PbNtp.usock);

	task_end(PbNtp.pMytask);			//任务结束
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
void Pb_plc_socket_init(mlu8 plcx,mlu8 *remoteip, mlu16 remoteport,mlu16 (*datagramProcesses)(mlu8 *msg,mlu16 len))
{
    mlu8 i;

    //创建任务
    

    //消息泵创建
    PbNtp.RxCallBack = rxcallback;

    //创建socket，pb
    PbNtp.usock = usocket_create (pNetHardWare, e_sck_md_tcp, PLC_TX_BUF_SIZE, PLC_RX_BUF_SIZE);
    PbNtp.usock->remoteport = remoteport;
    PbNtp.usock->myport = 20207;

    for (i = 0; i < 4; ++i)
    {
    	PbNtp.usock->remoteip[i] = remoteip[i];
    }
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
void Pb_init(Netcardptr pNetHardWare,mlu8 *remoteip, mlu16 remoteport,mlu16 (*rxcallback)(mlu8 *msg,mlu16 len))
{
    mlu8 i;

    //创建任务
    PbNtp.pMytask = task_create (PLC_TASK_PRIO, Pb_task, &PbNtp, ltimer_create(),"upper");

    //消息泵创建
    PbNtp.RxCallBack = rxcallback;

    //创建socket，pb
    PbNtp.usock = usocket_create (pNetHardWare, e_sck_md_tcp, PLC_TX_BUF_SIZE, PLC_RX_BUF_SIZE);
    PbNtp.usock->remoteport = remoteport;
    PbNtp.usock->myport = 20207;

    for (i = 0; i < 4; ++i)
    {
    	PbNtp.usock->remoteip[i] = remoteip[i];
    }
}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译



