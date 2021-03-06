/**
 ******************************************************************************
 * 文件:combine.c
 * 作者:   
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *	cbl:combine_business_logic 并机业务逻辑的缩写
 *	实现 中位机并机 业务应用，
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

#include "combine_config.h"
#include "combine_bl.h"
#include "combine_ .h"
//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

typedef struct 
{

	CmbPwrBoxptr pCurPwrBox;//当前运行的电源箱

	 ListPtr pwrBoxList;		//电源箱管理链表

	Taskptr pMyMainTask;						//并机业务，主任务
	
}CombineBusinessLogic,*CBLptr;

//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------

//并机的业务逻辑
CombineBusinessLogic* cbl=nullptr;

//电源箱指针&地址映射表，按地址可以直接索引到box
CmbPwrBoxptr cmbPwrBoxTbale[CMB_PWR_BOX_MAX]={0};


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
void cbl_power_box_register(CmbPwrBoxptr pPwrBox)
{
	MLOS_ASSERT(pPwrBox==nullptr);

	cmbPwrBoxTbale[pPwrBox->addr]=pPwrBox;//地址映射表

	list_append(cbl->pwrBoxList,pPwrBox);//插入设备链表

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
CmbPwrBoxptr cbl_power_box_run(CmbPwrBoxptr pPwrBox)
{

	return pPwrBox;
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
Taskstate cbl_task(void*args)
{
	task_declaration();//任务声明
	mlu8 i;
	//每次都需要执行的代码，add here what you want
	if(cbl->pCurPwrBox==nullptr)
	{
		cbl->pCurPwrBox=cbl->pwrBoxList->head;//从链表头开始运行box
	}

	task_begin(cbl->pMyMainTask);//任务开始
	//任务开始时，需要执行的代码，只执行一次，add here what you want

	//任务主循环
	while (mltrue)
	{

		cbl_power_box_run(cbl->pCurPwrBox);
		task_yield(cbl->pMyMainTask,TASK_STA_NONBLOCKING_YIELD);//任务非阻塞退出，让出cpu资源
	}
	
	task_end(cbl->pMyMainTask);
	
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
//	cbl:Combine Business Logic 并机业务逻辑的缩写
//
//修改履历：
//
//
//-----------------------------------------------------------------------------
void cbl_init(void)
{
	cbl=mlos_malloc(e_mem_sram,sizeof(CombineBusinessLogic));//分配业务逻辑内存

	cbl->pwrBoxList=list_create(e_mem_sram,e_lst_sortUnordered,e_lst_linkSingly);//单向链表管理电源箱

	cbl->pMyMainTask=task_create(CBL_TASK_PRIO, cbl_task, nullptr, ltimer_create(), "cbl");//穿件任务

	cbl->pCurPwrBox=nullptr;


	cbl_ctp_init();//can协议初始化
}


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif





