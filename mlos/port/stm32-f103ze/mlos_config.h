/**
 ******************************************************************************
 * 文件:mlos_config.h
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

#ifndef  _MLOS_CONFIG_H__
#define  _MLOS_CONFIG_H__
 
//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// task 任务优先级划分
//----------------------------------------------------------------------------

#define TASK_STACK_DEPTH_MAX 		10



//----------------------------------------------------------------------------
// 参数断言
//----------------------------------------------------------------------------

#define MLOS_ASSERT_ENABLE 			(1)

//----------------------------------------------------------------------------
// log service 日志服务
//----------------------------------------------------------------------------

#define OS_LOG_ENABLE 	(0)

#define LOG_CACHE_SIZE 		1024


//----------------------------------------------------------------------------
// shell service shell服务
//----------------------------------------------------------------------------

//shell开关
#define OS_SHELL_ENABLE 	(0)


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
