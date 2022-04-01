/**
 ******************************************************************************
 * 文件:mlos_shell.h
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


#ifndef _MLOS_SHELL_H_
#define _MLOS_SHELL_H_
//----------------------------------------------------------------------------
//文件条件编译 #if
//----------------------------------------------------------------------------
#include "mlos_default_config.h"

#if (OS_SHELL_ENABLE)


//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"
#include "mlos_task.h"
//#include"mlos_print.h"

//----------------------------------------------------------------------------
//defines
//----------------------------------------------------------------------------

#define SHELL_CMD_NAME_BYTE_MAX 				(12) 		//cmd 名字的长度 12byte

#define SHELL_CMD_ARGC_MAX 					(10) 		//cmd 参数个数最大 10个

#define SHELL_CMD_ARG_BYTE_MAX 				(16) 		//cmd 单个参数长度 16 byte


//----------------------------------------------------------------------------
//scmd 回调处理函数
//----------------------------------------------------------------------------

typedef mlu8 (*ShellCmdhandler )(void*);


//----------------------------------------------------------------------------
// shell Command structure 
//----------------------------------------------------------------------------
typedef struct ShllCmdNode
{
	//
	struct ShllCmdNode *pnext;					//指向下一个cmd
	const char *name;							//命令名称
	ShellCmdhandler exe;						//命令执行函数
	const char *helpDocument;					//帮助文档
	
}ShellCommand,ShllCmd,*ShllCmdptr;

//----------------------------------------------------------------------------
// shell Command reader 指令阅读器
//----------------------------------------------------------------------------
typedef struct ShellCommandReader{

	mlu8   argc;													//参数个数		
	char name[SHELL_CMD_NAME_BYTE_MAX];								//当前接收cmd的name
	char args[SHELL_CMD_ARGC_MAX][SHELL_CMD_ARG_BYTE_MAX];			//当前接收cmd的参数
	ShllCmdptr pCmd; 												//匹配已注册cmd
	
}ShellCmdReader,*ShllCmdRderptr;


//---------------------------------------------------------
//extern var
//---------------------------------------------------------



//---------------------------------------------------------
//export marco
//---------------------------------------------------------


//---------------------------------------------------------
//export funcation
//---------------------------------------------------------
void shell_init(void);
MLBool shell_cmd_register(const char * name,ShellCmdhandler handler,const char *helpDoc);
mlu8 shell_rcv_cmd(void* args);
void shell_print_redirect(mlu16 (*shellPrintRedirectPort)(mlu8*,mlu16));
void shell_print(char* format , ...);
void shell_reply(const char* str);

//----------------------------------------------------------------------------
//文件条件编译 #else
//----------------------------------------------------------------------------
#else//条件编译

#define shell_init() 
#define shell_cmd_register(name,handler,helpDoc)
#define shell_rcv_cmd 										nullptr
#define shell_print_redirect(shellPrintRedirectPort) 
#define shell_print(format,...)
#define  shell_reply(str)
#endif // 文件条件编译结束

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif




