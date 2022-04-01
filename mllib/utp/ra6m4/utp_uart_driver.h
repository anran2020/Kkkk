/**
 ******************************************************************************
 * 文件:  utp_uart_driver.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    
 * 内容简介:     
 *                   
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                               创建该文件
 *
 *
 *
 ******************************************************************************
 **/
#ifndef  _UTP_UART_H__
#define _UTP_UART_H__

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include"mlos.h"
#include"utp_def.h"


//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------



#define  UART_BAUDRATE   _115200Bps_






//-------------------------------------- -------------------------------------
//export   fucation 
//----------------------------------------------------------------------------

void uart_init(void);


mlu16 uart0_tx(mlu8*pdat,mlu16 len);
mlu16 uart1_tx(mlu8*pdat,mlu16 len);

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

#endif
