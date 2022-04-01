/**
 ******************************************************************************
 * 文件:ds_file.h
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


#ifndef _DS_FILE_H_
#define _DS_FILE_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------


//---------------------------------------------------------
//文件条件编译
//---------------------------------------------------------

#if  DS_EN

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "mlos_dtd.h"

//---------------------------------------------------------
//#define
//---------------------------------------------------------

//文件版本
typedef mlu8 DSFileVersion;  

//文件头标识
#define DS_FILE_HEADER_FLAG1 				0x5A
#define DS_FILE_HEADER_FLAG2 				0xA5


//
#define DS_FILE_LINE_NULL 			0xffffffff//空行号，不执行，文件的行写入

//---------------------------------------------------------
//数据存储的文件头 分区备份
//---------------------------------------------------------
enum DSFileBackup{

	DS_FILE_BACKUP_A=0,
	DS_FILE_BACKUP_B,
	DS_FILE_BACKUP_PAGE_NUM,				//用来备份文件页数
};

//---------------------------------------------------------
//数据存储文件类型
//---------------------------------------------------------
enum DSFileType{

	e_dsft_log=1,					//日志文件	
	e_dsft_trayData,				//托盘数据文件

};

//---------------------------------------------------------
//数据存储的文件头定义
//---------------------------------------------------------
typedef struct{

	//固定字段
	mlu8 flag1;						//文件类型	
	mlu8 version;					//文件版本
	mlu8 coveredWriting;			//文件已经循环覆盖写入
	mlu8 modifyTimes;				//文件修改次数
	mlu32 offsetLine;				//当前写入偏移行号
	//mlu32 offsetPage;

	//可变字段
	//add here what you want
	mlu32 lastReadLine;//记录上次读取的行号，文件按行读写
	mlu8 alignRsv[3];			//4byte 对齐 预留，后续可使用
	//固定字段
	mlu8 flag2;				//文件类型

}DSFileHeader,*DSFileHeaderPtr;

//---------------------------------------------------------
//数据存储的文件定义
//---------------------------------------------------------
typedef struct{


	DSFileHeader header; 				//文件头

	mlu8 bOpened;						//文件有效打开标识
	mlu8 headerBackupAB;				//当前文件头备份分区
	mlu8 linesPerPage; 					//行每页，
	mlu8 pagesPerLine; 					//页每行，
	
	mlu8 linePageIndx;				//一行数据的页索引
	mlu8 hdl;
	mlu8 alignRsv[3];			//4byte 对齐 预留，后续可使用	
	
	mlu32 basePage;						//文件头基地址
	mlu32 endPage;						//文件页数
	mlu32 lineMax;						//文件最大行数
	
	mlu32 curDataPage;					//当前写入数据页地址

}DSFile,*DSFilePtr;

//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//extern maroc
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//extern function
//----------------------------------------------------------------------------

MLBool dsf_open(DSFilePtr dsfile,DSFileVersion ver);
void  dsf_close(DSFilePtr dsfile);	
MLBool dsf_backup(DSFilePtr dsfile);
MLBool dsf_write_page(DSFilePtr dsfile,mlu8* pdat);
mlu8* dsf_read_page(DSFilePtr dsfile,mlu32 pageAddr);
MLBool dsf_clear(DSFilePtr dsfile);

MLBool dsf_write_line(DSFilePtr dsfile,mlu32 line,mlu8* pdat);//文件行写入

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 				//文件条件编译结束
#endif 


