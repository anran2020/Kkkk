

#ifndef __TRAY_H__
#define __TRAY_H__


#include "basic.h"
#if TRAY_ENABLE
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

extern void trayNpRatioSet(u8 trayIdx, u8eNpOprCode npOpr, u8 swVal, s16 npVal);
extern void trayNpSwProtDelay(Timer *timer);
extern void trayNpReachChk(Tray *tray);
extern void trayNpChnFree(Tray *tray, Channel *chn);
extern Ret trayNpChnAlloc(Tray *tray, Channel *chn, u8 stepId);
extern void trayNpSwRstDelay(Timer *timer);
extern b8 trayChkStepNpSame(Channel *chn, u8 stepIdPre, u8 stepIdNew);
extern void traySmplSetAux(Tray *tray);
extern void trayStopByDisk(Tray *tray, u16eCauseCode causeCode);
extern void trayNpReset(u8 trayIdx);
extern void trayIgnoreNdbd(void);
extern void trayStopByProt(Tray *tray, u16eCauseCode causeCode);
extern void trayNdbdBreakChk(Timer *timer);
extern void trayNdbdBreak(u8 trayIdx);
extern void trayNdbdCtrl(u8 trayIdx, u8eNdbdCtrlType type, s16 oprVal);
extern b8 trayNdbdBeOnline(u8 trayIdx);

#ifdef __cplusplus
}
#endif

#endif
#endif
