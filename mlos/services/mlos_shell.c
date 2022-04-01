/**
 ******************************************************************************
 * 文件:mlos_shell.c
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

#include "mlos_shell.h"

#if (OS_SHELL_ENABLE)

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
#include "mlos_shell.h"



//----------------------------------------------------------------------------
//shell 状态
//----------------------------------------------------------------------------
enum{
	e_shll_sta_idle=0,
	e_shll_sta_rxCmd,
	e_shll_sta_txCmd,
	
};

//----------------------------------------------------------------------------
//shell 服务
//----------------------------------------------------------------------------
typedef struct{

	mlu8 status; 							//当前状态
	mlu16 cmdBufDatlen;						//cmdBuf 有效数据长度
	ShellCmdReader cmdReader;			//cmd阅读器
	
	ListPtr pCmdList;									//shell cmd list
	TaskPtr pMyTask;									//任务
	mlu8 cmdBuf[SHELL_CACHE_SIZE]; 					// cmd 缓存
	mlu16 (*redirectPrint)(mlu8*,mlu16);		//命令回复接口

}ShellService;

//----------------------------------------------------------------------------
//local global  variable
//----------------------------------------------------------------------------

//
ShellService shell;


//----------------------------------------------------------------------------
//extern funcation
//----------------------------------------------------------------------------

extern ListPtr mlos_mem_list(void);

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
mlu8 shell_cmd_check(const char* cmdname,char*cmdbuf)
{
	mlu16 i;
	for (i = 0; cmdname[i]!='\0'; ++i)
	{
		if(cmdname[i]!=cmdbuf[i])
			return 0;
	}
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
ShllCmdptr shell_cmd_seek(char* name)
{
	ShllCmdptr pCmd;

	pCmd=shell.pCmdList->head;
	while (nullptr!=pCmd)
	{
		if(strcmp(pCmd->name, name) == 0)
		{
			return pCmd;
		}

		pCmd=pCmd->pnext;
	}

	return nullptr;
	
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
void shell_reply(const char* str)
{
	mlu16 i;
	shell.cmdBufDatlen=0;
	for (i = 0; (*str!='\0')&& (i < SHELL_CACHE_SIZE); ++i)
	{
		shell.cmdBuf[i]=*str++;
		shell.cmdBufDatlen++;
	}
	
	shell.status=e_shll_sta_txCmd;
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
void shell_print(char* format , ...)
{
	mlu16 len;
	va_list vArgList;
	
	shell.cmdBufDatlen=0;
	va_start (vArgList, format);
	len=vsprintf((char*)shell.cmdBuf,format,vArgList);
	va_end(vArgList); 
	if(len>0)
	{
		shell.cmdBufDatlen=len;
		shell.status=e_shll_sta_txCmd;
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
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
mlu8 shell_cmd_help(void * args)
{
	//mlu16 i,len;
	ShllCmdptr pCmd;
	ShllCmdRderptr pCmdReader;

	pCmdReader=(ShllCmdRderptr)args;
	
	//
	if (pCmdReader->argc==0)
	{
		shell_reply(pCmdReader->pCmd->helpDocument);
		return 1;
	}

	// help -cmd
	//取 参数
	pCmd=shell_cmd_seek(pCmdReader->args[0]);
	if(pCmd!=nullptr)
	{
		if (pCmd->helpDocument!=nullptr)
		{
			shell_reply(pCmd->helpDocument);
		}
		else
		{
			shell_reply("help cmd ,error cmd have no helpDocument !");
		}
		
	}
	else
	{
		shell_reply("help cmd ,error cmd unknow!");
	}
	
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
mlu8 shell_cmd_reboot(void* args)
{

	
	//系统重启
	
	shell_reply("boot is running，now");
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
mlu8 shell_cmd_log(void * pShllCmd,char* args)
{

	
	//日志命令,取模块参数
	
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
mlu8 shell_cmd_reset(void * pShllCmd,char* args)
{

	
	//
	task_debug_reset();
	shell_print(" mlos reset over!");
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
mlu8 shell_cmd_mem(char* args)
{
	mlu32 i;
	MemBlockPtr pMemBlock;
	
	pMemBlock=mlos_mem_list()->head;
	
	//获取内存使用情况
	i=0;
	while(nullptr!=pMemBlock)
	{
		i++;
		shell_print("Mem_%u @0x%X:size= %u byte;used= %u byte;free= %u byte;",i,pMemBlock->mem
			,pMemBlock->size,pMemBlock->nextFreebyte,(pMemBlock->size-pMemBlock->nextFreebyte));
		pMemBlock=pMemBlock->pnext;
	}
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
MLBool shell_cmd_register(const char * name,ShellCmdhandler handler,const char *helpDoc)
{
	mlu16 len;
	ShllCmdptr pCmd;

	len=strlen(name);
	if(len==0||len>=SHELL_CMD_NAME_BYTE_MAX||handler==nullptr)		//参数合法判断
	{
		return mlfalse;
	}

	pCmd=mlos_malloc(e_mem_sram, sizeof(ShellCommand));//新建cmd
	pCmd->pnext=nullptr;
	pCmd->name=name;
	pCmd->exe=handler;							
	pCmd->helpDocument=helpDoc;				//装载cmd帮助文档

	list_append(shell.pCmdList, pCmd);

	return mltrue;
	
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
mlu8* shell_cmd_buf(void)
{
	return shell.cmdBuf;
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
MLBool shell_rcv_cmd(void * args)
{
	mlu16 i,len;
	mlu8 * pCmdDat;
	pCmdDat=(mlu8 *)args;
	//len addr cmd data seed sumcheck
	len=*(pCmdDat+1);
	len=(len<<8)+*pCmdDat-4;

	pCmdDat+=4;
	for (i = 0; (i<len)&& (i < SHELL_CACHE_SIZE); ++i)
	{
		shell.cmdBuf[i]=pCmdDat[i];
	}
	
	shell.status=e_shll_sta_rxCmd;
	shell.cmdBufDatlen=len;

	return mltrue;
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
void shell_print_redirect(mlu16 (*shellPrintRedirectPort)(mlu8*,mlu16))
{
	shell.redirectPrint=shellPrintRedirectPort;
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
MLBool shell_cmd_buf_read(ShllCmdRderptr pCmdReader)
{
	mlu16 i,argc,bytes;
	MLBool bValidChar;

	//
	if ((shell.cmdBuf[0]==' ')||(shell.cmdBuf[0]=='\n'))
	{
		shell_print("Error: cmd first char illegal !");
		return mlfalse;
	}
	
	//name
	for (i = 0; i < shell.cmdBufDatlen; ++i)
	{
		if(i>=(SHELL_CMD_NAME_BYTE_MAX-1))
		{
			shell_print("Error: cmd name too long!");
			return mlfalse;									//cmd name 长度超过最大值
		}
		if(shell.cmdBuf[i]==' '||shell.cmdBuf[i]=='\0')
		{
			pCmdReader->name[i]='\0';
			break;
		}
		pCmdReader->name[i]=shell.cmdBuf[i];
	}

	//args
	bValidChar=mlfalse;
	argc=0;
	bytes=0;
	for (; i < shell.cmdBufDatlen; ++i)
	{
		
		if(shell.cmdBuf[i]==' '||shell.cmdBuf[i]=='\0')
		{
			if(bValidChar==mltrue)										//有效字符接收结束，即产生一个参数
			{
				pCmdReader->args[argc][bytes++]='\0';
				argc++;
			}
			bValidChar=mlfalse;						
			bytes=0;
			if(shell.cmdBuf[i]=='\0')									//cmd字符串结束
				break;		
		}
		else
		{
			bValidChar=mltrue; 											//接收有效字符
			pCmdReader->args[argc][bytes++]=shell.cmdBuf[i];
			if(bytes>=(SHELL_CMD_ARG_BYTE_MAX))							//cmd 参数超长				
			{
				shell_print("Error: cmd arg  too long!");
				return mlfalse;
			}
		}
		
	}

	pCmdReader->argc=argc;						//参数个数
	return mltrue;
	
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
Taskstate shell_task(void *args)
{
	task_declaration();//任务声明
	ShllCmdptr pCmd;
	
	task_begin(shell.pMyTask);//任务开始

	//处理 cmd
	if(e_shll_sta_rxCmd==shell.status)
	{
		shell.status=e_shll_sta_idle;								//释放状态
		if (shell_cmd_buf_read(&shell.cmdReader)) 					//从cmdbuf 中读取 cmd
		{
			pCmd=shell_cmd_seek(shell.cmdReader.name);				//查找cmd
			if(pCmd!=nullptr)		
			{
				shell.cmdReader.pCmd=pCmd;
				if(pCmd->exe(&shell.cmdReader)==mlfalse)					//执行cmd	
				{
					shell_reply(pCmd->helpDocument);
				}
			}
			else
			{
				shell_reply("error: cmd  UNregister!");						//查找不到指令，指令未注册
			}
		}
	}

	//响应 回复cmd
	if(e_shll_sta_txCmd==shell.status)
	{
		
		if(shell.redirectPrint(shell.cmdBuf,shell.cmdBufDatlen)) 		//回复cmd
		{
			shell.cmdBufDatlen=0;
			shell.status=e_shll_sta_idle;
		}
	}


	task_end(shell.pMyTask);

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
void shell_init(void)
{

	shell.status=e_shll_sta_idle;

	shell.pCmdList=list_create(e_mem_sram,e_lst_sortUnordered,e_lst_linkSingly);//cmd单向链表


	//创建shell task
	shell.pMyTask=task_create(SHELL_TASK_PRIO,shell_task, nullptr, nullptr,"shell");


	//系统指令等级
	shell_cmd_register("help", shell_cmd_help,"help cmd: show cmd help document!");
	shell_cmd_register("reboot", shell_cmd_reboot,"reboot: Reboot the board by software!");
	shell_cmd_register("mem", shell_cmd_mem,"mem: os memory!");
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


