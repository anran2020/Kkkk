//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#include "protect.h"
#include "box.h"
#include "host.h"
#include "uart.h"
#include "plc.h"
#include "tray.h"

/*关于负压符号:只有设置负压时plc要求非负,其余所有负压数据均为有符号*/
void trayNpRatioSet(u8 trayIdx, u8eNpOprCode npOpr, u8 sw, s16 npVal)
{
    Tray *tray;

    tray = &gDevMgr->tray[trayIdx];
    tray->trayProtMgr.npWiSw = True;
    timerStart(&tray->npSwProtDelayTmr, TidNpSwProtDelay, 10000, WiReset);

    if (NpOprRatioBrkVacum == npOpr)  /*破真空*/
    {
        plcRegWriteTry(trayIdx, NdbdSetBrkVacum, 1);
        plcRegWriteTry(trayIdx, NdbdSetSwValve, 1);
        plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0);
    }
    else if (NpOprRatioMkVacum == npOpr)  /*抽真空*/
    {
        plcRegWriteTry(trayIdx, NdbdSetBrkVacum, 0);
        plcRegWriteTry(trayIdx, NdbdSetSwValve, 0);
        plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0-npVal);
    }
    else if (NpOprRatioHold == npOpr)  /*保压*/
    {
        plcRegWriteTry(trayIdx, NdbdSetBrkVacum, 0);
        plcRegWriteTry(trayIdx, NdbdSetSwValve, 1);
        plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0);
    }
    else if (NpOprRatioReset == npOpr)  /*比例阀负压系统常态*/
    {
        plcRegWriteTry(trayIdx, NdbdSetBrkVacum, 0);
        plcRegWriteTry(trayIdx, NdbdSetSwValve, 0);
        plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0);
    }
    else if (NpOprRatioSwRatio == npOpr) /*sw:0关1开*/
    {
        plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0==sw ? 0 : 0-npVal);
    }
    else if (NpOprRatioSwValve == npOpr)
    {
        plcRegWriteTry(trayIdx, NdbdSetSwValve, sw);
    }
    else  /*NpOprRatioSwBrk*/
    {
        plcRegWriteTry(trayIdx, NdbdSetBrkVacum, sw);
    }

    return;
}

void trayNpSwRstDelay(Timer *timer)
{
    Tray *tray;

    tray = Container(Tray, npSwProtDelayTmr, timer);
    trayNpRatioSet(tray->trayIdx, NpOprRatioReset, 0, 0);
    return;
}

void trayNpSwProtDelay(Timer *timer)
{
    Tray *tray;

    tray = Container(Tray, npSwProtDelayTmr, timer);
    tray->trayProtMgr.npWiSw = False;
    return;
}

/*todo,暂时不匹配类型,以后高低负压时要加*/
Ret trayNpChnAlloc(Tray *tray, Channel *chn, u8 stepId)
{
    TrayNpMgr *npMgr;
    StepNpReq *stepNpReq;

    stepNpReq = (StepNpReq *)(gTmpStepNpInfo + sizeof(StepNpReq)*stepId);
    if (NpTypeNone == stepNpReq->npType)
    {
        return Ok;
    }

    npMgr = &tray->npMgr;
    if (0 != npMgr->refCnt)
    {
        if (npMgr->npExpect==stepNpReq->stepNpExpect)
        {
            chn->beWiNp = True;
            npMgr->refCnt++;
            if (npMgr->beReach)
            {
                return Ok;
            }
        }
        return Nok;
    }

    if (NpTypeNml==stepNpReq->npType || stepNpReq->stepNpExpect>npMgr->npExpect) /*高切低要先破*/
    {
        trayNpRatioSet(tray->trayIdx, NpOprRatioBrkVacum, 0, 0);
    }
    else
    {
        trayNpRatioSet(tray->trayIdx, NpOprRatioMkVacum, 0, stepNpReq->stepNpExpect);
    }

    chn->beWiNp = True;
    npMgr->refCnt++;
    npMgr->beReach = False;
    npMgr->npType = stepNpReq->npType;
    npMgr->npExpect = stepNpReq->stepNpExpect;
    npMgr->npMax = stepNpReq->stepNpMax;
    npMgr->npMin = stepNpReq->stepNpMin;
    npMgr->npReqStartSec = sysTimeSecGet();
    return Nok;
}

/*todo,当下相对妥善的是beWiNp与beWiNp严格耦合,以后状态机完善后可以改为松散*/
/*todo,现在是强耦合,释放前必须先修改通道状态.以后需要松开耦合*/
void trayNpChnFree(Tray *tray, Channel *chn)
{
    TrayNpMgr *npMgr;

    npMgr = &tray->npMgr;
    if (!chn->beWiNp)  /*todo, 判断放到channel.c*/
    {
        return;
    }

    chn->beWiNp = False;
    if (0 == npMgr->refCnt) /*todo,不该走到这里,这个防呆只是形式,不解决问题*/
    {
        return;
    }

    npMgr->refCnt--;
    if (0 == npMgr->refCnt)
    {
        Channel *chn;
        Channel *chnCri;

        npMgr->npType = NpTypeNone;
        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (ChnStaNp == chn->chnStateMed)
            {
                trayNpChnAlloc(tray, chn, chn->stepIdTmp);
            }
        }
    }

    return;
}

void trayNpReachNtfy(Tray *tray)
{
    Channel *chn;
    Channel *chnCri;
    StepNpReq *stepNpReq;
    TrayNpMgr *npMgr;
    Box *box;
    Box *boxCri;

    npMgr = &tray->npMgr;
    for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
    {
        stepNpReq = (StepNpReq *)(gTmpStepNpInfo + sizeof(StepNpReq)*chn->stepIdTmp);
        if (ChnStaNp==chn->chnStateMed && stepNpReq->npType==npMgr->npType
            && stepNpReq->stepNpExpect==npMgr->npExpect)
        {
            boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
            chn->chnStateMed = ChnStaFlowStart;
            chn->dynStaCnt = 0;
        }
    }

    for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
    {
        if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
        {
            boxCtrlTxTry(box);
        }
    }

    return;
}

/*只检查成功不检查失败*/
void trayNpReachChk(Tray *tray)
{
    TrayNpMgr *npMgr;
    NdbdData *ndbdData;
    s16 crntNpVal;

    npMgr = &tray->npMgr;
    ndbdData = &tray->ndbdData;
    if (0==npMgr->refCnt || npMgr->beReach || !ndbdData->ndbdDataValid)
    {
        return;
    }

    crntNpVal = ndbdData->status[NdbdSenNpVal];
    if (1==ndbdData->status[NdbdSenBrkVacum] && NpTypeNml!=npMgr->npType)
    {
        /*破开着且目标不是常压,那么一定是高切低*/
        if (crntNpVal > npMgr->npExpect+100)
        {
            trayNpRatioSet(tray->trayIdx, NpOprRatioMkVacum, 0, npMgr->npExpect);
        }
        return;
    }

    if (crntNpVal>=npMgr->npMin && crntNpVal<=npMgr->npMax)
    {
        npMgr->beReach = True;
        npMgr->npOverLmtSec = 0;
        trayNpReachNtfy(tray);
        return;
    }
}


