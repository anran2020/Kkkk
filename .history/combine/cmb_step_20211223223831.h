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
#define  WST_CCCV   				'A'     
#define  WST_CCC   				'B'     // . _CCC---constant current charge   恒流充电
#define  WST_CCD 				'C'     // . _CCD---constant current discharge 恒流放电
#define  WST_QL	           			  'D'   // . _QL----- quiet lay   静置
#define  WST_PULSC      			 'O'    //.  _PULSC--- pulse charge  		脉冲恒流充电
#define  WST_PULSD     			  'P'   //. _PULSD---  pulse discharge		脉冲恒流放电
#define  WST_SLOPEC      		'F'  // 斜坡充电
#define  WST_SLOPED    			'E'  // 斜坡放电
#define  WST_CWC				'G'    // . _CWC---constant (power)watt charge 恒功率充电,恒定的瓦特充电
#define  WST_CWD			       'H'    // . _CWD---constant  (power)watt discharge 恒功率放电,恒定的瓦特放电
#define  WST_CVC      				  'I'   // . _CVC---constant voltage charge   恒压充电 
#define  WST_JUMP         			   'J'    // . _JUMP ---- jump   静置跳转工步
#define  WST_CCPC     			 'K'    // . _CCPC---constant current pre charge   恒流预充电
#define  WST_CVPC     			 'L'    // . _CVPC---constant voltage  precharge   恒压预充电
#define  WST_CVPC2           'x' 
#define  WST_MIDDLE     			 'M'   // ._MIDDLE---middle 中间工步
//#define  WST_NOBAT  			  'N'    // 无电池
#define  WST_NULL				'S'     // . _NULL-----null  空工步类型 即无操作
//#define  WST_OVER			       'O'     // 流程测试完成
#define  WST_DCIR         	 		 'V'  // . _DCIR-----direct current internal regsister  直流内阻测试  直流内阻三
#define  WST_DCIRI         	 		 'R'  // . _DCIRI-----direct current internal regsister I直流内阻1
#define  WST_CDCIR         	 		 'U'   //充电直流内阻

#define  WST_WCS         	 		 'W'  // _WCS  ------work condition simulation 工况模拟

#define  WST_CRD         	 		 'T'  // _CRD------constant register discharge

////探针测试
//#define  WST_NeedleTest   		'a'     

//恒压放电 test
#define  WST_CVD							'a'
#define  WST_CCCV2							'y'     // . _CCCV2---constant current charge   恒流恒压2  国轩小电流




















//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif



