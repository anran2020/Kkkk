/**
 ******************************************************************************
 * 文件:mlos_mpump.c
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 * mpump->message pump,消息泵
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

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "mlos.h"
#include "mlos_malloc.h"

//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//local global  variable
//----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : 
// 	MsgPumpptr 返回创建消息泵的指针
// 参数   :  
//MemoryType mt 内存类型，给消息泵分配内存
//mlu8 msgMax  消息最大数，即消息泵容纳消息的最大数，
//        msgMax=0，标识先不分配消息的内存,建立一个链表的管理消息，
//		 msgMax>0,分配连续内存（数组）管理消息
//功能描述：
//			创建一个消息泵，派送消息
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//----------------------------------------------------------------------------
MsgPumpptr mpump_create(MemoryType mt,mlu8 msgMax)
{
	mlu8 i;
	MsgPumpptr pMpump;

	pMpump=mlos_malloc(mt,sizeof(MessagePump));					//创建消息泵
	mlos.pumpConut++;
	
	if(msgMax==0)
	{
		pMpump->type=e_pump_list;//链表模式消息泵
		pMpump->pMymsglist=list_create(e_mem_sram,e_lst_sortUnordered, e_lst_linkSingly);	//单向序链表，链接消息
		
	}
	else
	{
		pMpump->type=e_pump_array;//数组模式消息泵
		pMpump->msgMax=msgMax;
		mlos.msgConut+=msgMax;														//统计系统消息个数
		pMpump->msgArray=mlos_malloc(e_mem_sram, msgMax*sizeof(ArrayModeMessage));			//开辟连续内存，管理消息
		for (i = 0; i < msgMax; ++i)												//初始化，每个数组项
		{
			pMpump->msgArray[i].pConsumerList=list_create(e_mem_sram,e_lst_sortUnordered, e_lst_linkSingly);
		}
		
	}

	return pMpump;

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
//----------------------------------------------------------------------------
ListModeMsgptr list_message_find(MsgPumpptr pMsgpump,MsgIDN idn)
{
	ListModeMsgptr pMsgNode;
	
	pMsgNode = pMsgpump->pMymsglist->head;
	while(nullptr!=pMsgNode)
	{
		if (pMsgNode->idn==idn)
		{
			return pMsgNode;
		}
		pMsgNode = pMsgNode->pnext;
	}

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
//----------------------------------------------------------------------------
ListModeMsgptr list_message_register(MsgPumpptr pMsgpump,MsgIDN idn)
{
	ListModeMsgptr pMsgNode=nullptr;


	pMsgNode=list_message_find(pMsgpump,idn);	//判断消息是否已经存在消息泵链表里
	if(pMsgNode==nullptr)					//消息不存在泵链表里,新建一个消息
	{
		pMsgNode=mlos_malloc(e_mem_sram,sizeof(ListModeMessage));//开辟消息内存
		pMsgNode->pnext=nullptr;
		pMsgNode->pConsumerList=list_create(e_mem_sram,e_lst_sortUnordered, e_lst_linkSingly);//创建消息消费者链表
		pMsgNode->idn=idn;
		list_append(pMsgpump->pMymsglist,pMsgNode);	//将消息添加到链表里
		mlos.msgConut++;
	}

	return pMsgNode;
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
//----------------------------------------------------------------------------
Consumerptr consumer_find(ListPtr pConsumerlist,MsgCallbackFunction callback)
{
	Consumerptr  pConsumer;
	
	pConsumer = pConsumerlist->head;
	while(nullptr!=pConsumer)
	{
		if (pConsumer->callback==callback)
		{
			break;
		}
		pConsumer = pConsumer->pnext;
	}
	return pConsumer;
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
//----------------------------------------------------------------------------
void consumer_register(ListPtr pConsumerlist,MsgCallbackFunction callback)
{

	Consumerptr  pConsumer;

	pConsumer=consumer_find(pConsumerlist,callback);				//消息消费者是否已经存在
	if (nullptr!=pConsumer)
	{
		return;
	}
	pConsumer=mlos_malloc(e_mem_sram,sizeof(Consumer));					//新建一个消息消费者
	pConsumer->pnext=nullptr;
	pConsumer->callback=callback;
	list_append(pConsumerlist,pConsumer);								//登记消费者,加入链表
	mlos.consumerCount++;

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
//功能描述：注册一个消息的消费者，
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//----------------------------------------------------------------------------
void mpump_consumer_register(MsgPumpptr pMsgpump,MsgIDN  idn,MsgCallbackFunction callback)
{
	ListPtr pConsumerlist=nullptr;

	if (pMsgpump==nullptr||callback==nullptr)	//参数检测
	{
		return;
	}

	
	if(pMsgpump->type==e_pump_list)						
	{
		pConsumerlist=list_message_register(pMsgpump,idn)->pConsumerList;			//注册链表消息
	}
	else
	{
		if(idn>=pMsgpump->msgMax)													//消息id 非法，超出pump最大容量
		{
			return ;
		}
		pConsumerlist=pMsgpump->msgArray[idn].pConsumerList;						//注册数组消息							
		
	}

	consumer_register(pConsumerlist,callback);								//注册消费者
	
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
//----------------------------------------------------------------------------
ListPtr mpump_consumers_find(MsgPumpptr pMsgpump,MsgIDN idn)
{
	ListModeMsgptr pMsgNode;

	if(pMsgpump->type==e_pump_array)
	{
		if(idn<pMsgpump->msgMax)
		{
			return pMsgpump->msgArray[idn].pConsumerList;
		}
	}
	else
	{
		pMsgNode = pMsgpump->pMymsglist->head;
		while(nullptr!=pMsgNode)
		{
			if (pMsgNode->idn==idn)
			{
				return pMsgNode->pConsumerList;
			}
			pMsgNode = pMsgNode->pnext;
		}
	}

	return nullptr;
	
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : MLBool：消息被处理返回-mltrue；否者返回-mlfalse；
// 参数   :  void
//
//
//
//功能描述：
//分派消息给消费者，消息未被消费则，返回给任务 
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//----------------------------------------------------------------------------
MLBool mpump_dispatch(MsgPumpptr pMsgpump,MsgIDN idn,void * args)
{
	ListPtr pConsumerList;
	Consumerptr pConsumer=nullptr;
	
	if (nullptr==pMsgpump)								//参数合法判断
	{
		return mlfalse;
	}

	
	pConsumerList = mpump_consumers_find(pMsgpump,idn);							//获取消息的消费者链表
	if (nullptr==pConsumerList)
	{
		return mlfalse;															//消息没有被消费，抛给任务自己处理
	}

	pConsumer=pConsumerList->head;											//消息处理者，处理消息
	while(nullptr!=pConsumer)
	{
		if(nullptr!=pConsumer->callback&&pConsumer->callback(args))			//回调处理消息内容
		{
			return mltrue;													//消息被处理直接返回，
		}
		pConsumer=pConsumer->pnext;											//消息接着被下一个处理者处理
	}
			
	return mlfalse;															//消息未被处理，抛给任务自己处理

}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------



