/**
 ******************************************************************************
 * 文件:    
 * 作者:   
 * 版本:   V0.01
 * 日期:    
 * 内容简介:    
 *          
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                   创建该文件
 *
 *
 *
 ******************************************************************************
 **/
 
//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "ctp_can_driver.h"
#include "ctp_config.h"
#include "r_can.h"
#include "hal_data.h"

//----------------------------------------------------------------------------
//   define
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//   global variable 
//----------------------------------------------------------------------------


//接收缓存报文队列，由传输层申请，供底层驱动使用
Queptr pCANRxQue[CAN_NUM]={nullptr};

mlvu8 txCompletedFlag[CAN_NUM]={0};

can_instance_ctrl_t * pCANCtrl[]={&g_can0_ctrl,&g_can1_ctrl};


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
// 1.  zzx ：  2014-12-01  ：创建函数
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
// 1.  zzx ：  2014-12-01  ：创建函数
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
// 1.  zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
_inline void  can_rx(mlu8 cid,can_frame_t * prxframe)
{

	mlu16 u16temp;
	CANFrmptr pCanframe;

	//队列接收报文
	if(pCANRxQue[cid]!=nullptr)
	{
		pCanframe=que_en(pCANRxQue[cid]);
		if(pCanframe!=nullptr)
		{
			pCanframe->id=prxframe->id;
			pCanframe->data[0]=prxframe->data[0];
			pCanframe->data[1]=prxframe->data[1];
			pCanframe->data[2]=prxframe->data[2];
			pCanframe->data[3]=prxframe->data[3];
			pCanframe->data[4]=prxframe->data[4];
			pCanframe->data[5]=prxframe->data[5];
			pCanframe->data[6]=prxframe->data[6];
			pCanframe->data[7]=prxframe->data[7];
			pCanframe->sfRxIDbit.CANID=cid;
		}
	}
	else
	{
				//接收缓存已满，写日志
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
// 1.  zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
mlu8 can_tx(CANCardID cid,CANFrmptr pframe)
{
	mlu32 counter,i;
	
	//malbox 0 为发送box
	if (pCANCtrl[cid]->p_reg->MCTL_TX_b[0].TRMREQ!=0)
	{
		return 1;//
	}

	//
	pCANCtrl[cid]->p_reg->MB[0].ID=pframe->id;
	pCANCtrl[cid]->p_reg->MB[0].DL_b.DLC = 8;//固定长度8byte
	 for (i = 0; i < (mlu16)CAN_SF_DAT_LEN; ++i)
	 {
		pCANCtrl[cid]->p_reg->MB[0].D[i] = pframe->data[i];
	 }

	 txCompletedFlag[cid]=_reset;

	 pCANCtrl[cid]->p_reg->MCTL_TX[0] = 0x80;//启动发送

	 counter=(mlu16)100000;
	 //等待发送完成
	 while (counter>0&&txCompletedFlag[cid]==_reset)
	{
		 counter--;
	}

	if (txCompletedFlag[cid]==_reset)
	{
		 return 2;//发送超时
	}
	 
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
// 1.  zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void can_callback(can_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case CAN_EVENT_TX_COMPLETE:
        {
        	txCompletedFlag[p_args->channel]=_set;
            break;
        }

        case CAN_EVENT_RX_COMPLETE:
        {

           // memcpy(&g_can_rx_frame, p_args->p_frame, sizeof(can_frame_t));  //copy the received data to rx_frame
           can_rx(p_args->channel,p_args->p_frame);
            break;
        }

        case CAN_EVENT_MAILBOX_MESSAGE_LOST:    //overwrite/overrun error event
        case CAN_EVENT_BUS_RECOVERY:            //Bus recovery error event
        case CAN_EVENT_ERR_BUS_OFF:             //error Bus Off event
        case CAN_EVENT_ERR_PASSIVE:             //error passive event
        case CAN_EVENT_ERR_WARNING:             //error warning event
        {
            break;
        }

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
// 1.  zzx ：  2014-12-01  ：创建函数
//
//----------------------------------------------------------------------------- 
void can_init(CANCardID cid,mlu8 equipAddr,CANBaudRate baudrate,Queptr rxQue)
{

	if (cid==CAN_CARD_1)
	{
		R_CAN_Open(&g_can0_ctrl,&g_can0_cfg);
		
		pCANRxQue[cid]=rxQue;
		//g_can0_ctrl.p_reg->CTLR_b.CANM=CAN_OPERATION_MODE_HALT;
		//g_can0_ctrl.p_reg->TCR_b.TSTE=1;
		//g_can0_ctrl.p_reg->TCR_b.TSTM=2;
		//g_can0_ctrl.p_reg->CTLR_b.CANM=CAN_OPERATION_MODE_NORMAL;

		
	}
	else if(cid==CAN_CARD_2)
	{

		R_CAN_Open(&g_can1_ctrl,&g_can1_cfg);
		
		pCANRxQue[cid]=rxQue;

		//g_can1_ctrl.p_reg->CTLR_b.CANM=CAN_OPERATION_MODE_HALT;
		//g_can1_ctrl.p_reg->TCR_b.TSTE=1;
		//g_can1_ctrl.p_reg->TCR_b.TSTM=2;
		//g_can1_ctrl.p_reg->CTLR_b.CANM=CAN_OPERATION_MODE_NORMAL;
	}
	else
	{


	}

}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------


