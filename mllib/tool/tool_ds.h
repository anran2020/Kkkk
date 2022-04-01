/**
 ******************************************************************************
 * 文件:tool_ds.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *		
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----日期   ---作者    ----------   说明-------------------------------------
 * v0.1     2014-12-01   zzx           创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef  _TOOL_DS_H__
#define  _TOOL_DS_H__

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"tool_config.h"
#if ((TOOL_ENABLE)&&(TOOL_DS_ENABLE))

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------


//协议对文件的操作
#define DSF_READ_HEADER 			1 				//读文件头
#define DSF_READ_DATA 				2				//读文件数据
#define DSF_CLEAR 					3				//清除文件内容


//----------------------------------------------------------------------------
//  
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

void tool_ds_init(void);

#else

#define tool_ds_init()

#endif
//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------

#endif


