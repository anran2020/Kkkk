#ifndef _TIMER_H_
#define _TIMER_H_
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"


/*采样由定时器触发,每can一个独立,外加其余通用,共仨*/


#ifdef __LINUX_SYSTEM__
#if 0
typedef struct timeval Time;
#define TimeAdd(a, b, sum) \
do { \
    (sum)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
    (sum)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
    if ((sum)->tv_usec >= 1000000) \
    { \
        (sum)->tv_usec -= 1000000; \
        (sum)->tv_sec++; \
    } \
} while(0)

#define TimeSub(big, sml, diff) \
do { \
    if ((big)->tv_usec < (sml)->tv_usec) \
    { \
        (big)->tv_usec += 1000000; \
        (big)->tv_sec--; \
    } \
 \
    if ((big)->tv_sec < (sml)->tv_sec) \
    { \
        (diff)->tv_sec = 0;\
        (diff)->tv_usec = 0; \
    } \
    else \
    { \
        (diff)->tv_sec = (big)->tv_sec - (sml)->tv_sec; \
        (diff)->tv_usec = (big)->tv_usec - (sml)->tv_usec; \
    } \
} while(0)

/*a大于或等于b,则为真*/
#define TimeBiggerOrEqualThan(a, b) ((a)->tv_sec>(b)->tv_sec \
    || (a)->tv_sec==(b)->tv_sec&&(a)->tv_usec>=(b)->tv_usec)

/*a小于b,则为真*/
#define TimeSmallerThan(a, b)  (!TimeBiggerOrEqualThan(a, b))

/*a和b取大,若相等则取前者*/
#define TimeMaxer(a, b) (TimeBiggerOrEqualThan(a, b) ? a : b)
#endif
#else
#if 1
typedef struct timeTag
{
    s32 sec;
    u16 mSec;
    u16 rsvd;
}Times;
#define TimeAdd(a, b, sum) \
do { \
    (sum)->sec = (a)->sec + (b)->sec; \
    (sum)->mSec = (a)->mSec + (b)->mSec; \
    if ((sum)->mSec >= 1000) \
    { \
        (sum)->mSec -= 1000; \
        (sum)->sec++; \
    } \
} while(0)

#define TimeSub(big, sml, diff) \
do { \
    if ((big)->mSec < (sml)->mSec) \
    { \
        (big)->mSec += 1000; \
        (big)->sec--; \
    } \
    (diff)->sec = (big)->sec - (sml)->sec; \
    (diff)->mSec = (big)->mSec - (sml)->mSec; \
} while(0)

/*a大于b,则为真*/
#define TimeBiggerThan(a, b) ((a)->sec>(b)->sec \
        || (a)->sec==(b)->sec&&(a)->mSec>(b)->mSec)
    
/*a大于或等于b,则为真*/
#define TimeBiggerOrEqualThan(a, b) ((a)->sec>(b)->sec \
    || (a)->sec==(b)->sec&&(a)->mSec>=(b)->mSec)

/*a小于b,则为真*/
#define TimeSmallerThan(a, b) ((a)->sec<(b)->sec \
        || (a)->sec==(b)->sec&&(a)->mSec<(b)->mSec)

/*a和b取大,若相等则取前者*/
#define TimeMaxer(a, b) (TimeBiggerOrEqualThan(a, b) ? a : b)
#endif
#endif

typedef u8 u8eTimerType;
#define TidCanSmplLoop  0x00  /*can采样周期*/
#define TidConnUpper  0x01  /*联机上位机*/
#define TidCanRxCtrlAck  0x02  /*can启停等应答*/
#define TidCanRxSmplAck  0x03  /*can采样等应答*/
#define TidCanRxAuxAck  0x04  /*can修调升级等应答*/
#define TidUartSmplLoop  0x05  /*uart采样周期*/
#define TidUartRxBlkAck 0x06  /*uart转发等应答*/
#define TidUartRxCtrlAck 0x07  /*uart控制等应答*/
#define TidUartRxSmplAck 0x08  /*uart采样等应答*/
#define TidAppSw2Boot 0x09  /*转boot前短暂停留以备消息发出*/
#define TidPlcSmplLoop  0x0a  /*plc采样周期*/
#define TidPlcRxSmplAck 0x0b  /*plc采样等应答*/
#define TidPlcRxCtrlAck 0x0c  /*plc控制等应答*/
#define TidBootIdleStay 0x0d  /*boot内停留*/
#define TidBootSw2App 0x0e  /*转app前短暂停留以备消息发出*/
#define TidKeepAlive  0x0f  /*上位机保活*/
#define TidTrayProtEna 0x10  /*压合延时后保护*/
#define TidMixProtPolicyAct 0x11  /*组合保护策略延时*/
#define TidNpSwProtDelay 0x12  /*负压切换保护延时*/
#define TidNpSwRstDelay 0x13   /*针床脱开后恢复各个负压开关缺省*/
#define TimerIdCri  0x14

typedef struct
{
    Times exprTime;
    ChainD chain;
    u8eTimerType timerType;
    u8 rsvd[3];
}Timer;

#define TimerInit(tmr) ChainInitD(&(tmr)->chain)
#define TimerStop(tmr) ChainDelSafeD(&(tmr)->chain)
#define TimerIsRun(tmr) ChainInList(&(tmr)->chain)

typedef void (*TimerExprProc)(Timer *);

typedef struct
{
    ListD timerList;
    TimerExprProc exprProc[TimerIdCri];
}TimerMgr;

#ifdef __cplusplus
extern "C"  {
#endif

extern void timerInit(void);
extern void ctrlTimerTask(void);
extern void timerStart(Timer *timer, u8eTimerType type, u32 mSec, b8 reset);
extern void sysTimeGet(Times *time);
extern u32 sysTimeMsGet(void);
extern u32 sysTimeSecGet(void);

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译
#endif
