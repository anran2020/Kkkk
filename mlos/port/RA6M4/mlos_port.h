/**
 ******************************************************************************
 * 文件:port.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
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

#ifndef  _PORT_H__
#define  _PORT_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------

#include "mlos_dtd.h"
#include "r_timer_api.h"
#include "r_gpt.h"
#include "hal_data.h"

//----------------------------------------------------------------------------
// mlos 需要用到的硬件 timer define
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
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
static _inline void  port_clock_us_delay(mlu16 us)
{
    us++;
}

//----------------------------------------------------------------------------
//export fun
//----------------------------------------------------------------------------

// 供 mlos clock 调用
void port_clock_init(void);

//给 mlos malloc 调用
void port_mem_init(void);

//任务用时测量计时器
void task_clock_start(void);
mlu16 task_clock_stop(void);

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif
