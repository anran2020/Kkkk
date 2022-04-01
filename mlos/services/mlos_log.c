/**
 ******************************************************************************
 * 文件:mlos_log.c
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

//----------------------------------------------------------------------------
//条件编译
//----------------------------------------------------------------------------
#include "mlos_log.h"
#include "mlos_shell.h"
#include <stdio.h>



#if (OS_LOG_ENABLE)


//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mlos.h"
#include "mlos_clock.h"
#include "mlos_ltimer.h"
#include "mlos_task.h"
#include "mlos_mpump.h"
#include "mlos_log.h"



//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//local global  variable
//----------------------------------------------------------------------------

//日志对象
LogService log;

//日志打赢等级掩码，屏蔽日志的等级 打印，默认全打开
mlu16 logPrintLevelMask=0xffff;

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void log_save_disk_port_set(void(*logSaveDiskPort)(mlu8*,mlu16))
{
	log.saveDisk=logSaveDiskPort;
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void log_print_switch(LogLevel lglvl,SwitchState sta)
{

	if(sta==_open)
	{
		//打开功能
		logPrintLevelMask|=	lglvl;
		return ;
	}

	logPrintLevelMask&=(~lglvl);
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void log_print(LogLevel lglvl,char* format , ...)
{
	va_list vArgList;

	//日志重定向，打印
	if(log.len>0&&log.redirectPrint!=nullptr)
	{
		log.redirectPrint(log.buf,log.len);
	}
	
	va_start (vArgList, format);
	//printf(format, vArgList);
	log.len=vsprintf((char*)log.buf,format,vArgList)+1;		//打印日志到缓存
	va_end(vArgList); 

	//有日志数据
	if(log.len>0)
	{

		//分等级存盘日志
		if(lglvl>=LOG_LVL_WRANING&&log.saveDisk!=nullptr)
		{
			log.saveDisk(log.buf,log.len);
		}

		//日志重定向，打印
		if(log.redirectPrint!=nullptr&&logPrintLevelMask>0)
		{
			log.len=log.len-log.redirectPrint(log.buf,log.len);
		}
		else
		{
			log.len=0;
		}
	}
	
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//			重定向c库函数printf
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
//int fputc(int ch, FILE *f)
//{
      
//     return (ch);
//}


//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//			重定向，打印日志，可以通过网口、串口、can口，打印日志
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void log_print_redirect(mlu16 (*logPrintRedirectPort) (mlu8*,mlu16))
{

	log.len=0;

	//传输协议服务
	log.redirectPrint=logPrintRedirectPort;
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
mlu8 log_shell_cmd_handle(void * args)
{
	//mlu16 i,len;
	SwitchState swSta;
	LogLevel lglvl;
	ShllCmdRderptr pCmdReader;

	pCmdReader=(ShllCmdRderptr)args;
	
	//
	if (pCmdReader->argc==0)
	{
		return 0;
	}

	// arg 1 [on/off]
	if (strcmp(pCmdReader->args[0],"on")==0)
	{
		//
		swSta=_on;
		if (pCmdReader->argc<2)
		{
			log_print_switch(LOG_LVL_ALL,_on);
			return 1;
		}
	}
	else if(strcmp(pCmdReader->args[0],"off")==0)
	{
		swSta=_off;
		//
		if (pCmdReader->argc<2)
		{
			log_print_switch(LOG_LVL_ALL,_off);
			return 1;
		}
	}
	else
	{

		return 0;
	}
	
#if 0
	//arg2  [all/err/war/dbg/inf] 
	if (strcmp(pCmdReader->args[1],"all")==0)
	{
		lglvl=LOG_LVL_ALL;
	}
	else if(strcmp(pCmdReader->args[1],"err")==0)
	{
		lglvl=LOG_LVL_ERROR;
	}
	else if(strcmp(pCmdReader->args[1],"dbg")==0)
	{
		lglvl=LOG_LVL_DEBUG;
	}
	else if(strcmp(pCmdReader->args[1],"info")==0)
	{
		lglvl=LOG_LVL_INFO;
	}
	else
	{

		return 0;
	}

	log_print_switch(lglvl,swSta);							//开关日志
#endif

	return 1;
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void log_init(void)
{
	//log.overflow=0;
	log.len=0;

	//分配缓存
	//log.buf=mlos_malloc(e_mem_sram, LOG_CACHE_SIZE);
	
	//创建 task
	//log.pMyTask=task_create(LOG_TASK_PRIO,log_task, nullptr, nullptr,"log");

	//传输协议服务的日志输出
	log.redirectPrint=nullptr;
	log.saveDisk=nullptr;

	//log 接收shell cmd
	shell_cmd_register("log", log_shell_cmd_handle,"log: [on/off] open close [all/err/war/dbg/info] log level" );
	
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


