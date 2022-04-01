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
#include"tool_config.h"
#if (TOOL_ENABLE)

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "mlos_log.h"
#include "mlos.h"
#include "tool.h"
#include "u_dtd.h"
#include "ntp_w5500.h"
#include "tool_debug.h"
#include "tool_iap.h"
#include "tool_ds.h"

//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------

//工具传输协议服务
ToolNtp toolNtp;

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
mlu8* tool_txbuf_malloc(mlu16 len)
{

	mlu8*u8ptr;
	
	if(toolNtp.usock->sta!=e_sck_sta_working)
	{
		toolNtp.usock->txDataLen=0;
		return nullptr;
	}

	if((toolNtp.usock->txSize-toolNtp.usock->txDataLen)<len)
	{
		return nullptr;
	}
	u8ptr=toolNtp.usock->pTxBuf+toolNtp.usock->txDataLen;
	toolNtp.usock->txDataLen+=len;
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
void tool_txbuf_free(mlu16 len)
{
	if(toolNtp.usock->txDataLen>len)
	{
		toolNtp.usock->txDataLen-=len;
	}
	else
	{
		toolNtp.usock->txDataLen=0;
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
mlu8 tool_data_sum_check( mlu8 * pNetRxBuf,mlu16 len)
{
	mlu16 i,_len;
	mlu8 *ptr,sum;

	//boxAddr 开始求和到seed
	ptr=pNetRxBuf+2;
	sum=0;
	_len=len-3;
	
	for(i=0;i<_len;i++)
	{
		sum+=*ptr++;
	}

	if(sum==*ptr)
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
void tool_consumer_register(ToolNtpMsgIDN idn, MsgCallbackFunction callback)
{

	mpump_consumer_register(toolNtp.pMyMpump, idn, callback);

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
int tool_tx_data(mlu8 cmd,mlu8 addr,mlu8*pdat,mlu16 datlen)
{

	//报文格式：head(2)|len|addr|cmd|data(n)|seed|sum

	mlu8 *pNetTxBuf;
	mlu16 i,j,len,offset;

	//
	if (pdat==nullptr||datlen==0)
	{
		return 0;
	}
	
	//数据包的整体长度为
	len=10+datlen;
	
	//获取发送缓冲区
	pNetTxBuf=tool_txbuf_malloc(len);
	if(pNetTxBuf==nullptr)
		return 0;
	
	//包头
	pNetTxBuf[0]=0xee;
	pNetTxBuf[1]=0xee;
	pNetTxBuf[2]=0xee;
	pNetTxBuf[3]=0xee;
	
	//长度、地址、命令字
	len=len-6;
	pNetTxBuf[4]=len;
	pNetTxBuf[5]=(len>>8);
	pNetTxBuf[6]=addr;
	pNetTxBuf[7]=cmd;
	j=8;
	for(i=0;i<datlen;i++)
	{
		pNetTxBuf[j++]=*pdat++;
	}
	pNetTxBuf[j++]=0;
	pNetTxBuf[j]=0;
	offset=6;
	len=len-1;
	for(i=0;i<len;i++)
	{
		pNetTxBuf[j]+=pNetTxBuf[offset+i];
	}

	return datlen;


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
mlu8* tool_packet_extract(void)
{

	mlu16 i,len,byteTotal,port;
	mlu8 *u8ptr;

	byteTotal=toolNtp.usock->rxDataLen;

	//调试数据
	if(byteTotal < 9)
	{
		return nullptr;
	}
	//udp 协议包格式
	//ip(4),port(2),len(2),
	//head(4),len,addr,cmd,datan,seed,sumcheck

	u8ptr=toolNtp.usock->pRxBuf;
	if(toolNtp.usock->haveUDPHead)
	{
		//分析udp 数据包
		// udp 源地址 ip \port 获取
		for(i=0;i<4;i++)
		{
			toolNtp.usock->remoteip[i]=u8ptr[i];
		}
		toolNtp.curRemotePort=u8ptr[4];
		toolNtp.curRemotePort=(toolNtp.curRemotePort<<8)+u8ptr[5];

		// 2 byte 数据长度,偏移8byte
		u8ptr+=8;
	}
	//
	if((0xff == *u8ptr++)
		&&(0xff == *u8ptr++)
		&&(0xff ==*u8ptr++)
		&&(0xff == *u8ptr++))
	{
		//计算包内数据域长度
		len=*((mlu16*)u8ptr)+2;

		//对比数据包的长度，与总接收数据的长度
		if((len+6)>byteTotal)
		{
			//数据未接收完整
			//log_print(...);
			return nullptr;
		}

		//计算校验和
		if(tool_data_sum_check(u8ptr,len))
		{
			return u8ptr;//返回数据包的起始地址
			
		}
		else
		{
			//校验失败
			//log_print(...);
		}

	}
	toolNtp.usock->rxDataLen=0;

	return nullptr;
	
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
Taskstate tool_task(void* args)
{
	task_declaration();//任务声明
//	mlu16 i,len,byteTotal;
	mlu8 *pMsgPacket;


	task_begin(toolNtp.pMytask);//任务开始

	//驱动通讯套接字,获取数据
	usocket_receive(toolNtp.usock);

	//提取消息数据包
	pMsgPacket=tool_packet_extract();
	if(nullptr!=pMsgPacket)
	{
		//len(2),addr,cmd,data(n),seed,sumcheck
		// offset 3  cmd is msg idn
		//分派消息给消费者处理
		mpump_dispatch(toolNtp.pMyMpump,*(pMsgPacket+3),pMsgPacket);
		toolNtp.usock->rxDataLen=0;
	}

	tool_debug_task();//调试功能任务


	//再次驱动通讯套接字,发送数据
	usocket_send(toolNtp.usock);

	task_end(toolNtp.pMytask);//任务结束
	
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
void tool_init(void)
{
	//mlu8 i;


	//创建任务
	toolNtp.pMytask=task_create(TOOL_TASK_PRIO, tool_task, &toolNtp, ltimer_create(),"tool");

	//消息泵创建
	toolNtp.pMyMpump=mpump_create(e_mem_sram,0);



	//创建socket，udp
	toolNtp.usock=usocket_create(w5500_network_card(),e_sck_md_udp, TOOL_TX_BUF_SIZE, TOOL_RX_BUF_SIZE);
	toolNtp.usock->remoteport=20208;
	toolNtp.usock->myport=20209;
	toolNtp.usock->remoteip[0]=192;
	toolNtp.usock->remoteip[1]=168;
	toolNtp.usock->remoteip[2]=0;
	toolNtp.usock->remoteip[3]=100;
	toolNtp.usock->haveUDPHead=1;

	//工具支持的功能模块，初始化
	tool_iap_init();
	tool_debug_int();
	tool_ds_init();
	tool_setting_init();
}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
