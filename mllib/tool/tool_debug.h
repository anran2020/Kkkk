/**
 ******************************************************************************
 * 文件:tool_debug.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *		
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----日期   ---作者    ----------   说明-------------------------------------
 * v0.1     2014-12-01   zzx           创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef  __TOOL_DEBUG_H__
#define  __TOOL_DEBUG_H__

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"tool_config.h"
#if ((TOOL_ENABLE)&&(TOOL_DEBUG_ENABLE))

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

#define TOOL_DEBUG_TASK_PRIO 			(TASK_PRIO_LOWEST)


//----------------------------------------------------------------------------
//  工具调试打印功能枚举
//----------------------------------------------------------------------------
typedef enum {

	e_dbg_print_null=0,
	e_dbg_print_taskinfo=0x00000001,

}ToolDebgPrintFuncation;



//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

void tool_debug_int(void);
void tool_debug_task(void );

#else

#define tool_debug_int()
#define tool_debug_task()

#endif
//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
