/**
 ******************************************************************************
 * 文件:
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1         2014-12-01     zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#if (1)

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "mlos.h"
#include "upper_ntp.h"
#include "ntp.h"
#include "ntp_w5500.h"
#include "u_function.h"

//#include "basic.h"
#include "entry.h"
#include "host.h"
#include "timer.h"

//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------

//上位机传输协议服务
UpperNtp upperNtp;

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
mlu8* upper_txbuf_malloc(mlu16 len)
{
    mlu8 *u8ptr=nullptr;

    if (upperNtp.usock->sta != e_sck_sta_working)
    {
        upperNtp.usock->txDataLen = 0;
        return nullptr;
    }

    if((upperNtp.usock->txDataLen+len)<=upperNtp.usock->txSize)
    {
        u8ptr = upperNtp.usock->pTxBuf + upperNtp.usock->txDataLen;
        upperNtp.usock->txDataLen += len;
    }

    return u8ptr;

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
mlu8 upper_sum_check(mlu8 *pNetRxBuf, mlu16 len)
{
    mlu16 i;
    mlu8 *ptr, sum;

    sum = 0;
    ptr = pNetRxBuf;
    for (i = 0; i < len; i++)
    {
        sum += *ptr++;
    }

    if (sum == *ptr)
    {
        return 1;
    }

    return 0;

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
void upper_keepalive_reconn(void)
{
	usocket_reset(upperNtp.usock);
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
void upper_consumer_register(UpperNtpMsgIDN idn, MsgCallbackFunction callback)
{

    mpump_consumer_register (upperNtp.pMyMpump, idn, callback);

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
mlu8 upper_data_send(mlu8 *msg, mlu16 msgLen)
{
    mlu8 *buf;

    buf = (mlu8 *)upper_txbuf_malloc(msgLen);
    if (nullptr == buf)
    {
        return 1;
    }

    memcpy(buf, msg, msgLen);
    return 0;
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
void upper_datagram_process
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
void upper_datagram_process(USockptr sock)
{
	sock->rxDataLen = upDataRecv(sock->pRxBuf, sock->pRxBuf, sock->rxDataLen) - sock->pRxBuf;
#if 0
	UpMsgHead *msgHead;
	UpMsgTail *msgTail;
	mlu8 *head,*pos;
	mlu16 len;

	head=upperNtp.usock->pRxBuf;
	pos=upperNtp.usock->pRxBuf+upperNtp.usock->rxDataLen;
	for (; ;head+=UpMsgMandLen+msgHead->pldLen)
	{
		len = pos - head;
		if (len < UpMsgMandLen)
		{
			break;		//长度不够包的最小长度，继续等待接收
		}

		msgHead = (UpMsgHead *)head;
		if (UpMsgMagic!=msgHead->magicTag || msgHead->pldLen>UpMsgPldLenMax)
		{
			upperNtp.usock->rxDataLen=0;/*丢弃，清空*/
			return ;  
		}

		if (len < UpMsgMandLen+msgHead->pldLen)
		{
			break;//数据包未接收完继续等待接收
		}

		msgTail = (UpMsgTail *)(head+sizeof(UpMsgHead)+msgHead->pldLen);
		if (msgTail->crcChk == crc16(head+sizeof(mlu32), msgHead->pldLen+UpMsgMandCrcLen))
		{
			mlu8 msgGrp;

			log_print(LOG_LVL_INFO,"\r\nrecv upper msg: 0x%04x\r\n", msgHead->msgId);
			/*outHex(head, msgHead->pldLen+UpMsgMandLen);*/
			msgGrp = msgHead->msgId >> UpMsgGrpOffset;
			
			mpump_dispatch(upperNtp.pMyMpump, msgGrp, head);//派送消息

		}
		else
		{
			log_print(LOG_LVL_ERROR,"recv upper msg err crc\r\n");
			upperNtp.usock->rxDataLen=0;/*丢弃，清空*/
			return ; 
		}
	}


	if (0!=len && head!=upperNtp.usock->pRxBuf)
	{
		memcpy(upperNtp.usock->pRxBuf, head, len);
	}
#endif
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
Taskstate upper_ntp_task(void *args)
{
    task_declaration();			//任务声明
    mlu16 i, len, byteTotal;
    mlu8 *pMsgPacket;

    task_begin(upperNtp.pMytask);//任务开始

    //驱动通讯套接字,获取数据
	usocket_receive( upperNtp.usock);
		
    //提取消息数据包
    upper_datagram_process(upperNtp.usock);

    //再次驱动通讯套接字,发送数据
    usocket_send( upperNtp.usock);

	task_end(upperNtp.pMytask);			//任务结束

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
void upper_ntp_init(mlu8 *remoteip, mlu16 remoteport,mlu16 myport)

{
    mlu8 i;
	mlu8 defaultRemoteip[4] =
	{ 192, 168, 0,57 };

    //创建任务
    upperNtp.pMytask = task_create (UPPER_TASK_PRIO, upper_ntp_task, &upperNtp, ltimer_create(),"upper");

    //消息泵创建
    upperNtp.pMyMpump = mpump_create (e_mem_sram,nullptr);

    //创建socket，udp
    upperNtp.usock = usocket_create (w5500_network_card(), e_sck_md_tcp, UPPER_TX_BUF_SIZE, UPPER_RX_BUF_SIZE);


	if(remoteport==0XFFFF)
	{
		//没配置启用默认参数
		upperNtp.usock->remoteport = 2021;
	    upperNtp.usock->myport = 2022;
		for (i = 0; i < 4; ++i)
    	{
        	upperNtp.usock->remoteip[i] = defaultRemoteip[i];
   	 	}
	}
	else
	{
		upperNtp.usock->remoteport = remoteport;
	    upperNtp.usock->myport = myport;
		for (i = 0; i < 4; ++i)
    	{
        	upperNtp.usock->remoteip[i] = remoteip[i];
   	 	}
	}


}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif

