/**
 ******************************************************************************
 * 文件:mlos_rtimer.h（实时定时器）
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *	rtimer->real-time timer 实时定时器
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1         2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef _MLOS_RTIMER_H_
#define _MLOS_RTIMER_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "mlos_dtd.h"
#include "mlos_list.h"
#include "mlos_malloc.h"
#include "mlos_clock.h"



//---------------------------------------------------------
//defines
//---------------------------------------------------------

//定时器任务优先级
#define RTMR_TASK_PRIO 				(TASK_PRIO_HIGHEST)//最高优先级保证定时器的实时性


//定时器中断服务函数
typedef void (*RTimerIsr)(void*);


//定时器加载模式
typedef enum {

	e_rtmr_loadManu=0,//手动重载
	e_rtmr_loadAuto,//自动重载

}RTimerLoadMode;

//----------------------------------------------------------------------------
//定时器结构体
//----------------------------------------------------------------------------
//#pragma anon_unions
typedef struct RTimerNode{

	//双向有序链表
	struct RTimerNode *pnext;
	struct RTimerNode *pprev;
	ListSortkey endTime;//定时终点,排序

	//定时器标识
	mlu8 running;
	mlu8 reloadMode;
	mlu8 timeout;
	mlu8 id;

	//定时周期
	mlu32 period;

	//中断服务函数
	RTimerIsr isr;

	//中断函数参数
	void*args;

	//定时器所在链表
	ListPtr pMyTimerList;
	
}RTimer,*RTimerPtr;


//----------------------------------------------------------------------------
//extern
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//extern function
//----------------------------------------------------------------------------

void rtimer_init(RTimerPtr ptmr,mlu32 msPeriod,RTimerIsr isr,void *args,RTimerLoadMode loadMode);
RTimerPtr rtimer_create(mlu32 msPeriod,RTimerIsr isr,void *args,RTimerLoadMode loadMode);
void rtimer_task_create(void);
void rtimer_start(RTimerPtr ptmr);
void rtimer_stop(RTimerPtr ptmr);


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif //


