

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
#include "box.h"
#include "host.h"
#include "channel.h"
#include "protect.h"
#include "tray.h"

ChnMgr *gChnMgr;

void chnProcIgnore()
{
    return;
}

void chnProtBufInit(ChnProtBuf *chnProtBuf)
{
    chnProtBuf->cellTmprPre.tmprBeValid = False;
    chnProtBuf->cellTmprCrnt.tmprBeValid = False;
    chnProtBuf->cellTmprCrnt.tmprInvalidCnt = 0;
    chnProtBuf->newPowerBeValid = False;
    chnProtBuf->prePowerBeValid = False;
    chnProtBuf->newCauseCode = CcNone;
    chnProtBuf->preCauseCode = CcNone;
    chnProtBuf->mixSubHpnBitmap = 0;
    return;
}

/*判断通道是否在工步运行态*/
b8 chnBeInRun(Channel *chn)
{
    if (ChnStaRun==chn->chnStateMed || ChnStaLowNmlEnd==chn->chnStateMed)
    {
        return True;
    }

    return False;
}

void chnRunTimeUpd(Channel *chn, u32 timeStampMs)
{
    if (timeStampMs < chn->stepRunTimeBase)
    {
        chn->stepRunTime = (u32)0xffffffff - chn->stepRunTimeBase + 1 + timeStampMs;
    }
    else
    {
        chn->stepRunTime = timeStampMs - chn->stepRunTimeBase;
    }
    return;
}

void chnEnterIdle(Channel *chn)
{
    Cell *cell;
    Cell *cellCri;

    chn->capStep = 0;
    chn->capCtnu = 0;
    chn->stepRunTimeBase = 0;
    chn->stepRunTime = 0;
    chn->stepRunTimeCtnu = 0;
    if (ChnStaIdle == chn->chnStateMed)
    {
        chn->stepTypeTmp = StepTypeNull;
        chn->stepIdTmp = StepIdNull;
    }
    chn->chnProtBuf.idleTimeStampSec = sysTimeSecGet();
    chn->chnProtBuf.idleVolBaseValid = False;
    chn->chnProtBuf.flowIdleVolIntvlRiseBaseValid = False;

    if (NULL != chn->cell)
    {
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            cell->chnProtBuf.idleTimeStampSec = sysTimeSecGet();
            cell->chnProtBuf.idleVolBaseValid = False;
            cell->chnProtBuf.flowIdleVolIntvlRiseBaseValid = False;
        }
    }
    return;
}

void chnEnterRun(Channel *chn)
{
    Cell *cell;
    Cell *cellCri;

    chn->stepRunTime = 0;

    chn->chnProtBuf.idleTimeStampSec = sysTimeSecGet();
    chn->chnProtBuf.idleVolBaseValid = False;
    chn->chnProtBuf.flowIdleVolIntvlRiseBaseValid = False;

    if (NULL != chn->cell)
    {
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            cell->chnProtBuf.idleTimeStampSec = sysTimeSecGet();
            cell->chnProtBuf.idleVolBaseValid = False;
            cell->chnProtBuf.flowIdleVolIntvlRiseBaseValid = False;
        }
    }
    return;
}

void chnStepSwPre(Channel *chn)
{
    Cell *cell;
    Cell *cellCri;
    ChnProtBuf *chnProtBuf;

    chnProtBuf = &chn->chnProtBuf;
    chnProtBuf->idleVolCtnuSmlRiseCnt = 0;
    chnProtBuf->idleVolCtnuSmlDownCnt = 0;
    chnProtBuf->quietVolDownBaseValid = False;
    chnProtBuf->cccVolDownAbnmValid = False;
    chnProtBuf->ccVolIntvlFluctBaseValid = False;
    chnProtBuf->cccVolRiseCtnuCnt = False;
    chnProtBuf->cccVolDownCtnuCnt = False;
    chnProtBuf->ccdVolRiseAbnmValid = False;
    chnProtBuf->cvcCurRiseAbnmValid = False;
    chnProtBuf->flowIdleVolIntvlRiseBaseValid = False;
    chnProtBuf->flowCcdcVolIntvlFluctBaseValid = False;
    chnProtBuf->ccdcVolBigDownValid = False;
    chnProtBuf->idleVolBaseValid = False;

    if (NULL != chn->cell)
    {
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            chnProtBuf = &cell->chnProtBuf;
            chnProtBuf->idleVolCtnuSmlRiseCnt = 0;
            chnProtBuf->idleVolCtnuSmlDownCnt = 0;
            chnProtBuf->quietVolDownBaseValid = False;
            chnProtBuf->cccVolDownAbnmValid = False;
            chnProtBuf->ccVolIntvlFluctBaseValid = False;
            chnProtBuf->cccVolRiseCtnuCnt = False;
            chnProtBuf->cccVolDownCtnuCnt = False;
            chnProtBuf->ccdVolRiseAbnmValid = False;
            chnProtBuf->cvcCurRiseAbnmValid = False;
            chnProtBuf->flowIdleVolIntvlRiseBaseValid = False;
            chnProtBuf->flowCcdcVolIntvlFluctBaseValid = False;
            chnProtBuf->ccdcVolBigDownValid = False;
            chnProtBuf->idleVolBaseValid = False;
        }
    }
    return;
}

u8eChgType getChgType(Channel *chn, u8 stepId)
{
    UpStepInfo *step;

    step = (UpStepInfo *)getChnStepInfo(chn->box->tray->trayIdx, stepId);
    if (NULL == step)
    {
        return ChgTypeCri;
    }

    if (StepTypeCCC == step->stepType || StepTypeCCCVC == step->stepType
        || StepTypeCVC == step->stepType)
    {
        return ChgTypeChg;
    }
    if (StepTypeCCD == step->stepType || StepTypeCCCVD == step->stepType
        || StepTypeCVD == step->stepType)
    {
        return ChgTypeDisChg;
    }

    return ChgTypeCri;
}

/*下位机正常截止,这个是执行保护逻辑之后的逻辑*/
void chnStepLowNmlEnd(Channel *chn)
{
    u8 stepIdPre;
    u8 stepIdNew;

    chnStepSwPre(chn);
    chn->capStep = 0;
    chn->capCtnu = 0;
    chn->stepRunTime = 0;
    chn->stepRunTimeCtnu = 0;
    chn->stepRunTimeBase = 0;
    stepIdPre = chn->stepIdTmp;
    stepIdNew = chn->stepIdTmp + 1;
    if (stepIdNew < gTmpStepAmt[chn->box->tray->trayIdx])
    {
        UpStepInfo *step;

        step = (UpStepInfo *)getChnStepInfo(chn->box->tray->trayIdx, stepIdNew);
        chn->stepIdTmp = stepIdNew;
        chn->stepTypeTmp = chn->upStepType = step->stepType;
        if (trayChkStepNpSame(chn, stepIdPre, chn->stepIdTmp))
        {
            chn->chnStateMed = ChnStaStart;
            boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
        }
        else
        {
            trayNpChnFree(chn->box->tray, chn);
            if (Ok == trayNpChnAlloc(chn->box->tray, chn, chn->stepIdTmp))
            {
                chn->chnStateMed = ChnStaStart;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
            }
            else
            {
                chn->chnStateMed = ChnStaNpWait;
                chnEnterIdle(chn);
            }
        }
    }
    else
    {
        trayNpChnFree(chn->box->tray, chn);
        chn->chnStateMed = ChnStaIdle;
        chnEnterIdle(chn);
    }

    return;
}

/*从有效采样的数据中获取通道电源柜数据,以做保护*/
/*能直接用上传采样做保护是好的,不过托盘采样和通道采样不同,那样保护要两份*/
/*单独保存可以不用两份保护,不过要多一些原本不必要的内存操作*/
/*以后调整方向为托盘采样和通道采样相同,这是最好的*/
void chnSavePowerSmpl(Channel *chn, void *lowSmpl)
{
    TrayChnSmpl *smplCri;
    ChnProtBuf *chnProtBuf;

    chnProtBuf = &chn->chnProtBuf;
    chnProtBuf->newPowerBeValid = True;
    if (NULL == chn->cell)
    {
        ChnSmplParall *lowParalSmpl;

        lowParalSmpl = (ChnSmplParall *)lowSmpl;
        chnProtBuf->newCur = lowParalSmpl->current;
        chnProtBuf->newVolCell = lowParalSmpl->volCell;
        chnProtBuf->newVolCur = lowParalSmpl->volCur;
        chnProtBuf->newVolPort = lowParalSmpl->volPort;
        chn->volInner = lowParalSmpl->volInner;
        chn->capLow = lowParalSmpl->capacity;
        chn->stepSubType = lowParalSmpl->stepType & 0x07;
        chn->lowCause = lowParalSmpl->cause;
    }
    else
    {
        ChnSmplSeries *lowSeriesSmpl;
        CellSmplSeriesWoSw *cellSmpl;
        Cell *cell;
        Cell *cellCri;

        lowSeriesSmpl = (ChnSmplSeries *)lowSmpl;
        chnProtBuf->newCur = lowSeriesSmpl->current;
        chnProtBuf->newVolCell = 0;
        chnProtBuf->newVolCur = 0;
        chnProtBuf->newVolPort = lowSeriesSmpl->volPort;
        chn->volInner = lowSeriesSmpl->volInner;
        chn->capLow = lowSeriesSmpl->capacity;
        chn->stepSubType = lowSeriesSmpl->stepType & 0x07;
        chn->lowCause = lowSeriesSmpl->cause;

        cellSmpl = lowSeriesSmpl->cellSmplWoSw;
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            chnProtBuf = &cell->chnProtBuf;
            chnProtBuf->newPowerBeValid = True;
            chnProtBuf->newCur = lowSeriesSmpl->current;
            chnProtBuf->newVolCell = cellSmpl[cell->lowCellIdxInChn].volCell;
            chnProtBuf->newVolCur = cellSmpl[cell->lowCellIdxInChn].volCur;
            chnProtBuf->newVolPort = lowSeriesSmpl->volPort;
        }
    }
    return;
}

/*todo,目前只有并联和极简串联*/
void chnSaveTraySmpl(Channel *chn, TrayChnSmpl *trayChnSmpl)
{
    ChnProtBuf *chnProtBuf;

    chn->smplPres = True;
    chn->box->tray->smplMgr.smplChnAmt++;
    chnProtBuf = &chn->chnProtBuf;
    trayChnSmpl->chnType = ChnTypeMainChn;
    trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);
    trayChnSmpl->stepId = chn->stepIdTmp;
    trayChnSmpl->stepType = chn->stepTypeTmp;
    trayChnSmpl->stepSubType = chn->stepSubType;
    trayChnSmpl->inLoop = True;
    trayChnSmpl->causeCode = chn->lowCause;
    trayChnSmpl->stepRunTime = chn->stepRunTime + chn->stepRunTimeCtnu;
    trayChnSmpl->volCell = chnProtBuf->newVolCell;
    trayChnSmpl->volCur = chnProtBuf->newVolCur;
    trayChnSmpl->volPort = chnProtBuf->newVolPort;
    trayChnSmpl->volInner = chn->volInner;
    trayChnSmpl->current = chnProtBuf->newCur;
    trayChnSmpl->capacity = chn->capStep;
    if (NULL != chn->cell)
    {
        Cell *cell;
        Cell *cellCri;
        ChnProtBuf *cellProtBuf;
        
        trayChnSmpl++;
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++,trayChnSmpl++)
        {
            cellProtBuf = &cell->chnProtBuf;
            trayChnSmpl->chnType = ChnTypeSeriesCell;
            trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);
            trayChnSmpl->stepId = chn->stepIdTmp;
            trayChnSmpl->stepType = chn->stepTypeTmp;
            trayChnSmpl->stepSubType = chn->stepSubType;
            trayChnSmpl->inLoop = True;
            trayChnSmpl->causeCode = chn->lowCause;
            trayChnSmpl->stepRunTime = chn->stepRunTime + chn->stepRunTimeCtnu;
            trayChnSmpl->volCell = cellProtBuf->newVolCell;
            trayChnSmpl->volCur = cellProtBuf->newVolCur;
            trayChnSmpl->volPort = cellProtBuf->newVolPort;
            trayChnSmpl->volInner = chn->volInner;
            trayChnSmpl->current = cellProtBuf->newCur;
            trayChnSmpl->capacity = chn->capStep;
        }
    }

    return;
}

u8eUpChnState chnStaMapMed2Up(Channel *chn)
{
    return gDevMgr->chnStaMapMed2Up[chn->chnStateMed];
}

void chnStopByProt(Channel *chn, b8 beTrayProt)
{
    Cell *cell;
    Cell *cellCri;
    TraySmpl *traySmpl;
    TrayChnSmpl *trayChnSmpl;
    
    traySmpl = ((TraySmplRcd *)chn->box->tray->smplMgr.smplBufAddr)->traySmpl;
    trayChnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
    if (ChnStaRun == chn->chnStateMed) /*正在运行就截止容量*/
    {
        u8eChgType chgType;

        chgType = getChgType(chn, chn->stepIdTmp);
        if (ChgTypeChg == chgType)
        {
            chn->capFlowCrnt += chn->capStep;
            chn->capChgTtl += chn->capStep;
        }
        else if (ChgTypeDisChg == chgType)
        {
            chn->capFlowCrnt += chn->capStep;
            chn->capDisChgTtl += chn->capStep;
        }
    }

    if (!chn->smplPres)  /*本轮无采样就用老数据*/
    {
        chnSaveTraySmpl(chn, trayChnSmpl);
    }

    /*确定是否要下发停止和修改状态*/
    if (ChnStaRun==chn->chnStateMed || ChnStaStart==chn->chnStateMed)
    {
        chn->chnStateMed = gTmpStepProtState[chn->box->tray->trayIdx]==ChnStaPause||beTrayProt ? ChnStaMedPauseWait : ChnStaMedStopWait;
        boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
    }
    else if (ChnStaLowProtEnd==chn->chnStateMed || ChnStaLowNmlEnd==chn->chnStateMed)
    {
        chn->chnStateMed = gTmpStepProtState[chn->box->tray->trayIdx]==ChnStaPause||beTrayProt ? ChnStaMedPauseWait : ChnStaMedStopWait;
    }
    else if (ChnStaNpWait == chn->chnStateMed
        || ChnStaUpStopReq==chn->chnStateMed || ChnStaUpPauseReq==chn->chnStateMed
        || ChnStaUpStopStartReq==chn->chnStateMed || ChnStaUpPauseStartReq==chn->chnStateMed)
    {
        chn->chnStateMed = gTmpStepProtState[chn->box->tray->trayIdx]==ChnStaPause||beTrayProt ? ChnStaPause : ChnStaStop;
    }

    /*修改采样异常码和状态*/
    CcChkModify(trayChnSmpl->causeCode, chn->chnProtBuf.newCauseCode);
    if (ChnUpStateStart==trayChnSmpl->chnUpState || ChnUpStateNp==trayChnSmpl->chnUpState)
    {
        trayChnSmpl->chnUpState = ChnUpStateRun;
    }
    if (NULL != chn->cell)
    {
        trayChnSmpl++;
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++,trayChnSmpl++)
        {
            CcChkModify(cell->chnProtBuf.newCauseCode, chn->chnProtBuf.newCauseCode);
            CcChkModify(trayChnSmpl->causeCode, cell->chnProtBuf.newCauseCode);
            if (ChnUpStateStart==trayChnSmpl->chnUpState || ChnUpStateNp==trayChnSmpl->chnUpState)
            {
                trayChnSmpl->chnUpState = ChnUpStateRun;
            }
        }
    }

    trayNpChnFree(chn->box->tray, chn);
    chn->capStep = 0;
    chn->capCtnu = 0;
    chn->stepRunTime = 0;
    chn->stepRunTimeCtnu = 0;
    return;
}

void _____begin_chn_state_machine_____(){}

/*todo,三个闲时的状态检查*/
void chnStaMapIdle(Channel *chn, void *lowData)
{
    return;
}

void chnStaMapStop(Channel *chn, void *lowData)
{
    return;
}

void chnStaMapPause(Channel *chn, void *lowData)
{
    return;
}

void chnStaMapNpWait(Channel *chn, void *lowData)
{
    return;
}

/*已下发启动*/
void chnStaMapStart(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun==lowSmpl->chnLowState && chn->stepIdTmp==lowSmpl->stepId
        && chn->stepTypeTmp == lowSmpl->stepType>>3)
    {
        u8eChgType chgType;

        chn->chnStateMed = ChnStaRun;
        chn->stepRunTime = 0;
        chn->stepRunTimeBase = lowSmpl->timeStamp;
        chn->capStep = chn->capCtnu + chn->capLow;
        chgType = getChgType(chn, chn->stepIdTmp);
        if (ChgTypeCri == chn->chgTypePre)
        {
            chn->chgTypePre = chgType;
        }
        else if (ChgTypeCri!=chgType && chn->chgTypePre!=chgType)
        {
            chn->chgTypePre = chgType;
            chn->capFlowCrnt = 0;
        }

        if (0 != lowSmpl->cause)
        {
            if (ChgTypeChg == chgType)
            {
                chn->capFlowCrnt += chn->capStep;
                chn->capChgTtl += chn->capStep;
            }
            else if (ChgTypeDisChg == chgType)
            {
                chn->capFlowCrnt += chn->capStep;
                chn->capDisChgTtl += chn->capStep;
            }

            if (lowSmpl->cause < CcFlowPauseEnd) /*上来就截止*/
            {
                chn->chnStateMed = ChnStaLowNmlEnd;
            }
            else if (lowSmpl->cause > CcFlowStopEnd)  /**/
            {
                chn->chnStateMed = ChnStaLowProtEnd;
            }
        }
    }
    else
    {
        chn->dynStaCnt++;
        if (chn->dynStaCnt > LowDynStaCntMax)
        {
            setChnAbnml(chn, Cc1LowStartExpr);
        }
    }
    return;
}

/*运行态防呆,比如下位机超时不截止,todo*/
void chnStaMapRun(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun!=lowSmpl->chnLowState || chn->stepIdTmp!=lowSmpl->stepId)
    {
        setChnAbnml(chn, Cc1LowAbnmlEnd);
        return;
    }

    chnRunTimeUpd(chn, lowSmpl->timeStamp);
    chn->capStep = chn->capCtnu + chn->capLow;
    if (0 != lowSmpl->cause)
    {
        u8eChgType chgType;

        chgType = getChgType(chn, chn->stepIdTmp);
        if (ChgTypeChg == chgType)
        {
            chn->capFlowCrnt += lowSmpl->capacity;
            chn->capChgTtl += lowSmpl->capacity;
        }
        else if (ChgTypeDisChg == chgType)
        {
            chn->capFlowCrnt += lowSmpl->capacity;
            chn->capDisChgTtl += lowSmpl->capacity;
        }

        if (lowSmpl->cause < CcFlowPauseEnd)
        {
            chn->chnStateMed = ChnStaLowNmlEnd;
        }
        else if (lowSmpl->cause > CcFlowStopEnd)  /**/
        {
            chn->chnStateMed = ChnStaLowProtEnd;
        }
    }
    return;
}

/*上位机触发停止,停止已下发,需等到下位机最后的数据*/
void chnStaMapUpStopReq(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun == lowSmpl->chnLowState)
    {
        if (0 == chn->stepRunTimeBase)
        {
            u8eChgType chgType;

            chn->stepRunTime = 0;
            chn->stepRunTimeBase = lowSmpl->timeStamp;
            chn->capStep = chn->capCtnu + chn->capLow;
            chgType = getChgType(chn, chn->stepIdTmp);
            if (ChgTypeCri == chn->chgTypePre)
            {
                chn->chgTypePre = chgType;
            }
            else if (ChgTypeCri!=chgType && chn->chgTypePre!=chgType)
            {
                chn->chgTypePre = chgType;
                chn->capFlowCrnt = 0;
            }
        }
        else
        {
            chnRunTimeUpd(chn, lowSmpl->timeStamp);
            chn->capStep = chn->capCtnu + chn->capLow;
        }

        if (0 != lowSmpl->cause)
        {
            u8eChgType chgType;

            chgType = getChgType(chn, chn->stepIdTmp);
            if (ChgTypeChg == chgType)
            {
                chn->capFlowCrnt += lowSmpl->capacity;
                chn->capChgTtl += lowSmpl->capacity;
            }
            else if (ChgTypeDisChg == chgType)
            {
                chn->capFlowCrnt += lowSmpl->capacity;
                chn->capDisChgTtl += lowSmpl->capacity;
            }
            chn->chnStateMed = ChnStaUpStopEnd;
        }
        else
        {
            chn->dynStaCnt++;
            if (chn->dynStaCnt > LowDynStaCntMax)
            {
                setChnAbnml(chn, Cc1LowEndExpr);
            }
        }
    }
    else
    {
        chn->dynStaCnt++;
        if (ChnStaUpStopReq == chn->chnStateMed)
        {
            setChnAbnml(chn, Cc1LowAbnmlEnd);
        }
        else if (ChnStaUpStopStartReq == chn->chnStateMed)
        {
            if (chn->dynStaCnt > LowDynStaCntMax)
            {
                setChnAbnml(chn, Cc1LowEndExpr);
            }
        }
        else  /*ChnStaUpStopNpReq*/
        {
            chn->lowCause = CcFlowStopEnd;
            chn->chnStateMed = ChnStaUpStopEnd;
        }
    }
    return;
}

/*上位机触发暂停,停止已下发,需等到下位机最后的数据*/
void chnStaMapUpPauseReq(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun == lowSmpl->chnLowState)
    {
        if (0 == chn->stepRunTimeBase)
        {
            u8eChgType chgType;

            chn->stepRunTime = 0;
            chn->stepRunTimeBase = lowSmpl->timeStamp;
            chn->capStep = chn->capCtnu + chn->capLow;
            chgType = getChgType(chn, chn->stepIdTmp);
            if (ChgTypeCri == chn->chgTypePre)
            {
                chn->chgTypePre = chgType;
            }
            else if (ChgTypeCri!=chgType && chn->chgTypePre!=chgType)
            {
                chn->chgTypePre = chgType;
                chn->capFlowCrnt = 0;
            }
        }
        else
        {
            chnRunTimeUpd(chn, lowSmpl->timeStamp);
            chn->capStep = chn->capCtnu + chn->capLow;
        }

        if (0 != lowSmpl->cause)
        {
            u8eChgType chgType;

            chgType = getChgType(chn, chn->stepIdTmp);
            if (ChgTypeChg == chgType)
            {
                chn->capFlowCrnt += lowSmpl->capacity;
                chn->capChgTtl += lowSmpl->capacity;
            }
            else if (ChgTypeDisChg == chgType)
            {
                chn->capFlowCrnt += lowSmpl->capacity;
                chn->capDisChgTtl += lowSmpl->capacity;
            }
            if (CcFlowStopEnd == lowSmpl->cause)
            {
                chn->lowCause = CcFlowPauseEnd;
            }
            chn->chnStateMed = ChnStaUpPauseEnd;
        }
        else
        {
            chn->dynStaCnt++;
            if (chn->dynStaCnt > LowDynStaCntMax)
            {
                setChnAbnml(chn, Cc1LowEndExpr);
            }
        }
    }
    else
    {
        chn->dynStaCnt++;
        if (ChnStaUpPauseReq == chn->chnStateMed)
        {
            setChnAbnml(chn, Cc1LowAbnmlEnd);
        }
        else if (ChnStaUpPauseStartReq == chn->chnStateMed)
        {
            if (chn->dynStaCnt > LowDynStaCntMax)
            {
                setChnAbnml(chn, Cc1LowEndExpr);
            }
        }
        else  /*ChnStaUpPauseNpReq*/
        {
            chn->lowCause = CcFlowPauseEnd;
            chn->chnStateMed = ChnStaUpPauseEnd;
        }
    }
    return;
}

/*上位机触发停止已完成且已经收到下位机的截止数据并上传,等下位机空闲*/
void chnStaMapUpStopEnd(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun == lowSmpl->chnLowState)
    {
        chn->dynStaCnt++;
        chn->chnStateMed = ChnStaMedIdleWait;
        chn->stepTypeTmp = StepTypeNull;
        chn->stepIdTmp = StepIdNull;
    }
    else
    {
        chn->chnStateMed = ChnStaIdle;
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }
    return;
}

/*上位机触发暂停已完成且已经收到下位机的截止数据并上传,只等下位机空闲*/
void chnStaMapUpPauseEnd(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun == lowSmpl->chnLowState)
    {
        chn->dynStaCnt++;
        chn->chnStateMed = ChnStaMedPauseWait;
        chn->stepTypeTmp = StepTypeNull;
        chn->stepIdTmp = StepIdNull;
    }
    else
    {
        chn->chnStateMed = ChnStaPause;
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }
    return;
}

/*下位机保护,且截止数据完毕,只等下位机空闲*/
void chnStaMapLowProtEnd(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun==lowSmpl->chnLowState || LowChnStateStart==lowSmpl->chnLowState)
    {
        chn->stepTypeTmp = StepTypeNull;
        chn->stepIdTmp = StepIdNull;
        chn->chnStateMed = gTmpStepProtState[chn->box->tray->trayIdx]==ChnStaPause ? ChnStaMedPauseWait : ChnStaMedStopWait;
        chn->dynStaCnt++;
    }
    else
    {
        chn->chnStateMed = gTmpStepProtState[chn->box->tray->trayIdx];
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }
    return;
}

void chnStaMapStopWait(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun==lowSmpl->chnLowState || LowChnStateStart==lowSmpl->chnLowState)
    {
        chn->dynStaCnt++;
        if (chn->dynStaCnt > LowDynStaCntMax)
        {
            chn->chnStateMed = ChnStaStop;
            chnEnterIdle(chn);
            setChnAbnml(chn, Cc1LowEndExpr);
        }
    }
    else
    {
        chn->chnStateMed = ChnStaStop;
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }
    return;
}

/*上位机暂停,中位机启动态且停止已下发,需等下位机空闲,或中位机等待负压*/
/*注意,下发停止时,下位机可能在运行态或启动态*/
void chnStaMapPauseWait(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun==lowSmpl->chnLowState || LowChnStateStart==lowSmpl->chnLowState)
    {
        chn->dynStaCnt++;
        if (chn->dynStaCnt > LowDynStaCntMax)
        {
            chn->chnStateMed = ChnStaPause;
            chnEnterIdle(chn);
            setChnAbnml(chn, Cc1LowEndExpr);
        }
    }
    else
    {
        chn->chnStateMed = ChnStaPause;
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }
    return;
}
void chnStaMapIdleWait(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun==lowSmpl->chnLowState)
    {
        chn->dynStaCnt++;
        if (chn->dynStaCnt > LowDynStaCntMax)
        {
            chn->chnStateMed = ChnStaIdle;
            chnEnterIdle(chn);
            setChnAbnml(chn, Cc1LowEndExpr);
        }
    }
    else
    {
        chn->chnStateMed = ChnStaIdle;
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }
    return;
}

void _____endof_chn_state_machine_____(){}

void chnLowSmplProc(Channel *chn, void *upSmplBuf, void *lowSmpl)
{
    TrayChnSmpl *trayChnSmpl;

    trayChnSmpl = (TrayChnSmpl *)upSmplBuf;
    chnSavePowerSmpl(chn, lowSmpl);
    gChnMgr->chnStaMap[chn->chnStateMed](chn, lowSmpl);
    chnSaveTraySmpl(chn, trayChnSmpl);
    if (chn->box->tray->protEnable)
    {
        chnProtWork(chn);
    }
    if (ChnStaLowNmlEnd == chn->chnStateMed)
    {
        chnStepLowNmlEnd(chn);
    }
    return;
}

void ChnInit()
{
    ChnMgr *mgr;
    u16 idx;

    gChnMgr = mgr = sysMemAlloc(sizeof(ChnMgr));
    for (idx=0; idx<TimerIdCri; idx++)
    {
        mgr->chnStaMap[idx] = (ChnStaMap)chnProcIgnore;
    }
    mgr->chnStaMap[ChnStaIdle] = chnStaMapIdle;
    mgr->chnStaMap[ChnStaStop] = chnStaMapStop;
    mgr->chnStaMap[ChnStaPause] = chnStaMapPause;
    mgr->chnStaMap[ChnStaNpWait] = chnStaMapNpWait;
    mgr->chnStaMap[ChnStaStart] = chnStaMapStart;
    mgr->chnStaMap[ChnStaRun] = chnStaMapRun;
    mgr->chnStaMap[ChnStaUpStopReq] = chnStaMapUpStopReq;
    mgr->chnStaMap[ChnStaUpPauseReq] = chnStaMapUpPauseReq;
    mgr->chnStaMap[ChnStaUpStopEnd] = chnStaMapUpStopEnd;
    mgr->chnStaMap[ChnStaUpPauseEnd] = chnStaMapUpPauseEnd;
    mgr->chnStaMap[ChnStaUpStopStartReq] = chnStaMapUpStopReq;
    mgr->chnStaMap[ChnStaUpPauseStartReq] = chnStaMapUpPauseReq;
    mgr->chnStaMap[ChnStaUpStopNpReq] = chnStaMapUpStopReq;
    mgr->chnStaMap[ChnStaUpPauseNpReq] = chnStaMapUpPauseReq;
    mgr->chnStaMap[ChnStaLowProtEnd] = chnStaMapLowProtEnd;
    mgr->chnStaMap[ChnStaMedStopWait] = chnStaMapStopWait;
    mgr->chnStaMap[ChnStaMedPauseWait] = chnStaMapPauseWait;
    mgr->chnStaMap[ChnStaMedIdleWait] = chnStaMapIdleWait;

    return;
}

#endif


