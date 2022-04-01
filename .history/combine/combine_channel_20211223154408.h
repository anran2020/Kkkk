/**
 ******************************************************************************
 * 文件:combine_channel.h
 * 作者:   
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                      
 *
 *
 *
 ******************************************************************************
 **/



#ifndef  _COMBINE_CHANNEL__H__
#define  _COMBINE_CHANNEL__H__
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if COMBINE_ENABLE
//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"
#include "mlos_list.h"
#include "mlos_task.h"
#include "combine_module.h"



//----------------------------------------------------------------------------
//define 
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// 通道状态
//----------------------------------------------------------------------------
typedef enum {

	e_ch_sta_init=1,//初始化
	e_ch_sta_idle,//空闲
	e_ch_sta_working,//工作
	e_ch_sta_starting,//启动
	e_ch_sta_paused,//暂停
	e_ch_sta_stopping,//停止
	e_ch_sta_fault,//故障
	e_ch_sta_npWaiting,//negative pressure waiting
	
}ChannelState;

//----------------------------------------------------------------------------
//并机电源通道定义
//----------------------------------------------------------------------------
 typedef   struct CmbPwrChnlItem{

	STRUCT_INHERIT(CmbPwrChnlItem,SLListItemBaseStruct);    //继承单向链表的item属性
	
	//通道在虚拟设备中的序号，即第几个通道
	mlu8 addr;/**/
	mlu8 sta;/**/
	mlu8 stepType;/**/
	mlu8 substepType;/**/
	mlu8  stackDepth;/**/
	mlu8 stepNum;/**/
	mlu8 workStepRcvCnt;/**/
	mlu8 workStepRealCnt;/**/
	mlu8 jumpToStepNum;/**/
	mlu8 startBy;/**/
	mlu8 stopBy;/**/
	mlu8 DDC;/**/

	//通道延时检测状态
	mlu16 staCheckDlyTime;

	//操作
	mlu32  operate;
	
	//工步运行截止时间
	mlu32  stepRunTime;
    mlu32  lastCapaCalcTime;
	mlu32  stepRemainTime;
	
	//当前采样电流电压值，未经过逻辑处理的数据
	// 经过逻辑处理，便生成当前运行数据
	mls32 current;
	mls32 voltage;

	//上一次电流值
	mls32  lastCurrent;
	mlu32 lastVoltage;
	mlu32	lastCapaCalcCurrnet;
	
	//定时采样，采样间隔
	mlu32 sampleInterval;

	//通道数据缓存的索引、起始地址定义
	mlu32 cacheIndx;
	mlu32 curCacheAddr;

	//模块均分电流的临界值
	mlu32 avrgCriticalCurrent;


	//igbt 设备风机控制因数电流和功率记录
	mlu32 fanCtrlCurrent;
	mlu32 fanCtrlPower;

	//数据缓存索引
	mlu32 dataIndex;
	mlu32 dataSaveEndAddr;

	//通道容量
	uint64_t prevCapacity;
	uint64_t capacity;
	
	CmbPwrMdlptr mdl;	//模块
	ListPtr mdlList;//通道的模块链表
	CmbPwrMdlptr pCVmdl;//当前恒压模块

	subt

}CombinePowerChannel,CmbPwrChnl,*CmbPwrChnlptr;




//----------------------------------------------------------------------------
//export vari
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------

void cmbchnl_init(CmbPwrChnlptr pChnl,CmbPwrMdlptr pMdl,mlu8 mdlNum);


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif



