
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include "box.h"
#include "host.h"
#include "uart.h"
#include "plc.h"
#include "channel.h"
#include "tray.h"
#ifdef DebugVersion
#else
#include "mlos_log.h"
#endif


void trayProtEnable(Timer *timer)
{
    Tray *tray;
    Channel *chn;
    Channel *chnCri;
    TrayProtMgr *trayProtMgr;
    ChnProtBuf *protBuf;
    Cell *cell;
    Cell *cellCri;

    tray = Container(Tray, protEnaTmr, timer);
    trayProtMgr = &tray->trayProtMgr;
    tray->protDisable &= ~ProtDisableTouch;
    trayProtMgr->slotTmprPre.tmprBeValid = False;
    trayProtMgr->preNdbdValid = False;
    trayProtMgr->smokeCtnuHpnCnt = 0;
    trayProtMgr->coCtnuHpnCnt = 0;
    trayProtMgr->allSlotTmprUpLmtCnt = 0;
    trayProtMgr->trayCausePre = CcNone;
    trayProtMgr->busySlotTmprLowLmtCnt = 0;

    for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        protBuf = &chn->chnProtBuf;
        protBuf->cellTmprPre.tmprBeValid = False;
        protBuf->prePowerBeValid = False;
        protBuf->idleVolCtnuSmlRiseCnt = 0;
        protBuf->idleVolCtnuSmlDownCnt = 0;
        protBuf->idleTmprUpSmlCnt = 0;
        protBuf->idleCurLeakCnt = 0;
        protBuf->allChnTmprUpLmtCnt = 0;
        protBuf->busyChnTmprLowLmtCnt = 0;
        protBuf->idleProtEna = False;
        chnEnterIdle(chn);
        if (NULL != chn->cell)
        {
            for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                protBuf = &cell->chnProtBuf;
                protBuf->cellTmprPre.tmprBeValid = False;
                protBuf->prePowerBeValid = False;
                protBuf->idleVolCtnuSmlRiseCnt = 0;
                protBuf->idleVolCtnuSmlDownCnt = 0;
                protBuf->idleTmprUpSmlCnt = 0;
                protBuf->idleCurLeakCnt = 0;
                protBuf->allChnTmprUpLmtCnt = 0;
                protBuf->busyChnTmprLowLmtCnt = 0;
                protBuf->idleProtEna = False;
            }

            if (NULL != chn->bypsSeriesInd)
            {
                for (cell=chn->cell; cell<cellCri; cell++)
                {
                    cellEnterIdle(cell);
                }
            }
        }
    }
    return;
}

/*??????????????????????????????????????????,??????????????????*/
/*?????????,???????????????,??????????????????????????????????????????????????????*/
Ret chnProtReverse(Channel *chn)
{
    Tray *tray;
    TraySmpl *traySmpl;
    TrayChnSmpl *trayChnSmpl;
    s32 reverseVolCfg;
    Ret ret;

    tray = chn->box->tray;
    traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
    ret = Ok;
    reverseVolCfg = chn->flowProtEntry->flowProtCfg->reverseProtVol;
    if (SmplModeTray == gDevMgr->smplMode)
    {
        if (NULL == chn->cell)  /*??????*/
        {
            if (!chn->chnProtBuf.prePowerBeValid && !chn->chnProtBuf.newPowerBeValid)
            {
                return ret; /*todo, ?????????adbt???????????????????????????,???????????????????????????,?????????????????????*/
            }

            if (chn->chnProtBuf.newVolCell < reverseVolCfg)
            {
                ret = Nok;
                tray->trayWarnPres = True;
                CcChkModify(chn->chnProtBuf.newCauseCode, Cc1Reverse);
                if (chn->smplPres)
                {
                    trayChnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
                    CcChkModify(trayChnSmpl->causeCode, Cc1Reverse);
                }
            }
        }
        else if (NULL == chn->bypsSeriesInd)
        {
            Cell *cell;
            Cell *cellCri;

            for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                if (cell->chnProtBuf.newVolCell < reverseVolCfg)
                {
                    ret = Nok;
                    CcChkModify(cell->chnProtBuf.newCauseCode, Cc1Reverse);
                    if (chn->smplPres)
                    {
                        trayChnSmpl = &traySmpl->chnSmpl[cell->genIdxInTray];
                        CcChkModify(trayChnSmpl->causeCode, Cc1Reverse);
                    }
                }
            }

            if (Nok == ret)
            {
                tray->trayWarnPres = True;
                CcChkModify(chn->chnProtBuf.newCauseCode, Cc0SeriesTrig);
                if (chn->smplPres)
                {
                    trayChnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
                    CcChkModify(trayChnSmpl->causeCode, Cc0SeriesTrig);
                }
                for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
                {
                    if (cell->chnProtBuf.newVolCell >= reverseVolCfg)
                    {
                        CcChkModify(cell->chnProtBuf.newCauseCode, Cc0SeriesTrig);
                        if (chn->smplPres)
                        {
                            trayChnSmpl = &traySmpl->chnSmpl[cell->genIdxInTray];
                            CcChkModify(trayChnSmpl->causeCode, Cc0SeriesTrig);
                        }
                    }
                }
            }
        }
        else
        {
            Cell *cell;
            Cell *cellCri;
            BypsSeriesInd *seriesInd;

            seriesInd = chn->bypsSeriesInd;
            for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                if (BitIsSet(seriesInd->runCell, cell->cellIdxInChn)
                    && cell->chnProtBuf.newVolCell<reverseVolCfg)
                {
                    BitClr(seriesInd->runCell, cell->cellIdxInChn);
                    BitSet(seriesInd->stopedCell, cell->cellIdxInChn);
                    CcChkModify(cell->chnProtBuf.newCauseCode, Cc1Reverse);
                    if (chn->smplPres)
                    {
                        trayChnSmpl = &traySmpl->chnSmpl[cell->genIdxInTray];
                        CcChkModify(trayChnSmpl->causeCode, Cc1Reverse);
                    }
                }
            }

            if (0 == seriesInd->runCell)
            {
                ret = Nok;
                tray->trayWarnPres = True;
            }
        }
    }

    return ret;
}

/*????????????????????????????????????*/
void setTrayProtCause(Tray *tray, u16eCauseCode causeCode)
{
    TrayProtMgr *trayProtMgr;

    trayProtMgr = &tray->trayProtMgr;
    CcChkModify(trayProtMgr->trayCauseNew, causeCode);
    return;
}

/*????????????????????????????????????*/
void setChnNmlProt(Channel *chn, ChnProtBuf *chnProtBuf, u16eCauseCode causeCode)
{
    CcChkModify(chnProtBuf->newCauseCode, causeCode);
    if (BoxTypeSeriesWoSw == chn->box->boxType)
    {
        CcChkModify(chn->chnProtBuf.newCauseCode, Cc0SeriesTrig);
    }

    return;
}

/*????????????????????????????????????,????????????????????????*/
void setChnAbnml(Channel *chn, u16eCauseCode causeCode)
{
    CcChkModify(chn->chnProtBuf.newCauseCode, causeCode);
    if (NULL != chn->cell)
    {
        Cell *cell;
        Cell *cellCri;

        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            CcChkModify(cell->chnProtBuf.newCauseCode, causeCode);
        }
    }
    return;
}

/*?????????????????????????????????*/
void setChnMixSubProt(Channel *chn, ChnProtBuf *chnProtBuf, u8eMixSubId mixSubId, u16eCauseCode causeCode)
{
    BitSet(chnProtBuf->mixSubHpnBitmap, mixSubId);
    chnProtBuf->mixSubHpnSec[mixSubId] = sysTimeSecGet();
    setChnNmlProt(chn, chnProtBuf, causeCode);
    return;
}

u32 resistanceCalc(Channel *chn, s32 vol01, s32 vol02, s32 current)
{
    u32 dif;
    u32 curAbs;

    curAbs = AbsVal(current);
    if (curAbs < 1000000) /*??????10A???????????????????????????????????????????????????*/
    {
        if (StepTypeCVC==chn->chnStepType || StepTypeCVD==chn->chnStepType
            || (StepTypeCCCVC==chn->chnStepType && StepSubTypeCV==chn->stepSubType)
            || (StepTypeCCCVD==chn->chnStepType && StepSubTypeCV==chn->stepSubType))
        {
            return 0;
        }
    }

    dif = AbsDifVal(vol01, vol02);
    dif *= 1000;
    curAbs /= 10;

    return 0==curAbs ? 0 : dif/curAbs;
}

void trayNpBigRiseRst(Tray *tray)
{
    tray->trayProtMgr.npBigRiseCnt = 0;
    return;
}

/*?????????????????????????????????,????????????adbt???????????????*/
/*??????????????????-500mv?????????????????????,???????????????????????????????????????????????????????????????*/
/*todo,,??????????????????,?????????????????????*/
b8 volProtIgnore(s32 baseVol, s32 crntVol)
{
    return (baseVol<-50000 && crntVol>0) ? True : False;
}

/*???????????????????????????,?????????????????????????????????*/
void chnCellLoopChk(Channel *chn)
{
    Cell *cell;
    Cell *sentry;
    BypsSeriesInd *seriesInd;
    b8 loopErr;
    
    if (ChnStaRun != chn->chnStateMed)
    {
        return;
    }

    seriesInd = chn->bypsSeriesInd;
    for (loopErr=False,cell=chn->cell,sentry=cell+chn->chnCellAmt; cell<sentry; cell++)
    {
        if (BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
        {
            if (LowBypsSwOut == cell->bypsSwState)
            {
                if (seriesInd->loopErrCnt < LowStartExprCnt)
                {
                    seriesInd->loopErrCnt++;
                    return;
                }
                loopErr = True;
                CcChkModify(cell->chnProtBuf.newCauseCode, Cc1CellLoopStatus);
            }
        }
        else
        {
            if (LowBypsSwOut != cell->bypsSwState)
            {
                if (seriesInd->loopErrCnt < LowStartExprCnt)
                {
                    seriesInd->loopErrCnt++;
                    return;
                }
                loopErr = True;
                CcChkModify(cell->chnProtBuf.newCauseCode, Cc1CellLoopStatus);
            }
        }
    }

    if (loopErr)
    {
        CcChkModify(chn->chnProtBuf.newCauseCode, Cc1CellLoopStatus);
        for (cell=chn->cell,sentry=cell+chn->chnCellAmt; cell<sentry; cell++)
        {
            if (BitIsSet(seriesInd->startCell, cell->cellIdxInChn))
            {
                CcChkModify(cell->chnProtBuf.newCauseCode, Cc0SeriesTrig);
            }
        }
    }
    seriesInd->loopErrCnt = 0;
    return;
}

/*??????????????????,??????????????????????????????,???????????????????????????*/
/*??????????????????,??????*/
void trayBusyNpBigRise0100(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    TrayProtMgr *trayProtMgr;

    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    trayProtMgr = &tray->trayProtMgr;
    if (trayProtMgr->npWiSw || !trayProtMgr->newNdbdValid || !trayProtMgr->preNdbdValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (0 == trayProtMgr->npBigRiseCnt)
        {
            if (trayProtMgr->newNdbdNp-trayProtMgr->preNdbdNp > prot->protParam[0])
            {
                trayProtMgr->npBigRiseCnt++;
                trayProtMgr->npBigRiseBase = trayProtMgr->preNdbdNp;
                trayProtMgr->newNdbdNpTsBak = trayProtMgr->newNdbdNpTs;
            }
        }
        else
        {
            if (trayProtMgr->newNdbdNpTsBak == trayProtMgr->newNdbdNpTs)
            {
                return;  /*?????????????????????????????????*/
            }

            trayProtMgr->newNdbdNpTsBak = trayProtMgr->newNdbdNpTs;
            if (trayProtMgr->newNdbdNp-trayProtMgr->npBigRiseBase > prot->protParam[0])
            {
                trayProtMgr->npBigRiseCnt++;
                if (3 == trayProtMgr->npBigRiseCnt)
                {
                    setTrayProtCause(tray, Cc3BusyNpBigRise);
                    setChnMixSubProt(chn, chnProtBuf, MixSubBusyNpBigRise, Cc3BusyNpBigRise);
                    trayProtMgr->npBigRiseCnt = 0;
                }
            }
            else
            {
                trayProtMgr->npBigRiseCnt = 0;
            }
        }
    }

    return;
}

/*????????????????????????,??????*/
void trayBusyChnTmprBigRise0101(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    TmprData *tmprPre;
    TmprData *tmprCrnt;

    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    tmprPre = &chnProtBuf->cellTmprPre;
    tmprCrnt = &chnProtBuf->cellTmprCrnt;
    if (!tmprPre->tmprBeValid || !tmprCrnt->tmprBeValid || tmprPre->tmprVal>tmprCrnt->tmprVal)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (tmprCrnt->tmprVal-tmprPre->tmprVal > prot->protParam[0])
        {
            setChnMixSubProt(chn, chnProtBuf, MixSubBusyChnTmprBigRise, Cc4BusyChnTmprBigRise);
        }
    }

    return;
}

/*??????????????????,?????????*/
void trayBusyTmprLowLmt0103(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    TmprData *tmpr;

    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (prot->paramEnable & 1<<2)  /*??????*/
    {
        TrayProtMgr *trayProtMgr;

        trayProtMgr = &tray->trayProtMgr;
        tmpr = &trayProtMgr->slotTmprCrnt;
        if (tmpr->tmprBeValid && tmpr->tmprVal<prot->protParam[2])
        {
            if (prot->paramEnable & 1<<3)
            {
                trayProtMgr->busySlotTmprLowLmtCnt++;
            }
            if (!(prot->paramEnable & 1<<3) || trayProtMgr->busySlotTmprLowLmtCnt>=prot->protParam[3])
            {
                setTrayProtCause(tray, Cc0BusySlotTmprLowLmt);
                trayProtMgr->busySlotTmprLowLmtCnt = 0;
            }
        }
        else
        {
            trayProtMgr->busySlotTmprLowLmtCnt = 0;
        }
    }

    if (prot->paramEnable & 1<<0)  /*????????????*/
    {
        tmpr = &chnProtBuf->cellTmprCrnt;
        if (tmpr->tmprBeValid && tmpr->tmprVal<prot->protParam[0])
        {
            if (prot->paramEnable & 1<<1)
            {
                chnProtBuf->busyChnTmprLowLmtCnt++;
            }
            if (!(prot->paramEnable & 1<<1) || chnProtBuf->busyChnTmprLowLmtCnt>=prot->protParam[1])
            {
                setChnNmlProt(chn, chnProtBuf, Cc2BusyChnTmprLowLmt);
                chnProtBuf->busyChnTmprLowLmtCnt = 0;
            }
        }
        else
        {
            chnProtBuf->busyChnTmprLowLmtCnt = 0;
        }
    }

    return;
}

/*?????????????????????,??????*/
void chnIdleVolRise0200(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    u32 cnt;
    s32 dif;
    u32 sysSec;
    
    if (!chnBeInIdle(chn, chnProtBuf) || !chnProtBuf->idleProtEna)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    sysSec = sysTimeSecGet();
    if (sysSec<chnProtBuf->idleTimeStampSec || sysSec-chnProtBuf->idleTimeStampSec<2)
    {
        return;  /*todo,???????????????????????????????????????,??????????????????*/
    }

    dif = chnProtBuf->newVolCell - chnProtBuf->preVolCell;
    if (prot->paramEnable & 1<<0)  /*??????1*/
    {
        if (dif > prot->protParam[0])
        {
            if (!volProtIgnore(chnProtBuf->preVolCell, chnProtBuf->newVolCell))
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubIdleVolBigRise1, Cc4IdleVolBigRise1);
            }
        }
    }

    if (prot->paramEnable & 1<<2)  /*??????2*/
    {
        if (dif > prot->protParam[2])
        {
            if (!volProtIgnore(chnProtBuf->preVolCell, chnProtBuf->newVolCell))
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubIdleVolBigRise2, Cc4IdleVolBigRise2);
            }
        }
    }

    if (prot->paramEnable & 1<<4) /*??????????????????*/
    {
        cnt = prot->paramEnable & 1<<5 ? prot->protParam[5] : 1;
        if (dif > prot->protParam[4])
        {
            if (!volProtIgnore(chnProtBuf->preVolCell, chnProtBuf->newVolCell))
            {
                chnProtBuf->idleVolCtnuSmlRiseCnt++;
                if (chnProtBuf->idleVolCtnuSmlRiseCnt >= cnt)
                {
                    setChnMixSubProt(chn, chnProtBuf, MixSubIdleVolCtnuSmlRise, Cc4IdleVolCtnuSmlRise);
                    chnProtBuf->idleVolCtnuSmlRiseCnt = 0;
                }
            }
        }
        else
        {
            chnProtBuf->idleVolCtnuSmlRiseCnt = 0;
        }
    }

    return;
}

/*?????????????????????,??????*/
void chnIdleTmprUpFluct0201(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    u32 cnt;
    s32 dif;
    u32 sysSec;
    
    if (!chnBeInIdle(chn, chnProtBuf) || !chnProtBuf->idleProtEna)
    {
        return;
    }

    if (!chnProtBuf->cellTmprCrnt.tmprBeValid || !chnProtBuf->cellTmprPre.tmprBeValid)
    {
        return;
    }

    sysSec = sysTimeSecGet();
    if (sysSec<chnProtBuf->idleTimeStampSec || sysSec-chnProtBuf->idleTimeStampSec<2)
    {
        return;  /*todo,???????????????????????????????????????,??????????????????*/
    }

    if (chnProtBuf->cellTmprCrnt.tmprVal > chnProtBuf->cellTmprPre.tmprVal)
    {
        dif = chnProtBuf->cellTmprCrnt.tmprVal - chnProtBuf->cellTmprPre.tmprVal;
    }
    else
    {
        chnProtBuf->idleTmprUpSmlCnt = 0;
        return;
    }

    if (prot->paramEnable & 1<<0)  /*??????*/
    {
        if (dif > prot->protParam[0])
        {
            setChnMixSubProt(chn, chnProtBuf, MixSubIdleChnTmprBigRise, Cc4IdleTmprBigRise);
        }
    }

    if (prot->paramEnable & 1<<2) /*??????????????????*/
    {
        cnt = prot->paramEnable & 1<<3 ? prot->protParam[3] : 1;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->idleTmprUpSmlCnt++;
            if (chnProtBuf->idleTmprUpSmlCnt >= cnt)
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubIdleChnTmprCtnuSmlRise, Cc4IdleTmprCtnuSmlRise);
                chnProtBuf->idleTmprUpSmlCnt = 0;
            }
        }
        else
        {
            chnProtBuf->idleTmprUpSmlCnt = 0;
        }
    }

    return;
}

/*?????????????????????,??????*/
void chnIdleVolDown0204(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    u32 cnt;
    s32 dif;
    u32 sysSec;
    
    if (!chnBeInIdle(chn, chnProtBuf) || !chnProtBuf->idleProtEna)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    sysSec = sysTimeSecGet();
    if (sysSec<chnProtBuf->idleTimeStampSec || sysSec-chnProtBuf->idleTimeStampSec<2)
    {
        return;  /*todo,???????????????????????????????????????,??????????????????*/
    }

    dif = chnProtBuf->preVolCell - chnProtBuf->newVolCell;
    if (prot->paramEnable & 1<<0)  /*??????1*/
    {
        if (dif > prot->protParam[0])
        {
            setChnMixSubProt(chn, chnProtBuf, MixSubIdleVolBigDown1, Cc4IdleVolBigDown1);
        }
    }

    if (prot->paramEnable & 1<<2)  /*??????2*/
    {
        if (dif > prot->protParam[2])
        {
            setChnMixSubProt(chn, chnProtBuf, MixSubIdleVolBigDown2, Cc4IdleVolBigDown2);
        }
    }

    if (prot->paramEnable & 1<<4) /*??????????????????*/
    {
        cnt = prot->paramEnable & 1<<5 ? prot->protParam[5] : 1;
        if (dif > prot->protParam[4])
        {
            chnProtBuf->idleVolCtnuSmlDownCnt++;
            if (chnProtBuf->idleVolCtnuSmlDownCnt >= cnt)
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubIdleVolCtnuSmlDown, Cc4IdleVolCtnuSmlDown);
                chnProtBuf->idleVolCtnuSmlDownCnt = 0;
            }
        }
        else
        {
            chnProtBuf->idleVolCtnuSmlDownCnt = 0;
        }
    }

    return;
}

/*??????????????????,??????*/
void trayAllNpUpLmt0300(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (prot->paramEnable & 1<<0)
    {
        if (tray->ndbdData.status[NdbdSenNpVal] > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc3AllNpUpLmt);
            setChnMixSubProt(chn, chnProtBuf, MixSubAllAirPrsUpLmt, Cc3AllNpUpLmt);
        }
    }
    return;
}

void trayProt0302CntCalc(Tray *tray, s32 tmprProt)
{
    TmprData *tmpr;

    if (BoxTypeParallel == tray->boxCfg.boxType)
    {
        Channel *chn;
        Channel *chnCri;

        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            tmpr = &chn->chnProtBuf.cellTmprCrnt;
            if (tmpr->tmprBeValid && tmpr->tmprVal>tmprProt)
            {
                tray->trayProtMgr.allChnTmprUpLmtPointAmt++;
            }
        }
    }
    else
    {
        Cell *cell;
        Cell *cellCri;

        for (cell=tray->cell,cellCri=cell+tray->trayCellAmt; cell<cellCri; cell++)
        {
            tmpr = &cell->chnProtBuf.cellTmprCrnt;
            if (tmpr->tmprBeValid && tmpr->tmprVal>tmprProt)
            {
                tray->trayProtMgr.allChnTmprUpLmtPointAmt++;
            }
        }
    }
    return;
}

/*??????????????????,??????2???,??????????????????*/
void trayAllTmprUpLmt0302(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    TmprData *tmpr;
    TrayProtMgr *trayProtMgr;
    u8 ctnuCntCfg;

    trayProtMgr = &tray->trayProtMgr;
    if (prot->paramEnable & 1<<4)  /*??????*/
    {
        tmpr = &trayProtMgr->slotTmprCrnt;
        if (tmpr->tmprBeValid)
        {
            if (tmpr->tmprVal>prot->protParam[4])
            {
                ctnuCntCfg = prot->paramEnable & 1<<5 ? prot->protParam[5] : 1;
                if (++trayProtMgr->allSlotTmprUpLmtCnt >= ctnuCntCfg)
                {
                    setTrayProtCause(tray, Cc0AllSlotTmprUpLmt);
                    trayProtMgr->allSlotTmprUpLmtCnt = 0;
                }
            }
            else
            {
                trayProtMgr->allSlotTmprUpLmtCnt = 0;
            }
        }
    }

    tmpr = &chnProtBuf->cellTmprCrnt;
    if (!tmpr->tmprBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /*????????????1,????????????*/
    {
        if (tmpr->tmprVal > prot->protParam[0])
        {
            ctnuCntCfg = prot->paramEnable & 1<<1 ? prot->protParam[1] : 1;
            if (++chnProtBuf->allChnTmprUpLmtCnt >= ctnuCntCfg)
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubAllCellTmprUpLmt1, Cc4AllCellTmprUpLmt);
                chnProtBuf->allChnTmprUpLmtCnt = 0;
            }
        }
        else
        {
            chnProtBuf->allChnTmprUpLmtCnt = 0;
        }
    }

    if (prot->paramEnable & 1<<2)  /*????????????2,???????????????*/
    {
        u8 pointAmtCfg;

        tmpr = &chnProtBuf->cellTmprCrnt;
        if (tmpr->tmprVal > prot->protParam[2])
        {
            if (0 == trayProtMgr->allChnTmprUpLmtPointAmt)
            {
                trayProt0302CntCalc(tray, prot->protParam[2]);
            }

            pointAmtCfg = prot->paramEnable & 1<<3 ? prot->protParam[3] : 3;
            if (trayProtMgr->allChnTmprUpLmtPointAmt >= pointAmtCfg)
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubAllCellTmprUpLmt2, Cc4AllCellTmprUpLmt);
            }
        }
    }

    return;
}

/*????????????,??????2???*/
void trayAllSmoke0303(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    u8 ctnuCntCfg;  /*????????????*/
    u8 pointAmtCfg;  /*??????*/
    u8 amtHpn;

    ctnuCntCfg = 1;
    pointAmtCfg = 2;
    if (NULL != prot)
    {
        if (prot->paramEnable & 1<<0)
        {
            ctnuCntCfg = prot->protParam[0];
        }
        if (prot->paramEnable & 1<<1)
        {
            pointAmtCfg = prot->protParam[1];
        }
    }

    if (tray->ndbdData.warn[NdbdWarnSmoke])
    {
        tray->trayProtMgr.smokeCtnuHpnCnt++;
        if (tray->trayProtMgr.smokeCtnuHpnCnt >= ctnuCntCfg)
        {
            setTrayProtCause(tray, Cc3AllSmoke);
            setChnMixSubProt(chn, chnProtBuf, MixSubAllSmoke1, Cc3AllSmoke);
            tray->trayProtMgr.smokeCtnuHpnCnt = 0;
        }

        amtHpn = 3==tray->ndbdData.warn[NdbdWarnSmoke] ? 2 : 1;
        if (amtHpn >= pointAmtCfg)
        {
            setTrayProtCause(tray, Cc3AllSmoke);
            setChnMixSubProt(chn, chnProtBuf, MixSubAllSmoke2, Cc3AllSmoke);
        }
    }
    else
    {
        tray->trayProtMgr.smokeCtnuHpnCnt = 0;
    }

    if (tray->ndbdData.warn[NdbdWarnCo])
    {
        tray->trayProtMgr.coCtnuHpnCnt++;
        if (tray->trayProtMgr.coCtnuHpnCnt >= ctnuCntCfg)
        {
            setTrayProtCause(tray, Cc3AllCo);
            setChnMixSubProt(chn, chnProtBuf, MixSubAllCo, Cc3AllCo);
            tray->trayProtMgr.coCtnuHpnCnt = 0;
        }
    }
    else
    {
        tray->trayProtMgr.coCtnuHpnCnt = 0;
    }

    return;
}

/*????????????,????????????,?????????*/
void chnQuietVolFluct0401(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    
    if (!chnBeInRun(chn, chnProtBuf) || StepTypeQuiet!=chn->chnStepType)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<1)  /*??????*/
    {
        if (chn->stepRunTime < prot->protParam[1])
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        dif = AbsDifVal(chnProtBuf->newVolCell,chnProtBuf->preVolCell);
        if (dif > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1QuietVolFluct);
        }
    }

    return;
}

/*????????????,????????????,??????*/
/*??????????????????????????????,?????????????????????????????????,???????????????????????????*/
void chnQuietVolDown0402(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf) || StepTypeQuiet!=chn->chnStepType)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (BitIsSet(prot->paramEnable, 0))
    {
        if (chn->stepRunTime < prot->protParam[0])
        {
            return;
        }
    }

    if (!chnProtBuf->quietVolDownBaseValid)
    {
        chnProtBuf->quietVolDownBase = chnProtBuf->newVolCell;
        chnProtBuf->quietVolDownBaseValid = True;
        return;
    }

    if (BitIsSet(prot->paramEnable, 1))  /**/
    {
        if (chnProtBuf->newVolCell < chnProtBuf->quietVolDownBase)
        {
            if (chnProtBuf->quietVolDownBase-chnProtBuf->newVolCell > prot->protParam[1])
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubQuietVolDown, Cc4QuietVolDown);
            }
        }
    }

    return;
}

/*???????????????,?????????*/
void chnQuietCurLeak0403(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 cur;

    if (!chnBeInRun(chn, chnProtBuf) || StepTypeQuiet!=chn->chnStepType)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        cur = chnProtBuf->newCur<0 ? 0-chnProtBuf->newCur : chnProtBuf->newCur;
        if (cur > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1QuietCurLeak);
        }
    }

    return;
}

/*?????????????????????,?????????*/
void chnStepVolLmt0404(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newVolCell > prot->protParam[0])
        {
            if (StepTypeQuiet == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepQuietVolUpLmt);
            }
            else if (StepTypeCCC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccVolUpLmt);
            }
            else if (StepTypeCCD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCcdVolUpLmt);
            }
            else if (StepTypeCCCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvcVolUpLmt);
            }
            else if (StepTypeCCCVD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvdVolUpLmt);
            }
            else if (StepTypeCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvcVolUpLmt);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvdVolUpLmt);
            }
            return;
        }
    }

    if (prot->paramEnable & 1<<1)
    {
        if (chnProtBuf->newVolCell < prot->protParam[1])
        {
            if (StepTypeQuiet == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepQuietVolLowLmt);
            }
            else if (StepTypeCCC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccVolLowLmt);
            }
            else if (StepTypeCCD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCcdVolLowLmt);
            }
            else if (StepTypeCCCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvcVolLowLmt);
            }
            else if (StepTypeCCCVD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvdVolLowLmt);
            }
            else if (StepTypeCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvcVolLowLmt);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvdVolLowLmt);
            }
        }
    }

    return;
}

/*????????????????????????,?????????*/
void stepChnTmprBigRise0405(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    TmprData *tmprPre;
    TmprData *tmprCrnt;

    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    tmprPre = &chnProtBuf->cellTmprPre;
    tmprCrnt = &chnProtBuf->cellTmprCrnt;
    if (!tmprPre->tmprBeValid || !tmprCrnt->tmprBeValid || tmprPre->tmprVal>tmprCrnt->tmprVal)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (tmprCrnt->tmprVal-tmprPre->tmprVal > prot->protParam[0])
        {
            if (StepTypeQuiet == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepQuietChnTmprBigRise);
            }
            else if (StepTypeCCC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccChnTmprBigRise);
            }
            else if (StepTypeCCD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCcdChnTmprBigRise);
            }
            else if (StepTypeCCCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvcChnTmprBigRise);
            }
            else if (StepTypeCCCVD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvdChnTmprBigRise);
            }
            else if (StepTypeCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvcChnTmprBigRise);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvdChnTmprBigRise);
            }
        }
    }

    return;
}

/*??????????????????,?????????*/
void chnCccSetTimeVol0406(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }
    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chn->stepRunTime <= prot->protParam[0])
        {
            if (chnProtBuf->newVolCell > prot->protParam[1])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccSetTimeVolUpLmt);
            }
        }
    }

    if (prot->paramEnable & 1<<2)
    {
        if (chn->stepRunTime > prot->protParam[2])
        {
            if (chnProtBuf->newVolCell < prot->protParam[3])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccSetTimeVolLowLmt);
            }
        }
    }

    return;
}

/*????????????,?????????*/
void chnCcCurOfst0407(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    u32 checkDelayMs;

    if (!chnBeInRun(chn, chnProtBuf) || CcNone!=chnProtBuf->newCauseCode)
    {
        return;
    }

    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType)
        && StepTypeCCD!=chn->chnStepType && (StepTypeCCCVD!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    checkDelayMs = prot->paramEnable & 1<<1 ? prot->protParam[1] : 2000;
    if (chn->stepRunTime <= checkDelayMs)
    {
        return;  /*????????????????????????????????????,??????????????????,???????????????????????????*/
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        StepObj *step;
        s32 curAbs;
        s32 dif;

        step = chn->crntStepNode->stepObj;
        curAbs = AbsVal(chnProtBuf->newCur);
        dif = AbsDifVal(curAbs, step->stepParam[0]);
        if (dif > prot->protParam[0])
        {
            if (StepTypeCCC==chn->chnStepType || StepTypeCCCVC==chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccCurOfst);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CcdCurOfst);
            }
        }
    }

    return;
}

/*cc?????????????????????,??????*/
void chnCccVolDownAbnm0408(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->cccVolDownAbnmValid)
    {
        if (!(prot->paramEnable & 1<<0)
            || chn->capStep>=(u32)prot->protParam[0] && chn->capStep<=(u32)prot->protParam[1])
        {
            chnProtBuf->cccVolDownAbnmValid = True;
            chnProtBuf->cccVolDownAbnmVolBase = chnProtBuf->newVolCell;
            chnProtBuf->cccVolDownAbnmCapMin = prot->protParam[0];
            chnProtBuf->cccVolDownAbnmCnt = 0;
        }

        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->cccVolDownAbnmCapMin != (u32)prot->protParam[0])
        {
            if (chn->capStep>=(u32)prot->protParam[0] && chn->capStep<=(u32)prot->protParam[1])
            {
                chnProtBuf->cccVolDownAbnmVolBase = chnProtBuf->newVolCell;
                chnProtBuf->cccVolDownAbnmCapMin = prot->protParam[0];
                chnProtBuf->cccVolDownAbnmCnt = 0;
            }
            return;
        }
        else
        {
            if (chn->capStep > (u32)prot->protParam[1])
            {
                chnProtBuf->cccVolDownAbnmValid = False;
                return;
            }
        }
    }

    if (chnProtBuf->newVolCell > chnProtBuf->cccVolDownAbnmVolBase)
    {
        chnProtBuf->cccVolDownAbnmVolBase = chnProtBuf->newVolCell;
        chnProtBuf->cccVolDownAbnmCnt = 0;
        return;
    }

    if (!chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (chnProtBuf->newVolCell >= chnProtBuf->preVolCell)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)
    {
        s32 dif;

        dif = chnProtBuf->cccVolDownAbnmVolBase - chnProtBuf->newVolCell;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->cccVolDownAbnmCnt++;
            if (!(prot->paramEnable & 1<<3) || chnProtBuf->cccVolDownAbnmCnt >= prot->protParam[3])
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubCcDcVolAbnm, Cc4CccVolDownAbnm);
            }
        }
    }

    return;
}

/*??????????????????,?????????*/
void chnCccVolRiseRate0409(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;

    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chn->capStep<=(u32)prot->protParam[0] || chn->capStep>(u32)prot->protParam[1])
        {
            return;
        }
    }

    if (!(prot->paramEnable & 1<<0) || 0==prot->protParam[0])
    {
        if (chn->stepRunTime <= 2000)
        {
            return;
        }
    }

    dif = chnProtBuf->newVolCell - chnProtBuf->preVolCell;
    if (prot->paramEnable & 1<<2)
    {
        if (dif > prot->protParam[2])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1CccVolRiseRateUpLmt);
        }
    }

    if (prot->paramEnable & 1<<3)
    {
        if (dif < prot->protParam[3])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1CccVolRiseRateLowLmt);
        }
    }
    return;
}

/*????????????,??????,??????*/
void chnCcVolBigDown040a(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf) || CcNone!=chnProtBuf->newCauseCode)
    {
        return;
    }

    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType)
        && StepTypeCCD!=chn->chnStepType && (StepTypeCCCVD!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chn->capStep<(u32)prot->protParam[0] || chn->capStep>(u32)prot->protParam[1])
        {
            chnProtBuf->ccdcVolBigDownValid = False;
            return;
        }
    }

    if (prot->paramEnable & 1<<3)  /*??????*/
    {
        if (chn->stepRunTime <= (u32)prot->protParam[3])
        {
            return;
        }
    }

    if ((!(prot->paramEnable & 1<<0) || 0==prot->protParam[0])
        && (!(prot->paramEnable & 1<<3) || prot->protParam[3]<=1000))
    {
        if (chn->stepRunTime <= 1000)
        {
            return;
        }
    }

    if (!chnProtBuf->ccdcVolBigDownValid)
    {
        chnProtBuf->ccdcVolBigDownValid = True;
        return; /*????????????????????????*/
    }

    if (prot->paramEnable & 1<<2)  /**/
    {
        if (chnProtBuf->newVolCell < chnProtBuf->preVolCell)
        {
            s32 dif;

            dif = chnProtBuf->preVolCell - chnProtBuf->newVolCell;
            if (dif > prot->protParam[2])
            {
                if (StepTypeCCC==chn->chnStepType || StepTypeCCCVC==chn->chnStepType)
                {
                    setChnMixSubProt(chn, chnProtBuf, MixSubCcDcVolBigDown, Cc4CccVolBigDown);
                }
                else
                {
                    setChnMixSubProt(chn, chnProtBuf, MixSubCcDcVolBigDown, Cc4CcdVolBigDown);
                }
            }
        }
    }

    return;
}

/*???????????????,?????????,todo,???????????????????????????????????????,??????????????????????????????*/
void chnStepCapLmt040b(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf) || StepTypeQuiet==chn->chnStepType)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chn->capStep > (u32)prot->protParam[0])
        {
            if (StepTypeCCC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccCapUpLmt);
            }
            else if (StepTypeCCD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCcdCapUpLmt);
            }
            else if (StepTypeCCCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvcCapUpLmt);
            }
            else if (StepTypeCCCVD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvdCapUpLmt);
            }
            else if (StepTypeCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvcCapUpLmt);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvdCapUpLmt);
            }
            return;
        }
    }

    if ((prot->paramEnable & 1<<1) && ChnStaLowNmlEnd==chn->chnStateMed)
    {
        if (chn->capStep < (u32)prot->protParam[1])
        {
            if (StepTypeCCC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccCapLowLmt);
            }
            else if (StepTypeCCD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCcdCapLowLmt);
            }
            else if (StepTypeCCCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvcCapLowLmt);
            }
            else if (StepTypeCCCVD == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCccvdCapLowLmt);
            }
            else if (StepTypeCVC == chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvcCapLowLmt);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCvdCapLowLmt);
            }
        }
    }

    return;
}

/*??????????????????,????????????,?????????*/
void chnCcVolIntvlFluct040c(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    u32 intvl;
    
    if (!chnBeInRun(chn, chnProtBuf) || CcNone!=chnProtBuf->newCauseCode)
    {
        return;
    }

    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType)
        && StepTypeCCD!=chn->chnStepType && (StepTypeCCCVD!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->ccVolIntvlFluctBaseValid)
    {
        chnProtBuf->ccVolIntvlFluctVolBase = chnProtBuf->newVolCell;
        chnProtBuf->ccVolIntvlFluctTimeBaseMs = chn->stepRunTime;
        chnProtBuf->ccVolIntvlFluctBaseValid = True;
        return;
    }

    dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->ccVolIntvlFluctVolBase);
    if (prot->paramEnable & 1<<1)  /**/
    {
        if (dif > prot->protParam[1])
        {
            if (StepTypeCCC==chn->chnStepType || StepTypeCCCVC==chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccVolIntvlFluctUpLmt);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CcdVolIntvlFluctUpLmt);
            }
        }
    }

    intvl = (prot->paramEnable & 1<<0) ? prot->protParam[0] : 10000;
    if (chn->stepRunTime - chnProtBuf->ccVolIntvlFluctTimeBaseMs < intvl)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)  /**/
    {
        if (dif < prot->protParam[2])
        {
            if (StepTypeCCC==chn->chnStepType || StepTypeCCCVC==chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccVolIntvlFluctLowLmt);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CcdVolIntvlFluctLowLmt);
            }
        }
    }

    chnProtBuf->ccVolIntvlFluctVolBase = chnProtBuf->newVolCell;
    chnProtBuf->ccVolIntvlFluctTimeBaseMs = chn->stepRunTime;
    return;
}

/*ccc??????????????????,?????????*/
void chnCccVolRiseCtnu040d(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    u32 cnt;
    s32 dif;
    
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)  /*??????*/
    {
        if (chn->stepRunTime <= prot->protParam[2])
        {
            return;
        }
    }

    if (!(prot->paramEnable & 1<<0))
    {
        return;
    }
    
    if (chnProtBuf->newVolCell > chnProtBuf->preVolCell)
    {
        dif = chnProtBuf->newVolCell - chnProtBuf->preVolCell;
        if (dif > prot->protParam[0])
        {
            chnProtBuf->cccVolRiseCtnuCnt++;
            cnt = prot->paramEnable & 1<<1 ? prot->protParam[1] : 1;
            if (chnProtBuf->cccVolRiseCtnuCnt >= cnt)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccVolRiseCtnu);
                chnProtBuf->cccVolRiseCtnuCnt = 0;
            }
            return;
        }
    }

    chnProtBuf->cccVolRiseCtnuCnt = 0;
    return;
}

/*ccc??????????????????,?????????*/
void chnCccVolDownCtnu040e(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    u32 cnt;
    s32 dif;
    
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)  /*??????*/
    {
        if (chn->stepRunTime <= prot->protParam[2])
        {
            return;
        }
    }

    if (!(prot->paramEnable & 1<<0))
    {
        return;
    }
    
    if (chnProtBuf->preVolCell > chnProtBuf->newVolCell)
    {
        dif = chnProtBuf->preVolCell - chnProtBuf->newVolCell;
        if (dif > prot->protParam[0])
        {
            chnProtBuf->cccVolDownCtnuCnt++;
            cnt = prot->paramEnable & 1<<1 ? prot->protParam[1] : 1;
            if (chnProtBuf->cccVolDownCtnuCnt >= cnt)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccVolDownCtnu);
                chnProtBuf->cccVolDownCtnuCnt = 0;
            }
            return;
        }
    }

    chnProtBuf->cccVolDownCtnuCnt = 0;
    return;
}

/*dc?????????????????????,????????????,??????*/
void chnCcdVolRiseAbnm040f(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (StepTypeCCD!=chn->chnStepType && (StepTypeCCCVD!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->ccdVolRiseAbnmValid)
    {
        if (!(prot->paramEnable & 1<<0)
            || chn->capStep>=(u32)prot->protParam[0] && chn->capStep<=(u32)prot->protParam[1])
        {
            chnProtBuf->ccdVolRiseAbnmValid = True;
            chnProtBuf->ccdVolRiseAbnmVolBase = chnProtBuf->newVolCell;
            chnProtBuf->ccdVolRiseAbnmCapMin = prot->protParam[0];
            chnProtBuf->ccdVolRiseAbnmCnt = 0;
        }

        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->ccdVolRiseAbnmCapMin != (u32)prot->protParam[0])
        {
            if (chn->capStep>=(u32)prot->protParam[0] && chn->capStep<=(u32)prot->protParam[1])
            {
                chnProtBuf->ccdVolRiseAbnmVolBase = chnProtBuf->newVolCell;
                chnProtBuf->ccdVolRiseAbnmCapMin = prot->protParam[0];
                chnProtBuf->ccdVolRiseAbnmCnt = 0;
            }
            return;
        }
        else
        {
            if (chn->capStep > (u32)prot->protParam[1])
            {
                chnProtBuf->ccdVolRiseAbnmValid = False;
                return;
            }
        }
    }

    if (chnProtBuf->newVolCell < chnProtBuf->ccdVolRiseAbnmVolBase)
    {
        chnProtBuf->ccdVolRiseAbnmVolBase = chnProtBuf->newVolCell;
        chnProtBuf->ccdVolRiseAbnmCnt = 0;
        return;
    }

    if (!chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (chnProtBuf->newVolCell <= chnProtBuf->preVolCell)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)
    {
        s32 dif;

        dif = chnProtBuf->newVolCell - chnProtBuf->ccdVolRiseAbnmVolBase;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->ccdVolRiseAbnmCnt++;
            if (!(prot->paramEnable & 1<<3) || chnProtBuf->ccdVolRiseAbnmCnt >= prot->protParam[3])
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubCcDcVolAbnm, Cc4CcdVolRiseAbnm);
            }
        }
    }

    return;
}

/*????????????,?????????*/
void chnCvcVolOfst0410(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf) || CcNone!=chnProtBuf->newCauseCode)
    {
        return;
    }

    if (StepTypeCVC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCV!=chn->stepSubType)
        && StepTypeCVD!=chn->chnStepType && (StepTypeCCCVD!=chn->chnStepType || StepSubTypeCV!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<1)  /*??????*/
    {
        if (chn->stepRunTime < prot->protParam[1])
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        StepObj *step;
        s32 dif;

        step = chn->crntStepNode->stepObj;
        dif = AbsDifVal(chnProtBuf->newVolCell, step->stepParam[1]);
        if (dif > prot->protParam[0])
        {
            if (StepTypeCVC==chn->chnStepType || StepTypeCCCVC==chn->chnStepType)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CvcVolOfst);
            }
            else
            {
                setChnNmlProt(chn, chnProtBuf, Cc1DvVolOfst);
            }
        }
    }

    return;
}

/*?????????????????????,????????????,?????????*/
void chnCvcCurRiseAbnm0411(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (StepTypeCVC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCV!=chn->stepSubType)
        && StepTypeCVD!=chn->chnStepType && (StepTypeCCCVD!=chn->chnStepType || StepSubTypeCV!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->cvcCurRiseAbnmValid)
    {
        if (!(prot->paramEnable & 1<<0)
            || chn->capStep>=(u32)prot->protParam[0] && chn->capStep<=(u32)prot->protParam[1])
        {
            chnProtBuf->cvcCurRiseAbnmValid = True;
            chnProtBuf->cvcCurRiseAbnmCurBase = chnProtBuf->newCur;
            chnProtBuf->cvcCurRiseAbnmCapMin = prot->protParam[0];
            chnProtBuf->cvcCurRiseAbnmCnt = 0;
        }

        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->cvcCurRiseAbnmCapMin != (u32)prot->protParam[0])
        {
            if (chn->capStep>=(u32)prot->protParam[0] && chn->capStep<=(u32)prot->protParam[1])
            {
                chnProtBuf->cvcCurRiseAbnmCurBase = chnProtBuf->newCur;
                chnProtBuf->cvcCurRiseAbnmCapMin = prot->protParam[0];
                chnProtBuf->cvcCurRiseAbnmCnt = 0;
            }
            return;
        }
        else
        {
            if (chn->capStep > (u32)prot->protParam[1])
            {
                chnProtBuf->cvcCurRiseAbnmValid = False;
                return;
            }
        }
    }

    if (StepTypeCVC==chn->chnStepType || StepTypeCCCVC==chn->chnStepType)
    {
        if (chnProtBuf->newCur < chnProtBuf->cvcCurRiseAbnmCurBase)
        {
            chnProtBuf->cvcCurRiseAbnmCurBase = chnProtBuf->newCur;
            chnProtBuf->cvcCurRiseAbnmCnt = 0;
            return;
        }
    }
    else
    {
        if (chnProtBuf->newCur > chnProtBuf->cvcCurRiseAbnmCurBase)
        {
            chnProtBuf->cvcCurRiseAbnmCurBase = chnProtBuf->newCur;
            chnProtBuf->cvcCurRiseAbnmCnt = 0;
            return;
        }
    }

    if (!chnProtBuf->prePowerBeValid || !(prot->paramEnable & 1<<2))
    {
        return;
    }

    if (StepTypeCVC==chn->chnStepType || StepTypeCCCVC==chn->chnStepType)
    {
        if (chnProtBuf->newCur <= chnProtBuf->preCur)
        {
            return;
        }

        dif = chnProtBuf->newCur - chnProtBuf->cvcCurRiseAbnmCurBase;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->cvcCurRiseAbnmCnt++;
            if (!(prot->paramEnable & 1<<3) || chnProtBuf->cvcCurRiseAbnmCnt >= prot->protParam[3])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CvcCurRiseAbnm);
            }
        }
    }
    else
    {
        if (chnProtBuf->newCur >= chnProtBuf->preCur)
        {
            return;
        }

        dif = chnProtBuf->cvcCurRiseAbnmCurBase - chnProtBuf->newCur;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->cvcCurRiseAbnmCnt++;
            if (!(prot->paramEnable & 1<<3) || chnProtBuf->cvcCurRiseAbnmCnt >= prot->protParam[3])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1DvCurRiseAbnm);
            }
        }
    }
    return;
}

/*????????????,????????????,?????????*/
void chnCvcCurBigRise0412(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (StepTypeCVC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCV!=chn->stepSubType)
        && StepTypeCVD!=chn->chnStepType && (StepTypeCCCVD!=chn->chnStepType || StepSubTypeCV!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chn->capStep<=(u32)prot->protParam[0] || chn->capStep>(u32)prot->protParam[1])
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<3)  /*??????*/
    {
        if (chn->stepRunTime <= prot->protParam[3])
        {
            return;
        }
    }

    if ((!(prot->paramEnable & 1<<0) || 0==prot->protParam[0])
        && (!(prot->paramEnable & 1<<3) || prot->protParam[3]<=1000))
    {
        if (chn->stepRunTime <= 1000)
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<2)  /**/
    {
        s32 dif;

        if (StepTypeCVC==chn->chnStepType || StepTypeCCCVC==chn->chnStepType)
        {
            dif = chnProtBuf->newCur - chnProtBuf->preCur;
            if (dif > prot->protParam[2])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CvcCurBigRise);
            }
        }
        else
        {
            dif = chnProtBuf->preCur - chnProtBuf->newCur;
            if (dif > prot->protParam[2])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1DvCurBigRise);
            }
        }
    }

    return;
}

/*????????????1--???????????????,????????????,?????????*/
void chnQuietVolChkBack0413(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    
    if (!chnBeInRun(chn, chnProtBuf) || StepTypeQuiet!=chn->chnStepType)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->newVolCur);
        if (dif > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1QuietVolChkBack);
        }
    }

    return;
}

/*????????????2--???????????????,????????????,????????????,?????????*/
void chnQuietVolChkBack0414(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    
    if (!chnBeInRun(chn, chnProtBuf) || StepTypeQuiet!=chn->chnStepType || NULL!=chn->cell)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->newVolPort);
        if (dif > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1QuietVolChkBack2);
        }
    }

    return;
}

/*?????????????????????,?????????*/
void flowChnTmprLmt0501(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    TmprData *tmpr;

    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    tmpr = &chnProtBuf->cellTmprCrnt;
    if (!tmpr->tmprBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /*??????????????????*/
    {
        if (tmpr->tmprVal>prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnTmprUpLmt);
        }
    }

    if (prot->paramEnable & 1<<1)  /*??????????????????*/
    {
        if (tmpr->tmprVal<prot->protParam[1])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnTmprLowLmt);
        }
    }

    return;
}

/*??????,?????????*/
void flowChgOvld0502(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf) || (StepTypeCCC!=chn->chnStepType
        && StepTypeCCCVC!=chn->chnStepType && StepTypeCVC!=chn->chnStepType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /*????????????*/
    {
        if (chnProtBuf->newVolCell > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnChgOvldVol);
        }
    }

    if (prot->paramEnable & 1<<1)  /*????????????*/
    {
        u32 capCrntTtl;

        capCrntTtl = chn->capFlowCrnt;
        if (ChnStaRun == chn->chnStateMed)
        {
            capCrntTtl += chn->capStep;
        }
        if (capCrntTtl > (u32)prot->protParam[1])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnChgOvldCap);
        }
    }

    if (prot->paramEnable & 1<<2)  /*????????????*/
    {
        s32 curAbs;

        curAbs = chnProtBuf->newCur;
        if (curAbs > prot->protParam[2])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnChgOvldCur);
        }
    }

    return;
}

/*??????,?????????*/
void flowDisChgOvld0503(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf) || (StepTypeCCD!=chn->chnStepType
        && StepTypeCCCVD!=chn->chnStepType && StepTypeCVD!=chn->chnStepType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /*????????????*/
    {
        if (chnProtBuf->newVolCell < prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnDisChgOvldVol);
        }
    }

    if (prot->paramEnable & 1<<1)  /*????????????*/
    {
        u32 capCrntTtl;

        capCrntTtl = chn->capFlowCrnt;
        if (ChnStaRun == chn->chnStateMed)
        {
            capCrntTtl += chn->capStep;
        }
        if (capCrntTtl > (u32)prot->protParam[1])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnDisChgOvldCap);
        }
    }

    if (prot->paramEnable & 1<<2)  /*????????????*/
    {
        s32 curAbs;

        curAbs = AbsVal(chnProtBuf->newCur);
        if (curAbs > prot->protParam[2])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnDisChgOvldCur);
        }
    }

    return;
}

/*??????,?????????*/
/*?????????????????????????????????????????????*/
/*??????????????????????????????????????????*/
void flowContactChk0504(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    u32 resis;

    if (!chnBeInRun(chn, chnProtBuf) || CcNone!=chnProtBuf->newCauseCode)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (NULL == chn->cell)  /*??????*/
    {
        if (prot->paramEnable & 1<<0)  /*??????????????????????????????*/
        {
            dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->newVolPort);
            if (dif > prot->protParam[0])
            {
                setChnNmlProt(chn, chnProtBuf, Cc2FlowChnVolCell2Port);
            }
        }

        if (StepTypeQuiet!=chn->chnStepType && (prot->paramEnable & 1<<3))  /*????????????*/
        {
            resis = resistanceCalc(chn, chnProtBuf->newVolPort, chnProtBuf->newVolCell, chnProtBuf->newCur);
            if (resis > prot->protParam[3])
            {
                setChnNmlProt(chn, chnProtBuf, Cc2FlowChnLoopResis);
            }
        }
    }
    else if (NULL != chn->bypsSeriesInd)  /*??????*/
    {
        Cell *cell;
        Cell *cri;
        s32 cellVolTtl;

        for (cellVolTtl=0,cell=chn->cell,cri=cell+chn->chnCellAmt; cell<cri; cell++)
        {
            if (LowBypsSwOut != cell->bypsSwState)
            {
                cellVolTtl += cell->chnProtBuf.newVolCell;
            }
        }

        if (prot->paramEnable & 1<<0)  /*??????????????????????????????*/
        {
            dif = AbsDifVal(cellVolTtl, chnProtBuf->newVolPort);
            if (dif > prot->protParam[0])
            {
                setChnNmlProt(chn, &chn->chnProtBuf, Cc2FlowChnVolCell2Port);
            }
        }

        if (StepTypeQuiet!=chn->chnStepType && (prot->paramEnable & 1<<3))  /*????????????*/
        {
            resis = resistanceCalc(chn, chnProtBuf->newVolPort, cellVolTtl, chn->chnProtBuf.newCur);
            if (resis > prot->protParam[3])
            {
                setChnNmlProt(chn, &chn->chnProtBuf, Cc2FlowChnLoopResis);
            }
        }
    }
    else if (chnProtBuf == &chn->cell->chnProtBuf) /*????????????????????????1???????????????*/
    {
        Cell *cell;
        Cell *cri;
        s32 cellVolTtl;

        for (cellVolTtl=0,cell=chn->cell,cri=cell+chn->chnCellAmt; cell<cri; cell++)
        {
            cellVolTtl += cell->chnProtBuf.newVolCell;
        }

        if (prot->paramEnable & 1<<0)  /*??????????????????????????????*/
        {
            dif = AbsDifVal(cellVolTtl, chnProtBuf->newVolPort);
            if (dif > prot->protParam[0])
            {
                setChnNmlProt(chn, chnProtBuf, Cc2FlowChnVolCell2Port);
            }
        }

        if (StepTypeQuiet!=chn->chnStepType && (prot->paramEnable & 1<<3))  /*????????????*/
        {
            resis = resistanceCalc(chn, chnProtBuf->newVolPort, cellVolTtl, chnProtBuf->newCur);
            if (resis > prot->protParam[3])
            {
                setChnNmlProt(chn, chnProtBuf, Cc2FlowChnLoopResis);
            }
        }
    }

    if (prot->paramEnable & 1<<1)  /*??????????????????????????????*/
    {
        dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->newVolCur);
        if (dif > prot->protParam[1])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnVolCell2Cur);
        }
    }

    if (StepTypeQuiet!=chn->chnStepType && (prot->paramEnable & 1<<2))  /*??????????????????*/
    {
        s32 resis;

        resis = resistanceCalc(chn, chnProtBuf->newVolCell, chnProtBuf->newVolCur, chnProtBuf->newCur);
        if (resis > prot->protParam[2])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnContactResis);
        }
    }

    return;
}

/*?????????????????????????????????,??????,??????????????????*/
void flowIdleVolIntvlRise0505(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    u32 intvlSec;
    u32 intvlCfg;
    u32 sysSec;
    
    if (!chnBeInIdle(chn, chnProtBuf) || !chnProtBuf->idleProtEna)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!(prot->paramEnable & 1<<1))
    {
        return;
    }

    sysSec = sysTimeSecGet();
    intvlCfg = (prot->paramEnable & 1<<0) ? prot->protParam[0]/1000 : 10;
    if (!chnProtBuf->flowIdleVolIntvlRiseBaseValid) /*????????????*/
    {
        intvlSec = sysSec - chnProtBuf->idleTimeStampSec;
        if (intvlSec>=2 && intvlSec>=intvlCfg)  /*??????2????????????,??????????????????*/
        {
            chnProtBuf->flowIdleVolIntvlRiseBaseValid = True;
            chnProtBuf->flowIdleVolIntvlRiseTimeBaseSec = sysSec;
            chnProtBuf->flowIdleVolIntvlRiseVolBase = chnProtBuf->newVolCell;
        }
        return;
    }

    intvlSec = sysSec - chnProtBuf->flowIdleVolIntvlRiseTimeBaseSec;
    if (intvlSec < intvlCfg)  /*????????????*/
    {
        return;
    }

    if (chnProtBuf->newVolCell > chnProtBuf->flowIdleVolIntvlRiseVolBase)
    {
        dif = chnProtBuf->newVolCell - chnProtBuf->flowIdleVolIntvlRiseVolBase;
        if (dif > prot->protParam[1])
        {
            if (!volProtIgnore(chnProtBuf->flowIdleVolIntvlRiseVolBase, chnProtBuf->newVolCell))
            {
                setTrayProtCause(tray, Cc0TrayTrig);
                setChnMixSubProt(chn, chnProtBuf, MixSubFlowIdleVolIntvlRiseUpLmt, Cc4FlowIdleVolIntvlRiseUpLmt);
            }
        }
    }

    chnProtBuf->flowIdleVolIntvlRiseVolBase = chnProtBuf->newVolCell;
    chnProtBuf->flowIdleVolIntvlRiseTimeBaseSec = sysSec;
    return;
}

/*????????????????????????,????????????,??????*/
void flowCcdcVolIntvlFluct0506(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    u32 intvl;
    
    if (!chnBeInRun(chn, chnProtBuf))
    {
        return;
    }

    if (StepTypeCCC!=chn->chnStepType && (StepTypeCCCVC!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType)
        && StepTypeCCD!=chn->chnStepType && (StepTypeCCCVD!=chn->chnStepType || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->flowCcdcVolIntvlFluctBaseValid)
    {
        chnProtBuf->flowCcdcVolIntvlFluctVolBase = chnProtBuf->newVolCell;
        chnProtBuf->flowCcdcVolIntvlFluctTimeBaseMs = chn->stepRunTime;
        chnProtBuf->flowCcdcVolIntvlFluctBaseValid = True;
        return;
    }

    intvl = (prot->paramEnable & 1<<0) ? prot->protParam[0] : 10000;
    if (chn->stepRunTime - chnProtBuf->flowCcdcVolIntvlFluctTimeBaseMs < intvl)
    {
        return;
    }

    dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->flowCcdcVolIntvlFluctVolBase);
    if (prot->paramEnable & 1<<1)  /**/
    {
        if (dif < prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubFlowCcDcVolIntvlFluctLowLmt, Cc4FlowCcdcVolIntvlFluctLowLmt);
        }
    }

    chnProtBuf->flowCcdcVolIntvlFluctVolBase = chnProtBuf->newVolCell;
    chnProtBuf->flowCcdcVolIntvlFluctTimeBaseMs = chn->stepRunTime;
    return;
}

/*????????????????????????????????????,??????*/
void flowChnCrashVol0507(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnProtBuf->idleProtEna)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newVolCell > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubFlowVolCrashStop, Cc4FlowCrashStopVolUpLmt);
            return;
        }
    }

    if (prot->paramEnable & 1<<1)
    {
        if (chnProtBuf->newVolCell < prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubFlowVolCrashStop, Cc4FlowCrashStopVolLowLmt);
        }
    }

    return;
}

/*????????????????????????*/
void flowChnCapTtl0508(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    if (!chnBeInRun(chn, chnProtBuf) || (StepTypeCCC!=chn->chnStepType
        && StepTypeCCCVC!=chn->chnStepType && StepTypeCVC!=chn->chnStepType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /*???????????????*/
    {
        s32 capTtl;

        if (chn->capChgTtl >= chn->capDisChgTtl)
        {
            capTtl = chn->capChgTtl - chn->capDisChgTtl;
        }
        else
        {
            capTtl = chn->capDisChgTtl - chn->capChgTtl;
            capTtl = 0 - capTtl;
        }

        if (ChnStaRun==chn->chnStateMed)
        {
            capTtl += chn->capStep;
        }
        if (capTtl > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnCapTtl);
        }
    }

    return;
}

/*???????????????,??????*/
void flowIdleCurLeak0509(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 cur;
    u32 cnt;
    u32 sysSec;

    if (!chnBeInIdle(chn, chnProtBuf) || !chnProtBuf->idleProtEna)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    sysSec = sysTimeSecGet();
    if (sysSec<chnProtBuf->idleTimeStampSec || sysSec-chnProtBuf->idleTimeStampSec<2)
    {
        return;  /*todo,???????????????????????????????????????,??????????????????*/
    }

    if (prot->paramEnable & 1<<0)
    {
        cur = chnProtBuf->newCur<0 ? 0-chnProtBuf->newCur : chnProtBuf->newCur;
        if (cur > prot->protParam[0])
        {
            chnProtBuf->idleCurLeakCnt++;
            cnt = prot->paramEnable & 1<<1 ? prot->protParam[1] : 1;
            if (chnProtBuf->idleCurLeakCnt >= cnt)
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubFlowIdleCurLeak, Cc4FlowIdleCurLeak);
            }
        }
        else
        {
            chnProtBuf->idleCurLeakCnt = 0;
        }
    }

    return;
}

/*????????????????????????,??????*/
/*???????????????,????????????????????????????????????????????????,???????????????????????????????????????????????????*/
void FlowIdleVolTtlRise050a(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, ProtObj *prot)
{
    s32 dif;
    u32 sysSec;

    if (!chnBeInIdle(chn, chnProtBuf) || !chnProtBuf->idleProtEna)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    sysSec = sysTimeSecGet();
    if (sysSec<chnProtBuf->idleTimeStampSec || sysSec-chnProtBuf->idleTimeStampSec<2)
    {
        return;  /*todo,???????????????????????????????????????,??????????????????*/
    }

    if (!chnProtBuf->idleVolBaseValid)
    {
        if (!(prot->paramEnable & 1<<1) || sysTimeSecGet()-chnProtBuf->idleTimeStampSec>=prot->protParam[1]/1000)
        {
            chnProtBuf->idleVolBase = chnProtBuf->newVolCell;
            chnProtBuf->idleVolBaseValid = True;
        }
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newVolCell < chnProtBuf->idleVolBase)
        {
            return;
        }

        dif = chnProtBuf->newVolCell - chnProtBuf->idleVolBase;
        if (dif > prot->protParam[0])
        {
            if (!volProtIgnore(chnProtBuf->idleVolBase, chnProtBuf->newVolCell))
            {
                setChnMixSubProt(chn, chnProtBuf, MixSubFlowIdleVolRiseTtl, Cc4FlowIdleVolRiseTtl);
            }
        }
    }

    return;
}

/*???????????????,???,?????????????????????,?????????????????????,todo:?????????,?????????????????????*/
void tmprBoxProt(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    TmprSmpl *tmprSmpl;

    if (NULL == gDevMgr->tmprSmpl)
    {
        return;
    }

    tmprSmpl = &gDevMgr->tmprSmpl[tray->tmprSmplIdxBase];
    if (!tmprSmpl->online)
    {
        setTrayProtCause(tray, Cc0TmprBoxOffline);
    }

    if (tray->trayProtMgr.slotTmprCrnt.tmprInvalidCnt > 3)
    {
        setTrayProtCause(tray, Cc0SlotTmprBad);
    }

    if (chnProtBuf->cellTmprCrnt.tmprInvalidCnt > 3)
    {
        setChnNmlProt(chn, chnProtBuf, Cc1CellTmprBad);
    }

    return;
}

void npExprProt(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    TrayNpMgr *npMgr;
    StepNpCfg *stepNpReq;
    u32 sysSec;

    npMgr = &tray->npMgr;
    if (ChnStaRun!=chn->chnStateMed && ChnStaNpWait!=chn->chnStateMed)
    {
        return;
    }

    stepNpReq = chn->crntStepNode->stepNpCfg;
    if (NULL==stepNpReq || NpTypeNone==stepNpReq->npType)
    {
        return;
    }

    sysSec = sysTimeSecGet();
    if (ChnStaRun == chn->chnStateMed)
    {
        TrayProtMgr *trayProtMgr;

        trayProtMgr = &tray->trayProtMgr;
        if (trayProtMgr->newNdbdValid)
        {
            if (trayProtMgr->newNdbdNp > stepNpReq->stepNpMax
                || trayProtMgr->newNdbdNp < stepNpReq->stepNpMin)
            {
                if (0 == npMgr->npOverLmtTimeStampSec)
                {
                    npMgr->npOverLmtTimeStampSec = sysSec;
                }
                else
                {
                    if (sysSec > npMgr->npOverLmtTimeStampSec+npMgr->npLmtExprSecT06)
                    {
                        setTrayProtCause(tray, Cc0StepNpOverLmt);
                    }
                }
            }
            else
            {
                npMgr->npOverLmtTimeStampSec = 0;
            }
        }
    }
    else /* ChnStaNpWait==chn->chnStateMed */
    {
        if (stepNpReq->npType==npMgr->npType && stepNpReq->stepNpExpect==npMgr->npExpect)
        {
            if (NpTypeNml == stepNpReq->npType) /*??????*/
            {
                if (sysSec > npMgr->npReqTimeStampSec+npMgr->brkNpExprSecT10)
                {
                    setTrayProtCause(tray, Cc0MkNpExpr);
                }
            }
            else if (sysSec > npMgr->npReqTimeStampSec+npMgr->mkNpExprSecT00)
            {
                setTrayProtCause(tray, Cc0MkNpExpr);
            }
        }
    }

    return;
}

void ndbdDevProt(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    TrayProtMgr *trayProtMgr;
    
    if (!trayNdbdBeOnline(tray->plcIdx))
    {
        setTrayProtCause(tray, Cc0NdbdDisc);
        return;
    }

    /*????????????????????????????????????????????????,??????????????????,???????????????*/
    trayProtMgr = &tray->trayProtMgr;
    if (!trayProtMgr->newNdbdValid)
    {
        return;
    }

    if (trayProtMgr->newGasWarn)
    {
        setTrayProtCause(tray, Cc0TrayGas);
    }

    if (trayProtMgr->newCoWarn)
    {
        setTrayProtCause(tray, Cc0TrayCo);
    }

    if (trayProtMgr->newFanWarn)
    {
        setTrayProtCause(tray, Cc0TrayFan);
    }
}

void trayProtWork(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    ChainS *chain;
    ListS *list;
    ProtNode *protNode;
    ProtObj *protObj;
    b8 smokeChked;

    if (NULL == tray->trayProtEntry)
    {
        trayAllSmoke0303(tray, chn, chnProtBuf, NULL);
        return;
    }

    smokeChked = False;
    list = &tray->trayProtEntry->protList;
    ListForEach(list, chain)
    {
        protNode = Container(ProtNode, chain, chain);
        protObj = protNode->protObj;
        switch (protObj->protId)
        {
            case 0x0100:
                trayBusyNpBigRise0100(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0101:
                trayBusyChnTmprBigRise0101(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0103:
                trayBusyTmprLowLmt0103(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0200:
                chnIdleVolRise0200(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0201:
                chnIdleTmprUpFluct0201(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0204:
                chnIdleVolDown0204(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0300:
                trayAllNpUpLmt0300(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0302:
                trayAllTmprUpLmt0302(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0303:
                trayAllSmoke0303(tray, chn, chnProtBuf, protObj);
                smokeChked = True;
                break;
            default:
                break;
        }
    }

    if (!smokeChked)  /*?????????????????????*/
    {
        trayAllSmoke0303(tray, chn, chnProtBuf, NULL);
    }
    return;
}
void flowProtWrok(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    ChainS *chain;
    ListS *list;
    ProtNode *protNode;
    ProtObj *protObj;

    if (NULL == chn->flowProtEntry)
    {
        return;
    }

    list = &chn->flowProtEntry->protList;
    ListForEach(list, chain)
    {
        protNode = Container(ProtNode, chain, chain);
        protObj = protNode->protObj;
        switch (protObj->protId)
        {
            case 0x0501:
                flowChnTmprLmt0501(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0502:
                flowChgOvld0502(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0503:
                flowDisChgOvld0503(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0504:
                flowContactChk0504(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0505:
                flowIdleVolIntvlRise0505(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0506:
                flowCcdcVolIntvlFluct0506(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0507:
                flowChnCrashVol0507(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0508:
                flowChnCapTtl0508(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0509:
                flowIdleCurLeak0509(tray, chn, chnProtBuf, protObj);
                break;
            case 0x050a:
                FlowIdleVolTtlRise050a(tray, chn, chnProtBuf, protObj);
                break;
            default:
                break;
        }
    }
    return;
}
void stepProtWork(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    ChainS *chain;
    ListS *list;
    ProtNode *protNode;
    ProtObj *protObj;

    if (chn->chnStateMed < ChnStaStart) /*??????????????????????????????*/
    {
        return;
    }

    if (NULL == chn->crntStepNode->stepProtEntry)  /*?????????????????????*/
    {
        return;
    }

    list = &chn->crntStepNode->stepProtEntry->protList;
    ListForEach(list, chain)
    {
        protNode = Container(ProtNode, chain, chain);
        protObj = protNode->protObj;
        switch (protObj->protId)
        {
            case 0x0401:
                chnQuietVolFluct0401(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0402:
                chnQuietVolDown0402(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0403:
                chnQuietCurLeak0403(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0404:
                chnStepVolLmt0404(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0405:
                stepChnTmprBigRise0405(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0406:
                chnCccSetTimeVol0406(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0407:
                chnCcCurOfst0407(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0408:
                chnCccVolDownAbnm0408(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0409:
                chnCccVolRiseRate0409(tray, chn, chnProtBuf, protObj);
                break;
            case 0x040a:
                chnCcVolBigDown040a(tray, chn, chnProtBuf, protObj);
                break;
            case 0x040b:
                chnStepCapLmt040b(tray, chn, chnProtBuf, protObj);
                break;
            case 0x040c:
                chnCcVolIntvlFluct040c(tray, chn, chnProtBuf, protObj);
                break;
            case 0x040d:
                chnCccVolRiseCtnu040d(tray, chn, chnProtBuf, protObj);
                break;
            case 0x040e:
                chnCccVolDownCtnu040e(tray, chn, chnProtBuf, protObj);
                break;
            case 0x040f:
                chnCcdVolRiseAbnm040f(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0410:
                chnCvcVolOfst0410(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0411:
                chnCvcCurRiseAbnm0411(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0412:
                chnCvcCurBigRise0412(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0413:
                chnQuietVolChkBack0413(tray, chn, chnProtBuf, protObj);
                break;
            case 0x0414:
                chnQuietVolChkBack0414(tray, chn, chnProtBuf, protObj);
                break;
            default:
                break;
        }
    }
    return;
}

/*??????????????????*/
void mixSubRmvExpr(MixProtCfg *mixProtCfg, ChnProtBuf *chnProtBuf, u32 crntSec)
{
    u8eMixSubId idx;

    for (idx=0; idx<MixSubCri; idx++)
    {
        if (BitIsSet(chnProtBuf->mixSubHpnBitmap, idx))
        {
            if (chnProtBuf->mixSubHpnSec[idx] + mixProtCfg->lifePeriodSec[idx] < crntSec)
            {
                BitClr(chnProtBuf->mixSubHpnBitmap, idx);
            }
        }
    }

    return;
}

/*??????????????????*/
void mixProtPolicySet(TrayProtMgr *trayProtMgr, MixProtCfg *mixProtCfg, MixExp *mixExp, u32 crntSec)
{
    MixPolicy *policy;
    MixPolicy *policyCri;

    policy = (MixPolicy *)&mixProtCfg->policy[mixExp->policyOfst];
    for (policyCri=policy+mixExp->policyAmt; policy<policyCri; policy++)
    {
        if (!trayProtMgr->policyActNeed[policy->policyId])
        {
            trayProtMgr->policyActNeed[policy->policyId] = True;
            trayProtMgr->policyActSec[policy->policyId] = crntSec + policy->delaySec;
            if (PolicyNdbdBrk == policy->policyId)
            {
                trayProtMgr->policyNdbdBrkNeed = True;
            }
        }
    }

    return;
}

/*??????????????????*/
void mixProtWork(Tray *tray, ChnProtBuf *chnProtBuf, u32 crntSec)
{
    MixProtCfg *mixProtCfg;
    TrayProtMgr *trayProtMgr;
    MixExp *mixExp;
    u8 idx;

    mixProtCfg = &tray->mixProtCfg;
    trayProtMgr = &tray->trayProtMgr;
    mixSubRmvExpr(mixProtCfg, chnProtBuf, crntSec);
    for (idx=0; idx<mixProtCfg->mixExpAmt; idx++)
    {
        if (0 != trayProtMgr->mixProtHpn[idx])
        {
            continue;
        }

        mixExp = &mixProtCfg->mixExp[idx];
        if (Ok != expSuffixCalc(&mixProtCfg->suffix[mixExp->suffixOfst],
            mixExp->suffixLen, chnProtBuf->mixSubHpnBitmap, &trayProtMgr->mixProtHpn[idx])
            || 0 == trayProtMgr->mixProtHpn[idx])
        {
            continue;
        }

        setTrayProtCause(tray, Cc4TrayMixTrag);
        CcChkModify(chnProtBuf->newCauseCode, CcMixBase+idx);
        mixProtPolicySet(trayProtMgr, mixProtCfg, mixExp, crntSec);
        break;  /*??????????????????????????????*/
    }

    return;
}

void mixProtPolicyAct(Timer *timer)
{
    TrayProtMgr *trayProtMgr;
    Tray *tray;
    u32 crntSec;
    u32 delaySec;
    u8eProtPolicy policyIdx;

    trayProtMgr = Container(TrayProtMgr, protPolicyTmr, timer);
    tray = Container(Tray, trayProtMgr, trayProtMgr);
    crntSec = sysTimeSecGet();

    if (trayProtMgr->policyNdbdBrkNeed)  /*??????????????????,?????????????????????*/
    {
        Channel *chn;
        Channel *chnCri;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (chn->chnStateMed > ChnStaNpWait) /*???????????????????????????*/
            {
                if (trayProtMgr->policyActSec[PolicyNdbdBrk] < 2)
                {
                    trayProtMgr->policyActSec[PolicyNdbdBrk] = 2;
                }
                break;
            }
        }

        trayProtMgr->policyNdbdBrkNeed = False;
    }

    /*??????????????????*/
    for (delaySec=0,policyIdx=0; policyIdx<PolicyCri; policyIdx++)
    {
        if (trayProtMgr->policyActNeed[policyIdx] && !trayProtMgr->policyActOver[policyIdx])
        {
            if (trayProtMgr->policyActSec[policyIdx] <= crntSec)
            {
                trayProtMgr->policyActOver[policyIdx] = True;
                if (PolicyNdbdBrk == policyIdx)
                {
                    trayNdbdBreak(tray->trayIdx);  /*??????,???????????????,???????????????*/
                }
                else if (PolicyFireDoorClose == policyIdx)
                {
                    trayNdbdCtrl(tray->trayIdx, NdbdSetFireDoor, 2);
                }
                else if (PolicyFanStopTray == policyIdx)
                {
                    trayNdbdCtrl(tray->trayIdx, NdbdSetSlotFan, 0);
                }
                else if (PolicySmokeRemove == policyIdx)
                {
                    trayNdbdCtrl(tray->trayIdx, NdbdSetSlotFan, 1);
                }
            }
            else
            {
                if (0 == delaySec)
                {
                    delaySec = trayProtMgr->policyActSec[policyIdx] - crntSec;
                }
                else
                {
                    delaySec = Miner(delaySec, (trayProtMgr->policyActSec[policyIdx]-crntSec));
                }
            }
        }
    }

    /*?????????????????????,??????????????????????????????????????????*/
    /*??????????????????????????????,??????????????????,???????????????????????????*/
    if (0!=delaySec && !TimerIsRun(&trayProtMgr->protPolicyTmr))
    {
        timerStart(&trayProtMgr->protPolicyTmr, TidMixProtPolicyAct, delaySec*1000, WiReset);
    }

    return;
}

/*???????????????,???????????????,?????????????????????????????????????????????*/
/*???????????????,??????????????????,???????????????,??????(??????)?????????*/
/*???????????????????????????*/
/*????????????????????????,?????????????????????,???????????????????????????*/
/*????????????????????????,???????????????????????????????????????*/
/*???????????????????????????????????????????????????????????????????????????*/
/*?????????????????????????????????????????????:0?????????,1~5????????????,7????????????*/
/*????????????????????????,??????????????????,?????????????????????????????????*/
/*????????????box?????????*/
/*????????????box???????????????*/
/*??????????????????????????????,?????????????????????(??????????????????)*/
/*????????????box?????????*/
/*???????????????(?????????????????????????????????)*/
void chnProtWork(Channel *chn)
{
    Tray *tray;
    TrayProtMgr *trayProtMgr;
    ChnProtBuf *chnProtBuf;
    Cell *cell;
    Cell *cellCri;
    u32 crntSec;

    tray = chn->box->tray;
    if (tray->protDisable)
    {
        return;
    }

    trayProtMgr = &tray->trayProtMgr;
    crntSec = sysTimeSecGet();
    if (NULL == chn->cell)
    {
        chnProtBuf = &chn->chnProtBuf;
        trayProtWork(tray, chn, chnProtBuf);
        flowProtWrok(tray, chn, chnProtBuf);
        stepProtWork(tray, chn, chnProtBuf);
        tmprBoxProt(tray, chn, chnProtBuf);
        npExprProt(tray, chn, chnProtBuf);
        ndbdDevProt(tray, chn, chnProtBuf);
        mixProtWork(tray, chnProtBuf, crntSec);
    }
    else
    {
        if (NULL != chn->bypsSeriesInd)
        {
            chnCellLoopChk(chn);
        }

        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            chnProtBuf = &cell->chnProtBuf;
            trayProtWork(tray, chn, chnProtBuf);
            flowProtWrok(tray, chn, chnProtBuf);
            stepProtWork(tray, chn, chnProtBuf);
            tmprBoxProt(tray, chn, chnProtBuf);
            npExprProt(tray, chn, chnProtBuf);
            ndbdDevProt(tray, chn, chnProtBuf);
            mixProtWork(tray, chnProtBuf, crntSec);
        }
    }

    CcChkModify(chn->chnProtBuf.newCauseCode, trayProtMgr->trayCauseNew);
    if (CcNone != chn->chnProtBuf.newCauseCode)
    {
        tray->trayWarnPres = True;
        chnStopByProt(chn, CcNone==trayProtMgr->trayCauseNew ? False : True);
    }
    else if (NULL != chn->bypsSeriesInd)
    {
        bypsSeriesCellProt(chn);
    }
    return;
}

#if 0
int main()
{
    MemDesc desc[] = { {64, 64}, {128, 32}, {256, 32}, {512, 16} };
    s8 *src = "1-4,7,9-16";
    u8 dst[128];
    u8 amt;
    u8 *infix = "(3|4|5)&6&9&12|13|10&7";
    u8 infix1[] = "(4|4|5)&6&9&12|13|10&7@2=6000,3,5=500";
    u8 infix2[] = "(3|5|9)&6&10&12|15|10&7@";
    u8 infix3[] = "(3|5|9)&6&10&12|15|10&7@6=300000";
    u8 suffix[128];
    u32 elemVal;
    u8 suffixLen;
    u32 result;
    MixProtCfg mixProtCfg;
    MixExp *item;
    u8 *suffix1;
    MixPolicy *policy;
    u8 idx;

    funcInit(desc, sizeof desc/sizeof *desc);
    logInit();
#if 0
    if (Ok == arrayParse(src, dst, 32, &amt))
    {
        printf("amt:%d\r\n", amt);
        outHex(dst, amt);
    }
#endif
#if 0
    //elemVal[4] = elemVal[6] = elemVal[9] = elemVal[12] = 1;
    if (Ok == expInfix2Suffix(infix, infix+strlen(infix), 32, suffix, suffix+128, &suffixLen)
        && Ok == expSuffixCalc(suffix, suffixLen, elemVal, &result))
    {
        printf("infix:%s\r\n", infix);
        printf("suffixLen:%d, result:%d\r\n", suffixLen, result);
        for (amt=0; amt<suffixLen; amt++)
        {
            if (suffix[amt] > 128)
            {
                printf("%2c ", suffix[amt]-128);
            }
            else
            {
                printf("%2d ", suffix[amt]);
            }
        }
        printf("\r\n");
    }
#endif
#if 1
    memset(mixProtCfg.lifePeriodSec, 0, sizeof(u32)*MixSubCri);
    mixProtCfg.mixExpAmt = 0;
    mixProtCfg.suffixOfst = 0;
    mixProtCfg.policyOfst = 0;
    if (Ok == protExpParse(infix1, strlen(infix1), &mixProtCfg)
        && Ok == protExpParse(infix2, strlen(infix2), &mixProtCfg)
        && Ok == protExpParse(infix3, strlen(infix3), &mixProtCfg))
    {
        printf("protect amt:%d\r\n", mixProtCfg.mixExpAmt);
        for (idx=0; idx<3; idx++)
        {
            item = &mixProtCfg.mixExp[idx];
            printf("item[%d] policy amt:%d\r\n", idx, item->policyAmt);
            suffix1 = &mixProtCfg.suffix[item->suffixOfst];
            for (amt=0; amt<item->suffixLen; amt++)
            {
                if (suffix1[amt] > 128)
                {
                    printf("%2c ", suffix1[amt]-128);
                }
                else
                {
                    printf("%2d ", suffix1[amt]);
                }
            }
            printf("\r\n");
            policy = (MixPolicy *)&mixProtCfg.policy[item->policyOfst];
            for (amt=0; amt<item->policyAmt; amt++,policy++)
            {
                printf("p:%d,t:%d    ", policy->policyId, policy->delaySec);
            }
            printf("\r\n");
        }
    }
    else
    {
        printf("error\r\n");
    }
#endif
    return 0;
}
#endif


#endif
