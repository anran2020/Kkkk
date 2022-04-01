/**
 ******************************************************************************
 * 文件:combine_module.h
 * 作者:   
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
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

#ifndef  _COMBINE_MODULE__H__
#define  _COMBINE_MODULE__H__
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if COMBINE_ENABLE

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"
#include "mlos_list.h"


//----------------------------------------------------------------------------
//define 
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//并机的硬件模块，在逻辑上对应下位机的一个通道，
//----------------------------------------------------------------------------
typedef struct CombineModuleListItem{
						
	STRUCT_INHERIT(CombineModuleListItem,SLListItemBaseStruct);   //继承单向链表item属性

	//运行参数
	mlu32 iDA;//电流设定值
	mlu32 vDA;//电压设定值
	mlu32 time;	//运行时间
	

	//采样数据
	mls32 current;
	mls32 voltage;
	
	void * pMyCBox;				//指向所属箱体
	void * pMyCChnl;			//指向所属通道

}CombinePowerModule,*CmbPwrMdlptr;

//----------------------------------------------------------------------------
//export vari
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------

void cmdl_init(CmbPwrMdlptr pCMdl);


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif




