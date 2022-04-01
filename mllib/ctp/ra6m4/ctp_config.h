/**
 ******************************************************************************
 * 文件:    ctp_default_config.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    MSG_LEA_IDN
 * 内容简介:    lc mc 通讯协议相关数据类型定义，功能函数
 *                    接口导出
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                   zzx            创建该文件
 *
 *
 *
 ******************************************************************************
 **/
#ifndef  _CTP_DEFAULT_CONFIG_H__
#define _CTP_DEFAULT_CONFIG_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include"mlos_dtd.h"
#include"ctp_dtd.h"
#include"ctp_config.h"

//----------------------------------------------------------------------------
// config
//----------------------------------------------------------------------------

#ifndef CAN_NUM
//can 设备被数
#define CAN_NUM 		2
#endif

#ifndef CAN_BCAST_ADDR
//广播地址
#define CAN_BCAST_ADDR 				63
#endif


//----------------------------------------------------------------------------
// 配置，CAN 卡1 
//----------------------------------------------------------------------------
#ifndef CAN_CARD1_ENABLE
//使能CAN卡1
#define CAN_CARD1_ENABLE 					(1)

//CAN卡主从站配置
#define CAN_CARD1_STATION 		 			CTP_MASTER_STATION 		//主站

//收发队列大小配置
#define CAN_CARD1_TX_QUE_LEN				128
#define CAN_CARD1_RX_QUE_LEN 				128


//多帧接受buf 的大小 
#define CAN_CARD1_MFRXBUF_SIZE 			1280

//离线判断
#endif

//----------------------------------------------------------------------------
// 配置，CAN 卡2 
//----------------------------------------------------------------------------
#if (CAN_NUM>=2)
#ifndef CAN_CARD2_ENABLE
//使能CAN卡1
#define CAN_CARD2_ENABLE 					(1)

//CAN卡主从站配置
#define CAN_CARD2_STATION  		 			CTP_MASTER_STATION 		//主站

//收发队列大小配置
#define CAN_CARD2_TX_QUE_LEN				128
#define CAN_CARD2_RX_QUE_LEN 				128


//多帧接受buf 的大小 单位byte
#define CAN_CARD2_MFRXBUF_SIZE 				1280


//CAN 卡2 共享 CAN 卡1 消息泵，
#define CAN_CARD2_SHARE_CARD1_MSGPUMP 			(1)
#endif
#endif
//----------------------------------------------------------------------------
// ctp 属性配置,由CTP_TASK_PRIO 的定义，来启动默认定义
//----------------------------------------------------------------------------
#ifndef CTP_TASK_PRIO

//任务优先级
#define CTP_TASK_PRIO 				1

//从站，通讯间隔，即超过间隔无通讯，判定通讯失联
#define CTP_TM_SlvCommInterval			3000

//从站联机的时间间隔，即在离线状态下，没隔多久联机一次
#define CTP_TM_SlvConnInterval  		 1000			

#endif


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


