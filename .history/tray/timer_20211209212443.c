
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
#include "box.h"
#include "host.h"
#include "uart.h"
#include "plc.h"
#include "boot.h"
#include "protect.h"
#include "tray.h"

#ifdef DebugVersion
#else
#include "mlos_clock.h"
#endif

TimerMgr *gTimerMgr;

void exprIgnore()
{
    return;
}

u32 sysTimeSecGet()
{
#ifdef DebugVersion
    return 0;
#else
    return (mlos_time())->s;
#endif
}

u32 sysTimeMsGet()
{
#ifdef DebugVersion
    return 0;
#else
    return mlos_ms_clock();
#endif
}

void sysTimeGet(Times *time)
{
#ifdef DebugVersion
    time->sec = 0;
    time->mSec = 0;
#else
    memcpy(time, mlos_time(), sizeof(Times));
#endif
}

/*
reset-mode,复位表示重新计时，loop-mode表示周期循环计时
链表由小到大排序
*/
void timerStart(Timer *timer, u8eTimerType type, u32 mSec, b8 reset)
{
    ChainD *chain;
    ChainD *tmpChn;
    Timer *bigger;
    ListD *list;
    Times *exprTime;

    exprTime = &timer->exprTime;
    if (reset)
    {
        sysTimeGet(exprTime);
    }

    exprTime->mSec += mSec;
    if (exprTime->mSec >= 1000)
    {
        exprTime->sec += exprTime->mSec/1000;
        exprTime->mSec %= 1000;
    }

    timer->timerType = type;
    list = &gTimerMgr->timerList;
    ListForEachSafe(list, chain, tmpChn)
    {
        bigger = Container(Timer, chain, chain);
        if (TimeSmallerThan(exprTime, &bigger->exprTime))
        {
            break;
        }
    }

    if (ChainInList(&timer->chain))
    {
        ChainDeleteD(&timer->chain);
    }

    ChainInsertD(chain, &timer->chain);
    return;
}

/*核心逻辑任务定时器调用入口*/
/*链表由小到大排序,所以,遇到未超时即终止遍历*/
void ctrlTimerTask()
{
    ChainD *chain;
    ChainD *tmpChn;
    ListD *list;
    Timer *timer;
    Times crntTime;

    sysTimeGet(&crntTime);
    list = &gTimerMgr->timerList;
    ListForEachSafe(list, chain, tmpChn)
    {
        timer = Container(Timer, chain, chain);
        if (TimeBiggerThan(&timer->exprTime, &crntTime))
        {
            return;
        }

        ChainDelSafeD(chain);
        gTimerMgr->exprProc[timer->timerType](timer);
    }

    return;
}

/*重启进boot*/
void devAppSw2Boot()
{
    return;
}


/*初始化接口*/
void timerInit()
{
    TimerMgr *mgr;
    u8 cnt;

    mgr = gTimerMgr = sysMemAlloc(sizeof(TimerMgr));
    if (NULL == mgr)
    {
        return;
    }

    ListInitD(&mgr->timerList);
    for (cnt=0; cnt<TimerIdCri; cnt++)
    {
        mgr->exprProc[cnt] = (TimerExprProc)exprIgnore;
    }
#ifdef COMPILE_BOOT_CODE
    mgr->exprProc[TidBootIdleStay] = bootIdleStayExpr;
    mgr->exprProc[TidBootSw2App] = devBootSw2App;
#else
    mgr->exprProc[TidCanSmplLoop] = boxSmplTxTry;
    mgr->exprProc[TidConnUpper] = sendUpConnCmd;
    mgr->exprProc[TidCanRxCtrlAck] = boxExprCtrlAck;
    mgr->exprProc[TidCanRxSmplAck] = boxExprSmplAck;
    mgr->exprProc[TidCanRxAuxAck] = boxExprAuxAck;
    mgr->exprProc[TidUartSmplLoop] = uartSmplBegin;
    mgr->exprProc[TidUartRxBlkAck] = uartExprBlkAck;
    mgr->exprProc[TidUartRxCtrlAck] = uartExprCtrlAck;
    mgr->exprProc[TidUartRxSmplAck] = uartExprSmplAck;
    mgr->exprProc[TidAppSw2Boot] = devAppSw2Boot;
    mgr->exprProc[TidPlcSmplLoop] = plcSmplBegin;
    mgr->exprProc[TidPlcRxSmplAck] = plcExprRxSmplAck;
    mgr->exprProc[TidPlcRxCtrlAck] = plcExprRxCtrlAck;
    mgr->exprProc[TidKeepAlive] = keepAliveChk;
    mgr->exprProc[TidTrayProtEna] = trayProtEnable;
    mgr->exprProc[TidMixProtPolicyAct] = mixProtPolicyAct;
    mgr->exprProc[TidNpSwProtDelay] = trayNpSwProtDelay;
    mgr->exprProc[TidNpSwRstDelay] = trayNpSwRstDelay;
#endif
    return;
}



