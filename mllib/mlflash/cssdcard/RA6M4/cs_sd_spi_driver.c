/**
 ******************************************************************************
 * 文件: cs_sd_spi_driver.c
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

#include "mlos_ltimer.h"
#include "mlos_task.h"

#include "hal_data.h"
#include "r_spi.h"
#include"common_data.h"

//---------------------------------------------------------
//文件条件编译
//---------------------------------------------------------

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

mlu32 cssdSpievent;


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
void cssd_spi_select(void)
{

	R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_04_PIN_13,BSP_IO_LEVEL_LOW);
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
void cssd_spi_deselect(void)
{

	R_IOPORT_PinWrite(&g_ioport_ctrl,BSP_IO_PORT_04_PIN_13,BSP_IO_LEVEL_HIGH);
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
void cssd_spi_callback(spi_callback_args_t *p_args)
{
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
    {
        cssdSpievent = SPI_EVENT_TRANSFER_COMPLETE;
    }
    else
    {
        cssdSpievent = SPI_EVENT_TRANSFER_ABORTED;
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
mlu8 cssd_spi_read_byte(void)
{

	while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
	g_cssdSpi_ctrl.p_regs->SPDR_BY = 0xFF; 
	while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
	return (g_cssdSpi_ctrl.p_regs->SPDR_BY);

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
mlu8 cssd_spi_write_byte(mlu8 dat)
{
	while(g_cssdSpi_ctrl.p_regs->SPSR_b.SPTEF==_reset);
	g_cssdSpi_ctrl.p_regs->SPDR_BY=dat;//发送ADDR
	while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
	return (g_cssdSpi_ctrl.p_regs->SPDR_BY);
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
void cssd_rw_buf(mlu32 addrSel, mlu8 *buf, mlu16 len, mlu8 rw)
{
    mlu16 i, temp;
    //
    if (buf == 0 || len == 0)
    {
        return;
    }
    cssd_spi_select();
    mlos_us_delay (1);
#if 0

    //
   // if (rw == 0)
       ;// addrSel = (addrSel | _W5500_SPI_WRITE_);

    while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_cssdSpi_ctrl.p_regs->SPDR_BY = ((addrSel & 0x00FF0000) >> 16); //发送ADDR
    while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
    i = g_cssdSpi_ctrl.p_regs->SPDR_BY; //接收

    while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_cssdSpi_ctrl.p_regs->SPDR_BY = ((addrSel & 0x0000FF00) >> 8); //发送ADDR
    while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
    i = g_cssdSpi_ctrl.p_regs->SPDR_BY; //接收

    while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
    g_cssdSpi_ctrl.p_regs->SPDR_BY = (addrSel & 0x000000FF); //发送control
    while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
    i = g_cssdSpi_ctrl.p_regs->SPDR_BY; //接收

    for (i = 0; i < len; i++)
    {
        while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
        
        if (rw)
        {
       		g_cssdSpi_ctrl.p_regs->SPDR_BY = 0x55; //读
         	while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
            buf[i] = g_cssdSpi_ctrl.p_regs->SPDR_BY; //接收
        }
        else
        {
        	g_cssdSpi_ctrl.p_regs->SPDR_BY = buf[i]; //写
        	while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
            temp = g_cssdSpi_ctrl.p_regs->SPDR_BY; //接收
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
void cssd_spi_write(mlu8 *buf, mlu16 len)
{
    mlu16 i, temp;
    //
    if (buf == 0 || len == 0)
    {
        return;
    }
#if 1
    for (i = 0; i < len; i++)
    {
		while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPTEF == _reset);
		g_cssdSpi_ctrl.p_regs->SPDR_BY = buf[i]; //写
		while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
		temp = g_cssdSpi_ctrl.p_regs->SPDR_BY; //接收
        
    }
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
void cssd_spi_read(mlu8 *buf, mlu16 len)
{
    mlu16 i;
    //
    if (buf == 0 || len == 0)
    {
        return;
    }

#if 1

    for (i = 0; i < len; i++)
    {
        while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPTEF == _reset);   
       	g_cssdSpi_ctrl.p_regs->SPDR_BY = 0xFF; //读
        while (g_cssdSpi_ctrl.p_regs->SPSR_b.SPRF == _reset);
        buf[i] = g_cssdSpi_ctrl.p_regs->SPDR_BY; //接收
    }
	
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
void cssd_spi_low_speed_set(void)
{
	//初始化过程中，SD卡时钟信号周期需为100KHz～400KHz之间，不能大于400KHz。
	// 200k,速率配置
	//100M/(2*(SPBR+1)*2(BRDV次方))
	//100M/2*125*2=200k
	g_cssdSpi_ctrl.p_regs->SPCMD_b[0].BRDV=1; //2的1·次方
	g_cssdSpi_ctrl.p_regs->SPBR=124;		//124+1=125
	

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
void cssd_spi_high_speed_set(void)
{
	// 10M,速率配置
	//100M/(2*(SPBR+1)*2(BRDV次方))
	//100M/2*5*1=10M
	
	g_cssdSpi_ctrl.p_regs->SPCMD_b[0].BRDV=0; //2的0·次方
	g_cssdSpi_ctrl.p_regs->SPBR=7;		//4+1=5
	


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
void cssd_spi_clean_up(void)
{

	R_SPI_Close(&g_cssdSpi_ctrl);
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
void cssd_spi_init(void)
{
    fsp_err_t err = FSP_SUCCESS;     // Error status

    //w5500 支持 0 3 模式
    //spi.SPI_CPOL = SPI_CPOL_High;//SPI_CPOL_Low;
    //spi.SPI_CPHA = SPI_CPHA_2Edge;//SPI_CPHA_1Edge;

	//R_SPI_Close(&g_cssdSpi_ctrl);
	

    err=R_SPI_Open (&g_cssdSpi_ctrl, &g_cssdSpi_cfg);
    if(err == FSP_SUCCESS)
    {
    	g_cssdSpi_ctrl.p_regs->SPCR_b.SPE = 1;
    }

	g_cssdSpi_ctrl.bit_width=SPI_BIT_WIDTH_8_BITS;
	g_cssdSpi_ctrl.p_regs->SPDCR_b.SPBYT=1;
	r_spi_bit_width_config(&g_cssdSpi_ctrl);

	//g_cssdSpi_ctrl.p_regs->SPPCR_b.SPLP=1;//回环测试
	g_cssdSpi_ctrl.p_regs->SPCR_b.SPTIE=0;
	g_cssdSpi_ctrl.p_regs->SPCR_b.SPRIE=0;

	//0: Select data sampling on leading edge, data change on trailing edge
	//1: Select data change on leading edge, data sampling on trailing edge
	g_cssdSpi_ctrl.p_regs->SPCMD_b[0].CPHA=1; //!< [0..0] RSPCK Phase Setting
	//0: Set RSPCK low during idle
	//1: Set RSPCK high during idle
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].CPOL=1; //!< [1..1] RSPCK Polarity Setting
    
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].BRDV=2; //!< [3..2] Bit Rate Division Setting
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].SSLA=0; //!< [6..4] SSL Signal Assertion Setting
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].SSLKP=0; //!< [7..7] SSL Signal Level Keeping
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].SPB=7; //!< [11..8] SPI Data Length Setting
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].LSBF=0; //!< [12..12] SPI LSB First
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].SPNDEN=1; //!< [13..13] SPI Next-Access Delay Enable
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].SLNDEN=1; //!< [14..14] SSL Negation Delay Setting Enable
    g_cssdSpi_ctrl.p_regs->SPCMD_b[0].SCKDEN=1; //!< [15..15] RSPCK Delay Setting Enable




}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif


