/**
 ******************************************************************************
 * 文件:mlos_dtd.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:  dtd :data type define
 *
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


#ifndef _MLOS_DATA_TYPE_DEF_H_
#define _MLOS_DATA_TYPE_DEF_H_

//---------------------------------------------------------
//include
//---------------------------------------------------------
#include "mlos_port_define.h"


//---------------------------------------------------------
//defines
//---------------------------------------------------------

//全局，和 本地私有，变量和函数前缀
#define _public 			
#define _private 	static


//空指针
#define nullptr 		0

//是否判断
#define _YES 	1
#define _NO 	0

typedef enum{_no=0,_yes}YN,yn;

//对错判断
#define _RIGHT 		1
#define _WRONG 		0
typedef enum{_wrong=0,_right}RigntWrongJudg;

//置位置，复位,标志的状态
#define _SET 		1
#define _RESET 		0
typedef enum{_reset=0,_set}MLFlagState;//标志状态

typedef enum{_now=0,_later}ActTime;

typedef enum{_disable=0,_enable}FuncState;//功能状态


typedef enum{_off=0,_on=1,_close=0,_open=1}SwitchState;//功能状态


//布尔变量数据类型定义
typedef enum {mlfalse = 0, mltrue } MLBool;

typedef enum{_disconnected=0,_connected,_unconfirmed}ConnectionStatus;//连接状态


//----------------------------------------------------------------------------
//基本数据类型，最大值
//----------------------------------------------------------------------------
#define MLU8_MAX 	((mlu8)0xff)
#define MLU16_MAX 	((mlu16)0xffff)
#define MLU32_MAX 	((mlu32)0xffffffff)

//----------------------------------------------------------------------------
//结构体继承，
//----------------------------------------------------------------------------
//father：父结构体，child:子结构体
//子结构体继承父结构体的属性
#define STRUCT_INHERIT(childStruct,fatherStruct)			fatherStruct(childStruct)


//----------------------------------------------------------------------------
//数据与数组之间的转换
//----------------------------------------------------------------------------
static inline void u8buf_to_dat(mulu8*u8buf,mlu8*pdat,mlu8 bytes)
{
    mlu32 u32Dat;
    u32Dat=u8buf[3];
    u32Dat=(u32Dat<<8)+u8buf[2];
    u32Dat=(u32Dat<<8)+u8buf[1];
    u32Dat=(u32Dat<<8)+u8buf[0];
    return u32Dat;
}

static inline void dat_to_u8buf(mlu8*dat,mulu8*u8buf,mlu8 bytes)
{
    u8buf[0]=u32Dat;
    u8buf[1]=(u32Dat>>8);
    u8buf[2]=(u32Dat>>16);
    u8buf[3]=(u32Dat>>24);
}


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif // 

