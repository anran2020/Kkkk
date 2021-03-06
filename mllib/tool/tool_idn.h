/**
 ******************************************************************************
 * 文件:ntp_msg_idn.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *		net消息id，资源声明
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----日期   ---作者    ----------   说明-------------------------------------
 * v0.1     2014-12-01   zzx           创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef  _NTP_TOOL_MSG_IDN_H__
#define  _NTP_TOOL_MSG_IDN_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// tool 复位设备 模块  枚举
//----------------------------------------------------------------------------

typedef enum{

	DFM_McNetwork=1,	//中位机网络模块


}DeviceFunctionModule;



#if 1

//----------------------------------------------------------------------------
// tool 消息 id
//----------------------------------------------------------------------------
typedef enum
{

    TOOL_MSG_NULL = 0,

    //MSG for mc
    TOOL_MSG_connect = 1, //联机
    TOOL_MSG_iapRerun  = 2, //重启boot、app
    TOOL_MSG_iapStart=3,             //启动升级
    TOOL_MSG_iapFileData=4,          //升级文件数据
    TOOL_MSG_iapBackup=5,            //升级备份
        
    TOOL_MSG_writeAttri = 6, //写属性
    TOOL_MSG_readAttri = 7, //读属性
	TOOL_MSG_resetDevMdl = 8, //复位重启 设备模块


	TOOL_MSG_taskInfo=8,									//打印任务信息
	TOOL_MSG_ShellCMD=9,									//打印shell
	TOOL_MSG_OSInfo=10,									//打印os信息
	TOOL_MSG_LogPrint=11,									//打印日志
	TOOL_MSG_DiskLogFileOp=12,				//磁盘文件读写操作
	TOOL_MSG_DiskTrayFileOp=13,				//磁盘文件读写操作
	TOOL_MSG_MAX=13,


} ToolNtpMsgIDN;

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif

