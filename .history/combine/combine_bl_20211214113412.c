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
#include "entry.h"

#include "combine_box.h"
#include "combine_bl.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

typedef struct 
{
	mlu8 trayCount;


	//设备资源声明
	 CmbPwrBoxptr pwrBoxBase;		//并机电源箱资源定义
	 CmbPwrChnlptr pwrChnlBase;		//并机通道
	 CmbPwrMdlptr pwrMdlBase; 		//并机硬件模块

	// TmprBoxptr 	tmprBox;					//温度箱
	// VolBoxptr	volBox;						//电压采样箱
	// CVBoardptr  cvBoard;					//恒压板  

	//can线并发管理，设备
	 ListPtr pPwrBoxList[CBL_CAN_LINE_NUM];		//根据CAN通讯接口独立，建立box链表

	 CmbPwrBoxptr pCurSmplPowerBox[CBL_CAN_LINE_NUM];		//当前采样		电源箱		

	// //Trayptr pCurPollTray;						//当前轮询托盘
	// Subtaskptr pPowBoxSmplSubtask[CBL_CAN_LINE_NUM];				//电源箱采样子任务

	Taskptr pMyMainTask;						//主任务

	Subtaskptr 	pTmprSmplSubtask;														//温度采样子任务

	CmbPwrBoxptr pwrBoxBase[CBL_CAN_LINE_NUM][CBL_CAN_LINE_BOX_MAX];		//电源箱表格
		
}CombineBusinessLogic,CBL;

//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------

//并机的业务逻辑
CombineBusinessLogic cbl={0};

//电源箱
CmbPwrBoxptr 

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
CmbPwrBoxptr cbl_power_box_get(mlu8 CANLine,mlu8 addr)
{
	if(CANLine>=CBL_CAN_LINE_NUM||addr>=CBL_CAN_LINE_BOX_MAX)
	return nullptr;

	return cbl.pwrBoxBase[CANLine][addr];
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
void cbl_power_box_run(mlu8 CANx)
{
	if(cbl.pCurSmplPowerBox[CANx]=nullptr)
		cbl.pCurSmplPowerBox[CANx]=cbl.pPwrBoxList[CANx]->head;

	if(cbl.pCurSmplPowerBox[CANx]!=nullptr)
	{
		//轮询
		if(mlfalse==cbox_run(cbl.pCurSmplPowerBox[CANx]))
		{
			//轮询下一个箱
			cbl.pCurSmplPowerBox[CANx]=cbl.pCurSmplPowerBox[CANx]->pnext;
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
//
//
//-----------------------------------------------------------------------------
Taskstate cbl_task(void*args)
{
	task_declaration();
	mlu8 i;
	
	task_begin(cbl.pMyMainTask);


	//不同的can线并发运行，电源箱
	for (i = 0; i < CBL_CAN_LINE_NUM; ++i)
	{
		cbl_power_box_run(i);			//采样子任务
	}

	task_end(cbl.pMyMainTask);
	
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

	cbl.pMyMainTask=task_create(CBL_TASK_PRIO, cbl_task, nullptr, ltimer_create(), "cbl");


}


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif





