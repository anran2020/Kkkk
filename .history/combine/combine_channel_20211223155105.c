/**
 ******************************************************************************
 * 文件:combine_channel.c
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

#include "combine_channel.h"

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------

#if 0
//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//			遍历模块，执行模块的接口，
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool chnl_traverse_module( CmbPwrChnlptr  pCChnl, MLBool (*mdlAPI)(CmbPwrMdlptr))
{
	MLBool bRet=mltrue;
	CmbPwrMdlptr pMdl;
	pMdl=pCChnl->mdlList->head;
	while(pMdl!=nullptr)
	{
		if(mlfalse==mdlAPI(pMdl))
		{
			bRet=mlfalse;
		}
		pMdl=pMdl->pnext;
	}

	return bRet;
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
//---------------------------------------------------------------------------
void chnl_data_save(CmbPwrChnlptr pChnl)
{




}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
void chnl_sta_migrate(CmbPwrChnlptr pChnl,ChannelState gotosta)
{
	
	ChannelState nextSta;

	//
	nextSta=gotosta;

	//启动状态
	if(gotosta==e_ch_sta_starting) 
	{
		//启动状态
		//pChnl->DDC = DDC_Normal;
		//pChnl->flags.bit.middle=_set;
		
		
	}	
	else if(gotosta==e_ch_sta_fault)
	{
		;
	}
	else if(gotosta==e_ch_sta_idle)
	{
		;

	}
	else
	{
		;
	}

	//进入停止状态
	if(nextSta==e_ch_sta_stopping)
	{				
		//pChnl->flags.bit.middle=_set;
		//pChnl->flags.bit.running=_reset;
	}

	//取状态
	pChnl->sta=nextSta;

}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
void chnl_sample(CmbPwrChnlptr pChnl)
{
#if 1



#else
	float isum;
	Module *pMdl;


	if(pChnl->pCvModule==nullptr)
	{
		pChnl->pCvModule=pChnl->mdlListHead;
	}

	//m模块数据 是否更新
	if (pChnl->pCvModule->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	

	//采通道电压
	pChnl->voltage=pChnl->pCvModule->voltage;

	//在允许误差 范围内 修正 第二路电压 显示值
	if (calibrate.running==_set)
	{
		pChnl->voltage2=pChnl->pCvModule->voltage2;
		pChnl->portVoltage=pChnl->pCvModule->portVoltage;
	}
	else
	{
		pChnl->voltage2=pChnl->pCvModule->voltage2fake;
	    pChnl->portVoltage=pChnl->pCvModule->portVoltageFake;
	}
	
	
	//采通道电流值
	pMdl=pChnl->mdlListHead;
	isum=0;
	while(pMdl!=nullptr)
	{
		//取模块电流
		if (pChnl->flags.bit.running==_set)
		{
			if (pMdl->run==_set)
			{
				isum+=pMdl->current;
			}
		}
		//pChnl->current+=pMdl->current;
		pMdl->flags.bit.ivUpdated=_reset;
		pMdl=pMdl->pNext;
	}

#if	0
	//放电电流，换算成负数
	if(isum<0)
	{
		isum=0-isum;
	}
    if((pChnl->flags.bit.running == _reset) || (pChnl->stepType == PST_QL) || (pChnl->sta == e_sta_npWaiting))
    {
        //屏蔽 虚电流
        if(isum<=1000)
        {
            isum = 0;
        }
    }
#else
	if(pChnl->flags.bit.running == _reset)
	{
		//屏蔽 虚电流
		isum = 0;
	}
	else
	{
		if((pChnl->stepType == PST_OCV) ||(pChnl->stepType == PST_QL) || (pChnl->sta == e_sta_npWaiting))
		{
			isum=0;
		}
		else if(isum<0)
		{
			isum = 0;
		}
	}
#endif

	if(pChnl->flags.bit.limitAmp==_set&&
		(calibrate.calibrating==_reset||calibrate.rechecking==_set))
	{
	    if(pChnl->iref>3000)
	    {
	        pChnl->current=filter_limit_amplitude(pChnl->iref, 3000, isum);
	    }
	    else
	    {
	         pChnl->current=filter_limit_amplitude(pChnl->iref, 1000, isum);
	    }
	}
	else
	{
		pChnl->current=isum;
	}

	//上传电流转换
	pChnl->s32U100uACurrent=pChnl->current*10;
	if (pChnl->flags.bit.charge==_reset)
	{
		pChnl->s32U100uACurrent=0-pChnl->s32U100uACurrent;
	}

	//通道温度
	

	//通道端口电压

	//标识通道数据更新
	pChnl->flags.bit.ivUpdated=_set;
	
#endif	
	
}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：
//	mltrue, 堆栈有错误
//  mlfalse,堆栈无错误
//
//----------------------------------------------------------------------------------
// 功    能：
//	检查维护通道堆栈
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool  chnl_stack_check(CmbPwrChnlptr pChnl)
{
//	u32 temp;


	//若当前工步为跳转工步，则做堆栈维护
	while(pChnl->stepType==PST_JUMP)
	{

		// 堆栈深度大于0 ，栈顶的压栈工步是当前工步	
		// 不需要压栈，只需对出栈值判断
		if((pChnl->stackDepth>0)&&
			(pChnl->stack[pChnl->stackDepth-1].pushStepNo==pChnl->stepNo))
		{
			// 出栈值==0 做出栈动作，执行下一工步
			if(0==pChnl->stack[pChnl->stackDepth-1].popValue)
			{
				pChnl->stackDepth--;
				if((pChnl->stepNo+1)>=pChnl->procfile->rcvstepNum)
				{	
					//获取工步信息失败
					//pChnl->DDC=DDC_stepException;
					pChnl->stopByWay=e_chnl_stopBy_over;
					chnl_sta_migrate(pChnl,e_ch_sta_stopping);
					return mltrue;
				}	
				pChnl->stepNo++;
			}
			// 出栈值>0   ,则减减，执行到需要跳转到的工步
			else
			{			
				pChnl->stack[pChnl->stackDepth-1].popValue--;
				pChnl->stepNo=pChnl->procfile->steps[pChnl->stepNo].gotoStepNo;
				if(pChnl->stepNo>=pChnl->procfile->rcvstepNum)
				{	
					//获取工步信息失败					
					pChnl->DDC=DDC_stepException;
					pChnl->stopByWay=e_chnl_stopBy_exception;
					chnl_sta_migrate(pChnl,e_ch_sta_stopping);
					return mltrue;	
				}
			}

		}
		// 堆栈为空、 当前工步不在栈顶，直接将当前工步压栈，并减减出栈值
		// 压栈后，跳转到指定的工步运行,
		else
		{
			if(pChnl->stackDepth<CHNL_STACK_DEPTH)		
			{
				//压栈
				 pChnl->stack[pChnl->stackDepth].pushStepNo=pChnl->stepNo;
				pChnl->stack[pChnl->stackDepth].popValue=pChnl->procfile->steps[pChnl->stepNo].gotoStepNo-1;
				pChnl->stackDepth++;

				//跳转工步
				pChnl->stepNo=pChnl->procfile->steps[pChnl->stepNo].gotoStepNo;
				if(pChnl->stepNo>=pChnl->procfile->rcvstepNum)
				{	
					//获取工步信息失败
					pChnl->DDC=DDC_stepException;
					pChnl->stopByWay=e_chnl_stopBy_exception;
					chnl_sta_migrate(pChnl,e_ch_sta_stopping);
					return mltrue;	
				}
			}
			else
			{
				//堆栈溢出，关闭通道
				pChnl->DDC=DDC_stepException;
				pChnl->stopByWay=e_chnl_stopBy_exception;
				chnl_sta_migrate(pChnl,e_ch_sta_stopping);
				return mltrue;
			}

		}		
	
		//取需要运行的工步的工步类型
		pChnl->stepType=(ProcessStepType)pChnl->procfile->steps[pChnl->stepNo].type;
		
	}

	return mlfalse;
		
}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//	设置通道测试参数
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
void chnl_mdl_param_set(CmbPwrChnlptr pChnl,u32 vref,u32 iref,u32 irefMod)
{
	Mdlptr pMdl; 

	pMdl=pChnl->pMdlList->head;
	while(pMdl!=nullptr)
	{

		if(pMdl==pChnl->pMdlList->head)
		{
			//不能均分的DA 加在第一个模块
			//mdl_param_set(pMdl,vref,iref+irefMod);
		}
		else
		{

			//mdl_param_set(pMdl,vref,iref);
		}

		//pMdl->needstart=_set;
		pMdl=pMdl->pnext;
		
	}
			
}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//	设置通道测试参数
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
void  chnl_param_set(CmbPwrChnlptr pChnl)
{
	u16 saveChargeflag,flags,limitcurrentflag;
	u32 iAvrg,iModVal,iMax;
	u32 current,voltage,runtime;
	Module *pMdl;

	//初始化 局部 变量
	//save last chrage flag
	saveChargeflag=pChnl->flags.bit.charge;

	pChnl->staChckDelay=1000;
	runtime=0;
	//iMax=attri.currentUseRange*attri.mdlparaNum;
	limitcurrentflag=_set;
	

	//根据工步类型设置参数
	switch(pChnl->stepType)
	{
		// 组合恒流充电
		case PST_CCCC :
			current=pChnl->procfile->steps[pChnl->stepNo].ccCurrent/10;
			voltage=pChnl->procfile->steps[pChnl->stepNo].cvVoltage/10;
			//配置相关参数变量	
			pChnl->flags.bit.charge=_set;
			pChnl->flags.bit.limitAmp=_set;
			break;
		//单独恒流充
		case PST_ICCC:
		{
			//
			current=pChnl->procfile->steps[pChnl->stepNo].ccCurrent/10;
            voltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10+2;
			//配置相关参数变量
			pChnl->flags.bit.charge=_set;
			pChnl->flags.bit.limitAmp=_set;
		}
		break;
		
		//恒功率充电
		case PST_CPC:
		{

			current=pChnl->procfile->steps[pChnl->stepNo].power*10;
			current=((float)current/pChnl->voltage)*1000;
			voltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10+5;
			//配置相关参数变量
			pChnl->flags.bit.charge=_set;
			pChnl->flags.bit.limitAmp=_reset;
			
		}
		break;
		//恒功率放电
		case PST_CPD:
		{

			current=pChnl->procfile->steps[pChnl->stepNo].power*10;
			current=((float)current/pChnl->voltage)*1000;
			voltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10-2;

			//配置相关参数变量	
			pChnl->flags.bit.charge=_reset;
			pChnl->flags.bit.limitAmp=_reset;
		}
		break;

		//
		case PST_DCIRD2:
		{
			current=pChnl->procfile->steps[pChnl->stepNo].dcirCurrent1/10;
			voltage=0;
			//配置相关参数变量	
			pChnl->stepSta=e_step_dcir_discharge1;
			pChnl->flags.bit.charge=_reset;
			pChnl->flags.bit.limitAmp=_reset;
		}
			
		break;
		
		case PST_DCIRD3:
		{
			current=pChnl->procfile->steps[pChnl->stepNo].dcirCurrent1/10;
			voltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10;

			//配置相关参数变量	
			pChnl->stepSta=e_step_dcir_discharge1;
			pChnl->flags.bit.charge=_reset;
			pChnl->flags.bit.limitAmp=_reset;
		}
		break;
		case PST_DCIRC:
		{
			current=pChnl->procfile->steps[pChnl->stepNo].dcirCurrent1/10;
			voltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10+200;
			//配置相关参数变量	
			pChnl->stepSta=e_step_dcir_charge1;
			pChnl->flags.bit.charge=_set;
			pChnl->flags.bit.limitAmp=_reset;
		}			
		break;

		case PST_CRD:
		{
			current=pChnl->voltage/pChnl->procfile->steps[pChnl->stepNo].resistance;
			voltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10-1;
			//配置相关参数变量	
			pChnl->flags.bit.charge=_reset;
			pChnl->flags.bit.limitAmp=_reset;


		}			
		break;
				
		//组合恒流放电
		case PST_CCCD :
		{
			current=pChnl->procfile->steps[pChnl->stepNo].ccCurrent/10;
			voltage=pChnl->procfile->steps[pChnl->stepNo].cvVoltage/10;
			//配置相关参数变量	
			pChnl->flags.bit.charge=_reset;
			pChnl->flags.bit.limitAmp=_set;
		}
		break;
		

		//单独恒流放电
		case PST_ICCD :
		{
			current=pChnl->procfile->steps[pChnl->stepNo].ccCurrent/10;
			voltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10-1;

			//配置相关参数变量	
			pChnl->flags.bit.charge=_reset;
			pChnl->flags.bit.limitAmp=_set;
		}
		break;

		//单独的恒压充电
		case PST_ICVC:
		{
			current=0;
			voltage=pChnl->voltage+5;
			//配置相关参数变量	
			pChnl->flags.bit.charge=_reset;
			break;	
		}
		//单独的恒压放电
		case PST_ICVD:
		{
			current=0;
			voltage=pChnl->voltage-1;
			//配置相关参数变量	
			pChnl->flags.bit.charge=_reset;

		}
		break;

		//静置
		case PST_QL :
		{
			//计算需要运行的模块数
			current=0;
			voltage=0;			
			//配置相关参数变量		
			pChnl->staChckDelay=0;
			pChnl->flags.bit.charge=_reset;
			pChnl->flags.bit.limitAmp=_reset;
			
		}
		break;

		//静置
		case PST_OCV :
		{
			//计算需要运行的模块数
			current=0;
			voltage=0;			
			//配置相关参数变量		
			pChnl->staChckDelay=0;
			pChnl->flags.bit.charge=_reset;
			pChnl->flags.bit.limitAmp=_reset;
			
		}
		break;
		
		
	}

	//限制 最大电流
	if(limitcurrentflag==_set&&current>iMax)
	{
		current=iMax;
	}
	
	//保存电流参考值
	pChnl->iref=current;


	//
	//计算需要运行的模块数
	if (pChnl->flags.bit.calibrating)
	{
		pChnl->runMdlNum=1;
	}
	else
	{
		pChnl->runMdlNum=pChnl->mdlTotal;
	}
	
	pChnl->pCvModule=pChnl->pMdlList->head;

	//配置模块DA 、置启动信号
	iAvrg=current/pChnl->runMdlNum;
	iModVal=current%pChnl->runMdlNum;
	chnl_mdl_param_set(pChnl, voltage,iAvrg, iModVal);
	
	//charge discharge cut
	if(saveChargeflag!=pChnl->flags.bit.charge)
	{
		pChnl->flags.bit.changeCDSta=_set;
	}

	if (runtime)
	{
		pChnl->stepruntime=runtime;
	}
	else
	{
		pChnl->stepruntime=pChnl->procfile->steps[pChnl->stepNo].endTime;
	}
		
	pChnl->flags.bit.waitParaRefresh = _reset;
	
}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：
//		mltrue，有启动条件不满足，不继续启动；
//		mlfalse，无启动条件满足，继续启动
//----------------------------------------------------------------------------------
// 功    能：
//	启动条件，预检查，检查启动条件是否满足
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool chnl_starting_cond_precheck(CmbPwrChnlptr pChnl)
{
	MLBool bHaveErr;
	u32 vparam;

	bHaveErr=mlfalse;
	
	// 启动方式 处理
	switch (pChnl->startByWay)
	{
		case e_chnl_startBy_autoNextStep:									//自动跳转
			if((pChnl->stepNo+1)>=pChnl->procfile->rcvstepNum)
			{
				pChnl->stopByWay=e_chnl_stopBy_over;
				chnl_sta_migrate(pChnl,e_ch_sta_stopping);					//迁徙停止状态，停止通道
				bHaveErr= mltrue;
			}
			else
			{
	         	pChnl->stepNo++;												//跳下一步
			}
			break;
		
		case e_chnl_startBy_forceGoto :										//用户强制跳转
			pChnl->stepNo=pChnl->gotoStepNo;
			if(pChnl->stepNo>=pChnl->procfile->rcvstepNum)
			{
				pChnl->stopByWay=e_chnl_stopBy_exception;
				pChnl->DDC=DDC_stepException;
				chnl_sta_migrate(pChnl,e_ch_sta_stopping);
				bHaveErr= mltrue;
			}
			break;

		
		case e_chnl_startBy_user:												//用户启动
			pChnl->stepNo=pChnl->gotoStepNo;
			if(pChnl->stepNo>=pChnl->procfile->rcvstepNum)
			{			
				pChnl->DDC=DDC_stepException;
				chnl_sta_migrate(pChnl,e_ch_sta_stopping);
				bHaveErr= mltrue;
			}
			break;

		default :
			pChnl->stopByWay=e_chnl_stopBy_exception;
			pChnl->DDC=DDC_stepException;
			chnl_sta_migrate(pChnl,e_ch_sta_stopping);
			bHaveErr=mltrue;
			break;
			
	}

		
	if(bHaveErr)					//有错误直接返回
	{
		return bHaveErr;
	}
	
	pChnl->flags.bit.middle=_set;
	pChnl->lastStepType=pChnl->stepType;										//保存类型工步
	
	//取需要运行的工步的工步类型
	pChnl->stepType=(ProcessStepType)pChnl->procfile->steps[pChnl->stepNo].type;

    //堆栈
     if (pChnl->stepType==PST_JUMP)
     {
        if(chnl_stack_check(pChnl))						//堆栈检查
        {
           return mltrue;									//返回false，启动条件不满足
        }
     }


    //iccd 截止电压 预判断
     if(pChnl->flags.bit.hwageing == _reset)
     {
         vparam=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10;
         if (((pChnl->stepType==PST_ICCD)||(pChnl->stepType==PST_PULSD)||(pChnl->stepType==PST_RAMPD)||(pChnl->stepType==PST_PRAMPD))&& pChnl->voltage < vparam)
         {
			//往下一步跳转
			pChnl->DDC= DDC_voltageEnd;
			pChnl->startByWay=e_chnl_startBy_autoNextStep;
			chnl_data_save(pChnl);
			chnl_sta_migrate(pChnl,e_ch_sta_starting);
			return mltrue;
         }
     }

	 
	 //恒流恒压充电拆分两个工步
	 if (pChnl->stepType==PST_CCCVC)
	 {

	 	 pChnl->pCvModule=pChnl->pMdlList->head;
		 pChnl->stepType=PST_CCCC;
		 vparam=pChnl->procfile->steps[pChnl->stepNo].cvVoltage/10;
		 //判断当前电压是否大于恒压点
		 if (pChnl->voltage>vparam)
		 {
			//往下一步跳转
			pChnl->DDC= DDC_cvException;
			pChnl->startByWay=e_chnl_startBy_autoNextStep;
			chnl_data_save(pChnl);
			chnl_sta_migrate(pChnl,e_ch_sta_starting);
			return mltrue;
		 }
	 
	 }

	 //恒流恒压放电拆分两个工步
	 if (pChnl->stepType==PST_CCCVD)
	 {

	 	 pChnl->pCvModule=pChnl->pMdlList->head;
		 pChnl->stepType=PST_CCCD;
		vparam=pChnl->procfile->steps[pChnl->stepNo].cvVoltage/10;	 
		 //判断当前电压是否大于恒压点
		 if (pChnl->voltage<vparam)
		 {
			 //往下一步跳转
			 pChnl->DDC= DDC_cvException;
			 pChnl->startByWay=e_chnl_startBy_autoNextStep;
			 chnl_data_save(pChnl);
			 chnl_sta_migrate(pChnl,e_ch_sta_starting);
			 return mltrue;

		 }
	 
	 }
	 
	 //iccc 截止电压 预判断
	 vparam=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10;
	 if (pChnl->stepType==PST_ICCC&& pChnl->voltage > vparam)

	 {
		 //往下一步跳转
		 pChnl->DDC= DDC_voltageEnd;
		 pChnl->startByWay=e_chnl_startBy_autoNextStep;
		 chnl_data_save(pChnl);
		 chnl_sta_migrate(pChnl,e_ch_sta_starting);
		 return mltrue;

	 }
			 
	//icvd 电压 预判断
	vparam=pChnl->procfile->steps[pChnl->stepNo].cvVoltage/10;
	 if (pChnl->stepType==PST_ICVD&& pChnl->voltage < vparam)

	 {
		 //往下一步跳转
		 pChnl->DDC= DDC_currentEnd;
		 pChnl->startByWay=e_chnl_startBy_autoNextStep;
		 chnl_data_save(pChnl);
		 chnl_sta_migrate(pChnl,e_ch_sta_starting);
		 return mltrue;
	 }

	//icvc 电压 预判断
	vparam=pChnl->procfile->steps[pChnl->stepNo].cvVoltage/10;
	 if (pChnl->stepType==PST_ICVC && pChnl->voltage > vparam)

	 {
		 //往下一步跳转
		 pChnl->DDC= DDC_currentEnd;
		 pChnl->startByWay=e_chnl_startBy_autoNextStep;
		 chnl_data_save(pChnl);
		 chnl_sta_migrate(pChnl,e_ch_sta_starting);
		 return mltrue;

	 }		 


     //工步类型断言
     if (pChnl->stepType!=PST_CCCC&&pChnl->stepType!=PST_ICCC&&pChnl->stepType!=PST_ICCD
             &&pChnl->stepType!=PST_QL&&pChnl->stepType!=PST_ICVC&&pChnl->stepType!=PST_CCCD
             &&pChnl->stepType!=PST_ICVD&&pChnl->stepType!=PST_CPC&&pChnl->stepType!=PST_CPD
             &&pChnl->stepType!=PST_DCIRC&&pChnl->stepType!=PST_DCIRD2&&pChnl->stepType!=PST_DCIRD3
             &&pChnl->stepType!=PST_CRD&&pChnl->stepType!=PST_OCV &&pChnl->stepType!=PST_PULSD
             &&pChnl->stepType!=PST_RAMPD&&pChnl->stepType!=PST_PRAMPD)
	 {
		 pChnl->DDC= DDC_stepException;
		 pChnl->stopByWay=e_chnl_stopBy_exception;
		 chnl_data_save(pChnl);
		 chnl_sta_migrate(pChnl,e_ch_sta_stopping);
		 return mltrue;	 
	 }

	return mlfalse;

}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：
//	mltrue，继续运行通道状态机，发生状态改变;
//  mlfalse ，退出状态机的运行，等待下次轮询
//----------------------------------------------------------------------------------
// 功    能：
//	启动通道
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool chnl_starting_sta_handle_sgnl(CmbPwrChnlptr pChnl)
{
	MLBool bStaChange;
	bStaChange=mlfalse;
	
	if(chnl_sgnl_get(pChnl, SGNL_ForceStop))					//工步停止
	{
		pChnl->DDC=DDC_forceStop;
		pChnl->stopByWay=e_chnl_stopBy_force;
		chnl_sta_migrate(pChnl,e_ch_sta_stopping);
		bStaChange=mltrue;
	}

	if(chnl_sgnl_get(pChnl, SGNL_ForceJumpStep))				//工步跳转
	{
		//chnl_sgnl_clr(pChnl);
		if(pChnl->gotoStepNo!=pChnl->stepNo)
		{
			//不等于当前工步，跳转
			pChnl->startByWay=e_chnl_startBy_forceGoto;
			pChnl->DDC=DDC_forceJump;
			chnl_sta_migrate(pChnl,e_ch_sta_starting);
			bStaChange=mltrue;
		}
	}

	if(chnl_sgnl_get(pChnl, SGNL_Pause))				//工步暂停
	{
		//chnl_sgnl_clr(pChnl);
		if(pChnl->gotoStepNo!=pChnl->stepNo)
		{
			//不等于当前工步，跳转
			pChnl->startByWay=e_chnl_stopBy_pause;
			pChnl->DDC=DDC_pause;
			chnl_sta_migrate(pChnl,e_ch_sta_stopping);
			bStaChange=mltrue;
		}
	}

	//处理其它信号
	chnl_sgnl_clr(pChnl);

	if(bStaChange)								//通道状态改变
	{
		chnl_data_save(pChnl);				//保存当前数据
	}
	
	return bStaChange;
	
}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//	设置通道测试参数
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool chnl_mdl_started(CmbPwrChnlptr pChnl)
{
	Mdlptr pMdl; 

	pMdl=pChnl->pMdlList->head;
	while(pMdl!=nullptr)
	{

		pMdl=pMdl->pnext;
	}

	return mltrue;
}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：
//	mltrue，继续运行通道状态机，发生状态改变;
//  mlfalse ，退出状态机的运行，等待下次轮询
//----------------------------------------------------------------------------------
// 功    能：
//	启动通道
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool chnl_starting(CmbPwrChnlptr pChnl)
{
	subtask_declaration();								//任务声明


	if(chnl_starting_sta_handle_sgnl(pChnl))			//启动状态处理控制信号
	{
		subtask_exit(pChnl->pMyTask, mltrue);			//退出当前状态任务
	}
	

	subtask_begin(pChnl->pMyTask);						//任务开始

	
	if (chnl_starting_cond_precheck(pChnl))					//启动条件检查
	{
		subtask_exit(pChnl->pMyTask, mltrue);			//退出当前状态任务
	}
			
	
	chnl_param_set(pChnl);								//启动参数设置


	
	//chnl_traverse_module(pChnl,module_start);			//启动模块		


	subtask_wait_cnd_until(pChnl->pMyTask, chnl_mdl_started(pChnl),500, mlfalse);					//等待启动完成

		
	
	//标识通道开始运行
	pChnl->DDC=DDC_Normal;
	pChnl->flags.bit.running=_set;
	pChnl->flags.bit.taskLocalSusp=_reset;


	// 启动通道状态检测延时
	if(pChnl->staChckDelay>0)
	{
		pChnl->flags.bit.taskLocalSusp=_set;
		//ltimer_load_start(&pChnl->commonTimer,pChnl->staChckDelay);
	}


	//clear 
	pChnl->flags.bit.stepJump=_reset;
	pChnl->flags.bit.changeCDSta=_reset;
	pChnl->startByWay=e_chnl_startBy_null;
	pChnl->errorCounter=0;
	pChnl->capacity=0;
//	pChnl->capacity2=0;
	pChnl->capacity3=0;
	pChnl->mAHCapacity=0;
	pChnl->startTime = mlos_ms_clock();
	
	//启动完成，进入 工作状态
	chnl_sta_migrate(pChnl,e_ch_sta_working);
	
	subtask_end(pChnl->pMyTask, mltrue);					//任务结束
	
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
//---------------------------------------------------------------------------
void chnl_iccd_check(CmbPwrChnlptr pChnl)
{
	float endVoltage;
	float ref,offset;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;

	endVoltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10;

	//工步跳转判断
	if(pChnl->voltage<=endVoltage)
	{	

		//电压截止
	    pChnl->DDC=DDC_voltageEnd;
		chnl_sgnl_set(pChnl,SGNL_AutoJumpStep);
		//debug.flag1=1;
		return;
	}

#if 0
		ref=pChnl->procfile->steps[pChnl->stepNo].ccCurrent/10;
		offset=ref*Current_Exception_Offset_Percent;
	
		//电流异常判断
		if (pChnl->current<(ref-offset)||pChnl->current>(ref+offset))
		{
			pChnl->errorCounter++;
			if(pChnl->errorCounter>=200)
			{
				pChnl->DDC=DDC_ccCurrentException;
				chnl_sgnl_set(pChnl,SGNL_FaultStop);
				pChnl->errorCounter=0;
				return;
			}
		}
		else
		{
			pChnl->errorCounter=0;	
		}
	
#endif

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
//--------------------------------------------------------------------------- 
void chnl_cccd_check(CmbPwrChnlptr pChnl)
{
	Module *pMdl;
	float ref,offset,cvVoltage;
//	float endCurrent;


	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;

	cvVoltage=(pChnl->procfile->steps[pChnl->stepNo].cvVoltage/10)+1;

	if ((pChnl->voltage<=cvVoltage))
	{
        //保存类型工步
        pChnl->lastStepType=pChnl->stepType;
		//进入cv
		pChnl->stepType=PST_CCVD;
        pMdl=pChnl->pMdlList->head;
		return;
	}

#if 0
		ref=pChnl->procfile->steps[pChnl->stepNo].CCCVD.current/10;
		offset=ref*Current_Exception_Offset_Percent;
	
		//电流异常判断
		if (pChnl->current<(ref-offset)||pChnl->current>(ref+offset))
		{
			pChnl->errorCounter++;
			if(pChnl->errorCounter>=200)
			{
				pChnl->DDC=DDC_ccCurrentException;
				chnl_sgnl_set(pChnl,SGNL_FaultStop);
				pChnl->errorCounter=0;
				return;
			}
		}
		else
		{
			pChnl->errorCounter=0;	
		}
	
#endif

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
//---------------------------------------------------------------------------
void chnl_cccc_check(CmbPwrChnlptr pChnl)
{
	Module *pMdl;
	float ref,offset,cvVoltage;
//	float endCurrent;


	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;

	cvVoltage=pChnl->procfile->steps[pChnl->stepNo].cvVoltage/10;

	//进入恒压判断
//	if ((pChnl->voltage>=cvVoltage)&&(inClaFlag[pChnl->myAddr].InCvflag == 1))
    if ((pChnl->voltage>=cvVoltage))
	{
        //保存类型工步
        pChnl->lastStepType=pChnl->stepType;
		//进入cv
        pChnl->stepType=PST_CCVC;
	    pMdl=pChnl->pMdlList->head;
		return;
	}

#if 0
		ref=pChnl->procfile->steps[pChnl->stepNo].CCCVC.current/10;
		offset=ref*Current_Exception_Offset_Percent;
	
		//电流异常判断
		if (pChnl->current<(ref-offset)||pChnl->current>(ref+offset))
		{
			pChnl->errorCounter++;
			if(pChnl->errorCounter>=200)
			{
				pChnl->DDC=DDC_ccCurrentException;
				chnl_sgnl_set(pChnl,SGNL_FaultStop);
				pChnl->errorCounter=0;
				return;
			}
		}
		else
		{
			pChnl->errorCounter=0;	
		}
	
#endif

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
//---------------------------------------------------------------------------
void chnl_icccTo_iccv(CmbPwrChnlptr pChnl)
{
    Module*pMdl;
    float _vref=0,addCvOffset;

    if(pChnl->flags.bit.withNegPresStarted==_reset)
    {
        if(((pChnl->stepNo+1)<pChnl->procfile->rcvstepNum)&&
			(pChnl->procfile->steps[pChnl->stepNo].type==PST_ICCC)&&
			(pChnl->procfile->steps[pChnl->stepNo+1].type==PST_ICVC))
        {
            //记录当前工步最后一条数据
            //chnl_event_save(pChnl);
            pChnl->DDC = DDC_Normal;
            pChnl->preCurrent=
            pChnl->currentAvg=pChnl->procfile->steps[pChnl->stepNo].ccCurrent;
            //保存类型工步
            pChnl->lastStepType=pChnl->stepType;
            pChnl->stepNo++;
            pChnl->stepType=(ProcessStepType)pChnl->procfile->steps[pChnl->stepNo].type;
            return;
        }
        else
        {
            chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
        }
    }
    else
    {
        chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
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
//---------------------------------------------------------------------------
void chnl_iccc_check(CmbPwrChnlptr pChnl)
{
//	Module *pMdl;

	float ref,offset,endVoltage;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;
	
	endVoltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10;

	if(pChnl->voltage>=endVoltage)
	{
		//电压截止
	    pChnl->DDC=DDC_voltageEnd;
	    chnl_icccTo_iccv(pChnl);
        return;
	}

#if 0
		ref=pChnl->procfile->steps[pChnl->stepNo].ICCC.current/10;
		offset=ref*Current_Exception_Offset_Percent;
	
		//电流异常判断
		if (pChnl->current<(ref-offset)||pChnl->current>(ref+offset))
		{
			pChnl->errorCounter++;
			if(pChnl->errorCounter>=200)
			{
				pChnl->DDC=DDC_ccCurrentException;
				chnl_sgnl_set(pChnl,SGNL_FaultStop);
				pChnl->errorCounter=0;
				return;
			}
		}
		else
		{
			pChnl->errorCounter=0;	
		}
	
#endif

	
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
//---------------------------------------------------------------------------
void chnl_ccvd_check(CmbPwrChnlptr pChnl)
{
	float endCurrent;


	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;
	
/*
	//恒压偏大设定点
	if(pChnl->voltage>
		(pChnl->procfile->steps[pChnl->stepNo].mainPara+200))		
	{
		pChnl->DDC=DDC_VOL_OVER;
		*outNextEvent=Event_JumpStep;
		return ;
	}
*/	
#if 0
	//多个模块，并联恒压
	//恒压模块，切换
	if (pChnl->pCvModule->current<50)
	{
		module_close(pChnl->pCvModule);		
		if (pChnl->pCvModule->pNext!=nullptr)
		{
			pChnl->pCvModule=pChnl->pCVModule->pNext;

			//下一个模块进入恒压
			module_cc_to_cv(pChnl->pCvModule);
		}
	
	}
#endif
#if 0
	//多个模块，并联恒压
	//恒压模块，切换
	if (pChnl->pCvModule->pNext!=nullptr&&pChnl->pCvModule->current<50)
	{		
		module_close(pChnl->pCvModule);		
		pChnl->pCvModule=pChnl->pCvModule->pNext;
		//下一个模块进入恒压
		module_cc_to_cv(pChnl->pCvModule);
	}

#endif

	//工步跳转检测
	endCurrent=pChnl->procfile->steps[pChnl->stepNo].endCurrent/10;
	if((pChnl->current<=endCurrent))//||(pChnl->currentAvg<=endCurrent))
	{
		//电压截止
	    pChnl->DDC=DDC_currentEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;
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
//---------------------------------------------------------------------------
void chnl_ccvc_check(CmbPwrChnlptr pChnl)
{
	float endCurrent;
    s32 currentAvg;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;
		

#if 0
	//多个模块，并联恒压
	//恒压模块，切换
	if (pChnl->pCvModule->pNext!=nullptr&&pChnl->pCvModule->current<50)
	{		
		module_close(pChnl->pCvModule);		
		pChnl->pCvModule=pChnl->pCvModule->pNext;
		//下一个模块进入恒压
		module_cc_to_cv(pChnl->pCvModule);
	}
#endif
	//工步跳转检测

	endCurrent=pChnl->procfile->steps[pChnl->stepNo].endCurrent/10;
    currentAvg=pChnl->currentAvg/10;
//    if((pChnl->procfile->steps[pChnl->stepNo].CCCVC.endCurrent.value>10)&&(pChnl->current<=endCurrent))
    if((pChnl->procfile->steps[pChnl->stepNo].endCurrent>10)&&((pChnl->current<=endCurrent)||(currentAvg<=endCurrent)))
    {
		//电压截止
	    pChnl->DDC=DDC_currentEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;
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
//--------------------------------------------------------------------------- 
void chnl_icvc_check(CmbPwrChnlptr pChnl)
{
	float endCurrent;
	s32 currentAvg;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;
	pChnl->flags.bit.limitAmp=_reset;
		
/*
	//恒压偏大设定点
	if(pChnl->voltage>
		(pChnl->procfile->steps[pChnl->stepNo].mainPara+200))		
	{
		pChnl->DDC=DDC_VOL_OVER;
		*outNextEvent=Event_JumpStep;
		return ;
	}
*/	

#if 0
	//多个模块，并联恒压
	//恒压模块，切换
	if (pChnl->pCvModule->pNext!=nullptr&&pChnl->pCvModule->current<50)
	{		
		module_close(pChnl->pCvModule); 	
		pChnl->pCvModule=pChnl->pCvModule->pNext;
		//下一个模块进入恒压
		module_cc_to_cv(pChnl->pCvModule);
	}
#endif
	

	//工步跳转检测
	endCurrent=pChnl->procfile->steps[pChnl->stepNo].endCurrent/10;
	currentAvg=pChnl->currentAvg/10;
	if((pChnl->procfile->steps[pChnl->stepNo].endCurrent>10)&&
		((pChnl->current<=endCurrent)||(((float)currentAvg)<=endCurrent)))
	{
		//电压截止
	    pChnl->DDC=DDC_currentEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;
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
//---------------------------------------------------------------------------
void chnl_cwd_check(CmbPwrChnlptr pChnl)
{

	u32 v1,v2;
	u32 avrg,more;
	u32 current;
	Module *pMdl;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;


	v1=pChnl->voltage;
	v2=pChnl->lastvoltage;


	//截止条件
	if(v1<=(pChnl->procfile->steps[pChnl->stepNo].endVoltage/10))
	{
		//电压截止
	    pChnl->DDC=DDC_voltageEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;
	}

	if(v1==v2)
	{
		return;
	}

	//刷新电流，
	if(pChnl->voltage>0)
	{
	   	current = pChnl->procfile->steps[pChnl->stepNo].power*1000;
	    current =(float)current/pChnl->voltage*10;
	}
	else
	{
		current=0;
	}

	//配置模块DA 、置启动信号
	avrg=current/pChnl->runMdlNum;
	more=current%pChnl->runMdlNum;

	pMdl=pChnl->pMdlList->head;
	while(pMdl!=nullptr)
	{
		if(pMdl==pChnl->pMdlList->head)
		{
			//不能均分的DA 加在第一个模块
			//module_iref_set(pMdl,avrg+more);
		}
		else
		{
			//module_iref_set(pMdl,avrg);
		}
		//module_loop_iref_refresh(pMdl);
		pMdl=pMdl->pnext;	
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
//---------------------------------------------------------------------------
void chnl_cwc_check(CmbPwrChnlptr pChnl)
{

	u32 v1,v2;
	u32 avrg,more;
	u32 current;
	Module *pMdl;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;
	
	v1=pChnl->voltage;
	v2=pChnl->lastvoltage;
	

	//截止条件
	if(v1>=(pChnl->procfile->steps[pChnl->stepNo].endVoltage/10))
	{
		//电压截止
	    pChnl->DDC=DDC_voltageEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;
	}

	if(v1==v2)
	{
		return;
	}

	//刷新电流，
	if(pChnl->voltage>0)
	{
	   	current = pChnl->procfile->steps[pChnl->stepNo].power*1000;
	    current =(float)current/pChnl->voltage*10;
	}
	else
	{
		current=0;
	}

	//配置模块DA 、置启动信号
	avrg=current/pChnl->runMdlNum;
	more=current%pChnl->runMdlNum;

	pMdl=pChnl->pMdlList->head;
	while(pMdl!=nullptr)
	{
		if(pMdl==pChnl->pMdlList->head)
		{
			//不能均分的DA 加在第一个模块
			//module_iref_set(pMdl,avrg+more);
		}
		else
		{
			//module_iref_set(pMdl,avrg);
		}
		//module_loop_iref_refresh(pMdl);
		pMdl=pMdl->pnext;	
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
//---------------------------------------------------------------------------
void chnl_dcirc_check(CmbPwrChnlptr pChnl)
{
	u32 avrg,more;
	float endVoltage;
	u32 current;
	Module*pMdl;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;
	

	endVoltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10;

	if(pChnl->voltage>=endVoltage)
	{
		//电压截止
	    pChnl->DDC=DDC_voltageEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;	
	}
	
	//if(ltimer_timeout(&pChnl->stepTimer)==_no)
	{	
		//return;
	}


	if(pChnl->stepSta==e_step_dcir_charge1)
	{
	
		current=pChnl->procfile->steps[pChnl->stepNo].dcirCurrent2/10;
		//配置模块DA 、置启动信号
		avrg=current/pChnl->runMdlNum;
		more=current%pChnl->runMdlNum;

		pMdl=pChnl->pMdlList->head;
		while(pMdl!=nullptr)
		{
			if(pMdl==pChnl->pMdlList->head)
			{
				//不能均分的DA 加在第一个模块
				//module_iref_set(pMdl,avrg+more);
			}
			else
			{
				//module_iref_set(pMdl,avrg);
			}
			//module_loop_iref_refresh(pMdl);
			pMdl=pMdl->pnext;	
		}

		//放电时间
		//ltimer_load_start(&pChnl->stepTimer, pChnl->procfile->steps[pChnl->stepNo].DCIRC.time2);
		pChnl->stepSta=e_step_dcir_charge2;

	}
	else
	{
		pChnl->DDC=DDC_timeEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		pChnl->stepSta=e_step_over;
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
//---------------------------------------------------------------------------
void chnl_dcird2_check(CmbPwrChnlptr pChnl)
{
	u32 avrg,more;
	u32 current;
	Module*pMdl;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;
	

	//if(ltimer_timeout(&pChnl->stepTimer)==_no)
	{	
		//return;
	}


	if(pChnl->stepSta==e_step_dcir_discharge1)
	{
	
		current=pChnl->procfile->steps[pChnl->stepNo].dcirCurrent2/10;
		//配置模块DA 、置启动信号
		avrg=current/pChnl->runMdlNum;
		more=current%pChnl->runMdlNum;

		pMdl=pChnl->pMdlList->head;
		while(pMdl!=nullptr)
		{
			if(pMdl==pChnl->pMdlList->head)
			{
				//不能均分的DA 加在第一个模块
				//module_iref_set(pMdl,avrg+more);
			}
			else
			{
				//module_iref_set(pMdl,avrg);
			}
			//module_loop_iref_refresh(pMdl);
			pMdl=pMdl->pnext;	
		}

		//放电时间
		//ltimer_load_start(&pChnl->stepTimer, pChnl->procfile->steps[pChnl->stepNo].DCIRD2.time.value);
		pChnl->stepSta=e_step_dcir_discharge2;

	}
	else
	{
		pChnl->DDC=DDC_timeEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		pChnl->stepSta=e_step_over;
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
//---------------------------------------------------------------------------
void chnl_dcird3_check(CmbPwrChnlptr pChnl)
{
	u32 avrg,more;
	u32 current;
	float endVoltage;
	Module*pMdl;

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;
	

	endVoltage=pChnl->procfile->steps[pChnl->stepNo].endVoltage/10;

	if(pChnl->voltage<=endVoltage)
	{
		//电压截止
	    pChnl->DDC=DDC_voltageEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;	
	}
	
	//if(ltimer_timeout(&pChnl->stepTimer)==_no)
	{	
		//return;
	}


	if(pChnl->stepSta==e_step_dcir_discharge1)
	{
	
		current=pChnl->procfile->steps[pChnl->stepNo].dcirCurrent2/10;
		//配置模块DA 、置启动信号
		avrg=current/pChnl->runMdlNum;
		more=current%pChnl->runMdlNum;

		pMdl=pChnl->pMdlList->head;
		while(pMdl!=nullptr)
		{
			if(pMdl==pChnl->pMdlList->head)
			{
				//不能均分的DA 加在第一个模块
				//module_iref_set(pMdl,avrg+more);
			}
			else
			{
				//module_iref_set(pMdl,avrg);
			}
			//module_loop_iref_refresh(pMdl);
			pMdl=pMdl->pnext;	
		}

		//放电时间
		//ltimer_load_start(&pChnl->stepTimer, pChnl->procfile->steps[pChnl->stepNo].DCIRD3.time2);
		pChnl->stepSta=e_step_dcir_discharge2;

	}
	else
	{
		pChnl->DDC=DDC_timeEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		pChnl->stepSta=e_step_over;
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
//---------------------------------------------------------------------------
void chnl_crd_check(CmbPwrChnlptr pChnl)
{

	u32 v1,v2;
	u32 avrg,more;
	u32 current;
	Module *pMdl;

	//工步运行结束
	if(ltimer_timeout(&pChnl->stepTimer))
	{	
		pChnl->DDC=DDC_timeEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;
	}

	//通道数是否更新判断
	if (pChnl->flags.bit.ivUpdated==_reset)
	{
		return ;
	}
	pChnl->flags.bit.ivUpdated=_reset;


	v1=pChnl->voltage;
	v2=pChnl->lastvoltage;


	//截止条件
	if(v1<=(pChnl->procfile->steps[pChnl->stepNo].endVoltage/10))
	{
		//电压截止
	    pChnl->DDC=DDC_voltageEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
		return;
	}

	if(v1==v2)
	{
		return;
	}

	//刷新电流，
	if(pChnl->voltage>0)
	{
	    current = pChnl->voltage/pChnl->procfile->steps[pChnl->stepNo].resistance;
	}
	else
	{
		current=0;
	}

	//配置模块DA 、置启动信号
	avrg=current/pChnl->runMdlNum;
	more=current%pChnl->runMdlNum;

	pMdl=pChnl->pMdlList->head;
	while(pMdl!=nullptr)
	{
		if(pMdl==pChnl->pMdlList->head)
		{
			//不能均分的DA 加在第一个模块
			//module_iref_set(pMdl,avrg+more);
		}
		else
		{
			//module_iref_set(pMdl,avrg);
		}
		//module_loop_iref_refresh(pMdl);
		pMdl=pMdl->pnext;	
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
//---------------------------------------------------------------------------
void chnl_ql_check(CmbPwrChnlptr pChnl)
{

	//工步运行结束
	if(ltimer_timeout(&pChnl->stepTimer))
	{	
		pChnl->DDC=DDC_timeEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
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
//--------------------------------------------------------------------------- 
void chnl_ocv_check(CmbPwrChnlptr pChnl)
{

	//工步运行结束
	if(ltimer_timeout(&pChnl->stepTimer))
	{	
		pChnl->DDC=DDC_timeEnd;
		chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
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
//---------------------------------------------------------------------------
void chnl_check_hook(CmbPwrChnlptr pChnl)
{

	// add what you  want

	pChnl->lastCurrent=0;
	pChnl->capacity=0;
	pChnl->mAHCapacity=0;
	pChnl->timeSum=0;

	if (pChnl->flags.bit.contiStart==_set)
	{
		//续接启动
	    if((pChnl->stepContinueRunTime > pChnl->procfile->steps[pChnl->stepNo].endTime)
	            || (pChnl->stepContinueRunTime==0))
	    {
            pChnl->stepruntime=pChnl->procfile->steps[pChnl->stepNo].endTime;
	    }
	    else
	    {
	        pChnl->stepruntime=pChnl->stepContinueRunTime;
	    }
		pChnl->capacity=pChnl->haveRunCapacity;
		pChnl->flags.bit.contiStart=_reset;
	}

    // 时间截止的工步，启动计时
    if(pChnl->stepruntime==0)
    {
        pChnl->stepruntime=0xffffffff;
    }

	ltimer_load_start(&pChnl->stepTimer, pChnl->stepruntime);

	pChnl->lastCalCapTime=mlos_ms_clock();


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
//---------------------------------------------------------------------------
void chnl_check(CmbPwrChnlptr pChnl)
{
	//通道工步状态检测
//	if((pChnl->flags.bit.running==_set)&&(pChnl->flags.bit.taskLocalSusp!=_set))
	{
	
		//容量计算
		chnl_capacity_caculate(pChnl,_set);
		
		//容量截止
		if (pChnl->procfile->steps[pChnl->stepNo].endCapacity>0)
		{
			if (pChnl->capacity>=pChnl->procfile->steps[pChnl->stepNo].endCapacity)
			{
				pChnl->endCapacity = (u32)pChnl->capacity;
				pChnl->DDC=DDC_capacityEnd;
		        chnl_icccTo_iccv(pChnl);
//				chnl_sgnl_set(pChnl, SGNL_AutoJumpStep);
				return;
			}
		}

		//通道状态检测
		switch(pChnl->stepType)
		{
			//单独的恒流放电
			case PST_ICCD:
				chnl_iccd_check(pChnl);
			break;
			
			//组合的恒流放电
			case PST_CCCD:
			{
				chnl_cccd_check(pChnl);
			}
			break;

			//组合的恒流充电
			case PST_CCCC:
			{
				chnl_cccc_check(pChnl);
			}
			break;

			//单独的恒流充电
			case PST_ICCC:
				chnl_iccc_check(pChnl);
			break;

			//恒压充电
			case PST_CCVC:
				chnl_ccvc_check(pChnl);
				break;
			
			case PST_ICVC:
		        chnl_icvc_check(pChnl);
			break;

			//恒压放电电
			case PST_CCVD:
			case PST_ICVD:
				chnl_ccvd_check(pChnl);
			break;
			
			//静置
			case PST_QL:
				chnl_ql_check(pChnl);
			break;
			
			case PST_OCV:
				chnl_ocv_check(pChnl);
			break;

			//恒功率放电
			case PST_CPD :
				chnl_cwd_check( pChnl);
			break;

			case PST_CPC :
				chnl_cwc_check( pChnl);
			break ;

			case  PST_DCIRD2 :
				chnl_dcird2_check( pChnl);
			break;
			
			case  PST_DCIRD3 :
				chnl_dcird3_check( pChnl);
			break;	
			case  PST_DCIRC :
				chnl_dcirc_check( pChnl);
			break;

			case  PST_CRD :
				chnl_crd_check( pChnl);
			break;

		}						
	}
	
}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
 MLBool chnl_working(CmbPwrChnlptr pChnl)
{
	u16 ddc;
	//采样数据
	chnl_sample(pChnl);

    chnl_data_save(pChnl);

	//故障检测	
	//if((calibrate.running == _reset))
	{
		ddc=chnl_fault_detect(pChnl);
		if(ddc!=DDC_Normal)
		{
			//
			pChnl->DDC=ddc;
			pChnl->stopByWay=e_chnl_stopBy_fault;
			chnl_sta_migrate(pChnl,e_ch_sta_stopping);
			return mltrue;
		}
	}
	
	//强制停止
	if(chnl_sgnl_get(pChnl, SGNL_ForceStop))
	{
		pChnl->stopByWay=e_chnl_stopBy_force;
		pChnl->DDC=DDC_forceStop;
		//pChnl->DDC=DDC_Normal;
		chnl_sta_migrate(pChnl,e_ch_sta_stopping);
		chnl_sgnl_del(pChnl, SGNL_ForceStop);
		return mltrue;
	}

	//强制跳转
	if(chnl_sgnl_get(pChnl, SGNL_ForceJumpStep))
	{
		chnl_sgnl_del(pChnl, SGNL_ForceJumpStep);
		//if((calibrate.running == _set)||(pChnl->flags.bit.hwageing == _set)
		//	||(pChnl->gotoStepNo!=pChnl->stepNo))
		{
			//
			pChnl->startByWay=e_chnl_startBy_forceGoto;
			pChnl->DDC=DDC_forceJump;
			chnl_sta_migrate(pChnl,e_ch_sta_starting);
			return mltrue;
		}
	}

	//暂停
	if(chnl_sgnl_get(pChnl, SGNL_Pause))
	{
		pChnl->stopByWay=e_chnl_stopBy_pause;
		pChnl->DDC=DDC_pause;
		chnl_sta_migrate(pChnl,e_ch_sta_stopping);
		chnl_sgnl_del(pChnl, SGNL_Pause);
		return mltrue;
	}	

	//启动信号
	if(chnl_sgnl_get(pChnl, SGNL_Start))
	{
		chnl_sgnl_del(pChnl, SGNL_Start);
		pChnl->flags.bit.middle=_reset;
	}
	
	//工步检查
	if((pChnl->flags.bit.running==_set)&&(pChnl->flags.bit.taskLocalSusp!=_set))
	{

		//清除中间工步标识
		if(pChnl->flags.bit.middle==_set)
		{
			//
			chnl_check_hook(pChnl);
			pChnl->flags.bit.middle=_reset;	
			pChnl->flags.bit.effectWorking = _set;
		}

		//异常检测
		ddc=chnl_exception_detect(pChnl);
		if(ddc!=DDC_Normal)
		{
			pChnl->DDC=ddc;
			pChnl->stopByWay=e_chnl_stopBy_exception;
			chnl_sta_migrate(pChnl,e_ch_sta_stopping);
			return mltrue;
		}
					
		//通道状态检查
		chnl_check(pChnl);
		
	}


	//自动跳转
	if(chnl_sgnl_get(pChnl, SGNL_AutoJumpStep))
	{
		pChnl->startByWay=e_chnl_startBy_autoNextStep;
		chnl_sta_migrate(pChnl,e_ch_sta_starting);
		chnl_sgnl_del(pChnl, SGNL_AutoJumpStep);
		return mltrue;
	}

	//自动停止
	if(chnl_sgnl_get(pChnl, SGNL_AutoStop))
	{
		//pChnl->stopByWay=CHNL_SBW_;
		chnl_sta_migrate(pChnl,e_ch_sta_stopping);
		chnl_sgnl_del(pChnl, SGNL_AutoStop);
		return mltrue;
	}


	//故障停止
	if(chnl_sgnl_get(pChnl, SGNL_FaultStop))
	{
		pChnl->stopByWay=e_chnl_stopBy_exception;
		chnl_sta_migrate(pChnl,e_ch_sta_stopping);
		chnl_sgnl_del(pChnl, SGNL_FaultStop);
		return mltrue;
	}
	

	//复位事件
	//pChnl->event=CH_Event_Null;
	pChnl->lastvoltage=pChnl->voltage;

	//处理其它信号
	chnl_sgnl_clr(pChnl);

	return mlfalse;
	
}


//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool chnl_stopping(CmbPwrChnlptr pChnl)
{
	subtask_declaration();								//任务声明
	u16 i;


	//获取采样数据
	chnl_sample(pChnl);

	subtask_begin(pChnl->pMyTask);						//任务开始


	


	//停止所有模块
	//chnl_traverse_module(pChnl,module_stop);

	
	//复位管理单元参数
	pChnl->flags.bit.running=_reset;
	pChnl->flags.bit.charge=_reset;
	pChnl->flags.bit.firstStaChck=_reset;
	pChnl->flags.bit.middle=_reset;
	pChnl->flags.bit.limitAmp=_reset;
	pChnl->flags.bit.cvResistor=_reset;
	pChnl->flags.bit.delayExceptionCheck=_reset;
	pChnl->flags.bit.calibrating=_reset;
	pChnl->flags.bit.withNegPresStarted=_reset;
	pChnl->flags.bit.contiStart=_reset;
	pChnl->flags.bit.cvfiltering = _reset;
	pChnl->flags.bit.effectWorking = _reset;
	pChnl->flags.bit.waitParaRefresh = _reset;
	
	pChnl->stepType=PST_NULL;
	pChnl->lastStepType=PST_NULL;
//	pChnl->procfile->rcvstepNum=0;
	pChnl->stepNo=0xff;
	pChnl->stackDepth=0;
	pChnl->runMdlNum=0;
	pChnl->current=0;
	pChnl->capacity=0;
	pChnl->mAHCapacity=0;
	pChnl->haveRunCapacity=0;
	pChnl->stepContinueRunTime=0;
	pChnl->errorCounter=0;
	pChnl->preCurrent = 0;
	pChnl->pCvModule=pChnl->pMdlList->head;


	for(i=0;i<CHNL_STACK_DEPTH;++i)
	{
		pChnl->stack[i].popValue=0;
		pChnl->stack[i].pushStepNo=0;
		pChnl->stack[i].rsv=0;
	}


	//退出停止状态，只保留启动信号
	//chnl_sgnl_keep(pChnl,SGNL_Start);

	//
	chnl_sta_migrate(pChnl,e_ch_sta_idle);

	subtask_end(pChnl->pMyTask, mltrue);					//任务结束

}


//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool chnl_fault(CmbPwrChnlptr pChnl)
{
	u16 ddc,i;
	
	//获取采样数据
	chnl_sample(pChnl);

    chnl_data_save(pChnl);


	//处理其它信号
	chnl_sgnl_clr(pChnl);

	return mlfalse;

}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
MLBool chnl_idle(CmbPwrChnlptr pChnl)
{
	//获取采样数据
	chnl_sample(pChnl);
	
	chnl_data_save(pChnl);

	pChnl->lastvoltage=pChnl->voltage;
	

	//处理启动信号
	if(chnl_sgnl_get(pChnl, SGNL_Start))
	{
		pChnl->startByWay=e_chnl_startBy_user;
		chnl_sta_migrate(pChnl,e_ch_sta_starting);
		chnl_sgnl_del(pChnl, SGNL_Start);
		chnl_sgnl_clr(pChnl);
		return mltrue;
	}

	//处理其它信号
	chnl_sgnl_clr(pChnl);
	return mlfalse;

}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
void chnl_reset(CmbPwrChnlptr pChnl)
{
	mlu16 i;

	pChnl->flags.all=0;
	pChnl->sta=e_ch_sta_idle;
	pChnl->stackDepth=0;
	pChnl->stepNo=0xff;
	pChnl->stepType=PST_NULL;
	pChnl->lastStepType=PST_NULL;
	pChnl->DDC=DDC_Normal;	
	pChnl->stepruntime=0;	
	pChnl->staChckDelay=0;			
	pChnl->startByWay=e_chnl_startBy_null;
	pChnl->ctrlSignal=0;
	pChnl->runMdlNum=0;
	pChnl->mdlTotal=1;
	pChnl->lastCurrent=0;
	pChnl->s32U100uACurrent=0;
	pChnl->current=0;
	pChnl->capacity=0;
	pChnl->mAHCapacity=0;
	pChnl->pCvModule=nullptr;
	pChnl->batReverCounter = 0;


    pChnl->sampleSerialNum = 0;
    pChnl->sampleAddr = pChnl->myAddr*60;
    pChnl->storageAddr = 0;
    pChnl->lastSampleTime = 0;
    pChnl->power = 0;
    pChnl->stepSta=e_step_over;
    pChnl->waitCountr = 0;


	//获取工步模板
	pChnl->procfile=proc_file_get(pChnl->myAddr);

    //堆栈
    for(i=0;i<CHNL_STACK_DEPTH;i++)
    {
        pChnl->stack[i].pushStepNo=0;
        pChnl->stack[i].rsv=0;
        pChnl->stack[i].popValue=0;
    }


    for(i=0; i<storageData_Len_Max; i++)
    {
        pChnl->sampleData[i].stepNo=0xFF;
        pChnl->sampleData[i].stepType=PST_NULL;
        pChnl->sampleData[i].subStepType=PST_NULL;

        pChnl->sampleData[i].current=0;
        pChnl->sampleData[i].voltage=0;
        pChnl->sampleData[i].capacityL=0;
        pChnl->sampleData[i].capacityH=0;
        pChnl->sampleData[i].second=0;
        pChnl->sampleData[i].mSecond=0;

        pChnl->sampleData[i].DDC = DDC_Normal;
        pChnl->sampleData[i].serialNum=0;
    }

	//清零通道输出电流电压
	pChnl->voltage=0;
	pChnl->current=0;
	pChnl->currentAvg=0;
	pChnl->preCurrent=0;



}

//----------------------------------------------------------------------------------
//名    称：
//----------------------------------------------------------------------------------
//
// 入口参数：无
//
// 出口参数：无
//
//----------------------------------------------------------------------------------
// 功    能：
//
//
//----------------------------------------------------------------------------------
//修改记录:
//
//----------------------------------------------------------------------------------
void chnl_init(CmbPwrChnlptr pChnl)
{
	u16 i,j;

	pChnl->pMdlList=list_create(e_mem_sram, e_lst_sortUnordered, e_lst_linkSingly);		//单向链表

	pChnl->pMyTask=subtask_create(ltimer_create());					//创建通道任务

	chnl_reset(pChnl);

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
//---------------------------------------------------------------------------
void chnl_run(CmbPwrChnlptr pChnl)   
{

	//通道被并联判断
	if(pChnl->flags.bit.subParall==_set)
	{
		return;
	}

	MLBool bRunning=mltrue;

	while (bRunning)
	{
		switch (pChnl->sta)
		{

			case e_ch_sta_working:
				bRunning=chnl_working(pChnl);					//工作状态
			break;
		
			case e_ch_sta_idle:
				bRunning=chnl_idle(pChnl);						//空闲状态
			break;

			case e_ch_sta_starting:
				bRunning=chnl_starting(pChnl);					//启动状态
			break;

			case e_ch_sta_stopping:
				bRunning=chnl_stopping(pChnl);					//故障状态
			break;
			case e_ch_sta_fault:
				bRunning=chnl_fault(pChnl);						//停止状态
			break;
			default:
				pChnl->sta=e_ch_sta_idle;		
				bRunning=mltrue;
			break;
						
		}
		
	}

}
#endif

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
void cmbchnl_init(CmbPwrChnlptr pChnl,CmbPwrMdlptr pMdl,mlu8 mdlNum)
{

	pChnl->pMytask=subtask_create( ltimer_create());//创建通道任务
	pChnl->mdlList=list_create(e_mem_sram,e_lst_sortUnordered,e_lst_linkSingly);
	list_append(pChnl->mdlList,pMdl);
	pChnl->mdlNum=mdlNum;//并机默认一个通道一个 
	pChnl->mdl=pMdl;

}


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif







