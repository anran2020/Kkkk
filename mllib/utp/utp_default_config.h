/**
 ******************************************************************************
 * 文件:  utp_default_config.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    
 * 内容简介:    
 *                    
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                                                            创建该文件
 *
 *
 *
 ******************************************************************************
 **/
#ifndef  _UTP_DEFAULT_CONFIG_H__
#define _UTP_DEFAULT_CONFIG_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include"mlos_dtd.h"
#include"utp_def.h"


//----------------------------------------------------------------------------
// config
//----------------------------------------------------------------------------
//UART卡的数量
#define UART_NUMS 2

//接收数组大小
#define RS48_QUEUE_SIZE 256

//----------------------------------------------------------------------------
// utp 属性配置,由UTP_TASK_PRIO 的定义，来启动默认定义
//----------------------------------------------------------------------------
#ifndef UTP_TASK_PRIO

//任务优先级
#define UTP_TASK_PRIO 				2

		

#endif


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


