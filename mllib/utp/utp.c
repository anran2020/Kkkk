/**
 ******************************************************************************
 * 文件:    ctp.c
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:    lc mc 通讯协议
 *          
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                     2014-12-01            zzx                      创建该文件
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "utp.h"
#include "mlos_clock.h"
#include "public.h"

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------
Taskstate utp_task(void *args);

//----------------------------------------------------------------------------
// global  variable
//----------------------------------------------------------------------------	
//串口传输协议
UartTransmitProtocol 	utp;

//----------------------------------------------------------------------------
// extern variable
//----------------------------------------------------------------------------

void Utp_TxMessage(mlu8 portid,mlu8* msg,mlu16 datalen)
{
	memcpy(utp.Uartcard[portid].TxBuffer,msg,datalen);
	utp.Uartcard[portid].txlen 	= datalen;
	utp.Uartcard[portid].mb_sta = mb_wait_tx;
}

//-----------------------------------------------------------------------------
// 函数名：utp_init
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
// 功能描述：
// 初始化各路uart
//-----------------------------------------------------------------------------
void utp_init(uint8_t portid,void (*RxMes)(mlu8 portid,mlu8 *mes,mlu16 datalen))
{
	mlu8 i = 0;

	if(portid >= UART_NUMS)
	{
		return;
	}

	utp.Uartcard[portid].mb_sta = mb_init;
	utp.Uartcard[portid].TxBuffer			= mlos_malloc(e_mem_sram, 256);
	utp.Uartcard[portid].RxBuffer			= mlos_malloc(e_mem_sram, 256);
	utp.Uartcard[portid].id					= portid;
	utp.Uartcard[portid].rxindex			= 0;
	utp.Uartcard[portid].txlen				= 0;
	utp.Uartcard[portid].TimeOut_cnt		= 0;

	if(portid == 0)
	{
		utp.Uartcard[portid].tx				= uart0_tx;
	}
	else if(portid == 1)
	{
		utp.Uartcard[portid].tx				= uart1_tx;
	}

	utp.Uartcard[portid].pMySubtask  		= subtask_create(ltimer_create ());
	utp.Uartcard[portid].cardsta			= 1;

	utp.Uartcard[portid].RxMesCallBack		= RxMes;

	
	if(utp.pMytask == NULL)
	{
		utp.pMytask=task_create(UTP_TASK_PRIO, utp_task,  &utp, ltimer_create (),"utp_task");//轮询发送，并且队列发送

		uart_init();
	}


	return;
}

/*
 * 函数名:Rx_Poll
 * 参数:Puart：对应哪一路uart
 * 功能：轮询查看接收状态
 */
mlu8 utp_RxComplete(UartCardptr Puart)
{
	mlu8 ret = 0;
	mlu32 timeff;
	MODBUS_RSTA mb_sta;

	mb_sta = Puart->mb_sta;

	if(mb_sta != mb_rxing)
	{
		ret = 1;
	}
	else
	{
		timeff = GetOSRunTimeDiff(Puart->Tstamp);

		if(mb_sta == mb_rxing && timeff > 5)//多次识别
		{
			Puart->TimeOut_cnt++;
			if(Puart->TimeOut_cnt == 3)
			{
				Puart->TimeOut_cnt = 0;
				Puart->mb_sta = mb_rxcomplete;
			}
			else
			{
				ret = 2;
			}
		}
		else
		{
			Puart->TimeOut_cnt = 0;
			ret = 3;
		}
	}



	return ret;
}

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
 * 返回值:接收的长度
 *
 */void Utp_RxMessage(mlu8 portid,mlu8 *msg,mlu16 len)
{
	 UartCardptr Puart = NULL;

	 Puart = &utp.Uartcard[portid];

	 //CRC校验
	 if(crc16_check(msg,len) != 0)
	 {
		 return;
	 }

	 len -= 2;
	 if(Puart->RxMesCallBack != NULL)
	 {
		 Puart->RxMesCallBack(portid,msg,len);
	 }
}

/*
*	UTP队列数据发送
*/
mlu8 utp_tx(UartCardptr Puart)
{
	mlu16 crc16;

	if(Puart->mb_sta != mb_wait_tx)
	{
		return 1;
	}

	crc16 = crc16_check(Puart->TxBuffer, Puart->txlen);
	Puart->TxBuffer[Puart->txlen++] = crc16;
	Puart->TxBuffer[Puart->txlen++] = crc16 >> 8;

	//数据发送
	Puart->rxindex = 0;
	Puart->mb_sta  = mb_wait_rx;
	Puart->tx(Puart->TxBuffer,Puart->txlen);
	Puart->txlen = 0;

	return 0;
}

//-----------------------------------------------------------------------------
// 函数名：Utp_SubMasterStationtask
//-----------------------------------------------------------------------------
// 返回值 : void
// 参数   :  args(指向对应的uart卡)
//功能描述：发送一条消息且接收处理一条消息
//-----------------------------------------------------------------------------
mlu8 Utp_SubMasterStationtask(void* args)
{
	UartCardptr UCardPtr;

	subtask_declaration();										//任务开始

	UCardPtr = (UartCardptr)args;

	subtask_begin(UCardPtr->pMySubtask);						//任务开始

	//轮训是否有消息需要接收
	if(!utp_RxComplete(UCardPtr))
	{
		Utp_RxMessage(UCardPtr->id,UCardPtr->RxBuffer,UCardPtr->rxindex);
	}

	//轮训是否有消息需要发送
	if(utp_tx(UCardPtr))
	{
		subtask_exit(UCardPtr->pMySubtask,0);
	}

	subtask_wait_cnd_until(UCardPtr->pMySubtask,!UCardPtr->txover,50,0);//等待发送完成，防止发送阻塞任务
	if(UCardPtr->txover == 1)//发送失败
	{
		subtask_exit(UCardPtr->pMySubtask,0);
	}

	subtask_end(UCardPtr->pMySubtask,0);									//任务结束
}

Taskstate utp_task(void *args)
{
	mlu8 i;
	task_declaration();

	task_begin(utp.pMytask);								//任务开始

	for(i = 0;i < UART_NUMS;i++)
	{
		if(utp.Uartcard[i].cardsta == 1)
		{
			Utp_SubMasterStationtask(&utp.Uartcard[i]);
		}
	}

	task_end(utp.pMytask);									//任务结束
}
//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

