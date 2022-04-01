/**
 ******************************************************************************
 * 文件:    ctp_candriver.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    
 * 内容简介:     通讯协议 消息 id 定义生声明
 *                   
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                                                            创建该文件
 *
 *
 *
 ******************************************************************************
 **/
#ifndef  _CTP_CAN_H__
#define _CTP_CAN_H__

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include"mlos.h"
#include"ctp_dtd.h"


//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------



//can编号
#define CAN1_No				0
#define CAN2_No 			1

#define  CAN_BAUDRATE   _500KBps_



//----------------------------------------------------------------------------
//协议数据帧定义
//----------------------------------------------------------------------------
//#pragma anon_unions
typedef struct CANFrameNode{

	//data，数据域
	union {
		
		mlu8 data[8];
		struct {
			mlu32 dataL;
			mlu32 dataH;
		};
				struct {
			mlu32 dataL;
			mlu32 dataH;
		};
		struct {
			mlu8 u8Data1;	
			mlu8 u8Data2;
			mlu8 u8Data3;
			mlu8 u8Data4;
			mlu8 u8Data5;	
			mlu8 u8Data6;
			mlu8 u8Data7;
			mlu8 u8Data8;
		};
		struct{

			mlu16 u16Data1;	
			mlu16 u16Data2;
			mlu16 u16Data3;
			mlu16 u16Data4;
		} ;
		
	};

	//id,不同的mcu，id定义不同
	union {
 
		mlu32 id;
	
		struct {

			mlu8 DATA9; 				//数据9
			mlu8 DATA10;				//数据10

			mlu16 PP:2;					//流水号，发送累加，接收原值返回
			mlu16 IDN:5;				//帧标示符(identifer number)，可定义0-31 值域的不同报文
			mlu16 ADDR:5;				//地址(address)，即可表示源地址，也可表示目的地址
			mlu16 PF:1;					//帧格式 PF=0:单帧;PF=1、2:多帧；
			
			mlu16 CANID:1;//报文的地址Can地址
			mlu16 EMPTY:1;//用作报文节点是否为空的标识//u32 RTR:1;//协议只使用数据帧
			mlu16 IDE:1;// identifier extension	0:标准帧；1:扩展真

		}sfRxIDbit;//单帧接收ID bit
		
		struct {
		
			mlu8 DATA9; 				//数据9
			mlu8 DATA10;				//数据10

			mlu16 PP:2; 				//参数页，不同IDN报文，携带的参数。
			mlu16 IDN:5;				//帧标示符(identifer number)，可定义0-31 值域的不同报文
			mlu16 ADDR:5;				//地址(address)，即可表示源地址，也可表示目的地址
			mlu16 PF:1; 				//帧格式 PF=0:单帧;PF=1、2:多帧；

			mlu16 ZERO:1;				//The write value should be 0
			mlu16 EMPTY:1;				//用作报文节点是否为空的标识//u32 RTR:1;//=0，标识数据帧，协议只使用数据帧
			mlu16 IDE:1;				// identifier extension	0:标准帧；1:扩展真
		
		}sfTxIDbit;
		
		 struct {
		
			mlu8 DATA9;					//数据9
			mlu8 DATA10; 					//数据10
			
			mlu16 INDX:7;					// 帧序
			mlu16 ADDR:5;
			mlu16 PF:1;						//帧格式 PF=0:单帧;PF=1、2:多帧；
			mlu16 CANID:1;					//报文的地址Can地址
			mlu16 EMPTY:1;					//用作报文节点是否为空的标识//u32 RTR:1;//协议只使用数据帧
			mlu16 IDE:1;					// identifier extension	0:标准帧；1:扩展真
		
		}mfRxIDbit;

		struct {

			mlu8 DATA9;						//数据9
			mlu8 DATA10; 					//数据10
			
			mlu16 INDX:7;					// 帧序
			mlu16 ADDR:5;
			mlu16 PF:1; 					//帧格式 PF=0:单帧;PF=1、2:多帧；
			mlu16 ZERO:1;					//The write value should be 0
			mlu16 EMPTY:1;					//用作报文节点是否为空的标识//u32 RTR:1;//协议只使用数据帧
			mlu16 IDE:1;					// identifier extension	0:标准帧；1:扩展真

		}mfTxIDbit;
	
	};
	
}CANFrame,*CANFrmptr;//协议帧



//----------------------------------------------------------------------------
// can 报文ID域 值设定接口
//----------------------------------------------------------------------------
//单帧id 设置
_inline static void can_sfid_set(CANFrmptr pFrame,mlu8 addr,mlu8 idn,mlu8 pp,mlu8 data9,mlu8 data10)
{
	pFrame->sfTxIDbit.PF=CAN_PF_SINGLE;
	pFrame->sfTxIDbit.IDN=idn;
	pFrame->sfTxIDbit.ADDR=addr;
	pFrame->sfTxIDbit.PP=pp;
	pFrame->sfTxIDbit.DATA9=data9;
	pFrame->sfTxIDbit.DATA10=data10;
	pFrame->sfTxIDbit.IDE=CAN_EXID;
	pFrame->sfTxIDbit.ZERO=0;
	pFrame->sfTxIDbit.EMPTY=0;//不为空

}

//多帧id设置
_inline static void can_mfid_set(CANFrmptr pFrame,mlu8 addr,mlu8 indx)
{
	pFrame->mfTxIDbit.PF=CAN_PF_MULTI;
	pFrame->mfTxIDbit.ADDR=addr;
	pFrame->mfTxIDbit.INDX=indx;
	pFrame->mfTxIDbit.IDE=CAN_EXID;
	pFrame->mfTxIDbit.ZERO=0;
	pFrame->mfTxIDbit.EMPTY=0;				//不为空

}

//-------------------------------------- -------------------------------------
//export   fucation 
//----------------------------------------------------------------------------
//can 初始化
void can_init(CANCardID cid,mlu8 equipAddr,CANBaudRate baudrate,Queptr rxQue);


//发送
mlu8 can_tx(CANCardID cid,CANFrmptr pframe);


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

#endif

