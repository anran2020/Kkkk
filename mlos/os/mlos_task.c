/**
 ******************************************************************************
 * 文件:mlos_task.c
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
#include <string.h>
#include "mlos_task.h"
#include "mlos_ltimer.h"
#include "mlos_malloc.h"
#include "mlos_log.h"
#include "mlos.h"

//----------------------------------------------------------------------------
//条件编译
//----------------------------------------------------------------------------
#if 1

//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------

//任务优先级组
typedef struct TaskPriorityGroup{

	struct TaskPriorityGroup*pnext;

	Taskptr pCurTask;//当前运行的任务
	ListPtr ptasklist;//单向链表
	
}TaskGroup,*TaskGroupptr;

//----------------------------------------------------------------------------
//local global  variable
//----------------------------------------------------------------------------

//任务组
TaskGroup taskgroup[TASK_PRIO_LEVEL_MAX];
TaskGroupptr pCurTaskgroup;//当前轮训任务组

//挂起任务的链表
List suspendTaskList;

//任务s统计的时间
mlu32 lastSecondStatisticsTime;

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	:  void
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
void task_name_set(Taskptr ptask,char* name)
{
	static const char* nameless="NUL";
	if (name==nullptr)
	{
		ptask->name=nameless;					//无名任务
	}
	ptask->name=name;
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
Taskptr task_create(TaskPriority prio,TaskFunction tf,void*args,LTimerPtr tskTmr,char* taskName)
{

	TaskPtr ptask;

	//创建任务
	ptask= mlos_malloc(e_mem_sram,sizeof(Task));
	
	//初始化任务
	ptask->lcl=0;
	ptask->run=tf;
	ptask->args=args;
	ptask->prio=prio;
	ptask->pTimer=tskTmr;

	//优先级判断
	if(ptask->prio>=TASK_PRIO_LEVEL_MAX)
	{
		ptask->prio=TASK_PRIO_LOWEST;
		prio=TASK_PRIO_LOWEST;
	}
	
	//根据优先级，加入任务组链表，双向链表，不循环
	list_append(taskgroup[prio].ptasklist,ptask);

	if (taskgroup[prio].pCurTask==nullptr)
	{
		taskgroup[prio].pCurTask=taskgroup[prio].ptasklist->head;
	}

	mlos.taskID++;
	ptask->id=mlos.taskID;
	ptask->prio=prio;

	//取任务名称
	task_name_set(ptask,taskName);

	
	return ptask;

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
Subtaskptr subtask_create(LTimerPtr tskTmr)
{

	SubtaskPtr psubtask;

	psubtask= mlos_malloc(e_mem_sram,sizeof(Subtask));

	psubtask->lcl=0;
	psubtask->pTimer=tskTmr;

	mlos.subtaskID++;
	return psubtask;
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
mlu16 mlos_all_task_name(mlu8 *outNames)
{
	char* outstr;		
	mlu16 len,i,nameLen;
	TaskPtr ptask;

	outstr=(char*)outNames;
	len=0;
	for (i = 0; i < TASK_PRIO_LEVEL_MAX; ++i)
	{
		ptask=(TaskPtr)taskgroup[i].ptasklist->head;
		while(ptask!=nullptr)
		{
			strcpy(outstr,ptask->name);
			nameLen=strlen(ptask->name)+1;
			outstr+=nameLen;
			len+=nameLen;
			ptask=ptask->pnext;
		}
	}	
	
	return len;
	
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
mlu16 mlos_task_statistics_info(mlu8 *outStatiInfo)
{
	mlu16 len,i;
	TaskPtr ptask;
	TaskDbgInfoptr pDbgInfo;

	pDbgInfo=(TaskDbgInfoptr)outStatiInfo;
	len=0;
	for (i = 0; i < TASK_PRIO_LEVEL_MAX; ++i)
	{
		ptask=(TaskPtr)taskgroup[i].ptasklist->head;
		while(ptask!=nullptr)
		{
			pDbgInfo->id=ptask->id;
			pDbgInfo->prio=ptask->prio;
			pDbgInfo->perSecRunTimes=ptask->perSecRunTimes;//运行次数
			pDbgInfo->runTimeMax=ptask->runTimeMax;//运行时间，单位us
			pDbgInfo->intervalMax=ptask->intervalMax;//运行间隔，间隔多久运行一次，ms
			pDbgInfo->rsv=0;
			pDbgInfo++;
			len+=sizeof(TaskDbgInfo);
			ptask=ptask->pnext;
		}
	}	
	
	return len;
	
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
void task_statistics_reset(void)
{
	mlu8 i;
	TaskPtr ptask;

	for (i = 0; i < TASK_PRIO_LEVEL_MAX; ++i)
	{
		ptask=(TaskPtr)taskgroup[i].ptasklist->head;
		while(ptask!=nullptr)
		{
			ptask->perSecRunTimes=0;//运行时间，单位us
			ptask->runTimeMax=0;//运行时间，单位us
			ptask->runCount=0;//运行次数
			ptask->intervalMax=0;//运行间隔，间隔多久运行一次，ms
			//ptask->lastTime=mlos_ms_clock();//上次进入任务的时间点
			ptask=ptask->pnext;
		}
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
void task_schedule_prepare(void)
{
	mlu8 i=0;
	Taskptr ptsk;
	
	pCurTaskgroup=&taskgroup[0];				//从最高优先级任务组开始运行
	

	//初始化任务的调试统计
	for (i = 0; i < TASK_PRIO_LEVEL_MAX; ++i)
	{
		ptsk=taskgroup[i].ptasklist->head;
		while (ptsk)
		{
			ptsk->runCount=0;
			ptsk->runTimeMax=0;
			ptsk->intervalMax=0;
			ptsk->perSecRunTimes=0;
			ptsk->lastTime=mlos_ms_clock();
			ptsk=ptsk->pnext;
		}
	}

	lastSecondStatisticsTime=mlos_ms_clock();

}
#if TASK_DEBUG_ENABLE
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
_inline void task_statistics_start(Taskptr ptsk)
{
	mlu8 i;
	mlu16 interval;
	interval=mlos_ms_clock()-ptsk->lastTime;				//任务调度间隔
	if (interval>ptsk->intervalMax)
	{
		ptsk->intervalMax=interval;						//任务最大调度间隔
	}
	ptsk->lastTime=mlos_ms_clock();								//记录当前调度的时间点
	ptsk->runCount++;											//任务运行次数

	//运行次数的统计
	//初始化任务的调试统计
	if((mlos_ms_clock()-lastSecondStatisticsTime)>1000)
	{
		for (i = 0; i < TASK_PRIO_LEVEL_MAX; ++i)
		{
			ptsk=taskgroup[i].ptasklist->head;
			while (ptsk)
			{
				ptsk->perSecRunTimes=ptsk->runCount;			//每秒的运行次数
				ptsk->runCount=0;
				ptsk=ptsk->pnext;
			}
		}
	}
	lastSecondStatisticsTime=mlos_ms_clock();	
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
_inline void task_statistics_end(Taskptr ptsk)
{
	mlu16 elapse;	
	elapse=mlos_ms_clock()-ptsk->lastTime;//task_clock_stop(); 									//任务运行的时间
	if(elapse>ptsk->runTimeMax) 
	{
		ptsk->runTimeMax=elapse;								//任务运行最大时间
	}

}
#endif

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
void mlos_task_schedule(void)
{
	Taskstate taskSta;

	//任务调度前的准备工作
	task_schedule_prepare();

	//任务调度循环
	while(mltrue)
	{

		//任务调度
		while(pCurTaskgroup->pCurTask==nullptr)
		{
			pCurTaskgroup->pCurTask=pCurTaskgroup->ptasklist->head;
			pCurTaskgroup=pCurTaskgroup->pnext;//轮训下一个任务组
		}

		
		#if TASK_DEBUG_ENABLE
		task_statistics_start(pCurTaskgroup->pCurTask);
		#endif

		//执行当前轮询任务
		taskSta=pCurTaskgroup->pCurTask->run(pCurTaskgroup->pCurTask->args);

		#if TASK_DEBUG_ENABLE
		task_statistics_end(pCurTaskgroup->pCurTask);
		#endif
		
		if(TASK_STA_BLOCKING_YIELD!=taskSta)
		{
			//轮训下一个任务
			pCurTaskgroup->pCurTask=pCurTaskgroup->pCurTask->pnext;
			//任务组轮训完
			if(pCurTaskgroup->pCurTask==nullptr)
			{
				if(pCurTaskgroup==&taskgroup[0])
				{
					pCurTaskgroup->pCurTask=pCurTaskgroup->ptasklist->head;
					pCurTaskgroup=pCurTaskgroup->pnext;//轮训下一个任务组
					continue;//
				}
			}
		}
		else
		{
				//任务阻塞，不往下运行，只能被高优先级的任务抢占
				;
		}

		//高优先级任务，抢占资源
		if(pCurTaskgroup!=&taskgroup[0])
		{
			pCurTaskgroup=&taskgroup[0];
			pCurTaskgroup->pCurTask=pCurTaskgroup->ptasklist->head;
		}
		
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
void mlos_task_init(void)
{
	mlu16 i,next;
	
	for (i = 0; i < TASK_PRIO_LEVEL_MAX; ++i)
	{
		taskgroup[i].pCurTask=nullptr;
		taskgroup[i].ptasklist=list_create(e_mem_sram, e_lst_sortUnordered, e_lst_linkDoubly);//双向链表提高访问速度
		next=(i+1)%TASK_PRIO_LEVEL_MAX;
		taskgroup[i].pnext=&taskgroup[next];
	}

	pCurTaskgroup=taskgroup;
}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif

