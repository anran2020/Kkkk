/**
 ******************************************************************************
 * 文件:tray_bl.c
 * 作者:   
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *				tray_business_logic
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1        
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"
#if (TRAY_ENABLE)

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "mlos.h"
#include "entry.h"
#include "tray_bl.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

typedef struct 
{

	Taskptr 	pMyMainTask;			//托盘业务逻辑主任务
		
}TrayBusinessLogic,TBL;

//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------


TrayBusinessLogic trayBl;





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
//
//
//-----------------------------------------------------------------------------
Taskstate tbl_task(void*args)
{
	task_declaration();

	task_begin(trayBl.pMyMainTask);

		//58
	ctrlTimerTask();


	task_end(trayBl.pMyMainTask);
	
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
//
//
//-----------------------------------------------------------------------------
void tbl_init(void)
{

	trayBl.pMyMainTask=task_create(TBL_TASK_PRIO,tbl_task,nullptr,ltimer_create(),"traBL");

	Pb_init();// 

	ctrlInitApp();


}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif




