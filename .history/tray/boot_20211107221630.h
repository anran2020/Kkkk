
#ifndef _BOOT_H_
#define _BOOT_H_

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

#endif



