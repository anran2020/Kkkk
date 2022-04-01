/**
 ******************************************************************************
 * 文件:mlos_rtimer.c
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 * rtimer->real-time timer 实时定时器
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
#include "mlos_clock.h"
#include "mlos_task.h"
#include "mlos_default_config.h"


//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//local global  variable
//----------------------------------------------------------------------------


//定时器任务
TaskPtr pRtimerTask;


//当前工作定时器链表
ListPtr pCurTmrList;

//溢出定时器，等待系统时钟翻转
ListPtr pOvfTmrList;//overflowed tTimer list


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
void rtimer_init(RTimerPtr ptmr,mlu32 msPeriod,RTimerIsr isr,void *args,RTimerLoadMode loadMode)
{

	ptmr->running=0;
	ptmr->timeout=0;
	ptmr->reloadMode=loadMode;
	ptmr->period=msPeriod;
	ptmr->isr=isr;
	ptmr->args=args;
	ptmr->pprev=nullptr;
	ptmr->pnext=nullptr;
	ptmr->pMyTimerList=nullptr;

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
RTimerPtr rtimer_create(mlu32 msPeriod,RTimerIsr isr,void *args,RTimerLoadMode loadMode)
{
	RTimerPtr prtmr;

	//分配内存
	prtmr=mlos_malloc(e_mem_sram,sizeof(RTimer));
	prtmr->running=0;
	prtmr->timeout=0;
	prtmr->reloadMode=loadMode;
	prtmr->period=msPeriod;
	prtmr->isr=isr;
	prtmr->args=args;
	prtmr->pprev=nullptr;
	prtmr->pnext=nullptr;
	prtmr->pMyTimerList=nullptr;
   	mlos.rtimerCount++; 
    return prtmr;
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
void rtimer_clock_overflow(void)
{
	RTimerPtr ptmr,next;
	ptmr=pOvfTmrList->head;
	while (ptmr!=nullptr)
	{
		next=ptmr->pnext;
		ptmr->pprev=nullptr;
		ptmr->pnext=nullptr;
		list_append(pCurTmrList,ptmr);
		ptmr->pMyTimerList=pCurTmrList;
		ptmr=next;
	}

	pOvfTmrList->head=nullptr;
	pOvfTmrList->tail=nullptr;
	pOvfTmrList->itemCount=0;
	
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
void rtimer_start(RTimerPtr ptmr)
{
	mlu32 nowtime;

	//获取当前时间
	nowtime=mlos_ms_clock();

	//移出
	list_dl_remove(ptmr->pMyTimerList,ptmr);
	
	//加载
	ptmr->timeout=_no;
	ptmr->running=1;
	ptmr->endTime=nowtime+ptmr->period;

	//判断时间是否溢出
	if (ptmr->endTime<nowtime)
	{
		//时间溢出,插入时间溢出链表
		list_append(pOvfTmrList,ptmr);
		ptmr->pMyTimerList=pOvfTmrList;
	}
	else
	{
		//插入当前链表
		list_append(pCurTmrList,ptmr);
		ptmr->pMyTimerList=pCurTmrList;

	}

	//插入链表

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
void rtimer_stop(RTimerPtr ptmr)
{

	ptmr->running=_no;
	ptmr->timeout=_no;
	list_dl_remove(ptmr->pMyTimerList,ptmr);
	ptmr->pMyTimerList=nullptr;

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
Taskstate rtimer_task(void *args)
{
	task_declaration();//任务声明
	MLBool  bOverflow=mlfalse;
	RTimerPtr nextTmPtr;
	RTimerPtr ptmr;
	mlu32 nowTime;
	static mlu32 lastTime=0;


	task_begin(pRtimerTask);//任务开始
#if 1
	//判断基时滴答是否溢出
	nowTime=mlos_ms_clock();

	//基时溢出判断
	if (lastTime>nowTime){bOverflow=mltrue;}
	lastTime=nowTime;


	//判断定时器是否超时
	ptmr=pCurTmrList->head;
	//任务循环
	while(nullptr!=ptmr)
	{
		//判断定时器是否超时
		if (ptmr->endTime>nowTime
			&&(!bOverflow))
		{
			break;
		}

		//超时
		//执行中断函数
		if(nullptr!=ptmr->isr)
		{
			ptmr->isr(ptmr->args);
		}

		//重新加载定时器
		if (e_rtmr_loadAuto==ptmr->reloadMode)
		{
			//自动加载
			nextTmPtr=ptmr->pnext;
			rtimer_start(ptmr);
			ptmr=nextTmPtr;
		}
		else
		{
			//手动加载
			ptmr->timeout=_yes;
			ptmr->running=_no;
			nextTmPtr=ptmr->pnext;
			list_dl_remove(pCurTmrList,ptmr);
			ptmr=nextTmPtr;
		}
		
	}


	//基时溢出，
	if(bOverflow)
	{
		rtimer_clock_overflow();
		bOverflow=0;
	}
#endif	
	task_end(pRtimerTask);//任务结束

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
void rtimer_task_create(void)
{
	//初始化定时器链表
	pCurTmrList=list_create(e_mem_sram,e_lst_sortAscending,e_lst_linkDoubly);
	pOvfTmrList=list_create(e_mem_sram,e_lst_sortUnordered,e_lst_linkDoubly);
	
	//创建定时器任务，系统任务
	pRtimerTask=task_create(RTMR_TASK_PRIO, rtimer_task, nullptr, nullptr,"rtimer");

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------


