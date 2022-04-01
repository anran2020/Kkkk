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
#include "combine_config.h"

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
CmbPwrBoxptr pwrbox_create(mlu8 comx,mlu8 addr,mlu8 chnlNum)
{
	CmbPwrBoxptr pbox;

	if(comx>=CBL_CAN_NUM||addr>=CBL_CAN_LINE_BOX_MAX||chnlNum>=CBL_PWR_BOX_CHNL_MAX)
	{
		return nullptr;
	}

	pbox=mlos_malloc(e_mem_sram,sizeof(CombinePowerBox));//箱分配内存

	for (mlu8 i = 0; i < chnlNum; i++)
	{
		/* code */
		pbox->chnl=mlos_malloc(e_mem_sram,sizeof(CombinePowerBox));
		pbox->mdl=
	}
	
	//模块分配内存
	//通道分配内存
	

	return pbox;
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
MLBool pwrbox_offline(CmbPwrBoxptr pCBox)
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
MLBool pwrbox_working(CmbPwrBoxptr pCBox)
{
	subtask_declaration();

	subtask_begin(pCBox->pMytask);

	while(mltrue)
	{
		



	}

	subtask_end(pCBox->pMytask,mltrue);	


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






