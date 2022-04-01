/**
 ******************************************************************************
 * 文件:base_box.h
 * 作者:   
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *		设备箱体的基础结构体，可被不用类型的设备箱体继承，
 *    提到代码的
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
#if 1

#ifndef  _BASE_BOX__H__
#define  _BASE_BOX__H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"

//----------------------------------------------------------------------------
//box 状态机 
//----------------------------------------------------------------------------
enum BoxState{

	BOX_STA_OFFLINE=0,			//离线状态
	BOX_STA_WORKING,			//工作状态
	BOX_STA_CALIBRATION,		//修调状态
	BOX_STA_UPGRADE,			//升级状态

};


//----------------------------------------------------------------------------
//电源箱体的基结构体，即所有类型的电源箱体的父结构体，继承接口
//----------------------------------------------------------------------------
#define PowerBoxBaseStruct(SonBoxType) 		struct {\
\
	struct SonBoxType*pnext;/*指向下一个箱体，具备单向链表节点的属性*/\
	mlu8 type;/*设备类型*/\
	mlu8 addr;/*设备地址*/\
	mlu8 commx;/*通讯端口*/\
	mlu8 swv;/*软件版本*/\
	mlu8 sta;/*状态*/\
	mlu8 chnlNum;/*通道数*/\
	mlu8 mldNum;/*硬件模块数*/\
	mlu8 auxiBoxNum;/*辅助设备箱体，例如，cv旁路板、电压采样板*/\
	mlu32 currentRange;/*通道的电流电压量程*/\
	mlu32 voltageRange;/*通道的电流电压量程*/\
}


typdef PowerBoxBaseStruct(PowerBoxBaseObject)	PowerBox;  //电源箱父结构体 

//----------------------------------------------------------------------------
//export vari
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif



