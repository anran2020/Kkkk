/**
 ******************************************************************************
 * 文件:  u_dtd.h  
 * 作者:   
 * 版本:   V0.01
 * 日期:    
 * 内容简介:    
 *          
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                           创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef  _U_DTD_H__
#define _U_DTD_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// 设备类型枚举
//---------------------------------------------------------------------------- 

typedef enum {

    DEV_Middle=0,//中位机
    DEV_ParallPower,
    DEV_SerialPower,
    DEV_SerialPowerPlus,
    DEV_CVBoard,
    DEV_TemBoard,
    DEV_VolBoard,
    DEV_needbedCtrlBoard,
    DEV_toolBoard,
    DEV_OCVBoard,
	DEV_NetParallPower,
    DEV_NULL,
    
}DeviceType;	

//----------------------------------------------------------------------------
//数据与数组之间的转换
//----------------------------------------------------------------------------
static inline mlu32 u8buf_to_u32(mulu8*u8buf,)
{
    mlu32 u32Dat;
    u32Dat=u8buf[3];
    u32Dat=(u32Dat<<8)+u8buf[2];
    u32Dat=(u32Dat<<8)+u8buf[1];
    u32Dat=(u32Dat<<8)+u8buf[0];
    return u32Dat;
}

static inline void u32_to_u8buf(mlu32 u32Dat,mulu8*u8buf)
{
    u8buf[0]=u32Dat;
    u8buf[1]=(u32Dat>>8);
    u8buf[2]=(u32Dat>>16);
    u8buf[3]=(u32Dat>>24);
}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


