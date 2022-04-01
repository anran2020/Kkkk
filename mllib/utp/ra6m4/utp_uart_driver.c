/**
 ******************************************************************************
 * 文件:    
 * 作者:   
 * 版本:   V0.01
 * 日期:    
 * 内容简介:    
 *          
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                   创建该文件
 *
 *
 *
 ******************************************************************************
 **/
 
//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "utp_uart_driver.h"
#include "r_sci_uart.h"
#include "utp.h"
#include "public.h"

//----------------------------------------------------------------------------
//   define
//----------------------------------------------------------------------------

//发送使能

// uart0-channel 7
//P611	SCI0_CTS	串行通讯0使能
//P612		
//P613	SCI0_TXD	串行通讯0TXD
//P614	SCI0_RXD	串行通讯0RXD

#define uart0_tx_enable() 		R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_06_PIN_11,BSP_IO_LEVEL_HIGH)
#define uart0_tx_disable() 		R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_06_PIN_11,BSP_IO_LEVEL_LOW)

// uart1-channel 7
//P711	SCI1_CTS	串行通讯1使能
//P710		
//P709	SCI1_TXD	串行通讯1TXD
//P708	SCI1_RXD	串行通讯1RXD
#define uart1_tx_enable() 		R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_07_PIN_11,BSP_IO_LEVEL_HIGH)
#define uart1_tx_disable() 		R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_07_PIN_11,BSP_IO_LEVEL_LOW)


//----------------------------------------------------------------------------
//   global variable 
//----------------------------------------------------------------------------


mlu16 uart0_tx(mlu8*pdat,mlu16 len)
{
	uint32_t i;

	if(len > 256 || len == 0)
	{
		return len;
	}

	delay_rough_us(200);  /*wangzg*/
	uart0_tx_enable();
	//delay_rough_ms(1);
	delay_rough_us(100);  /*wangzg*/
	utp.Uartcard[0].txover=1;
	R_SCI_UART_Write(&g_uart0_ctrl,pdat,len);

	return len;	

}

mlu16 uart1_tx(mlu8*pdat,mlu16 len)
{
	if(len > 256 || len == 0)
	{
		return len;
	}

	delay_rough_us(200);  /*wangzg*/
	uart1_tx_enable();
	//delay_rough_ms(1);
	delay_rough_us(100);  /*wangzg*/
	utp.Uartcard[1].txover=1;
	R_SCI_UART_Write(&g_uart1_ctrl,pdat,len);

	return len;

}

//-----------------------------------------------------------------------------
// 函数名：uart_callback
//-----------------------------------------------------------------------------
// 返回值 : void
// 参数   :  p_args:对应uart得状态信息
//功能描述：uart中断
//-----------------------------------------------------------------------------
void uart_callback(uart_callback_args_t *p_args)
{
    if(p_args->event & (0x01 << 2) ||  p_args->event & (0x01 << 0))
    {
		if(p_args->channel == 7)//uart 0
		{
			if(utp.Uartcard[0].mb_sta == mb_wait_rx)
			{
				utp.Uartcard[0].rxindex = 0;
				utp.Uartcard[0].mb_sta = mb_rxing;
			}

			if(utp.Uartcard[0].mb_sta == mb_rxing)
			{
				utp.Uartcard[0].Tstamp =GetOSRunTime();
				utp.Uartcard[0].RxBuffer[utp.Uartcard[0].rxindex++] = p_args->data;
				if(utp.Uartcard[0].rxindex >= 256)
				{
					utp.Uartcard[0].rxindex = 0;
				}
			}

		}

		else if(p_args->channel == 1)//uart 1
		{

			if(utp.Uartcard[1].mb_sta == mb_wait_rx)
			{
				utp.Uartcard[1].rxindex = 0;
				utp.Uartcard[1].mb_sta = mb_rxing;
			}

			if(utp.Uartcard[1].mb_sta == mb_rxing)
			{
				utp.Uartcard[1].Tstamp =GetOSRunTime();
				utp.Uartcard[1].RxBuffer[utp.Uartcard[1].rxindex++] = p_args->data;
				if(utp.Uartcard[1].rxindex >= 256)
				{
					utp.Uartcard[1].rxindex = 0;
				}
			}
		}
    }
	else if(UART_EVENT_TX_DATA_EMPTY == p_args->event)
	{


	}
	else if(UART_EVENT_TX_COMPLETE == p_args->event)
	{
		if(p_args->channel == 7)//uart 0
		{
			utp.Uartcard[0].txover = 0;
            delay_rough_us(100);  /*wangzg*/
			uart0_tx_disable();
			
		}
		else if(p_args->channel == 1)//uart 1
		{
			utp.Uartcard[1].txover = 0;
            delay_rough_us(100);  /*wangzg*/
			uart1_tx_disable();
		}
	}
	else
	{

	}
}

//-----------------------------------------------------------------------------
// 函数名：uart_init
// 功能：uart硬件初始化
//-----------------------------------------------------------------------------
void uart_init(void)
{
	R_SCI_UART_Open(&g_uart0_ctrl,&g_uart0_cfg);

	R_SCI_UART_Open(&g_uart1_ctrl,&g_uart1_cfg);

	uart0_tx_disable();

	uart1_tx_disable();

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------


