/**
 ******************************************************************************
 * 文件:combine_config.h
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



#ifndef  _COMBINE_CONFIG__H__
#define  _COMBINE_COFIG__H__
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if COMBINE_ENABLE
//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"



//----------------------------------------------------------------------------
//define 
//----------------------------------------------------------------------------

//并机最大电源箱数
#define  CBL_PWR_BOX_MAX             (32)


#define  CBL_PWR_BOX_CHNL_MAX             (8)//单箱，最大通道数，

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



