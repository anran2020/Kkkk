
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
void pb_plc_usocket_reconn(mlu8 plcx)
{
    usocket_reset
    if (e_sck_sta_working == prsBed.usock[plcx]->sta)
    {
        prsBed.usock[plcx]->txDataLen = 0;
        prsBed.usock[plcx]->rxDataLen = 0;
        prsBed.usock[plcx]->sta = e_sck_sta_closed;
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


    //plc网口通讯
    for(i=0;i<PB_CTRL_DEV_MAX;++i)
    {
        if(prsBed.usock[i]!=nullptr)
        {
            //驱动通讯套接字,获取数据
            usocket_receive( prsBed.usock[i]);

            //提取消息数据包
            if(prsBed.datagramProcesses[i] != nullptr && prsBed.usock[i]->rxDataLen != 0)
            {
                prsBed.usock[i]->rxDataLen = prsBed.datagramProcesses[i](prsBed.usock[i]->pRxBuf,prsBed.usock[i]->rxDataLen);
            }

            //再次驱动通讯套接字,发送数据
            usocket_send( prsBed.usock[i]);
        }
    }


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
    prsBed.usock[plcx] = usocket_create (w5500_network_card(), e_sck_md_tcp, PB_TX_BUF_SIZE, PB_RX_BUF_SIZE);
    prsBed.usock[plcx] ->remoteport = remoteport;
    prsBed.usock[plcx] ->myport = 6200+plcx;

    for (i = 0; i < 4; ++i)
    {
    	prsBed.usock[plcx] ->remoteip[i] = remoteip[i];
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
    mlu8 i;
    for(i=0;i<PB_CTRL_DEV_MAX;++i)
    {
        prsBed.usock[i]=nullptr;    
        prsBed.datagramProcesses[i]=nullptr; 
    }

}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译



