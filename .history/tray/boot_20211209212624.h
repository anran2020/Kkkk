
#ifndef _BOOT_H_
#define _BOOT_H_
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"
#include "func.h"
#include "log.h"
#include "timer.h"


#ifdef __cplusplus
extern "C"  {
#endif


extern void bootIdleStayExpr(Timer *timer);
extern void devBootSw2App(Timer *timer);

#ifdef __cplusplus
}
#endif
//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译
#endif



