
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

#ifdef DebugVersion
#else
#include "cs_sdcard.h"
#endif

void tmprGetSingle(u16 *tmpr, u8 tmprAmt, TmprData *tmprRcd)
{
    u16 *tmprCri;

    tmprRcd->tmprBeValid = False;
    if (0 == tmprAmt)
    {
        return;
    }

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
        trayProtMgr->newGasWarn = ndbdData->warn[NdbdWarnGas];
        trayProtMgr->newFanWarn = ndbdData->warn[NdbdWarnFan];
        trayProtMgr->newNdbdNp = ndbdData->status[NdbdSenNpVal];
        trayProtMgr->newNdbdNpTs = ndbdData->ndbdSmplTsSec;
    }

    return;
}

/*将温度针床等数据写入托盘采样*/
void traySmplSetAuxSmpl(Tray *tray)
{
    TraySmpl *traySmpl;
    TrayNdbdSmpl *trayNdbdSmpl;
    NdbdDynData *dynData;
    NdbdData *ndbdData;
    u16 idx;

    traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
    traySmpl = (TraySmpl *)&traySmpl->chnSmpl[tray->genChnAmt];
    traySmpl->smplType = TraySmplNdbd;
    traySmpl->smplSize = sizeof(TrayNdbdSmpl); /*下面还要追加动态数据长度*/
    traySmpl->smplAmt = 1;

    ndbdData = &tray->ndbdData;
    trayNdbdSmpl = traySmpl->ndbdSmpl;
    trayNdbdSmpl->enterState = ndbdData->status[NdbdStaEnter];
    trayNdbdSmpl->touchState = ndbdData->status[NdbdStaTouch];
    trayNdbdSmpl->cylinderWarn = ndbdData->warn[NdbdWarnCylinder];
    trayNdbdSmpl->smokeWarn = ndbdData->warn[NdbdWarnSmoke];
    trayNdbdSmpl->npVal = ndbdData->status[NdbdSenNpVal];
    trayNdbdSmpl->ratioAnalog = ndbdData->status[NdbdSenRatioVal];
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

    dynData = trayNdbdSmpl->dynData;
    if (0 != ndbdData->tmprAmtPerCell)  /*电芯温度*/
    {
        u16 *tmprDst;
        u16 *tmprSrc;

        dynData->dataType = NdbdDynCellTmpr;
        dynData->rsvd = 0;
        dynData->dataLen = tray->trayCellAmt * ndbdData->tmprAmtPerCell * sizeof(u16);
        tmprDst = (u16 *)dynData->data;
        if (NULL == tray->cell)  /*并联*/
        {
            for (idx=0; idx<tray->trayChnAmt; idx++)
            {
                tmprSrc = &ndbdData->cellTmpr[tray->chn[idx].tmprIdx*ndbdData->tmprAmtPerCell];
                mem2Copy(tmprDst, tmprSrc, ndbdData->tmprAmtPerCell*sizeof(u16));
                tmprDst += ndbdData->tmprAmtPerCell;
            }
        }
        else
        {
            for (idx=0; idx<tray->trayCellAmt; idx++)
            {
                tmprSrc = &ndbdData->cellTmpr[tray->cell[idx].tmprIdx*ndbdData->tmprAmtPerCell];
                mem2Copy(tmprDst, tmprSrc, ndbdData->tmprAmtPerCell*sizeof(u16));
                tmprDst += ndbdData->tmprAmtPerCell;
            }
        }

        traySmpl->smplSize += sizeof(NdbdDynData) + Align8(dynData->dataLen);
        dynData = (NdbdDynData *)((u8 *)dynData + sizeof(NdbdDynData) + Align8(dynData->dataLen));
    }

    if (0 != ndbdData->slotTmprAmt)  /*库位环境温度*/
    {
        dynData->dataType = NdbdDynSlotTmpr;
        dynData->rsvd = 0;
        dynData->dataLen = ndbdData->slotTmprAmt * sizeof(u16);
        mem2Copy((u16 *)dynData->data, ndbdData->slotTmpr, dynData->dataLen);
        traySmpl->smplSize += sizeof(NdbdDynData) + Align8(dynData->dataLen);
        dynData = (NdbdDynData *)((u8 *)dynData + sizeof(NdbdDynData) + Align8(dynData->dataLen));
    }

    if (0 != ndbdData->slotWaterTmprAmt)  /*库位水箱温度*/
    {
        dynData->dataType = NdbdDynWaterTmpr;
        dynData->rsvd = 0;
        dynData->dataLen = ndbdData->slotWaterTmprAmt * sizeof(u16);
        mem2Copy((u16 *)dynData->data, ndbdData->slotWaterTmpr, dynData->dataLen);
        traySmpl->smplSize += sizeof(NdbdDynData) + Align8(dynData->dataLen);
        dynData = (NdbdDynData *)((u8 *)dynData + sizeof(NdbdDynData) + Align8(dynData->dataLen));
    }
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
/*简单粗暴有效但不细致,,todo,,可更换机制*/
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

    if (tray->ndbdCtrlByMcu)  /*mcu为0忽略1关2开,,比例阀除外*/
    {
        if (NpOprRatioBrkVacum == npOpr)  /*破真空*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetBrkVacum, 2);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetNpGate, 2);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetRatioVal, 0);
        }
        else if (NpOprRatioMkVacum == npOpr)  /*抽真空*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetBrkVacum, 1);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetNpGate, 2);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetRatioVal, npVal);
        }
        else if (NpOprRatioHold == npOpr)  /*保压*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetBrkVacum, 1);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetNpGate, 1);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetRatioVal, 0);
        }
        else if (NpOprRatioReset == npOpr)  /*比例阀负压系统常态*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetBrkVacum, 1);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetNpGate, 2);
            uartNdbdMcuCtrlAdd(trayIdx, NdbdSetRatioVal, 0);
        }

        uartNdbdMcuCtrlTxTry(trayIdx);
    }
    else
    {
        if (NpOprRatioBrkVacum == npOpr)  /*破真空*/
        {
            plcRegWriteTry(trayIdx, NdbdSetBrkVacum, 1);
            plcRegWriteTry(trayIdx, NdbdSetNpGate, 0);
            plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0);
        }
        else if (NpOprRatioMkVacum == npOpr)  /*抽真空*/
        {
            plcRegWriteTry(trayIdx, NdbdSetBrkVacum, 0);
            plcRegWriteTry(trayIdx, NdbdSetNpGate, 0);
            plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0-npVal); /*plc要求填正值*/
        }
        else if (NpOprRatioHold == npOpr)  /*保压*/
        {
            plcRegWriteTry(trayIdx, NdbdSetBrkVacum, 0);
            plcRegWriteTry(trayIdx, NdbdSetNpGate, 1);
            plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0);
        }
        else if (NpOprRatioReset == npOpr)  /*比例阀负压系统常态*/
        {
            plcRegWriteTry(trayIdx, NdbdSetBrkVacum, 0);
            plcRegWriteTry(trayIdx, NdbdSetNpGate, 0);
            plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0);
        }
        else if (NpOprRatioSwRatio == npOpr) /*sw:0关1开*/
        {
            plcRegWriteTry(trayIdx, NdbdSetRatioVal, 0==swVal ? 0 : 0-npVal);
        }
        else if (NpOprRatioNpGate == npOpr)
        {
            plcRegWriteTry(trayIdx, NdbdSetNpGate, swVal);
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

/*todo,暂时不匹配类型,以后高低负压时要加*/
/*Ok--直接跑,Nok--等待*/
Ret trayNpChnAlloc(Tray *tray, Channel *chn)
{
    TrayNpMgr *npMgr;
    StepNpCfg *stepNpReq;

    stepNpReq = chn->crntStepNode->stepNpCfg;
    if (NULL==stepNpReq || NpTypeNone==stepNpReq->npType)
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
        if (CcNone == tray->trayProtMgr.trayCauseNew)  /*todo,逻辑可以考虑挪出*/
        {
            Channel *chn;
            Channel *chnCri;

            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                if (ChnStaNpWait == chn->chnStateMed)
                {
                    trayNpChnAlloc(tray, chn);
                }
            }
        }
    }

    return;
}

/*工步切换时重新申请负压,Ok--直接跑,Nok--等待*/
Ret trayNpChnReAlloc(Tray *tray, Channel *chn)
{
    TrayNpMgr *npMgr;
    StepNpCfg *stepNpReq;

    stepNpReq = chn->crntStepNode->stepNpCfg;
    npMgr = &tray->npMgr;
    if (chn->beWiNp)
    {
        if (npMgr->npType==stepNpReq->npType && npMgr->npExpect==stepNpReq->stepNpExpect)
        {
            return Ok;
        }

        trayNpChnFree(tray, chn);
    }

    return trayNpChnAlloc(tray, chn);
}

void trayNpReachNtfy(Tray *tray)
{
    Channel *chn;
    Channel *chnCri;
    TrayNpMgr *npMgr;
    Box *box;
    Box *boxCri;

    npMgr = &tray->npMgr;
    for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
    {
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
        if (crntNpVal>npMgr->npExpect+50 || crntNpVal>-20)
        {
            trayNpRatioSet(tray->trayIdx, NpOprRatioMkVacum, 0, npMgr->npExpect);
        }
        return;
    }

    if (crntNpVal>=npMgr->npMin && crntNpVal<=npMgr->npMax)
    {
        npMgr->beReach = True;
        npMgr->npOverLmtTimeStampSec = 0;
        trayNpBigRiseRst(tray);
        trayNpReachNtfy(tray);
    }

    return;
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
            tray->ndbdData.status[NdbdStaTouch] = 1; /*压合,用于通知下位机,否则不需置1*/
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
        if (NdbdSetNpGate != type)  /*目前针床主控没有*/
        {
            uartNdbdMcuCtrlAdd(trayIdx, type, oprVal);
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

/*收到上位机开始维护指令*/
void trayMntnEnter(Tray *tray, u8eBoxWorkMode mode)
{
#if 0 /*最初出于节约而不采样,但上和下均有适配不利索,改为继续采样*/
    if (BoxModeMntnCali == mode)
    {
        traySmplMgrRst(tray);
    }
#endif
    tray->trayWorkMode |= mode;
    tray->protDisable |= ProtDisableMntn;
    timerStart(&tray->mntnExprTmr, TidTrayMntnExpr, 75000, WiReset);
    return;
}

/*收到上位机后续维护指令,更新定时器*/
void trayMntnKeep(Tray *tray)
{
    timerStart(&tray->mntnExprTmr, TidTrayMntnExpr, 75000, WiReset);
    return;
}

/*收到级联设备离开维护应答*/
/*对于上位机触发的离开修调,收到最后一个box的应答后,托盘才离开修调*/
void trayMntnEnd(Tray *tray, u8eBoxWorkMode mode)
{
    if (BoxModeMntnCali == mode)
    {
        Box *box;
        Box *boxCri;
    
        for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
        {
            if (BoxModeMntnCali == box->boxWorkMode)
            {
                return;  /*只要有还在修调的box,托盘就不离开维护*/
            }
        }
    }

    tray->trayWorkMode &= ~mode;
    if (BoxModeManu == tray->trayWorkMode)
    {
        TimerStop(&tray->mntnExprTmr);
        tray->protDisable &= ~ProtDisableMntn;
    }

    return;
}

/*托盘维护超时*/
void trayMntnExpr(Timer *timer)
{
    Tray *tray;

    tray = Container(Tray, mntnExprTmr, timer);
    if (tray->trayWorkMode & BoxModeMntnCali)
    {
        Box *box;
        Box *boxCri;
    
        for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
        {
            if (BoxModeMntnCali == box->boxWorkMode)
            {
                /*todo,如果需要通知下位机离开,注掉下一行*/
                box->boxWorkMode = BoxModeManu;
                //box->online = False;
            }
        }
    }

    tray->trayWorkMode = BoxModeManu;
    tray->protDisable &= ~ProtDisableMntn;
    return;
}

/*todo,,下面这些单独建立文件,现在忘了咋加了*/
SmplDiskMgr *gSmplDiskMgr = NULL;

/*核实头信息是否有效*/
b8 diskHeadChkValid(SmplDiskHead *head)
{
    u32 chkSum;

    if (head->headLen != HeadLenWoChkSum(head->trayAmt))
    {
        return False;
    }

    chkSum = chkSum32((u32 *)&head->headLen, head->headLen);
    if (chkSum != head->chkSum)
    {
        return False;
    }

    return True;
}

/*确定头信息并存储到headDst,若两个参数均无效则失败*/
Ret diskHeadSelect(SmplDiskHead *headDst, SmplDiskHead *headCmp)
{
    b8 validDst;
    b8 validCmp;

    validDst = diskHeadChkValid(headDst);
    validCmp = diskHeadChkValid(headCmp);
    if (validDst)
    {
        if (!validCmp)
        {
            return Ok;
        }
        if (headDst->abTagCnt>headCmp->abTagCnt || 0==headDst->abTagCnt&&255==headCmp->abTagCnt)
        {
            return Ok;
        }
        memcpy(&headDst->headLen, &headCmp->headLen, headCmp->headLen);
        return Ok;
    }

    if (validCmp)
    {
        memcpy(&headDst->headLen, &headCmp->headLen, headCmp->headLen);
        return Ok;
    }

    return Nok;
}

/*复位头信息,不包括时间戳*/
/*headSet--上层的需求,输入输出*/
/*headMgr--本层管理*/
/*wtDiskNeed--是否需要重写磁盘头信息*/
void diskHeadReset(SmplDiskHead *headReq, SmplDiskHead *headDisk, b8 wtDiskNeed)
{
    u32 pageBase;
    u16 smplDiskSize;
    u8 idx;

    headDisk->headLen = HeadLenWoChkSum(headReq->trayAmt);
    headDisk->version = headReq->version;
    headDisk->abTagCnt = 255;  /*从a区0开始*/
    headDisk->trayAmt = headReq->trayAmt;
    headDisk->smplAmtPerTs = SmplAmtPerTs;
    pageBase = SmplHeadDiskPage + 2;
    for (idx=0; idx<headReq->trayAmt; idx++)
    {
        headDisk->trayHead[idx].trayCellAmt = headReq->trayHead[idx].trayCellAmt;
        headDisk->trayHead[idx].trayBoxAmt = headReq->trayHead[idx].trayBoxAmt;
        headDisk->trayHead[idx].smplSize = headReq->trayHead[idx].smplSize;
        headDisk->trayHead[idx].smplAmt = headReq->trayHead[idx].smplAmt;
        headDisk->trayHead[idx].smplSeq = 0;
        headDisk->trayHead[idx].diskLooped = False;
        headDisk->trayHead[idx].pageBase = pageBase;
        headDisk->trayHead[idx].writeCnt = 0;
        smplDiskSize = headReq->trayHead[idx].smplSize + sizeof(SmplExtraInfo);
        headDisk->trayHead[idx].smplPageAmt = smplDiskSize/DiskPageSize;
        if (smplDiskSize % DiskPageSize)
        {
            headDisk->trayHead[idx].smplPageAmt += 1;
        }
        pageBase += headDisk->trayHead[idx].smplAmt * headDisk->trayHead[idx].smplPageAmt;

        headReq->trayHead[idx].smplSeq = 0;
        headReq->trayHead[idx].diskLooped = False;
    }

    /*将复位信息写入磁盘,也即破坏磁盘原有合法数据*/
    if (wtDiskNeed)
    {
        SmplDiskHead *headReset;
        headReset = (SmplDiskHead *)gSmplDiskMgr->buffer;
        headReset->headLen = 0;
    #ifdef DebugVersion
    #else
        sdSpiWrite(gSmplDiskMgr->buffer, SmplHeadDiskPage, 1, 16);
        sdSpiWrite(gSmplDiskMgr->buffer, SmplHeadDiskPage+1, 1, 16);
    #endif
    }

    return;
}

/*检查从磁盘读出的信息与需求是否匹配*/
b8 diskHeadChkMatch(SmplDiskHead *headReq, SmplDiskHead *headDisk)
{
    u8 idx;

    if (headReq->version != headDisk->version
        || headReq->trayAmt != headDisk->trayAmt)
    {
        return False;
    }

    for (idx=0; idx<headReq->trayAmt; idx++)
    {
        if (headReq->trayHead[idx].trayCellAmt != headDisk->trayHead[idx].trayCellAmt
            || headReq->trayHead[idx].trayBoxAmt != headDisk->trayHead[idx].trayBoxAmt
            || headReq->trayHead[idx].smplSize != headDisk->trayHead[idx].smplSize
            || headReq->trayHead[idx].smplAmt != headDisk->trayHead[idx].smplAmt)
        {
            return False;
        }
    }

    return True;
}

/*查找最后的数据,将下一条序号以及循环标志填写到headReq返给上层*/
/*buffer是临时用的缓存*/
void diskSmplSeek(SmplDiskHead *headReq, SmplDiskHead *headDisk, u8 *buffer)
{
    SmplDiskTrayHead *trayHead;
    SmplExtraInfo *extraInfo;
    u32 pageId;
    u16 smplSeq;
    u16 readCnt;
    u16 bufSize;
    u16 idx;

    for (idx=0; idx<headDisk->trayAmt; idx++)
    {
        trayHead = &headDisk->trayHead[idx];
        extraInfo = (SmplExtraInfo *)(buffer + trayHead->smplSize);
        headReq->trayHead[idx].diskLooped = trayHead->diskLooped;
        bufSize = trayHead->smplSize + sizeof(SmplExtraInfo);
        for (smplSeq=trayHead->smplSeq,readCnt=0; readCnt<headDisk->smplAmtPerTs; readCnt++)
        {
            pageId = trayHead->pageBase + smplSeq*trayHead->smplPageAmt;
        #ifdef DebugVersion
        #else
            if (Ok != sdSpiRead(buffer, pageId, trayHead->smplPageAmt, bufSize)
                || extraInfo->timeStamp != trayHead->timeStamp)
            {
                break;
            }
        #endif

            if (smplSeq == trayHead->smplAmt-1)
            {
                smplSeq = 0;
                headReq->trayHead[idx].diskLooped = True;
            }
            else
            {
                smplSeq += 1;
            }
        }

        headReq->trayHead[idx].smplSeq = smplSeq;
    }

    return;
}

/*目前只支持1条的读写,不排除性能原因而多条读写*/
Ret smplDiskRead(u8 trayIdx, u8 smplAmt, u8 *smplData, u16 smplSeq)
{
    SmplDiskMgr *mgr;
    SmplDiskTrayHead *trayHead;
    u32 pageId;

    mgr = gSmplDiskMgr;
    if (NULL==mgr || trayIdx>=mgr->smplDiskHead->trayAmt)
    {
        return Nok;
    }

    trayHead = &mgr->smplDiskHead->trayHead[trayIdx];
    pageId = trayHead->pageBase + smplSeq*trayHead->smplPageAmt;
#ifdef DebugVersion
    return Nok;
#else
    return sdSpiRead(smplData, pageId, trayHead->smplPageAmt, trayHead->smplSize);
#endif
}

Ret smplDiskWrite(u8 trayIdx, u8 smplAmt, u8 *smplData, u16 smplSeq)
{
    SmplDiskMgr *mgr;
    SmplDiskHead *diskHead;
    SmplDiskTrayHead *trayHead;
    SmplExtraInfo extra;
    u32 pageId;
    Ret ret;

    mgr = gSmplDiskMgr;
    if (NULL==mgr || trayIdx>=mgr->smplDiskHead->trayAmt)
    {
        return Nok;
    }

    diskHead = mgr->smplDiskHead;
    trayHead = &diskHead->trayHead[trayIdx];
    if (0 == trayHead->writeCnt%SmplAmtPerTs)
    {
        extra.timeStamp = mgr->timeStampBase + sysTimeSecGet();
    }
    else
    {
        extra.timeStamp = trayHead->timeStamp;
    }

    pageId = trayHead->pageBase + smplSeq*trayHead->smplPageAmt;
#ifdef DebugVersion
    ret = Ok;
#else
    ret = sdSpiWriteSmpl(smplData, pageId, trayHead->smplPageAmt,
        trayHead->smplSize, extra.timeStamp, extra.chkSum);
#endif
    if (Ok != ret)
    {
        return Nok;
    }

    if (smplSeq == trayHead->smplAmt-1)
    {
        trayHead->diskLooped = True;
    }
    if (0 == trayHead->writeCnt%SmplAmtPerTs)
    {
        diskHead->timeStampBase = trayHead->timeStamp = extra.timeStamp;
        diskHead->abTagCnt++;
        trayHead->smplSeq = smplSeq;
        diskHead->chkSum = chkSum32((u32 *)&diskHead->headLen, diskHead->headLen);
        pageId = 0==diskHead->abTagCnt%2 ? SmplHeadDiskPage : SmplHeadDiskPage+1;
    #ifdef DebugVersion
    #else
        ret = sdSpiWrite((u8 *)diskHead, pageId, 1, diskHead->headLen+sizeof(u32));
    #endif
        if (Ok != ret)
        {
            diskHead->abTagCnt--;
            return Nok;
        }
    }

    trayHead->writeCnt++;
    return Ok;
}

Ret smplDiskInit(SmplDiskHead *headSet)
{
    SmplDiskMgr *mgr;
    u16 idx;
    u16 smplPageAmtMax;
    u16 smplDiskSize;  /*单采样存储大小,实际采样加额外信息*/
    u16 headSize;
    u32 diskTtlSize;
    u8 smplPageAmt;
    Ret ret;

    if (NULL!=gSmplDiskMgr || NULL==(mgr=sysMemAlloc(sizeof(SmplDiskMgr))))
    {
        return Nok;
    }

    /*存储时附带时间戳和校验和,其中校验和暂时不用*/
    for (smplPageAmtMax=0,diskTtlSize=0,idx=0; idx<headSet->trayAmt; idx++)
    {
        smplDiskSize = headSet->trayHead[idx].smplSize + sizeof(SmplExtraInfo);
        smplPageAmt = smplDiskSize/DiskPageSize;
        if (smplDiskSize % DiskPageSize)
        {
            smplPageAmt += 1;
        }

        diskTtlSize += smplPageAmt * DiskPageSize;
        if (smplPageAmtMax < smplPageAmt)
        {
            smplPageAmtMax = smplPageAmt;
        }
    }

    if (diskTtlSize+DiskPageSize*2 > SmplDiskSizeMax)
    {
        return Nok;
    }

    headSize = sizeof(SmplDiskHead) + sizeof(SmplDiskTrayHead)*headSet->trayAmt;
    mgr->smplDiskHead = sysMemAlloc(headSize);
    mgr->buffer = sysMemAlloc(smplPageAmtMax * DiskPageSize);
    if (NULL==mgr->smplDiskHead || NULL==mgr->buffer)
    {
        return Nok;  /*目前系统,不用释放*/
    }
    gSmplDiskMgr = mgr;

    /*读取磁盘头信息,a区*/
#ifdef DebugVersion
#else
    sdSpiRead(mgr->buffer, SmplHeadDiskPage, 1, DiskPageSize);
#endif
    memcpy(mgr->smplDiskHead, mgr->buffer, headSize);

    /*读取磁盘头信息,b区*/
#ifdef DebugVersion
#else
    sdSpiRead(mgr->buffer, SmplHeadDiskPage+1, 1, DiskPageSize);
#endif
    ret = diskHeadSelect(mgr->smplDiskHead, (SmplDiskHead *)mgr->buffer);
    if (Ok != ret) /*磁盘头信息无效,不用重写头信息*/
    {
        diskHeadReset(headSet, mgr->smplDiskHead, False);
        mgr->timeStampBase = 12345;
        return Ok;
    }

    /*磁盘头信息合法,再与初始化入参即需求相比较*/
    /*如果磁盘头信息与入参需求不符,清空磁盘*/
    mgr->timeStampBase = mgr->smplDiskHead->timeStampBase + 2;
    if (!diskHeadChkMatch(headSet, mgr->smplDiskHead))
    {
        diskHeadReset(headSet, mgr->smplDiskHead, True);
        return Ok;
    }

    /*都匹配,就找到最后一条数据的序号,以及循环标志,一起返回给上层*/
    diskSmplSeek(headSet, mgr->smplDiskHead, mgr->buffer);
    for (idx=0; idx<mgr->smplDiskHead->trayAmt; idx++)
    {
        mgr->smplDiskHead->trayHead[idx].writeCnt = 0;
    }
    return Ok;
}


void testMakeSmpl(u8 *buffer, u16 len, u16 seq, u8 trayId, u8 base)
{
    u8 *bufCri;

    for (bufCri=buffer+len; buffer<bufCri; buffer++)
    {
        *buffer = (u8)seq + trayId + base++;
    }
    return;
}
u32 testSmplDisk()
{
    u8 *bufWt;
    u8 *bufRd;
    SmplDiskHead *smplDiskHead;
    u16 smplSize;
    u16 idx;
    u16 testTimes;
    u16 smplSeq;
    u32 tempMs;
    u8 trayAmt;
    u8 ret;
    u8 trayIdx;
    u8 looped;
    u32 timeBase;
    u32 timeCrnt;
    u32 timeMaxWt;
    u32 timeMaxRd;

    timeMaxWt = timeMaxRd = 0;
    smplSize = 1400;
    trayAmt = 4;
    bufWt = sysMemAlloc(smplSize);
    bufRd = sysMemAlloc(smplSize);
    smplDiskHead = sysMemAlloc(sizeof(SmplDiskHead) + sizeof(SmplDiskTrayHead)*trayAmt);
    smplDiskHead->trayAmt = trayAmt;
    smplDiskHead->version = 3;
    for (idx=0; idx<trayAmt; idx++)
    {
        smplDiskHead->trayHead[idx].trayCellAmt = 32;
        smplDiskHead->trayHead[idx].trayBoxAmt = 8;
        smplDiskHead->trayHead[idx].smplSize = smplSize;
        smplDiskHead->trayHead[idx].smplAmt = 2000;
        smplDiskHead->trayHead[idx].smplSeq = 136;  /*随便弄个,以确定变化,这个是回传不是输入*/
    }

    if (Ok != smplDiskInit(smplDiskHead))
    {
        tempMs = sysTimeMsGet();
        return tempMs;
    }

    if (0==smplDiskHead->trayHead[0].smplSeq && !smplDiskHead->trayHead[0].diskLooped)
    {
        timeBase = sysTimeMsGet();
        for (smplSeq=0; smplSeq<2064; smplSeq++)
        {
            for (trayIdx=0; trayIdx<trayAmt; trayIdx++)
            {
                testMakeSmpl(bufWt, smplSize, smplSeq%2000, trayIdx, 13);
                ret = smplDiskWrite(trayIdx, 1, bufWt, smplSeq%2000);
                if (Ok != ret)
                {
                    tempMs = sysTimeMsGet();
                    return tempMs;
                }
                ret = smplDiskRead(trayIdx, 1, bufRd, smplSeq%2000);
                if (Ok != ret)
                {
                    tempMs = sysTimeMsGet();
                    return tempMs;
                }
                if (memcmp(bufWt, bufRd, smplSize))
                {
                    tempMs = sysTimeMsGet();
                    return tempMs;
                }
            }
        }
        timeCrnt = sysTimeMsGet();
        tempMs = timeCrnt-timeBase;
    }
    else
    {
        for (smplSeq=0; smplSeq<2000; smplSeq++)
        {
            for (trayIdx=0; trayIdx<trayAmt; trayIdx++)
            {
                testMakeSmpl(bufWt, smplSize, smplSeq, trayIdx, 13);
                timeBase = sysTimeMsGet();
                ret = smplDiskRead(trayIdx, 1, bufRd, smplSeq);
                if (Ok != ret)
                {
                    tempMs = sysTimeMsGet();
                    return tempMs;
                }
                timeCrnt = sysTimeMsGet();
                tempMs = timeCrnt-timeBase;
                if (tempMs > timeMaxRd)
                {
                    timeMaxRd = tempMs;
                }
                if (memcmp(bufWt, bufRd, smplSize))
                {
                    tempMs = sysTimeMsGet();
                    return tempMs;
                }
            }
        }

        for (smplSeq=0; smplSeq<2000; smplSeq++)
        {
            for (trayIdx=0; trayIdx<trayAmt; trayIdx++)
            {
                testMakeSmpl(bufWt, smplSize, smplSeq, trayIdx, 14);
                timeBase = sysTimeMsGet();
                ret = smplDiskWrite(trayIdx, 1, bufWt, smplSeq);
                if (Ok != ret)
                {
                    tempMs = sysTimeMsGet();
                    return tempMs;
                }
                timeCrnt = sysTimeMsGet();
                tempMs = timeCrnt-timeBase;
                if (tempMs > timeMaxWt)
                {
                    timeMaxWt = tempMs;
                }
            }
        }

        for (smplSeq=0; smplSeq<2000; smplSeq++)
        {
            for (trayIdx=0; trayIdx<trayAmt; trayIdx++)
            {
                testMakeSmpl(bufWt, smplSize, smplSeq, trayIdx, 14);
                ret = smplDiskRead(trayIdx, 1, bufRd, smplSeq);
                if (Ok != ret)
                {
                    tempMs = sysTimeMsGet();
                    return tempMs;
                }
                if (memcmp(bufWt, bufRd, smplSize))
                {
                    tempMs = sysTimeMsGet();
                    return tempMs;
                }
            }
        }
    }

    tempMs = sysTimeMsGet() + timeMaxWt + timeMaxRd;
    return tempMs;
}


#endif
