/**
 ******************************************************************************
 * 文件:combine.h
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


#ifndef  _COMBINE__H__
#define  _COMBINE__H__
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if COMBINE_ENABLE
//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "cmb_box.h"



//----------------------------------------------------------------------------
//define 
//----------------------------------------------------------------------------

#define CBL_TASK_PRIO               (3)             //并机任务的优先级

//----------------------------------------------------------------------------
//export vari
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------

extern void cbl_init(void);
extern void cbl_power_box_register(CmbPwrBoxptr pBox);
extern CmbPwrBoxptr cbl_power_box_get(mlu8 addr);

//----------------------------------------------------------------------------
//文件不被编译，需要声明的导出接口
//----------------------------------------------------------------------------
#else

#define cbl_init() 

#endif
//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif


