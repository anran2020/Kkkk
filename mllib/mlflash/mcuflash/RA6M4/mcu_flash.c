/**
 ******************************************************************************
 * 文件:  mcu_flash.c  
 * 作者:   
 * 版本:   V0.01
 * 日期:    
 * 内容简介:
 * 
 * 
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   ----   作者 --------   说明
 * v0.1    2021-10-07     djx        创建该文件
 *
 *
 *
 ******************************************************************************
 **/
 
//----------------------------------------------------------------------------
//文件条件编译
//----------------------------------------------------------------------------
#if (1)

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------

#include "mcu_flash.h"
#include "hal_data.h"
#include "r_flash_hp.h"
#include "mlos.h"

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

#define MCU_FLASH_EVENT_NULL				(0xffffffff)
#define MCU_FLASH_EVENT_TIMEOUT 			(0xffffffff) 					//等待事件超时

//----------------------------------------------------------------------------
//local global  variable
//----------------------------------------------------------------------------

mlvu32  mcuFlashEvent =0;

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
//1.djx   2021-10-12  ：创建函数
//
//-----------------------------------------------------------------------------
MLBool mcu_flash_erase(mlu32 startAddr, mlu32 blockNum)
{
	mlu8 isCodeFlash = 0 ;
	fsp_err_t err = 0 ;
	mlu32 tm=mlos_ms_clock();

	__disable_irq() ;
	mcuFlashEvent=MCU_FLASH_EVENT_NULL;
    err = R_FLASH_HP_Erase(&g_mcuflash_ctrl, startAddr, blockNum);
	 __enable_irq();
	 
    if( FSP_SUCCESS != err )
	{
	    return mlfalse ;
    }
#if 0
    //等待写入完成
    while( mcuFlashEvent==MCU_FLASH_EVENT_NULL )
    {
        if((mlos_ms_clock()-tm)>3000)
        {
            break;
        }
    }

	if(mcuFlashEvent!=FLASH_EVENT_ERASE_COMPLETE)
	{
		return mlfalse;
	}
	
#endif

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
//1.djx ：  2021-10-12  ：创建函数
//
//-----------------------------------------------------------------------------
/**
 * @brief  往flash写数据
 * @param  flashType        写入flash类型
 *          @arg DATA_FLASH_OPERATIONS          data flash
 *          @arg CODE_FLASH_OPERATIONS          code flash
 * @param  blockNum         内存块号
 *          @note data flash -> 0-7，1K为一块，  code flash -> 0-127，2K为一块，该参数决定起始地址
 * @param  pData            数据指针
 * @param  size             数据长度，单位：字节
 * @return 写入成功返回SUCCESS，否则返回ERROR
 * @note 该函数仅支持以块为单位的写操作
 */
MLBool mcu_flash_write(mlu32 address, mlu32 *pData, mlu32 size)
{

	fsp_err_t err = 0 ;
	mlu32 tm=mlos_ms_clock();


	__disable_irq() ;
	mcuFlashEvent=MCU_FLASH_EVENT_NULL;
	err = R_FLASH_HP_Write(&g_mcuflash_ctrl, (mlu32) pData, address, size);
	  __enable_irq();
	  
    if( FSP_SUCCESS != err )
	{
    	return mlfalse ;
    }
	
  
		
#if 0
	//等待写入完成
	while( mcuFlashEvent==MCU_FLASH_EVENT_NULL )
	{
		if((mlos_ms_clock()-tm)>3000)
		{
			break;
		}
	}
	
	if(mcuFlashEvent!=FLASH_EVENT_WRITE_COMPLETE)
	{
		return mlfalse;
	}

#endif
	//读取校验
	mlu32 i;
	size=size/4;
	mlu32* u32ptr=(mlu32*)address;
	for (i = 0; i < size; ++i)
	{
		if((*u32ptr)!=pData[i])
		{
			return mlfalse;
		}
		u32ptr++;
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
//1.djx ：  2021-10-12  ：创建函数
//
//-----------------------------------------------------------------------------
void mcu_flash_read(mlu32 address, mlu8 *pData, mlu32 size)
{

    memcpy(pData, (mlu8 *)address, size) ;

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
//1.djx ：  2021-10-12  ：创建函数
//
//-----------------------------------------------------------------------------

void mcu_flash_callback(flash_callback_args_t * p_args)
{

	//mcuFlashEvent=p_args->event;

    switch(p_args->event)
    {
        case FLASH_EVENT_NOT_BLANK:
            mcuFlashEvent=FLASH_EVENT_NOT_BLANK;
            break;
        case FLASH_EVENT_BLANK:
             mcuFlashEvent=FLASH_EVENT_BLANK ;
            break;
        case FLASH_EVENT_ERASE_COMPLETE:
            mcuFlashEvent=FLASH_EVENT_ERASE_COMPLETE;
            break;
        case FLASH_EVENT_WRITE_COMPLETE:
            mcuFlashEvent=FLASH_EVENT_WRITE_COMPLETE;
            break;
        default:
            break;
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
//1.djx ：  2021-10-12  ：创建函数
//
//-----------------------------------------------------------------------------
MLBool mcu_flash_init( void )
{
    fsp_err_t err = FSP_SUCCESS;     // Error status

    /* Open Flash_LP module */
    err = R_FLASH_HP_Open(&g_mcuflash_ctrl, &g_mcuflash_cfg);
    /* handle error */
    if (FSP_SUCCESS != err)
    {
        /* Flash_LP Failure message */
        //APP_ERR_PRINT("** R_FLASH_LP_Open API for Flash_LP failed ** \r\n");

        return mlfalse ;
    }
	
    /* Setup Default  Block 0 as Startup Setup Block */
    err = R_FLASH_HP_StartUpAreaSelect(&g_mcuflash_ctrl, FLASH_STARTUP_AREA_BLOCK0, mlfalse);
    if (err != FSP_SUCCESS)
    {
     	return mlfalse;
    }

    return mltrue ;
}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif
