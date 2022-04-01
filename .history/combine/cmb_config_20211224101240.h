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
#define  CMB_PWR_BOX_MAX             (31)



//通道缓存数据: 缓存多少个数据、一个数据的长度
#define   CMB_CH_DAT_RCRD_Cnt 		(100)
#define   Igbt_ChnlOneDataRec_Len 	 	(32)




//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif



