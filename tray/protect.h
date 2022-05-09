
#ifndef _PROTECT_H_
#define _PROTECT_H_

#include "basic.h"
#if TRAY_ENABLE
#include "define.h"
#include "enum.h"
#include "type.h"
#include "cause.h"
#include "func.h"
#include "timer.h"
#include "log.h"
#include "flow.h"
#include "entry.h"

#ifdef __cplusplus
extern "C"  {
#endif

extern void trayProtEnable(Timer *timer);
extern void trayProtWorkBox(Tray *tray);
extern Ret chnProtReverse(Channel *chn);
extern void mixProtPolicyAct(Timer *timer);
extern void chnProtWork(Channel *chn);
extern void setChnAbnml(Channel *chn, u16eCauseCode causeCode);
extern void trayNpBigRiseRst(Tray *tray);

#ifdef __cplusplus
}
#endif

#endif
#endif


