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
Queptr pTrayCacheQue[DS_TRAY_MAX]={nullptr};

//托盘数据缓存区,每个托盘缓存DS_TRAY_CACHE_NUM
TrayDiskCache trayDiskCache[DS_TRAY_MAX][DS_TRAY_CACHE_NUM];		


//托盘数据文件
DSFilePtr trayFile[DS_TRAY_MAX];

//一条托盘数据的大小
mlu32 trayBaseDataSize=0;

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
MLBool ds_tray_write_chache(mlu8 traAddr,mlu8* pData)
{
	TrayDiskCachePtr pCache=que_en(pTrayCacheQue[traAddr]);	
	if(pCache==nullptr)
	{
		return mlfalse;
	}
	
	pCache->isEmpty=0;
	pCache->curWriteDiskPage=0;

	memcopy(pCache->pDat,pData,trayBaseDataSize);

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
MLBool ds_tray_read(void)
{




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
MLBool ds_tray_subtask(void)
{
	mlu8 i;
	mlu32 bufOffset;
	TrayDiskCachePtr pCache;

	//将缓存数据写磁盘
	for (i = 0; i < DS_TRAY_MAX; ++i)
	{
		pCache=
		if (trayDiskCache[i].isEmpty)
		{
			continue; 									//无日志要存储，直接返回
		}
		
		bufOffset=trayDiskCache[i].curWriteDiskPage;
		bufOffset=bufOffset*DS_FLASH_PAGE_SIZE;
		if(mlfalse==dsf_write_page(trayFile[i],trayDiskCache[i].buf+bufOffset))
		{
				//cssd_driver_uninstall();
				//cssd_init();
				MLOS_BKPT(mlfalse);
				return mltrue;				//写入失败
		}

		trayDiskCache[i].curWriteDiskPage++;
		if(trayDiskCache[i].curWriteDiskPage>=DS_TRAY_PAGE_PER_LINE)
		{
			trayDiskCache[i].curWriteDiskPage=0;
			trayDiskCache[i].isEmpty=1;
		}

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
void ds_tray_file_init(mlu8 trayNum,mlu8 version,mlu32 baseDatSize,mlu32 datNum)
{
	mlu32 i,j;
	//
	if(trayNum>DS_TRAY_MAX)
	{
		trayNum=DS_TRAY_MAX;//限制托盘最大数	
	}
	trayBaseDataSize=baseDatSize;

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

		dsf_open(trayFile[i],version);			//打开创建文件，根据版本号，打开文件

		//缓存初始化
		for (j = 0; j < DS_TRAY_CACHE_NUM; j++)
		{
			trayDiskCache[i][j].curWriteDiskPage=0;
			trayDiskCache[i][j].isEmpty=1;
			trayDiskCache[i][j].pDat=mlos_malloc(e_mem_sram,baseDatSize);//为缓存的一条数据，分配内存
		}
		pTrayCacheQue[i]=que_create(e_mem_sram,DS_TRAY_CACHE_NUM,sizeof(TrayDiskCache),trayDiskCache[i]);//创建数据缓存队列

	}
	
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 






