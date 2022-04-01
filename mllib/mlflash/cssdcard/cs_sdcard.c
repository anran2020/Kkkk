/**
 ******************************************************************************
 * 文件:cs_sdcard.c
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
#include "cs_sdcard.h"
#include "mlos.h"
#include"cs_sd_spi_driver.h"

/** 
  * @brief  Card Specific Data: CSD Register   
  */ 
typedef struct {

	union{

		mlu16 CSD_STRUCTURE;
		struct{
			mlu8  Reserved1:6;            /*!< Reserved */
			mlu8  CSDStruct:2;			  /*!< CSD structure */
		};
	};
	
	mlu8  TAAC;                 /*!< Data read access-time 1 */
	mlu8  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
	mlu8  SPEED;        /*!< Max. bus clock frequency */


	union{

		mlu16 CCC_RDBLKEN;
		struct{
		mlu16  RdBlockLen:4;           /*!< Max. read data block length */
		mlu16 CCC:12;      /*!< Card command classes */
		};
	};

	union{

		mlu32 C_SIZE;
	struct{
		mlu32  DeviceSize:22;		   /*!< Device Size */
		mlu32  Reserved2:6; 		   /*!< Reserved */
		mlu32  DSRImpl:1;			   /*!< DSR implemented */
		mlu32  RdBlockMisalign:1;	   /*!< Read block misalignment */
		mlu32  WrBlockMisalign:1;	   /*!< Write block misalignment */
		mlu32  PartBlockRead:1;        /*!< Partial blocks for read allowed */
		};	
	
	};
	
	
	union{

		mlu16 SECTOR_SIZE;
		struct{
			mlu16  WrProtectGrSize:7;      /*!< Write protect group size */
			mlu16  EraseSectorSize:7;   /*!< Max. write current @ VDD min */
			mlu16  EraseBlkEn:1;   /*!< Max. read current @ VDD max */
			mlu16  Reserved3:1;   /*!< Max. read current @ VDD min */
		};
	};

	union{

		mlu32 R2W_FACTOR;
		struct{
			mlu32  Reserved7:1;            /*!< always 1*/
			mlu32  CSD_CRC:7;              /*!< CSD CRC */
			mlu32  Reserved6:2; 				 /*!< ECC code */
			mlu32  FileFormat:2;		   /*!< File Format */
			mlu32  TempWrProtect:1; 	   /*!< Temporary write protection */
			mlu32  PermWrProtect:1; 	   /*!< Permanent write protection */
			mlu32  CopyFlag:1;			   /*!< Copy flag (OTP) */
			mlu32  FileFormatGrouop:1;	   /*!< File format group */
			mlu32  Reserved5:5; 		   /*!< Reserded */
			mlu32  WriteBlockPaPartial:1;  /*!< Partial blocks for write allowed */
			mlu32  MaxWrBlockLen:4; 	   /*!< Max. write data block length */
			mlu32  WrSpeedFact:3;		   /*!< Write speed factor */
			mlu32  Reserved4:2; 		  /*!< Manufacturer default ECC */		
			mlu32  WrProtectGrEnable:1;    /*!< Write protect group enable */
		};
	};
} SD_CSD;

/** 
  * @brief  Card Identification Data: CID Register   
  */
typedef struct {
	mlu8  ManufacturerID;       /*!< ManufacturerID */
	mlu16 OEM_AppliID;          /*!< OEM/Application ID */
	mlu32 ProdName[5];            /*!< Product Name part1 */
	mlu8  ProdRev;              /*!< Product Revision */
	mlu32 ProdSN[4];               /*!< Product Serial Number */
	mlu8  Reserved1;            /*!< Reserved1 */
	mlu16 ManufactDate;         /*!< Manufacturing Date */
	mlu8  CID_CRC;              /*!< CID CRC */
	mlu8  Reserved2;            /*!< always 1 */
} SD_CID;

/** 
  * @brief SD Card information 
  */
typedef struct {
	SD_CSD SD_csd;
	SD_CID SD_cid;
	mlu64 CardCapacity;  /*!< Card Capacity */
	mlu32 CardBlockSize; /*!< Card Block Size */
} SD_CardInfo;
/*
 * @brief  Start Data tokens:
 *         Tokens (necessary because at nop/idle (and CS active) only 0xff is
 *         on the data/command line)
 */
#define SD_START_DATA_SINGLE_BLOCK_READ    0xFE  /*!< Data token start byte, Start Single Block Read */
#define SD_START_DATA_MULTIPLE_BLOCK_READ  0xFE  /*!< Data token start byte, Start Multiple Block Read */
#define SD_START_DATA_SINGLE_BLOCK_WRITE   0xFE  /*!< Data token start byte, Start Single Block Write */
#define SD_START_DATA_MULTIPLE_BLOCK_WRITE 0xFC  /*!< Data token start byte, Start Multiple Block Write */

//---------------------------------------------------------
//define
//---------------------------------------------------------

#define SD_CMD0          0   /*!< CMD0 = 0x40 */
#define SD_CMD8          8   /*!< CMD8 = 0x48 */
#define SD_CMD9          9   /*!< CMD9 = 0x49 */
#define SD_CMD10         10  /*!< CMD10 = 0x4A */
#define SD_CMD12         12  /*!< CMD12 = 0x4C */
#define SD_CMD16         16  /*!< CMD16 = 0x50 */
#define SD_CMD17         17  /*!< CMD17 = 0x51 */
#define SD_CMD18         18  /*!< CMD18 = 0x52 */
#define SD_ACMD23        23  /*!< CMD23 = 0x57 */
#define SD_CMD24         24  /*!< CMD24 = 0x58 */
#define SD_CMD25         25  /*!< CMD25 = 0x59 */
#define SD_ACMD41        41  /*!< ACMD41 = 0x41 */
#define SD_CMD55         55  /*!< CMD55 = 0x55 */
#define SD_CMD58         58  /*!< CMD58 = 0x58 */
#define SD_CMD59         59  /*!< CMD59 = 0x59 */

//---------------------------------------------------------
//define
//---------------------------------------------------------

typedef struct{


	mlu8 rbuf[CSSD_SECTOR_SIZE];
	mlu8 wbuf[CSSD_SECTOR_SIZE];
	
	Taskptr pMytask; 				//flash任务

	mlu32 sector;
	
}CSSDChip;


//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//global variable
//----------------------------------------------------------------------------



CSSDChip cssd;


SD_CardInfo cardinfo;

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
_inline void cssd_cs_high()
{

	cssd_spi_deselect();
	cssd_spi_write_byte(0xff);
	cssd_spi_write_byte(0xff);
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
_inline void cssd_cs_low()
{
	cssd_spi_deselect();
	cssd_spi_write_byte(0xff);
	cssd_spi_select();

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
mlu8 cssd_response_read(void)
{
	mlu8 r1;
	mlu32 retryTimes;
	
	//等待响应，或超时退出
	retryTimes=0;
	r1 = cssd_spi_read_byte();
	while(r1==0xFF)
	{
		retryTimes++;
		if(retryTimes > 800)
		{
			break;
		}

		r1 = cssd_spi_read_byte();
	}

	return r1;
	
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
mlu8 cssd_cmd_send(mlu8 cmd, mlu32 arg, mlu8 crc)
{
	mlu8 r1;
	mlu32 retryTimes;


	cssd_spi_write_byte(cmd | 0x40);
	cssd_spi_write_byte(arg >> 24);
	cssd_spi_write_byte(arg >> 16);
	cssd_spi_write_byte(arg >> 8);
	cssd_spi_write_byte(arg);
	cssd_spi_write_byte(crc);
	

	//等待响应，或超时退出
	retryTimes=0;
	r1 = cssd_spi_read_byte();
	while(r1==0xFF)
	{
		retryTimes++;
		if(retryTimes > 800)
		{
			break;
		}

		r1 = cssd_spi_read_byte();
	}

	return r1;
	
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
mlu8 cssd_dataresponse_read(void)
{
	mlu8 response;

	cssd_spi_read(&response, 1);

	response &= 0x1F;						//Mask unused bits
	if (response != 0x05)
		return 0xFF;
	
	cssd_spi_read(&response, 1);						//Wait null data
	while (response == 0)
		cssd_spi_read(&response, 1);

	return 0;
}

/*
 * @brief  Reads a block of data from the SD.
 * @param  data_buff: pointer to the buffer that receives the data read from the
 *                  SD.
 * @param  sector: SD's internal address to read from.
 * @retval The SD Response:
 *         - 0xFF: Sequence failed
 *         - 0: Sequence succeed
 */
MLBool cssd_read_sector(mlu8 *outBuff, mlu32 sector, mlu32 secCount,mlu16 rSecBytes)
{
	mlu8 frame[2], rMultiSecFlag,result;
	/*!< Send CMD17 (SD_CMD17) to read one block */

	sector=(sector<<9);
	cssd_cs_low();
	if (secCount == 1) 
	{
		rMultiSecFlag = 0;
		result=cssd_cmd_send(SD_CMD17, sector, 0);
	}
	else
	{
		rMultiSecFlag = 1;
		result=cssd_cmd_send(SD_CMD18, sector, 0);
	}
	/*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
	if (result!= 0x00) 
	{
		log_print(LOG_LVL_INFO,"sd read sector %d error=%d!", sector,result);
		cssd_cs_high();
		return mlfalse;
	}
	while (secCount) 
	{
		if (cssd_response_read() != SD_START_DATA_SINGLE_BLOCK_READ)
			break;
		/*!< Read the SD block data : read NumByteToRead data */
		cssd_spi_read(outBuff, rSecBytes);
		/*!< Get CRC bytes (not really needed by us, but required by SD) */
		cssd_spi_read(frame, 2);
		outBuff += rSecBytes;
		secCount--;
	}
	cssd_cs_high();
	if (rMultiSecFlag)
	{
		cssd_cmd_send(SD_CMD12, 0, 0);
		cssd_response_read();
		cssd_cs_high();
		cssd_cs_high();
	}
	/*!< Returns the reponse */
	return secCount > 0 ? mlfalse : mltrue;
	
}



mlu32 cssd_read_a_sector(mlu32 sector,mlu8 *outBuf, mlu32 len)
{
	mlu8 frame[2],result;
	/*!< Send CMD17 (SD_CMD17) to read one block */
	cssd_cs_low();

	//读一个扇区
	result=cssd_cmd_send(SD_CMD17, sector, 0);

	/*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
	if (result!= 0x00) 
	{
		log_print(LOG_LVL_INFO,"sd read sector %d error=%d!", sector,result);
		cssd_cs_high();
		return 0;
	}

	//读取数据
	if (cssd_response_read() != SD_START_DATA_SINGLE_BLOCK_READ)
		len=0;
	/*!< Read the SD block data : read NumByteToRead data */
	cssd_spi_read(outBuf, len);
	/*!< Get CRC bytes (not really needed by us, but required by SD) */
	cssd_spi_read(frame, 2);

	cssd_cs_high();

	/*!< Returns the reponse */
	return len;
	
}

/*
 * @brief  Writes a block on the SD
 * @param  data_buff: pointer to the buffer containing the data to be written on
 *                  the SD.
 * @param  sector: address to write on.
 * @retval The SD Response:
 *         - 0xFF: Sequence failed
 *         - 0: Sequence succeed
 */
MLBool cssd_write_sector(mlu8 *inBuff, mlu32 sector, mlu32 count,mlu16 wSecBytes)
{
	mlu8 result,i;
	mlu8 frame[2] = {0xFF};

	sector=(sector<<9);

	cssd_cs_low();
	if (count == 1)
	{
		frame[1] = SD_START_DATA_SINGLE_BLOCK_WRITE;
		result=cssd_cmd_send(SD_CMD24, sector, 0);
	} else 
	{
		frame[1] = SD_START_DATA_MULTIPLE_BLOCK_WRITE;
		result=cssd_cmd_send(SD_ACMD23, count, 0);
		cssd_cs_high();
		cssd_cmd_send(SD_CMD25, sector, 0);
	}
	/*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
	if (result != 0x00) 
	{
		log_print(LOG_LVL_INFO,"sd write sector %d error=%d!", sector,result);
		cssd_cs_high();
		return mlfalse;
	}
	cssd_cs_high();
	for (i = 0; i < 100; ++i)
	{
		cssd_spi_write_byte(0xff);
	}
	cssd_cs_low();
	while (count--) 
	{
		cssd_spi_write_byte(0xfe);
		cssd_spi_write(inBuff, wSecBytes);
		cssd_spi_write_byte(0xff);
		cssd_spi_write_byte(0xff);
		inBuff += wSecBytes;
		/*!< Read data response */
		if (cssd_dataresponse_read() != 0x00) 
		{
			cssd_cs_high();
			return mlfalse;
		}
	}
	cssd_cs_high();
	cssd_cs_high();
	/*!< Returns the reponse */
	return mltrue;
}

mlu32 cssd_write_a_sector(mlu32 sector,mlu8 *inBuf, mlu32 len)
{
	mlu8 result,i;
	mlu8 frame[2] = {0xFF};
	
	sector=sector<<9;
	if(len>CSSD_SECTOR_SIZE)
	{
		len=CSSD_SECTOR_SIZE;
	}
	
	cssd_cs_low();

	//读单独一个block
	frame[1] = SD_START_DATA_SINGLE_BLOCK_WRITE;
	result=cssd_cmd_send(SD_CMD24, sector, 0);
 

	/*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
	if (result != 0x00) 
	{
		log_print(LOG_LVL_INFO,"sd write sector %d error=%d!", sector,result);
		cssd_cs_high();
		return 0;
	}
	cssd_cs_high();
	for (i = 0; i < 100; ++i)
	{
		cssd_spi_write_byte(0xff);
	}
	cssd_cs_low();

	//写入
	cssd_spi_write_byte(0xfe);
	cssd_spi_write(inBuf, len);
	cssd_spi_write_byte(0xff);
	cssd_spi_write_byte(0xff);
	
	//读应答
	if (cssd_dataresponse_read() != 0x00) {
		cssd_cs_high();
		return 0;
	}

	cssd_cs_high();
	cssd_cs_high();
	
	return len;			//返回实际写入的数据
	
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
/*
 * @brief  Read the CSD card register
 *         Reading the contents of the CSD register in SPI mode is a simple
 *         read-block transaction.
 * @param  SD_csd: pointer on an SCD register structure
 * @retval The SD Response:
 *         - 0xFF: Sequence failed
 *         - 0: Sequence succeed
 */

mlu8 cssd_csdregister_read(SD_CSD *SD_csd)
{
#if 1
	mlu8 csd_tab[18],result;
	mlu16 u16temp;
	/*!< Send CMD9 (CSD register) or CMD10(CSD register) */
	cssd_cs_low();
	result=cssd_cmd_send(SD_CMD9, 0, 0);
	
	/*!< Wait for response in the R1 format (0x00 is no errors) */
	if (result!= 0x00) {

		cssd_cs_high();
		return 0xFF;
	}
	if (cssd_response_read() != SD_START_DATA_SINGLE_BLOCK_READ) {
		cssd_cs_high();
		return 0xFF;
	}
	/*!< Store CSD register value on csd_tab */
	/*!< Get CRC bytes (not really needed by us, but required by SD) */
	cssd_spi_read(csd_tab, 18);
	cssd_cs_high();

	
	SD_csd->CSD_STRUCTURE=csd_tab[0];
	SD_csd->TAAC = csd_tab[1];
	SD_csd->NSAC = csd_tab[2];
	SD_csd->SPEED = csd_tab[3];
	SD_csd->CCC_RDBLKEN=csd_tab[4];
	SD_csd->CCC_RDBLKEN=(SD_csd->CCC_RDBLKEN<<8)+csd_tab[5];
	
	SD_csd->C_SIZE=	csd_tab[6];
	SD_csd->C_SIZE=(SD_csd->C_SIZE<<8)+csd_tab[7];
	SD_csd->C_SIZE=(SD_csd->C_SIZE<<8)+csd_tab[8];
	SD_csd->C_SIZE=(SD_csd->C_SIZE<<8)+csd_tab[9];


	SD_csd->SECTOR_SIZE=csd_tab[10];
	SD_csd->SECTOR_SIZE=(SD_csd->SECTOR_SIZE<<8)+csd_tab[11];

	SD_csd->R2W_FACTOR=	csd_tab[12];
	SD_csd->R2W_FACTOR=(SD_csd->R2W_FACTOR<<8)+csd_tab[13];
	SD_csd->R2W_FACTOR=(SD_csd->R2W_FACTOR<<8)+csd_tab[14];
	SD_csd->R2W_FACTOR=(SD_csd->R2W_FACTOR<<8)+csd_tab[15];
	
#if 0
	
	/*!< Byte 0 */
	SD_csd->CSDStruct = (csd_tab[0] >> 6)&0x03 ;
	SD_csd->Reserved1 = (csd_tab[0] & 0x3F);
	/*!< Byte 1 */
	SD_csd->TAAC = csd_tab[1];
	/*!< Byte 2 */
	SD_csd->NSAC = csd_tab[2];
	/*!< Byte 3 */
	SD_csd->SPEED = csd_tab[3];
	/*!< Byte 4 */
	SD_csd->CCC = csd_tab[4];
	SD_csd->CCC=(SD_csd->CCC<<4);
	SD_csd->CCC |= ((csd_tab[5]>>4)&0x0f);
	SD_csd->RdBlockLen = csd_tab[5] & 0x0F;
	
	/*!< Byte 6 */
	SD_csd->PartBlockRead = ((csd_tab[6]>>7)&0x01);
	SD_csd->WrBlockMisalign =((csd_tab[6]>>6)&0x01);
	SD_csd->RdBlockMisalign = ((csd_tab[6]>>5)&0x01);
	SD_csd->DSRImpl = ((csd_tab[6]>>4)&0x01);
	
	SD_csd->Reserved2 = ((csd_tab[6]&0x0f);//6bit
	SD_csd->Reserved2|= ((csd_tab[7]>>6)&0x03);
	
	SD_csd->DeviceSize = (csd_tab[7] & 0x3F);//22bit
	SD_csd->DeviceSize=(SD_csd->DeviceSize<<16);
	
	SD_csd->DeviceSize= ((csd_tab[8]>>2)&0x3f);

	SD_csd->Reserved3 = ((csd_tab[8]>>1)&0x01);
	SD_csd->EraseBlkEn = ((csd_tab[8]>>0)&0x01);

	SD_csd->EraseSectorSize = ((csd_tab[9]>>1)& 0x7f);

	SD_csd->WrProtectGrSize = (csd_tab[9] & 0x01);
	SD_csd->WrProtectGrSize=(SD_csd->WrProtectGrSize<<6);
	SD_csd->WrProtectGrSize |=((csd_tab[10] >>2)&0x03f);

	SD_csd->WrProtectGrEnable = ((csd_tab[10] >>1)&0x01);
	
	SD_csd->Reserved4 = ((csd_tab[10] >>0)&0x01);
	SD_csd->Reserved4=(SD_csd->Reserved4<<1);
	SD_csd->Reserved4 |=((csd_tab[11] >>7)&0x01);
	
	SD_csd->WrSpeedFact = ((csd_tab[11] >>4)&0x07); ;//
	
	SD_csd->MaxWrBlockLen = ((csd_tab[11] >>0)&0x0F);
	
	SD_csd->WriteBlockPaPartial = ((csd_tab[11] >>3)&0x01);
	SD_csd->Reserved3 = 0;//5bit
	/*!< Byte 14 */
	SD_csd->FileFormatGrouop = ((csd_tab[12] >>5)&0x01);
	SD_csd->CopyFlag = ((csd_tab[12] >>4)&0x01);
	
	SD_csd->PermWrProtect = ((csd_tab[12] >>3)&0x01);
	SD_csd->TempWrProtect =((csd_tab[12] >>2)&0x01);
	
	SD_csd->FileFormat = (csd_tab[12] & 0x03);
	//SD_csd->ECC = (csd_tab[14] & 0x03);
	/*!< Byte 15 */
	SD_csd->CSD_CRC = (csd_tab[15] & 0xFE) >> 1;
	SD_csd->Reserved4 = 1;
	/*!< Return the reponse */
#endif
#endif
	return 0;

}


/*
 * @brief  Read the CID card register.
 *         Reading the contents of the CID register in SPI mode is a simple
 *         read-block transaction.
 * @param  SD_cid: pointer on an CID register structure
 * @retval The SD Response:
 *         - 0xFF: Sequence failed
 *         - 0: Sequence succeed
 */
mlu8 cssd_cidregister_read(SD_CID *SD_cid)
{
#if 1
	mlu8 cid_tab[18],result;
	/*!< Send CMD10 (CID register) */
	cssd_cs_low();
	result=cssd_cmd_send(SD_CMD10, 0, 0);
	/*!< Wait for response in the R1 format (0x00 is no errors) */
	if (result != 0x00) {
		cssd_cs_high();
		return 0xFF;
	}
	if (cssd_response_read() != SD_START_DATA_SINGLE_BLOCK_READ) {
		cssd_cs_high();
		return 0xFF;
	}
	/*!< Store CID register value on cid_tab */
	/*!< Get CRC bytes (not really needed by us, but required by SD) */
	cssd_spi_read(cid_tab, 18);
	cssd_cs_high();
	/*!< Byte 0 */
	SD_cid->ManufacturerID = cid_tab[0];
	/*!< Byte 1 */
	SD_cid->OEM_AppliID = cid_tab[1];
	/*!< Byte 2 */
	SD_cid->OEM_AppliID = (SD_cid->OEM_AppliID <<8)+cid_tab[2];
	/*!< Byte 3 */
	SD_cid->ProdName[0] = cid_tab[3];
	/*!< Byte 4 */
	SD_cid->ProdName[1]= cid_tab[4];
	/*!< Byte 5 */
	SD_cid->ProdName[2]= cid_tab[5];
	/*!< Byte 6 */
	SD_cid->ProdName[3]= cid_tab[6];
	/*!< Byte 7 */
	SD_cid->ProdName[4] = cid_tab[7];
	/*!< Byte 8 */
	SD_cid->ProdRev = cid_tab[8];
	/*!< Byte 9 */
	SD_cid->ProdSN[3] = cid_tab[9];
	/*!< Byte 10 */
	SD_cid->ProdSN[2]= cid_tab[10];
	/*!< Byte 11 */
	SD_cid->ProdSN[1]= cid_tab[11];
	/*!< Byte 12 */
	SD_cid->ProdSN[0]= cid_tab[12];
	/*!< Byte 13 */
	SD_cid->Reserved1 = ((cid_tab[13] & 0xF0) >> 4);
	SD_cid->ManufactDate = (cid_tab[13] & 0x0F);
	SD_cid->ManufactDate =(SD_cid->ManufactDate<<8)+cid_tab[14];
	/*!< Byte 15 */
	SD_cid->CID_CRC = ((cid_tab[15] & 0xFE) >> 1);
	SD_cid->Reserved2 = (cid_tab[15] & 0x01);
	/*!< Return the reponse */
	#endif
	return 0;
}

/*
 * @brief  Returns information about specific card.
 * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD
 *         card information.
 * @retval The SD Response:
 *         - 0xFF: Sequence failed
 *         - 0: Sequence succeed
 */
mlu8 sd_get_cardinfo(SD_CardInfo *pCardinfo)
{
	if (cssd_csdregister_read(&(pCardinfo->SD_csd)))
		return 0xFF;
	if (cssd_cidregister_read(&(pCardinfo->SD_cid)))
		return 0xFF;
	pCardinfo->CardCapacity = (pCardinfo->SD_csd.DeviceSize + 1) * 1024;
	pCardinfo->CardBlockSize = 1 << (pCardinfo->SD_csd.RdBlockLen);
	pCardinfo->CardCapacity *= pCardinfo->CardBlockSize;
	/*!< Returns the reponse */
	return 0;
}

/*
 * @brief  Initializes the SD/SD communication.
 * @param  None
 * @retval The SD Response:
 *         - 0xFF: Sequence failed
 *         - 0: Sequence succeed
 */
mlu8 csssd_card_init(void)
{
	mlu8 frame[10], index, result;

	cssd_spi_low_speed_set();

	/*!< Send dummy byte 0xFF, 10 times with CS high */
	/*!< Rise CS and MOSI for 80 clocks cycles */
	/*!< Send dummy byte 0xFF */
	for (index = 0; index < 10; index++)
		frame[index] = 0xFF;
	for (index = 0; index < 10; index++)
		cssd_spi_write(frame, 10);
	/*------------Put SD in SPI mode--------------*/
	/*!< SD initialized and set to SPI mode properly */

    index = 0xFF;
    while (index--) {
		cssd_cs_low();
        result=cssd_cmd_send(SD_CMD0, 0, 0x95);
		cssd_cs_high();
        if (result == 0x01)
            break;
    }
    if (index == 0)
    {
        log_print(LOG_LVL_INFO,"SD_CMD0 is %X\n", result);
        return 0xFF;
    }

#if 1
	cssd_cs_low();
	result=cssd_cmd_send(SD_CMD8, 0x01AA, 0x87);
	cssd_spi_read(frame, 4);	//CMD8命令后会传回4字节的数据，要跳过再结束本命令
	//buff[0] = SPI_ReadWriteByte(0xFF); //should be 0x00
	//buff[1] = SPI_ReadWriteByte(0xFF); //should be 0x00
	//buff[2] = SPI_ReadWriteByte(0xFF); //should be 0x01
	//buff[3] = SPI_ReadWriteByte(0xFF); //should be 0xAA
	cssd_cs_high();
	if (result != 0x01) /*!< 0x01 or 0x05 */
	{
		//=0x05,标识 v1.0版本的卡，
        log_print(LOG_LVL_INFO,"SD_CMD8 is %X\n", result);
		return 0xFF;
    }
	//(result == 0x01) 标识 V2.0的卡，

	
	index = 0xFF;
	while (index--) {
		cssd_cs_low();
		result=cssd_cmd_send(SD_CMD55, 0, 0);
		cssd_cs_high();
		if (result != 0x01)
			return 0xFF;
		cssd_cs_low();
		result =cssd_cmd_send(SD_ACMD41, 0x40000000, 0);
		cssd_cs_high();
		if (result == 0x00)
			break;
	}
	if (index == 0)
	{
        log_print(LOG_LVL_INFO,"SD_CMD55 is %X\n", result);
		return 0xFF;
    }
	index = 255;
	while(index--){
		cssd_cs_low();
		result =cssd_cmd_send(SD_CMD58, 0, 1);
		cssd_spi_read(frame, 4);
		cssd_cs_high();
		if(result == 0){
			break;
		}
	}
	if(index == 0)
	{
	    log_print(LOG_LVL_INFO,"SD_CMD58 is %X\n", result);
		return 0xFF;
	}
	//if ((frame[0] & 0x40) == 0)
	//	return 0xFF;

	cssd_spi_high_speed_set();
	//for (index = 0; index < 100; index++)
		//cssd_spi_write_byte(0xff);
	return sd_get_cardinfo(&cardinfo);
#endif
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
Taskstate cssd_task(void*args)
{
	task_declaration();									//任务声明
	mlu16 i;

	task_begin(cssd.pMytask);							//任务开始
	
	while (mltrue)
	{
		task_ms_delay(cssd.pMytask, 300, TASK_STA_NONBLOCKING_YIELD);


		cssd_read_sector(cssd.rbuf,cssd.sector,1,512);

		for (i = 0; i < CSSD_SECTOR_SIZE; ++i)
		{
			cssd.wbuf[i]=mlos_ms_clock();
		}

		cssd_write_sector(cssd.wbuf,cssd.sector,1,512);
		

		cssd_read_sector(cssd.rbuf,cssd.sector,1,512);

		for (i = 0; i < CSSD_SECTOR_SIZE; ++i)
		{
			if(cssd.rbuf[i]!=cssd.wbuf[i])
			{
				log_print(LOG_LVL_INFO,"sd rw cmp : sector-%d-byte-%d : rbyte=%d != wbyte=%d\n", cssd.sector,i,
					cssd.rbuf[i],cssd.wbuf[i]);
				break;
			}
		}
		
		cssd.sector++;
		if (cssd.sector>=CSSD_SECTOR_MAX)
		{
			cssd.sector=1;
		}
	}

	task_end(cssd.pMytask); 							//任务结束
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
void cssd_driver_uninstall (void)
{

	cssd_spi_clean_up();

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
void cssd_init(void)
{

	cssd_spi_init();


	csssd_card_init();
	
	//cssd.pMytask=task_create(TASK_PRIO_6, cssd_task,nullptr,ltimer_create(),"cssd");

	cssd.sector=1;
	
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 





