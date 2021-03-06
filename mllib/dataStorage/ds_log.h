/**
 ******************************************************************************
 * 文件:ds_log.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1      2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/


#ifndef _DS_LOG_H_
#define _DS_LOG_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------


//---------------------------------------------------------
//文件条件编译
//---------------------------------------------------------

#if  (DS_LOG_EN && DS_EN)

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "ds_file.h"


//---------------------------------------------------------
//define
//---------------------------------------------------------




//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//extern maroc
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//extern function
//----------------------------------------------------------------------------

void ds_log_init(void);
void ds_log_subtask(void);



//----------------------------------------------------------------------------
//文件不被编译，需要声明以下
//----------------------------------------------------------------------------
#else 

#define ds_log_init()
#define ds_log_subtask() 


#endif 				//文件条件编译结束
//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 






