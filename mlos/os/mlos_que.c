/**
 ******************************************************************************
 * 文件:mlos_que.c
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

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "mlos_que.h"
#include "mlos_malloc.h"

//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//variable
//----------------------------------------------------------------------------




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
void que_clear(Queptr pQue)
{
	pQue->tail=pQue->head;
	pQue->isfull=0;
	pQue->taken=0;
}

//-----------------------------------------------------------------------------
// 函数名：取用数据
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：先取用数据(调用que_take函数)，才可出列数据
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
QueDataptr que_take(Queptr pQue)
{
	
	if (pQue->head==pQue->tail)					//队列已空
	{
		return nullptr;
	}
	
	pQue->taken=1;
	return pQue->head->pdata;	
}

//-----------------------------------------------------------------------------
// 函数名：出列
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
void que_de(Queptr pQue)
{
	if (pQue->taken==0)
	{
		return ;
	}
	
	if (pQue->isfull)//队列已满，则列尾需要，主动后移
	{
		pQue->tail=pQue->tail->pnext;
	}
	pQue->head=pQue->head->pnext;
	pQue->isfull=0;
	pQue->taken=0;

}

//-----------------------------------------------------------------------------
// 函数名：入列
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
QueDataptr que_en(Queptr pQue)
{
	QueDataptr pQueDat;

	if (pQue->isfull)									//队列已满，不再入列数据
	{
		return nullptr;
	}

	pQueDat=pQue->tail->pdata;								//返回入列节点，数据存储单元
	
	if(pQue->tail->pnext==pQue->head)
	{
		pQue->isfull=1;										//队列已满，尾节点不后移位
	}
	else
	{
		pQue->tail=pQue->tail->pnext;							//入列，移动尾节点
	}

	return pQueDat;

}

//-----------------------------------------------------------------------------
// 函数名：que_create，创建队列
//-----------------------------------------------------------------------------
//
// 返回值 : 
// Queptr: 返回创建的队列对象，=nullptr标识创建失败
// 参数   : 
//mlu8*	pItemBuf:元素数组，创建队列前，已以数组的形式申请了元素的内存
// 	
//quelen：
// 			队列长度，即最大可入列多少个数据,>=2
//itemDatSize：
//			队列数据大小，一个入列数据的大小
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
Queptr que_create(MemoryType memtype,mlu16 quelen ,mlu32 itemDatSize,mlu8* pItemBuf)
{
	Queptr pQue;
	mlu16 i;
	QueItemptr item;

	while (quelen<2)
	{
		pQue=nullptr;
	}

	//创建队列
	pQue=mlos_malloc(memtype, sizeof(Que));

	pQue->isfull=0;
	pQue->taken=0;
	pQue->size=quelen;

	//分配节点内存
	if (pItemBuf==nullptr)
	{
		pItemBuf=mlos_malloc(memtype,itemDatSize*quelen);
	}
	
	//head
	item=mlos_malloc(memtype,sizeof(QueItem));
	item->pnext=nullptr;
	item->pdata=pItemBuf;
	pQue->head=item;
	pQue->tail=pQue->head;
	pItemBuf+=itemDatSize;
	for (i = 1; i < quelen; ++i)
	{
		item=mlos_malloc(memtype,sizeof(QueItem));					//创建节点
		item->pnext=nullptr;
		item->pdata=pItemBuf;	
		pQue->tail->pnext=item;										//插入队列
		pQue->tail=item;
		pItemBuf+=itemDatSize;
	}

	pQue->tail->pnext=pQue->head;								//环形链接
	pQue->tail=pQue->head;										//头尾在一起，队列为空

	return pQue;
	
}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------






