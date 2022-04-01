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

extern MLBool ds_tray_write_chache(mlu8 traAddr,mlu8 datNum,mlu8*const pData);
extern MLBool ds_tray_subtask(void);
extern MLBool ds_tray_file_seek_end(mlu8 trayAddr,mlu32*curWriteLine);//文件移到末尾
extern mlu8 ds_tray_file_read_lines(mlu8 traAddr, mlu32 startline,mlu8 lineNum,mlu8*outDatBuf);//行读取文件数据
//----------------------------------------------------------------------------
//文件不被编译，需要声明以下
//----------------------------------------------------------------------------
#else 

#define ds_tray_init( trayNum, version, baseDatSize, datNum)
#define ds_tray_write_chache(traAddr,pData)		mlfalse
#define ds_tray_subtask() 					mlfalse

#endif 				//文件条件编译结束
//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 





