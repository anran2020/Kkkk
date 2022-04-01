/**
 ******************************************************************************
 * 文件:combine_ctp.c
 * 作者:   
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *				
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1        
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if COMBINE_ENABLE


//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "mlos.h"
#include "ctp.h"
#include "combine_bl.h"
#include "combine_ctp.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------




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
//
//
//-----------------------------------------------------------------------------
MLBool cmbctp_box_auto_connect(void *args)
{
	//联机信息，
	CANFrmptr prxfrm;
	CmbPwrMdlptr pBox;
	
	prxfrm=(CANFrmptr)args;

	pBox=pwrbox_create(prxfrm->mfRxIDbit.CANID,prxfrm->mfRxIDbit.ADDR,prxfrm->mfRxIDbit.);
	cbl_power_box_register(pBox);//等级电源箱

	//回复下位机联机成功
	CANFrmptr ptxfrm;
	ptxfrm=ctp_txframe_malloc((CANCardID)prxfrm->mfRxIDbit.CANID);
	if(ptxfrm!=nullptr)
	{
		//填充 搜索设备报文
		can_sfid_set(ptxfrm, prxfrm->mfRxIDbit.ADDR, CTP_MSG_iapRerun,rerunflag, subAddr, devt);
		ptxfrm->dataH=0;
		ptxfrm->dataL=0;
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
//
//
//-----------------------------------------------------------------------------
void cbl_ctp_init(void)
{

	



}


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif







