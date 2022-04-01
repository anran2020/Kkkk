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
//  
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
//							end  of file
//----------------------------------------------------------------------------
#endif


