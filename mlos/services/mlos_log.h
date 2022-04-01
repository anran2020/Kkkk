/**
 ******************************************************************************
 * 文件:mlos_log.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                       2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/


#ifndef _MLCOS_LOG_H_
#define _MLCOS_LOG_H_

//----------------------------------------------------------------------------
//条件编译 #if
//----------------------------------------------------------------------------
#include "mlos_default_config.h"

#if (OS_LOG_ENABLE)

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "mlos_dtd.h"
#include "mlos_clock.h"
#include "mlos_task.h"
#include <string.h>
#include <stdio.h>


//------------------------------------------------------------
//define
//-------------------------------------------------------------
#define LOG_ONE_SAVE_LEN_MAX 			(128) 		//存盘的日志最大长度，128


//------------------------------------------------------------
//log 等级
//-------------------------------------------------------------
typedef enum
{
	
    LOG_LVL_INFO = 0,                                //提示信息
    LOG_LVL_DEBUG=0x01 ,                             //调试 
    LOG_LVL_WRANING =0x02,                           //警告,日志最大长度受限，     <=     LOG_ONE_SAVE_LEN_MAX                         
    LOG_LVL_ERROR =0x04,                             //错误，日志最大长度受限，     <=     LOG_ONE_SAVE_LEN_MAX
    LOG_LVL_FLOW=0x08,								//业务流程日志，最大长度受限，     <=LOG_ONE_SAVE_LEN_MAX
    LOG_LVL_ALL =0xffff,                             //所有日志
    
} LogLevel;
	

//---------------------------------------------------------
//log 服务
//---------------------------------------------------------
typedef struct{

	//mlu16 overflow;
	
	//数据长度
	mlu16 len;

	//缓存
	mlu8 buf[LOG_CACHE_SIZE];
	
	//任务
	//TaskPtr pMyTask;

	//基于某个，传输协议服务，输出日志的接口
	mlu16 (*redirectPrint) (mlu8*,mlu16);

	//日志存盘接口
	void(*saveDisk)(mlu8*,mlu16);

}LogService;

//---------------------------------------------------------
//extern var
//---------------------------------------------------------


//---------------------------------------------------------
//export marco
//---------------------------------------------------------

//日志打印 
//#define log_print(...)		mlos_printer_init(&logPrinter); printf(__VA_ARGS__)

/*
//log 带定位的输出
//#define LOG(format,...)  	mlos_printer_init(&logPrinter);\
//					printf("[%d:%d][%s@%d]~ "format,time.s, time.ms,__func__,__LINE__,##__VA_ARGS__)

//#define LOG(format,...)  	\
//					log_print("[%d:%d][%s@%d]~ "format,time.s, time.ms,__func__,__LINE__,##__VA_ARGS__)
*/
//---------------------------------------------------------
//export funcation
//---------------------------------------------------------
void log_init(void);

void log_print_redirect(mlu16 (*logPrintRedirectPort) (mlu8*,mlu16));
void log_save_disk_port_set(void(*logSaveDiskPort)(mlu8*,mlu16));

void log_print(LogLevel lglvl,char * format, ...);


//----------------------------------------------------------------------------
//文件条件编译 #else
//----------------------------------------------------------------------------
#else//条件编译

//关闭日之后，定义log接口为空
#define log_init() 
#define log_print(lglvl,...)
//#define LOG(format,...)
#define log_print_redirect(logPrintRedirectPort)
#define log_save_disk_port_set(logSaveDiskPort)

#endif//条件编译结束

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

#endif

