/**
 ******************************************************************************
 * 文件:mlos_port_define.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *		定义系统，移植到的mcu架构，跟mcu相关的基本数据类型定义
 *
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

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#if 1

#ifndef  _PORT_DEF_H__
#define  _PORT_DEF_H__

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//IDE compiler
//----------------------------------------------------------------------------

//keil 开发
//内联函数，关键字，重定义
#define _inline 		__inline //默认cc-ram

//----------------------------------------------------------------------------
//arm cortex-m
//----------------------------------------------------------------------------

typedef char mls8;
typedef unsigned char mlu8;
typedef volatile unsigned char mlvu8;
typedef short mls16;
typedef unsigned short mlu16;
typedef volatile unsigned short mlvu16;
typedef int mls32;
typedef unsigned int mlu32;
typedef volatile unsigned int mlvu32;
typedef float mlf32;

typedef unsigned long long mlu64;
typedef  long long mls64;


//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif

