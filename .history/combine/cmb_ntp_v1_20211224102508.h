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

#ifndef  _COMBINE_NTP_V1__H__
#define  _COMBINE_NTP_V1__H__

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
//					
//----------------------------------------------------------------------------

//联机命令
#define  CMD_Connect                       	 'A'    

// 修调通道
#define  CMD_Repair                     		'B'    

// 通道修调信息更新
#define  CMD_UpdateRepairInfo             'C'    

//通道并联
#define  CMD_SetupParallel          		 'D'    
#define  CMD_DestroyParallel 			'E'    

#define  CMD_WorkStepInfo                  		 'F'    // 'F'传送动作控制代码

 // 更新通道的跳转堆栈的信息、igbt设备通道续接
#define  CMD_UpdateStack      					 'G'   
#define  CMD_ContinueTest					 'G'   

//获取跳转堆栈信息
#define  CMD_GetStack       			 'H'    

//托盘通道续接、清除所有绑定
#define CMD_TrayContinue                      'I'
#define CMD_ClearAllParall                      'I'

//托盘联机命令
#define   CMD_ConnectTray          		 'J'

#define  CMD_LED_ON                     		 'K'    // 'K': 通道指示灯

//控制通道(启停)、启动通道、启动托盘
#define  CMD_ControlChnl                 	'L'    // 'L'：通道控制
#define  CMD_StartChnl                 		  'L'  
#define  CMD_StartTray                 		  'L'  

#define  CMD_LED_OFF                		  'M'    // 'M': 代表将所有指示不合格的指示灯熄灭

//强制跳转
#define   CMD_ForceJump                     'N' 
#define   CMD_RSV04                               'O'

//电压保护、停止通道、停止托盘
#define  CMD_VolSafeProtect               	 'P'    
#define  CMD_StopChnl		             	 'P'  
#define  CMD_StopTray            		  	 'P' 

#define  CMD_R_EEPROM_BATCH             'Q'    // 批量读取SBS数据
#define  CMD_Pause           		       'Q'    	//通道暂停

#define  CMD_ROM_READ 		           'R'    // 'R'：读，
#define  CMD_CHNL_PARALELL               	 'R' 

#define  CMD_Sample                 			   'S'    // 采集AD转换器读取的数据
#define  CMD_PASS_NG             	   'T'    // 标记通道好坏命令
#define  CMD_SERIAL_INFO                		 'U'    // 序列号信息更新

#define   CMD_RSV05                              'V'

//工况模拟信息、写cpu ROM 、
#define  CMD_WCSInfo 		           'W'    
#define  CMD_ROMWrite 		    	     'W'    


#define   CMD_RSV06                              'X'
#define   CMD_RSV07                              'Y'

#define  CMD_StopAll                   		 'Z'    // 广播停止


#define   CMD_RSV08                         		   'a'
#define  CMD_LED_BLINK               			  'b'     //通道指示灯闪烁


//、配置ip 信息、请求子工步信息
#define CMD_CONFIG_IP_INFO			 'c'
#define  CMD_ReqSubStepInfo                'c'

//在线升级连接、开始、数据、结束、校验、跳转到app
#define CMD_IAPConnect 			 'c'
#define CMD_IAPStart 			 'd'
#define CMD_IAPData			       'e'
#define CMD_IAPEnd			       'f'
#define CMD_IAPVerify			'g'
#define CMD_IAPJumpToApp		'h'


#define CMD_rsv9			     'i'
#define CMD_rsv10			     'j'
#define CMD_rsv11			     'k'

#define CMD_rsv12 				 'l'

#define CMD_rsv13			     'm'
#define CMD_rsv14			     'n'
#define CMD_rsv15			     'o'

//托盘暂停
#define CMD_TrayPause			'p'

#define CMD_rsv17			     'r'


#define  CMD_SBS_SAMPLE              's'    // 's'：采集SMBUS总线读取的数据
#define CMD_GET_IP_INFO		's'

#define CMD_rsv18			     't'

//协议握手成功事件消息,虚拟定义成命令字
#define CMD_rsv19			     'u'

// 停止托盘通道
#define  CMD_StopTrayChnl 		     'w'    

//#define  CMD_ 		     'x'    
//#define  CMD_		     'y'  

//网络断开事件消息，虚拟定义成命令字
#define  CMD_NetConnLost 		     'z' 

#define ALARM_LIGHT    'f'  //报警灯命令字

//----------------------------------------------------------------------------
//export vari
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------

void cmb_ntp_init(void);


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif



