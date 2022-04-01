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
// 数据类型转换
//---------------------------------------------------------------------------- 
#define 	u16_to_u8(u16Dat,u8Dat1,u8Dat2)              u8Dat1= u16Dat;\
												u8Dat2= (u16Dat>>8)
												
#define 	u32_to_u8(u32Dat,u8Dat1,u8Dat2,u8Dat3,u8Dat4)	u8Dat1=(u32Dat&0xff);\
													u8Dat2=((u32Dat>>8)&0xff);\
													u8Dat3=((u32Dat>>16)&0xff);\
													u8Dat4=((u32Dat>>24)&0xff)

#define 	u32_to_u8buf(u32Dat,u8DatBuf)   u8DatBuf[0]=(u32Dat&0xff);\
										    u8DatBuf[1]=((u32Dat>>8)&0xff);\
										    u8DatBuf[2]=((u32Dat>>16)&0xff);\
										    u8DatBuf[3]=((u32Dat>>24)&0xff)
													
#define 	u64_to_u8buf(u64Dat,u8DatBuf)	u8DatBuf[0]=(u64Dat&0xff);\
										u8DatBuf[1]=((u64Dat>>8)&0xff);\
										u8DatBuf[2]=((u64Dat>>16)&0xff);\
										u8DatBuf[3]=((u64Dat>>24)&0xff);\
										u8DatBuf[4]=((u64Dat>>32)&0xff);\
										u8DatBuf[5]=((u64Dat>>40)&0xff);\
										u8DatBuf[6]=((u64Dat>>48)&0xff);\
										u8DatBuf[7]=((u64Dat>>56)&0xff)								
										
#define 	u8_to_u16(u8Dat1,u8Dat2,u16Dat)		u16Dat=u8Dat2;\
												u16Dat=(u16Dat<<8)+u8Dat1	
			
#define 	u8_to_u32(u8Dat1,u8Dat2,u8Dat3,u8Dat4,u32Dat)		u32Dat=u8Dat4;\
															u32Dat=(u32Dat<<8)+u8Dat3;\
															u32Dat=(u32Dat<<8)+u8Dat2;\
															u32Dat=(u32Dat<<8)+u8Dat1	
//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif


