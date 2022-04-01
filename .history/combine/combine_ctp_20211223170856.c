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
#include "combine_channel.h"
#include "combine_module.h"
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
//电源箱，自主联机的应答
//
//修改履历：
//
//
//-----------------------------------------------------------------------------
MLBool pwrbox_auto_connect(void *args)
{
	//联机信息，
	CANFrmptr prxfrm;
	CmbPwrBoxptr pBox;
	
// 1	PF	1bit	0	帧格式，单帧										
// 2	ADDR	5bit	x	设备地址，										
// 3	IDN	5bit	CTP_MSG_autoConnect	报文ID，下位机自动联机报文										
// 4	PP	2bit	0	报文参数，未使用，后续可扩展										
// 5	DATA10	1byte	x	设备类型，										
// 6	DATA9	1byte	x	软件版本，										
// 7	DATA8	1byte	x	通道数，										
// 8	DATA7	1byte	x	模块数，										
// 9	DATA6	1byte	0	RSV，										
// 10	DATA5	1byte	0	RSV，										
// 11	DATA4-3	2byte	x	电压量程，上传通道的电压量程，单位V										
// 12	DATA2-1	2byte	x	电流量程，上传通道的电流量程，单位A										

	prxfrm=(CANFrmptr)args;

	pBox=pwrbox_create(prxfrm->mfRxIDbit.CANID,prxfrm->mfRxIDbit.ADDR,prxfrm->u8Data8);
	cbl_power_box_register(pBox);//等级电源箱
	pBox->type=prxfrm->sfRxIDbit.DATA10;//设备类型
	pBox->hdwMdlNum=prxfrm->u8Data7;
	pBox->voltageRange=prxfrm->u16Data2*1000;
	pBox->voltageRange=prxfrm->u16Data1*1000;
	pBox->swv=prxfrm->sfRxIDbit.DATA9;
	
	//回复下位机联机成功
	CANFrmptr ptxfrm;
// 1	PF	1bit	0	帧格式，单帧				
// 2	ADDR	5bit	x	设备地址，				
// 3	IDN	5bit	CTP_MSG_ack	报文ID，应答报文				
// 4	PP	2bit	0	报文参数，				
// 5	DATA10	1byte	CTP_MSG_autoConnect	应答报文的ID，应答下位机联机报文				
// 6	DATA9	1byte	x	错误信息，=0标识联机成功，>=1标识联机错误				
// 7	DATA8	1byte	0	RSV，				
// 8	DATA7	1byte	0	RSV，				
// 9	DATA6	1byte	0	RSV，				
// 10	DATA5	1byte	0	RSV，				
// 11	DATA4-1	4byte	0	RSV，				

	ptxfrm=ctp_txframe_malloc((CANCardID)prxfrm->mfRxIDbit.CANID);
	if(ptxfrm!=nullptr)
	{
		//填充 搜索设备报文
		can_sfid_set(ptxfrm, prxfrm->mfRxIDbit.ADDR, CTP_MSG_ack,0, 0, CTP_MSG_autoConnect);
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
//电源箱，采样数据处理
//
//修改履历：
//
//
//-----------------------------------------------------------------------------
MLBool pwrbox_sample_data(void *args)
{
	//联机信息，
	CANFrmptr prxfrm;
	CmbPwrBoxptr pBox;
	CMbMdlPtr pmdl;
//报文内容	
// 1	PF		1bit	0											帧格式，单帧				
// 2	ADDR	5bit	x										设备地址，				
// 3	IDN		5bit	CTP_MSG_cmbSample						报文ID，应答报文				
// 4	PP		2bit	0										报文参数，=0单通道采样电流电压;>=1后续扩展使用				
// 5	DATA10	1byte	x						"通道状态[bit8-5]通道号[bit1-4]， 通道状态：=0空闲;=1启动，=2工作,=3停止,=4故障"			
// 6	DATA9	1byte	x						DDC，数据描述码				
// 7	DATA8-5	4byte	x						电压，通道的电压采样数据，单位0.01mV				
// 8	DATA4-1	4byte	x						电流，通道的电流采样数据，单位0.01mA				
								
	prxfrm=(CANFrmptr)args;//转换消息

	pBox=cbl_power_box_get(prxfrm->mfRxIDbit.ADDR);//获取数据对应的电源箱
	if(pBox==nullptr)
	return mltrue;

	//取数据
	mlu8 indx=(prxfrm->sfRxIDbit.DATA10&0x0f);
	(pBox->mdl[indx])->DDC=;
	pBox=pwrbox_create(prxfrm->mfRxIDbit.CANID,prxfrm->mfRxIDbit.ADDR,prxfrm->u8Data8);
	cbl_power_box_register(pBox);//等级电源箱
	pBox->type=prxfrm->sfRxIDbit.DATA10;//设备类型
	pBox->hdwMdlNum=prxfrm->u8Data7;
	pBox->voltageRange=prxfrm->u16Data2*1000;
	pBox->voltageRange=prxfrm->u16Data1*1000;
	pBox->swv=prxfrm->sfRxIDbit.DATA9;
	
	//回复下位机联机成功
	CANFrmptr ptxfrm;
// 1	PF	1bit	0	帧格式，单帧				
// 2	ADDR	5bit	x	设备地址，				
// 3	IDN	5bit	CTP_MSG_ack	报文ID，应答报文				
// 4	PP	2bit	0	报文参数，				
// 5	DATA10	1byte	CTP_MSG_autoConnect	应答报文的ID，应答下位机联机报文				
// 6	DATA9	1byte	x	错误信息，=0标识联机成功，>=1标识联机错误				
// 7	DATA8	1byte	0	RSV，				
// 8	DATA7	1byte	0	RSV，				
// 9	DATA6	1byte	0	RSV，				
// 10	DATA5	1byte	0	RSV，				
// 11	DATA4-1	4byte	0	RSV，				

	ptxfrm=ctp_txframe_malloc((CANCardID)prxfrm->mfRxIDbit.CANID);
	if(ptxfrm!=nullptr)
	{
		//填充 搜索设备报文
		can_sfid_set(ptxfrm, prxfrm->mfRxIDbit.ADDR, CTP_MSG_ack,0, 0, CTP_MSG_autoConnect);
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
//电源箱，应答报文
//
//修改履历：
//
//
//-----------------------------------------------------------------------------
MLBool pwrbox_ack(void *args)
{
	//联机信息，
	CANFrmptr prxfrm;
	CmbPwrBoxptr pBox;
	
// 1	PF	1bit	0	帧格式，单帧										
// 2	ADDR	5bit	x	设备地址，										
// 3	IDN	5bit	CTP_MSG_autoConnect	报文ID，下位机自动联机报文										
// 4	PP	2bit	0	报文参数，未使用，后续可扩展										
// 5	DATA10	1byte	x	设备类型，										
// 6	DATA9	1byte	x	软件版本，										
// 7	DATA8	1byte	x	通道数，										
// 8	DATA7	1byte	x	模块数，										
// 9	DATA6	1byte	0	RSV，										
// 10	DATA5	1byte	0	RSV，										
// 11	DATA4-3	2byte	x	电压量程，上传通道的电压量程，单位V										
// 12	DATA2-1	2byte	x	电流量程，上传通道的电流量程，单位A										

	prxfrm=(CANFrmptr)args;

	pBox=pwrbox_create(prxfrm->mfRxIDbit.CANID,prxfrm->mfRxIDbit.ADDR,prxfrm->u8Data8);
	cbl_power_box_register(pBox);//等级电源箱
	pBox->type=prxfrm->sfRxIDbit.DATA10;//设备类型
	pBox->hdwMdlNum=prxfrm->u8Data7;
	pBox->voltageRange=prxfrm->u16Data2*1000;
	pBox->voltageRange=prxfrm->u16Data1*1000;
	pBox->swv=prxfrm->sfRxIDbit.DATA9;
	
	//回复下位机联机成功
	CANFrmptr ptxfrm;
// 1	PF	1bit	0	帧格式，单帧				
// 2	ADDR	5bit	x	设备地址，				
// 3	IDN	5bit	CTP_MSG_ack	报文ID，应答报文				
// 4	PP	2bit	0	报文参数，				
// 5	DATA10	1byte	CTP_MSG_autoConnect	应答报文的ID，应答下位机联机报文				
// 6	DATA9	1byte	x	错误信息，=0标识联机成功，>=1标识联机错误				
// 7	DATA8	1byte	0	RSV，				
// 8	DATA7	1byte	0	RSV，				
// 9	DATA6	1byte	0	RSV，				
// 10	DATA5	1byte	0	RSV，				
// 11	DATA4-1	4byte	0	RSV，				

	ptxfrm=ctp_txframe_malloc((CANCardID)prxfrm->mfRxIDbit.CANID);
	if(ptxfrm!=nullptr)
	{
		//填充 搜索设备报文
		can_sfid_set(ptxfrm, prxfrm->mfRxIDbit.ADDR, CTP_MSG_ack,0, 0, CTP_MSG_autoConnect);
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

	ctp_consumer_register(CAN_CARD_1,CTP_MSG_ack,pwrbox_ack);	//来自lc应答消息
	ctp_consumer_register(CAN_CARD_1,CTP_MSG_autoConnect,pwrbox_auto_connect);//来自lc的自主联机消息
	ctp_consumer_register(CAN_CARD_1,CTP_MSG_autoConnect,pwrbox_sample_data);//lc采样回复
}


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif







