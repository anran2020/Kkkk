/**
 ******************************************************************************
 * 文件:combine_box.h
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

#ifndef  _COMBINE_BOX__H__
#define  _COMBINE_BOX__H__

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if COMBINE_ENABLE

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"
#include "combine_channel.h"
#include "base_box.h"
#include "mlos_list.h"
#include "combine_module.h"
#include "mlos_task.h"

//----------------------------------------------------------------------------
//define 
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//并机设备的电源箱体
//----------------------------------------------------------------------------
typedef struct CombinePowerBoxItem{

	STRUCT_INHERIT(CombinePowerBoxItem,SLListItemBaseStruct);		//继承单向链表节点属性
	STRUCT_INHERIT(CombinePowerBoxItem,PowerBoxBaseStruct);			//CombinePowerBoxItem继承电源箱父结构体PowerBoxBaseStruct

	CmbPwrChnlptr  chnl;			//箱体的通道，指针数组，创建集体通道数据，指向数组的基地址
	CmbPwrMdlptr  mdl;			//	箱体的硬件模块，即一个或多个模块组成一个通道

	Subtaskptr pMytask;
}CombinePowerBox,*CmbPwrBoxptr;


//----------------------------------------------------------------------------
//export vari
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------

extern void cbox_init(CmbPwrBoxptr pCBox);
extern MLBool cbox_run(CmbPwrBoxptr pCBox);

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif



