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
#include "ctp.h"
#include<string.h>
#include "box.h"

//----------------------------------------------------------------------------
// defines
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
// global  variable
//----------------------------------------------------------------------------	

CANFrame  can1RxfrmBuf[CAN_CARD1_RX_QUE_LEN];							//接收帧缓存
CANFrame  can1TxfrmBuf[CAN_CARD1_TX_QUE_LEN];							//发送帧缓存

#if (CAN_NUM>=2)
CANFrame  can2RxfrmBuf[CAN_CARD2_RX_QUE_LEN];
CANFrame  can2TxfrmBuf[CAN_CARD2_TX_QUE_LEN];

#endif

//系统传输协议
CanTransmitProtocol 	ctp;

mlu8 ctpTxbuf[1048];

//----------------------------------------------------------------------------
// extern variable
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	 :	void
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
mlu8* ctp_txbuf_malloc(mlu16 len)
{	
	return ctpTxbuf;
}



//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	 :	void
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
CANFrmptr ctp_txframe_malloc(CANCardID ccid)
{

	return que_en(ctp.canCard[ccid].pTxQue);
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
 void ctp_conn_port_set(CANCardID ccid,void (*txConnectMsgPort)(void))
{
	if(ctp.canCard[ccid].pMySlvStation!=nullptr)
	{
		ctp.canCard[ccid].pMySlvStation->connect=txConnectMsgPort;
		
	}
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	 :	void
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
 void ctp_connected(CANCardID ccid)
{
	 if(ctp.canCard[ccid].pMySlvStation!=nullptr)
	 {
		 ctp.canCard[ccid].pMySlvStation->connSta=_connected;//从站联机成功
		 //重启联机定时器,进入通讯间隔监测
		 ltimer_load_start(ctp.canCard[ccid].pMySlvStation->pConnTimer,CTP_TM_SlvCommInterval);
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
void ctp_slaver_connection_monitor(CANCardptr pCANCrad)
{
	//slvaer 才需要监测联机
	if(pCANCrad->pMySlvStation==nullptr)
	{
		return;
	}

	//已建立联机
	if(pCANCrad->pMySlvStation->connSta==_connected)
	{
		//通讯是否正常，监测
		if(ltimer_timeout(pCANCrad->pMySlvStation->pConnTimer))
		{
			//CAN slaver 离线，发送系统消息
			mpump_dispatch(pCANCrad->pMymsgpump,CTP_MSG_commException, nullptr);			//发送CAN通讯掉线消息
			//重启定时器，进入联机时间间隔
			ltimer_load_start(pCANCrad->pMySlvStation->pConnTimer,CTP_TM_SlvConnInterval);
			pCANCrad->pMySlvStation->connSta=_disconnected;//置离线状态
		}
		
	}
	else 
	{
		//建立连接
		if(ltimer_timeout(pCANCrad->pMySlvStation->pConnTimer))
		{
			//发生自主联机
			if(pCANCrad->pMySlvStation->connect!=nullptr)
			{
				pCANCrad->pMySlvStation->connect();
			}

			//启动定时，下次联机
			ltimer_start(pCANCrad->pMySlvStation->pConnTimer);
		}
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
void ctp_consumer_register(CANCardID ccid,CANtpMsgIDN idn, MsgCallbackFunction callback)
{

	mpump_consumer_register(ctp.canCard[ccid].pMymsgpump, idn, callback);

}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : 
//CtpSerialNumber 返回发送数据的流水号，
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
CtpSerialNumber ctp_sn_generate(CANCardID cid)
{
	ctp.canCard[cid].mfSeriNmber++;
	return ctp.canCard[cid].mfSeriNmber;
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : 
//CtpSerialNumber 返回发送数据的流水号，=CTP_SN_ERROR标识发送失败，
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
MLBool ctp_send(CANCardID ccid,mlu8 addr,mlu8*pBuf,mlu16 len)
{
	
	mlu8 frmtotal,lastFrmBytes;	
	mlu16 i,j;
	CANFrmptr pCANFrame;
	Queptr pQue;

	//MLOS_ASSERT((len==0||pBuf==nullptr));
	

	pQue=ctp.canCard[ccid].pTxQue;
	//计算总帧数
	lastFrmBytes=len%CAN_MF_DAT_LEN;
	frmtotal=(lastFrmBytes>0)?(len/CAN_MF_DAT_LEN+1):(len/CAN_MF_DAT_LEN);

	
	//填充帧数据
	for(i=0;i<frmtotal;i++)
	{
		pCANFrame=que_en(pQue);
		if (pCANFrame==nullptr)
		{
			return mlfalse;
		}
		can_mfid_set(pCANFrame,addr,i);
		for (j = 0; j < CAN_SF_DAT_LEN; ++j)
		{
			pCANFrame->data[j]=*pBuf++;
		}
		pCANFrame->mfTxIDbit.DATA9=*pBuf++;
		pCANFrame->mfTxIDbit.DATA10=*pBuf++;
	}

	return len;
	
}

//-----------------------------------------------------------------------------
// 函数名：ctp_mfmsg_repack
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：多帧数据包重组
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void ctp_mfmsg_repack(CANCardptr pCANCrad,CANFrmptr pRxfrm)
{
	mlu16 datlen=CAN_MF_DAT_LEN; 	
	mlu8* prxDat;
	prxDat=pRxfrm->data;
	if(pRxfrm->mfRxIDbit.INDX==0)												//当帧序=0 表示多帧的起始
	{
		pCANCrad->pMfmsg->sta=CAN_MFRX_ING;											//装载起始帧数据
		pCANCrad->pMfmsg->ccid=pCANCrad->id;
		pCANCrad->pMfmsg->addr=pRxfrm->mfRxIDbit.ADDR;
		pCANCrad->pMfmsg->nextFrame=1;
		pCANCrad->pMfmsg->offsetptr=pCANCrad->pMfmsg->buffer;
		while (datlen--)
		{
			*pCANCrad->pMfmsg->offsetptr++=*prxDat++;
		}
		pCANCrad->pMfmsg->frmTotal=((pRxfrm->u16Data2%10)>0)?(pRxfrm->u16Data2/10+1):(pRxfrm->u16Data2/10);
		pCANCrad->pMfmsg->idn=pRxfrm->u8Data2;
	}
	else
	{
		
		if(pRxfrm->mfRxIDbit.ADDR==pCANCrad->pMfmsg->addr&&
			pCANCrad->pMfmsg->nextFrame==pRxfrm->mfRxIDbit.INDX)				//接收数据，判断帧序，是否正确
		{
			while (datlen--)
			{
				*pCANCrad->pMfmsg->offsetptr++=*prxDat++;
			}
			pCANCrad->pMfmsg->nextFrame++;
		}
		else
		{	
				pCANCrad->mfRxError++; 										//帧序 错误，统计
		}
		
	}


	if(pCANCrad->pMfmsg->nextFrame>=pCANCrad->pMfmsg->frmTotal)				//判断多帧接收完整
	{			
		pCANCrad->pMfmsg->sta=CAN_MFRX_COMPLETED;							//输出完整消息
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
_inline void ctp_slaver_online(CANCardptr pCANCrad)
{
	//从站，刷新在线状态
	if(pCANCrad->pMySlvStation!=nullptr&&pCANCrad->pMySlvStation->connSta==_connected)
	{
		//重启计时器
		ltimer_start(pCANCrad->pMySlvStation->pConnTimer);
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
void ctp_rx(CANCardptr pCANCrad)
{
	QueItemptr  pCurTail;
	CANFrmptr pCANFrame;
	Queptr pQue;

	
	//取CAN对应的接收队列
	pQue=pCANCrad->pRxQue;
	pCurTail=pQue->tail;//记录本次处理队列的终点，保证队列的原子操作，若有新数据入列，下次再出
	

	//循环处理队列里所有报文
	while(pQue->head!=pCurTail)
	{
		//取报文解析
		pCANFrame=que_take(pQue);
		if(pCANFrame!=nullptr)
		{
			//接收到报文，刷新从站联机状态
			ctp_slaver_online(pCANCrad);
			
			//单帧处理
			if(pCANFrame->sfRxIDbit.PF==CAN_PF_SINGLE)
			{
				mpump_dispatch(pCANCrad->pMymsgpump,pCANFrame->sfRxIDbit.IDN, pCANFrame);						//发送消息
			}
			else
			{
				
				ctp_mfmsg_repack(pCANCrad,pCANFrame);															//接收重组多帧消息
				if(pCANCrad->pMfmsg->sta == CAN_MFRX_COMPLETED)
				{
					//分派消息
					if(mlfalse==mpump_dispatch(pCANCrad->pMymsgpump,pCANCrad->pMfmsg->idn, pCANCrad->pMfmsg))
					{
						canRxMsg(pCANCrad->pMfmsg->ccid, pCANCrad->pMfmsg->addr, pCANCrad->pMfmsg->buffer, pCANCrad->pMfmsg->frmTotal*10);
					}				
                    //boxRxMsg(pCANCrad->pMfmsg->ccid, pCANCrad->pMfmsg->addr, pCANCrad->pMfmsg->buffer, pCANCrad->pMfmsg->frmTotal*10);
               
					pCANCrad->pMfmsg->sta = CAN_MFRX_FREE;
				}
			}		
		}	

		//处理完报文，出列
		que_de(pQue);
	
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
void ctp_tx(CANCardptr pCANCrad)
{
	Queptr pQue;
	CANFrmptr pCANFrame;

	//发送报文	
	pQue=pCANCrad->pTxQue;

	//循环发送队列里所有报文
	while(pQue->head!=pQue->tail)
	{
		//取报文发送
		pCANFrame=que_take(pQue);
		if(pCANFrame!=nullptr)
		{
			//取队列节点数据
			if(pCANCrad->tx(pCANCrad->id,pCANFrame))
			{
					//发送失败
					break;
			}
		}
		
		//发送完报文，出列
		que_de(pQue);
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
void ctp_test(void)
{
	static CANCardID ccid=0;
	CANFrmptr ptxframe;

	if (!ltimer_timeout(ctp.pTestTimer))
	{
		return;
	}
	

	
	ptxframe=ctp_txframe_malloc(ccid);
	if (ptxframe!=nullptr)
	{
		//
		if(ccid==CAN_CARD_1)
		{
			ptxframe->id=0x1234;
			ptxframe->dataH=0x56789;
			ptxframe->dataL=0x12345;
		}
		else
		{
			ptxframe->id=0x4321;
			ptxframe->dataH=0x98765;
			ptxframe->dataL=0x54321;
		}

	
	}
	
	if(ccid==CAN_CARD_1)
	{
		ccid=CAN_CARD_2;
	}
	else
	{
		ccid=CAN_CARD_1;
	}


	ltimer_start(ctp.pTestTimer);
	
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
Taskstate ctp_task(void* args)
{
	task_declaration();										//任务开始
	mlu8 i;
	CANCardptr pCANCrad;
	
	task_begin(ctp.pMytask);								//任务开始

	//ctp_test();

	for (i = 0; i < CAN_NUM; ++i)						//轮询CAN卡，处理消息
	{
		pCANCrad=ctp.canCard+i;
	
		ctp_rx(pCANCrad);										//接收消息
		ctp_slaver_connection_monitor(pCANCrad); 				//协议连接监测，从站需要检测自身的联机状态	
		ctp_tx(pCANCrad);										//发送消息报文	
	}

	task_end(ctp.pMytask);									//任务结束
	
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
// 1.  zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void ctp_cancard1_init(mlu8 CAN1Addr,CANBaudRate baudrate)
{
	CANCardptr pCANCard;
	MsgPumpptr pMpump;
	
	//CAN 卡 1 初始化
	pCANCard=ctp.canCard+CAN_CARD_1;
	pCANCard->id=CAN_CARD_1;
	pCANCard->station=CAN_CARD1_STATION;
	pCANCard->mfRxError=0;

	//收发队列初始化
	pCANCard->pRxQue=que_create(e_mem_sram, CAN_CARD1_RX_QUE_LEN, sizeof(CANFrame),(mlu8*)can1RxfrmBuf);
	pCANCard->pTxQue=que_create(e_mem_sram,CAN_CARD1_TX_QUE_LEN, sizeof(CANFrame),(mlu8*)can1TxfrmBuf);
	
	
	//多帧接收单元初始化
	pCANCard->pMfmsg=mlos_malloc(e_mem_sram, sizeof(CtpMultiFrameMsg));
	pCANCard->pMfmsg->buffer=mlos_malloc(e_mem_sram, CAN_CARD1_MFRXBUF_SIZE);
	pCANCard->pMfmsg->bufSize=CAN_CARD1_MFRXBUF_SIZE;
	pCANCard->tx=can_tx;
	pCANCard->pMfmsg->sta=CAN_MFRX_FREE;
	pCANCard->pMfmsg->nextFrame=0;
	
	//若can卡初始从站
	if(pCANCard->station==CTP_SLAVER_STATION)
	{
		//从站初始化
		pCANCard->pMySlvStation=mlos_malloc(e_mem_sram,sizeof(CANtpSlaverStation));
		pCANCard->pMySlvStation->connSta=_disconnected;
		pCANCard->pMySlvStation->connect=nullptr;
		pCANCard->pMySlvStation->pConnTimer=ltimer_create();
		ltimer_load_start(pCANCard->pMySlvStation->pConnTimer, CTP_TM_SlvConnInterval);//启动定时联机
	}
	else
	{
		//主站，
		pCANCard->pMySlvStation=nullptr;
	}

	//创建CAN卡消息泵
	pMpump=mpump_create(e_mem_sram,0);
	pCANCard->pMymsgpump=pMpump;
	
	//初始化can外设
	can_init(CAN_CARD_1,CAN1Addr,baudrate, pCANCard->pRxQue);

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
// 1.  zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
#if (CAN_CARD2_ENABLE)
void ctp_cancard2_init(mlu8 CAN2Addr,CANBaudRate baudrate)
{
	CANCardptr pCANCard;
	MsgPumpptr pMpump;

	
	pCANCard=ctp.canCard+CAN_CARD_2;
	pCANCard->id=CAN_CARD_2;
	pCANCard->station=CAN_CARD2_STATION;
	pCANCard->mfRxError=0;

	//收发队列初始化
	pCANCard->pRxQue=que_create(e_mem_sram, CAN_CARD2_RX_QUE_LEN, sizeof(CANFrame),(mlu8*)can2RxfrmBuf);
	pCANCard->pTxQue=que_create(e_mem_sram,CAN_CARD2_TX_QUE_LEN, sizeof(CANFrame),(mlu8*)can2TxfrmBuf);

	
	//多帧接收单元初始化
	pCANCard->pMfmsg=mlos_malloc(e_mem_sram, sizeof(CtpMultiFrameMsg));
	pCANCard->pMfmsg->buffer=mlos_malloc(e_mem_sram, CAN_CARD2_MFRXBUF_SIZE);
	pCANCard->pMfmsg->bufSize=CAN_CARD2_MFRXBUF_SIZE;
	pCANCard->tx=can_tx;
	pCANCard->pMfmsg->sta=CAN_MFRX_FREE;
	pCANCard->pMfmsg->nextFrame=0;

		//若can卡初始从站
	if(pCANCard->station==CTP_SLAVER_STATION)
	{
		pCANCard->pMySlvStation=mlos_malloc(e_mem_sram,sizeof(CANtpSlaverStation));
		pCANCard->pMySlvStation->connSta=_disconnected;
		pCANCard->pMySlvStation->connect=nullptr;
		pCANCard->pMySlvStation->pConnTimer=ltimer_create();
		ltimer_load_start(pCANCard->pMySlvStation->pConnTimer, CTP_TM_SlvConnInterval);//启动定时联机
	}
	else
	{
		pCANCard->pMySlvStation=nullptr;
	}
	
	//CAN卡共享消息泵
	#if (CAN_CARD2_SHARE_CARD1_MSGPUMP)
	{
		//共享，can card 1 的消息泵
		pCANCard->pMymsgpump=ctp.canCard[CAN_CARD_1].pMymsgpump;
	}
	#else
	{
		//创建自己的CAN卡消息泵
		pMpump=mpump_create(e_mem_sram);
		pCANCard->pMymsgpump=pMpump;
	}
	#endif
	
	//初始化can外设
	can_init(CAN_CARD_2,CAN2Addr,baudrate, pCANCard->pRxQue);


}
#endif

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
// 1.  zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void ctp_init(mlu8 CAN1Addr,CANBaudRate baudrate1,mlu8 CAN2Addr,CANBaudRate baudrate2)
{


	//CAN card 1 init
	ctp_cancard1_init(CAN1Addr,baudrate1);

	//CAN 卡 2 初始化,CAN2 根据MCU配置，不是所有的MCU都有两个CAN
	#if (CAN_CARD2_ENABLE)
		ctp_cancard2_init(CAN2Addr,baudrate2);
	#endif


	//创建协议任务
	ctp.pMytask=task_create(CTP_TASK_PRIO, ctp_task, nullptr, nullptr,"ctp");

	ctp.pTestTimer=ltimer_create();
	ltimer_load_start(ctp.pTestTimer, 1000);
	
}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

