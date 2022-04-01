
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
PressBed prsBed;

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

    if (prsBed.usock->sta != e_sck_sta_working)
    {
    	prsBed.usock->txDataLen = 0;
        return nullptr;
    }

    if((prsBed.usock->txDataLen+len)<=prsBed.usock->txSize)
    {
        u8ptr = prsBed.usock->pTxBuf + prsBed.usock->txDataLen;
        prsBed.usock->txDataLen += len;
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
    if (e_sck_sta_working == prsBed.usock->sta)
    {
        prsBed.usock->txDataLen = 0;
        prsBed.usock->rxDataLen = 0;
        prsBed.usock->sta = e_sck_sta_closed;
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

    task_begin(prsBed.pMytask);//任务开始


    //plc 
    for()
    {


        
    }
    //驱动通讯套接字,获取数据
	usocket_receive( prsBed.usock);

    //提取消息数据包
	if(prsBed.RxCallBack != NULL && prsBed.usock->rxDataLen != 0)
	{
		prsBed.usock->rxDataLen = prsBed.RxCallBack(prsBed.usock->pRxBuf,prsBed.usock->rxDataLen);
	}

    //再次驱动通讯套接字,发送数据
    usocket_send( prsBed.usock);

	task_end(prsBed.pMytask);			//任务结束
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
    if(plcx>=PB_CTRL_DEV_MAX)
        return;

    //创建任务
    
    //消息泵创建
    prsBed.datagramProcesses[plcx] = datagramProcesses;

    //创建socket，pb
    prsBed.usock = usocket_create (w5500_network_card(), e_sck_md_tcp, PB_TX_BUF_SIZE, PB_RX_BUF_SIZE);
    prsBed.usock->remoteport = remoteport;
    prsBed.usock->myport = 6200+plcx;

    for (i = 0; i < 4; ++i)
    {
    	prsBed.usock->remoteip[i] = remoteip[i];
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
void Pb_init(void)
{

    //创建任务
    prsBed.pMytask = task_create (PB_TASK_PRIO, Pb_task, &prsBed, ltimer_create(),"prBed");

}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译



