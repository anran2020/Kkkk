#ifndef __CHANNEL_H__
#define __CHANNEL_H__


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

typedef void (*ChnStaMap)(Channel *, void *);

typedef struct
{
    ChnStaMap chnStaMap[ChnMedStaCri];
}ChnMgr;

#ifdef __cplusplus
extern "C"  {
#endif

extern void cellEnterIdle(Cell *cell);
extern void chnEnterIdle(Channel *chn);
extern void chnStepSwPre(Channel *chn);
extern void chnLowSmplProc(Channel *chn, void *upSmpl, void *lowSmpl);
extern void chnInit(void);
extern u8eUpChnState chnStaMapMed2Up(Channel *chn);
extern void chnStopByProt(Channel *chn, b8 beTrayProt);
extern b8 chnBeInIdle(Channel *chn, ChnProtBuf *protBuf);
extern b8 chnBeInRun(Channel *chn, ChnProtBuf *protBuf);
extern u8eChgType stepType2ChgType(u8eStepType stepType);
extern void chnProtBufInit(ChnProtBuf *chnProtBuf);
extern void chnSaveTraySmplOld(Channel *chn, u16eCauseCode cause);
extern void chnEndCapCalc(Channel *chn);

#ifdef __cplusplus
}
#endif


#endif
#endif

