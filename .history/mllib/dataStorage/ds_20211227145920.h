/**
 ******************************************************************************
 * 文件:ds_.h
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


#ifndef _DATA_STORAGE_H_
#define _DATA_STORAGE_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------
#include "ds_config.h"

//---------------------------------------------------------
//文件条件编译
//---------------------------------------------------------

#if  DS_EN

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "mlos_malloc.h"
#include "ds_log.h"
#include "ds_tray.h"


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

void ds_init(void);

mlu32 

//----------------------------------------------------------------------------
//文件不被编译，需要声明以下
//----------------------------------------------------------------------------
#else 

#define ds_init() 		(void)0



#endif 				//文件条件编译结束
//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 





