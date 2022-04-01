/**
 ******************************************************************************
 * 文件:mlos_default_config.h
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

#ifndef  _MLOS_DEFAULT_CONFIG_H__
#define  _MLOS_DEFAULT_CONFIG_H__
 
//---------------------------------------------------------
//include
//---------------------------------------------------------
#include "mlos_config.h"


//----------------------------------------------------------------------------
// task 任务优先级划分
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// task 任务优先级划分
//----------------------------------------------------------------------------

//任务优先级，等级配置，
#ifndef TASK_PRIO_LEVEL_MAX
#define TASK_PRIO_LEVEL_MAX 		10 //默认10个优先等级，0-9，0优先级最高，9优先级最小
#endif

//----------------------------------------------------------------------------
// 参数断言
//----------------------------------------------------------------------------

#ifndef  MLOS_ASSERT_ENABLE
#define MLOS_ASSERT_ENABLE 			(1)
#endif 

//----------------------------------------------------------------------------
// 调试软件断点
//----------------------------------------------------------------------------

#ifndef  MLOS_BKPT_ENABLE
#define MLOS_BKPT_ENABLE 			(0)
#endif


//----------------------------------------------------------------------------
// log service 日志服务
//----------------------------------------------------------------------------

//log 开关
#ifndef OS_LOG_ENABLE
#define OS_LOG_ENABLE 	(0)
#endif 

//任务优先级
#ifndef LOG_TASK_PRIO
#define LOG_TASK_PRIO 		TASK_PRIO_LOWEST
#endif

// shell cmd 缓存区大小
#ifndef LOG_CACHE_SIZE
#define LOG_CACHE_SIZE 		1024
#endif


//----------------------------------------------------------------------------
// shell service shell服务
//----------------------------------------------------------------------------

//shell开关
#ifndef OS_SHELL_ENABLE
#define OS_SHELL_ENABLE 	(0)
#endif 

//任务优先级
#ifndef SHELL_TASK_PRIO
#define SHELL_TASK_PRIO 		TASK_PRIO_LOWEST
#endif

// shell cmd 缓存区大小
#ifndef SHELL_CACHE_SIZE
#define SHELL_CACHE_SIZE 		1024
#endif


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif




