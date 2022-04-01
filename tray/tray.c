
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#include "protect.h"
#include "box.h"
#include "host.h"
#include "uart.h"
#include "plc.h"
#include "tray.h"
#include "channel.h"

void tmprGetSingle(u16 *tmpr, u8 tmprAmt, TmprData *tmprRcd)
{
    u16 *tmprCri;

    tmprRcd->tmprBeValid = False;
    for (tmprCri=tmpr+tmprAmt; tmpr<tmprCri; tmpr++)
    {
        if (*tmpr<TmprValidMin || *tmpr>TmprValidMax)
        {
            continue;
        }

        if (!tmprRcd->tmprBeValid)
        {
            tmprRcd->tmprBeValid = True;
            tmprRcd->tmprVal = *tmpr;
            tmprRcd->tmprInvalidCnt = 0;
        }
        else
        {
            if (*tmpr > tmprRcd->tmprVal)
            {
                tmprRcd->tmprVal = *tmpr;
            }
        }
    }

    if (!tmprRcd->tmprBeValid)
    {
        tmprRcd->tmprInvalidCnt++;
    }

    return;
}

/*准备托盘采样保护逻辑需要的温度针床等辅助数据*/
/*1个通道或者1个库位可能不止1个温度,保护只用1个最高的*/
/*注意从采样拿还是从托盘针床数据拿,目前可以用后者,但不绝对*/
/*从后者拿稍简单,todo,以后还是要改为从前者拿,以往万一*/
void traySmplSetProtAux(Tray *tray)
{
    TmprData *tmprRcd;
    NdbdData *ndbdData;
    TrayProtMgr *trayProtMgr;
    u16 *tmpr;

    ndbdData = &tray->ndbdData;
    trayProtMgr = &tray->trayProtMgr;
    tmprGetSingle(ndbdData->slotTmpr, ndbdData->slotTmprAmt, &trayProtMgr->slotTmprCrnt);
    if (NULL == tray->cell)
    {
        Channel *chn;
        Channel *chnCri;

        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            tmpr = &ndbdData->cellTmpr[chn->tmprIdx*ndbdData->tmprAmtPerCell];
            tmprGetSingle(tmpr, ndbdData->tmprAmtPerCell, &chn->chnProtBuf.cellTmprCrnt);
        }
    }
    else  /*串联时候,主通道不需要针床数据*/
    {
        Cell *cell;
        Cell *cellCri;

        for (cell=tray->cell,cellCri=cell+tray->trayCellAmt; cell<cellCri; cell++)
        {
            tmpr = &ndbdData->cellTmpr[cell->tmprIdx*ndbdData->tmprAmtPerCell];
            tmprGetSingle(tmpr, ndbdData->tmprAmtPerCell, &cell->chnProtBuf.cellTmprCrnt);
        }
    }

    trayProtMgr->newNdbdValid = ndbdData->ndbdDataValid;
    if (trayProtMgr->newNdbdValid)
    {
        trayProtMgr->newCoWarn = ndbdData->warn[NdbdWarnCo];
        trayProtMgr->newSmokeWarn = ndbdData->warn[NdbdWarnSmoke];
        trayProtMgr->newNdbdNp = ndbdData->status[NdbdSenNpVal];
    }

    return;
}

/*将温度针床等数据写入托盘采样*/
void traySmplSetAuxSmpl(Tray *tray)
{
    TraySmpl *traySmpl;
    TrayNdbdSmpl *trayNdbdSmpl;
    NdbdData *ndbdData;
    u16 *cellTmpr;
    u16 *cellTmprSrc;
    u16 idx;
    u8 cnt;

    traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
    traySmpl = (TraySmpl *)&traySmpl->chnSmpl[tray->genChnAmt];
    traySmpl->smplType = TraySmplNdbd;
    traySmpl->smplSize = sizeof(TrayNdbdSmpl); /*下面还要追加温度长度*/
    traySmpl->smplAmt = 1;

    ndbdData = &tray->ndbdData;
    trayNdbdSmpl = traySmpl->ndbdSmpl;
    trayNdbdSmpl->enterState = ndbdData->status[NdbdStaEnter];
    trayNdbdSmpl->touchState = ndbdData->status[NdbdStaTouch];
    trayNdbdSmpl->npVal = ndbdData->status[NdbdSenNpVal];
    trayNdbdSmpl->ratioAnalog = ndbdData->status[NdbdSenRatioVal];
    trayNdbdSmpl->cellTmprAmt = tray->trayCellAmt * ndbdData->tmprAmtPerCell;
    trayNdbdSmpl->bitmapState = 0;
    trayNdbdSmpl->bitmapAlarm = 0;
    for (idx=0; idx<BitSenCri; idx++)
    {
        if (ndbdData->status[ndbdData->bit2IdxSta[idx]])
        {
            BitSet(trayNdbdSmpl->bitmapState, idx);
        }
    }
    for (idx=0; idx<BitWarnCri; idx++)
    {
        if (ndbdData->warn[ndbdData->bit2IdxWarn[idx]])
        {
            BitSet(trayNdbdSmpl->bitmapAlarm, idx);
        }
    }

    trayNdbdSmpl->cylinderWarn = ndbdData->warn[NdbdWarnCylinder];
    trayNdbdSmpl->smokeWarn = ndbdData->warn[NdbdWarnSmoke];
    trayNdbdSmpl->slotTmprAmt = ndbdData->slotTmprAmt;
    trayNdbdSmpl->tmprAmtPerCell = ndbdData->tmprAmtPerCell;
    for (idx=0; idx<ndbdData->slotTmprAmt; idx++)
    {
        trayNdbdSmpl->slotTmpr[idx] = ndbdData->slotTmpr[idx];
    }

    cellTmpr = trayNdbdSmpl->slotTmpr + Align16(ndbdData->slotTmprAmt);
    if (NULL == tray->cell)  /*并联*/
    {
        for (idx=0; idx<tray->trayChnAmt; idx++)
        {
            cellTmprSrc = &ndbdData->cellTmpr[tray->chn[idx].tmprIdx*ndbdData->tmprAmtPerCell];
            mem2Copy(cellTmpr, cellTmprSrc, ndbdData->tmprAmtPerCell*sizeof(u16));
            cellTmpr += ndbdData->tmprAmtPerCell;
        }
    }
    else
    {
        for (idx=0; idx<tray->trayCellAmt; idx++)
        {
            cellTmprSrc = &ndbdData->cellTmpr[tray->cell[idx].tmprIdx*ndbdData->tmprAmtPerCell];
            mem2Copy(cellTmpr, cellTmprSrc, ndbdData->tmprAmtPerCell*sizeof(u16));
            cellTmpr += ndbdData->tmprAmtPerCell;
        }
    }
    traySmpl->smplSize += sizeof(u16) * Align16(cellTmpr-trayNdbdSmpl->slotTmpr);
    return;
}

/*准备针床温度等辅助数据*/
/*时机1,收到下位机采样需要调用*/
/*时机2,极端情况都采不到样,所以最后生成托盘采样时也要调用*/
void traySmplSetAux(Tray *tray)
{
    if (!tray->smplMgr.auxDataDone)
    {
        traySmplSetAuxSmpl(tray);
        traySmplSetProtAux(tray);
        tray->smplMgr.auxDataDone = True;
    }
    return;
}

/*上位机任何操作负压,都复位负压管理*/
void trayNpReset(u8 trayIdx)
{
    Tray *tray;
    Channel *chn;
    Channel *chnCri;

    tray = &gDevMgr->tray[trayIdx];
    tray->npMgr.beReach = False;
    tray->npMgr.refCnt = 0;
    for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
    {
        chn->beWiNp = False;
    }
    return;
}

/*关于负压符号:只有设置负压时plc要求非负,其余所有负压数据均为有符号*/
void trayNpRatioSet(u8 trayIdx, u8eNpOprCode npOpr, u8 swVal, s16 npVal)
{
    Tray *tray;

    tray = &gDevMgr->tray[trayIdx];
    tray->trayProtMgr.npWiSw = True;
    timerStart(&tray->npSwProtDelayTmr, TidNpSwProtDelay, 80000, WiReset);
    TimerStop(&tray->npSwRstDelayTmr);

    if (tray->ndbdCtrlByMcu)  /*mcu为0忽略1关2开*/
    {
        if (NpOprRatioBrkVacum == npOpr)  /*破真空*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetBrkVacum, 2, 0);
            //uartNdbdMcuCtrlAdd(trayIdx, NdbdSetSwValve, 1, 0);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetRatioVal, 1, 0);
        }
        else if (NpOprRatioMkVacum == npOpr)  /*抽真空*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetBrkVacum, 1, 0);
            //uartNdbdMcuCtrlAdd(trayIdx, NdbdSetSwValve, 2, 0);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetRatioVal, 2, npVal);
        }
        else if (NpOprRatioHold == npOpr)  /*保压*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetBrkVacum, 1, 0);
            //uartNdbdMcuCtrlAdd(trayIdx, NdbdSetSwValve, 1, 0);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetRatioVal, 1, 0);
        }
        else if (NpOprRatioReset == npOpr)  /*比例阀负压系统常态*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetBrkVacum, 1, 0);
            //uartNdbdMcuCtrlAdd(trayIdx, NdbdSetSwValve, 2, 0);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetRatioVal, 1, 0);
        }

        uartNdbdMcuCtrlTxTry(trayIdx);
    }
    else
    {
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
            plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0-npVal); /*plc要求填正值*/
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
            plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0==swVal ? 0 : 0-npVal);
        }
        else if (NpOprRatioSwValve == npOpr)
        {
            plcRegWriteTry(trayIdx, NdbdSetSwValve, swVal);
        }
        else  /*NpOprRatioSwBrk*/
        {
            plcRegWriteTry(trayIdx, NdbdSetBrkVacum, swVal);
        }
    }

    return;
}

void trayNpSwRstDelay(Timer *timer)
{
    Tray *tray;

    tray = Container(Tray, npSwRstDelayTmr, timer);
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

b8 trayChkStepNpSame(Channel *chn, u8 stepIdPre, u8 stepIdNew)
{
    StepNpReq *stepNpReqPre;
    StepNpReq *stepNpReqNew;

    stepNpReqPre = (StepNpReq *)(gTmpStepNpInfo[chn->box->tray->trayIdx] + sizeof(StepNpReq)*stepIdPre);
    stepNpReqNew = (StepNpReq *)(gTmpStepNpInfo[chn->box->tray->trayIdx] + sizeof(StepNpReq)*stepIdNew);
    if (stepNpReqNew->npType == stepNpReqPre->npType
        && stepNpReqNew->stepNpExpect == stepNpReqPre->stepNpExpect)
    {
        return True;
    }

    return False;
}

/*todo,暂时不匹配类型,以后高低负压时要加*/
Ret trayNpChnAlloc(Tray *tray, Channel *chn, u8 stepId)
{
    TrayNpMgr *npMgr;
    StepNpReq *stepNpReq;

    stepNpReq = (StepNpReq *)(gTmpStepNpInfo[tray->trayIdx] + sizeof(StepNpReq)*stepId);
    if (NpTypeNone == stepNpReq->npType)
    {
        return Ok;
    }

    npMgr = &tray->npMgr;
    if (0 != npMgr->refCnt)
    {
        if (npMgr->npType==stepNpReq->npType && npMgr->npExpect==stepNpReq->stepNpExpect)
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
    npMgr->npReqTimeStampSec = sysTimeSecGet();
    return Nok;
}

/*todo,现在是强耦合,释放前必须先修改通道状态.以后需要松开耦合*/
/*todo,注意检查释放逻辑是否遗漏,工步切换等*/
void trayNpChnFree(Tray *tray, Channel *chn)
{
    TrayNpMgr *npMgr;

    npMgr = &tray->npMgr;
    if (!chn->beWiNp)
    {
        return;
    }

    chn->beWiNp = False;
    npMgr->refCnt--;
    if (0 == npMgr->refCnt)
    {
        npMgr->npType = NpTypeNone;
        if (CcNone == tray->trayProtMgr.trayCauseNew)  /*todo,换一下逻辑,这里不合适*/
        {
            Channel *chn;
            Channel *chnCri;

            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                if (ChnStaNpWait == chn->chnStateMed)
                {
                    trayNpChnAlloc(tray, chn, chn->stepIdTmp);
                }
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
        stepNpReq = (StepNpReq *)(gTmpStepNpInfo[tray->trayIdx] + sizeof(StepNpReq)*chn->stepIdTmp);
        if (ChnStaNpWait==chn->chnStateMed && chn->beWiNp)
        {
            boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
            chn->chnStateMed = ChnStaStart;
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
        if (crntNpVal > npMgr->npExpect+80)
        {
            trayNpRatioSet(tray->trayIdx, NpOprRatioMkVacum, 0, npMgr->npExpect);
        }
        return;
    }

    if (crntNpVal>=npMgr->npMin && crntNpVal<=npMgr->npMax)
    {
        npMgr->beReach = True;
        npMgr->npOverLmtTimeStampSec = 0;
        trayNpReachNtfy(tray);
        return;
    }
}

/*随时调用*/
void trayStopByProt(Tray *tray, u16eCauseCode causeCode)
{
    Channel *chn;
    Channel *chnCri;
    b8 hasRunChn;

    for (hasRunChn=False,chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        if (ChnStaRun==chn->chnStateMed || ChnStaStart==chn->chnStateMed)
        {
            hasRunChn = True;
            CcChkModify(tray->trayProtMgr.trayCauseNew, causeCode);
            break;
        }
    }

    if (hasRunChn && Cc0NdbdBrkAbnml==causeCode)
    {
        if (tray->ndbdData.status[NdbdSenNpVal] < -200)
        {
            trayNpRatioSet(tray->trayIdx, NpOprRatioBrkVacum, 0, 0);
        }
    }

    return;
}

/*确定在产生托盘采样后调用,目前只有磁盘满,命名以后再说*/
void trayStopByDisk(Tray *tray, u16eCauseCode causeCode)
{
    TrayProtMgr *trayProtMgr;
    Channel *chn;
    Channel *chnCri;
    Box *box;
    Box *boxCri;

    tray->trayWarnPres = True;
    trayProtMgr = &tray->trayProtMgr;
    CcChkModify(trayProtMgr->trayCauseNew, causeCode);
    if (causeCode!=trayProtMgr->trayCausePre && CcBaseGet(causeCode)>=CcBaseGet(trayProtMgr->trayCausePre))
    {
        trayProtMgr->trayCausePre = causeCode;
        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            CcChkModify(chn->chnProtBuf.newCauseCode, causeCode);
            chnStopByProt(chn, True);
        }

        for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
        {
            if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
            {
                boxCtrlTxTry(box);
            }
        }
    }
    return;
}

void trayNdbdBreakChk(Timer *timer)
{
    Tray *tray;

    tray = Container(Tray, ndbdBrkWaitNpTmr, timer);
    if (tray->ndbdData.status[NdbdSenNpVal] < -50)
    {
        if (sysTimeSecGet()-tray->npMgr.baseTimeSecT07 < tray->npMgr.ndbdBrkWaitNpSecT07)
        {
            timerStart(timer, TidNdbdBrkWaitNp, 2000, WoReset);
        }
        else
        {
            trayNdbdCtrl(tray->trayIdx, NdbdSetTouch, 2);
        }
    }
    else
    {
        trayNdbdCtrl(tray->trayIdx, NdbdSetTouch, 2);
    }
    return;
}

void trayNdbdBreak(u8 trayIdx)
{
    Tray *tray;

    tray = &gDevMgr->tray[trayIdx];
    if (tray->ndbdData.status[NdbdSenNpVal] < -100)
    {
        trayNpRatioSet(trayIdx, NpOprRatioBrkVacum, 0, 0);
        timerStart(&tray->ndbdBrkWaitNpTmr, TidNdbdBrkWaitNp, 2000, WiReset);
        tray->npMgr.baseTimeSecT07 = sysTimeSecGet();
    }
    else
    {
        trayNdbdCtrl(trayIdx, NdbdSetTouch, 2);
    }
    return;
}

/*家里跑可能不需要连针床控制,那么配置为plc且plc数量为零即可*/
void trayIgnoreNdbd()
{
    Tray *tray;

    for (tray=gDevMgr->tray; tray<&gDevMgr->tray[gDevMgr->trayAmt]; tray++)
    {
        if (!tray->ndbdCtrlByMcu)
        {
            trayProtEnable(&tray->protEnaTmr);  /*使能保护*/
            tray->ndbdData.status[NdbdStaWorkMode] = 0; /*自动,允许启动*/
        }
    }

    return;
}

b8 trayNdbdBeOnline(u8 trayIdx)
{
    Tray *tray;

    tray = &gDevMgr->tray[trayIdx];
    if (tray->ndbdCtrlByMcu)
    {
        return gDevMgr->ndbdMcu[trayIdx].online;
    }
    else
    {
        return plcBeOnline(trayIdx);
    }
}

/*除去负压之外的针床相关动作,负压单独做接口trayNpRatioSet*/
/*先做的plc,所以plc的直接调用,针床主控的需要适配*/
void trayNdbdCtrl(u8 trayIdx, u8eNdbdCtrlType type, s16 oprVal)
{
    Tray *tray;

    tray = &gDevMgr->tray[trayIdx];
    if (tray->ndbdCtrlByMcu)
    {
        if (NdbdSetSwValve != type)  /*目前针床主控没有*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, type, oprVal, 0);
            uartNdbdMcuCtrlTxTry(trayIdx);
        }
    }
    else
    {
        if (NdbdSetSmokeRmv != type)  /*目前plc没有*/
        {
            plcRegWriteTry(trayIdx, type, oprVal);
        }
    }
    return;
}

#endif
