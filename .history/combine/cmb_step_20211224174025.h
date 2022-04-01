/**
 ******************************************************************************
 * 文件:combine_ntp_v1.h
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

#ifndef  _COMBINE_STEP__H__
#define  _COMBINE_STEP__H__

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if COMBINE_ENABLE

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"


//----------------------------------------------------------------------------
//define 
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// 工步类型定义
//----------------------------------------------------------------------------
#define  STEP_CCCV   				'A'     
#define  STEP_CCC   				'B'     // . _CCC---constant current charge   恒流充电
#define  STEP_CCD 				'C'     // . _CCD---constant current discharge 恒流放电
#define  STEP_QL	           			  'D'   // . _QL----- quiet lay   静置
#define  STEP_PULSC      			 'O'    //.  _PULSC--- pulse charge  		脉冲恒流充电
#define  STEP_PULSD     			  'P'   //. _PULSD---  pulse discharge		脉冲恒流放电
#define  STEP_SLOPEC      		'F'  // 斜坡充电
#define  STEP_SLOPED    			'E'  // 斜坡放电
#define  STEP_CWC				'G'    // . _CWC---constant (power)watt charge 恒功率充电,恒定的瓦特充电
#define  STEP_CWD			       'H'    // . _CWD---constant  (power)watt discharge 恒功率放电,恒定的瓦特放电
#define  STEP_CVC      				  'I'   // . _CVC---constant voltage charge   恒压充电 
#define  STEP_JUMP         			   'J'    // . _JUMP ---- jump   静置跳转工步
#define  STEP_CCPC     			 'K'    // . _CCPC---constant current pre charge   恒流预充电
#define  STEP_CVPC     			 'L'    // . _CVPC---constant voltage  precharge   恒压预充电
#define  STEP_CVPC2           'x' 
#define  STEP_MIDDLE     			 'M'   // ._MIDDLE---middle 中间工步
//#define  STEP_NOBAT  			  'N'    // 无电池
#define  STEP_NULL				'S'     // . _NULL-----null  空工步类型 即无操作
//#define  STEP_OVER			       'O'     // 流程测试完成
#define  STEP_DCIR         	 		 'V'  // . _DCIR-----direct current internal regsister  直流内阻测试  直流内阻三
#define  STEP_DCIRI         	 		 'R'  // . _DCIRI-----direct current internal regsister I直流内阻1
#define  STEP_CDCIR         	 		 'U'   //充电直流内阻

#define  STEP_WCS         	 		 'W'  // _WCS  ------work condition simulation 工况模拟

#define  STEP_CRD         	 		 'T'  // _CRD------constant register discharge

////探针测试
//#define  STEP_NeedleTest   		'a'     

//恒压放电 test
#define  STEP_CVD							'a'
#define  STEP_CCCV2							'y'     // . _CCCV2---constant current charge   恒流恒压2  国轩小电流


//----------------------------------------------------------------------------
// 工步定义
//----------------------------------------------------------------------------
#define STEP_BYTES                55
typedef  struct {

    struct 
    {
        mlu32 type:8;//工步类型
        mlu32 sampleInterval:24;//工步采样时间
    };

	// 1-5 5个主参数
	union{
		mlu32 mPara1;// 主参数1 
		mlu32 current; //cc cpc
		mlu32 voltage; //cvc  cvpc
		mlu32 power; //cwc cwd 
		mlu32 resistance; // dcir
		mlu32 jumpToStepNum;  //jump
	};
	
    mlu32 mPara2;
    mlu32 mPara3;
    mlu32 mPara4;
    mlu32 mPara5;
	
	// 1-4   4个限制参数
	union{

		//限制参数1
		mlu32  lPara1;
		mlu32  endVoltage;
		mlu32  endCurrent;
		mlu32  volLowEnd;
		mlu32  jumpTimes;
	};
	
	union{
		mlu64  lPara2;
		mlu64  endCapacity;
	};
	
	union{		
		mlu32  lPara3;
		mlu32  endTime;
	};
	
	union{
		mlu32  lPara4;

		mlu32  cvVoltage;//cv 恒压点电压cccv 工步
		mlu32  volUpperEnd;
	};

    mlu8 lParaAction[4];//限制参数的动作
    mlu8 lParaGotoStepNo[4];//限制参数跳转的工步号

}Step,*Stepptr;


//----------------------------------------------------------------------------
//  工步堆栈信息存储结构体定义
//----------------------------------------------------------------------------

 typedef struct{
	//压入堆栈的工步的工步号
	mlu8   pushStepNo;
	mlu8 	rsv;
	mlu16    popValue;//    出栈值，当值为0时出栈
}StepStack;

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif



