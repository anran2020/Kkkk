/**
 ******************************************************************************
 * 文件:ds_.c
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

#include "ds.h"
#include "mlos.h"
#include"tool_ds.h"


//---------------------------------------------------------
//define
//---------------------------------------------------------

//---------------------------------------------------------
//define
//---------------------------------------------------------


//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//global variable
//----------------------------------------------------------------------------

//数据存储任务
Taskptr pDSTask=nullptr;


LTimerPtr pDSTestTimer;

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
void ds_tray_dat_test(void)
{
#if 0
	mlu8 i,*buf;
	static mlu16 smplTimes=0;
	for(i=0;i<DS_TRAY_NUM;++i)
	{

		buf=ds_tray_disk_chache(i);
		if(buf==nullptr)
		{
			log_print(LOG_LVL_INFO, "%s-%s-%d :托盘%d，缓存已满，写入失败!",__FILE__,__FUNCTION__,__LINE__,i+1);
			continue;
		}
		
		UpChnlSmpl*pSmplDat;		
		//通道数
		mlu16 j;
		pSmplDat=(UpChnlSmpl*)buf;
		for (j = 0; j < 24; ++j)
		{
			memset(pSmplDat,0,sizeof(UpChnlSmpl));
			
			pSmplDat->timeStampSec=sysRealTime.s;
			pSmplDat->timeStampMs=sysRealTime.ms;
			pSmplDat->chnIdx=j;
			pSmplDat->chnType=1;
			pSmplDat->chnState=2;
			pSmplDat->stepId=3;
			pSmplDat->stepType=4;
			pSmplDat->stepSubType=5;
			pSmplDat->inLoop=6;	/*串联时，电芯是否在回路中*/
			pSmplDat->stepSmplSeq=smplTimes;   /*工步内采样序号，工步切换时清零*/
			pSmplDat->volCell=3200;    /*电池电压，极柱电压，电压采样线电压*/
			pSmplDat->volCur=3200;	  /*电流探针电压*/
			pSmplDat->volPort=3200;    /*设备端口电压*/
			pSmplDat->volInner=3200;	/*内部电压*/
			pSmplDat->current=-100;
			pSmplDat->capacity=smplTimes*100;
			pSmplDat->causeCode=7;
			pSmplDat->ariPress=8;
			pSmplDat->smokePres=9;	/*smoke present*/
	
			pSmplDat++;
		}
		
	}

	smplTimes++;
#endif
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : MLBool，返回mltrue标识文件已经循环覆盖写入
// 参数   :  
//fhdl，文件句柄，索引对应的文件
//curWriteLine，返回文件当前的写入行号
//
//功能描述：
//	文件移到末尾,获取当前写入序号，返回mltrue标识文件已经循环写入
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
MLBool ds_file_seek_end(DSFilileHandle fhdl,mlu32*curWriteLine)
{
	if(fhdl<e_dsf_Max)//文件句柄非法判断
	{
		return ds_tray_file_seek_end;
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
Taskstate ds_task(void*args)
{
	task_declaration();									//任务声明

	
	if(!ltimer_is_running(pDSTestTimer))
	{
		ltimer_load_start(pDSTestTimer, 1000);
	}

	if (ltimer_timeout(pDSTestTimer))
	{
		ds_tray_dat_test();
		ltimer_start(pDSTestTimer);
	}

	task_begin(pDSTask);							//任务开始
	
	while (mltrue)
	{
		//task_ms_delay(pDSTask, 30, TASK_STA_NONBLOCKING_YIELD);

		while(ds_tray_subtask())										//写托盘文件，保存托盘数据，优先于log的写盘
		{
			task_yield(pDSTask, TASK_STA_NONBLOCKING_YIELD);		//数据写入，退出任务，让出cpu资源，保证系统的实时
		}

		ds_log_subtask();												//写日志文件保存，日志
		task_yield(pDSTask, TASK_STA_NONBLOCKING_YIELD);	
	}

	task_end(pDSTask); 							//任务结束
	
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
void ds_init(void)
{

	//初始化数据存储的flash
	cssd_init();

	//创建数据存储任务
	pDSTask=task_create(TASK_PRIO_6, ds_task,nullptr,ltimer_create(),"ds");


	//日志存储初始化
	ds_log_init();

	//托盘数据存储初始化
	ds_tray_init();

	//
	pDSTestTimer=ltimer_create();
	
	
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 






