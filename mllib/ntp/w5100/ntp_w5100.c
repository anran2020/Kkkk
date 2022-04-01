/**
******************************************************************************
* 文件:net.c
* 作者:   zzx
* 版本:   V0.01
* 日期:     2014-12-01
* 内容简介:
*
******************************************************************************
*  					文件历史
*
* 版本号-----  日期   -----   作者    ------   说明
* v0.1                       2014-12-01          zzx                 创建该文件
*
*
*
******************************************************************************
**/

//----------------------------------------------------------------------------
//文件条件编译
//----------------------------------------------------------------------------


#if (1)


//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------  
#include "mlos_ltimer.h"
#include "mlos_task.h"
#include "ntp_usocket.h"
#include"ntp_w5100.h"
#include "ntp_def.h"
#include"ntp_w5100_bus_driver.h"


//----------------------------------------------------------------------------
//  defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// global variable
//----------------------------------------------------------------------------

NetworkCard w5100;


//----------------------------------------------------------------------------
// extern func
//----------------------------------------------------------------------------


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
void w5100_swt_isr(void)
{
	
	unsigned char *ptr,regVal;	
	
	//读取s0 中断寄存器，查询sndok bit
	//
	//回写清中断标志

	//读取中断标识
	ptr=( unsigned char*)W5100_IR;
	regVal=*ptr;
	*ptr=regVal;

	//s1 tcp 只监测tcp 中断管脚长时间低电平
	ptr=( unsigned char*)W5100_S0_IR;
	regVal=*ptr;
	*ptr=regVal;
						

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
unsigned int w5100_socket_rx(USockptr pusck)
{
	unsigned char *ptr;
	unsigned int i,rx_size,rx_offset;

	//读取接收数据的字节数
	ptr=(unsigned char*)(W5100_S0_RX_RSR+pusck->sn*0x100);
	rx_size=*ptr;
	ptr++;
	rx_size=(rx_size<<8);
	rx_size+=*ptr;

	//读取接收缓冲区的偏移量
	ptr=(unsigned char*)(W5100_S0_RX_RR+pusck->sn*0x100);
	rx_offset=*ptr;
	ptr++;
	rx_offset=(rx_offset<<8);
	rx_offset+=*ptr;

	//计算实际的物理偏移量，S_RX_SIZE需要在前面#define中定义
	//S_RX_SIZE的值在W5100_Init()函数的W5100_RMSR中确定
	i=rx_offset/S_RX_SIZE;
	rx_offset=rx_offset-i*S_RX_SIZE;

	//实际物理地址为W5100_RX+rx_offset
	//将数据缓存到Rx_buffer数组中
	ptr=(unsigned char*)(W5100_RX+pusck->sn*S_RX_SIZE+rx_offset);
	for(i=0;i<rx_size;i++)
	{
		if(rx_offset>=S_RX_SIZE)
		{
			ptr=(unsigned char*)(W5100_RX+pusck->sn*S_RX_SIZE);
			rx_offset=0;
		}
		pusck->pRxBuf[i]=*ptr;
		ptr++;
		rx_offset++;
	}

	//计算下一次偏移量
	ptr=(unsigned char*)(W5100_S0_RX_RR+pusck->sn*0x100);
	rx_offset=*ptr;
	ptr++;
	rx_offset=(rx_offset<<8);
	rx_offset+=*ptr;
	
	rx_offset=rx_offset+rx_size;
	ptr=(unsigned char*)(W5100_S0_RX_RR+pusck->sn*0x100);
	*ptr=(rx_offset>>8);
	ptr++;
	*ptr=rx_offset;

	//设置RECV命令，等下一次接收
	ptr=(unsigned char*)(W5100_S0_CR+pusck->sn*0x100);
	*ptr=S_CR_RECV;

	//返回接收数据的字节数
	pusck->rxDataLen+=rx_size;
	return rx_size;
	
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
unsigned int w5100_socket_tx(USockptr pusck)
{
	unsigned char *ptr;
	unsigned int i;
	unsigned int txFreeSize,txOffset;

	//如果是UDP模式,可以在此设置目的主机的IP和端口号
	if(pusck->mode==S_MR_UDP)
	{
		//socket 1 
			ptr=(unsigned char*)(W5100_S0_DIPR+pusck->sn*0x100);
			for(i=0;i<4;i++){
				*ptr=pusck->remoteip[i];
				ptr++;
			}
			ptr=(unsigned char*)(W5100_S0_DPORT+pusck->sn*0x100);
			*ptr=(pusck->remoteport>>8);
			ptr++;
			*ptr=pusck->remoteport;

	}

	//读取缓冲区剩余的长度
	//如果剩余的字节长度小于发送字节长度,则返回
	ptr=(unsigned char*)(W5100_S0_TX_FSR+pusck->sn*0x100);
	txFreeSize=*ptr;
	txFreeSize=(txFreeSize<<8);
	ptr++;
	txFreeSize+=(*ptr);
	if(txFreeSize<pusck->txDataLen)
		return 0;

	ptr=(unsigned char*)(W5100_S0_TX_WR+pusck->sn*0x100);
	txOffset=*ptr;
	txOffset=(txOffset<<8);
	ptr++;
	txOffset+=(*ptr);

	//计算实际的物理偏移量
	i=txOffset/S_TX_SIZE;
	txOffset=txOffset-i*S_TX_SIZE;

	//实际物理地址为W5100_TX+txOffset
	//将Tx_buffer缓冲区中的数据写入到发送缓冲区
	ptr=(unsigned char*)(W5100_TX+pusck->sn*S_TX_SIZE+txOffset);
	for(i=0;i<pusck->txDataLen;i++)
	{
		if(txOffset>=S_TX_SIZE)
		{
			ptr=(unsigned char*)(W5100_TX+pusck->sn*S_TX_SIZE);
			txOffset=0;
		}
		*ptr=pusck->pTxBuf[i];
		ptr++;
		txOffset++;
	}

	//计算下一次的偏移量
	ptr=(unsigned char*)(W5100_S0_TX_WR+pusck->sn*0x100);
	txOffset=*ptr;
	txOffset=(txOffset<<8);
	ptr++;
	txOffset+=(*ptr);

	txOffset+=pusck->txDataLen;
	ptr=(unsigned char*)(W5100_S0_TX_WR+pusck->sn*0x100);
	*ptr=(txOffset>>8);
	ptr++;
	*ptr=txOffset;

	//设置SEND命令,启动发送
	ptr=(unsigned char*)(W5100_S0_CR+pusck->sn*0x100);
	*ptr=S_CR_SEND;

	return pusck->txDataLen;

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
void w5100_loop_back_test(void)
{
//	unsigned short size;
//	unsigned char *ptr;
		/***** First step: Read data from RX buffer and dump them in S_Buffer ****/
//		size=s_rx_process(0);
		/***** Second step: Write S_Buffer data to TX buffer and send out ****/
		/**** Wait for SendOK or TimeOut event occur***/

//		ptr=(unsigned char*)W5100_S0_CR;
//		while((socketSndOkFlag==0)&&(socketTimeOutFlag==0))
		{
//			if((*ptr)==0)	
//			{
//				socketSndOkFlag=1;
//			}
		}
//		socketSndOkFlag=0;
//		socketTimeOutFlag=0;
//		s_tx_process(0, size);	

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
unsigned char w5100_s1_udp_loop_back(void)
{
#if 0
	unsigned short i,j;
	unsigned char *ptr;
	
		/***** First step: Read data from RX buffer and dump them in S_Buffer ****/
	i=s_rx_process(1);
	
	/* Set Destination IP */
	ptr=(unsigned char*)W5100_S1_DIPR;
	*ptr++=w5100MU.rxBuf[0];
	*ptr++=w5100MU.rxBuf[1];
	*ptr++=w5100MU.rxBuf[2];
	*ptr=w5100MU.rxBuf[3];
	
	/* Set Destination Port Number */
	ptr=(unsigned char*)W5100_S1_DPORT;
	*ptr++=w5100MU.rxBuf[4];
	*ptr=w5100MU.rxBuf[5];
	
	/* Adjust TX data size */
	i-=8;
	
	/* Move RX_Buffer data to TX_Buffer */
	for(j=0;j<i;j++)
		w5100MU.rxBuf[j]=w5100MU.rxBuf[j+8];
	
	/***** Second step: Write S_Buffer data to TX buffer and send out ****/
	/**** Wait for SendOK or TimeOut event occur***/
	ptr=(unsigned char*)W5100_S1_CR;
	while((*ptr)!=0);

//	s_tx_process(1, i);	
#endif
	return 1;
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
void w5100_ir_handler(void)
{
	//u8 i,snINT;
	volatile unsigned char irReg,snIRReg;
	volatile unsigned char *ptr;
	

	//读取中断标识
	ptr=(unsigned char*)W5100_IR;
	irReg=*ptr;

	//回写清中断标志
	*ptr=(irReg&0xf0);

	//IP地址冲突异常处理
	if((irReg&IR_CONFLICT)==IR_CONFLICT)
	{
	//		led_01_on();
	}
	// udp 模式下无法到达，异常处理
	if((irReg&IR_UNREACH)==IR_UNREACH)
	{
	//		led_02_on();
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
mlu8 w5100_reset(Netcardptr pNetcard)
{
	subtask_declaration();//任务声明
	unsigned char* u8ptr;
	
	subtask_begin(pNetcard->pMyDriveTask);//任务开始

	//hw reset
	w5100_reset_pin_l();
	//delya 2ms
	subtask_ms_delay(pNetcard->pMyDriveTask, 20, TASK_STA_NONBLOCKING_YIELD);
	w5100_reset_pin_h();
	//delya 50ms
	subtask_ms_delay(pNetcard->pMyDriveTask, 100, TASK_STA_NONBLOCKING_YIELD);
	//软复位W5100
	u8ptr=(unsigned char*)W5100_MODE;
	*u8ptr=MODE_RST;
	//delya 100ms
	subtask_ms_delay(pNetcard->pMyDriveTask, 100, TASK_STA_NONBLOCKING_YIELD);

	//进入 餐设置
	pNetcard->sta=e_netcard_setting;
	subtask_end(pNetcard->pMyDriveTask, TASK_STA_ENDED);
	
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
mlu8 val[6];
int w5100_setting(Netcardptr pNetcard)
{
	mlvu8* ptr;
	mlu16 i;

	for(i=0;i<6;++i)
	{
		val[i]=0;
	}
	//	设置网关(Gateway)的IP地址，4字节
	//	使用网关可以使通信突破子网的局限，
	// 通过网关可以访问到其它子网或进入Internet
	ptr=(mlu8*)W5100_GAR;
	for(i=0;i<4;++i)
	{
		*ptr=pNetcard->gateway[i];
		ptr++;
	}

	ptr=(mlu8*)W5100_GAR;
	for(i=0;i<4;++i)
	{
		val[i]=*ptr;
		ptr++;
	}

	//设置子网掩码(MASK)值，4字节。子网掩码用于子网运算
	ptr=(mlu8*)W5100_SUBR;
	for(i=0;i<4;++i)
	{
		*ptr=pNetcard->subnetmask[i];
		ptr++;
	}
	ptr=(mlu8*)W5100_SUBR;
	for(i=0;i<4;++i)
	{
		val[i]=*ptr;
		ptr++;
	}
    
	//	设置物理地址，6字节，用于唯一标识网络设备的物理地址值
	//	该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	//	如果自己定义物理地址，注意第一个字节必须为偶数
	ptr=(mlu8*)W5100_SHAR;
	for(i=0;i<6;i++)
	{
		*ptr=pNetcard->mac[i];
		ptr++;
	}
	ptr=(mlu8*)W5100_SHAR;
	for(i=0;i<6;i++)
	{
		val[i]=*ptr;
		ptr++;
	}
	//	设置本机的IP地址，4个字节
	//	注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
	ptr=(mlu8*)W5100_SIPR;
	for(i=0;i<4;i++)
	{
		*ptr=pNetcard->ip[i];
		ptr++;
	}
	ptr=(mlu8*)W5100_SIPR;
	for(i=0;i<4;i++)
	{
		val[i]=*ptr;/*sys.attributeMU.myIP为4字节unsigned char数组,自己定义*/
		ptr++;
	}
	
	//设置发送缓冲区和接收缓冲区的大小，参考W5100数据手册
	//Socket Rx memory size  2k 
	ptr=(mlu8*)W5100_RMSR;
	*ptr=0x55;
	//Socket Tx mempry size  2k
	ptr=(mlu8*)W5100_TMSR;
	*ptr=0x55;

	///* 启动中断，参考W5100数据手册确定自己需要的中断类型
	//IMR_CONFLICT是IP地址冲突异常中断
	//IMR_UNREACH是UDP通信时，地址无法到达的异常中断
	//其它是Socket事件中断，根据需要添加 */
	ptr=(mlu8*)W5100_IMR;
	*ptr=(IMR_CONFLICT|IMR_S0_INT|IMR_S1_INT);
	
	//
	pNetcard->sta=e_netcard_working;
	return (val[0]+val[1]+val[2]+val[3]);
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
int w5100_working(Netcardptr pNetcard)
{
	
	//查询中断，处理中断
	w5100_ir_handler();

	return 0;
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
int w5100_unlinked(Netcardptr pNetcard)
{

	return 1;

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
void w5100_fsm_run(Netcardptr pNetcard)
{
	//
	switch (pNetcard->sta)
	{
		case e_netcard_reset:
			w5100_reset(pNetcard);
			break;
		
		case e_netcard_setting:
			w5100_setting(pNetcard);
			break;

		case e_netcard_working:
			w5100_working(pNetcard);
			break;

		case e_netcard_PHYUnlinked:
			w5100_unlinked(pNetcard);
			break;
	}
	
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	   :  void
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
void w5100_usocket_isr(USockptr pusck)
{
	mlu8 snIRReg;
	unsigned char* ptr;

	//回写清中断标志
	//W5100_S0_IR
	ptr=(unsigned char*)(W5100_S0_IR+pusck->sn*0x100);
	snIRReg=*ptr;
	*ptr=snIRReg;
	
	//在TCP模式下,Socket0成功连接
	if(snIRReg&S_IR_CON)
	{
		pusck->connSta=e_sck_cnn_established;
	}
	
	//在TCP模式下Socke0t断开连接处理，自己添加代码
	if(snIRReg&S_IR_DISCON)
	{	
		//关闭端口，等待重新打开连接
		pusck->connSta=e_sck_cnn_closed;
	
	}
	
	//Socket0数据发送完成，可以再次启动S_tx_process()函数发送数据
	if(snIRReg&S_IR_SENDOK)
	{			
		pusck->txsta=e_sck_txFinished;
	}
	
	//Socket0接收到数据，可以启动S_rx_process()函数
	if(snIRReg&S_IR_RECV)
	{			
		pusck->rxsta=e_sck_rxData;
	
	}
	
	//Socket0连接或数据传输超时处理
	if(snIRReg&S_IR_TIMEOUT)
	{
		//关闭端口，等待重新打开连接
		//ptr=(unsigned char*)W5100_S0_CR;
		//*ptr=S_CR_CLOSE;
		
		pusck->timeOut=1;
		
	}

}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	  :  void
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
int w5100_usocket_open(USockptr pusck)
{
	subtask_declaration();//任务声明
	mlu8 regval,i;
	unsigned char* ptr;

	subtask_begin(pusck->pMyDriveTask);//任务开始

	//设置模式
	if(pusck->mode==S_MR_TCP)
	{
		// 设置Socket为TCP模式
		//为了提高响应速度，将ND/MC位置位 
		ptr=(unsigned char*)(W5100_S0_MR+pusck->sn*0x100);
		*ptr=(S_MR_TCP|S_MR_MC);
	}
	else{
		//设置Socket为UDP模式
		ptr=(unsigned char*)(W5100_S0_MR+pusck->sn*0x100);
		*ptr=S_MR_UDP;
		pusck->haveUDPHead=1;//有udp协议包头
	}
	
	//设置本机source的端口号
	ptr=(unsigned char*)(W5100_S0_PORT+pusck->sn*0x100);
	*ptr= (pusck->myport>>8);
	ptr++;
	*ptr=pusck->myport;

	
	//tcp 需要建立连接
	if(pusck->mode==S_MR_TCP)
	{
		//设置远程主机IP地址，即服务器的IP地址
		ptr=(unsigned char*)(W5100_S0_DIPR+pusck->sn*0x100);
		for(i=0;i<4;i++)
		{
			*ptr=pusck->remoteip[i];
			ptr++;
		}
		//Socket的目的端口号为
		ptr=(unsigned char*)(W5100_S0_DPORT+pusck->sn*0x100);
		*ptr=(pusck->remoteport>>8);
		ptr++;
		*ptr=pusck->remoteport;

	}
	
	//打开Socket
	ptr=(unsigned char*)(W5100_S0_CR+pusck->sn*0x100);
	*ptr=S_CR_OPEN;

	subtask_ms_delay(pusck->pMyDriveTask,50, TASK_STA_NONBLOCKING_YIELD);

	//打开不成功，关闭Socket，然后返回
	ptr=(unsigned char*)(W5100_S0_SSR+pusck->sn*0x100);
	regval=*ptr;
	if((pusck->mode==S_MR_TCP&&regval==S_SSR_INIT)||
		(pusck->mode==S_MR_UDP&&regval==S_SSR_UDP))
	{
	
		//soc打开成功
		pusck->txsta=e_sck_txFinished;
		pusck->timeOut=0;
		pusck->rxsta=e_sck_rxNull;
		pusck->txDataLen=0;
		pusck->rxDataLen=0;
		pusck->connSta=e_sck_cnn_none;
		pusck->rxTimeoutCnt=0;
		if(pusck->mode==S_MR_TCP)
		{
			pusck->sta=e_sck_sta_connecting;
		}
		else
		{
			pusck->sta=e_sck_sta_working;
		}
		
	}
	else
	{
			pusck->sta=e_sck_sta_closed;
	}

	subtask_end(pusck->pMyDriveTask, TASK_STA_ENDED);//任务结束
	
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	  :  void
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
int w5100_usocket_connect(USockptr pusck)
{
	subtask_declaration();//任务声明
	mlu8 regval;
	unsigned char* ptr;


	subtask_begin(pusck->pMyDriveTask);//任务开始

	//设置Socket为Connect模式
	ptr=(unsigned char*)(W5100_S0_CR+pusck->sn*0x100);
	*ptr=S_CR_CONNECT;

	subtask_ms_delay(pusck->pMyDriveTask,200, TASK_STA_NONBLOCKING_YIELD);
	ptr=(unsigned char*)(W5100_S0_SSR+pusck->sn*0x100);
	regval=*ptr;
	if(regval==S_SSR_ESTABLISHED)
	{
		//建立连接,进入工作状态
		pusck->connSta=e_sck_cnn_established;
		pusck->sta=e_sck_sta_working;
	}
	else
	{	 
		//连接超时
			pusck->connSta=e_sck_cnn_failed;
			pusck->sta=e_sck_sta_closed;

	}

	subtask_end(pusck->pMyDriveTask, TASK_STA_ENDED);//任务结束

}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	  :  void
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
int w5100_usocket_close(USockptr pusck)
{
	
	subtask_declaration();//任务声明
	mlu8 regVal;
	unsigned char* ptr;

	subtask_begin(pusck->pMyDriveTask);//任务开始

	//close
	ptr=(unsigned char*)(W5100_S0_SSR+pusck->sn*0x100);
	if(*ptr!=S_SSR_CLOSED)
	{
		//关闭socket s
		ptr=(unsigned char*)(W5100_S0_CR+pusck->sn*0x100);
		*ptr=S_CR_CLOSE;
	}

	subtask_ms_delay(pusck->pMyDriveTask,500, TASK_STA_NONBLOCKING_YIELD);
	ptr=(unsigned char*)(W5100_S0_SSR+pusck->sn*0x100);
	if(*ptr!=S_SSR_CLOSED)
	{
		w5100.sta=e_netcard_reset;//复位网络模块
	}
	else
	{
			//启动重连
			pusck->sta=e_sck_sta_opening;
	}
		
	//清除所有中断标识
	//回写清中断标志
	ptr=(unsigned char*)(W5100_S0_IR+pusck->sn*0x100);
	regVal=*ptr;
	*ptr=regVal;


	//延时2000ms
	subtask_ms_delay(pusck->pMyDriveTask,2000, TASK_STA_NONBLOCKING_YIELD);


	subtask_end(pusck->pMyDriveTask, TASK_STA_ENDED);//任务结束

}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数	  :  void
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
int w5100_usocket_work(USockptr pusck)
{
//	unsigned char *ptr;
	//unsigned char i,reg;
#if 1

	//状态检测
	w5100_usocket_isr(pusck);


	//接收数据
	if (pusck->rxsta==e_sck_rxData&&w5100_socket_rx(pusck))
	{
		pusck->rxsta=e_sck_rxNull;
		pusck->rxTimeoutCnt=0;
		//lbtimer_load_start(pusck->rxtimer,1000);
	}

	//发送数据
	if (pusck->txDataLen>0&&w5100_socket_tx(pusck))
	{
		pusck->txDataLen=0;
	}

	//发送超时
	//if()
	{

	}
	
	//
	if(pusck->connSta==e_sck_cnn_closed||pusck->timeOut==_set)
	{
		pusck->txDataLen=0;
		pusck->txDataLen=0;
		pusck->sta=e_sck_sta_closed;
	}
	return 1;
#endif
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
void w5100_usocket_drive(void* pDriveSck)
{

	USockptr pusck;
	
	pusck=(USockptr)pDriveSck;

	//模块状态机
	w5100_fsm_run(pusck->pMynetcard);


	//检查网络硬件模块是否在工作
	if (pusck->pMynetcard->sta != e_netcard_working)
	{
		return;
	}
	
	//socket状态机
	switch (pusck->sta)
	{
		case e_sck_sta_opening://打开网络套接字
			w5100_usocket_open(pusck);
		break;
		case e_sck_sta_connecting://建立连接
			w5100_usocket_connect(pusck);
		break;
		case e_sck_sta_working://工作
			w5100_usocket_work(pusck);
		break;
		case e_sck_sta_closed://关闭状态
			w5100_usocket_close(pusck);
		break;
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
Netcardptr w5100_network_card(void)
{

	return &w5100;
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
mls8 w5100_ussn_generate(void)
{
	mls8 sn;
	if (w5100.usockedCount>=4)
	{
		sn=-1;	
	}
	sn=w5100.usockedCount;
	w5100.usockedCount++;
	return sn;
	
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
void w5100_init(mlu8* ip,mlu8* mask,mlu8* gateway,mlu8* mac)
{
	mlu8 i;
	
	//初始化 mcu 外设
	w5100_mcu_init();


	//初始化状态复位
	w5100.sta=e_netcard_reset;
	w5100.usocket_drive=w5100_usocket_drive;
	w5100.pMyDriveTask=subtask_create(ltimer_create());					//创建驱动任务
	w5100.usocket_sn_generate=	w5100_ussn_generate;					//创建一个socket，返回socket 序号
	w5100.usockedCount=0;
	
	for (i = 0; i < 4; ++i)
	{
		w5100.ip[i]=ip[i];
		w5100.subnetmask[i]=mask[i];
		w5100.gateway[i]=gateway[i];
		w5100.mac[i]=mac[i];
	}
	w5100.mac[i]=mac[i];
	++i;
	w5100.mac[i]=mac[i];


	
}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif

