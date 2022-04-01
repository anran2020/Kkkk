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

//文件句柄
typedef enum{
    e_dsf_tray1=0,
    e_dsf_tray2,
    e_dsf_tray3,
    e_dsf_tray4,
}DSFilileHandle;

void ds_tray_file_init(mlu8 trayNum,mlu8 version,mlu32 baseDatSize,mlu32 datNum);

MLBool ds_file_valid_data_s(DSFilileHandle fhdl,mlu32*startSeq,mlu32*endSeq);//获取文件有效数据的区间

MLBool ds_write_file(DSFilileHandle fhdl,mlu8 datNum,mlu8*datBuf);//返回true，写入成功，返回false，写入失败

mlu8 ds_read_file(DSFilileHandle fhdl,mlu32 seq,mlu8 num,mlu8*datBuf);//返回读取的条数

mlu8 ds_file_clear(DSFilileHandle fhdl);//清除文件内容

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





