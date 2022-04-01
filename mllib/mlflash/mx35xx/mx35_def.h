/**
 ******************************************************************************
 * 文件:mx35_def.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1      2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/


#ifndef _MX35_DEF_H_
#define _MX35_DEF_H_

//---------------------------------------------------------
//文件条件编译
//---------------------------------------------------------

#if  1

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------


#include "mlos_dtd.h"


//---------------------------------------------------------
//define
//---------------------------------------------------------

// get/set features address definition 
#define  MX_FEATURES_BLOCK_PROTECTION_ADDR  		0xA0
#define  MX_FEATURES_SECURE_OTP_ADDR 				0xB0			//1G
#define  MX_FEATURES_CNFG_ADDR 						0xB0			//2G
#define  MX_FEATURES_STATUS_ADDR 					0xC0


// status register definition 

#define  MX_SR_OIP            	 	 		0x01 
#define  MX_SR_WEL			   	 			0x02
#define  MX_SR_ERASE_FAIL     	  			0x04
#define  MX_SR_PROGRAM_FAIL    	  			0x08
#define  MX_SR_ECC_STATUS_MASK    			0x30
#define  MX_SR_ECC_STATUS_NO_ERR    		0x00
#define  MX_SR_ECC_STATUS_ERR_COR    		0x10
#define  MX_SR_ECC_STATUS_ERR_NO_COR   		0x20

// secure OTP register definition 
#define  MX_SECURE_OTP_QE              				 0x01
#define  MX_SECURE_OTP_ECC_EN		     			0x10
#define  MX_SECURE_OTP_SECURE_OTP_EN    			0x40
#define  MX_SECURE_OTP_SECURE_OTP_PROT  			0x80


//cmd
#define MX_CMD_RDID					 0x9F		
#define MX_CMD_READ					 0x13
#define MX_CMD_READ_CACHE			 0x0B
#define MX_CMD_READ_CACHE2			 0x3B
#define MX_CMD_READ_CACHE4			 0x6B
#define MX_CMD_READ_CACHE_SEQUENTIAL 0x31
#define MX_CMD_READ_CACHE_END		 0x3F

#define MX_CMD_GET_FEATURE			 0x0F
#define MX_CMD_SET_FEATURE			 0x1F

#define MX_CMD_WREN                  0x06
#define MX_CMD_WRDI                  0x04
#define MX_CMD_PP_LOAD				 0x02
#define MX_CMD_PP_RAND_LOAD			 0x84
#define MX_CMD_4PP_LOAD				 0x32
#define MX_CMD_4PP_RAND_LOAD	     0x34
#define MX_CMD_PROGRAM_EXEC			 0x10

#define MX_CMD_BE					 0xD8
#define MX_CMD_RESET				 0xFF
#define MX_CMD_ECC_STAT_READ		 0x7C


//---------------------------------------------------------
//mx nand flash spi 参数定义
//---------------------------------------------------------
typedef struct {

	mlu8 mode;
	mlu8 cmdLen;
	mlu8 dummyLen;
	mlu8 addrLen;
	mlu8 wCmd;
	mlu8 rCmd;
	mlu8 *txBuf;	 
	mlu8 *rxBuf;	
	mlu32 baseAddr;
	
} MxSpi;

//---------------------------------------------------------
// spi transfer mode 
//---------------------------------------------------------
enum MxSpiTransferMode{

	MX_SPI_MODE_StandardRead=0,
	MX_SPI_MODE_FastRead=1,
	MX_SPI_MODE_FastReadDualOutput=2,
	MX_SPI_MODE_FastReadDualIO=3,
	MX_SPI_MODE_FastReadQuadOutput=4,
	MX_SPI_MODE_FastReadQuadIO=5,

};
	
//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 				//文件条件编译结束

#endif 

