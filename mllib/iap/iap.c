/**
 ******************************************************************************
 * 文件:    
 * 作者:   
 * 版本:   V0.01
 * 日期:    
 * 内容简介:
 * ISP--- In System Programming//stm官方串口升级
 * IAP----In Application Programming//用户通过其它通讯接口升级固件
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   ----   作者 --------   说明
 * v0.1    2015-11-07     zzx        创建该文件
 *
 *
 *
 ******************************************************************************
 **/
 
//----------------------------------------------------------------------------
//文件条件编译
//----------------------------------------------------------------------------
#if 1

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "iap.h"
#include "iap_mcu_port.h"
#include "mcu_flash.h"
#include "mlos.h"
#include "tool.h"

//----------------------------------------------------------------------------
//define 
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//升级功能模块，
//----------------------------------------------------------------------------
typedef struct {

    //状态
    IAPState state;
    mlu32 rcvBinFileByteCnt;        //接收文件数据计数，单位byte
    mlu32 packetSize;               //包大小 单位byte
    mlu32 packetSeq;                //包序号
    mlu32 packetTotal;              //包总数
    mlu32 wflashAddr;               //写入地址偏移

    //升级任务
    Taskptr pMytask;

}InApplicationProgramming,IAP;
//----------------------------------------------------------------------------
//global vari
//----------------------------------------------------------------------------
#ifdef COMPILE_BOOT_CODE

IAP iap;
AppFileInfo appFileinfo;

#endif

//----------------------------------------------------------------------------
//boot 代码段，实现升级中位机的功能，
//----------------------------------------------------------------------------

#ifdef COMPILE_BOOT_CODE

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by djx  2021-10-12		 编写函数
//---------------------------------------------------------------------------
void iap_be_ready(void)
{
	iap.state=e_iap_sta_ready;
}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by djx  2021-10-12		 编写函数
//---------------------------------------------------------------------------
MLBool iap_run_app(void)
{

#if 1
	APP_MAIN_FUNC app_main;
	mlu32* u32ptr;
	if(iap_app_exist())					//判断app是否存在
	{
		iap_disable_irq();
		iap_set_msp(mlfalse);
		
		u32ptr=(mlu32*)APP_ENTRY_ADDR;
		app_main=(APP_MAIN_FUNC)(*u32ptr);
		app_main();
	}
#endif	

	return mlfalse;
	
}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by djx  2021-10-12		 编写函数
//---------------------------------------------------------------------------
IapError iap_start(mlu32 binfileSize,mlu32 packetSize)
{

	//文件大小
	if( binfileSize > APP_FLASH_SIZE){
		return e_iap_err_fileSize ;
	}

	//擦除APP空间
	if( mlfalse == mcu_flash_erase(APP_FLASH_BASE_ADDR, APP_FLASH_BLCOK_NUM)){			//擦除全部APP空间
		//擦除失败
		return e_iap_err_flashErase ;
	}

	appFileinfo.size = binfileSize ;	
	iap.rcvBinFileByteCnt = 0 ;
	iap.packetSize = packetSize ;
	iap.packetSeq = 0 ;
	iap.packetTotal = ((binfileSize %packetSize)>0)?(binfileSize/packetSize+1):(binfileSize/packetSize) ;
	iap.wflashAddr = APP_SP_ADDR ;

	return e_iap_err_null;

}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by djx  2021-10-12		 编写函数
//---------------------------------------------------------------------------
IapError iap_data(mlu32 packetSeq,mlu32 validBytes,mlu32* pData)
{	
	mlu32 tm= 0 ;

	if( iap.packetSeq != packetSeq )
	{
		//包序错误
		return e_iap_err_packetSeq ;
	}

	if( mlfalse == mcu_flash_write(iap.wflashAddr, pData, iap.packetSize))
	{
		//写入失败
		return e_iap_err_flashWrite ;
	}

	iap.packetSeq++ ;
	iap.wflashAddr += iap.packetSize ;
	iap.rcvBinFileByteCnt += validBytes ;
	
	if( iap.packetSeq >= iap.packetTotal )
	{
		tm=mlos_ms_clock();
		while ((mlos_ms_clock()-tm)<100);

		if(mlfalse ==mcu_flash_write(APP_FILE_INFO_ADDR,(mlu32*)(&appFileinfo),sizeof(AppFileInfo)))			//升级完成,更新文件信息
			return e_iap_err_WriteAppFileInfo;
	}

	return e_iap_err_null;

}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by djx  2021-10-12		 编写函数
//---------------------------------------------------------------------------
void iap_backup(mlu32 packetSeq,mlu32 packetSize,mlu32* pData)
{
	
	mcu_flash_read((APP_FLASH_BASE_ADDR + (packetSeq * packetSize)), pData, packetSize);

}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
Taskstate iap_task(void *args)
{
	task_declaration();//任务声明
	

	task_begin(iap.pMytask);//任务开始

	//任务死循环
	while (mltrue)
	{
	
		//等待启动升级
		task_wait_cnd_until(iap.pMytask,(e_iap_sta_wait!=iap.state), 6000, TASK_STA_NONBLOCKING_YIELD);
		
		//等待超时
		if(e_iap_sta_wait==iap.state)
		{
			//退出升级,运行app
			iap_run_app();
			task_exit(iap.pMytask);
		}

		//启动升级

		//升级
		

		//校验


		//升级完成
	}

	task_end(iap.pMytask);
}

//----------------------------------------------------------------------------
//app 代码段，实现升级下位机的功能，转发报文
//----------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
MLBool iap_run_boot(void)
{
	BOOT_MAIN_FUNC boot_main;
	mlu32* u32ptr;

	iap_disable_irq();
	iap_set_msp(mltrue);
	u32ptr=(mlu32*)BOOT_ENTRY_ADDR;
	boot_main=(BOOT_MAIN_FUNC)(*u32ptr);
	boot_main();
		
	return mlfalse;
	
}

#endif

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void iap_init(void)
{

	
#ifdef COMPILE_BOOT_CODE
	//初始化状态
	iap.state=e_iap_sta_wait;

	AppFileInfoPtr fileInfoptr;
	
	fileInfoptr=(AppFileInfoPtr)APP_FILE_INFO_ADDR;
	memcpy(&appFileinfo,fileInfoptr,sizeof(AppFileInfo));

	//在boot，iap实现为一个任务
	iap.pMytask=task_create(IAP_TASK_PRIO, iap_task, nullptr, ltimer_create(),"iap");

#endif

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


