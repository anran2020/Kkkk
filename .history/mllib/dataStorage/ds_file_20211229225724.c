/**
 ******************************************************************************
 * 文件:ds_file.c
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
#include "ds_config.h"

//---------------------------------------------------------
//文件条件编译
//---------------------------------------------------------

#if  (DS_EN)

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------
#include<string.h>
#include "mlos.h"

#include "ds_file.h"
#include "cs_sdcard.h"


//---------------------------------------------------------
//define
//---------------------------------------------------------




//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//global variable
//----------------------------------------------------------------------------

//文件page 缓存
mlu8 dsfPageBuf[DS_FLASH_PAGE_SIZE];



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
MLBool dsf_backup(DSFilePtr dsfile)
{

	//备份文件头
	dsfile->headerBackupAB++;
	if(dsfile->headerBackupAB>=DS_FILE_BACKUP_PAGE_NUM)
	{
		dsfile->headerBackupAB=DS_FILE_BACKUP_A;
	}

	dsfile->header.modifyTimes++;
	if(mlfalse==cssd_write_sector(&dsfile->header.flag1,  dsfile->headerBackupAB+dsfile->basePage, 1, DS_FLASH_PAGE_SIZE))
	{
		dsfile->header.modifyTimes--;
		MLOS_BKPT(mltrue);					//备份失败	
		return mlfalse;
	}

	return mltrue;
	
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
MLBool dsf_write_line(DSFilePtr dsfile,mlu32 line,mlu8* pdat)
{
	if(!dsfile->bOpened)
	{
		return mlfalse;
	}

	mlu8 i;
	mlu32 pageAddr=dsfile->pagesPerLine;
	pageAddr=dsfile->basePage+DS_FILE_BACKUP_PAGE_NUM+line*pageAddr;//计算 页地址

	//一行多页的写入
	for ( i = 0; i < dsfile->pagesPerLine; i++)
	{
		if(mlfalse==cssd_write_sector(pdat, pageAddr, 1, DS_FLASH_PAGE_SIZE))
		{
			//写入失败
			return mlfalse;
		}
			//页偏移
		pageAddr++;
		if(pageAddr>=dsfile->endPage)			//循环覆盖写入日志
		{
			pageAddr=dsfile->basePage+DS_FILE_BACKUP_PAGE_NUM; 	//偏移用来备份的2page
		}

	}
	
	//页偏移
	dsfile->curDataPage++;
	if(dsfile->curDataPage>=dsfile->endPage)			//循环覆盖写入日志
	{
		dsfile->curDataPage=dsfile->basePage+DS_FILE_BACKUP_PAGE_NUM; 	//偏移用来备份的2page
	}

	//文件行维护，文件写入行，发生变化
	dsfile->header.offsetLine=line+1;		//文件尾，即下一个写入行号
	if(dsfile->header.offsetLine>=dsfile->lineMax)
	{
		dsfile->header.coveredWriting=1;
		dsfile->header.offsetLine=0;				//循环覆盖写入日志	
	}

	//文件行号改变，备份文件信息
	dsfile->headerBackupAB++;
	if(dsfile->headerBackupAB>=DS_FILE_BACKUP_PAGE_NUM)
	{
		dsfile->headerBackupAB=DS_FILE_BACKUP_A;
	}
	dsfile->header.modifyTimes++;
	if(mlfalse==cssd_write_sector(&dsfile->header.flag1,  dsfile->headerBackupAB+dsfile->basePage, 1, DS_FLASH_PAGE_SIZE))
	{
		dsfile->header.modifyTimes--;
		MLOS_BKPT(mltrue);					//备份失败	
		return mlfalse;
	}

	//打印日志
	log_print(LOG_LVL_INFO,"file[%d] w line %d page %",dsfile->hdl,line);
	return mltrue;
	
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
MLBool dsf_write_page(DSFilePtr dsfile,mlu8* pdat)
{
	if(!dsfile->bOpened)
	{
		return mlfalse;
	}
		
	if(mlfalse==cssd_write_sector(pdat, dsfile->curDataPage, 1, DS_FLASH_PAGE_SIZE))
	{
		//写入失败
		return mlfalse;
	}

	//页偏移
	dsfile->curDataPage++;
	if(dsfile->curDataPage>=dsfile->endPage)			//循环覆盖写入日志
	{
		dsfile->curDataPage=dsfile->basePage+DS_FILE_BACKUP_PAGE_NUM; 	//偏移用来备份的2page
	}

	//文件行维护，文件写入行，发生变化
	MLBool bLineChanged=mlfalse;
	
	if (dsfile->linesPerPage)
	{
		//line < page,1页有多行
		dsfile->header.offsetLine+=dsfile->linesPerPage;
		bLineChanged=mltrue;
	}
	else
	{
		//line > page，多页为一行
		dsfile->linePageIndx++;
		if(dsfile->linePageIndx>=dsfile->pagesPerLine)
		{
			dsfile->header.offsetLine++;		//一行写完，行号递增
			mlu32 line=dsfile->header.offsetLine;
			log_print(LOG_LVL_INFO,"tray file w  %d line ",line);
			dsfile->linePageIndx=0;			//返回行的0页开始写入	
			bLineChanged=mltrue;	
		}
	}
	
	if(dsfile->header.offsetLine>=dsfile->lineMax)
	{
		dsfile->header.coveredWriting=1;
		dsfile->header.offsetLine=0;				//循环覆盖写入日志	
	}

	//文件行号改变，备份文件信息
	if(bLineChanged)
	{
		dsfile->headerBackupAB++;
		if(dsfile->headerBackupAB>=DS_FILE_BACKUP_PAGE_NUM)
		{
			dsfile->headerBackupAB=DS_FILE_BACKUP_A;
		}
		
		dsfile->header.modifyTimes++;
		if(mlfalse==cssd_write_sector(&dsfile->header.flag1,  dsfile->headerBackupAB+dsfile->basePage, 1, DS_FLASH_PAGE_SIZE))
		{
			dsfile->header.modifyTimes--;
			MLOS_BKPT(mltrue);					//备份失败	
			return mlfalse;
		}
	}

	return mltrue;
	
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
mlu8* dsf_read_page(DSFilePtr dsfile,mlu32 pageAddr)
{

	if(mlfalse==cssd_read_sector(dsfPageBuf, pageAddr, 1, DS_FLASH_PAGE_SIZE))
	{
		//写入失败
		return nullptr;
	}

		
	return dsfPageBuf;
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
MLBool dsf_clear(DSFilePtr dsfile)
{
	
	memset(dsfPageBuf,0xFF,DS_FLASH_PAGE_SIZE);
	if(mlfalse==cssd_write_sector(dsfPageBuf, dsfile->basePage, 1, DS_FLASH_PAGE_SIZE))
	{
		//写入失败
		return mlfalse;
	}
	
	if(mlfalse==cssd_write_sector(dsfPageBuf, dsfile->basePage+1, 1, DS_FLASH_PAGE_SIZE))
	{
		//写入失败
		return mlfalse;
	}


	//从新部署文件
	dsfile->header.flag1=DS_FILE_HEADER_FLAG1;
	dsfile->header.coveredWriting=0;
	dsfile->header.modifyTimes=0;
	dsfile->header.offsetLine=0;
	dsfile->header.lastReadLine=0;
	dsfile->headerBackupAB=DS_FILE_BACKUP_A;
	dsfile->curDataPage=dsfile->basePage+DS_FILE_BACKUP_PAGE_NUM;	//偏移2page，为文件起始页
	return mltrue;
	
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
MLBool dsf_exist(DSFileHeaderPtr dsfHeader,DSFileVersion ver)
{

	if(dsfHeader->flag1==DS_FILE_HEADER_FLAG1&&dsfHeader->version==ver&&
		dsfHeader->flag2==DS_FILE_HEADER_FLAG2)
	{

		return mltrue;
	}
		
	return mlfalse;
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
MLBool dsf_open(DSFilePtr dsfile,DSFileVersion ver)
{
		
	//文件头 A,B区备份
	DSFileHeader headerA;
	if(mlfalse==cssd_read_sector(dsfPageBuf, dsfile->basePage, 1,DS_FLASH_PAGE_SIZE))
	{
		//读取失败
		MLOS_BKPT(mltrue);
		return mlfalse;
	}
	memcpy(&headerA.flag1,dsfPageBuf,sizeof(DSFileHeader));

	DSFileHeader headerB;
	if(mlfalse==cssd_read_sector(dsfPageBuf, dsfile->basePage+1, 1,DS_FLASH_PAGE_SIZE))
	{
		//读取失败
		MLOS_BKPT(mltrue);
		return mlfalse;
	}
	memcpy(&headerB.flag1,dsfPageBuf,sizeof(DSFileHeader));

	//判断文件是否存在
	MLBool bHeaderAexisted=dsf_exist(&headerA,ver);
	MLBool bHeaderBexisted=dsf_exist(&headerB,ver);
	
	DSFileHeaderPtr headerptr;
	if(bHeaderAexisted==mlfalse&&bHeaderBexisted==mlfalse)
	{
		//从新部署文件
		headerA.flag1=DS_FILE_HEADER_FLAG1;
		headerA.coveredWriting=0;
		headerA.version=ver;
		headerA.modifyTimes=0;
		headerA.offsetLine=0;
		headerA.lastReadLine=0;
		headerA.flag2=DS_FILE_HEADER_FLAG2;
		headerptr=&headerA;
		dsfile->headerBackupAB=DS_FILE_BACKUP_A;
		
	}
	else if(bHeaderAexisted==mltrue&&bHeaderBexisted==mltrue)
	{
		//取最后一次写入的文件头信息
		if((headerA.modifyTimes-headerB.modifyTimes)==1)
		{
			headerptr=&headerA;
			dsfile->headerBackupAB=DS_FILE_BACKUP_A;
		}
		else if((headerB.modifyTimes-headerA.modifyTimes)==1)
		{
			headerptr=&headerB;
			dsfile->headerBackupAB=DS_FILE_BACKUP_B;
		}
		else
		{
			//文件头信息错误
			headerptr=&headerA;
			dsfile->headerBackupAB=DS_FILE_BACKUP_A;
		}
	}
	else if(bHeaderAexisted==mltrue)
	{
		headerptr=&headerA;
		dsfile->headerBackupAB=DS_FILE_BACKUP_A;
	}
	else //if(bHeaderBexisted==mltrue)
	{
		headerptr=&headerB;
		dsfile->headerBackupAB=DS_FILE_BACKUP_B;
	}
	
	//
	memcpy(&dsfile->header.flag1,&headerptr->flag1,sizeof(DSFileHeader));			//取出文件头的内容

	//文件当前写入页地址
	if(dsfile->linesPerPage)
	{
		//多行组成一页的文件，一行数据<一页
		dsfile->curDataPage=dsfile->basePage+DS_FILE_BACKUP_PAGE_NUM+dsfile->header.offsetLine/dsfile->linesPerPage;
	}
	else
	{
		//多页组成一行的文件，一行数据>一页
		dsfile->curDataPage=dsfile->basePage+DS_FILE_BACKUP_PAGE_NUM+dsfile->header.offsetLine*dsfile->linesPerPage;
	}
	
	if(dsfile->curDataPage>=dsfile->endPage)
	{
		MLOS_BKPT(mltrue);
		dsfile->curDataPage=dsfile->basePage+DS_FILE_BACKUP_PAGE_NUM;	//超出文件大小
	}

	
	dsfile->bOpened=1;					//文件创建成功
	return mltrue;

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
void dsf_close(DSFilePtr dsfile)
{

	dsfile->bOpened=0;	

}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 
