/**
 ******************************************************************************
 * 文件:mlos_task.h
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


#ifndef _MLOS_TASK_H_
#define _MLOS_TASK_H_

//---------------------------------------------------------
//include
//---------------------------------------------------------
#include "mlos_dtd.h"
#include "mlos_ltimer.h"
#include "mlos_clock.h"
#include "mlos_mpump.h"
#include "mlos_list.h"
#include "mlos_malloc.h"



//---------------------------------------------------------
//config
//---------------------------------------------------------

//任务调试开关
#define TASK_DEBUG_ENABLE 	(1)

//任务名最大长度，byte
#define TASK_NAME_LEN_MAX 		(20)

//---------------------------------------------------------
//任务优先级
//---------------------------------------------------------
typedef enum{

	TASK_PRIO_HIGHEST=0,//最高优先级
	TASK_PRIO_1,
	TASK_PRIO_2,
	TASK_PRIO_3,
	TASK_PRIO_4,
	TASK_PRIO_5,
	TASK_PRIO_6,
	TASK_PRIO_7,
	TASK_PRIO_8,
	TASK_PRIO_9,
	TASK_PRIO_LOWEST=(TASK_PRIO_LEVEL_MAX-1),//任务最低优先级
	
}TaskPriority;


//---------------------------------------------------------
//任务状态 ，任务返回值 Task Return Value;
//---------------------------------------------------------

typedef enum{

	TASK_STA_BLOCKING_YIELD=0,//任务阻塞放弃，只会执行高优先级任务，低优先级任务被阻塞
	TASK_STA_NONBLOCKING_YIELD,//任务主动放弃
	TASK_STA_DELAY,
	TASK_STA_EXITED,//任务退出，任务中途结束退出
	TASK_STA_ENDED,//任务结束
	TASK_SUSPENSION,//任务挂起
	
}Taskstate;
	
//---------------------------------------------------------
//任务函数指针
//---------------------------------------------------------
typedef Taskstate (*TaskFunction)(void*);


//----------------------------------------------------------------------------
//任务结构体
//----------------------------------------------------------------------------
//#pragma anon_unions  
typedef struct TaskNode{

	//链表属性
	//DoubleLinked，双向链表，
	struct TaskNode *pnext;
	struct TaskNode *pprev;

	mlu8 prio;////任务优先级,数值越小优先级越高，0最高优先级
	mlu8 id;//任务标识，由系统分配
	mlu16 lcl;//local code line，定位代码行号，上次任务挂起退出的代码行号,从此行号开始执行任务代码
	
	//任务指定定时器
	LTimerPtr pTimer;

	//任务函数
	TaskFunction run;

	//任务参数
	void *args;

	//任务的名称
	const char*name;
	
#if TASK_DEBUG_ENABLE //调试开关
	mlu32 runCount;//运行次数
	mlu32 lastTime;//上次进入任务的时间点
	mlu16 intervalMax;//运行最大间隔，间隔多久运行一次，ms
	mlu16 runTimeMax;//运行最大时间，单位ms	
	mlu32 perSecRunTimes;//每秒的运行次数
	
#endif

}Task,*TaskPtr,*Taskptr;

typedef struct{

	mlu32 perSecRunTimes;//运行次数
	mlu16 runTimeMax;//运行最大时间，单位us
	mlu16 intervalMax;//运行最大间隔，间隔多久运行一次，ms
	mlu8 id;
	mlu8 prio;	
	mlu16 rsv;	
		
}TaskDbgInfo,*TaskDbgInfoptr;

//----------------------------------------------------------------------------
//子任务结构体
//----------------------------------------------------------------------------
typedef struct SubtaskNode{


	//local Continue Line;
	//定位代码行号，上次任务挂起退出的代码行号,从此行号开始执行任务代码
	mlu16 lcl;
	mlu16 id;

	LTimerPtr pTimer;//轻量级环回定时器
	
}Subtask,*SubtaskPtr,*Subtaskptr;

//---------------------------------------------------------
//任务声明
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define task_declaration() 	mlu8 yielded


//---------------------------------------------------------
//TASK_BEGIN：任务开始 marco;任务函数编程格式，以调用此宏开始，
//ptsk:任务对象
//mlu8 yielded: 标识任务是否有退出过的一个局部变量，
//		<PS：需要在任务函数定义的局部变量，变量名固定为yielded！！！！！！>
//		任务是否有过放弃退出，每次进入任务，都表示上次有放弃任务退出过;
//任务编程的固定格式：参考u_task.c任务模板 int template_task(void *args)
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define task_begin(ptsk) \
						yielded = 1;\
						if(yielded==1){;};\
						switch (ptsk->lcl){\
						case 0:

//---------------------------------------------------------
//TASK_END：任务结束 marco，任务函数编程格式，以调用此宏结束任务
//ptsk:任务对象
//retState:返回值 TASK_ENDED
//任务编程的固定格式：参考u_task.c任务模板 int template_task(void *args)
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------

#define task_end(ptsk) } 		ptsk->lcl=0; return TASK_STA_ENDED

//---------------------------------------------------------
//TASK_EXIT：任务退出 marco
//ptsk:任务对象
//retState:返回值 TASK_EXITED 返回任务结束退出
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------

#define task_exit(ptsk) 		ptsk->lcl=0;return TASK_STA_EXITED

//---------------------------------------------------------
//TASK_YIELD:任务主动放弃 marco,主动放弃让出mcu资源
//ptsk:任务对象
//retState:返回任务状态，TASK_BLOCKING_YIELD或者TASK_NONBLOCKING_YIELD
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------

#define task_yield(ptsk,retState) 		\
						  do {\
						  	yielded = 0;\
							ptsk->lcl = __LINE__; case __LINE__:\
							if(0 == yielded)\
							{\
								return retState;\
							}\
						  } while(0)

//---------------------------------------------------------
//TASK_WAIT_UNTIL:任务等待 marco,任务等待某个条件满足,直到超时
//ptsk->任务对象
//cnd-等待的条件，当条件为true，继续执行任务
//tmMs-等待超时的时间，单位ms，=0:标识永远等待
//retState:返回任务状态，TASK_BLOCKING_YIELD或者TASK_NONBLOCKING_YIELD
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------

#define task_wait_cnd_until(ptsk,cnd,ms,retState) \
			do {\
				if(nullptr != ptsk->pTimer)\
				{\
					ltimer_load_start(ptsk->pTimer, ms);\
					yielded = 0;\
					ptsk->lcl = __LINE__; case __LINE__:\
					if( (0 == yielded) ||\
					((!cnd)&&(ltimer_ticking(ptsk->pTimer))) )\
					{\
						return retState;\
					}\
				}\
			} while(0)

//---------------------------------------------------------
//TASK_WAIT_cnd:任务等待 marco,任务等待某个条件满足
//ptsk->任务对象
//cnd:等待的条件，当条件为true，继续执行任务
//retState:返回任务状态，TASK_BLOCKING_YIELD或者TASK_NONBLOCKING_YIELD
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------

#define task_wait_cnd(ptsk,cnd,retState) \
			do {\
				yielded = 0;\
				ptsk->lcl = __LINE__; case __LINE__:\
				if(0 == yielded||(!cnd))\
				{\
					return retState;\
				}\
			} while(0)

//---------------------------------------------------------
//task_ms_delay:任务挂起 marco,
//ptsk->任务对象
//ms:任务延时的时间，单位ms，
//retState:返回任务状态，TASK_BLOCKING_YIELD或者TASK_NONBLOCKING_YIELD
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------

#define task_ms_delay(ptsk,ms,retState)		 	\
		do {\
			if(nullptr!=ptsk->pTimer)\
			{\
				ltimer_load_start(ptsk->pTimer, ms);\
				yielded = 0;\
				ptsk->lcl = __LINE__; case __LINE__:\
				if((0 == yielded)||(ltimer_ticking(ptsk->pTimer)))\
				{\
					return retState;\
				}\
			}\
		} while(0)
		
//---------------------------------------------------------
//子任务 接口，功能同于上述，主任务
//子任务不支持任务阻塞，
//子任务声明
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define subtask_declaration() 	mlu8 yielded

#define subtask_reset(psubtsk) 	do{psubtsk->lcl=0;}while(0)

//---------------------------------------------------------
//子任务开始
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define subtask_begin(psubtsk) \
						yielded = 1;\
						if(yielded==1){;}\
						switch (psubtsk->lcl){\
						case 0:

//---------------------------------------------------------
//子任务结束，返回结果,子任务的返回结果可以是任意数据类型
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------

#define subtask_end(psubtsk,retval) } psubtsk->lcl=0; return retval


//---------------------------------------------------------
//子任务主动放弃退出，返回结果
//子任务的返回结果可以是任意数据类型,
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define subtask_yield(psubtsk,retval) 				\
			do {\
				yielded = 0;\
				psubtsk->lcl = __LINE__; case __LINE__:\
				if(0 == yielded) {return retval;}\
			} while(0)


//---------------------------------------------------------
//子任务退出
//子任务的返回结果可以是任意数据类型,
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define subtask_exit(psubtsk,retval) psubtsk->lcl=0;return retval

//---------------------------------------------------------
//子任务等待条件直到超时，
//子任务的返回结果可以是任意数据类型,
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define subtask_wait_cnd_until(psubtsk,cnd,ms,retval) \
			do {\
				if(nullptr!=psubtsk->pTimer)\
				{\
					ltimer_load_start(psubtsk->pTimer, ms);\
					yielded = 0;\
					psubtsk->lcl = __LINE__; case __LINE__:\
					if((0 == yielded)||\
					((!cnd)&&(ltimer_ticking(psubtsk->pTimer))) )\
					{\
						return retval;\
					}\
				}\
			} while(0)

//---------------------------------------------------------
//子任务等待条件阻塞退出，直到条件为true，继续执行任务
//子任务的返回结果可以是任意数据类型,
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define subtask_wait_cnd(psubtsk,cnd,retval) \
			do {\
				yielded = 0;\
				psubtsk->lcl = __LINE__; case __LINE__:\
				if(0 == yielded||(!cnd))\
				{\
					return retval;\
				}\
			} while(0)

//---------------------------------------------------------
//子任务延时
//子任务的返回结果可以是任意数据类型,
//修改履历：
//1.zzx ：  2014-12-01  ：创建
//
//---------------------------------------------------------
#define subtask_ms_delay(psubtsk,ms,retval) \
			do {\
				if(nullptr!=psubtsk->pTimer)\
				{\
					ltimer_load_start(psubtsk->pTimer, ms);\
					yielded = 0;\
					psubtsk->lcl = __LINE__; case __LINE__:\
					if((0 == yielded)||\
					(ltimer_ticking(psubtsk->pTimer)))\
					{\
						return retval;\
					}\
				}\
			} while(0)


//---------------------------------------------------------
//extern function
//---------------------------------------------------------

//任务
void mlos_task_init(void);


//创建任务
Taskptr task_create(TaskPriority prio,TaskFunction tf,void*args,LTimerPtr tskTmr,char* taskName);
Subtaskptr subtask_create(LTimerPtr tskTmr);

//任务调度
void mlos_task_schedule(void);


//调试接口
#if TASK_DEBUG_ENABLE
void task_statistics_reset(void);
mlu16 mlos_task_statistics_info(mlu8 *outStatiInfo);
mlu16 mlos_all_task_name(mlu8 *outNames);

#endif


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif // U_TASK_H_INCLUDED


