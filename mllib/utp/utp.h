/**
 ******************************************************************************
 * 文件:    utp.h
 * 作者:   流程
 * 版本:   V0.01
 * 日期:    
 * 内容简介:    lc mc 通讯协议相关数据类型定义，功能函数
 *                    接口导出
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
#ifndef  _UTP_H__
#define _UTP_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos.h"
#include "utp_default_config.h"
#include "utp_uart_driver.h"
#include "mlos_list.h"

//MB状态机
typedef enum
{
	mb_init			= 0,//初始化变态
	mb_wait_rx		= 1,//发送完等待接收
	mb_rxing 		= 2,//正在接受
	mb_rxcomplete 	= 3,//接受完成
	mb_wait_tx		= 4,//等待发送
}MODBUS_RSTA;

//----------------------------------------------------------------------------
//  串口 卡 定义
//----------------------------------------------------------------------------
typedef struct{
	mlu8 			id;
	mlu8			cardsta;		//0:未初始化完成，1初始化完成

	//MODBUS状态机
	MODBUS_RSTA 	mb_sta;
	Subtaskptr 		pMySubtask;
	
	mlu8*			TxBuffer;		//发送缓存
	mlu8* 			RxBuffer;		//RX缓存

	mlu16 			rxindex;		//接收索引
	mlu16			txlen;			//发送长度
	mlu8 			txover;			//发送是否完成
	mlu8 			TimeOut_cnt;	//记录达到t3.5次数
	mlu32	   		Tstamp;			//记录接收到新数据的系统时间
		
	//成员函数，定义
	mlu16 (*tx)(mlu8 *pbuf,mlu16 len);//uart卡的报文发送驱动接口
	void (*RxMesCallBack)(mlu8 portid,mlu8 *pbuf,mlu16 len);//uart卡的报文发送驱动接口
}UartCard,*UartCardptr;

//----------------------------------------------------------------------------
//  通讯传输协议数据类型定义
//----------------------------------------------------------------------------
 typedef struct{

 	//协议任务
 	Taskptr pMytask;

	UartCard Uartcard[UART_NUMS];
}UartTransmitProtocol,UARTtp,UTP,*UTPptr;

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

//外部声明
extern UartTransmitProtocol 	utp;

//初始化
void utp_init(uint8_t portid,void (*RxMes)(mlu8 portid,mlu8 *mes,mlu16 datalen));

/*
 * Utp_RxMessage
 * 参数:
 *     portid:id映射
 *     pbuf: 接收完成的buf的首地址
 *     datalen: 发送的长度
 *
 *
 * 功能：接收一条消息
 *
 * 返回值:NULL
 *
 */
void Utp_RxMessage(mlu8 portid,mlu8* msg,mlu16 len);

/*
 * Utp_TxMessage
 * 参数:portid:id映射
 *     pbuf: 需要发送的buf的首地址
 *     datalen: 发送的长度
 *
 * 返回值:NULL
 *
 * 功能:发送一则消息
 * 注意：平均1ms发送14个字节，250个字节需要21ms，32字节需要2~3ms
 */
void Utp_TxMessage(mlu8 portid,mlu8* msg,mlu16 len);

#endif
//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

