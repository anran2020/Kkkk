/**
 ******************************************************************************
 * 文件:mlos.h
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


#ifndef _MLOS_H_
#define _MLOS_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "mlos_dtd.h"

//os
#include "mlos_task.h"
#include "mlos_clock.h"
#include "mlos_ltimer.h"
#include "mlos_rtimer.h"
#include "mlos_mpump.h"
#include "mlos_list.h"
#include "mlos_malloc.h"
#include "mlos_que.h"

//service
#include "mlos_log.h"
#include "mlos_shell.h"
//#include "mlos_tps.h"

//---------------------------------------------------------
//defines
//---------------------------------------------------------

//----------------------------------------------------------------------------
// 系统 版本号
//----------------------------------------------------------------------------

//系统版本
#define MLOS_VERSION 		(1)


//---------------------------------------------------------
//define
//---------------------------------------------------------

//系统信息
typedef struct{

	//
	mlu8 ver;//版本
	mlu8 taskID;
	mlu16 subtaskID;
	//
	mlu8 taskPrioLevel;//优先等级划分
	mlu8 u8rsv2;
	mlu8 u8rsv3;
	//
	mlu8  pumpConut;//消息泵计数
	mlu16 msgConut;//消息计数
	mlu16 consumerCount;//消息消费者计数
	

	//
	mlu16 ltimerCount;//定时器的个数
	mlu16 rtimerCount;


	//
	mlu32 memSize;//内存总大小
	mlu32 usedMemSize;//已使用的内存大小
	

}MLOSInfo,*MLOSInfoptr;

//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------


extern MLOSInfo mlos;

//----------------------------------------------------------------------------
//exprot inline function
//----------------------------------------------------------------------------

//运行操作系统
_inline static void mlos_run(void)
{

	//开始任务调度
	mlos_task_schedule();

}

//----------------------------------------------------------------------------
//exprot function
//----------------------------------------------------------------------------

//初始化操作系统
void mlos_init(void);

//退出操作系统
void mlos_exit(void);


void mlos_mem_copy(unsigned char * des,unsigned char*src,int len);


//参数断言接口
#if MLOS_ASSERT_ENABLE
void mlos_assert_failed(char* file, mlu16 line);
#define MLOS_ASSERT(expr) ((expr) ? mlos_assert_failed((char *)__FILE__,(mlu16) __LINE__):(void)0)
#else
  #define MLOS_ASSERT(expr) ((void)0)
#endif


//软件调试断点接口
#if MLOS_BKPT_ENABLE
void mlos_bkpt(MLBool bBreak,char *file,char *func,mlu16 line);

#define MLOS_BKPT(bBreak) 		mlos_bkpt(bBreak,(char *)__FILE__,(char *)__FUNCTION__,(mlu16)__LINE__)
#else

#define MLOS_BKPT(bBreak) 		

#endif


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif // 




