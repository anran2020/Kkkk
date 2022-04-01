/**
 ******************************************************************************
 * 文件:ds_tray.c
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

#if  (DS_TRAY_EN&&DS_EN)

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "mlos.h"
#include "ds_tray.h"
#include "ds_file.h"




//---------------------------------------------------------
//define
//---------------------------------------------------------

//---------------------------------------------------------
//define
//---------------------------------------------------------


//----------------------------------------------------------------------------
//托盘一行数据的缓存
//----------------------------------------------------------------------------
typedef struct{

	mlu8 isEmpty;					//是否为空
	mlu8 curWriteDiskPage;			//当前写入页，一行数据大小为 x page
		
	//mlu8 buf[DS_TRAY_PAGE_PER_LINE*DS_FLASH_PAGE_SIZE];
	mlu8 *pDat;//数据缓存

}TrayDiskCache,*TrayDiskCachePtr;


//----------------------------------------------------------------------------
//global variable
//----------------------------------------------------------------------------

//托盘数据缓存队列
Queptr pTrayCacheQue[DS_TRAY_NUM]={nullptr};

//托盘数据缓存区,每个托盘缓存DS_TRAY_CACHE_NUM
TrayDiskCache trayDiskCache[DS_TRAY_NUM][DS_TRAY_CACHE_NUM];		


//托盘数据文件
DSFilePtr trayFile[DS_TRAY_NUM];

//一条托盘数据的大小
mlu32 trayBaseDataSize=0;
//托盘的个数
mlu8 trayUsedNum=0;

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
MLBool ds_tray_write_chache(mlu8 traAddr,mlu8 datNum,mlu8* const pData,mlu32 lastReadLine)
{
	mlu8* pU8Dat=pData;

	if(traAddr>=trayUsedNum)
	{
		return mlfalse;
	}

	trayFile[traAddr]->header.lastReadLine=lastReadLine;
	for (mlu8 i = 0; i < datNum; i++)
	{
		TrayDiskCachePtr pCache=que_en(pTrayCacheQue[traAddr]);	
		if(pCache==nullptr)
		{
			return mlfalse;
		}
		
		pCache->isEmpty=0;
		pCache->curWriteDiskPage=0;

		memcpy(pCache->pDat,pData,trayBaseDataSize);
		pU8Dat+=trayBaseDataSize;//偏移一条数据
	}
	


	return mltrue;
	
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : MLBool，返回mltrue标识文件已经循环覆盖写入
// 参数   :  
//fhdl，文件句柄，索引对应的文件
//curWriteLine，返回文件当前的写入行号
//
//功能描述：
//	文件移到末尾,获取当前写入序号，返回mltrue标识文件已经循环写入
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
MLBool ds_tray_file_seek_end(mlu8 traAddr,mlu32*curWriteLine,mlu32* lastReadLine)
{
	if(traAddr>=trayUsedNum)
	{
		return mlfalse;
	}

	*curWriteLine=trayFile[traAddr]->header.offsetLine;
	*lastReadLine=trayFile[traAddr]->header.lastReadLine;
	
	return trayFile[traAddr]->header.coveredWriting;
}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : mlu8,返回实际读取的行数
// 参数   :  void
//
//
//
//功能描述：
//	行读取，托盘数据文件
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
mlu8 ds_tray_file_read_lines(mlu8 traAddr, mlu32 startline,mlu8 lineNum,mlu8*outDatBuf)
{
	mlu8 line,p;
	mlu8* pReadBuf=nullptr;
	mlu32 readBytes=0;
	mlu32 pageValidBytes=0;
	mlu32 pageAddr=startline*trayFile[traAddr]->pagesPerLine;

	if(traAddr>=trayUsedNum)
	{
		return 0;
	}

	log_print(LOG_LVL_INFO,"tra file r line:startline=%d,lineNum",startline,lineNum)
	for ( line = 0; line < lineNum; line++)
	{
		//分页读取一行数据
		for(p=0;p<trayFile[traAddr]->pagesPerLine;++p)
		{
			pReadBuf=dsf_read_page(trayFile[traAddr],trayFile[traAddr]->basePage+DS_FILE_BACKUP_PAGE_NUM+pageAddr+p);			
			if(pReadBuf==nullptr)
			{
				return line;
			}

			//计算当前页读取的有效数据,拷贝到输出buf
			readBytes+=DS_FLASH_PAGE_SIZE;
			pageValidBytes=trayBaseDataSize-readBytes;
			if(pageValidBytes>DS_FLASH_PAGE_SIZE)
			{
				pageValidBytes=DS_FLASH_PAGE_SIZE;
			}
			memcpy(outDatBuf,pReadBuf,pageValidBytes);
			outDatBuf+=pageValidBytes;
		}
	}

	return line;
	
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
MLBool ds_tray_subtask(void)
{
	mlu8 i;
	mlu32 offset;
	TrayDiskCachePtr pCache;

	//将缓存数据写磁盘，一次写一页数据，
	for (i = 0; i < trayUsedNum; ++i)
	{
		pCache=que_take(pTrayCacheQue[i]);

		if (pCache==nullptr)
		{
			continue; 									//队列无缓存
		}
		
		offset=pCache->curWriteDiskPage;//计算缓存数据的偏移，即接着上次的数据写盘
		offset=offset*DS_FLASH_PAGE_SIZE;

		//一次写盘一页
		if(mlfalse==dsf_write_page(trayFile[i],pCache->pDat+offset))
		{
				//cssd_driver_uninstall();
				//cssd_init();
				MLOS_BKPT(mlfalse);
				return mltrue;				//写入失败
		}

		pCache->curWriteDiskPage++;
		if(pCache->curWriteDiskPage>=DS_TRAY_PAGE_PER_LINE)
		{
			//一条缓存数据写完
			pCache->curWriteDiskPage=0;
			pCache->isEmpty=1;
			que_de(pTrayCacheQue[i]);//出列缓存数据
		}

		return mltrue;
		
	}

	return mlfalse;//返回false，标识没有写磁盘的操作
	
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
void ds_tray_file_init(mlu8 trayNum,mlu8 version,mlu32 baseDatSize,mlu32 datNum)
{
	mlu32 i,j;
	//
	if(trayNum>DS_TRAY_NUM)
	{
		trayNum=DS_TRAY_NUM;//限制托盘最大数	
	}
	trayBaseDataSize=baseDatSize;
	trayUsedNum=trayNum;
	//文件一条数据，称为一行，文件按行读写
	//一行数据占多少页计算
	mlu32 pagesPerLine=((baseDatSize%DS_FLASH_PAGE_SIZE)>0)?(baseDatSize/DS_FLASH_PAGE_SIZE+1):(baseDatSize/DS_FLASH_PAGE_SIZE);
	mlu32 trayPageCnt=datNum*pagesPerLine;//计算每个托盘，需要的flash页数

	for (i = 0; i < trayNum; ++i)
	{

		trayFile[i]=	mlos_malloc(e_mem_sram, sizeof(DSFile));		//分配内存
		trayFile[i]->bOpened=mlfalse;//文件未打开	
		trayFile[i]->basePage=DS_TRAY_FILE_BASE_PAGE+i*(trayPageCnt+DS_FILE_BACKUP_PAGE_NUM);	//计算文件基地址页
		trayFile[i]->endPage=trayFile[i]->basePage+trayPageCnt+DS_FILE_BACKUP_PAGE_NUM;	//文件结束页地址
		trayFile[i]->pagesPerLine=pagesPerLine;//多少页每行，即一行数据有多少页
		trayFile[i]->linesPerPage=0;
		trayFile[i]->linePageIndx=0;//行写入页索引
		trayFile[i]->lineMax=datNum;//文件行数，即文件有多少行

		dsf_open(trayFile[i],version+1);			//打开创建文件，根据版本号，打开文件

		//缓存初始化
		for (j = 0; j < DS_TRAY_CACHE_NUM; j++)
		{
			trayDiskCache[i][j].curWriteDiskPage=0;
			trayDiskCache[i][j].isEmpty=1;
			trayDiskCache[i][j].pDat=mlos_malloc(e_mem_sram,baseDatSize);//为缓存的一条数据，分配内存
		}
		pTrayCacheQue[i]=que_create(e_mem_sram,DS_TRAY_CACHE_NUM,sizeof(TrayDiskCache),(mlu8*)&trayDiskCache[i][0]);//创建数据缓存队列

	}
	
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 






