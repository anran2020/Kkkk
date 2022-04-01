#ifndef __CHANNEL_H__
#define __CHANNEL_H__


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

extern void chnEnterIdle(Channel *chn);
extern void chnStepSwPre(Channel *chn);
extern b8 chnStepBeJustNmlEnd(Channel *chn);

#ifdef __cplusplus
}
#endif



#endif

