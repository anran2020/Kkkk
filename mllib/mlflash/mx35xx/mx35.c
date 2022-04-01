/**
 ******************************************************************************
 * 文件:mx35_.c
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

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------
//#include "mlf_config.h"

//---------------------------------------------------------
//文件条件编译
//---------------------------------------------------------

#if  1

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------
#include "mlos.h"
#include "mx35.h"
#include "mx35_spi_driver.h"


//---------------------------------------------------------
//define
//---------------------------------------------------------

typedef struct{

	Taskptr pMytask; 				//flash任务

	mlu32 pageAddr;
	mlu32 blockAddr;
	mlu32 errorCount;
	
}MX35xxNandFlash;


//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//global variable
//----------------------------------------------------------------------------


MX35xxNandFlash mx35;
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
// 
//---------------------------------------------------------------------------
/* QSPI flash page Size */
#define PAGE_WRITE_SIZE                 (2048U)
/* QSPI flash address through page*/
#define QSPI_FLASH_ADDRESS(page_no)     (QSPI_DEVICE_START_ADDRESS + (page_no * PAGE_WRITE_SIZE))
/* default memory value */
#define DEFAULT_MEM_VAL                 (0xFF)
/* Status register pay-load */
#define STATUS_REG_PAYLOAD              {0x01,0x40,0x00}

/* data written to status register */
#define SET_SREG_VALUE                  (0x40)

/* sector size of QSPI flash device */
#define SECTOR_SIZE                     (4096U)

/* one byte data transfer */
#define ONE_BYTE                        (0x01)

/* SREG pay-load size */
#define SREG_SIZE                       (0x03)


mlu8   writeBuf[PAGE_WRITE_SIZE]= {0,};
mlu8   readBuf[PAGE_WRITE_SIZE] = {0,};
mlu8 feature[3];
mlu8 mx35lID[2];



#define MX35L_BLOCK_MAX 			2048
#define MX35L_PAGE_MAX 				64
#define MX35L_COLUMN_MAX 			2112//2048//





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
// 
//---------------------------------------------------------------------------
mlu8 mx_id_read(mlu8 *idBuf)
{
   // pQspi->SFMCMD = 1U;
	//pQspi->SFMCOM = 0x9F;
	//pQspi->SFMCOM =0;
	//mx35lID[0] = pQspi->SFMCOM ;
	//mx35lID[1] = pQspi->SFMCOM ;
   // pQspi->SFMCMD = 1U;
   // pQspi->SFMCMD = 0U;


	MxSpi mxspi ;
	mxspi.addrLen = 0;
	mxspi.dummyLen = 1;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_read(&mxspi,MX_CMD_RDID, 0, 2, idBuf);
	
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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_status_read(mlu8* staRegVal)
{
	 
	//pQspi->SFMCMD = 1U;
	//pQspi->SFMCOM = 0x0F;
	//pQspi->SFMCOM = 0xC0;
	//mlu8 status = pQspi->SFMCOM ;
	//pQspi->SFMCMD = 1U;
	//pQspi->SFMCMD = 0U;

	MxSpi mxspi ;
	mxspi.addrLen = 1;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_read(&mxspi,MX_CMD_GET_FEATURE, MX_FEATURES_STATUS_ADDR, 1, staRegVal);

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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_cnfg_read(mlu8* cfgRegVal)
{
   // pQspi->SFMCMD = 1U;

   // pQspi->SFMCOM = 0x0F;
	// pQspi->SFMCOM = 0xB0;
   // mlu8 regval = pQspi->SFMCOM ;
   // pQspi->SFMCMD = 1U;
   // pQspi->SFMCMD = 0U;
	
	MxSpi mxspi ;
	mxspi.addrLen = 1;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_read(&mxspi,MX_CMD_GET_FEATURE, MX_FEATURES_CNFG_ADDR, 1, cfgRegVal);

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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_block_protection_read(mlu8* blkpRegVal)
{
	 
	//pQspi->SFMCMD = 1U;
	//pQspi->SFMCOM = 0x0F;
	//pQspi->SFMCOM = 0xA0;
	//mlu8 regval = pQspi->SFMCOM ;
	//pQspi->SFMCMD = 1U;
	//pQspi->SFMCMD = 0U;

	MxSpi mxspi ;
	mxspi.addrLen = 1;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_read(&mxspi,MX_CMD_GET_FEATURE, MX_FEATURES_BLOCK_PROTECTION_ADDR, 1, blkpRegVal);

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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_quad_enable(void)
{

	mlu8 val;

	//mx_cnfg_read(&val);
	val=0x01;
	
	// pQspi->SFMCMD = 1U;
	//pQspi->SFMCOM = 0x1F;
	//pQspi->SFMCOM = 0xB0;
	//pQspi->SFMCOM=(feature[0]|0x01);
	//  pQspi->SFMCMD = 1U;
	// pQspi->SFMCMD = 0U;

	MxSpi mxspi ;
	mxspi.addrLen = 1;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_write(&mxspi,MX_CMD_SET_FEATURE, MX_FEATURES_CNFG_ADDR, 1, &val);
	
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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_protect_disable(void)
{

    //pQspi->SFMCMD = 1;
   // pQspi->SFMCOM=0x06;
   // pQspi->SFMCMD = 1U;
	//pQspi->SFMCOM = 0x1F;
	//pQspi->SFMCOM = 0xA0;
	//pQspi->SFMCOM=0x00;
   // pQspi->SFMCMD = 1U;
   // pQspi->SFMCMD = 0U;

	MxSpi mxspi ;
	mlu8 val=0;
	mxspi.addrLen = 1;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_write(&mxspi,MX_CMD_SET_FEATURE, MX_FEATURES_BLOCK_PROTECTION_ADDR, 1, &val);

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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_reset(void)
{
   // pQspi->SFMCMD = 1U;
   // pQspi->SFMCOM = 0xFF;
  //  pQspi->SFMCMD = 1U;
   // pQspi->SFMCMD = 0U;

	MxSpi mxspi ;
	mlu8 val=0xFF;
	mxspi.addrLen = 0;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_write(&mxspi,MX_CMD_RESET, 0, 1, &val);

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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_write_enable(void)
{
   // pQspi->SFMCMD = 1U;
   // pQspi->SFMCOM = 0x06;
  //  pQspi->SFMCMD = 1U;
   // pQspi->SFMCMD = 0U;

	MxSpi mxspi ;
	mlu8 val=0xFF;
	mxspi.addrLen = 0;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_write(&mxspi,MX_CMD_WREN, 0, 0, &val);

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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_block_erase(mlu32 bAddr)
{
	mlu8 regVal=0xFF;
	mlu32 chipAddress;
	

	chipAddress=(bAddr<<7)+((bAddr%2)<<0x06);//RA[0,16]

#if 1
	R_QSPI_Type* pQspi=R_QSPI;

	pQspi->SFMCMD = 1;
	pQspi->SFMCOM=0x06;
	pQspi->SFMCMD = 1;
	pQspi->SFMCMD = 0;

	pQspi->SFMCMD = 1;
	pQspi->SFMCOM=0xD8;
	pQspi->SFMCOM=(chipAddress >> 16);
	pQspi->SFMCOM=(chipAddress >> 8);
	pQspi->SFMCOM=(chipAddress);
	pQspi->SFMCMD = 1;
	pQspi->SFMCMD = 0;
#else
	mx_write_enable();				//写使能

	MxSpi mxspi ;
	mxspi.addrLen = 3;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	chipAddress=(bAddr<<7)+((bAddr%2)<<0x06);//RA[0,16]
	
	return mx_spi_flash_write(&mxspi,MX_CMD_BE,chipAddress, 0, &regVal);
	
#endif
	mx_status_read(&regVal);
	int i;
	while (regVal!=0)
	{
		for (i = 0; i < 100000; ++i)
		{
			
		}
		regVal = mx_status_read(&regVal);
		
	}

	return 0;
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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_program_execute(mlu32 addr)
{
	//pQspi->SFMCOM=0x10;
	//pQspi->SFMCOM=(chipAddress >> 16);
	//pQspi->SFMCOM=(chipAddress >> 8);
	//pQspi->SFMCOM=(chipAddress);
	//pQspi->SFMCMD = 1;


	MxSpi mxspi ;
	mlu8 val=0xFF;
	mxspi.addrLen = 3;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	return mx_spi_flash_write(&mxspi,MX_CMD_PROGRAM_EXEC, addr, 0, &val);

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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_page_write(mlu16 bkAddr,mlu8 pgAddr,mlu8* inputBuf,mlu16 byteCount)
{
	mlu8 regVal=0xFF;
	mlu32 chipAddress;

   //chipAddress = (mlu32) pDest - (mlu32) QSPI_DEVICE_START_ADDRESS + pQspi->SFMCNT1;

	chipAddress=(bkAddr%2);
   	chipAddress=(chipAddress<<12);
	
#if 1
	R_QSPI_Type* pQspi=R_QSPI;
	pQspi->SFMSMD_b.SFMRM=MX_SPI_MODE_FastReadQuadOutput; 

	pQspi->SFMCMD = 1;
	pQspi->SFMCOM=0x06;
    pQspi->SFMCMD = 1;
	pQspi->SFMCMD = 0;


	pQspi->SFMSAC_b.SFMAS=1;
	pQspi->SFMCMD = 1;
	pQspi->SFMCOM=0x02;//0x32;//0x02;
	pQspi->SFMCOM=(chipAddress >> 8);
	pQspi->SFMCOM=(chipAddress);

	mlu32 index = 0;
	while (index < byteCount)
	{
		pQspi->SFMCOM = inputBuf[index];
		index++;
	}
    pQspi->SFMCMD = 1;
	pQspi->SFMCMD = 0;

	pQspi->SFMSAC_b.SFMAS=2;
	pQspi->SFMCMD = 1;
	chipAddress=bkAddr;
	chipAddress=(chipAddress<<7)+((bkAddr%2)<<6)+pgAddr;
	pQspi->SFMCOM=0x10;
	pQspi->SFMCOM=(chipAddress >> 16);
	pQspi->SFMCOM=(chipAddress >> 8);
	pQspi->SFMCOM=(chipAddress);
	pQspi->SFMCMD = 1;
	pQspi->SFMCMD = 0;

#else
	mx_write_enable();				//写使能
	MxSpi mxspi ;
	mxspi.addrLen = 2;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_FastReadQuadOutput;
	
	mx_spi_flash_write(&mxspi,MX_CMD_PP_LOAD,chipAddress, byteCount, inputBuf);


	
	chipAddress=bkAddr;
	chipAddress=(chipAddress<<7)+((bkAddr%2)<<6)+pgAddr;
	
	mx_program_execute(chipAddress);

#endif
	mx_status_read(&regVal);
	int i;
	while (regVal!=0)
	{
		for (i = 0; i < 100000; ++i)
		{
			
		}
		regVal = mx_status_read(&regVal);
		
	}

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
// 1. by zzx  2015-07-29		 编写函数
// 
//---------------------------------------------------------------------------
mlu8 mx_page_read(mlu16 bkAddr,mlu8 pgAddr,mlu8* outputBuf,mlu16 byteCount)
{
	mlu8 regVal=0xFF;
	mlu32 chipAddress;
   
   //chipAddress = (mlu32) pSrc - (mlu32) QSPI_DEVICE_START_ADDRESS + pQspi->SFMCNT1;

	chipAddress=bkAddr;
	chipAddress=(chipAddress<<7)+((bkAddr%2)<<6)+pgAddr;
   
#if 0
	R_QSPI_Type* pQspi=R_QSPI;
	pQspi->SFMSMD_b.SFMRM=MX_SPI_MODE_FastReadQuadOutput; 
	pQspi->SFMSAC_b.SFMAS=2;

	//读取到cahche
	pQspi->SFMCMD = 1;
	pQspi->SFMCOM=0x13;
	pQspi->SFMCOM=(chipAddress >> 16);
	pQspi->SFMCOM=(chipAddress >> 8);
	pQspi->SFMCOM=(chipAddress);
    pQspi->SFMCMD = 1;
	pQspi->SFMCMD = 0;

	mx_status_read(&regVal);
	int i;
	while (regVal!=0)
	{
		for (i = 0; i < 100000; ++i)
		{
			
		}
		regVal = mx_status_read(&regVal);
		
	}


	chipAddress=(bkAddr%2);
	chipAddress=(chipAddress<<12);
    pQspi->SFMCMD = 1;
	pQspi->SFMCOM=0x03;//0x03;//0x6B;
	pQspi->SFMCOM=(chipAddress>>8);
	pQspi->SFMCOM=chipAddress;
	pQspi->SFMCOM=0;

    mlu32 index = 0;
    while (index < byteCount)
    {
        outputBuf[index]=pQspi->SFMCOM;
        index++;
    }
	
    pQspi->SFMCMD = 1;
    pQspi->SFMCMD = 0;		

#else
	MxSpi mxspi ;
	mxspi.addrLen = 3;
	mxspi.dummyLen = 0;
	mxspi.mode = MX_SPI_MODE_StandardRead;
	
	mx_spi_flash_write(&mxspi,MX_CMD_READ, chipAddress, 0, &regVal);				//读数据到cache

	
	mx_status_read(&regVal);
	int i;
	while (regVal!=0)
	{
		for (i = 0; i < 100000; ++i)
		{
			
		}
		regVal = mx_status_read(&regVal);
		
	}


	chipAddress=(bkAddr%2);
	chipAddress=(chipAddress<<12);
	regVal=0xFF;
	mxspi.addrLen = 2;
	mxspi.dummyLen = 1;
	mxspi.mode = MX_SPI_MODE_FastReadQuadOutput;
	return mx_spi_flash_read(&mxspi,MX_CMD_READ_CACHE,chipAddress, byteCount, outputBuf);
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
Taskstate mx_task(void*args)
{
	task_declaration(); 								//任务声明
	mlu32 i;
	

	task_begin(mx35.pMytask);							//任务开始
	for (i = 3; i < PAGE_WRITE_SIZE; i++)
    {
        writeBuf[i] = i;
    }
	mx_block_erase(mx35.blockAddr);
//	task_yield(mx35.pMytask, TASK_STA_NONBLOCKING_YIELD);
	while (mltrue)
	{

		//填充测试数据
		writeBuf[0] = mx35.blockAddr;
		writeBuf[1] = (mx35.blockAddr>>8);
		writeBuf[2] = mx35.pageAddr;
		
		//校验擦除
		mx_page_read(mx35.blockAddr,mx35.pageAddr,readBuf,PAGE_WRITE_SIZE);
		for (i = 0; i < PAGE_WRITE_SIZE; ++i)
		{
			if (readBuf[i]!=0xFF)
			{
				mx35.errorCount++;
				break;
			}
		}
		//rom 读取
		//flashRomAddr=(QSPI_DEVICE_START_ADDRESS + (pageCount* MX35L_COLUMN_MAX));
		//pageCount++;
		//memcpy(readBuf, (mlu8*)flashRomAddr, PAGE_WRITE_SIZE);//rom
		
		//写入
		mx_page_write(mx35.blockAddr,mx35.pageAddr,writeBuf,PAGE_WRITE_SIZE);

		//读取校验
		mx_page_read(mx35.blockAddr,mx35.pageAddr,readBuf,PAGE_WRITE_SIZE);
		//errorCount=0;
		for (i = 0; i < PAGE_WRITE_SIZE; ++i)
		{
			if (readBuf[i]!=writeBuf[i])
			{
				mx35.errorCount++;
				break;
			}
		}
		
		//if(memcmp(readBuf, writeBuf, PAGE_WRITE_SIZE))
		//{
		//	//写入失败
		//	readBuf[0]=0xff;
		//}
		//memcpy(readBuf, (mlu8*)flashRomAddr, PAGE_WRITE_SIZE);
		task_ms_delay(mx35.pMytask,100, TASK_STA_NONBLOCKING_YIELD);
		mx35.pageAddr++;
		if(mx35.pageAddr>=MX35L_PAGE_MAX)
		{
			mx35.pageAddr=0;
			mx35.blockAddr++;
			if(mx35.blockAddr>=MX35L_BLOCK_MAX)
			{
				mx35.blockAddr=0;
			}
			break;
		}
				

	}

	task_end(mx35.pMytask); 							//任务结束
	
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
void mx_init(void)
{
	
	mx_spi_init();


	mx_reset();
	
	mx_quad_enable();
	mx_protect_disable();
	
	mx_cnfg_read(feature);
	mx_status_read(feature+1);
	mx_block_protection_read(feature+2);
	
	mx_id_read(mx35lID);
	mx35.errorCount=0;
	mx35.pageAddr=0;
	mx35.blockAddr=0;

	mx35.pMytask=task_create(TASK_PRIO_6, mx_task,nullptr,ltimer_create(),"mx35");
	
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 





