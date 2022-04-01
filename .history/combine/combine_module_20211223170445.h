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
//并机的模块，在物理层上对应下位机的一个通道，
//----------------------------------------------------------------------------
typedef struct CombineModuleListItem{
						
	STRUCT_INHERIT(CombineModuleListItem,SLListItemBaseStruct);   //继承单向链表item属性

	mlu8 boxAddr;//所属box设备地址
	mlu8 comx;//通讯端口
	mlu8 addr;//我的地址
	mlu8 DDC;//采样数据 
	

	//运行参数
	mlu8 stepType;
	mlu32 iDA;//电流设定值
	mlu32 vDA;//电压设定值
	mlu32 time;	//运行时间
	
	//采样数据
	mls32 current;
	mls32 voltage;
	
}CombinePowerModule,*CmbPwrMdlptr;

//----------------------------------------------------------------------------
//export vari
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------

void cmbmdl_init(CmbPwrMdlptr pMdl,mlu8 comx,mlu8 boxAddr);


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif




