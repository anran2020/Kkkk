/**
 ******************************************************************************
 * 文件:iap_mcu_port.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1     2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef  __IAP_MCU_PORT_H__
#define  __IAP_MCU_PORT_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"


//----------------------------------------------------------------------------
//文件条件编译
//----------------------------------------------------------------------------
#if 1

//----------------------------------------------------------------------------
//define config
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------

#define	MCU_RAM_BASE_ADDR 				(0x20000000)				//内存起始地址



#define  BOOT_BLOCK_SIZE 				(1024*8) 			//8k
		
#define  BOOT_FLASH_BASE_ADDR 			0x00000000
#define  BOOT_SP_ADDR 					0x00000000
#define  BOOT_ENTRY_ADDR 				0x00000004
#define  BOOT_FLASH_BLCOK_NUM			8		//0-7
#define  BOOT_FLASH_SIZE	 			(BOOT_BLOCK_SIZE*BOOT_FLASH_BLCOK_NUM)  //boot 空间大小 0x80000-0x6000


#define APP_BLOCK_SIZE 					(1024*32) 			//32k
			
#define  APP_FILE_INFO_ADDR 			0x00010000				//1k存文件信息			
#define  APP_FLASH_BASE_ADDR 			0x00010000
#define  APP_SP_ADDR 					0x00010400
#define  APP_ENTRY_ADDR 				0x00010404
#define  APP_FLASH_BLCOK_NUM			10 		//8-17
#define  APP_FLASH_SIZE	 				(APP_FLASH_BLCOK_NUM*APP_BLOCK_SIZE-1024) //app 空间大小 0x80000-0x6000



//----------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// export fun 
//----------------------------------------------------------------------------

#ifdef COMPILE_BOOT_CODE
MLBool iap_app_exist(void);
#endif

void iap_disable_irq(void);
void iap_set_msp(MLBool bSetBootmsp);


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif//条件编译
#endif//编译一次

