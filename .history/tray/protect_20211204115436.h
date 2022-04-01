
#ifndef _PROTECT_H_
#define _PROTECT_H_

#include "basic.h"
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

extern void mixProtCfgReset(MixProtCfg *mixProtCfg);
extern Ret protExpParse(u8 *exp, u16 expLen, MixProtCfg *mixProtCfg);
extern u16eRspCode protExpSave(u8 *cmd);
extern void trayProtEnable(Timer *timer);
extern void trayProtWorkBox(Tray *tray);
extern void trayProtWork(Tray *tray, b8 wiNdbd);
extern void saveGenProtCfg(u8 trayIdx, u8 amt, u8 *buf);
extern Ret chnProtReverse(Channel *chn);
extern void mixProtPolicyAct(Timer *timer);

#ifdef __cplusplus
}
#endif


#endif


