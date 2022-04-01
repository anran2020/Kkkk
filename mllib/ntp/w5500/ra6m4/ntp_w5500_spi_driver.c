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

//----------------------------------------------------------------------------
//文件条件编译
//----------------------------------------------------------------------------

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


mlu32 w5500spievent;


extern void  r_spi_bit_width_config(spi_instance_ctrl_t * p_ctrl);


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
void w5500_spi_select(void)
{

	R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_07_PIN_03,BSP_IO_LEVEL_LOW);
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
void w5500_spi_deselect(void)
{

	R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_07_PIN_03,BSP_IO_LEVEL_HIGH);
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
void w5500_hdw_reset_enable(void)
{

	R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_00_PIN_15,BSP_IO_LEVEL_LOW);
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
void w5500_hdw_reset_disable(void)
{
	R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_00_PIN_15,BSP_IO_LEVEL_HIGH);
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
void w5500_spi_callback(spi_callback_args_t *p_args)
{
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
    {
        w5500spievent = SPI_EVENT_TRANSFER_COMPLETE;
    }
    else
    {
        w5500spievent = SPI_EVENT_TRANSFER_ABORTED;
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
mlu8 w5500_r_byte(mlu32 addrSel)
{

    mlu8 ret;
#if 1

    w5500_spi_select();
    mlos_us_delay (1);
    //addrSel =(addrSel |(_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_));

    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_w5500spi_ctrl.p_regs->SPDR_BY = ((addrSel & 0x00FF0000) >> 16); //发送ADDR
    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
    ret = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收

    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_w5500spi_ctrl.p_regs->SPDR_BY = ((addrSel & 0x0000FF00) >> 8); //发送ADDR
    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
    ret = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收

    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_w5500spi_ctrl.p_regs->SPDR_BY = (addrSel & 0x000000FF); //发送control
   while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
   ret = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收

    //r
    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_w5500spi_ctrl.p_regs->SPDR_BY = 0; //发送data
    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
	
    ret = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收
    mlos_us_delay (1);
    w5500_spi_deselect();
    mlos_us_delay (1);
#endif
    return ret;
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
mlu8 w5500_w_byte(mlu32 addrSel, mlu8 wb)
{
    mlu8 ret;
#if 1

	w5500_spi_select();
	mlos_us_delay(1);
	addrSel = (addrSel |_W5500_SPI_WRITE_ );
	
	while(g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF==_reset);
	g_w5500spi_ctrl.p_regs->SPDR_BY=((addrSel & 0x00FF0000) >> 16);//发送ADDR
	while(g_w5500spi_ctrl.p_regs->SPSR_b.SPRF==_reset);
	ret=g_w5500spi_ctrl.p_regs->SPDR_BY;//接收
	
	while(g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF==_reset);
	g_w5500spi_ctrl.p_regs->SPDR_BY=((addrSel & 0x0000FF00) >>  8);//发送ADDR
	while(g_w5500spi_ctrl.p_regs->SPSR_b.SPRF==_reset);
	ret=g_w5500spi_ctrl.p_regs->SPDR_BY;//接收
	
	while(g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF==_reset);
	g_w5500spi_ctrl.p_regs->SPDR_BY=(addrSel & 0x000000FF);//发送control
	while(g_w5500spi_ctrl.p_regs->SPSR_b.SPRF==_reset);
	ret=g_w5500spi_ctrl.p_regs->SPDR_BY;//接收

	//w
	while(g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF==_reset);
	g_w5500spi_ctrl.p_regs->SPDR_BY=wb;//发送data
	while(g_w5500spi_ctrl.p_regs->SPSR_b.SPRF==_reset);
	ret=g_w5500spi_ctrl.p_regs->SPDR_BY;//接收
	mlos_us_delay(1);
	w5500_spi_deselect();
	mlos_us_delay(1);
#endif
    return ret;
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
void w5500_rw_buf(mlu32 addrSel, mlu8 *buf, mlu16 len, mlu8 rw)
{
    mlu16 i, temp;
    //
    if (buf == 0 || len == 0)
    {
        return;
    }
    w5500_spi_select();
    mlos_us_delay (1);
#if 1

    //
    if (rw == 0)
        addrSel = (addrSel | _W5500_SPI_WRITE_);

    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_w5500spi_ctrl.p_regs->SPDR_BY = ((addrSel & 0x00FF0000) >> 16); //发送ADDR
    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
    i = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收

    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_w5500spi_ctrl.p_regs->SPDR_BY = ((addrSel & 0x0000FF00) >> 8); //发送ADDR
    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
    i = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收

    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_w5500spi_ctrl.p_regs->SPDR_BY = (addrSel & 0x000000FF); //发送control
    while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
    i = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收

    for (i = 0; i < len; i++)
    {
        while (g_w5500spi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
        
        if (rw)
        {
       		g_w5500spi_ctrl.p_regs->SPDR_BY = 0x55; //读
         	while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
            buf[i] = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收
        }
        else
        {
        	g_w5500spi_ctrl.p_regs->SPDR_BY = buf[i]; //写
        	while (g_w5500spi_ctrl.p_regs->SPSR_b.SPRF == _reset);
            temp = g_w5500spi_ctrl.p_regs->SPDR_BY; //接收
        }
    }
    mlos_us_delay (1);
    w5500_spi_deselect();
    mlos_us_delay (1);
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
void w5500_spi_init(void)
{
    fsp_err_t err = FSP_SUCCESS;     // Error status

    //w5500 支持 0 3 模式
    //spi.SPI_CPOL = SPI_CPOL_High;//SPI_CPOL_Low;
    //spi.SPI_CPHA = SPI_CPHA_2Edge;//SPI_CPHA_1Edge;

    err=R_SPI_Open (&g_w5500spi_ctrl, &g_w5500spi_cfg);
    if(err == FSP_SUCCESS)
    {
    	g_w5500spi_ctrl.p_regs->SPCR_b.SPE = 1;
    }

	g_w5500spi_ctrl.bit_width=SPI_BIT_WIDTH_8_BITS;
	g_w5500spi_ctrl.p_regs->SPDCR_b.SPBYT=1;
	r_spi_bit_width_config(&g_w5500spi_ctrl);

	//g_w5500spi_ctrl.p_regs->SPPCR_b.SPLP=1;//回环测试
	g_w5500spi_ctrl.p_regs->SPCR_b.SPTIE=0;
	g_w5500spi_ctrl.p_regs->SPCR_b.SPRIE=0;

	//0: Select data sampling on leading edge, data change on trailing edge
	//1: Select data change on leading edge, data sampling on trailing edge
	g_w5500spi_ctrl.p_regs->SPCMD_b[0].CPHA=1; //!< [0..0] RSPCK Phase Setting 
	//0: Set RSPCK low during idle
	//1: Set RSPCK high during idle
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].CPOL=1; //!< [1..1] RSPCK Polarity Setting 
    
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].BRDV=0; //!< [3..2] Bit Rate Division Setting                                          
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].SSLA=0; //!< [6..4] SSL Signal Assertion Setting                                       
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].SSLKP=0; //!< [7..7] SSL Signal Level Keeping                                           
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].SPB=7; //!< [11..8] SPI Data Length Setting                                           
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].LSBF=0; //!< [12..12] SPI LSB First                                                    
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].SPNDEN=1; //!< [13..13] SPI Next-Access Delay Enable                                     
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].SLNDEN=1; //!< [14..14] SSL Negation Delay Setting Enable 
    g_w5500spi_ctrl.p_regs->SPCMD_b[0].SCKDEN=1; //!< [15..15] RSPCK Delay Setting Enable  

}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif

