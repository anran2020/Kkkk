

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic.h"
#if TRAY_ENABLE
#include "define.h"
#include "enum.h"
#include "type.h"
#include "timer.h"
#include "func.h"
#include "log.h"
#include "cause.h"
#include "flow.h"
#include "entry.h"
#include "protect.h"
#include "box.h"
#include "host.h"
#include "channel.h"
#include "tray.h"

#ifdef DebugVersion
#else
#include "ctp.h"
#include "mlos_log.h"
#include "ds.h"
#endif

u8 *gBoxMsgBufTx;

CanMgr *gCanMgr;

void boxMsgIgnore()
{
    return;
}

void traySmplDefChnSmpl(Tray *tray, Channel *chn, u8eUpChnState state, u16eCauseCode cause)
{
    TraySmpl *traySmpl;
    TrayChnSmpl *trayChnSmpl;
    TrayChnSmpl *smplCri;

    traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
    trayChnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
    trayChnSmpl->chnType = ChnTypeMainChn;
    trayChnSmpl->chnUpState = state;
    trayChnSmpl->stepId = StepIdNull;
    trayChnSmpl->stepType = StepTypeNull;
    trayChnSmpl->stepSubType = StepSubTypeNull;
    trayChnSmpl->inLoop = True;
    trayChnSmpl->causeCode = cause;
    trayChnSmpl->stepRunTime = 0;
    trayChnSmpl->volCell = 0;
    trayChnSmpl->volCur = 0;
    trayChnSmpl->volPort = 0;
    trayChnSmpl->volInner = 0;
    trayChnSmpl->current = 0;
    trayChnSmpl->capacity = 0;
    if (0 == chn->chnCellAmt)
    {
        return;
    }

    trayChnSmpl++;
    for (smplCri=trayChnSmpl+chn->chnCellAmt; trayChnSmpl<smplCri; trayChnSmpl++)
    {
        trayChnSmpl->chnType = ChnTypeSeriesCell;
        trayChnSmpl->chnUpState = state;
        trayChnSmpl->stepId = StepIdNull;
        trayChnSmpl->stepType = StepTypeNull;
        trayChnSmpl->stepSubType = StepSubTypeNull;
        trayChnSmpl->inLoop = False;
        trayChnSmpl->causeCode = cause;
        trayChnSmpl->stepRunTime = 0;
        trayChnSmpl->volCell = 0;
        trayChnSmpl->volCur = 0;
        trayChnSmpl->volPort = 0;
        trayChnSmpl->volInner = 0;
        trayChnSmpl->current = 0;
        trayChnSmpl->capacity = 0;
    }

    return;
}

void trayUpdSmplSeq(SmplSaveMgr *smplMgr)
{
    if (smplMgr->smplSeqNext == smplMgr->smplSeqMax)
    {
        smplMgr->smplSeqNext = 0;
        smplMgr->smplDiskAddr = smplMgr->smplDiskAddrBase;
        smplMgr->isLooped = True;
    }
    else
    {
        smplMgr->smplSeqNext++;
        smplMgr->smplDiskAddr += smplMgr->smplItemDiskSize;
    }

    return;
}

void trayUpdSmplBuf(SmplSaveMgr *smplMgr)
{
    if (smplMgr->smplBufAddr == smplMgr->smplBufAddrMax)
    {
        smplMgr->smplBufAddr = smplMgr->smplBufAddrBase;
    }
    else
    {
        smplMgr->smplBufAddr += smplMgr->smplItemSize;
    }
    return;
}

void traySmplMgrRst(Tray *tray)
{
    Channel *chn;
    Channel *chnCri;
    Box *box;
    Box *boxCri;

    tray->smplMgr.auxDataDone = False;
    tray->smplMgr.smplChnAmt = 0;
    for (chn=tray->chn,chnCri=tray->chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        chn->smplPres = False;
    }

    tray->smplMgr.smplTryBoxAmt = 0;
    for (box=tray->box,boxCri=tray->box+tray->boxAmt; box<boxCri; box++)
    {
        box->boxHasSmplTry = False;
    }

    return;
}

/*??????????????????????????????????????????????????????,????????????*/
/*??????????????????????????????????????????,???????????????????????????????????????,?????????????????????*/
/*????????????????????????????????????,????????????????????????????????????????????????*/
/*??????????????????????????????????????????????????????,???????????????*/
/*???????????????????????????????????????,??????????????????*/
void trayRcdChnLastData(Channel *chn)
{
    ChnProtBuf *protBuf;

    chn->chnLowCause = CcNone;
    protBuf = &chn->chnProtBuf;
    if (protBuf->newPowerBeValid || !gDevMgr->can[chn->box->canIdx].smplAgainAct)
    {
        protBuf->cellTmprPre.tmprBeValid = protBuf->cellTmprCrnt.tmprBeValid;
        protBuf->cellTmprPre.tmprVal = protBuf->cellTmprCrnt.tmprVal;
        protBuf->cellTmprCrnt.tmprBeValid = False;
        protBuf->prePowerBeValid = protBuf->newPowerBeValid;
        protBuf->preCur = protBuf->newCur;
        protBuf->preVolCell = protBuf->newVolCell;
    }
    protBuf->newPowerBeValid = False;
    protBuf->newCauseCode = CcNone;
    if (NULL != chn->cell)
    {
        Cell *cell;
        Cell *cellCri;

        for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            protBuf = &cell->chnProtBuf;
            if (protBuf->newPowerBeValid || !gDevMgr->can[chn->box->canIdx].smplAgainAct)
            {
                protBuf->cellTmprPre.tmprBeValid = protBuf->cellTmprCrnt.tmprBeValid;
                protBuf->cellTmprPre.tmprVal = protBuf->cellTmprCrnt.tmprVal;
                protBuf->cellTmprCrnt.tmprBeValid = False;
                protBuf->prePowerBeValid = protBuf->newPowerBeValid;
                protBuf->preCur = protBuf->newCur;
                protBuf->preVolCell = protBuf->newVolCell;
            }
            protBuf->newPowerBeValid = False;
            protBuf->newCauseCode = CcNone;
        }
    }

    return;
}

/*????????????????????????????????????*/
void trayProtMgrEnd(Tray *tray)
{
    TrayProtMgr *trayProtMgr;
    ChnProtBuf *protBuf;
    Channel *chn;
    Channel *chnCri;

    trayProtMgr = &tray->trayProtMgr;
    trayProtMgr->allChnTmprUpLmtPointAmt = 0;
    trayProtMgr->trayCauseNew = CcNone;

    trayProtMgr->slotTmprPre.tmprBeValid = trayProtMgr->slotTmprCrnt.tmprBeValid;
    trayProtMgr->slotTmprPre.tmprVal = trayProtMgr->slotTmprCrnt.tmprVal;
    trayProtMgr->preNdbdValid = trayProtMgr->newNdbdValid;
    trayProtMgr->preNdbdNp = trayProtMgr->newNdbdNp;

    for (chn=tray->chn,chnCri=tray->chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        trayRcdChnLastData(chn);
    }

    return;
}

/*????????????,???????????????1???1???????????????,?????????????????????1???1???*/
/*????????????,???????????????,???????????????????????????4????????????????????????*/
/*??????1?????????1???????????????????????????????????????1????????????????????????1*/
/*??????????????????,???????????????????????????????????????????????????????????????*/
/*?????????????????????????????????????????????????????????????????????????????????*/
/*??????????????????,???????????????????????????,??????????????????.*/
/*?????????????????????,???????????????????????????,?????????????????????,?????????*/
/*??????(???)??????,??????box?????????2???,??????????????????2???????????????*/
void trayGenTraySmpl(Tray *tray)
{
    SmplSaveMgr *smplMgr;
    TraySmplRcd *traySmplRcd;
    TraySmpl *traySmpl;
    TrayProtMgr *trayProtMgr;
    Channel *chn;
    Channel *chnCri;
    Box *box;
    Box *boxCri;
    b8 mayStop;

    traySmplSetAux(tray);
    smplMgr = &tray->smplMgr;
    trayProtMgr = &tray->trayProtMgr;
    traySmplRcd = (TraySmplRcd *)smplMgr->smplBufAddr;
    traySmplRcd->timeStampSec = gAbsTimeSec + sysTimeSecGet();
    traySmpl = traySmplRcd->traySmpl;
    traySmpl->smplType = TraySmplChn;
    traySmpl->smplSize = sizeof(TrayChnSmpl);
    traySmpl->smplAmt = tray->genChnAmt;
    for (mayStop=False,chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        if (CcNone != trayProtMgr->trayCauseNew)
        {
            if (CcNone == chn->chnProtBuf.newCauseCode)
            {
                CcChkModify(chn->chnProtBuf.newCauseCode, trayProtMgr->trayCauseNew);
                chnStopByProt(chn, True);
                mayStop = True;
            }
        }
        else if (!chn->smplPres) /*????????????,????????????*/
        {
            traySmplDefChnSmpl(tray, chn, ChnUpStateTemp, CcNone);
        }
    }
    if (mayStop)
    {
        for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
        {
            if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
            {
                boxCtrlTxTry(box);
            }
        }
    }

    if (Cc4TrayMixTrag == trayProtMgr->trayCauseNew)
    {
        mixProtPolicyAct(&trayProtMgr->protPolicyTmr);
    }

    if (smplMgr->smplEnable)
    {
        u32 smplSeqDisk;
        u32 upSureSeq; /*??????????????????????????????,????????????????????????*/

        smplSeqDisk = smplMgr->smplSeqNext;  /*??????*/
        trayUpdSmplSeq(smplMgr);
        upSureSeq = 0==smplMgr->smplSeqUpReq ? smplMgr->smplSeqMax : smplMgr->smplSeqUpReq-1;
        if (upSureSeq==smplMgr->smplSeqNext&&smplMgr->isLooped || smplMgr->upDiscExpr)
        {
            trayStopByDisk(tray, Cc3UpDiscExpr);
            smplMgr->smplEnable = False;
        }

    #ifdef DebugVersion
    #else
    #if 1
        smplDiskWrite(tray->trayIdx, 1, smplMgr->smplBufAddr, smplSeqDisk);
    #else
        ds_write_file(tray->trayIdx, 1, smplMgr->smplBufAddr, smplSeqDisk, smplMgr->smplSeqUpReq);
    #endif
    #endif
        trayUpdSmplBuf(smplMgr);
    }

    return;
}

void canTxMsg(u8 canId, u8 addr, u8 *data, u16 len)
{
#ifdef DebugVersion
#else
    ctp_send(canId, addr, data, len);
#endif
}

void *allocAuxCanBuf()
{
    CanMgr *mgr;
    CanAuxCmdBuf *auxBuf;
    ChainD *chain;

    mgr = gCanMgr;
    if (ListIsEmpty(&mgr->auxBufList))
    {
        return NULL;
    }

    chain = mgr->auxBufList.next;
    ChainDelSafeD(chain);
    auxBuf = Container(CanAuxCmdBuf, chain, chain);
    return auxBuf;
}

void freeAuxCanBuf(void *buf)
{
    ListD *list;
    ChainD *chain;

    list = &gCanMgr->auxBufList;
    chain = &((CanAuxCmdBuf *)buf)->chain;
    ChainDeleteD(chain);  /*???????????????*/
    ChainInsertD(list, chain);  /*???????????????*/
    return;
}

void setCaliAuxCanBuf(CanAuxCmdBuf *auxBuf, Box *box, u16 upMsgId)
{
    auxBuf->box = box;
    auxBuf->reTxCnt = 0;
    auxBuf->upMsgFlowSeq = gUpItfCb->upMsgFlowSeq;
    auxBuf->upCanMsgId = upMsgId;
    return;
}

void boxAuxTx(Can *can, Box *box, CanAuxCmdBuf *auxBuf)
{
    BoxMsgHead *head;

    head = (BoxMsgHead *)auxBuf->msgBuf;
    head->msgFlowSeq = can->canMsgFlowSeq;
    can->waitAckAuxCmd = auxBuf;
    can->msgIdHasTx = head->msgId;
    canTxMsg(can->canIdx, box->addr, auxBuf->msgBuf, head->msgLen);
    timerStart(&can->waitCanAckTmr, TidCanRxAuxAck, 400, WiReset);
}

void boxAuxTxTry(u8 canIdx, void *buf)
{
    Can *can;
    Box *box;
    CanAuxCmdBuf *auxBuf;

    can = &gDevMgr->can[canIdx];
    auxBuf = (CanAuxCmdBuf *)buf;
    if (TimerIsRun(&can->waitCanAckTmr))
    {
        if (!ChainInList(&auxBuf->chain))
        {
            ChainInsertD(&can->auxWaitList, &auxBuf->chain);
        }
        return;
    }

    boxAuxTx(can, auxBuf->box, auxBuf);
    return;
}

/*??????????????????????????????,??????????????????????????????*/
/*?????????????????????????????????????????????????????????,?????????????????????*/
/*??????????????????????????????,??????????????????*/
/*???????????????????????????????????????, can-box-idx*/
void canSmplBoxUpdate(Can *can, u8 beginIdx)
{
    u8 idx;
    Box *box;

    for (idx=beginIdx; idx<can->boxAmt; idx++)
    {
        box = &gDevMgr->box[can->can2DevBoxIdx[idx]];
    #if 0 /*??????????????????????????????,?????????????????????????????????,??????????????????*/
        if (!(box->tray->trayWorkMode & BoxModeMntnCali))
    #endif
        {
            if (!can->smplAgainAct)
            {
                break;
            }
            else  /*?????????,????????????????????????????????????box*/
            {
                if (box->moreSmplPres)
                {
                    break;
                }
                else  /*????????????????????????,?????????????????????????????????*/
                {
                    box->tray->smplMgr.smplTryBoxAmt++;
                }
            }
        }
    }

    can->smplCanBoxIdx = idx;
    return;
}
Box *canSmplBoxGet(Can *can)
{
    if (can->smplCanBoxIdx < can->boxAmt)
    {
        return &gDevMgr->box[can->can2DevBoxIdx[can->smplCanBoxIdx]];
    }

    return NULL;
}

/*???????????????????????????*/
void boxSmplTx(Can *can, Box *box)
{
    BoxMsgHead *head;
    u8 *buf;

    buf = gBoxMsgBufTx;
    head = (BoxMsgHead *)buf;
    head->msgFlowSeq = can->canMsgFlowSeq;
    head->msgLen = sizeof(BoxMsgHead);
    if (box->online)
    {
        BoxSmplCmd *smpl;

        head->msgId = BoxMsgSmpl;
        smpl = (BoxSmplCmd *)(buf + sizeof(BoxMsgHead));
        smpl->lastErr = box->reTxSmplCmd||box->reTxSmplCnt>0 ? True : False;
        smpl->ndbdState = box->tray->ndbdData.status[NdbdStaTouch];
        smpl->rsvd = 0;
        head->msgLen += sizeof(BoxSmplCmd);
    }
    else
    {
        BoxConnCmd *conn;

        head->msgId = BoxMsgConn;
        conn = (BoxConnCmd *)(buf + sizeof(BoxMsgHead));
        head->msgLen += sizeof(BoxConnCmd);
        conn->lowWorkMode = gUsedMode;
        if (BoxTypeParallel != box->boxType)  /*????????????*/
        {
            ChnCellInd *cellInd;
            Channel *chn;
            Channel *chnCri;
            Cell *cell;
            Cell *cellCri;

            conn->chnAmt = box->boxChnAmt;
            head->msgLen += sizeof(ChnCellInd)*box->boxChnAmt;
            cellInd = conn->chnCellInd;
            for (chn=box->chn,chnCri=chn+box->boxChnAmt; chn<chnCri; chn++,cellInd++)
            {
                cellInd->ttlCellAmt = box->tray->boxCfg.chnCellAmt;
                cellInd->useCellAmt = chn->chnCellAmt;
                cellInd->useCellInd = 0;
                for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
                {
                    BitSet(cellInd->useCellInd, cell->lowCellIdxInChn);
                }
            }
        }
    }

    if (!box->boxHasSmplTry) /*????????????????????????*/
    {
        box->boxHasSmplTry = True;
        box->tray->smplMgr.smplTryBoxAmt++;
    }

    can->msgIdHasTx = head->msgId;
    canTxMsg(can->canIdx, box->addr, buf, head->msgLen);
    timerStart(&can->waitCanAckTmr, TidCanRxSmplAck, 30, WiReset);

    return;
}

void boxSmplTxTry(Timer *timer)
{
    Box *box;
    Can *can;

    can = Container(Can, canSmplLoopTmr, timer);
    canSmplBoxUpdate(can, 0);
    if (TimerIsRun(&can->waitCanAckTmr)) /*can???*/
    {
        return;
    }

    if (NULL != (box=canSmplBoxGet(can)))
    {
        boxSmplTx(can, box);
    }
    else /*can??????????????????box,????????????*/
    {
        timerStart(timer, TidCanSmplLoop, 1000, WoReset);
    }
    return;
}

void boxCtrlMix(Box *box, u16 chnSelectInd, u16 chnCtrlInd)
{
    BoxCtrlInd *ctrlInd;
    u8 bit;
    
    ctrlInd = &box->ctrlWaitSend.boxCtrlInd;
    if (ctrlInd->chnSelectInd & chnSelectInd) /*????????????????????????*/
    {
        for (bit=0; bit<box->boxChnAmt; bit++)
        {
            if (BitIsSet(chnSelectInd, bit))
            {
                if (BitIsSet(chnCtrlInd, bit))
                {
                    BitSet(ctrlInd->chnCtrlInd, bit);
                }
                else
                {
                    BitClr(ctrlInd->chnCtrlInd, bit);
                }
            }
        }
    }
    else /*????????????????????????*/
    {
        ctrlInd->chnCtrlInd |= chnCtrlInd;
    }

    ctrlInd->chnSelectInd |= chnSelectInd;
    return;
}

void boxCtrlAddChn(Box *box, u8 chnIdx, b8 isStart)
{
    BoxCtrlInd *ctrl;

    ctrl = &box->ctrlWaitSend.boxCtrlInd;
    BitSet(ctrl->chnSelectInd, chnIdx);
    if (isStart)
    {
        BitSet(ctrl->chnCtrlInd, chnIdx);
    }
    else
    {
        BitClr(ctrl->chnCtrlInd, chnIdx);
    }
    return;
}

/*todo, ?????????boxCtrlAddChn??????*/
void boxCtrlAddSeriesCell(Box *box, u8 chnIdx, b8 isStart)
{
    BoxCtrlInd *ctrl;

    ctrl = &box->seriesSwWaitSend.boxCtrlInd;
    BitSet(ctrl->chnSelectInd, chnIdx);
    if (isStart)
    {
        BitSet(ctrl->chnCtrlInd, chnIdx);
    }
    else
    {
        BitClr(ctrl->chnCtrlInd, chnIdx);
    }
    return;
}

/*??????????????????????????????????????????????????????,???????????????????????????????????????*/
b8 stepCvFollowCcChk(Channel *chn)
{
    StepObj *step;
    StepObj *stepNxt;
    StepNode *stepNode;

    step = chn->crntStepNode->stepObj;
    stepNode = getNxtStep(chn, chn->crntStepNode, False);
    stepNxt = NULL==stepNode ? NULL : stepNode->stepObj;
    if (StepTypeCCC == step->stepType)
    {
        if (NULL!=stepNxt && StepTypeCVC==stepNxt->stepType
            && step->stepParam[2]==stepNxt->stepParam[1])
        {
            return True;
        }
    }
    else if (StepTypeCCD == step->stepType)
    {
        if (NULL!=stepNxt && StepTypeCVD==stepNxt->stepType
            && step->stepParam[2]==stepNxt->stepParam[1])
        {
            return True;
        }
    }

    return False;
}

void boxCtrlTx(Can *can, Box *box)
{
    u8 *buf;
    BoxMsgHead *head;
    BoxCtrlCmd *ctrlCmd;
    ChnCtrl *chnCtrl;
    Channel *chn;
    Channel *chnCri;
    StepObj *step;
    u16 chnSelectInd;
    u16 chnCtrlInd;
    u8 cellIndByteCnt;
    u8 idx;

    buf = gBoxMsgBufTx;
    head = (BoxMsgHead *)buf;
    head->msgFlowSeq = can->canMsgFlowSeq;
    head->msgId = BoxMsgCtrl;
    head->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCtrlCmd);

    ctrlCmd = (BoxCtrlCmd *)(buf + sizeof(BoxMsgHead));
    ctrlCmd->isReTx = 0==box->reTxCtrlCnt ? False : True;
    ctrlCmd->chnSelectInd = 0;
    ctrlCmd->chnCtrlInd = 0;
    chnSelectInd = box->ctrlWaitSend.boxCtrlInd.chnSelectInd;
    chnCtrlInd = box->ctrlWaitSend.boxCtrlInd.chnCtrlInd;
    chnCtrl = ctrlCmd->chnCtrl;
    if (BoxTypeSeriesWiSw == box->boxType)
    {
        cellIndByteCnt = box->tray->boxCfg.chnCellAmt/8;
        cellIndByteCnt += box->tray->boxCfg.chnCellAmt%8 ? 1 : 0;
        head->msgLen += Align8(cellIndByteCnt);
        chnCtrl = (ChnCtrl *)(ctrlCmd->cellCtrlInd + Align8(cellIndByteCnt));
    }
    for (chn=box->chn,chnCri=chn+box->boxChnAmt; chn<chnCri; chn++)
    {
        if (!BitIsSet(chnSelectInd, chn->chnIdxInBox)) /*?????????*/
        {
            continue;
        }

        chn->dynStaCnt = 0;
        BitSet(ctrlCmd->chnSelectInd, chn->lowChnIdxInBox);
        if (!BitIsSet(chnCtrlInd, chn->chnIdxInBox)) /*???*/
        {
            if (NULL != chn->bypsSeriesInd)
            {
                BypsSeriesInd *seriesInd;

                seriesInd = chn->bypsSeriesInd;
                if (ChnStaRun!=chn->chnStateMed || seriesInd->stopingCell==seriesInd->startCell)
                {
                    seriesInd->stopingCell = 0xffffffff;
                }

                for (idx=0; idx<cellIndByteCnt; idx++)
                {
                    ctrlCmd->cellCtrlInd[idx] = (seriesInd->stopingCell>>idx*8) & 0xff;
                }
            }
            continue;
        }

        BitSet(ctrlCmd->chnCtrlInd, chn->lowChnIdxInBox);
        step = chn->crntStepNode->stepObj;
        if (NULL != chn->bypsSeriesInd)  /*todo,????????????????????????,??????1???1??????*/
        {
            for (idx=0; idx<cellIndByteCnt; idx++)
            {
                ctrlCmd->cellCtrlInd[idx] = (chn->bypsSeriesInd->runCell>>idx*8) & 0xff;
            }
        }

        chnCtrl->stepId = chn->chnStepId;
        chnCtrl->stepType = step->stepType;
        chnCtrl->cvFollowCc = stepCvFollowCcChk(chn);
        if (StepTypeQuiet == step->stepType)
        {
            chnCtrl->paramInd = 0x04;
            chnCtrl->timeStop = step->stepParam[0]-chn->stepRunTimeCtnu;
        }
        else if (StepTypeCCC==step->stepType || StepTypeCCD==step->stepType)
        {
            chnCtrl->paramInd = 0x1d;
            chnCtrl->curSet = step->stepParam[0];
            memcpy(&chnCtrl->timeStop, &step->stepParam[1], sizeof(u32)*3);
            chnCtrl->timeStop = step->stepParam[1]-chn->stepRunTimeCtnu;
            chnCtrl->capStop = step->stepParam[3]-chn->capCtnu;
        }
        else
        {
            chnCtrl->paramInd = step->paramEnable;
            memcpy(&chnCtrl->curSet, step->stepParam, sizeof(u32)*5);
            chnCtrl->timeStop = step->stepParam[2]-chn->stepRunTimeCtnu;
            chnCtrl->capStop = step->stepParam[4]-chn->capCtnu;
            if (!BitIsSet(step->paramEnable, 0))
            {
                chnCtrl->curSet = 0; /*????????????????????????????????????*/
            }
        }

        head->msgLen += sizeof(ChnCtrl);
        chnCtrl++;
    }

    memcpy(&box->ctrlWaitAck, &box->ctrlWaitSend.boxCtrlInd, sizeof(BoxCtrlInd));
    memset(&box->ctrlWaitSend.boxCtrlInd, 0, sizeof(BoxCtrlInd));
    can->msgIdHasTx = head->msgId;
    can->boxWaitCtrlAck = box;
    canTxMsg(can->canIdx, box->addr, buf, head->msgLen);
    timerStart(&can->waitCanAckTmr, TidCanRxCtrlAck, 30, WiReset);
    return;
}

void boxCtrlTxTry(Box *box)
{
    Can *can;

    can = &gDevMgr->can[box->canIdx];
    if (TimerIsRun(&can->waitCanAckTmr))
    {
        if (!ChainInList(&box->ctrlWaitSend.chain))
        {
            ChainInsertD(&can->ctrlWaitList, &box->ctrlWaitSend.chain);
        }
    }
    else
    {
        boxCtrlTx(can, box);
    }

    return;
}

void boxCtrlCellTx(Can *can, Box *box)
{
    u8 *buf;
    BoxMsgHead *head;
    BoxSeriesSwCmd *ctrlCmd;
    Channel *chn;
    Channel *chnCri;
    u16 chnSelectInd;
    u16 chnCtrlInd;
    u8 cellIndByteCnt;
    u8 idx;

    buf = gBoxMsgBufTx;
    head = (BoxMsgHead *)buf;
    head->msgFlowSeq = can->canMsgFlowSeq;
    head->msgId = BoxMsgCellSw;
    head->msgLen = sizeof(BoxMsgHead) + sizeof(BoxSeriesSwCmd);

    ctrlCmd = (BoxSeriesSwCmd *)(buf + sizeof(BoxMsgHead));
    ctrlCmd->chnSelectInd = 0;
    ctrlCmd->chnCtrlInd = 0;
    chnSelectInd = box->seriesSwWaitSend.boxCtrlInd.chnSelectInd;
    chnCtrlInd = box->seriesSwWaitSend.boxCtrlInd.chnCtrlInd;

    cellIndByteCnt = box->tray->boxCfg.chnCellAmt/8;
    cellIndByteCnt += box->tray->boxCfg.chnCellAmt%8 ? 1 : 0;
    head->msgLen += Align8(cellIndByteCnt);

    for (chn=box->chn,chnCri=chn+box->boxChnAmt; chn<chnCri; chn++)
    {
        if (!BitIsSet(chnSelectInd, chn->chnIdxInBox)) /*?????????*/
        {
            continue;
        }

        BitSet(ctrlCmd->chnSelectInd, chn->lowChnIdxInBox);
        if (BitIsSet(chnCtrlInd, chn->chnIdxInBox))  /*??????*/
        {
            BitSet(ctrlCmd->chnCtrlInd, chn->lowChnIdxInBox);
        }

        for (idx=0; idx<cellIndByteCnt; idx++)
        {
            ctrlCmd->cellCtrlInd[idx] = (chn->bypsSeriesInd->idleSwCell>>idx*8) & 0xff;
        }
        chn->bypsSeriesInd->idleSwCell = 0;
    }

    memset(&box->seriesSwWaitSend.boxCtrlInd, 0, sizeof(BoxCtrlInd));
    can->msgIdHasTx = head->msgId;
    canTxMsg(can->canIdx, box->addr, buf, head->msgLen);
    timerStart(&can->waitCanAckTmr, TidCanRxCellSwAck, 100, WiReset);
    return;
}

void boxCtrlCellTxTry(Box *box)
{
    Can *can;

    can = &gDevMgr->can[box->canIdx];
    if (TimerIsRun(&can->waitCanAckTmr))
    {
        BoxCtrlWaitSend *seriesSwWaitSend;

        seriesSwWaitSend = &box->seriesSwWaitSend;
        if (!ChainInList(&seriesSwWaitSend->chain))
        {
            ChainInsertD(&can->ctrlCellWaitList, &seriesSwWaitSend->chain);
        }
    }
    else
    {
        boxCtrlCellTx(can, box);
    }

    return;
}

void canIdleNtfy(Can *can)
{
    Box *box;
    Tray *tray;

    if (!ListIsEmpty(&can->ctrlWaitList))  /*??????,?????????*/
    {
        BoxCtrlWaitSend *ctrl;

        ctrl = Container(BoxCtrlWaitSend, chain, can->ctrlWaitList.next);
        box = Container(Box, ctrlWaitSend, ctrl);
        boxCtrlTx(can, box);
        ChainDelSafeD(&ctrl->chain);
        return;
    }

    if (!ListIsEmpty(&can->ctrlCellWaitList))  /*??????,???????????????*/
    {
        BoxCtrlWaitSend *ctrl;

        ctrl = Container(BoxCtrlWaitSend, chain, can->ctrlCellWaitList.next);
        box = Container(Box, seriesSwWaitSend, ctrl);
        boxCtrlCellTx(can, box);
        ChainDelSafeD(&ctrl->chain);
        return;
    }

    if (!ListIsEmpty(&can->auxWaitList))  /*???????????????*/
    {
        CanAuxCmdBuf *auxBuf;

        auxBuf = Container(CanAuxCmdBuf, chain, can->auxWaitList.next);
        boxAuxTx(can, auxBuf->box, auxBuf);
        return;
    }

    /*????????????????????????,????????????.*/
    if (TimerIsRun(&can->canSmplLoopTmr)) /*?????????????????????*/
    {
        return;
    }

    if (NULL != (box=canSmplBoxGet(can))) /*?????????????????????*/
    {
        boxSmplTx(can, box);
        return;
    }

    /*??????????????????????????????????????????*/
    if (SmplModeTray == gDevMgr->smplMode)  /*???????????????????????????????????????*/
    {
        for (tray=gDevMgr->tray; tray<&gDevMgr->tray[gDevMgr->trayAmt]; tray++)
        {
            if (tray->smplMgr.smplTryBoxAmt >= tray->boxAmt) /*???box????????????*/
            {
                if (0 != tray->smplMgr.smplChnAmt)
                {
                    trayGenTraySmpl(tray);  /*???????????????????????????,?????????????????????*/
                }
                trayProtMgrEnd(tray);
                traySmplMgrRst(tray);
            }
        }

        if (can->smplAgainNeed)  /*?????????????????????????????????????????????,moreSmplPres*/
        {
            can->smplAgainNeed = False;
            can->smplAgainAct = True;
            boxSmplTxTry(&can->canSmplLoopTmr);
            return;
        }
        else
        {
            can->smplAgainAct = False;
        }
    }

    /*????????????????????????????????????????????????*/
    timerStart(&can->canSmplLoopTmr, TidCanSmplLoop, 1000, WoReset);
    return;
}

void _____begin_of_can_ack_expire_____(){}

void boxExprCtrlAck(Timer *timer)
{
    Can *can;
    Box *box;
    BoxCtrlInd boxCtrlInd;

    can = Container(Can, waitCanAckTmr, timer);
    can->canMsgFlowSeq++;
    box = can->boxWaitCtrlAck;
    if (box->reTxCtrlCnt < 2)
    {
        box->reTxCtrlCnt++;
        memcpy(&boxCtrlInd, &box->ctrlWaitSend.boxCtrlInd, sizeof(BoxCtrlInd));
        memcpy(&box->ctrlWaitSend.boxCtrlInd, &box->ctrlWaitAck, sizeof(BoxCtrlInd));
        if (0 != boxCtrlInd.chnSelectInd)
        {
            boxCtrlMix(box, boxCtrlInd.chnSelectInd, boxCtrlInd.chnCtrlInd);
        }

        boxCtrlTxTry(box);
    }
    else  /*todo,????????????????????????,??????????????????*/
    {
        box->reTxCtrlCnt = 0;
        canIdleNtfy(can);
    }
    return;
}

void boxExprCellSwAck(Timer *timer)
{
    Can *can;

    can = Container(Can, waitCanAckTmr, timer);
    can->canMsgFlowSeq++;
    canIdleNtfy(can);
    return;
}

void boxExprSmplAck(Timer *timer)
{
    Can *can;
    Box *box;
    Channel *chn;
    Channel *chnCri;

    can = Container(Can, waitCanAckTmr, timer);
    can->canMsgFlowSeq++;

    box = &gDevMgr->box[can->can2DevBoxIdx[can->smplCanBoxIdx]];
    box->moreSmplPres = False;
    if (box->online)
    {
        if (box->reTxSmplCmd)
        {
            if (box->reTxSmplCnt < 2)
            {
                box->reTxSmplCnt++;
            }
            else
            {
                box->online = False;
                box->reTxSmplCnt = 0;
                sendUpConnNtfy();

                /*??????????????????????????????*/
                for (chn=box->chn,chnCri=box->chn+box->boxChnAmt; chn<chnCri; chn++)
                {
                    if (ChnStaRun==chn->chnStateMed || ChnStaNpWait==chn->chnStateMed
                        || ChnStaStart==chn->chnStateMed || ChnStaMedEnd==chn->chnStateMed)
                    {
                        if (ChnStaRun == chn->chnStateMed) /*???????????????????????????*/
                        {
                            chnEndCapCalc(chn);
                        }

                        chn->chnStateMed = ChnStaRun; /*?????????????????????*/
                        chnSaveTraySmplOld(chn, Cc1BoxOffline);
                    }
                    else if (ChnStaPause == chn->chnStateMed
                        || ChnStaUpPauseReq==chn->chnStateMed || ChnStaUpPauseStartReq==chn->chnStateMed
                        || ChnStaUpPauseNpReq==chn->chnStateMed || ChnStaMedPauseWait==chn->chnStateMed)
                    {
                        chn->chnStateMed = ChnStaPause;
                    }
                    else
                    {
                        chn->chnStateMed = ChnStaIdle;
                    }

                    trayNpChnFree(box->tray, chn);
                    chnEnterIdle(chn);
                }
            }
        }
        else  /*???????????????*/
        {
            box->reTxSmplCmd = True;
            goto canIdle;
        }
    }

    box->reTxSmplCmd = False;
    if (!box->online)
    {
        box->reTxConnCnt = 0;
        for (chn=box->chn,chnCri=box->chn+box->boxChnAmt; chn<chnCri; chn++)
        {
            if (ChnStaRun == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaPause;  /*???????????????*/
            }
            else
            {
                traySmplDefChnSmpl(box->tray, chn, ChnUpStateOffline, CcNone);
                chn->smplPres = True;
                box->tray->smplMgr.smplChnAmt++;
            }
        }
    }

    canSmplBoxUpdate(can, can->smplCanBoxIdx+1);

canIdle:
    canIdleNtfy(can);
    return;
}

void boxExprUpdCnfmAck(CanAuxCmdBuf *auxBuf)
{
    UpUpdCnfmAck *upAck;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpUpdCnfmAck *)(buf + sizeof(UpMsgHead));
    upAck->rspCode = RspRetry;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpUpdCnfmAck), UpCmdIdUpdCnfm);
    return;
}

void boxExprUpdSetupAck(CanAuxCmdBuf *auxBuf)
{
    UpUpdSetupAck *upAck;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpUpdSetupAck *)(buf + sizeof(UpMsgHead));
    upAck->rspCode = RspRetry;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpUpdSetupAck), UpCmdIdUpdSetup);
    return;
}

void boxExprUpdUpldAck(CanAuxCmdBuf *auxBuf)
{
    return;
}

void boxExprCaliNtfyAck(CanAuxCmdBuf *auxBuf)
{
    rspUpCaliNtfyAck(auxBuf->box->tray->trayIdx, auxBuf->box->boxIdxInTray, RspExpr);
    return;
}

void boxExprCaliSmplAck(CanAuxCmdBuf *auxBuf)
{
    BoxCaliSmplCmd *boxCmd;
    UpCaliSmplAck *upAck;
    Box *box;
    Channel *chn;
    u8 *buf;

    boxCmd = (BoxCaliSmplCmd *)(auxBuf->msgBuf+sizeof(BoxMsgHead));
    box = auxBuf->box;
    chn = &box->chn[boxCmd->moduIdx/box->chnModuAmt];
    buf = sendUpBuf;
    upAck = (UpCaliSmplAck *)(buf + sizeof(UpMsgHead));
    upAck->trayIdx = box->tray->trayIdx;
    upAck->smplAmt = 0;
    upAck->rspCode = RspExpr;
    upAck->moduIdx = boxCmd->moduIdx % box->chnModuAmt;
    upAck->chnIdx = chn->genIdxInTray;
    upAck->smplOnlyMainChn = boxCmd->smplAllSeries ? False : True;
    sendUpMsg(buf, sizeof(UpCaliSmplAck), UpCmdIdCaliSmpl);
    return;
}

void boxExprCmmnAuxAck(CanAuxCmdBuf *auxBuf)
{
    rspUpCmmn(auxBuf->upCanMsgId, auxBuf->box->tray->trayIdx, RspExpr);
    return;
}

void boxExprAuxAck(Timer *timer)
{
    Can *can;
    CanAuxCmdBuf *auxBuf;

    can = Container(Can, waitCanAckTmr, timer);
    can->canMsgFlowSeq++;
    auxBuf = can->waitAckAuxCmd;
    if (0 != auxBuf->reTxCnt)
    {
        auxBuf->reTxCnt--;
        if (!ChainInList(&auxBuf->chain))
        {
            ChainInsertD(&can->auxWaitList, &auxBuf->chain);
        }
    }
    else
    {
        BoxMsgHead *head;

        head = (BoxMsgHead *)auxBuf->msgBuf;
        gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
        gCanMgr->boxAuxAckExpr[head->msgId](auxBuf);
        freeAuxCanBuf(auxBuf);
    }

    canIdleNtfy(can);
    return;
}

void _____begin_of_can_ack_recieve_____(){}

void boxRxConnAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxConnAck *connAck;
    TrayBoxCfg *cfg;
    SubBox *subBox;
    SubBoxInfo *subBoxAck;
    u16 idx;
    u16 subOnlineCnt;
    u16eCauseCode causeCode;

    canSmplBoxUpdate(can, can->smplCanBoxIdx+1);
    connAck = (BoxConnAck *)pld;
    box->chnAmtTtl = connAck->chnAmt;
    box->maxCur = connAck->current;
    box->maxVol = connAck->voltage;
    box->volSmplAmt = connAck->volSmplAmt;
    box->bypsSwAmt = connAck->bypsSwAmt;
    box->softVer = connAck->softVer;
    box->chnModuAmt = connAck->chnModuAmt;
    cfg = &box->tray->boxCfg;
    causeCode = CcNone;
    if (connAck->protoVer < gLowProtoVer)
    {
        causeCode = Cc1LowProto;
    }
    else if (connAck->devType-1!=cfg->boxType || connAck->chnAmt!=cfg->chnAmtConn
        || connAck->chnModuAmt!=cfg->chnModuAmt
        || connAck->volSmplAmt!=cfg->volSmplAmt || connAck->bypsSwAmt!=cfg->bypsSwAmt
        || connAck->current!=cfg->maxTrayCur || connAck->voltage!=cfg->maxTrayVol
        || (BoxTypeParallel!=cfg->boxType && connAck->chnCellAmt!=box->boxCellAmt))
    {
        causeCode = Cc1LowCfg;
    }

    if (CcNone != causeCode)
    {
        Channel *chn;
        Channel *chnCri;

        /*???????????????,????????????????????????????????????*/
        for (chn=box->chn,chnCri=box->chn+box->boxChnAmt; chn<chnCri; chn++)
        {
            if (!chn->smplPres)
            {
                traySmplDefChnSmpl(box->tray, chn, ChnUpStateIdle, causeCode);
                chn->smplPres = True;
                box->tray->smplMgr.smplChnAmt++;
            }
        }

        box->tray->trayWarnPres = True;
        return;
    }

    for (idx=0,subBoxAck=connAck->boxSub,subOnlineCnt=0; idx<cfg->volSmplAmt; idx++)
    {
        subBox = &box->tray->volSmpl[box->volSmplAmt*box->boxIdxInTray+idx];
        subBox->softVer = subBoxAck->softVer;
        subBox->online = subBoxAck->online;
        if (subBox->online)
        {
            subOnlineCnt++;
        }
        subBox->addr = subBoxAck->boxAddr;
        subBox->subType = subBoxAck->devType;
        subBoxAck++;
    }
    for (idx=0; idx<cfg->bypsSwAmt; idx++)
    {
        subBox = &box->tray->bypsSw[box->bypsSwAmt*box->boxIdxInTray+idx];
        subBox->softVer = subBoxAck->softVer;
        subBox->online = subBoxAck->online;
        if (subBox->online)
        {
            subOnlineCnt++;
        }
        subBox->addr = subBoxAck->boxAddr;
        subBox->subType = subBoxAck->devType;
        subBoxAck++;
    }

    /*????????????????????????????????????????????????*/
    if (subOnlineCnt < cfg->volSmplAmt+cfg->bypsSwAmt)
    {
        box->reTxConnCnt++;
        if (box->reTxConnCnt < 3)
        {
            return;
        }
    }

    box->reTxConnCnt = 0;
    box->online = True;
    box->boxWorkMode = BoxModeManu;
    sendUpConnNtfy();
    return;
}
u16 getLowSmplSize(Channel *chn)
{
    if (BoxTypeParallel == chn->box->boxType)
    {
        return sizeof(ChnSmplParall);
    }
    else if (BoxTypeSeriesWoSw == chn->box->boxType)
    {
        return sizeof(ChnSmplSeries)+chn->chnCellAmt*sizeof(CellSmplSeriesWoSw);
    }
    else
    {
        return sizeof(ChnSmplSeries)+chn->chnCellAmt*sizeof(CellSmplSeriesWiSw);
    }
}
void boxRxSmplChn(Box *box, u16 chnIndBitMap, u8 *smpl)
{
    return;
}

/*??????????????????????????????????????????*/
/*???????????????????????????????????????????????????,???????????????????????????????????????*/
/*??????,??????????????????????????????????????????,??????????????????*/
/*???????????????????????????????????????,?????????????????????????????????????????????*/
/*???????????????????????????????????????,?????????????????????????????????????????????*/
void boxRxSmplTray(Box *box, u16 chnIndBitMap, u8 *smpl)
{
    Tray *tray;
    Channel *chn;
    Channel *chnCri;
    TraySmpl *traySmpl;
    u16 lowSmplSize;
    u8 lowChnIdx;

    tray = box->tray;
    traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
    lowSmplSize = getLowSmplSize(box->chn);
    traySmplSetAux(tray);
    for (lowChnIdx=0,chn=box->chn,chnCri=chn+box->boxChnAmt; chn<chnCri; chn++)
    {
        if (BitIsSet(chnIndBitMap, chn->lowChnIdxInBox)) /*???????????????*/
        {
            for (; lowChnIdx<chn->lowChnIdxInBox; lowChnIdx++)
            {
                if (BitIsSet(chnIndBitMap, lowChnIdx))
                {
                    smpl += lowSmplSize;
                }
            }
            chnLowSmplProc(chn, &traySmpl->chnSmpl[chn->genIdxInTray], smpl);
            smpl += lowSmplSize;
            lowChnIdx++;
        }
    #if 0
        else  /*??????????????????????????????????????????*/
        {
            if (chn->chnStateMed < ChnStaNpWait)
            {
                continue;
            }

            chn->noDataCnt++;
            if (chn->noDataCnt < 8)  /*???????????????????????????*/
            {
                continue;
            }

            chn->noDataCnt = 0;
            if (ChnStaRun == chn->chnStateMed) /*???????????????????????????*/
            {
                chnEndCapCalc(chn);
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaNpWait == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaRun; /*????????????????????????*/
            }
            else if (ChnStaStart == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaRun; /*????????????????????????*/
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }

            if (ChnStaRun == chn->chnStateMed) /*????????????????????????*/
            {
                chnSaveTraySmplOld(chn, Cc1ChnNoData);
                chn->chnStateMed = ChnStaPause;  /*?????????/??????/???????????????????????????*/
            }
            else if (ChnStaUpPauseReq==chn->chnStateMed || ChnStaUpPauseStartReq==chn->chnStateMed
                || ChnStaUpPauseNpReq==chn->chnStateMed || ChnStaMedPauseWait==chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaPause;
            }
            else
            {
                chn->chnStateMed = ChnStaIdle;
            }

            trayNpChnFree(box->tray, chn);
            chnEnterIdle(chn);
        }
    #endif
    }

    if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
    {
        boxCtrlTxTry(box);
    }
    return;
}

/*??????????????????,???????????????*/
/*???????????????????????????,??????????????????????????????????????????*/
void boxRxSmplAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxSmplAck *ack;

    ack = (BoxSmplAck *)pld;
    box->reTxSmplCmd = False;
    box->reTxSmplCnt = 0;
    gCanMgr->boxSmplProc[gDevMgr->smplMode](box, ack->chnSelectInd, (u8 *)ack->chnSmplParall);
    if (box->moreSmplPres)  /*????????????????????????*/
    {
        box->moreSmplPres = False;
    }
    else
    {
        if (ack->moreSmpl)
        {
            box->moreSmplPres = True;
            can->smplAgainNeed = True;
        }
    }

    canSmplBoxUpdate(can, can->smplCanBoxIdx+1);
    return;
}

/*todo,????????????,????????????????????????????????????*/
void boxRxCtrlAck(Can *can, Box *box, u8 *pld, u16 len)
{
    box->reTxCtrlCnt = 0;
    return;
}

void boxRxBoxChkAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxChkAck *boxAck;
    UpBoxChkAck *upAck;
    CanAuxCmdBuf *auxBuf;
    u8 *buf;
    u16 upPldLen;

    buf = sendUpBuf;
    upAck = (UpBoxChkAck *)(buf + sizeof(UpMsgHead));
    boxAck = (BoxChkAck *)pld;
    auxBuf = can->waitAckAuxCmd;
    upAck->rspCode = boxAck->rspCode;
    upAck->trayIdx = box->tray->trayIdx;
    upAck->boxIdx = box->boxIdxInTray;
    if (RspOk != upAck->rspCode)
    {
        upPldLen = sizeof(UpCmmnAck);
    }
    else
    {
        upPldLen = sizeof(UpBoxChkAck) + sizeof(BoxChkResult)*boxAck->chnAmt;
        upAck->chkStage = boxAck->chkStage;
        upAck->beTouch = boxAck->beTouch;
        upAck->chnAmt = boxAck->chnAmt;
        mem2Copy((u16 *)upAck->result, (u16 *)boxAck->result, sizeof(BoxChkResult)*boxAck->chnAmt);
    }
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    sendUpMsg(buf, upPldLen, UpCmdIdManuBoxChk);
    freeAuxCanBuf(auxBuf);
    return;
}

void boxRxCfgReadAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxCfgReadAck *boxAck;
    BoxCfgInd *cfgInd;
    UpCfgReadAck *upAck;
    CanAuxCmdBuf *auxBuf;
    UpCfgTlv *tlv;
    u8 *buf;
    u32 *param;
    u16 upPldLen;
    u8eBoxCfgType cfgType;

    buf = sendUpBuf;
    upAck = (UpCfgReadAck *)(buf + sizeof(UpMsgHead));
    boxAck = (BoxCfgReadAck *)pld;
    cfgInd = &boxAck->cfgInd;
    auxBuf = can->waitAckAuxCmd;
    upAck->rspCode = boxAck->rspCode;
    upAck->rsvd = 0;
    upPldLen = sizeof(UpCfgReadAck);
    for (cfgType=0,tlv=upAck->cfgTlv,param=cfgInd->param; cfgType<BoxCfgCri; cfgType++)
    {
        if (!BitIsSet(cfgInd->funcSelect, cfgType))
        {
            continue;
        }

        tlv->cfgType = BoxCfgBase + cfgType;
        tlv->cfgLen = BoxCfgVolRise==cfgType||BoxCfgCurDown==cfgType ? 12 : 8;
        tlv->cfgVal[0] = BitIsSet(cfgInd->funcEnable, cfgType) ? 1 : 0;
        if (tlv->cfgVal[0])
        {
            tlv->cfgVal[1] = *param++;
            if (12 == tlv->cfgLen)
            {
                tlv->cfgVal[2] = tlv->cfgVal[1] >> 16;
                tlv->cfgVal[1] = (tlv->cfgVal[1] & 0xffff) * 1000;
            }
        }
        tlv = (UpCfgTlv *)((u8 *)tlv + sizeof(UpCfgTlv) + tlv->cfgLen);
        upPldLen += sizeof(UpCfgTlv) + tlv->cfgLen;
    }

    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    sendUpMsg(buf, upPldLen, UpCmdIdUpdCfgRead);
    freeAuxCanBuf(auxBuf);
    return;
}

void boxRxUpdCnfmAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxUpdCnfmAck *boxAck;
    UpUpdCnfmAck *upAck;
    CanAuxCmdBuf *auxBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpUpdCnfmAck *)(buf + sizeof(UpMsgHead));
    boxAck = (BoxUpdCnfmAck *)pld;
    auxBuf = can->waitAckAuxCmd;
    upAck->rspCode = boxAck->rspCode;
    if (RspOk == boxAck->rspCode)
    {
        box->boxWorkMode = BoxModeManu;
        box->online = False;
    }
    upAck->updateVer = boxAck->cnfmVer;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpUpdCnfmAck), UpCmdIdUpdCnfm);
    freeAuxCanBuf(auxBuf);
    return;
}

void boxRxUpdSetupAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxUpdSetupAck *boxAck;
    UpUpdSetupAck *upAck;
    CanAuxCmdBuf *auxBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpUpdSetupAck *)(buf + sizeof(UpMsgHead));
    boxAck = (BoxUpdSetupAck *)pld;
    auxBuf = can->waitAckAuxCmd;
    upAck->rspCode = boxAck->rspCode;
    upAck->pageSize = boxAck->pageSize;
    if (RspOk==boxAck->rspCode && boxAck->pageSize>MaxUpdTransSize)
    {
        upAck->rspCode = RspPage; /*todo, ?????????????????????*/
    }
    upAck->familyVer = boxAck->bootVer;
    gUpItfCb->updMgr.pageSize = boxAck->pageSize;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpUpdSetupAck), UpCmdIdUpdSetup);
    freeAuxCanBuf(auxBuf);
    return;
}

void boxRxUpdUpldAck(Can *can, Box *box, u8 *pld, u16 len)
{
    return;
}

/*??????????????????????????????,?????????????????????,???????????????????????????*/
/*????????????????????????????????????,??????????????????,??????????????????*/
void boxRxCaliNtfyAck(Can *can, Box *box, u8 *pld, u16 len)
{
    CanAuxCmdBuf *auxBuf;
    BoxCaliNtfyAck *boxAck;

    auxBuf = can->waitAckAuxCmd;
    boxAck = (BoxCaliNtfyAck *)pld;
    if (box->tray->trayWorkMode & BoxModeMntnCali)
    {
        gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
        rspUpCaliNtfyAck(box->tray->trayIdx, box->boxIdxInTray, boxAck->rspCode);
    }
    freeAuxCanBuf(auxBuf);

    if (boxAck->rspCode==RspOk && !boxAck->caliEnable)
    {
        box->boxWorkMode = BoxModeManu;
        //box->online = False;
        if (box->tray->trayWorkMode & BoxModeMntnCali)
        {
            trayMntnEnd(box->tray, BoxModeMntnCali);
        }
    }

    return;
}

void boxRxCmmnAuxAck(Can *can, Box *box, u8 *pld, u16 len)
{
    CanAuxCmdBuf *auxBuf;
    BoxCmmnAck *boxAck;

    auxBuf = can->waitAckAuxCmd;
    boxAck = (BoxCmmnAck *)pld;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    rspUpCmmn(auxBuf->upCanMsgId, box->tray->trayIdx, boxAck->rspCode);
    freeAuxCanBuf(auxBuf);

    return;
}

void boxRxCaliSmplAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxCaliSmplAck *boxAck;
    UpCaliSmplAck *upAck;
    CanAuxCmdBuf *auxBuf;
    Channel *chn;
    u8 *buf;
    u16 upPldLen;

    buf = sendUpBuf;
    upAck = (UpCaliSmplAck *)(buf + sizeof(UpMsgHead));
    boxAck = (BoxCaliSmplAck *)pld;
    auxBuf = can->waitAckAuxCmd;
    chn = &box->chn[boxAck->moduIdx/box->chnModuAmt];

    upAck->rspCode = boxAck->rspCode;
    upAck->trayIdx = box->tray->trayIdx;
    if (0 == boxAck->chnIdx)
    {
        upAck->chnIdx = chn->genIdxInTray;
    }
    else
    {
        Cell *cell;

        cell = &chn->cell[boxAck->chnIdx-1];
        upAck->chnIdx = cell->genIdxInTray;
    }
    upAck->moduIdx = boxAck->moduIdx % box->chnModuAmt;
    upAck->smplOnlyMainChn = boxAck->smplAllSeries ? False : True;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    upPldLen = sizeof(UpCaliSmplAck) + sizeof(UpldCurVolSet);
    upAck->smplAmt = 1;
    if (0 == boxAck->chnIdx)  /*?????????*/
    {
        mem2Copy((u16 *)upAck->curVolSet, boxAck->smplVal, sizeof(UpldCurVolSet));
        if (boxAck->smplAllSeries)
        {
            u8 idx;

            upPldLen += sizeof(UpldCurVolSet) * chn->chnCellAmt;
            upAck->smplAmt += chn->chnCellAmt;
            for (idx=0; idx<chn->chnCellAmt; idx++)
            {
                mem2Copy((u16 *)&upAck->curVolSet[idx+1],
                    boxAck->smplVal+(sizeof(UpldCurVolSet)+sizeof(BoxCellCurVol)*idx)/2,
                    sizeof(BoxCellCurVol));
                upAck->curVolSet[idx+1].volInner = 0; /*???????????????????????????*/
            }
        }
    }
    else
    {
        mem2Copy((u16 *)upAck->curVolSet, boxAck->smplVal, sizeof(BoxCellCurVol));
        upAck->curVolSet[0].volInner = 0; /*???????????????????????????*/
    }

    sendUpMsg(buf, upPldLen, UpCmdIdCaliSmpl);
    freeAuxCanBuf(auxBuf);

    return;
}

/*
??????Can??????,????????????????????????????????????????????????
??????????????????????????????????????????????????????,????????????????????????????????????
??????,?????????????????????????????????????????????????????????,???????????????????????????
????????????????????????????????????,???????????????????????????????????????,????????????.
*/
/*??????len????????????,????????????????????????????????????*/
void canRxMsg(u8 canId, u8 addr, u8 *data, u16 len)
{
#if TRAY_ENABLE
    Can *can;
    Box *box;
    BoxMsgHead *head;

    head = (BoxMsgHead *)data;
    can = &gDevMgr->can[canId];
    box = &gDevMgr->box[can->addr2DevBoxIdx[addr]];
    if (head->msgId<BoxMsgCri && can->canMsgFlowSeq==head->msgFlowSeq
        && can->msgIdHasTx==head->msgId && head->msgLen>=sizeof(BoxMsgHead))
    {
        can->canMsgFlowSeq++;
        gCanMgr->boxMsgProc[head->msgId](can, box, head->payload, head->msgLen-sizeof(BoxMsgHead));
        TimerStop(&can->waitCanAckTmr);  /*?????????, ???????????????*/
        canIdleNtfy(can);
    }
#endif
    return;
}

void boxInit()
{
    CanMgr *mgr;
    CanAuxCmdBuf *auxBuf;
    Can *can;
    u16 idx;

    gBoxMsgBufTx = sysMemAlloc(1536);
    if (NULL == gBoxMsgBufTx)
    {
        return;
    }

    mgr = gCanMgr = sysMemAlloc(sizeof(CanMgr));
    ListInitD(&mgr->auxBufList);

    auxBuf = sysMemAlloc(sizeof(CanAuxCmdBuf) * CanAuxBufAmt);
    for (idx=0; idx<CanAuxBufAmt; idx++,auxBuf++)
    {
        ChainInsertD(&mgr->auxBufList, &auxBuf->chain);
    }

    for (idx=0; idx<BoxMsgCri; idx++)
    {
        mgr->boxMsgProc[idx] = (BoxMsgProc)boxMsgIgnore;
    }
    mgr->boxMsgProc[BoxMsgConn] = boxRxConnAck;
    mgr->boxMsgProc[BoxMsgSmpl] = boxRxSmplAck;
    mgr->boxMsgProc[BoxMsgCtrl] = boxRxCtrlAck;
    mgr->boxMsgProc[BoxMsgCfgRead] = boxRxCfgReadAck;
    mgr->boxMsgProc[BoxMsgCfgSet] = boxRxCmmnAuxAck;
    mgr->boxMsgProc[BoxMsgUpdCnfm] = boxRxUpdCnfmAck;
    mgr->boxMsgProc[BoxMsgUpdSetup] = boxRxUpdSetupAck;
    mgr->boxMsgProc[BoxMsgUpdDld] = boxRxCmmnAuxAck;
    mgr->boxMsgProc[BoxMsgUpdUpld] = boxRxUpdUpldAck;
    mgr->boxMsgProc[BoxMsgCaliNtfy] = boxRxCaliNtfyAck;
    mgr->boxMsgProc[BoxMsgCaliStart] = boxRxCmmnAuxAck;
    mgr->boxMsgProc[BoxMsgCaliSmpl] = boxRxCaliSmplAck;
    mgr->boxMsgProc[BoxMsgCaliKb] = boxRxCmmnAuxAck;
    mgr->boxMsgProc[BoxMsgCaliStop] = boxRxCmmnAuxAck;
    mgr->boxMsgProc[BoxMsgBoxChk] = boxRxBoxChkAck;

    for (idx=0; idx<BoxMsgCri; idx++)
    {
        mgr->boxAuxAckExpr[idx] = (BoxExprRxAck)boxMsgIgnore;
    }
    mgr->boxAuxAckExpr[BoxMsgCfgRead] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgCfgSet] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgUpdCnfm] = boxExprUpdCnfmAck;
    mgr->boxAuxAckExpr[BoxMsgUpdSetup] = boxExprUpdSetupAck;
    mgr->boxAuxAckExpr[BoxMsgUpdDld] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgUpdUpld] = boxExprUpdUpldAck;
    mgr->boxAuxAckExpr[BoxMsgCaliNtfy] = boxExprCaliNtfyAck;
    mgr->boxAuxAckExpr[BoxMsgCaliStart] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgCaliSmpl] = boxExprCaliSmplAck;
    mgr->boxAuxAckExpr[BoxMsgCaliKb] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgCaliStop] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgBoxChk] = boxExprCmmnAuxAck;
    
    for (idx=0; idx<SmplModeCri; idx++)
    {
        mgr->boxSmplProc[idx] = (BoxSmplProc)boxMsgIgnore;
    }
    mgr->boxSmplProc[SmplModeChn] = boxRxSmplChn;
    mgr->boxSmplProc[SmplModeTray] = boxRxSmplTray;

    for (idx=0; idx<gDevMgr->canAmt; idx++)
    {
        can = &gDevMgr->can[idx];
        if (0 != can->boxAmt)
        {
            timerStart(&can->canSmplLoopTmr, TidCanSmplLoop, 1000+idx*300, WiReset);
        }
    }

    return;
}

#endif

