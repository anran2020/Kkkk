/**
 ******************************************************************************
 * 文件:ntp_tool_mc.c
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1         2014-12-01     zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"tool_config.h"
#if ((TOOL_ENABLE)&&(TOOL_DEBUG_ENABLE))

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include"u_dtd.h"
#include "tool.h"
#include "tool_debug.h"
#include "mlos_log.h"
#include "mlos.h"
#include "u_app.h"

//----------------------------------------------------------------------------
//define s
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//global vari
//----------------------------------------------------------------------------

//调试打印使能
mlu32 toolDebugPrintEnable=0;

//调试 工具是否联机
MLBool toolDebugConnected=mlfalse;


//打印mc mlos 系统信息，定时器
LTimerPtr pOsinfoPrintTimer;

//工具调试任务
Taskptr pToolDebugTask=nullptr;


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
void tool_debug_print_switch(ToolDebgPrintFuncation func,SwitchState sta)
{

	if(sta==_open)
	{
		//打开功能
		toolDebugPrintEnable|=func;
		return ;
	}

	toolDebugPrintEnable&=(~func);
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
mlu16 tool_log_print(mlu8* pDat,mlu16 len)
{

	if(toolDebugConnected)
	{
		return tool_tx_data(TOOL_MSG_LogPrint,0xff,pDat,len);
	}

	return len;
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
mlu16 tool_shell_print(mlu8* pDat,mlu16 len)
{

	return tool_tx_data(TOOL_MSG_ShellCMD,0xff,pDat,len);

}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	 :	void
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
mlu8 tool_osinfo_print(void*args)
{
#if 1

	mlu8* u8ptr,*pNetTxBuf;
	mlu16 i,j,len,tskNamelen,freeLen,mallocLen;

	
	//head(4),len(2),addr,cmd,data,sumcheck
	mallocLen=sizeof(MLOSInfo)+TASK_NAME_LEN_MAX*mlos.taskID+9;

	//获取发送缓冲区
	pNetTxBuf=tool_txbuf_malloc(mallocLen);
	if(pNetTxBuf==nullptr)
		return 0;

	//包头
	pNetTxBuf[0]=0xee;
	pNetTxBuf[1]=0xee;
	pNetTxBuf[2]=0xee;
	pNetTxBuf[3]=0xee;

	//长度、地址、命令字
	//len=len-6;
	//pNetTxBuf[4]=len;
	//pNetTxBuf[5]=(len>>8);
	pNetTxBuf[6]=1;
	pNetTxBuf[7]=TOOL_MSG_OSInfo;
	u8ptr=pNetTxBuf+8;
	memcpy(u8ptr,&mlos.ver,sizeof(MLOSInfo));
	u8ptr+=sizeof(MLOSInfo);
	tskNamelen=mlos_all_task_name(u8ptr);

	//计算数据长度
	len=sizeof(MLOSInfo)+tskNamelen+9;
	freeLen=mallocLen-len;
	tool_txbuf_free(freeLen);//释放申请多余的内存

	//填充数数据包长度
	len=len-6;
	pNetTxBuf[4]=len;
	pNetTxBuf[5]=(len>>8);
	
	//结束符
	u8ptr+=tskNamelen;
	*u8ptr=0;
	i=6;
	len=len-1;
	for(j=0;j<len;j++)
		*u8ptr+=pNetTxBuf[i+j];
	
	return 0;
#endif

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
void tool_taskinfo_print(void)
{
#if 1

	mlu8* u8ptr,*pNetTxBuf,*src;
	mlu16 i,j,len;
	TaskPtr maintask;
//	SubtaskPtr subtask;
	mlu32 u32temp;


	
	//head(4),len(2),addr,cmd,sum
	//data:taskCount(2),time.ms(2),time.s(4)+taskinfo*n
	len=sizeof(TaskDbgInfo)*mlos.taskID+8+9;
	
	//获取发送缓冲区
	pNetTxBuf=tool_txbuf_malloc(len);
	if(pNetTxBuf==nullptr)
		return ;
	
	//包头
	pNetTxBuf[0]=0xee;
	pNetTxBuf[1]=0xee;
	pNetTxBuf[2]=0xee;
	pNetTxBuf[3]=0xee;
	
	//长度、地址、命令字
	len=len-6;
	pNetTxBuf[4]=len;
	pNetTxBuf[5]=(len>>8);
	pNetTxBuf[6]=1;
	pNetTxBuf[7]=TOOL_MSG_taskInfo;


	pNetTxBuf[8]=mlos.taskID;
	pNetTxBuf[9]=0;
	pNetTxBuf[10]=sysRealTime.ms;
	pNetTxBuf[11]=(sysRealTime.ms>>8);
	pNetTxBuf[12]=sysRealTime.s;
	pNetTxBuf[13]=(sysRealTime.s>>8);
	pNetTxBuf[14]=(sysRealTime.s>>16);
	pNetTxBuf[15]=(sysRealTime.s>>24);

	//任务信息
	mlos_task_statistics_info(pNetTxBuf+16);

	//结束符
	u8ptr=pNetTxBuf+16+sizeof(TaskDbgInfo)*mlos.taskID;
	*u8ptr=0;

	i=6;
	len=len-1;
	for(j=0;j<len;j++)
		*u8ptr+=pNetTxBuf[i+j];
	
#endif
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
mlu8 tool_shell_cmd_tskinfo(void * args)
{
	//mlu16 i,len;
	ShllCmdptr pCmd;
	ShllCmdRderptr pCmdReader;

	pCmdReader=(ShllCmdRderptr)args;

	//
	if (pCmdReader->argc==0)
	{
		tool_taskinfo_print();
		return 1;
	}

	// on / off 参数
	if (strcmp(pCmdReader->args[0],"on")==0)
	{
		tool_debug_print_switch(e_dbg_print_taskinfo,_on);
	}
	else if(strcmp(pCmdReader->args[0],"off")==0)
	{
		tool_debug_print_switch(e_dbg_print_taskinfo,_off);
	}
	else if(strcmp(pCmdReader->args[0],"rz")==0)
	{
		task_statistics_reset();
	}
	else
	{

		shell_reply(" cmd tskinfo arg error!");
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
MLBool tool_debug_connect(void*args)
{

	//tool 联机协议
	//len(2),addr,cmd,connMode,commDevAddr,devtype,subAddr,rsv(6),seed,sum
	DeviceType devtype;
	mlu8 * u8ptr;
	u8ptr=(mlu8*)args;

	//mlos 调试工具联机
	if(u8ptr[4]!=TOOL_MCDEBUG_CONNECT)
	{
		return mlfalse;
	}
	
	mlu8 txbuf[6];
	//回复联机报文格式
	//报文格式：len(2 dr|cmd|cnnmode(1)|commDevAddr(1)|devType(1)|subAddr|osversion|appVersion|
	txbuf[0]=TOOL_MCDEBUG_CONNECT;
	txbuf[1]=0;
	txbuf[2]=DEV_Middle;
	txbuf[3]=0;
	txbuf[4]=MLOS_VERSION;
	txbuf[5]=APP_VERSION;				
		
	tool_tx_data(TOOL_MSG_connect, 0,txbuf,6);

	toolDebugConnected=mltrue;					//工具建立连接
	
	return mltrue;

}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	 :	void
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
void tool_debug_task(void )
{	

	if(ltimer_timeout(pOsinfoPrintTimer))
	{
		//定时打印系统信息
		if (toolDebugPrintEnable&e_dbg_print_taskinfo)
		{
			tool_taskinfo_print();
		}
		ltimer_start(pOsinfoPrintTimer);
	}

}


//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	 :	void
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
void tool_debug_int(void)
{

	//
	pOsinfoPrintTimer=ltimer_create();
	ltimer_load_start(pOsinfoPrintTimer,1000);


	//app 支持 OS shell service
	shell_print_redirect(tool_shell_print);		//重定向，打印shell
	tool_consumer_register(TOOL_MSG_ShellCMD, shell_rcv_cmd);

	//app 支持 OS log service
	log_print_redirect(tool_log_print);

	//注册，调试shell cmd
	shell_cmd_register("mlosinfo", tool_osinfo_print,"mlosinfo: get mlos info!" ); 			//注册获取系统信息指令
	shell_cmd_register("tskinfo", tool_shell_cmd_tskinfo,"tskinfo: [on/off/rz] open close reset " );
	tool_consumer_register(TOOL_MSG_connect,tool_debug_connect); //响应联机消息

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif



