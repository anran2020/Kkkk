/**
 ******************************************************************************
 * 文件:tool_config.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *		
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----日期   ---作者    ----------   说明-------------------------------------
 * v0.1     2014-12-01   zzx           创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef  __TOOL_CONFIG_H__
#define  __TOOL_CONFIG_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------

#if 1

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

//工具使能模块使能
#define TOOL_ENABLE          (1)


//----------------------------------------------------------------------------
// boot app 编译选择
//----------------------------------------------------------------------------
#ifdef COMPILE_BOOT_CODE
//使能调试功能
#define TOOL_DEBUG_ENABLE          (0)

//使能升级功能
#define TOOL_IAP_ENABLE            (1)

//使能读取SD卡功能，即访问数据存储模块 DS
#define TOOL_DS_ENABLE            (0)

//使能中位机属性配置
#define TOOL_SETTING_ENABLE        (1)

#else
//使能调试功能
#define TOOL_DEBUG_ENABLE          (1)

//使能升级功能
#define TOOL_IAP_ENABLE            (1)

//使能读取SD卡功能，即访问数据存储模块 DS
#define TOOL_DS_ENABLE            (1)

//使能中位机属性配置
#define TOOL_SETTING_ENABLE            (1)

#endif

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif

