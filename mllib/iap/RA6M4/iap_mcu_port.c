/**
 ******************************************************************************
 * 文件:   iap_mcu_port.c 
 * 作者:   
 * 版本:   V0.01
 * 日期:    
 * 内容简介:
 * 	iap模块 ，mcu相关底层接口，根据不同mcu实现统一的标准接口
 * 
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   ----   作者 --------   说明
 * v0.1    2015-11-07     zzx        创建该文件
 *
 *
 *
 ******************************************************************************
 **/
 
//----------------------------------------------------------------------------
//文件条件编译
//----------------------------------------------------------------------------
#if 1

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "iap_mcu_port.h"
#include "hal_data.h"
#include "iap_def.h"
#include "base_addresses.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//global vari
//----------------------------------------------------------------------------



#ifdef COMPILE_BOOT_CODE

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
MLBool iap_app_exist(void)
{
	mlvu32 * vu32ptr,u32temp;
	
	//根据app文件信息，判断更新
#if 1
	AppFileInfoPtr appfileptr;

	appfileptr=(AppFileInfoPtr)APP_FILE_INFO_ADDR;

	if(appfileptr->size!=0xFFFFFFFF)
	{

		return mltrue;
	}
	
#endif	

	return mlfalse;
	
}
#endif

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by djx  2021-10-12		 编写函数
//---------------------------------------------------------------------------
void iap_disable_irq(void)
{
#if 1
	__disable_irq();

	R_BSP_IrqDisable(GPT0_COUNTER_OVERFLOW_IRQn);

	R_BSP_IrqDisable(SPI0_RXI_IRQn);
	R_BSP_IrqDisable(SPI0_TXI_IRQn);
	R_BSP_IrqDisable(SPI0_TEI_IRQn);
	R_BSP_IrqDisable(SPI0_ERI_IRQn);


	R_BSP_IrqDisable(CAN0_ERROR_IRQn);
	R_BSP_IrqDisable(CAN0_MAILBOX_RX_IRQn);
	R_BSP_IrqDisable(CAN0_MAILBOX_TX_IRQn);
	
	R_BSP_IrqDisable(CAN1_ERROR_IRQn);
	R_BSP_IrqDisable(CAN1_MAILBOX_RX_IRQn);
	R_BSP_IrqDisable(CAN1_MAILBOX_RX_IRQn);

	R_BSP_IrqDisable(SPI1_RXI_IRQn);
	R_BSP_IrqDisable(SPI1_TXI_IRQn);
	R_BSP_IrqDisable(SPI1_TEI_IRQn);
	R_BSP_IrqDisable(SPI1_ERI_IRQn);


	R_BSP_IrqDisable(SCI7_RXI_IRQn);
	R_BSP_IrqDisable(SCI7_TXI_IRQn);
	R_BSP_IrqDisable(SCI7_TEI_IRQn);
	R_BSP_IrqDisable(SCI7_ERI_IRQn);


	R_BSP_IrqDisable(SCI1_RXI_IRQn);
	R_BSP_IrqDisable(SCI1_TXI_IRQn);
	R_BSP_IrqDisable(SCI1_TEI_IRQn);
	R_BSP_IrqDisable(SCI1_ERI_IRQn);

	R_BSP_IrqDisable(GPT0_COUNTER_OVERFLOW_IRQn);

	__disable_irq();

#endif

}


//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by djx  2021-10-12		 编写函数
//---------------------------------------------------------------------------
void iap_set_msp(MLBool bSetBootmsp)
{
#if 1
    mlu32* mspptr;
	//
	if(bSetBootmsp)
	{
		
		mspptr=(mlu32*)(BOOT_SP_ADDR);
		SCB->VTOR = (BOOT_SP_ADDR & 0x1FFFFF80);
		__DSB(); 
		__ISB(); 
	}
	else
	{
		mspptr=(mlu32*)(APP_SP_ADDR);
		SCB->VTOR =(APP_SP_ADDR & 0x1FFFFF80); 
		__DSB(); 
		__ISB(); 
	}


#if BSP_FEATURE_BSP_HAS_SP_MON
	R_MPU_SPMON->SP[0].CTL = 0;
	while(R_MPU_SPMON->SP[0].CTL != 0);
#endif

	__set_MSP(*mspptr);


	if(bSetBootmsp)
	{
		//跳转boot，直接复位
		__DSB();														  /* Ensure all outstanding memory accesses included
																			 buffered write are completed before reset */
		SCB->AIRCR	= (mlu32)((0x5FAUL << SCB_AIRCR_VECTKEY_Pos)	|
								 (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
								  SCB_AIRCR_SYSRESETREQ_Msk    );		  /* Keep priority group unchanged */
		__DSB(); 
	}

#endif
}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


