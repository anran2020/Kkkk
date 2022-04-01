/**
 ******************************************************************************
 * 文件:ds_tool_ntp.c
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
#if ((TOOL_ENABLE)&&(TOOL_DS_ENABLE))

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "mlos.h"

#include"u_dtd.h"
#include "tool.h"
#include "tool_ds.h"

#include "ds.h"

//----------------------------------------------------------------------------
//define s
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//global vari
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//extern vari
//----------------------------------------------------------------------------
extern DSFilePtr logfile;
extern DSFilePtr trayFile[DS_TRAY_NUM];


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
void tool_ds_tray_header_upload(mlu8 traAddr)
{
#if 1

	mlu8* u8ptr,*pNetTxBuf,*headerBuf;
	mlu16 i,j,len,tskNamelen,freeLen,mallocLen;

	
	//head(4),len(2),addr,cmd,data,sumcheck
	
	mallocLen=sizeof(DSFileHeader)*2+2+9;

	//获取发送缓冲区
	pNetTxBuf=tool_txbuf_malloc(mallocLen);
	if(pNetTxBuf==nullptr)
		return ;

	//包头
	pNetTxBuf[0]=0xee;
	pNetTxBuf[1]=0xee;
	pNetTxBuf[2]=0xee;
	pNetTxBuf[3]=0xee;

	//长度、地址、命令字
	//填充数数据包长度
	len=mallocLen-6;
	pNetTxBuf[4]=len;
	pNetTxBuf[5]=(len>>8);

	pNetTxBuf[6]=0xff;
	pNetTxBuf[7]=TOOL_MSG_DiskTrayFileOp;
	pNetTxBuf[8]=1;						//1:读取文件头
	pNetTxBuf[9]=traAddr;
	
	u8ptr=pNetTxBuf+10;
	headerBuf=dsf_read_page(trayFile[traAddr],trayFile[traAddr]->basePage);			//read header A
	if(headerBuf==nullptr)
	{
		return;
	}
	memcpy(u8ptr,headerBuf,sizeof(DSFileHeader));
	u8ptr+=sizeof(DSFileHeader);
	headerBuf=dsf_read_page(trayFile[traAddr],trayFile[traAddr]->basePage+1);			//read header B
	if(headerBuf==nullptr)
	{
		return;
	}
	memcpy(u8ptr,headerBuf,sizeof(DSFileHeader));
	u8ptr+=sizeof(DSFileHeader);
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
void tool_ds_tray_line_upload(mlu8 traAddr,mlu32 lineNum)
{
#if 1

	mlu8* u8ptr,*pNetTxBuf,*rPageBuf;
	mlu16 i,j,len,tskNamelen,freeLen,mallocLen;

	
	//head(4),len(2),addr,cmd,op,startLine,data,sumcheck
	
	mallocLen=DS_FLASH_PAGE_SIZE*DS_TRAY_PAGE_PER_LINE+6+9;

	//获取发送缓冲区
	pNetTxBuf=tool_txbuf_malloc(mallocLen);
	if(pNetTxBuf==nullptr)
		return ;

	//包头
	pNetTxBuf[0]=0xee;
	pNetTxBuf[1]=0xee;
	pNetTxBuf[2]=0xee;
	pNetTxBuf[3]=0xee;

	//长度、地址、命令字
	//填充数数据包长度
	len=mallocLen-6;
	pNetTxBuf[4]=len;
	pNetTxBuf[5]=(len>>8);

	pNetTxBuf[6]=0xff;
	pNetTxBuf[7]=TOOL_MSG_DiskTrayFileOp;
	pNetTxBuf[8]=2;						//2:行读取文件
	pNetTxBuf[9]=traAddr;
		
	mlu32 pageAddr=lineNum*trayFile[traAddr]->pagesPerLine;
	pNetTxBuf[10]=lineNum;
	pNetTxBuf[11]=(lineNum>>8);
	pNetTxBuf[12]=(lineNum>>16);
	pNetTxBuf[13]=(lineNum>>24);

	
	u8ptr=pNetTxBuf+14;
	for(i=0;i<trayFile[traAddr]->pagesPerLine;++i)
	{
		rPageBuf=dsf_read_page(trayFile[traAddr],trayFile[traAddr]->basePage+DS_FILE_BACKUP_PAGE_NUM+pageAddr+i);			
		if(rPageBuf==nullptr)
		{
			return;
		}
		memcpy(u8ptr,rPageBuf,DS_FLASH_PAGE_SIZE);
		u8ptr+=DS_FLASH_PAGE_SIZE;
	}
	*u8ptr=0;
	i=6;
	len=len-1;
	for(j=0;j<len;j++)
	{
		*u8ptr+=pNetTxBuf[i+j];				//求和校验
	}
	
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
void tool_ds_tray_clear(mlu8 traAddr)
{

#if 1
	MLBool bRet;

	bRet=dsf_clear(trayFile[traAddr]);
	//trayDiskCache[traAddr].isEmpty=1;
	//trayDiskCache[traAddr].curWriteDiskPage=0;

	mlu8 txbuf[4];
	txbuf[0]=3;							//3,清除日志
	txbuf[1]=traAddr;
	if(bRet)
		txbuf[2]=0;
	else
		txbuf[2]=1;						//1,清除日志失败
	txbuf[3]=0;

	tool_tx_data(TOOL_MSG_DiskTrayFileOp, 0xff, txbuf, 4);				//回复tool
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
MLBool tool_ds_tray_dat_operate(void * args)
{
	mlu8 operation,traAddr;
	mlu8* pRxDat;

	//len(2),addr,cmd,data,sumcheck
	pRxDat=(mlu8*)args;

	operation=pRxDat[4];
	traAddr=pRxDat[5];
	if(traAddr>=DS_TRAY_NUM)
	{
		return mltrue;							//托盘索引超范围
	}
	
	//  参数
	if (operation==DSF_READ_HEADER)
	{
		//读取文件信息
		tool_ds_tray_header_upload(traAddr);		
	}
	else if(operation==DSF_READ_DATA)
	{
		//读取内容
		//len(2),addr,cmd,op(4),startLine(4),sumcheck
		mlu32 lineNum;
		lineNum=pRxDat[11];
		lineNum=(lineNum<<8)+pRxDat[10];
		lineNum=(lineNum<<8)+pRxDat[9];
		lineNum=(lineNum<<8)+pRxDat[8];

		//计算页
		if(lineNum<DS_TRAY_LINE_MAX)
		{
			tool_ds_tray_line_upload(traAddr,lineNum);
		}

	}
	else if(operation==DSF_CLEAR)
	{
		//清除
		tool_ds_tray_clear(traAddr);
	}
	else
	{
		;
	}
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
void tool_ds_log_header_upload(void)
{
#if 1

	mlu8* u8ptr,*pNetTxBuf,*headerBuf;
	mlu16 i,j,len,tskNamelen,freeLen,mallocLen;

	
	//head(4),len(2),addr,cmd,data,sumcheck
	
	mallocLen=sizeof(DSFileHeader)*2+1+9;

	//获取发送缓冲区
	pNetTxBuf=tool_txbuf_malloc(mallocLen);
	if(pNetTxBuf==nullptr)
		return ;

	//包头
	pNetTxBuf[0]=0xee;
	pNetTxBuf[1]=0xee;
	pNetTxBuf[2]=0xee;
	pNetTxBuf[3]=0xee;

	//长度、地址、命令字
	//填充数数据包长度
	len=mallocLen-6;
	pNetTxBuf[4]=len;
	pNetTxBuf[5]=(len>>8);

	pNetTxBuf[6]=0xff;
	pNetTxBuf[7]=TOOL_MSG_DiskLogFileOp;
	pNetTxBuf[8]=1;						//1:读取文件头
	
	u8ptr=pNetTxBuf+9;
	headerBuf=dsf_read_page(logfile,logfile->basePage);			//read header A
	if(headerBuf==nullptr)
	{
		return;
	}
	memcpy(u8ptr,headerBuf,sizeof(DSFileHeader));
	u8ptr+=sizeof(DSFileHeader);
	headerBuf=dsf_read_page(logfile,logfile->basePage+1);			//read header B
	if(headerBuf==nullptr)
	{
		return;
	}
	memcpy(u8ptr,headerBuf,sizeof(DSFileHeader));
	u8ptr+=sizeof(DSFileHeader);
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
void tool_ds_log_page_upload(mlu32 startLine)
{
#if 1

	mlu8* u8ptr,*pNetTxBuf,*rPageBuf;
	mlu16 i,j,len,tskNamelen,freeLen,mallocLen;

	
	//head(4),len(2),addr,cmd,op,startLine,data,sumcheck
	
	mallocLen=DS_FLASH_PAGE_SIZE+5+9;

	//获取发送缓冲区
	pNetTxBuf=tool_txbuf_malloc(mallocLen);
	if(pNetTxBuf==nullptr)
		return ;

	//包头
	pNetTxBuf[0]=0xee;
	pNetTxBuf[1]=0xee;
	pNetTxBuf[2]=0xee;
	pNetTxBuf[3]=0xee;

	//长度、地址、命令字
	//填充数数据包长度
	len=mallocLen-6;
	pNetTxBuf[4]=len;
	pNetTxBuf[5]=(len>>8);

	pNetTxBuf[6]=0xff;
	pNetTxBuf[7]=TOOL_MSG_DiskLogFileOp;
	pNetTxBuf[8]=2;						//2:行读取文件
		
	mlu32 pageAddr=startLine/logfile->linesPerPage;
	startLine=pageAddr*logfile->linesPerPage;					//计算起始行号
	pNetTxBuf[9]=startLine;
	pNetTxBuf[10]=(startLine>>8);
	pNetTxBuf[11]=(startLine>>16);
	pNetTxBuf[12]=(startLine>>24);

	
	u8ptr=pNetTxBuf+13;
	rPageBuf=dsf_read_page(logfile,logfile->basePage+DS_FILE_BACKUP_PAGE_NUM+pageAddr);			//read header A
	if(rPageBuf==nullptr)
	{
		return;
	}
	memcpy(u8ptr,rPageBuf,DS_FLASH_PAGE_SIZE);
	u8ptr+=DS_FLASH_PAGE_SIZE;
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
void tool_ds_log_clear(void)
{

	MLBool bRet;

	bRet=dsf_clear(logfile);

	mlu8 txbuf[4];
	txbuf[0]=3;							//3,清除日志
	txbuf[1]=0;
	if(bRet)
		txbuf[2]=0;
	else
		txbuf[2]=1;						//1,清除日志失败
	txbuf[3]=0;

	tool_tx_data(TOOL_MSG_DiskLogFileOp, 0xff, txbuf, 4);				//回复tool
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
MLBool tool_ds_log_operate(void * args)
{
	mlu8 operation;
	mlu8* pRxDat;

	//len(2),addr,cmd,data,sumcheck
	pRxDat=(mlu8*)args;

	operation=pRxDat[4];
	
	//  参数
	if (operation==DSF_READ_HEADER)
	{
		//读取日志文件信息
		tool_ds_log_header_upload();		
	}
	else if(operation==DSF_READ_DATA)
	{
		//读取日志内容
		//len(2),addr,cmd,op(4),startLine(4),sumcheck
		mlu32 startLine;
		startLine=pRxDat[11];
		startLine=(startLine<<8)+pRxDat[10];
		startLine=(startLine<<8)+pRxDat[9];
		startLine=(startLine<<8)+pRxDat[8];
		//计算页
		if(startLine<DS_LOG_LINE_MAX)
		{
			tool_ds_log_page_upload(startLine);
		}

	}
	else if(operation==DSF_CLEAR)
	{
		//清除日志
		tool_ds_log_clear();
	}
	else
	{
		;
	}
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
void tool_ds_init(void )
{

	tool_consumer_register(TOOL_MSG_DiskLogFileOp, tool_ds_log_operate);				//工具读写磁盘文件
	tool_consumer_register(TOOL_MSG_DiskTrayFileOp, tool_ds_tray_dat_operate);
}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif




