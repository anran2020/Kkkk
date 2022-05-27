

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
    chnProtBuf->mixSubHpnBitmap = 0;
    chnProtBuf->idleProtEna = False;
    return;
}

/*判断电芯是否在空闲*/
b8 chnBeInIdle(Channel *chn, ChnProtBuf *protBuf)
{
    if (NULL == chn->bypsSeriesInd)
    {
        if (chn->chnStateMed > ChnStaNpWait)
        {
            return False;
        }
    }
    else
    {
        Cell *cell;

        cell = Container(Cell, chnProtBuf, protBuf);
        if (LowBypsSwOut != cell->bypsSwState)
        {
            if (!BitIsSet(chn->bypsSeriesInd->idleSwInCell, cell->cellIdxInChn))
            {
                return False;
            }
        }
    }

    return True;
}

/*判断电芯是否在工步运行态*/
b8 chnBeInRun(Channel *chn, ChnProtBuf *protBuf)
{
    if (ChnStaRun!=chn->chnStateMed && ChnStaLowNmlEnd!=chn->chnStateMed)
    {
        return False;
    }

    if (NULL != chn->bypsSeriesInd)
    {
        Cell *cell;

        cell = Container(Cell, chnProtBuf, protBuf);
        if (!BitIsSet(chn->bypsSeriesInd->runCell, cell->cellIdxInChn))
        {
            return False;
        }
    }

    return True;
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

/*旁路串联时未必真正进入空闲,需配合其它信息判断空闲*/
void cellEnterIdle(Cell *cell)
{
    cell->chnProtBuf.idleTimeStampSec = sysTimeSecGet();
    cell->chnProtBuf.idleVolBaseValid = False;
    cell->chnProtBuf.flowIdleVolIntvlRiseBaseValid = False;
    return;
}

void chnEnterIdle(Channel *chn)
{
    Cell *cell;
    Cell *cellCri;

    chn->capStep = 0;
    chn->capCtnu = 0;
    chn->stepRunTime = 0;
    chn->stepRunTimeCtnu = 0;
    if (ChnStaIdle == chn->chnStateMed)
    {
        chn->chnStepType = StepTypeNull;
        chn->chnStepId = StepIdNull;
    }
    chn->chnProtBuf.idleTimeStampSec = sysTimeSecGet();
    chn->chnProtBuf.idleVolBaseValid = False;
    chn->chnProtBuf.flowIdleVolIntvlRiseBaseValid = False;

    if (BoxTypeSeriesWoSw == chn->box->boxType)  /*旁路串联不在这里*/
    {
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            cellEnterIdle(cell);
        }
    }
    return;
}

/*时间戳是下位机绝对时间*/
void chnEnterRun(Channel *chn, u32 timeStamp)
{
    u8eChgType chgType;
    
    chn->chnStateMed = ChnStaRun;
    chn->stepRunTime = 0;
    chn->stepRunTimeBase = timeStamp;
    chn->capStep = chn->capCtnu + chn->capLow;
    chgType = stepType2ChgType(chn->crntStepNode->stepObj->stepType);
    if (ChgTypeCri == chn->chgTypePre)
    {
        chn->chgTypePre = chgType;
    }
    else if (ChgTypeCri!=chgType && chn->chgTypePre!=chgType)
    {
        chn->chgTypePre = chgType;
        chn->capFlowCrnt = 0;
    }
    return;
}

void chnStepSwPre(Channel *chn)
{
    Cell *cell;
    Cell *cellCri;
    ChnProtBuf *chnProtBuf;

    chn->stepCapCalced = False;

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

u8eChgType stepType2ChgType(u8eStepType stepType)
{
    if (StepTypeCCC==stepType || StepTypeCCCVC==stepType || StepTypeCVC==stepType)
    {
        return ChgTypeChg;
    }
    if (StepTypeCCD==stepType || StepTypeCCCVD==stepType || StepTypeCVD==stepType)
    {
        return ChgTypeDisChg;
    }

    return ChgTypeCri;
}

void chnEndCapCalc(Channel *chn)
{
    u8eChgType chgType;
    
    if (!chn->stepCapCalced)
    {
        chn->stepCapCalced = True;
        chgType = stepType2ChgType(chn->crntStepNode->stepObj->stepType);
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

    return;
}

/*下位机正常截止,这个是执行保护逻辑之后的逻辑*/
void chnStepLowNmlEnd(Channel *chn)
{
    StepNode *stepNodeNxt;

    chnStepSwPre(chn);
    chn->capStep = 0;
    chn->capCtnu = 0;
    chn->stepRunTime = 0;
    chn->stepRunTimeCtnu = 0;
    stepNodeNxt = getNxtStep(chn, chn->crntStepNode, True);
    if (NULL == stepNodeNxt)
    {
        goto flowEnd;
    }
    if (NULL != chn->bypsSeriesInd) /*旁路串联的电芯都保护也流程结束*/
    {
        BypsSeriesInd *seriesInd;

        seriesInd = chn->bypsSeriesInd;
        if (seriesInd->startCell == (seriesInd->pausedCell|seriesInd->stopedCell))
        {
            seriesInd->nmlEndCell = seriesInd->runCell = seriesInd->endingCell = 0;
            goto flowEnd;
        }
        else
        {
            /*todo,,这里加上,,waitCell,,是否参与进来*/
            /*todo,,支持旁串续接时候,可能要重新获取下一个工步,,这里会很复杂*/
            seriesInd->runCell |= seriesInd->nmlEndCell;
            seriesInd->nmlEndCell = seriesInd->endingCell = 0;
        }
    }

    chn->crntStepNode = stepNodeNxt;
    chn->chnStepId = chn->crntStepNode->stepId;
    chn->chnStepType = chn->upStepType = stepNodeNxt->stepObj->stepType;
    if (0 == getStepEndTime(stepNodeNxt->stepObj))
    {
        /*如果工步截止时间为零,需要跳下个工步,并生成一条数据*/
        /*不能直接发下个工步,等下条采样生成工步数据,再发下个工步*/
        chn->chnStateMed = ChnStaMedEnd;
        return;
    }

    if (NULL != chn->bypsSeriesInd)
    {
        Cell *cell;
        Cell *cellCri;
        BypsSeriesInd *seriesInd;

        seriesInd = chn->bypsSeriesInd;
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            if (BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
            {
                cell->cellUpStepId = chn->chnStepId;
                cell->cellUpStepType = chn->upStepType;
                cell->cellCapCtnu = 0;
                cell->cellCapStep = 0;
            }
        }
    }

    if (Ok == trayNpChnReAlloc(chn->box->tray, chn))
    {
        chn->chnStateMed = ChnStaStart;
        boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
    }
    else
    {
        chn->chnStateMed = ChnStaNpWait;
        chnEnterIdle(chn);
    }
    return;

flowEnd:
    chn->chnStateMed = ChnStaIdle;
    trayNpChnFree(chn->box->tray, chn);
    chnEnterIdle(chn);
    if (NULL != chn->bypsSeriesInd) /*旁路串联要求下停止*/
    {
        boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
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
        chn->chnLowCause = lowParalSmpl->cause;
        chn->tmprPower = (u16)lowParalSmpl->tempPower;
        chn->volBus = lowParalSmpl->volBus;
    }
    else
    {
        ChnSmplSeries *lowSeriesSmpl;
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
        chn->chnLowCause = lowSeriesSmpl->cause;
        chn->tmprPower = (u16)lowSeriesSmpl->tempPower;
        chn->volBus = lowSeriesSmpl->volBus;

        if (NULL == chn->bypsSeriesInd)
        {
            CellSmplSeriesWoSw *cellSmplWoSw;

            cellSmplWoSw = lowSeriesSmpl->cellSmplWoSw;
            for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                chnProtBuf = &cell->chnProtBuf;
                chnProtBuf->newPowerBeValid = True;
                chnProtBuf->newCur = lowSeriesSmpl->current;
                chnProtBuf->newVolCell = cellSmplWoSw[cell->lowCellIdxInChn].volCell;
                chnProtBuf->newVolCur = cellSmplWoSw[cell->lowCellIdxInChn].volCur;
                chnProtBuf->newVolPort = lowSeriesSmpl->volPort;
            }
        }
        else
        {
            CellSmplSeriesWiSw *cellSmplWiSw;
            BypsSeriesInd *seriesInd;

            seriesInd = chn->bypsSeriesInd;
            for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                chnProtBuf = &cell->chnProtBuf;
                cellSmplWiSw = &lowSeriesSmpl->cellSmplWiSw[cell->lowCellIdxInChn];
                chnProtBuf->newPowerBeValid = True;
                chnProtBuf->newCur = LowBypsSwOut==cellSmplWiSw->swState ? 0 : lowSeriesSmpl->current;
                chnProtBuf->newVolCell = cellSmplWiSw->volCell;
                chnProtBuf->newVolCur = cellSmplWiSw->volCur;
                chnProtBuf->newVolPort = lowSeriesSmpl->volPort;

                cell->cellLowCause = cellSmplWiSw->cause;
                if (CcFlowStopEnd==cell->cellLowCause || CcFlowJumpEnd==cell->cellLowCause)
                {
                    /*中位机触发保护后的下位机截止,过滤掉异常码和电流*/
                    if (BitIsSet(seriesInd->medProtCell, cell->cellIdxInChn))
                    {
                        cell->cellLowCause = 0;
                        //chnProtBuf->newCur = 0;  /*先不过滤,,todo,,以后看情况*/
                    }
                }

                if (LowBypsSwOut!=cell->bypsSwState && LowBypsSwOut==cellSmplWiSw->swState)
                {
                    if (!BitIsSet(seriesInd->idleSwInCell, cell->cellIdxInChn))
                    {
                        cellEnterIdle(cell);  /*切入变为切出*/
                    }
                }
                cell->bypsSwState = cellSmplWiSw->swState;
            }
        }
    }
    return;
}

void chnSaveTraySmpl(Channel *chn, TrayChnSmpl *trayChnSmpl)
{
    ChnProtBuf *chnProtBuf;

    chn->smplPres = True;
    chn->box->tray->smplMgr.smplChnAmt++;
    chnProtBuf = &chn->chnProtBuf;
    trayChnSmpl->chnType = ChnTypeMainChn;
    trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);
    trayChnSmpl->stepId = chn->chnStepId;
    trayChnSmpl->stepType = chn->chnStepType;
    trayChnSmpl->stepSubType = chn->stepSubType;
    trayChnSmpl->inLoop = True;
    trayChnSmpl->causeCode = chn->chnLowCause;
    trayChnSmpl->stepRunTime = chn->stepRunTime + chn->stepRunTimeCtnu;
    trayChnSmpl->volCell = chnProtBuf->newVolCell;
    trayChnSmpl->volCur = chnProtBuf->newVolCur;
    trayChnSmpl->volPort = chnProtBuf->newVolPort;
    trayChnSmpl->volInner = chn->volInner;
    trayChnSmpl->current = chnProtBuf->newCur;
    trayChnSmpl->capacity = chn->capStep;
    trayChnSmpl->tmprPower = chn->tmprPower;
    trayChnSmpl->volBus = chn->volBus;
    if (NULL != chn->cell)
    {
        Cell *cell;
        Cell *cellCri;
        ChnProtBuf *cellProtBuf;
        
        trayChnSmpl++;
        if (NULL == chn->bypsSeriesInd)
        {
            for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++,trayChnSmpl++)
            {
                cellProtBuf = &cell->chnProtBuf;
                trayChnSmpl->chnType = ChnTypeSeriesCell;
                trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);
                trayChnSmpl->stepId = chn->chnStepId;
                trayChnSmpl->stepType = chn->chnStepType;
                trayChnSmpl->stepSubType = chn->stepSubType;
                trayChnSmpl->inLoop = True;
                trayChnSmpl->causeCode = chn->chnLowCause;
                trayChnSmpl->stepRunTime = chn->stepRunTime + chn->stepRunTimeCtnu;
                trayChnSmpl->volCell = cellProtBuf->newVolCell;
                trayChnSmpl->volCur = cellProtBuf->newVolCur;
                trayChnSmpl->volPort = cellProtBuf->newVolPort;
                trayChnSmpl->volInner = chn->volInner;
                trayChnSmpl->current = cellProtBuf->newCur;
                trayChnSmpl->capacity = chn->capStep;
                trayChnSmpl->tmprPower = chn->tmprPower;
                trayChnSmpl->volBus = chn->volBus;
            }
        }
        else  /*如下旁串数据作为过渡,后续需要细化cell结构体数据后重新梳理,,todo*/
        {  /*梳理目标:不以下位机回路状态为逻辑依据,只以cell结构体数据为依据*/
            BypsSeriesInd *seriesInd;

            seriesInd = chn->bypsSeriesInd;
            for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++,trayChnSmpl++)
            {
                cellProtBuf = &cell->chnProtBuf;
                trayChnSmpl->chnType = ChnTypeSeriesCell;
                trayChnSmpl->inLoop = cell->bypsSwState;
                trayChnSmpl->stepId = cell->cellUpStepId;
                trayChnSmpl->stepType = cell->cellUpStepType;

                /*适配下位机:主通道启动态,电芯正常截止,状态机处理在chnLowSmplProc*/
                if (ChnStaStart == chn->chnStateMed && 0!=cell->cellLowCause && cell->cellLowCause<CcFlowPauseEnd)
                {
                    trayChnSmpl->chnUpState = ChnUpStateRun;

                    /*todo,,下面两行,需要整理到状态机中,不应在这里处理逻辑*/
                    BitClr(seriesInd->runCell, cell->cellIdxInChn);
                    BitSet(seriesInd->nmlEndCell, cell->cellIdxInChn);
                }
                else if (BitIsSet(seriesInd->startCell, cell->cellIdxInChn))
                {
                    if (LowBypsSwOut == cell->bypsSwState)
                    {
                        trayChnSmpl->chnUpState = ChnUpStateIdle;

                        if (BitIsSet(seriesInd->pausedCell, cell->cellIdxInChn))
                        {
                            trayChnSmpl->chnUpState = ChnUpStatePause;
                        }
                        else if (ChnStaNpWait==chn->chnStateMed && BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
                        {
                            trayChnSmpl->chnUpState = ChnUpStateNp;
                        }
                        else if (ChnStaStart==chn->chnStateMed && BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
                        {
                            trayChnSmpl->chnUpState = ChnUpStateStart;
                        }
                        else if (ChnStaRun==chn->chnStateMed && BitIsSet(seriesInd->nmlEndCell, cell->cellIdxInChn))
                        {
                            trayChnSmpl->chnUpState = ChnUpStateStart;
                        }
                        else if (ChnStaRun==chn->chnStateMed && BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
                        {
                            trayChnSmpl->chnUpState = ChnUpStateStart;
                        }
                        else if (BitIsSet(seriesInd->waitCell, cell->cellIdxInChn))
                        {
                            trayChnSmpl->chnUpState = ChnUpStateStart;
                        }
                    }
                    else if (!BitIsSet(seriesInd->idleSwInCell, cell->cellIdxInChn))
                    {
                        trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);

                        /*中位机触发保护后的下位机截止*/
                        if (BitIsSet(seriesInd->medProtCell, cell->cellIdxInChn))
                        {
                            if (BitIsSet(seriesInd->pausedCell, cell->cellIdxInChn))
                            {
                                trayChnSmpl->chnUpState = ChnUpStatePause;
                            }
                            else if (BitIsSet(seriesInd->stopedCell, cell->cellIdxInChn))
                            {
                                trayChnSmpl->chnUpState = ChnUpStateIdle;
                            }
                        }
                        else if (!BitIsSet(seriesInd->runCell, cell->cellIdxInChn)
                            && !BitIsSet(seriesInd->nmlEndCell, cell->cellIdxInChn)
                            && !BitIsSet(seriesInd->waitCell, cell->cellIdxInChn))
                        {
                            trayChnSmpl->chnUpState = ChnUpStateRun;  /*上位机截止,有个滞后*/
                        }
                    }
                }
                else
                {
                    trayChnSmpl->chnUpState = ChnUpStateIdle;
                }

                trayChnSmpl->stepSubType = chn->stepSubType;
                trayChnSmpl->causeCode = cell->cellLowCause;
                if (LowBypsSwOut!=cell->bypsSwState && ChnStaRun==chn->chnStateMed
                    && BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
                {
                    cell->cellCapStep = chn->capStep;
                    cell->cellStepRunTime = chn->stepRunTime + chn->stepRunTimeCtnu;
                }
                trayChnSmpl->stepRunTime = cell->cellStepRunTime;
                trayChnSmpl->volCell = cellProtBuf->newVolCell;
                trayChnSmpl->volCur = cellProtBuf->newVolCur;
                trayChnSmpl->volPort = cellProtBuf->newVolPort;
                trayChnSmpl->volInner = chn->volInner;
                trayChnSmpl->current = cellProtBuf->newCur;
                trayChnSmpl->capacity = cell->cellCapStep;
                trayChnSmpl->tmprPower = chn->tmprPower;
                trayChnSmpl->volBus = chn->volBus;
            }
        }
    }

    return;
}

void chnSaveTraySmplOld(Channel *chn, u16eCauseCode cause)
{
    TraySmpl *traySmpl;
    TrayChnSmpl *trayChnSmpl;

    traySmpl = ((TraySmplRcd *)chn->box->tray->smplMgr.smplBufAddr)->traySmpl;
    trayChnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
    chnSaveTraySmpl(chn, trayChnSmpl);
    trayChnSmpl->causeCode = cause;
    if (NULL != chn->cell)
    {
        Cell *cell;
        Cell *cellCri;

        trayChnSmpl++;
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            trayChnSmpl->causeCode = cause;
            trayChnSmpl++;
        }
    }
    return;
}

u8eUpChnState chnStaMapMed2Up(Channel *chn)
{
    return gDevMgr->chnStaMapMed2Up[chn->chnStateMed];
}

void bypsSeriesProtSet(Channel *chn, u8eChnProtPolicy protPolicy)
{
    if (NULL != chn->bypsSeriesInd)
    {
        BypsSeriesInd *seriesInd;

        seriesInd = chn->bypsSeriesInd;
        seriesInd->medProtCell = seriesInd->startCell;
        if (ChnProtPause == protPolicy)
        {
            seriesInd->pausedCell = seriesInd->startCell & ~seriesInd->stopedCell;
        }
        else
        {
            seriesInd->stopedCell = seriesInd->startCell & ~seriesInd->pausedCell;
        }
    }
    return;
}

void bypsSeriesCellProt(Channel *chn)
{
    Tray *tray;
    Cell *cell;
    Cell *sentry;
    BypsSeriesInd *seriesInd;
    TraySmplRcd *traySmplRcd;
    TrayChnSmpl *trayChnSmpl;

    tray = chn->box->tray;
    traySmplRcd = (TraySmplRcd *)tray->smplMgr.smplBufAddr;
    trayChnSmpl = &traySmplRcd->traySmpl->chnSmpl[chn->cell->genIdxInTray];
    for (cell=chn->cell,sentry=cell+chn->chnCellAmt; cell<sentry; cell++,trayChnSmpl++)
    {
        CcChkModify(trayChnSmpl->causeCode, cell->chnProtBuf.newCauseCode);
    }

    if (ChnStaRun!=chn->chnStateMed && ChnStaLowNmlEnd!=chn->chnStateMed)
    {
        return;
    }

    seriesInd = chn->bypsSeriesInd;
    for (cell=chn->cell,sentry=cell+chn->chnCellAmt; cell<sentry; cell++)
    {
        if (CcNone != cell->chnProtBuf.newCauseCode)
        {
            if (BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
            {
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
                BitSet(seriesInd->stopingCell, cell->cellIdxInChn);
            }
            BitClr(seriesInd->runCell, cell->cellIdxInChn);
            BitClr(seriesInd->endingCell, cell->cellIdxInChn);
            BitClr(seriesInd->nmlEndCell, cell->cellIdxInChn);
            BitClr(seriesInd->waitCell, cell->cellIdxInChn);
            BitSet(seriesInd->medProtCell, cell->cellIdxInChn);
            if (ChnProtPause == chn->flowProtEntry->flowProtCfg->protPolicy)
            {
                BitSet(seriesInd->pausedCell, cell->cellIdxInChn);
            }
            else
            {
                BitSet(seriesInd->stopedCell, cell->cellIdxInChn);
            }
        }
    }

    if (seriesInd->runCell == seriesInd->endingCell)
    {
        chn->chnStateMed = ChnStaLowNmlEnd;
        chnEndCapCalc(chn);
    }
    return;
}

void chnStopByProt(Channel *chn, b8 beTrayProt)
{
    Tray *tray;
    Cell *cell;
    Cell *cellCri;
    TraySmplRcd *traySmplRcd;
    TrayChnSmpl *trayChnSmpl;
    u8eChnProtPolicy protPolicy;
    u8 expId;

    tray = chn->box->tray;
    traySmplRcd = (TraySmplRcd *)tray->smplMgr.smplBufAddr;
    trayChnSmpl = &traySmplRcd->traySmpl->chnSmpl[chn->genIdxInTray];
    if (ChnStaRun == chn->chnStateMed) /*正在运行就截止容量*/
    {
        chnEndCapCalc(chn);
    }

    if (!chn->smplPres)  /*本轮无采样就用老数据*/
    {
        chnSaveTraySmpl(chn, trayChnSmpl);
    }

    /*确定是否要下发停止和修改状态*/
    protPolicy = NULL==chn->flowProtEntry ? ChnProtPause : chn->flowProtEntry->flowProtCfg->protPolicy;
    protPolicy = beTrayProt ? ChnProtPause : protPolicy;
    bypsSeriesProtSet(chn, protPolicy);
    if (ChnStaRun==chn->chnStateMed || ChnStaStart==chn->chnStateMed)
    {
        chn->chnStateMed = protPolicy==ChnProtPause ? ChnStaMedPauseWait : ChnStaMedIdleWait;
        boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
    }
    else if (ChnStaLowProtEnd==chn->chnStateMed || ChnStaLowNmlEnd==chn->chnStateMed)
    {
        chn->chnStateMed = protPolicy==ChnProtPause ? ChnStaMedPauseWait : ChnStaMedIdleWait;
    }
    else if (ChnStaNpWait==chn->chnStateMed || ChnStaMedEnd==chn->chnStateMed)
    {
        chn->chnStateMed = protPolicy==ChnProtPause ? ChnStaPause : ChnStaIdle;
        if (ChnStaIdle == chn->chnStateMed)
        {
            chn->chnStepType = StepTypeNull;
            chn->chnStepId = StepIdNull;
        }
    }

    /*修改采样中的异常码和状态*/
    CcChkModify(trayChnSmpl->causeCode, chn->chnProtBuf.newCauseCode);
    if (CcMixBase == CcBaseGet(trayChnSmpl->causeCode))
    {
        expId = trayChnSmpl->causeCode & 0xff;
        trayChnSmpl->causeCode = (0x80+expId << 24) + tray->trayProtMgr.mixProtHpn[expId];
    }
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
            if (CcMixBase == CcBaseGet(trayChnSmpl->causeCode))
            {
                expId = trayChnSmpl->causeCode & 0xff;
                trayChnSmpl->causeCode = (0x80+expId << 24) + tray->trayProtMgr.mixProtHpn[expId];
            }
            if (ChnUpStateStart==trayChnSmpl->chnUpState || ChnUpStateNp==trayChnSmpl->chnUpState)
            {
                trayChnSmpl->chnUpState = ChnUpStateRun;
            }
        }
    }

    trayNpChnFree(tray, chn);
    chn->capStep = 0;
    chn->capCtnu = 0;
    chn->stepRunTime = 0;
    chn->stepRunTimeCtnu = 0;
    return;
}

void _____begin_chn_state_machine_____(){}

void chnStaMapIdle(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun == lowSmpl->chnLowState)
    {
        boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
        chn->chnStateMed = ChnStaIdle==chn->chnStateMed ? ChnStaMedIdleWait : ChnStaMedPauseWait;
    }

    if (lowSmpl->cause <= CcFlowJumpEnd)
    {
        lowSmpl->cause = 0;
    }
    return;
}

void chnStaMapNpWait(Channel *chn, void *lowData)
{
    return;
}

/*仅针对旁路串联扇入,不属于独立状态机*/
void chnStaMapRunWiSw(Channel *chn)
{
    Cell *cell;
    Cell *cellCri;
    BypsSeriesInd *seriesInd;
    
    seriesInd = chn->bypsSeriesInd;
    for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
    {
        if (BitIsSet(seriesInd->endingCell, cell->cellIdxInChn))
        {
            BitSet(seriesInd->nmlEndCell, cell->cellIdxInChn);
            BitClr(seriesInd->endingCell, cell->cellIdxInChn);
            BitClr(seriesInd->runCell, cell->cellIdxInChn);
        }
    }
    
    if (0 != chn->chnLowCause)
    {
        chnEndCapCalc(chn);
        if (chn->chnLowCause < CcFlowPauseEnd)
        {
            chn->chnStateMed = ChnStaLowNmlEnd;
            seriesInd->endingCell = seriesInd->runCell;
        }
        else if (chn->chnLowCause > CcFlowJumpEnd)  /**/
        {
            chn->chnStateMed = ChnStaLowProtEnd;
        }
    }
    else
    {
        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            if (0!=cell->cellLowCause && BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
            {
                if (cell->cellLowCause < CcFlowPauseEnd)
                {
                    BitSet(seriesInd->endingCell, cell->cellIdxInChn);
                }
                else if (cell->cellLowCause > CcFlowJumpEnd)
                {
                    BitClr(seriesInd->runCell, cell->cellIdxInChn);
                    BitClr(seriesInd->endingCell, cell->cellIdxInChn);
                    if (ChnProtPause == chn->flowProtEntry->flowProtCfg->protPolicy)
                    {
                        BitSet(seriesInd->pausedCell, cell->cellIdxInChn);
                    }
                    else
                    {
                        BitSet(seriesInd->stopedCell, cell->cellIdxInChn);
                    }
                }
            }
        }

        if (seriesInd->runCell == seriesInd->endingCell)
        {
            chn->chnStateMed = ChnStaLowNmlEnd;
            chnEndCapCalc(chn);
        }
    }

    return;
}

/*已下发启动*/
void chnStaMapStart(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun==lowSmpl->chnLowState && chn->chnStepId==lowSmpl->stepId
        && chn->chnStepType == lowSmpl->stepType>>3)
    {
        chn->chnStateMed = ChnStaRun;
        chnEnterRun(chn, lowSmpl->timeStamp);
        if (NULL == chn->bypsSeriesInd)
        {
            if (0 != chn->chnLowCause)
            {
                chnEndCapCalc(chn);
                if (chn->chnLowCause < CcFlowPauseEnd) /*上来就截止*/
                {
                    chn->chnStateMed = ChnStaLowNmlEnd;
                }
                else if (chn->chnLowCause > CcFlowJumpEnd)  /**/
                {
                    chn->chnStateMed = ChnStaLowProtEnd;
                }
            }
        }
        else
        {
            chnStaMapRunWiSw(chn);
        }
    }
    else
    {
        chn->dynStaCnt++;
        if (chn->dynStaCnt > LowStartExprCnt)
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
    if (LowChnStateRun!=lowSmpl->chnLowState || chn->chnStepId!=lowSmpl->stepId)
    {
        setChnAbnml(chn, Cc1LowAbnmlEnd);
        return;
    }

    chnRunTimeUpd(chn, lowSmpl->timeStamp);
    chn->capStep = chn->capCtnu + chn->capLow;
    if (NULL == chn->bypsSeriesInd)
    {
        if (0 != chn->chnLowCause)
        {
            chnEndCapCalc(chn);
            if (chn->chnLowCause < CcFlowPauseEnd)
            {
                chn->chnStateMed = ChnStaLowNmlEnd;
            }
            else if (chn->chnLowCause > CcFlowJumpEnd)  /**/
            {
                chn->chnStateMed = ChnStaLowProtEnd;
            }
        }
    }
    else
    {
        chnStaMapRunWiSw(chn);
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
        if (ChnStaUpStopStartReq == chn->chnStateMed)
        {
            chnEnterRun(chn, lowSmpl->timeStamp);
            chn->chnStateMed = ChnStaUpStopReq;
        }
        else
        {
            chnRunTimeUpd(chn, lowSmpl->timeStamp);
            chn->capStep = chn->capCtnu + chn->capLow;
        }

        if (0 != lowSmpl->cause)
        {
            chnEndCapCalc(chn);
            chn->chnStateMed = ChnStaUpStopEnd;
        }
        else
        {
            chn->dynStaCnt++;
            if (chn->dynStaCnt > LowStopExprCnt)
            {
                setChnAbnml(chn, Cc1LowEndExpr);
            }
        }
    }
    else
    {
        if (ChnStaUpStopReq == chn->chnStateMed)
        {
            setChnAbnml(chn, Cc1LowAbnmlEnd);
        }
        else if (ChnStaUpStopStartReq == chn->chnStateMed)
        {
            chn->dynStaCnt++;
            if (chn->dynStaCnt > LowStopExprCnt)
            {
                setChnAbnml(chn, Cc1LowEndExpr);
            }
        }
        else  /*ChnStaUpStopNpReq*/
        {
            chn->chnLowCause = CcFlowStopEnd;
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
        if (ChnStaUpPauseStartReq == chn->chnStateMed)
        {
            chnEnterRun(chn, lowSmpl->timeStamp);
            chn->chnStateMed = ChnStaUpPauseReq;
        }
        else
        {
            chnRunTimeUpd(chn, lowSmpl->timeStamp);
            chn->capStep = chn->capCtnu + chn->capLow;
        }

        if (0 != lowSmpl->cause)
        {
            chnEndCapCalc(chn);
            if (CcFlowStopEnd == lowSmpl->cause)
            {
                chn->chnLowCause = CcFlowPauseEnd;
            }
            chn->chnStateMed = ChnStaUpPauseEnd;
        }
        else
        {
            chn->dynStaCnt++;
            if (chn->dynStaCnt > LowStopExprCnt)
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
            if (chn->dynStaCnt > LowStopExprCnt)
            {
                setChnAbnml(chn, Cc1LowEndExpr);
            }
        }
        else  /*ChnStaUpPauseNpReq*/
        {
            chn->chnLowCause = CcFlowPauseEnd;
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
        chn->chnStepType = StepTypeNull;
        chn->chnStepId = StepIdNull;
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
        chn->chnStepType = StepTypeNull;
        chn->chnStepId = StepIdNull;
    }
    else
    {
        chn->chnStateMed = ChnStaPause;
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }
    return;
}

/*工步不走下位机而由中位机直接截止会走这里,目前只有定容*/
void chnStaMapMedEnd(Channel *chn, void *lowData)
{
    if (0 != chn->chnLowCause)
    {
        chn->chnStateMed = ChnStaLowProtEnd;
    }
    else
    {
        chn->chnLowCause = CcLowTimeEnd;
    }
    return;
}

/*下位机保护,且截止数据完毕,只等下位机空闲*/
void chnStaMapLowProtEnd(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;
    u8eChnProtPolicy policy;

    lowSmpl = (ChnSmplParall *)lowData;
    policy = NULL==chn->flowProtEntry ? ChnProtPause : chn->flowProtEntry->flowProtCfg->protPolicy;
    if (LowChnStateRun==lowSmpl->chnLowState || LowChnStateStart==lowSmpl->chnLowState)
    {
        chn->chnStepType = StepTypeNull;
        chn->chnStepId = StepIdNull;
        chn->chnStateMed = ChnProtPause==policy ? ChnStaMedPauseWait : ChnStaMedIdleWait;
        chn->dynStaCnt++;
    }
    else
    {
        chn->chnStateMed = ChnProtPause==policy ? ChnStaPause : ChnStaIdle;
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
        if (chn->dynStaCnt > LowStopExprCnt)
        {
            setChnAbnml(chn, Cc1LowEndExpr);
            boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
        }
    }
    else
    {
        chn->chnStateMed = ChnStaIdle;
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }

    if (lowSmpl->cause <= CcFlowJumpEnd)
    {
        lowSmpl->cause = 0;
    }
    return;
}

void chnStaMapPauseWait(Channel *chn, void *lowData)
{
    ChnSmplParall *lowSmpl;

    lowSmpl = (ChnSmplParall *)lowData;
    if (LowChnStateRun==lowSmpl->chnLowState || LowChnStateStart==lowSmpl->chnLowState)
    {
        chn->dynStaCnt++;
        if (chn->dynStaCnt > LowStopExprCnt)
        {
            setChnAbnml(chn, Cc1LowEndExpr);
            boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
        }
    }
    else
    {
        chn->chnStateMed = ChnStaPause;
        trayNpChnFree(chn->box->tray, chn);
        chnEnterIdle(chn);
    }

    if (lowSmpl->cause <= CcFlowJumpEnd)
    {
        lowSmpl->cause = 0;
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
    chnProtWork(chn);
    if (ChnStaStart==chn->chnStateMed && NULL!=chn->bypsSeriesInd)
    {
        /*旁路串联电芯直接截止的情况,主通道启动态,电芯正常截止+切出态*/
        /*上传采样的保存已经在chnSaveTraySmpl中做,这里检查全部直接截止*/
        /*若在chnSaveTraySmpl一并做,则修改主通道运行态会涉及保护,改的更多*/
        if (chn->bypsSeriesInd->runCell == chn->bypsSeriesInd->endingCell)
        {
            chn->chnStateMed = ChnStaLowNmlEnd;
            chnEndCapCalc(chn);
        }
    }

    if (ChnStaLowNmlEnd==chn->chnStateMed || ChnStaMedEnd==chn->chnStateMed)
    {
        chnStepLowNmlEnd(chn);
    }
    return;
}

void chnInit()
{
    ChnMgr *mgr;
    u16 idx;

    gChnMgr = mgr = sysMemAlloc(sizeof(ChnMgr));
    for (idx=0; idx<TimerIdCri; idx++)
    {
        mgr->chnStaMap[idx] = (ChnStaMap)chnProcIgnore;
    }
    mgr->chnStaMap[ChnStaIdle] = chnStaMapIdle;
    mgr->chnStaMap[ChnStaPause] = chnStaMapIdle;
    mgr->chnStaMap[ChnStaNpWait] = chnStaMapNpWait;
    mgr->chnStaMap[ChnStaStart] = chnStaMapStart;
    mgr->chnStaMap[ChnStaRun] = chnStaMapRun;
    mgr->chnStaMap[ChnStaUpStopReq] = chnStaMapUpStopReq;
    mgr->chnStaMap[ChnStaUpPauseReq] = chnStaMapUpPauseReq;
    mgr->chnStaMap[ChnStaUpStopEnd] = chnStaMapUpStopEnd;
    mgr->chnStaMap[ChnStaUpPauseEnd] = chnStaMapUpPauseEnd;
    mgr->chnStaMap[ChnStaMedEnd] = chnStaMapMedEnd;
    mgr->chnStaMap[ChnStaUpStopStartReq] = chnStaMapUpStopReq;
    mgr->chnStaMap[ChnStaUpPauseStartReq] = chnStaMapUpPauseReq;
    mgr->chnStaMap[ChnStaUpStopNpReq] = chnStaMapUpStopReq;
    mgr->chnStaMap[ChnStaUpPauseNpReq] = chnStaMapUpPauseReq;
    mgr->chnStaMap[ChnStaLowProtEnd] = chnStaMapLowProtEnd;
    mgr->chnStaMap[ChnStaMedPauseWait] = chnStaMapPauseWait;
    mgr->chnStaMap[ChnStaMedIdleWait] = chnStaMapIdleWait;

    return;
}

#endif


