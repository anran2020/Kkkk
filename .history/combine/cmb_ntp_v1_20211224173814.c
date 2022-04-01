/**
 ******************************************************************************
 * 文件:combine_ntp_v1.c
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
#include "u_dtd.h"
#include "upper_ntp.h"
#include "cmb_bl.h"
#include "cmb_ntp_v1.h"
#include "cmb_step.h"
#include "cmb_def.h"

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
//功能描述：回复上位机联机指令
//上传设备信息
//
//修改履历：
//
//
//-----------------------------------------------------------------------------
void cmb_ntp_cmd_reply(mlu8 cmd,mlu8 addr,mlu8 chnl,mlu8 oeFlag,mlu8 err)
{
	mlu8 *ptxbuf,i;
	
	ptxbuf=upper_txbuf_malloc(11);//获取发送缓冲区
	if(ptxbuf==nullptr)
		return ;
	
	ptxbuf[0]=0xee;//包头
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	ptxbuf[4]=5;//数据包数据长度
	ptxbuf[5]=0;
	ptxbuf[6]=addr;//地址域
	ptxbuf[7]=cmd;		//命令字
	ptxbuf[8]=chnl;
	ptxbuf[9]=oeFlag;

	ptxbuf[10]=err;
	for(i=0;i<4;i++)
	{
		ptxbuf[10]+=ptxbuf[6+i];
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
//功能描述：回复上位机联机指令
//上传设备信息
//
//修改履历：
//
//
//-----------------------------------------------------------------------------
MLBool cmb_upper_connect(void*args)
{
	mlu8 *ptxbuf,*prxbuf;
	mlu32 len,i;
	prxbuf=(mlu8*)args;
	

	//设备地址过滤
	//packet len take 0-1 2byte
	//offset 3 is cmd;  offset  2 is address;
	CmbPwrBoxptr pbox=cbl_power_box_get(prxbuf[2]);
	if(nullptr==pbox)
	{
		return mltrue;
	}
	
	if(pbox->online==_reset)//设备是否在线
	{
		return mltrue;
	}
	
	len=63+2+4;//联机数据包的整体长度为
	
	ptxbuf=upper_txbuf_malloc(len);//获取发送缓冲区
	if(ptxbuf==nullptr)
		return mltrue;
	
	ptxbuf[0]=0xee;//包头
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	len=63;
	ptxbuf[4]=(len&0xff);//数据包数据长度
	ptxbuf[5]=((len>>8)&0xff);
	ptxbuf[6]=pbox->addr;//地址域
	ptxbuf[7]=CMD_Connect;//命令字
	ptxbuf[8]=pbox->chnlNum;//设备通道数	
	
	ptxbuf[9]=pbox->swv;//软件版本号,4个字节
	ptxbuf[10]=0;
	ptxbuf[11]=0;
	ptxbuf[12]=1;

	//dat_to_u8buf(&pbox->vMax,ptxbuf,4);

	ptxbuf[13]=((pbox->vMax)&0xff);//下位机电压量程
	ptxbuf[14]=((pbox->vMax>>8)&0xff);
	ptxbuf[15]=((pbox->vMax>>16)&0xff);
	ptxbuf[16]=((pbox->vMax>>24)&0xff);

	ptxbuf[17]=((pbox->iMax)&0xff);//下位机电流量程
	ptxbuf[18]=((pbox->iMax>>8)&0xff);
	ptxbuf[19]=((pbox->iMax>>16)&0xff);
	ptxbuf[20]=((pbox->iMax>>24)&0xff);

	// falsh 最大地址
	len = (mlu32)CMB_CH_DAT_RCRD_CNT*CMB_CH_DAT_RCRD_LEN*pbox->chnlNum;
	ptxbuf[21]=(len&0xff);
	ptxbuf[22]=((len>>8)&0xff);
	ptxbuf[23]=((len>>16)&0xff);
	ptxbuf[24]=((len>>24)&0xff);

	
	//下位机功率量程
	mlu32 chnlKw=(pbox->iMax/100000)*(pbox->vMax/100000)/1000;
	ptxbuf[25]=(chnlKw&0xff);
	ptxbuf[26]=((chnlKw>>8)&0xff);
	ptxbuf[27]=((chnlKw)&0xff);
	ptxbuf[28]=((chnlKw>>8)&0xff);
	
	//校验和
	ptxbuf[68]=0;
	for(i=0; i<62; i++)
	{
		ptxbuf[68]+= ptxbuf[6+i];
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
//功能描述：回复上位机联机指令
//上传设备信息
//
//修改履历：
//
//
//-----------------------------------------------------------------------------
MLBool cmb_upper_sample(void *args)
{

	mlu8 chnlx,*ptr,*ptxbuf,j,*prxbuf;
	mlu32  sdIndex,num,temp;
	mlu32 i,len ;
	
	prxbuf=(mlu8*)args;
	ptr=prxbuf;
	
	//设备地址过滤
	//packet len take 0-1 2byte
	//offset 3 is cmd;  offset  2 is address;
	CmbPwrBoxptr pbox=cbl_power_box_get(prxbuf[2]);
	if(nullptr==pbox)
	{
		return mltrue;
	}
	
	if(pbox->online==_reset)//设备是否在线
	{
		return mltrue;
	}

	chnlx=prxbuf[4];//通道是否在线
	if(chnlx>=pbox->chnlNum)
	{
		return mltrue;
	}
	
	//采样地址索引号
	sdIndex=ptr[8];
	sdIndex=(sdIndex<<8)+ptr[7];
	sdIndex=(sdIndex<<8)+ptr[6];	
	sdIndex=(sdIndex<<8)+ptr[5];

	sdIndex=sdIndex-(chnlx*CMB_CH_DAT_RCRD_LEN*CMB_CH_DAT_RCRD_CNT);
		
	sdIndex=sdIndex/CMB_CH_DAT_RCRD_LEN;

	if(sdIndex>=CMB_CH_DAT_RCRD_CNT)
	{
//		debug_break_point(123);
		return mltrue;
	}
	
	//采样个数
	num=ptr[9];
	
// 1 4 ge 0xff
// 2	数据长度	2		不包括本身的2Byte
// 3	箱号地址	1		箱号0~15
// 4	命令类型	1	'S'	采样命令字
// 5	通道号	1		通道号0~7
// 6	当前写入地址	4		

	len=4+9+num*CMB_CH_DAT_RCRD_LEN+1;
		
	//获取发送缓冲区
	ptxbuf=upper_txbuf_malloc(len);
	if(ptxbuf==_NULL)
		return 1;


	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	//数据包数据长度
	ptxbuf[4]=((len-6)&0xff);
	ptxbuf[5]=(((len-6)>>8)&0xff);

	//地址域
	ptxbuf[6]=pbox->addr;

	//命令字
	ptxbuf[7]=CMD_Sample;

	//设备通道号
	ptxbuf[8]=chnlx;

	//当前写入地址
	temp=pbox->chnl[chnlx].dataSaveEndAddr+
			((mlu32)CMB_CH_DAT_RCRD_CNT*CMB_CH_DAT_RCRD_LEN*chnlx);

	u32_to_u8(temp, ptxbuf[9], ptxbuf[10], ptxbuf[11], ptxbuf[12]);

	//取数据
	for(i=0;i<num;i++)
	{
		for(j=0;j<CMB_CH_DAT_RCRD_LEN;j++)
		{
			ptxbuf[i*CMB_CH_DAT_RCRD_LEN+13+j]=pbox->chnl[chnlx].datBuf[sdIndex][j];
			
			if(pbox->chnl[chnlx].datBuf[sdIndex][1]=='M')
			{
					while(0);
			}
		}
		sdIndex++;
		if(sdIndex>=CMB_CH_DAT_RCRD_CNT)
		{
			sdIndex=0;	
		}
	}

	//sum check
	ptxbuf[len-1]=0;
	for(i=0;i<(len-7);i++)
	{
		ptxbuf[len-1]+=ptxbuf[6+i];
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
unsigned char cmb_receive_step(void *args)
{
	mlu8 *pStepBuf,retFlag,*prxbuf;
	mlu16 worksteps,i ,n;
	mlu32 temp1;
	CmbPwrChnlptr pChnl;

	prxbuf=(mlu8*)args;
	//设备地址过滤
	//packet len take 0-1 2byte
	//offset 3 is cmd;  offset  2 is address;
	CmbPwrBoxptr pbox=cbl_power_box_get(prxbuf[2]);
	if(nullptr==pbox)
	{
		return mltrue;
	}
	
	if(pbox->online==_reset||prxbuf[4]>=pbox->chnlNum)//设备是否在线
	{
		return mltrue;
	}
	
	pChnl=pbox->chnl+prxbuf[4];
	// len0 -len1-addr-cmd-chnl-allsteps-steps-seed-suncheck
	//总工步数;  offset  5 is allSteps;
	pStepBuf=prxbuf+5;
	pChnl->stepTotal=pStepBuf[0];
	
	//计算此次接收的公布数
	worksteps=prxbuf[0]+((mlu16)prxbuf[1]<<8);
	worksteps=(worksteps-5)/STEP_BYTES;

	//
	retFlag=OK_FLAG;
	
	//装载工步
	for(i=0;i<worksteps;i++)
	{

		if(pStepBuf[i*STEP_BYTES+1]>=CMB_CH_STEP_MAX)
		{
			retFlag=ERR_FLAG;
			break ;
		}
		 
		//工步索引=0 表示开始接收工步
		if(pStepBuf[i*STEP_BYTES+1]==0)
		{
			pChnl->rcvedStepCnt=0;
		}
		
		if(pStepBuf[i*STEP_BYTES+1]==pChnl->rcvedStepCnt)
		{
			n=pChnl->rcvedStepCnt;
				
			pChnl->steps[n].type=pStepBuf[i*STEP_BYTES+2];
			//2345 mpara1
			u8_to_u32(pStepBuf[i*STEP_BYTES+3], pStepBuf[i*STEP_BYTES+4], 
						pStepBuf[i*STEP_BYTES+5], pStepBuf[i*STEP_BYTES+6], temp1);

			if((pChnl->steps[n].type==STEP_CCCV)||(pChnl->steps[n].type==STEP_CCD)
				||(pChnl->steps[n].type==STEP_DCIR)||(pChnl->steps[n].type==STEP_CVPC)
				||(pChnl->steps[n].type==STEP_CCPC)||(pChnl->steps[n].type==STEP_CVD)||(pChnl->steps[n].type==STEP_CCCV2))
			{
				pChnl->steps[n].mPara1=temp1/10;
			}
			else
			{
				pChnl->steps[n].mPara1=temp1;
			}
			
			// 6789 lpara1
			u8_to_u32( pStepBuf[i*STEP_BYTES+7], pStepBuf[i*STEP_BYTES+8],
			pStepBuf[i*STEP_BYTES+9], pStepBuf[i*STEP_BYTES+10],temp1);
			if(pChnl->steps[n].type==STEP_JUMP)
			{
				pChnl->steps[n].lPara1=temp1;
			}
			else
			{
				pChnl->steps[n].lPara1=temp1/10;
			}
			pChnl->steps[n].lParaAction[0]=pStepBuf[i*STEP_BYTES+11];	
			pChnl->steps[n].lParaGotoStepNo[0]=pStepBuf[i*STEP_BYTES+12];		

			// 12 -19 lpara2 u64
			u8_to_u32( pStepBuf[i*STEP_BYTES+17], pStepBuf[i*STEP_BYTES+18], 
			pStepBuf[i*STEP_BYTES+19],pStepBuf[i*STEP_BYTES+20], temp1);
			pChnl->steps[n].lPara2=temp1;
			pChnl->steps[n].lPara2=(pChnl->steps[n].lPara2<<32);			

			u8_to_u32(pStepBuf[i*STEP_BYTES+13], pStepBuf[i*STEP_BYTES+14], 
				pStepBuf[i*STEP_BYTES+15],pStepBuf[i*STEP_BYTES+16],  temp1);				
			pChnl->steps[n].lPara2+=temp1;		
			
			pChnl->steps[n].lParaAction[1]=pStepBuf[i*STEP_BYTES+21];
			pChnl->steps[n].lParaGotoStepNo[1]=pStepBuf[i*STEP_BYTES+22];

			// 22 -25 lpara3
			u8_to_u32( pStepBuf[i*STEP_BYTES+23], pStepBuf[i*STEP_BYTES+24], 
			pStepBuf[i*STEP_BYTES+25],pStepBuf[i*STEP_BYTES+26], temp1);		
			pChnl->steps[n].lPara3=temp1;
			pChnl->steps[n].lParaAction[2]=pStepBuf[i*STEP_BYTES+27];
			pChnl->steps[n].lParaGotoStepNo[2]=pStepBuf[i*STEP_BYTES+28];

			//28-31 lpara4
			u8_to_u32( pStepBuf[i*STEP_BYTES+29], pStepBuf[i*STEP_BYTES+30], 
				pStepBuf[i*STEP_BYTES+31], pStepBuf[i*STEP_BYTES+32],temp1);			
			pChnl->steps[n].lPara4=temp1/10;
			pChnl->steps[n].lPara4=pStepBuf[i*STEP_BYTES+33];
			pChnl->steps[n].lPara4=pStepBuf[i*STEP_BYTES+34];	

			// 34-37
			u8_to_u32(pStepBuf[i*STEP_BYTES+35], pStepBuf[i*STEP_BYTES+36], 
				pStepBuf[i*STEP_BYTES+37],pStepBuf[i*STEP_BYTES+38],  temp1);
			pChnl->steps[n].sampleInterval=temp1;

			// 38-41
			u8_to_u32(pStepBuf[i*STEP_BYTES+39], pStepBuf[i*STEP_BYTES+40], 
				pStepBuf[i*STEP_BYTES+41],pStepBuf[i*STEP_BYTES+42],  temp1);
// 				if(pChnl->steps[n].type==STEP_DCIR)
					pChnl->steps[n].mPara2=temp1;

			// 42-45
			u8_to_u32(pStepBuf[i*STEP_BYTES+43], pStepBuf[i*STEP_BYTES+44],
				pStepBuf[i*STEP_BYTES+45], pStepBuf[i*STEP_BYTES+46], temp1);
// 				if(pChnl->steps[n].type==STEP_DCIR)
					pChnl->steps[n].mPara3=temp1;

			// 46-49
			u8_to_u32( pStepBuf[i*STEP_BYTES+47], pStepBuf[i*STEP_BYTES+48], 
					pStepBuf[i*STEP_BYTES+49], pStepBuf[i*STEP_BYTES+50],temp1);
			pChnl->steps[n].mPara4=temp1;

			// 50-53
			u8_to_u32(pStepBuf[i*STEP_BYTES+51], pStepBuf[i*STEP_BYTES+52], 
					pStepBuf[i*STEP_BYTES+53],pStepBuf[i*STEP_BYTES+54],  temp1);
			pChnl->steps[n].mPara5=temp1;		
			
			pChnl->rcvedStepCnt++;

		}
		else
		{
			retFlag=ERR_FLAG;
			break ;
		}
		
	}

	cmb_ntp_cmd_reply(CMD_WorkStepInfo, prxbuf[2],prxbuf[4] ,retFlag,0);//接收返回
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
unsigned char cmb_upload_stack(void *args)
{
	mlu8 i,*ptxbuf,*prxbuf;
	mlu16 len ;
	prxbuf=(mlu8*)args;
	//设备地址过滤
	//packet len take 0-1 2byte
	//offset 3 is cmd;  offset  2 is address;
	CmbPwrBoxptr pbox=cbl_power_box_get(prxbuf[2]);
	if(nullptr==pbox)
	{
		return mltrue;
	}
	
	if(pbox->online==_reset||prxbuf[4]>=pbox->chnlNum)//设备是否在线
	{
		return mltrue;
	}


	//	1	帧头	4	0xEE	起始字节
	//	2	数据长度	2		不包括本身的2Bytes
	//	3	箱号地址	1		箱号0~15
	//	4	命令类型	1	'H'	取通道跳转堆栈信息命令字
	//	5	获取跳转堆栈信息通道	1		0~7
	//	6	获取跳转堆栈信息是否成功	1		'O ': 成功
	//														'E': 失败
	//	7	上传通道异常信息	1		
	//	8	获取通道循环嵌套深度	1		最大值10
	//	9	获取每级深度下的跳转工步索引	1		
	//	10	获取每级深度下的跳转工步剩余次数	2		
	//	11	重复9,10步骤（10-1）	3*9		
	//	13	校验码	1	CHK	从箱号地址到数据体的校验和
		
//	len=4+2+6+10*3+1;
	len=43;
	//获取发送缓冲区
	ptxbuf=upper_txbuf_malloc(len);
	if(ptxbuf==_NULL)
		return 1;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	//数据包数据长度
	ptxbuf[4]=((len-6)&0xff);
	ptxbuf[5]=(((len-6)>>8)&0xff);

	//地址域
	ptxbuf[6]=prxbuf[2];
	
	//命令字
	ptxbuf[7]=CMD_GetStack;	

	ptxbuf[8]=prxbuf[4];
	
	ptxbuf[9]=OK_FLAG;
	
	ptxbuf[10]=pbox->chnl[prxbuf[4]].DDC;

	//堆栈深度
	ptxbuf[11]=pbox->chnl[prxbuf[4]].stackDepth;
	
	//堆栈信息
	for(i=0;i<30;i++)
	{
		ptxbuf[12+i*3+0]=pbox->chnl[prxbuf[4]].stack[i].pushStepNumber;
		ptxbuf[12+i*3+1]=pbox->chnl[prxbuf[4]].stack[i].popValue;
		ptxbuf[12+i*3+2]=(pbox->chnl[prxbuf[4]].stack[i].popValue>>8);	
	}
	
	ptxbuf[42]=0;// 
	for(i=0;i<36;i++)
	{
		ptxbuf[42]+=ptxbuf[6+i];
	}
		

	return 1; 
}

#if 0
/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明：wcsi ----eork condition simulation info
****************************************************************************/
// 1	帧头	4	0xEE	起始字节
// 2	数据长度	2		
// 3	箱号地址	1		箱号0~15
// 4	命令类型	1	'W'	
// 5	通道号	1		
// 6	工况模拟工步号	1		
// 7	剩余工步储存空间	2		下位机还可以缓存的工步数
// 8	工步配置是否成功	1		'O ': 成功
// 										'E': 失败
// 9	校验码	1	CHK	从箱号地址到数据体的校验和
//----------------------------------------------------------------------------
void ntp_wcsi_reply(u8 addr,u8 chnlx,u8 reflag,u8 belongStep,u32 serial,u32 substepcnt)
{
	u8 i,*ptxbuf;


	// 4+2+1+1+1+1+2+1+1=14
	//获取可使用发送缓冲区首地址

	ptxbuf=upper_txbuf_malloc(14);
	if(ptxbuf==_NULL)
		return;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;

	//长度,地址，命令字
	ptxbuf[4]=8;
	ptxbuf[5]=0;
		
	ptxbuf[6]=addr;
	ptxbuf[7]=CMD_WCSInfo;
	ptxbuf[8]=chnlx;
	ptxbuf[9]=belongStep;

	//
	ptxbuf[10]=serial;
	ptxbuf[11]=(serial>>8);

	//reply flag
	ptxbuf[12]=reflag;
	
	ptxbuf[13]=substepcnt;	
	ptxbuf[14]=(substepcnt>>8);
	
	
	//sum check
	ptxbuf[15]=0;
	for(i=0; i<9; i++)
	{
		ptxbuf[15]+= ptxbuf[6+i];
	}	
	
	//需要发送的数据类型
	ntp_tx_set(16,_tcpData);
	w5100_tx_data();


}

/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明： 
****************************************************************************/
void cblntp_req_sub_step(u8 equipAddr,u8 chnlx, u8 mainStepNum )
{
	u8 i,*ptxbuf;
	u16 len ;
	
	
//	len=4+2+addr+cmd+chnlx+stepNum+sumChck;
	len=11;

	//获取发送缓冲区
	ptxbuf=upper_txbuf_malloc(len);
	if(ptxbuf==_NULL)
		return;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	//数据包数据长度
	ptxbuf[4]=((len-6)&0xff);
	ptxbuf[5]=(((len-6)>>8)&0xff);

	//地址域
	ptxbuf[6]=equipAddr;
	
	//命令字
	ptxbuf[7]=CMD_ReqSubStepInfo;	

	ptxbuf[8]=chnlx;
	
	ptxbuf[9]=mainStepNum;
	

	ptxbuf[10]=0;
	for(i=0;i<4;i++)
		ptxbuf[10]+=ptxbuf[6+i];

	//需要发送的数据类型
	ntp_tx_set(11,_tcpData);
	w5100_tx_data();
	
}

/****************************************************************************
* 名	 称：
* 功	 能：
* 入口参数：
*		                 
* 出口参数：无
* 说	 明：
 ****************************************************************************/
// 2	数据长度	2		不包括本身的2Byte
// 3	箱号地址	1		箱号0~15
// 4	命令类型	1	'W'	
// 5	通道号	1		
// 6	工况模拟工步号	1		
// 7	序号	4		
// 8	工步数	1		
// 9	开始值	4		有符号
// 10	结束值	4		有符号
// 11	持续时间	2		单位ms
// 12	类型	1		电流或功率
// 12	重复9,11步骤			
//--------------------------------------------------------------------------------
unsigned char cblntp_receive_wcsi(void *args)
{

	u8 i,wcsiuIndex ,*prxbuf,err;
	u16 n;
	u32 serial;
 //  u8 chnlx;
	prxbuf=(u8*)args;
   //test
   
	//设备地址过滤
	//packet len take 0-1 2byte
	//offset 3 is cmd;  offset  2 is address;
  // chnlx=prxbuf[4];
	if((prxbuf[2]!=0xff)&&(prxbuf[2]!=attri.address))
	{
		return 1;
	}

	//通道是否在线
	// offset 4 byte is chnl ; 
	if((prxbuf[4]>=emu.igbt.onlineChnlNum)||(pbox->chnl[prxbuf[4]].flags.bit.online==_reset))
	{
		return 1;
	}
	
	//序列号
	u8_to_u32(prxbuf[6], prxbuf[7], prxbuf[8], prxbuf[9], serial);
	err=0;

	//开始接受模拟工况信息
	wcsiuIndex=0;
	if(serial==0)
	{
		pbox->chnl[prxbuf[4]].wcsiRcvSerial=0;
		pbox->chnl[prxbuf[4]].wcsMainStepNum=prxbuf[5];
		pbox->chnl[prxbuf[4]].wcsiu[0].subStepRcvAllCnt=0;
		pbox->chnl[prxbuf[4]].wcsiu[0].subStepNum=0;
		
		pbox->chnl[prxbuf[4]].wcsiu[1].subStepRcvAllCnt=0;
		pbox->chnl[prxbuf[4]].wcsiu[1].subStepNum=0;	
	}
	else
	{
		// 获取接受信息空间
		for(i=0;i<Igbt_WCSIU_Max;i++)
		{
			if(pbox->chnl[prxbuf[4]].wcsiu[i].subStepRcvAllCnt<Igbt_SubWorkStep_Max)
			{
				wcsiuIndex=i;
				break;
			}
		}
      //ERR_FLAG
		if(i>=Igbt_WCSIU_Max)
		{  
			err=1;
		}
		//ERR_FLAG
		//检查序列号
		if(serial!=pbox->chnl[prxbuf[4]].wcsiRcvSerial)
		{
			err=2;		
		}
		//ERR_FLAG
		//所属主工步是否相同
		if(pbox->chnl[prxbuf[4]].wcsMainStepNum!=prxbuf[5])	
		{
         //Add
			err=3;
		}
		
		if(err)
		{

			pbox->chnl[prxbuf[4]].Wrong_Serial=pbox->chnl[prxbuf[4]].SERIAL+prxbuf[10];
		
		   ntp_wcsi_reply(attri.address, prxbuf[4],err,
							   pbox->chnl[prxbuf[4]].wcsMainStepNum,pbox->chnl[prxbuf[4]].Wrong_Serial,
							   pbox->chnl[prxbuf[4]].wcsiRcvSerial); 

			return 1;
		}
		
	}

	// 开始装载信息para1 +para2+runTime+type=11Bytes;
	n=pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].subStepRcvAllCnt;
	for(i=0;i<prxbuf[10];i++)
	{  
		u8_to_u32(prxbuf[i*11+11+0], prxbuf[i*11+11+1], prxbuf[i*11+11+2],
			prxbuf[i*11+11+3], pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].substeps[n].para1);
		
		u8_to_u32(prxbuf[i*11+11+4], prxbuf[i*11+11+5], prxbuf[i*11+11+6],
			prxbuf[i*11+11+7], pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].substeps[n].para2);	
     
		u8_to_u16(prxbuf[i*11+11+8], prxbuf[i*11+11+9], 
				pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].substeps[n].runTime);

		pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].substeps[n].type=prxbuf[i*11+11+10];

		//处理参数
		if((pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].substeps[n].type==WCS_TYPE_CCC)||
			(pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].substeps[n].type==WCS_TYPE_CCD))
		{
			pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].substeps[n].para1/=10;
			pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].substeps[n].para2/=10;
		}
		
		n++;
      
		if(n>=Igbt_SubWorkStep_Max)
		{

			if(pbox->chnl[prxbuf[4]].reqSubStepSta==_wait)
			{
            
				//接收到子工步?
				pbox->chnl[prxbuf[4]].reqSubStepSta=_finished;
			}
			break;
		}
	}

	//
	pbox->chnl[prxbuf[4]].wcsiu[wcsiuIndex].subStepRcvAllCnt=n;
	pbox->chnl[prxbuf[4]].wcsiRcvSerial+=prxbuf[10];//prxbuf[10];
	//add
   pbox->chnl[prxbuf[4]].SERIAL=serial;
   
//	ntp_wcsi_reply(attri.address, prxbuf[4],OK_FLAG,
//					pbox->chnl[prxbuf[4]].wcsMainStepNum, pbox->chnl[prxbuf[4]].SERIAL,
//					pbox->chnl[prxbuf[4]].wcsiRcvSerial);
//	
	return 1;
	
}





/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明： 
****************************************************************************/
// 2	数据长度	2		不包括本身的2Byte
// 3	箱号地址	1		箱号0~15
// 4	命令类型	1	'L'	启动通道运行命令字
// 5	运行通道号	1		
// 6	运行工步号	1		
// 7	电脑同步时间	4		
//---------------------------------------------------------------------------
unsigned char cblntp_start_test(void  *args)
{	
	u8 *ptr,i,*ptxbuf,*prxbuf;
	u16 len ;
	u32 temp;
	
	prxbuf=(u8*)args;
	
	//设备地址过滤
	//packet len take 0-1 2byte
	//offset 3 is cmd;  offset  2 is address;
	if((prxbuf[2]!=0xff)&&(prxbuf[2]!=attri.address))
	{
		return 1;
	}

	//通道是否在线
	// offset 4 byte is chnl ; 
	if((prxbuf[4]>=emu.igbt.onlineChnlNum)||(pbox->chnl[prxbuf[4]].flags.bit.online==_reset))
	{
		return 1;
	}

	pbox->chnl[prxbuf[4]].stepNum=prxbuf[5];
	pbox->chnl[prxbuf[4]].startBy=StartBy_PC;
	pbox->chnl[prxbuf[4]].event=Event_StartTest;
	
	iChannel_goto_handle_event(&pbox->chnl[prxbuf[4]]);
	
	//ctp_start_chnl_test( addr,chnlx , * (prxbuf+5));

	// 命令回复
	// 1	帧头	4	0xEE	起始字节
	// 2	数据长度	2	5	
	// 3	箱号地址	1		箱号0~15
	// 4	命令类型	1	'L'	启动通道运行命令字
	// 5	运行通道号	1		
	// 6	电脑同步时间	4		
	// 7	下位机系统时间	6		
	// 8	当前通道最大存储地址	4		
	// 9	当前通道数据写入地址	4		
	// 8	通道启动是否成功	1		'O ': 成功'E': 失败
	// 9	校验码	1	CHK	从箱号地址到数据体的校验和

//	len=4+2+3+4+6+4+4+1+1;
	len=29;
	//获取发送缓冲区
	ptxbuf=upper_txbuf_malloc(len);
	if(ptxbuf==_NULL)
		return 1;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	//数据包数据长度
	ptxbuf[4]=((len-6)&0xff);
	ptxbuf[5]=(((len-6)>>8)&0xff);

	//地址域
	ptxbuf[6]=attri.address;

	//命令字
	ptxbuf[7]=CMD_StartChnl;	

	ptxbuf[8]=prxbuf[4];
	
	ptxbuf[9]=prxbuf[6];
	ptxbuf[10]=prxbuf[7];
	ptxbuf[11]=prxbuf[8];
	ptxbuf[12]=prxbuf[9];

	//系统时间
	ptr=(u8*)&sys.time;
	ptxbuf[13]=* ptr++;
	ptxbuf[14]=* ptr++;
	ptxbuf[15]=* ptr++;
	ptxbuf[16]=* ptr++;
	ptxbuf[17]=* ptr++;
	ptxbuf[18]=* ptr++;

	// falsh 最大地址
	len =CMB_CH_DAT_RCRD_CNT*CMB_CH_DAT_RCRD_LEN;
	
	ptxbuf[19]=(len&0xff);
	ptxbuf[20]=((len>>8)&0xff);
	ptxbuf[21]=((len>>16)&0xff);
	ptxbuf[22]=((len>>24)&0xff);

	//当前写入地址
	temp=pbox->chnl[prxbuf[4]].dataSaveEndAddr+
				(prxbuf[4]*CMB_CH_DAT_RCRD_CNT*CMB_CH_DAT_RCRD_LEN);
	
	u32_to_u8(temp, ptxbuf[23], ptxbuf[24], ptxbuf[25], ptxbuf[26]);

	ptxbuf[27]=OK_FLAG;

	ptxbuf[28]=0;
	for(i=0;i<22;i++)
		ptxbuf[28]+=ptxbuf[6+i];

	//需要发送的数据类型
	ntp_tx_set(29,_tcpData);
	w5100_tx_data();
	return 1;
	
}

/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明： 
****************************************************************************/
// 1	帧头	4	0xFF	起始字节
// 2	数据长度	2		不包括本身的2Byte
// 3	箱号地址	1		箱号0~15
// 4	命令类型	1	'P'	通道停止命令字
// 5	运行通道号	1		
//---------------------------------------------------------------------------
unsigned char cblntp_stop_test(void * args)
{	
	u8 *prxbuf;
//	u16 len ;
	prxbuf=(u8*)args;
	//设备地址过滤
	//packet len take 0-1 2byte
	//offset 3 is cmd;  offset  2 is address;
	if((prxbuf[2]!=0xff)&&(prxbuf[2]!=attri.address))
	{
		return 1;
	}

	//通道是否在线
	// offset 4 byte is chnl ; 
	if((prxbuf[4]>=emu.igbt.onlineChnlNum)||(pbox->chnl[prxbuf[4]].flags.bit.online==_no))
	{
		return 1;
	}

	//pbox->chnl[prxbuf[4]].stepNum=prxbuf[5];
	pbox->chnl[prxbuf[4]].stopBy=StopBy_PCForce;
	pbox->chnl[prxbuf[4]].event=Event_StopTest;
	pbox->chnl[prxbuf[4]].DDC=DDC_PC_FORCE_EXIT;
   
	iChannel_goto_handle_event(&pbox->chnl[prxbuf[4]]);

	//回复上位机
	ntp_v02_CMDReply(CMD_StopChnl,prxbuf[2],prxbuf[4],OK_FLAG,0);
	return 1;
	
}


/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明： 
****************************************************************************/
// 2	数据长度	2		不包括本身的2Byte
// 3	箱号地址	1		箱号0~15
// 4	命令类型	1	'N'	启动通道运行命令字
// 5	运行通道号	1		
// 6	运行工步号	1		
// 7	电脑同步时间	4		
//---------------------------------------------------------------------------
unsigned char cblntp_force_jump(void *args)
{	
	u8 *ptr,i,*ptxbuf,*prxbuf;
	u16 len ;
	u32 temp;
	prxbuf=(u8*)args;
	//设备地址过滤
	//packet len take 0-1 2byte
	//offset 3 is cmd;  offset  2 is address;
	if((prxbuf[2]!=0xff)&&(prxbuf[2]!=attri.address))
	{
		return 1;
	}

	//通道是否在线
	// offset 4 byte is chnl ; 
	if((prxbuf[4]>=emu.igbt.onlineChnlNum)||(pbox->chnl[prxbuf[4]].flags.bit.online==_no))
	{
		return 1;
	}

	pbox->chnl[prxbuf[4]].jumpToStepNum=prxbuf[5];
	pbox->chnl[prxbuf[4]].startBy=StartBy_PCForceStepJump;
	pbox->chnl[prxbuf[4]].DDC=DDC_FORCE_JUMP;
	pbox->chnl[prxbuf[4]].event=Event_StartTest;
	iChannel_goto_handle_event(&pbox->chnl[prxbuf[4]]);

	// 命令回复
	// 1	帧头	4	0xEE	起始字节
	// 2	数据长度	2	5	
	// 3	箱号地址	1		箱号0~15
	// 4	命令类型	1	'N'	启动通道运行命令字
	// 5	运行通道号	1		
	// 6	电脑同步时间	4		
	// 7	下位机系统时间	6		
	// 8	当前通道最大存储地址	4		
	// 9	当前通道数据写入地址	4		
	// 8	通道启动是否成功	1		'O ': 成功'E': 失败
	// 9	校验码	1	CHK	从箱号地址到数据体的校验和

//	len=4+2+3+4+6+4+4+1+1;
	len=29;
	//获取发送缓冲区
	ptxbuf=upper_txbuf_malloc(len);
	if(ptxbuf==_NULL)
		return 1;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	//数据包数据长度
	ptxbuf[4]=((len-6)&0xff);
	ptxbuf[5]=(((len-6)>>8)&0xff);

	//地址域
	ptxbuf[6]=attri.address;

	//命令字
	ptxbuf[7]=CMD_ForceJump;	

	ptxbuf[8]=prxbuf[4];
	
	ptxbuf[9]=* (prxbuf+6);
	ptxbuf[10]=* (prxbuf+7);
	ptxbuf[11]=* (prxbuf+8);
	ptxbuf[12]=* (prxbuf+9);

	//系统时间
	ptr=(u8*)&sys.time;
	ptxbuf[13]=* ptr++;
	ptxbuf[14]=* ptr++;
	ptxbuf[15]=* ptr++;
	ptxbuf[16]=* ptr++;
	ptxbuf[17]=* ptr++;
	ptxbuf[18]=* ptr++;

	// falsh 最大地址
	len =CMB_CH_DAT_RCRD_CNT*CMB_CH_DAT_RCRD_LEN;
	
	ptxbuf[19]=(len&0xff);
	ptxbuf[20]=((len>>8)&0xff);
	ptxbuf[21]=((len>>16)&0xff);
	ptxbuf[22]=((len>>24)&0xff);

	//当前写入地址
	temp=pbox->chnl[prxbuf[4]].dataSaveEndAddr+
				(prxbuf[4]*CMB_CH_DAT_RCRD_CNT*CMB_CH_DAT_RCRD_LEN);
	u32_to_u8(temp, ptxbuf[23], ptxbuf[24], ptxbuf[25], ptxbuf[26]);

	ptxbuf[27]=OK_FLAG;

	ptxbuf[28]=0;
	for(i=0;i<22;i++)
		ptxbuf[28]+=ptxbuf[6+i];

	//需要发送的数据类型
	ntp_tx_set(29,_tcpData);
	w5100_tx_data();
	return 1;
}

/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明：
*  
****************************************************************************/
unsigned char cblntp_reply_subStep_request(void *args)
{
	u8 *prxbuf;
//	u8 _ackFlag=_failure;
	prxbuf=(u8*)args;

	//len 'addr,cmd,chnl,
	
	if(prxbuf[2]!=attri.address)
	{
		return 1;
	}

	if(prxbuf[6]==0x01)
	{
  //    _ackFlag=_success;
		//请求成功	
	}
   
	return 1;

}

/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明：
*  
****************************************************************************/
unsigned char cblntp_lost_connect(void *args)
{
	u8 i;
	
	for(i=0;i<Igbt_Chnl_Max;i++)
	{
		pbox->chnl[i].DDC=DDC_NetLostConnect;
		if(pbox->chnl[i].flags.bit.running==_set)
		{
			pbox->chnl[i].event=Event_StopTest;
			pbox->chnl[i].stopBy=StopBy_Exception;
			iChannel_goto_handle_event(&pbox->chnl[i]);
		}
	}
	return 1;

}

/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明： 
* 
*ntp---- net transmit protocol 
****************************************************************************/
unsigned char cblntp_pause_test(void *args)
{
	u8 *ptxbuf,*prxbuf;
	u32 i,len ;

	prxbuf=(u8*)args;
//	u16 allBytes;
//	1	帧头	4	0xFF	起始字节
//	2	数据长度	2		不包括本身的2Bytes
//	3	箱号地址	1		箱号0~15
//	4	命令类型	1	'Q'	获取通道跳转堆栈信息命令字
//	5	通道号	1		0~7
//	6	加密	1		从命令类型开始数据加密
//	7	校验码	1	CHK	从箱号地址到数据体的校验和


	if((prxbuf[2]!=attri.address)||(emu.igbt.flags.bit.online==_reset)||(prxbuf[4]>=Igbt_Chnl_Max))
	{
		return 1 ;
	}

	//停止指定通道
	pbox->chnl[prxbuf[4]].DDC=DDC_PC_FORCE_EXIT;
	pbox->chnl[prxbuf[4]].event=Event_StopTest;
	pbox->chnl[prxbuf[4]].stopBy=StopBy_PCForce;
	iChannel_goto_handle_event(&pbox->chnl[prxbuf[4]]);

	//回复上位机
	// 4+2+chnlx+cmd+chnlx+flag+stackDep+stackInfo+sumCheck
	len=4+2+5+30+1;
		
	//获取发送缓冲区
	ptxbuf=upper_txbuf_malloc(len);
	if(ptxbuf==_NULL)
		return 1;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	//数据包数据长度
	ptxbuf[4]=((len-6)&0xff);
	ptxbuf[5]=(((len-6)>>8)&0xff);

	//地址域
	ptxbuf[6]=prxbuf[2];

	//命令字
	ptxbuf[7]=CMD_Pause;

	//设备通道号
	ptxbuf[8]=prxbuf[4];
	 ptxbuf[9]=OK_FLAG;
	 ptxbuf[10]=pbox->chnl[prxbuf[4]].stackDepth;
	 
	for(i=0;i<Stack_Depth_Max;i++)
	{
		 ptxbuf[11+i*3]=pbox->chnl[prxbuf[4]].stack[i].pushStepNumber;
		 ptxbuf[11+i*3+1]=pbox->chnl[prxbuf[4]].stack[i].popValue;
		 ptxbuf[11+i*3+2]=(pbox->chnl[prxbuf[4]].stack[i].popValue>>8);
	}

	
	//sum check
	ptxbuf[41]=0;
	for(i=0;i<(len-7);i++)
	{
		ptxbuf[41]+=ptxbuf[6+i];
	}
	
	//需要发送的数据类型
	ntp_tx_set(len,_tcpData);
	w5100_tx_data();

	return 1;
	
}

/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明： 
*   ie --igbt equipment  
*ntp---- net transmit protocol 
****************************************************************************/
// 1	帧头	4	0xFF	起始字节
// 2	数据长度	2		不包括本身的2Bytes
// 3	箱号地址	1		箱号0~15
// 4	命令类型	1	'G'	更新通道跳转堆栈信息命令字
// 5	运行通道号	1		
// 6	工步循环嵌套深度	1		最大值为10
// 7	跳转工步索引号	1		
// 8	跳转次数	2		
// 9	重复7,8步骤（10-1）	3*9	
// 10	工步最后容量	8		mams
// 11	工步剩余时间	4		ms
//12  续接的工不号 1byte
// 13	加密	1		从命令类型开始数据加密
// 14	校验码	1	CHK	从箱号地址到数据体的校验和
//---------------------------------------------------------------------------
unsigned char cblntp_continue_test(void *args)
{
	u8 *prxbuf,i,*ptxbuf;
	u32 temp;
	prxbuf=(u8*)args;

	if((prxbuf[2]!=attri.address)||(emu.igbt.flags.bit.online==_reset)||(prxbuf[4]>=Igbt_Chnl_Max))
	{
		return 1 ;
	}

	pbox->chnl[prxbuf[4]].stackDepth=prxbuf[5];
	for(i=0;i<Stack_Depth_Max;i++)
	{
		pbox->chnl[prxbuf[4]].stack[i].pushStepNumber=prxbuf[i*3+6];
		pbox->chnl[prxbuf[4]].stack[i].popValue=prxbuf[i*3+7];
		pbox->chnl[prxbuf[4]].stack[i].popValue+=((u16)prxbuf[i*3+8]<<8);
		
	}

	//容量，
	pbox->chnl[prxbuf[4]].prevCapacity=prxbuf[43];
	pbox->chnl[prxbuf[4]].prevCapacity=(pbox->chnl[prxbuf[4]].prevCapacity<<8)+prxbuf[42];
	pbox->chnl[prxbuf[4]].prevCapacity=(pbox->chnl[prxbuf[4]].prevCapacity<<8)+prxbuf[41];
	pbox->chnl[prxbuf[4]].prevCapacity=(pbox->chnl[prxbuf[4]].prevCapacity<<8)+prxbuf[40];
	pbox->chnl[prxbuf[4]].prevCapacity=(pbox->chnl[prxbuf[4]].prevCapacity<<8)+prxbuf[39];
	pbox->chnl[prxbuf[4]].prevCapacity=(pbox->chnl[prxbuf[4]].prevCapacity<<8)+prxbuf[38];
	pbox->chnl[prxbuf[4]].prevCapacity=(pbox->chnl[prxbuf[4]].prevCapacity<<8)+prxbuf[37];
	pbox->chnl[prxbuf[4]].prevCapacity=(pbox->chnl[prxbuf[4]].prevCapacity<<8)+prxbuf[36];
	
	//续接，工步剩余时间
	u8_to_u32(prxbuf[44], prxbuf[45], prxbuf[46], 
					prxbuf[47], pbox->chnl[prxbuf[4]].stepRemainTime);

	//续接的工不号
	pbox->chnl[prxbuf[4]].stepNum=prxbuf[48];
	pbox->chnl[prxbuf[4]].flags.bit.contiStart=_set;
	
	pbox->chnl[prxbuf[4]].event=Event_StartTest;
	pbox->chnl[prxbuf[4]].startBy=StartBy_PC;
	iChannel_goto_handle_event(&pbox->chnl[prxbuf[4]]);

	//回复上位机
	//获取可使用发送缓冲区首地址
	// 4+2+addr+cmd+chnlx+flag+4+sumCheck
	ptxbuf=upper_txbuf_malloc(15);
	if(ptxbuf==_NULL)
		return 1;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;

	//长度,地址，命令字
	ptxbuf[4]=9;
	ptxbuf[5]=0;
		
	ptxbuf[6]=attri.address;
	ptxbuf[7]=CMD_ContinueTest;
	ptxbuf[8]=prxbuf[4];
	// ok  or error flag 
	ptxbuf[9]=OK_FLAG;

		temp=pbox->chnl[prxbuf[4]].dataSaveEndAddr+
			((u32)CMB_CH_DAT_RCRD_CNT*CMB_CH_DAT_RCRD_LEN*prxbuf[4]);
	
	u32_to_u8(temp, ptxbuf[10], ptxbuf[11], ptxbuf[12], ptxbuf[13]);
	
	//sum check
	ptxbuf[14]=0;
	for(i=0; i<8; i++)
	{
		ptxbuf[14]+= ptxbuf[6+i];
	}	
	
	//
		//需要发送的数据类型
	ntp_tx_set(15,_tcpData);
	w5100_tx_data();	
  return 1;
}

/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明：
*  
****************************************************************************/
unsigned char cblntp_setup_parallel(void *args)
 {
	u8 *prxbuf,i,subChnlNum,*ptxbuf;
	u8 error,OorE;
	Module_TypeDef *pMdl;
	prxbuf=(u8*)args;
	
	
	error=0;
	OorE=OK_FLAG;

	//协议格式:
	// len(0-1) ,addr(2) ,cmd(3) ,mainBox_Chnl(4-5),subChnlCnt(6),subBox01_chnl01,....subBoxn_chnln,seed,sumchck

	if((prxbuf[4]!=attri.address)||(emu.igbt.flags.bit.online==_reset)
		||(prxbuf[5]>=Igbt_Chnl_Max)||(prxbuf[6]>=Igbt_Chnl_Max))
	{
		error=3;
		OorE=ERR_FLAG;
//		return 1 ;
	}
	
	//主通道是否已经发生并联了
	if((pbox->chnl[prxbuf[5]].flags.bit.subParall==_set)||(pbox->chnl[prxbuf[5]].flags.bit.mainParall==_set))
	{
		error=2;
		OorE=ERR_FLAG;
//		return 1;
	}
	
	//通道运行中
	if(pbox->chnl[prxbuf[5]].flags.bit.running==_set)
	{
		error=1;
		OorE=ERR_FLAG;
	}

	//检查所有的子通道是否已经被并联
	for(i=0;i<prxbuf[6];i++)
	{
		subChnlNum=prxbuf[7+i*2+1];
		if((pbox->chnl[subChnlNum].flags.bit.subParall==_set)||(pbox->chnl[subChnlNum].flags.bit.mainParall==_set))
		{
			error=2;
			OorE=ERR_FLAG;
//			return 1;
		}
	}
	
	if(error==0&&OorE==OK_FLAG)
	{
		//整里主通道的，自身模块
		pbox->chnl[prxbuf[5]].mdlMU.parallNum=0;
		pbox->chnl[prxbuf[5]].mdlMU.pExListHead=_NULL;
		pMdl=pbox->chnl[prxbuf[5]].mdlMU.pListHead;
		while(pMdl!=_NULL)
		{
			module_extend_insert(&pbox->chnl[prxbuf[5]].mdlMU, pMdl);
			pMdl->flags.bit.paralleled=_set;
			pMdl->pParallToIgbtChnl=&pbox->chnl[prxbuf[5]];
			pMdl=pMdl->pNext;		
		}
		
		//取并联的子通道
		for(i=0;i<prxbuf[6];i++)
		{
			subChnlNum=prxbuf[7+i*2+1];
			//将子通道的默认模块，并联到主通道，从而完成并联
			pMdl=pbox->chnl[subChnlNum].mdlMU.pListHead;
			while(pMdl!=_NULL)
			{
				module_extend_insert(&pbox->chnl[prxbuf[5]].mdlMU, pMdl);
				pMdl->flags.bit.paralleled=_set;
				pMdl->pParallToIgbtChnl=&pbox->chnl[prxbuf[5]];
				pMdl=pMdl->pNext;		
			}
			pbox->chnl[subChnlNum].flags.bit.subParall=_set;	
		}

		//主并联通道
		pbox->chnl[prxbuf[5]].flags.bit.mainParall=_set;
	}

	
	
	//回复上位机
	//获取发送缓冲区
	ptxbuf=upper_txbuf_malloc(13);
	if(ptxbuf==_NULL)
		return 1;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	//数据包数据长度
	ptxbuf[4]=7;
	ptxbuf[5]=0;

	//地址域
	ptxbuf[6]=prxbuf[2];

	//命令字
	ptxbuf[7]=CMD_SetupParallel;	

	//主通道的地址和通道号
	ptxbuf[8]=prxbuf[4];
	ptxbuf[9]=prxbuf[5];
	
	ptxbuf[10]=OorE;

	ptxbuf[11]=error;
	ptxbuf[12]=0;
	
	for(i=0;i<6;i++)
		ptxbuf[12]+=ptxbuf[6+i];
	
	//需要发送的数据类型
	ntp_tx_set(13,_tcpData);
	w5100_tx_data();	
	return 1;
	
}

/****************************************************************************
* 名    称：
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明：
*  
****************************************************************************/
unsigned char cblntp_destroy_parallel(void *args)
{
	u8 *prxbuf,i,*ptxbuf;
	u8 error, OorE;
	Module_TypeDef *pMdl;

	
	prxbuf=(u8*)args;
	error=0;
	OorE=OK_FLAG;

	//len ,addr(2),cmd,box,chnl,seed,sumchck
	if((prxbuf[4]!=attri.address)||(emu.igbt.flags.bit.online==_reset)
		||(prxbuf[5]>=Igbt_Chnl_Max))
	{
		error=3;
		OorE=ERR_FLAG;
//		return 1 ;
	}
	
	//通道正在运行
	if(pbox->chnl[prxbuf[5]].flags.bit.running==_set)
	{
		error=1;
		OorE=ERR_FLAG;
	}
	
	//通道本身没有被绑定
	if(pbox->chnl[prxbuf[5]].flags.bit.mainParall==_reset&&pbox->chnl[prxbuf[5]].flags.bit.subParall==_reset)
	{
		error=2;
		OorE=ERR_FLAG;
	}
	
	if(error==0&&OorE==OK_FLAG)
	{
			//解绑
		pMdl=pbox->chnl[prxbuf[5]].mdlMU.pExListHead;
		while(pMdl!=_NULL)
		{
			pMdl->pBelongToIgbtChnl->flags.bit.mainParall=_reset;
			pMdl->pBelongToIgbtChnl->flags.bit.subParall=_reset;
			pMdl->pParallToIgbtChnl=_NULL;
			pMdl->flags.bit.paralleled=_reset;
			pMdl=pMdl->pExNext;			
		}
		
		//清理主通道
		pbox->chnl[prxbuf[5]].flags.bit.mainParall=_reset;
		pbox->chnl[prxbuf[5]].flags.bit.subParall=_reset;
		pbox->chnl[prxbuf[5]].mdlMU.pExListHead=_NULL;
		pbox->chnl[prxbuf[5]].mdlMU.parallNum=0;
	}

	

	//回复上位机
	//获取发送缓冲区
	ptxbuf=upper_txbuf_malloc(13);
	if(ptxbuf==_NULL)
		return 1;
	
	//包头
	ptxbuf[0]=0xee;
	ptxbuf[1]=0xee;
	ptxbuf[2]=0xee;
	ptxbuf[3]=0xee;
	
	//数据包数据长度
	ptxbuf[4]=7;
	ptxbuf[5]=0;

	//地址域
	ptxbuf[6]=prxbuf[2];

	//命令字
	ptxbuf[7]=CMD_DestroyParallel;	

	//主通道的地址和通道号
	ptxbuf[8]=prxbuf[4];
	ptxbuf[9]=prxbuf[5];
	
	ptxbuf[10]=OorE;
	
	ptxbuf[11]=error;

	ptxbuf[12]=0;
	for(i=0;i<6;i++)
		ptxbuf[12]+=ptxbuf[6+i];
	
	//需要发送的数据类型
	ntp_tx_set(13,_tcpData);
	w5100_tx_data();
	return 1;
	
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
//		清除所有通道并联
//
//修改履历：
//
//
//-----------------------------------------------------------------------------
MLBool cblntp_clear_all_parallel(void *args)
{
	//u8 i,j;
	u8 j;
	CmbPwrMdlptr *pMdl;
	
//	prxbuf=(u8*)args;
	//len ,addr(2),cmd,seed,sumchck


	//解绑
	for(j=0;j<Igbt_Chnl_Max;j++)
	{
		pMdl=pbox->chnl[j].mdlMU.pExListHead;
		while(pMdl!=_NULL)
		{
			pMdl->pBelongToIgbtChnl->flags.bit.mainParall=_reset;
			pMdl->pBelongToIgbtChnl->flags.bit.subParall=_reset;
			pMdl->pParallToIgbtChnl=_NULL;
			pMdl->flags.bit.paralleled=_reset;
			pMdl=pMdl->pExNext;			
		}
		pbox->chnl[j].mdlMU.parallNum=0;
		pbox->chnl[j].flags.bit.subParall=_reset;
		pbox->chnl[j].flags.bit.mainParall=_reset;
		pbox->chnl[j].mdlMU.pExListHead=_NULL;
	}
	
	return 1;
	
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
void cmb_ntp_init(void)
{

	upper_consumer_register(CMD_Connect,cmb_upper_connect);
	upper_consumer_register(CMD_Sample,cmb_upper_sample);
	upper_consumer_register(CMD_WorkStepInfo,cmb_receive_step);
#if 0
//ntp 事件消息
	
	message_map(CMD_WorkStepInfo, &cblntpMU.ntpMsgResponders[1],cmb_receive_step, &ntpMU.messageSet);
	message_map(CMD_GetStack, &cblntpMU.ntpMsgResponders[2],cblntp_upload_stack, &ntpMU.messageSet);
	message_map(CMD_Sample, &cblntpMU.ntpMsgResponders[3],cblntp_sample, &ntpMU.messageSet);
	message_map(CMD_ForceJump, &cblntpMU.ntpMsgResponders[4],cblntp_force_jump, &ntpMU.messageSet);
	message_map(CMD_StartChnl, &cblntpMU.ntpMsgResponders[5],cblntp_start_test, &ntpMU.messageSet);
	message_map(CMD_StopChnl, &cblntpMU.ntpMsgResponders[6],cblntp_stop_test, &ntpMU.messageSet);
	message_map(CMD_WCSInfo, &cblntpMU.ntpMsgResponders[7],cblntp_receive_wcsi, &ntpMU.messageSet);
	message_map(CMD_ReqSubStepInfo, &cblntpMU.ntpMsgResponders[8],cblntp_reply_subStep_request, &ntpMU.messageSet);
	message_map(CMD_NetConnLost, &cblntpMU.ntpMsgResponders[9],cblntp_lost_connect, &ntpMU.messageSet);
	message_map(CMD_Pause, &cblntpMU.ntpMsgResponders[10],cblntp_pause_test, &ntpMU.messageSet);
	message_map(CMD_ContinueTest, &cblntpMU.ntpMsgResponders[11],cblntp_continue_test, &ntpMU.messageSet);
	message_map(CMD_SetupParallel, &cblntpMU.ntpMsgResponders[12],cblntp_setup_parallel, &ntpMU.messageSet);
	message_map(CMD_DestroyParallel, &cblntpMU.ntpMsgResponders[13],cblntp_destroy_parallel, &ntpMU.messageSet);
	message_map(CMD_ClearAllParall, &cblntpMU.ntpMsgResponders[14],cblntp_clear_all_parallel, &ntpMU.messageSet);
	message_map(ALARM_LIGHT_CMD, &cblntpMU.ntpMsgResponders[15],igbt_alarmled_cmd, &ntpMU.messageSet);
#endif
}


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif








