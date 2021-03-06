/**
 ******************************************************************************
 * 文件:
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1         2014-12-01     zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>


#include "ntp_w5500.h"
#include "mlos_ltimer.h"
#include "mlos_task.h"

#include "ntp_usocket.h"

#include "hal_data.h"
#include "r_spi.h"
#include"common_data.h"
#include "ntp_def.h"
#include "ntp_w5500_spi_driver.h"

#if 1


//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------


//op mode
#define _W5500_SPI_VDM_OP_          0x00
#define _W5500_SPI_FDM_OP_LEN1_     0x01
#define _W5500_SPI_FDM_OP_LEN2_     0x02
#define _W5500_SPI_FDM_OP_LEN4_     0x03

//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------
NetworkCard w5500;


mlu32 w5500spievent;

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
Netcardptr w5500_network_card(void)
{
	return &w5500;
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
void w5500_interrupt_handler(USockptr pusck)
{

    mlu8 reg;

    //读取中断标识
    reg = getSn_IR(pusck->sn);

    //回写清中断标志
    setSn_IR(pusck->sn, reg);
    //getSn_IR(pusck->sn);

    //当成功与对方建立连接，且 Sn_SR 变为 SOCK_ESTABLISHED 状态时，此位生效。
    if ((reg & Sn_IR_CON) == Sn_IR_CON)
    {
        pusck->connSta = e_sck_cnn_established;
    }
    // 当接收到对方的 FIN or FIN/ACK 包时，此位生效
    if ((reg & Sn_IR_DISCON) == Sn_IR_DISCON)
    {
        pusck->connSta = e_sck_cnn_failed;

    }

    //无论何时，只要接收到了对方数据，此位生效。
    if ((reg & Sn_IR_RECV) == Sn_IR_RECV)
    {
        pusck->rxsta = e_sck_rxData;
    }

    //当 ARPTO或者 TCPTO 超时被触发时，此位生效。
    if ((reg & Sn_IR_TIMEOUT) == Sn_IR_TIMEOUT)
    {
        pusck->timeOut = _yes;
        pusck->txsta = e_sck_txFinished;
    }
    //当 SEND 命令完成时，此位生效。
    if ((reg & Sn_IR_SENDOK) == Sn_IR_SENDOK)
    {
        pusck->txsta = e_sck_txFinished;
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
mlu16 w5500_rx_rsr(mlu8 sn)
{
    mlu16 val = 0, val1 = 0;
    do
    {
        val1 = w5500_r_byte (Sn_RX_RSR(sn));
        val1 = (val1 << 8) + w5500_r_byte (_W5500_OFFSET_INC(Sn_RX_RSR(sn), 1));
        if (val1 != 0)
        {
            val = w5500_r_byte (Sn_RX_RSR(sn));
            val = (val << 8) + w5500_r_byte (_W5500_OFFSET_INC(Sn_RX_RSR(sn), 1));
        }

    }
    while (val != val1);

    return val;
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
mlu16 w5500_tx_fsr(mlu8 sn)
{
    mlu16 val = 0, val1 = 0;
    do
    {
        val1 = w5500_r_byte (Sn_TX_FSR(sn));
        val1 = (val1 << 8) + w5500_r_byte (_W5500_OFFSET_INC(Sn_TX_FSR(sn), 1));
        if (val1 != 0)
        {
            val = w5500_r_byte (Sn_TX_FSR(sn));
            val = (val << 8) + w5500_r_byte (_W5500_OFFSET_INC(Sn_TX_FSR(sn), 1));
        }
    }
    while (val != val1);
    return val;
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
unsigned int w5500_socket_rx(USockptr pusck)
{
    mlu8 u8buf[8];
    mlu16 rxSize, rxOffset, rxAddr, rxNum1, rxNum2;
    mlu32 reg, addrsel, rxMax;

    //读取接收数据的字节数
    //w5500_rw_buf(Sn_RX_RSR(pusck->sn),(mlu8*)&rxSize,2,1);
    rxSize = w5500_rx_rsr (pusck->sn);
    //,缓存不够直接返回
    if (rxSize == 0)
    {
        return 0;
    }

#if 1
    rxAddr = getSn_RX_RD(pusck->sn);
    addrsel = ((mlu32) rxAddr << 8) + (_W5500_RXBUF_BLOCK(pusck->sn) << 3);
    w5500_rw_buf (addrsel, (pusck->pRxBuf + pusck->rxDataLen), rxSize, 1);

    rxAddr += rxSize;
    setSn_RX_RD(pusck->sn, rxAddr);
    mlos_us_delay (500);
    pusck->rxBaseAddr = getSn_RX_RD(pusck->sn);
    setSn_CR(pusck->sn, Sn_CR_RECV);
    mlos_us_delay (500);
    reg = getSn_CR(pusck->sn);
    pusck->rxDataLen += rxSize;
#else
	rxOffset = getSn_RX_RD(pusck->sn);
	rxAddr=(rxOffset&(USOCK_RX_SIZE-1));
	
	
	if((rxAddr+rxSize)<USOCK_RX_SIZE)
	{
		rxNum1=rxSize;
		rxNum2=0;
	}
	else
	{
		rxNum1=USOCK_RX_SIZE-rxAddr;
		rxNum2=rxSize-rxNum1;
	}
	
	//
	if(rxSize>(USOCK_RX_SIZE-pusck->rxDataLen))
	{
		return 0;
	}
	
	addrsel = (rxAddr << 8) + (_W5500_RXBUF_BLOCK(pusck->sn) << 3);
	w5500_rw_buf(addrsel, (pusck->rxBuf+pusck->rxDataLen), rxNum1,1);
	pusck->rxDataLen+=rxNum1;
	if(rxNum2)
	{
		rxAddr=0;
		addrsel = (rxAddr << 8) + (_W5500_RXBUF_BLOCK(pusck->sn) << 3);
		w5500_rw_buf(addrsel, (pusck->rxBuf+pusck->rxDataLen), rxNum2,1);
		pusck->rxDataLen+=rxNum2;
	}
		

	
	rxOffset += rxSize;
	setSn_RX_RD(pusck->sn,rxOffset);
    setSn_RX_RD(pusck->sn,rxOffset);
	pusck->rxBaseAddr=getSn_RX_RD(pusck->sn);

	
	setSn_CR(pusck->sn,Sn_CR_RECV);
	reg=getSn_CR(pusck->sn);
#endif
    //返回接收数据的字节数
    return rxSize;

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
mlu16 w5500_socket_tx(USockptr pusck)
{

    mlu32 txFreeSize, addrsel, reg;

    //读取缓冲区剩余的长度
    //如果剩余的字节长度小于发送字节长度,则返回
    txFreeSize = w5500_tx_fsr (pusck->sn);

    if (txFreeSize < pusck->txDataLen || pusck->txsta != e_sck_txFinished)
    {
        return 0;
    }

    reg = getSn_IR(pusck->sn);
    if (reg & Sn_IR_SENDOK)
    {
        setSn_IR(pusck->sn, Sn_IR_SENDOK);
    }
    else if (reg & Sn_IR_TIMEOUT)
    {
        setSn_IR(pusck->sn, Sn_IR_TIMEOUT);
    }
    else
    {
        reg = 0;
    }

    //如果是UDP模式,可以在此设置目的主机的IP和端口号
    if (pusck->mode == Sn_MR_UDP)
    {
        //设置远程主机IP地址，即服务器的IP地址
        setSn_DIPR(pusck->sn, pusck->remoteip);

        //Socket的目的端口号
        setSn_DPORT(pusck->sn, pusck->remoteport);
    }

    reg = getSn_TX_WR(pusck->sn);
    addrsel = (reg << 8) + (_W5500_TXBUF_BLOCK(pusck->sn) << 3);
    //
    w5500_rw_buf (addrsel, pusck->pTxBuf, pusck->txDataLen, 0);

    reg += pusck->txDataLen;
    setSn_TX_WR(pusck->sn, reg);

    setSn_CR(pusck->sn, Sn_CR_SEND);

    return pusck->txDataLen;

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

mlu8 w5500_usocket_open(USockptr pusck)
{
    subtask_declaration();	//任务声明
    mlu16 regval=0;
	mlu8 err=0;

    subtask_begin(pusck->pMyDriveTask);	//任务开始

    //设置模式
    if (pusck->mode == Sn_MR_TCP)
    {
        setSn_MR(pusck->sn, (pusck->mode|Sn_MR_MC));

    }
    else
    {
        setSn_MR(pusck->sn, pusck->mode);
    }
	
    regval=getSn_MR(pusck->sn);
    err=regval;

    //端口号
    setSn_PORT(pusck->sn, pusck->myport);
    regval=getSn_PORT(pusck->sn);
    err=regval;
	if (regval!=pusck->myport)
	{
		err=1;
	}
		
    if (pusck->mode == Sn_MR_TCP)
    {
        setSn_KPALVTR(pusck->sn, 5);
        regval = getSn_KPALVTR(pusck->sn);
    }

    //打开Socket
    setSn_CR(pusck->sn, Sn_CR_OPEN);
    regval = getSn_CR(pusck->sn);
    err=regval;
    subtask_ms_delay(pusck->pMyDriveTask, 10, TASK_STA_NONBLOCKING_YIELD);

    //判断打开是否成功
    regval = getSn_SR(pusck->sn);
    if ((pusck->mode == Sn_MR_TCP && regval == SOCK_INIT) || (pusck->mode == Sn_MR_UDP && regval == SOCK_UDP))
    {

        //soc打开成功
        pusck->txsta = e_sck_txFinished;
        pusck->timeOut = 0;
        pusck->rxsta = e_sck_rxNull;
        pusck->txDataLen = 0;
        pusck->rxDataLen = 0;
        pusck->connSta = e_sck_cnn_none;
        pusck->rxTimeoutCnt = 0;
        if (pusck->mode == Sn_MR_TCP)
        {
            //设置Socket为Connect模式
            //setSn_CR(pusck->sn, Sn_CR_CONNECT);
            pusck->sta = e_sck_sta_connecting;
            //lbtimer_load_start(pusck->rxtimer,250);

        }
        else
        {
            //lbtimer_stop(pusck->rxtimer);
            pusck->sta = e_sck_sta_working;
            //lbtimer_load_start(usock[USOCKT_DEBUG].rxtimer,5000);
        }

        //获取接收地址
        pusck->rxBaseAddr = getSn_RX_RD(pusck->sn);
    }
    else
    {
        pusck->sta = e_sck_sta_closed;
    }

	subtask_end(pusck->pMyDriveTask, regval);			//任务结束
#if 0

//打开不成功，关闭Socket，然后返回
regval=getSn_SR(pusck->sn);
if((pusck->mode==Sn_MR_TCP&&regval==SOCK_INIT)||
        (pusck->mode==Sn_MR_UDP&&regval==SOCK_UDP))
{

    //soc打开成功
    pusck->txsta=e_sck_txFinished;
    pusck->timeOut=0;
    pusck->rxsta=e_sck_rxNone;
    pusck->txDataLen=0;
    pusck->rxDataLen=0;
    pusck->connSta=e_sck_cnnNone;
    pusck->rxTimeoutCnt=0;
    if(pusck->mode==Sn_MR_TCP)
    {
        //设置Socket为Connect模式
        setSn_CR(pusck->sn, Sn_CR_CONNECT);
        pusck->sta=e_sck_sta_connecting;
        lbtimer_load_start(pusck->rxtimer,250);

    }
    else
    {
        lbtimer_stop(pusck->rxtimer);
        pusck->sta=e_sck_sta_working;
    }

}
else
{
    if (lbtimer_timeout(pusck->rxtimer))
    {
        //打开超时，关闭s，进入关闭状态，启动2s，tcp连接upper
        setSn_CR(pusck->sn, Sn_CR_CLOSE);
        pusck->sta=e_sck_sta_closed;
        //2s延时后，再次建立连接
        lbtimer_load_start(pusck->rxtimer,2000);
    }
}
#endif
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
mlu8 w5500_usocket_connect(USockptr pusck)
{

    subtask_declaration();			//任务声明
	mlu8 i,regval[4];

    //任务开始之前需要执行的
    w5500_interrupt_handler (pusck);
    regval[0] = getSn_SR(pusck->sn);
    if (regval[0] == SOCK_ESTABLISHED)
    {
        pusck->connSta = e_sck_cnn_established;
    }


    subtask_begin(pusck->pMyDriveTask);			//任务开始

    //设置远程主机IP地址，即服务器的IP地址
    //mlu32 addr=((0x000C << 8) + (0x01 << 3));
    //w5500_rw_buf(addr, pusck->remoteip,4,0);
    setSn_DIPR(pusck->sn, pusck->remoteip);
   // w5500_rw_buf(addr, regval,4,1);
	for (i = 0; i < 4; ++i)
	{
		if (regval[i]!=pusck->remoteip[i])
		{
			regval[i]=0xff;
		}
	}

    //Socket的目的端口号
    setSn_DPORT(pusck->sn, pusck->remoteport);
   // regval[0] = getSn_DPORT(pusck->sn);
  //  if (regval[0] != pusck->remoteport)
    {
    //    pusck->sta = e_sck_sta_closed;
       // pusck->pMynetcard->sta = e_netcard_reset;			//复位模块
    //    subtask_exit(pusck->pMyDriveTask, TASK_STA_EXITED);
    }

    //设置Socket为Connect模式
    setSn_CR(pusck->sn, Sn_CR_CONNECT);

    //等待建立连接
    subtask_wait_cnd_until(pusck->pMyDriveTask, (pusck->connSta==e_sck_cnn_established), 1000, TASK_STA_NONBLOCKING_YIELD);
    if(pusck->connSta!=e_sck_cnn_established)
    {
        //
        pusck->sta=e_sck_sta_closed;
        subtask_exit(pusck->pMyDriveTask, TASK_STA_EXITED);
    }

    //建立连接,进入工作状态
    ltimer_stop(pusck->pRxtimer);
    pusck->sta=e_sck_sta_working;

    subtask_end(pusck->pMyDriveTask, TASK_STA_EXITED);			//任务结束

#if 0

    if(pusck->connSta==e_sck_cnn_established)
    {
        //建立连接,进入工作状态
        lbtimer_stop(pusck->rxtimer);
        pusck->sta=e_sck_sta_working;
    }
    else
    {
        //连接超时
        if(lbtimer_timeout(pusck->rxtimer)||
                pusck->timeOut==_yes||pusck->connSta==e_sck_cnn_failed)
        {
            //关闭socket s
            setSn_CR(pusck->sn, Sn_CR_CLOSE);
            lbtimer_load_start(pusck->rxtimer,2000);
            pusck->sta=e_sck_sta_closed;
        }
    }
#endif
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
int w5500_usocket_close(USockptr pusck)
{
#if 1
    subtask_declaration();			//任务声明
    mlu8 regval[4],i;
    mlu16 port;

    subtask_begin(pusck->pMyDriveTask);			//任务开始

    //close
    //regval[0] = getSn_SR(pusck->sn);
    //if (regval[0] != SOCK_CLOSED)
    {
        setSn_CR(pusck->sn, Sn_CR_CLOSE);
        //regval[0] = getSn_CR(pusck->sn);
    }
    subtask_ms_delay(pusck->pMyDriveTask, 500, TASK_STA_NONBLOCKING_YIELD);
    regval[0] = getSn_SR(pusck->sn);
    if (regval[0] != SOCK_CLOSED)
    {
    	pusck->pMynetcard->sta = e_netcard_reset;			//复位网络模块
    }

    //清除所有中断标识
    setSn_IR(pusck->sn, 0xFF);
    regval[0] = getSn_IR(pusck->sn);

    //延时2000ms
    subtask_ms_delay(pusck->pMyDriveTask, 2000, TASK_STA_NONBLOCKING_YIELD);

    //启动重连
    pusck->sta = e_sck_sta_opening;

	subtask_end(pusck->pMyDriveTask, TASK_STA_EXITED);			//任务结束

#endif
#if 0
setSn_CR(pusck->sn,Sn_CR_CLOSE);
/* wait to process the command... */
while( getSn_CR(pusck->sn) );
/* clear all interrupt of the socket. */
setSn_IR(pusck->sn, 0xFF);

while(getSn_SR(pusck->sn) != SOCK_CLOSED);
pusck->sta=e_sck_staSetting;

#endif

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
int w5500_usocket_work(USockptr pusck)
{

#if 1
    subtask_declaration();			//任务声明
    unsigned char regVal;

    //发送数据
    if (pusck->txDataLen > 0 && w5500_socket_tx (pusck))
    {
        pusck->txDataLen = 0;
    }

    subtask_begin(pusck->pMyDriveTask);			//任务开始

    while (1)
    {
        //延时5ms 查询一次
        subtask_ms_delay(pusck->pMyDriveTask, 5, TASK_STA_NONBLOCKING_YIELD);

        //状态检测
        /*
         regVal=getSn_SR(pusck->sn);
         if(SOCK_UDP!=regVal&&SOCK_ESTABLISHED!=regVal)
         {
         //断开连接
         pusck->sta=e_sck_sta_closed;
         return TASK_STA_EXITED;
         }
         */
        //中断查询
        w5500_interrupt_handler (pusck);

        //接收数据
        if (w5500_socket_rx (pusck))
        {
            pusck->rxsta = e_sck_rxNull;
            pusck->rxTimeoutCnt = 0;
            //lbtimer_load_start(pusck->rxtimer,1000);
        }

        //
        if (pusck->connSta == e_sck_cnn_closed || pusck->timeOut == _set)
        {
            pusck->txDataLen = 0;
            pusck->txDataLen = 0;
            pusck->sta = e_sck_sta_closed;
        }

    }

	subtask_end(pusck->pMyDriveTask, TASK_STA_EXITED);			//任务结束

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
int w5500_reset(Netcardptr pNetcard)
{
	subtask_declaration();
	mlu8 regVal;

	subtask_begin(pNetcard->pMyDriveTask);

	//硬件复位
	w5500_hdw_reset_enable();

	subtask_ms_delay(pNetcard->pMyDriveTask, 100, TASK_STA_NONBLOCKING_YIELD);
	
	w5500_hdw_reset_disable();

	subtask_ms_delay(pNetcard->pMyDriveTask, 1000, TASK_STA_NONBLOCKING_YIELD);

	//sw reset
	setMR(MR_RST);
	
	subtask_ms_delay(pNetcard->pMyDriveTask, 500, TASK_STA_NONBLOCKING_YIELD);

	regVal = getMR();
	pNetcard->sta = e_netcard_setting;
	subtask_end(pNetcard->pMyDriveTask, regVal);

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
int w5500_setting(Netcardptr pNetcard)
{
	subtask_declaration();//任务声明
	mlu8 regVal[6];
    mlu8 i;
    mlu16 port;

    subtask_begin(pNetcard->pMyDriveTask);//任务开始
    

    //  设置物理地址，6字节，用于唯一标识网络设备的物理地址值
    //  该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
    //  如果自己定义物理地址，注意第一个字节必须为偶数
    setSHAR(w5500.mac);
    getSHAR(regVal);
	for (i = 0; i < 6; ++i)
	{
		if (w5500.mac[i]!=regVal[i])
		{
			regVal[i]=0xff;
		}
	}

    //
    //  设置网关(Gateway)的IP地址，4字节
    //  使用网关可以使通信突破子网的局限，
    // 通过网关可以访问到其它子网或进入Internet
    setGAR(w5500.gateway);
    getGAR(regVal);
	for (i = 0; i < 4; ++i)
	{
		if (w5500.gateway[i]!=regVal[i])
		{
			regVal[i]=0xff;
		}
	}

    //设置子网掩码(MASK)值，4字节。子网掩码用于子网运算
    setSUBR(w5500.subnetmask);
    getSUBR(regVal);
	for (i = 0; i < 4; ++i)
	{
		if (w5500.subnetmask[i]!=regVal[i])
		{
			regVal[i]=0xff;
		}
	}

    //  设置本机的IP地址，4个字节
    //  注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
    setSIPR(w5500.ip);
    getSIPR(regVal);
	for (i = 0; i < 4; ++i)
	{
		if (w5500.ip[i]!=regVal[i])
		{
			regVal[i]=0xff;
		}
	}


    //设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册
    //Socket Rx memory size  2k 
    for (i = 0; i < 8; i++)
    {
        //setSn_TXBUF_SIZE(i, 0x02);
        //setSn_RXBUF_SIZE(i, 0x02);
    }

    //
   // setRTR(0x07D0);	// 0x07D0 200ms
   // setRCR(0x08);

    //

    //复位phy
    //w5500_Write_1Byte(PHYCFGR, 0X58);
  //  setPHYCFGR(0X58);	//100M全双工，重启物理连接 0x60
    
   	//延时
//	subtask_ms_delay(pNetcard->pMyDriveTask, 10, TASK_STA_NONBLOCKING_YIELD);
	//
//	setPHYCFGR(0XD8);	//0XD8
    regVal[0]=getPHYCFGR();
	pNetcard->sta = e_netcard_PHYUnlinked;
   
   subtask_end(pNetcard->pMyDriveTask, TASK_STA_ENDED);//任务结束

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
int w5500_working(Netcardptr pNetcard)
{

    subtask_declaration();//任务声明

    subtask_begin(pNetcard->pMyDriveTask);//任务开始

    while (1)
    {
        subtask_ms_delay(pNetcard->pMyDriveTask, 1000, TASK_STA_NONBLOCKING_YIELD);
        pNetcard->PHYLinked = getPHYCFGR();
        if (!(pNetcard->PHYLinked & 0x01))
        {
            //断开物理链接，关闭socket
           pNetcard->sta = e_netcard_PHYUnlinked;
            break;
        }

    }

	subtask_end(pNetcard->pMyDriveTask, TASK_STA_EXITED);//任务结束

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
int w5500_unlinked(Netcardptr pNetcard)
{

    subtask_declaration();

    subtask_begin(pNetcard->pMyDriveTask);

    while (1)
    {
        subtask_ms_delay(pNetcard->pMyDriveTask, 1000, TASK_STA_NONBLOCKING_YIELD);
        pNetcard->PHYLinked = getPHYCFGR();
        if (pNetcard->PHYLinked & 0x01)
        {
           pNetcard->sta = e_netcard_working;
            break;
        }

    }

subtask_end(pNetcard->pMyDriveTask, TASK_STA_EXITED);

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
void w5500_fsm_run(Netcardptr pNetcard)
{
    //
    switch (pNetcard->sta)
    {
        case e_netcard_reset:
            w5500_reset (pNetcard);
        break;

        case e_netcard_setting:
            w5500_setting (pNetcard);
        break;

        case e_netcard_working:
            w5500_working (pNetcard);
        break;

        case e_netcard_PHYUnlinked:
            w5500_unlinked (pNetcard);
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
void w5500_usocket_drive(void* pDriveSck)
{
	USockptr pusck;

	pusck=(USockptr)pDriveSck;
	
    //模块状态机驱动
    w5500_fsm_run (pusck->pMynetcard);

    //检查网络硬件模块是否在工作
    if (w5500.sta != e_netcard_working)
    {
       // pusck->sta = e_sck_sta_closed;
        return;
    }

    //中断查询
    //w5500_interrupt_handler(pusck);

    //tcp socket 优先状态机
    switch (pusck->sta)
    {
        case e_sck_sta_opening:	//打开网络套接字
            w5500_usocket_open (pusck);
        break;
        case e_sck_sta_connecting:	//建立连接
            w5500_usocket_connect (pusck);
        break;
        case e_sck_sta_working:	//工作
            w5500_usocket_work (pusck);
        break;
        case e_sck_sta_closed:	//关闭状态
            w5500_usocket_close (pusck);
        break;

    }

}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : 
//mls8 ，返回-1，创建失败
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
mls8 w5500_ussn_generate(void)
{
	mls8 sn;
	if (w5500.usockedCount>=4)
	{
		return -1;	
	}

	sn=w5500.usockedCount;
	w5500.usockedCount++;
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
void w5500_init(mlu8 *ip, mlu8 *mask, mlu8 *gateway, mlu8 *mac)
{
    mlu8 i;
	
    w5500_spi_init ();

    //
    w5500.sta = e_netcard_reset;
	w5500.usocket_drive=w5500_usocket_drive;
    w5500.pMyDriveTask = subtask_create (ltimer_create());
    w5500.usocket_sn_generate=	w5500_ussn_generate;					//创建一个socket，返回socket 序号
    w5500.usockedCount=0;

    for (i = 0; i < 4; ++i)
    {
        w5500.ip[i] = ip[i];
        w5500.subnetmask[i] = mask[i];
        w5500.gateway[i] = gateway[i];
        w5500.mac[i] = mac[i];
    }
    w5500.mac[i] = mac[i];
    ++i;
    w5500.mac[i] = mac[i];

	//
	

}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif

