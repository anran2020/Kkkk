/**
 ******************************************************************************
 * 文件:combine_box.c
 * 作者:   
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *				
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

#if COMBINE_ENABLE


//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "mlos.h"

#include "combine_box.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// global variable 
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
//
//
//-----------------------------------------------------------------------------
MLBool cbox_offline(CmbPwrBoxptr pCBox)
{



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
MLBool cbox_working(CmbPwrBoxptr pCBox)
{
	subtask_declaration();

	subtask_begin()	


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
MLBool cbox_run(CmbPwrBoxptr pCBox)
{
	MLBool bRunning=mltrue;

	while (bRunning)
	{	
		
		//fsm，运行
		switch(pCBox->sta)
		{
			case BOX_STA_OFFLINE:					//离线，通讯异常
			bRunning=cbox_offline(pCBox);
			break;

			case BOX_STA_WORKING:					//工作状态
			bRunning=cbox_working(pCBox);
			break;

		}
	}

	return mltrue;
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
void cbox_init(CmbPwrBoxptr pCBox)
{

	//tbl.pMyMainTask=task_create(TBL_TASK_PRIO, tbl_task, nullptr, ltimer_create(), "tbl");
	
	pCBox->pnext=nullptr;

}


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif






