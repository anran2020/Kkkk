

#ifndef __TRAY_H__
#define __TRAY_H__

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
#include "cause.h"
#include "func.h"
#include "timer.h"
#include "log.h"
#include "entry.h"
#include "flow.h"



#ifdef __cplusplus
extern "C"  {
#endif

extern void trayNpRatioSet(u8 trayIdx, u8eNpOprCode npOpr, u8 sw, s16 npVal);
extern void trayNpSwProtDelay(Timer *timer);
extern void trayNpReachChk(Tray *tray);
extern void trayNpChnFree(Tray *tray, Channel *chn);
extern Ret trayNpChnAlloc(Tray *tray, Channel *chn, u8 stepId);
extern void trayNpSwRstDelay(Timer *timer);

#ifdef __cplusplus
}
#endif


#endif
