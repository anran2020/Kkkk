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
//global variable
//----------------------------------------------------------------------------

//托盘数据缓存队列
Queptr pTrayQue=nullptr;

//托盘数据缓存区,一个托盘只缓存一条数据
TrayDiskCache trayDiskCache[DS_TRAY_NUM];		


//托盘数据文件
DSFilePtr trayFile[DS_TRAY_NUM];



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
mlu8* ds_tray_disk_chache(mlu8 traAddr)
{
	
	if(traAddr>=DS_TRAY_NUM||trayDiskCache[traAddr].isEmpty==0)
	{

		return nullptr;
	}

	trayDiskCache[traAddr].isEmpty=0;
	trayDiskCache[traAddr].curWriteDiskPage=0;

	return trayDiskCache[traAddr].buf;
	
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

	//将缓存数据写磁盘
	for (i = 0; i < DS_TRAY_NUM; ++i)
	{

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
	mlu32 i;
	//
	if(trayNum>DS_TRAY_MAX)
	{
		trayNum=DS_TRAY_MAX;//限制托盘最大数	
	}

	//计算每个托盘，需要的flash页数
	mlu32 traypageCnt
	for (i = 0; i < trayNum; ++i)
	{

		trayFile[i]=	mlos_malloc(e_mem_sram, sizeof(DSFile));		//分配内存
		trayFile[i]->bOpened=mlfalse;//文件未打开	
		trayFile[i]->basePage=DS_TRAY_FILE_BASE_PAGE+i*(DS_TRAY_PAGE_COUNT+DS_FILE_BACKUP_PAGE_NUM);
		trayFile[i]->endPage=trayFile[i]->basePage+DS_LOG_PAGE_OUNT+DS_FILE_BACKUP_PAGE_NUM;				//文件数据基地址
		trayFile[i]->pagesPerLine=DS_TRAY_PAGE_PER_LINE;
		trayFile[i]->linesPerPage=0;
		trayFile[i]->linePageIndx=0;
		trayFile[i]->lineMax=DS_TRAY_LINE_MAX;
		dsf_open(trayFile[i],DS_TRAY_FILE_VERSION);			//打开创建文件


		//缓存初始化
		trayDiskCache[i].curWriteDiskPage=0;
		trayDiskCache[i].isEmpty=1;
	}
	
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif 






