/**
 ******************************************************************************
 * 文件:mlos_mpump.c
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 * mpump->message pump,消息泵
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                       2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/


#ifndef _MLOS_MSG_PUMP_H_
#define _MLOS_MSG_PUMP_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------
#include "mlos_dtd.h"
#include "mlos_list.h"
#include "mlos_malloc.h"


//---------------------------------------------------------
//define
//---------------------------------------------------------



//---------------------------------------------------------
//消息处理回调函数
//---------------------------------------------------------

//消息回调函数
//参数：void*，消息的内容,需要被处理者分析处理
//返回值：MLBool，消息处理完返回mltrue，否则返回mlfalse，留给下一个消费者处理
typedef MLBool (*MsgCallbackFunction)(void*);


//---------------------------------------------------------
//消息标识 IDN---id number
//---------------------------------------------------------

//
typedef mlu8 MsgIDN;


//----------------------------------------------------------------------------
//消息消费者
//----------------------------------------------------------------------------
typedef struct MsgConsumerItem{
	
	STRUCT_INHERIT(MsgConsumerItem,SLListItemBaseStruct)//继承单向链表item属性，

	MsgCallbackFunction callback;						//消息处理，回调函数

}Consumer,*Consumerptr;

//----------------------------------------------------------------------------
//消息链表的节点 MLMessage ML前缀为区分，应用程序 
//----------------------------------------------------------------------------
//#pragma anon_unions
typedef struct MLMessageListItem{

	STRUCT_INHERIT(MessageListItem,SLListItemBaseStruct)//继承单向链表item属性，
	mlu8 idn;										//消息id number，
	mlu8 u8rsv;										//未使用字段
	mlu16 u16rsv;									//未使用字段
	ListPtr pConsumerList;							//消息消费链表，一个消息可以被多个对象消费

}MLMessage,*MLMsgptr;

//----------------------------------------------------------------------------
//消息数组的项
//----------------------------------------------------------------------------
//#pragma anon_unions
typedef struct {

	ListPtr pConsumerList; 				//消息消费链表，一个消息可以被多个对象消费

	//MsgCallbackFunction callback; 		//一个消息，对应一个消费者
	
}MessageArrayItem,MsgItem,*MsgItemptr,*MessageArrayptr;

//----------------------------------------------------------------------------
//消息泵类型枚举
//----------------------------------------------------------------------------
enum{

	e_pump_list=0,				//链表管理消息，消息id可以不连续
	e_pump_array, 				//数组管理消息，消息id连续，消息效率高，
		
};

//----------------------------------------------------------------------------
//消息泵，消息循环分派给消费者处理
//----------------------------------------------------------------------------
//#pragma anon_unions
typedef struct{

	mlu8  type;								//类型
	mlu8  msgMax;							//消息最大数
	
	union{
		ListPtr pMymsglist; 							//消息链表
		MessageArrayptr msgArray;						//消息数组
	};	

}MessagePump,*MsgPumpptr;


//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//extern function
//----------------------------------------------------------------------------

//生成一个消息泵
MsgPumpptr mpump_create(MemoryType mt,mlu8 msgMax);

//消息消费者，登记
void mpump_consumer_register(MsgPumpptr pMsgpump,MsgIDN     idn,MsgCallbackFunction callback);

//消息分派，给消费者
MLBool mpump_dispatch(MsgPumpptr pMsgpump,MsgIDN        idn,void * args);



//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif // 




