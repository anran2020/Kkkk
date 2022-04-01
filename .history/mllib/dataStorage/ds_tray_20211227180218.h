/**
 ******************************************************************************
 * 文件:ds_tray.h
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


#ifndef _DS_TRAY_H_
#define _DS_TRAY_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------
#include "ds_config.h"


//---------------------------------------------------------
//文件条件编译
//---------------------------------------------------------

#if  (DS_TRAY_EN&&DS_EN)

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "ds_file.h"


//---------------------------------------------------------
//define
//---------------------------------------------------------

#define DS_TRAY_CACHE_NUM 		(4)//每个通道最大缓存四条



//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//extern maroc
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//extern function
//----------------------------------------------------------------------------

extern void ds_tray_file_init(mlu8 trayNum,mlu8 version,mlu32 baseDatSize,mlu32 datNum);

extern mlu8* ds_tray_disk_chache(mlu8 traAddr);
MLBool ds_tray_write_chache(mlu8 traAddr,mlu8* pData);

extern MLBool ds_tray_subtask(void);

//----------------------------------------------------------------------------
//文件不被编译，需要声明以下
//----------------------------------------------------------------------------
#else 

#define ds_tray_init()
#define ds_tray_disk_chache(traAddr)		nullptr
#define ds_tray_subtask() 					mlfalse

#endif 				//文件条件编译结束
//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 





