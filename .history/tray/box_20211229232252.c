

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
    SmplSaveMgr *smplMgr;
    Channel *chn;
    Channel *chnCri;
    Box *box;
    Box *boxCri;

    smplMgr = &tray->smplMgr;
    smplMgr->smplGenMore = False;
    for (chn=tray->chn,chnCri=tray->chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        chn->smplAmtInLoop = 0;
    }

    smplMgr->smplTryBoxAmt = 0;
    for (box=tray->box,boxCri=tray->box+tray->boxAmt; box<boxCri; box++)
    {
        box->boxHasSmplTry = False;
    }

    return;
}

/*从有效采样的数据中获取通道电源柜数据,以做保护*/
/*能直接用上传采样做保护是好的,不过托盘采样和通道采样不同,那样保护要两份*/
/*单独保存可以不用两份保护,不过要多一些原本不必要的内存操作*/
/*以后调整方向为托盘采样和通道采样相同,这是最好的*/
void trayRcdChnLastData(Channel *chn)
{
    ChnProtBuf *protBuf;

    protBuf = &chn->chnProtBuf;
    protBuf->cellTmprPre.tmprBeValid = protBuf->cellTmprCrnt.tmprBeValid;
    protBuf->cellTmprPre.tmprVal = protBuf->cellTmprCrnt.tmprVal;
    protBuf->cellTmprCrnt.tmprBeValid = False;
    protBuf->prePowerBeValid = protBuf->newPowerBeValid;
    protBuf->preCur = protBuf->newCur;
    protBuf->preVolCell = protBuf->newVolCell;
    protBuf->newPowerBeValid = False;
    protBuf->preCauseCode = protBuf->newCauseCode;
    protBuf->newCauseCode = CcNone;
    if (NULL != chn->series)
    {
        Cell *cell;
        Cell *cellCri;

        for (cell=chn->series->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            protBuf = &cell->chnProtBuf;
            protBuf->cellTmprPre.tmprBeValid = protBuf->cellTmprCrnt.tmprBeValid;
            protBuf->cellTmprPre.tmprVal = protBuf->cellTmprCrnt.tmprVal;
            protBuf->cellTmprCrnt.tmprBeValid = False;
            protBuf->prePowerBeValid = protBuf->newPowerBeValid;
            protBuf->preCur = protBuf->newCur;
            protBuf->preVolCell = protBuf->newVolCell;
            protBuf->newPowerBeValid = False;
            protBuf->preCauseCode = protBuf->newCauseCode;
            protBuf->newCauseCode = CcNone;
        }
    }

    return;
}
void trayRcdLastData(Tray *tray)
{
    TrayProtMgr *trayProtMgr;
    ChnProtBuf *protBuf;
    Channel *chn;
    Channel *chnCri;

    trayProtMgr = &tray->trayProtMgr;
    trayProtMgr->slotTmprPre.tmprBeValid = trayProtMgr->slotTmprCrnt.tmprBeValid;
    trayProtMgr->slotTmprPre.tmprVal = trayProtMgr->slotTmprCrnt.tmprVal;
    trayProtMgr->preNdbdValid = tray->ndbdData.ndbdDataValid;
    trayProtMgr->preNdbdNp = tray->ndbdData.status[NdbdSenNpVal];

    for (chn=tray->chn,chnCri=tray->chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        trayRcdChnLastData(chn);
    }

    return;
}

/*当前机制,下位机正常1秒1条缓存采样,中位机正常频率1秒1采*/
/*这种机制,恶劣情况下,下位机可能缓存如下4条采样等中位机采*/
/*正常1加截止1加中位机干预所致新工步首样1加下位机保护截止1*/
/*在整盘机制下,中位机要应对通道没有采样和有累积采样的情况*/
/*没有采样的情况相对容易，上述堆积采样的恶劣情况很难处理*/
/*上述恶劣情况,堆积采样都不是闲时,貌似都不能扔.*/
/*下位机缓存采样,上位机要求整盘采样,中位机夹在中间,太恶心*/
/*每轮(秒)采样,每个box最多采2次,也即最多生成2条托盘采样*/
void trayGenTraySmpl(Tray *tray)
{
    SmplSaveMgr *smplMgr;
    TraySmplRcd *traySmplRcd;
    TraySmpl *traySmpl;
    Channel *chn;
    Channel *chnCri;
    u32 seq; /*上位机确认的最后采样,这里不管转圈与否*/

    traySmplSetAux(tray);
    smplMgr = &tray->smplMgr;
    traySmplRcd = (TraySmplRcd *)smplMgr->smplBufAddr;
    traySmplRcd->timeStampSec = gAbsTimeSec + sysTimeSecGet();
    traySmpl = traySmplRcd->traySmpl;
    traySmpl->smplType = TraySmplChn;
    traySmpl->smplSize = sizeof(TrayChnSmpl);
    traySmpl->smplAmt = tray->genChnAmt;
    if (tray->protEnable)
    {
        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            if (0 == chn->smplAmtInLoop)  /*没采到样的执行通道保护*/
            {
                chnProtWork(chn);
            }
        }

        trayRcdLastData(tray); /*做完保护之后记录相关数据*/
    }
#ifdef DebugVersion
#else
    ds_write_file(tray->trayIdx, 1, smplMgr->smplBufAddr, smplMgr->smplSeqUpReq);
#endif

    trayUpdSmplSeq(smplMgr);
    seq = 0==smplMgr->smplSeqUpReq ? smplMgr->smplSeqMax : smplMgr->smplSeqUpReq-1;
    if (seq == smplMgr->smplSeqNext&&smplMgr->isLooped)
    {
        trayStopByProt(tray, Cc3UpDisc);
        smplMgr->smplEnable = False;
    }

    trayUpdSmplBuf(smplMgr);
    tray->trayProtMgr.auxDataDone = False;
    tray->trayProtMgr.allChnTmprUpLmtPointAmt = 0;
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
    ChainInsertD(list, chain);
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
    timerStart(&can->waitCanAckTmr, TidCanRxAuxAck, 200, WiReset);
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

/*初始化时设置临界类型,以避免非超时进入采样*/
/*采样定时器超时是进入采样轮次的唯一入口,初始化采样目标*/
/*应答超时或收到应答后,更新采样目标*/
/*获取下一个需要采样或联机的, can-box-idx*/
void canSmplBoxUpdate(Can *can, u8 beginIdx)
{
    u8 idx;
    Box *box;

    for (idx=beginIdx; idx<can->boxAmt; idx++)
    {
        box = &gDevMgr->box[can->can2DevBoxIdx[idx]];
        if (BoxModeManu == box->boxWorkMode)
        {
            break;
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

/*离线和采样视同处理*/
void boxSmplTx(Can *can, Box *box)
{
    BoxMsgHead *head;
    BoxSmplCmd *smpl;
    u8 *buf;

    buf = gBoxMsgBufTx;
    head = (BoxMsgHead *)buf;
    head->msgFlowSeq = can->canMsgFlowSeq;
    head->msgLen = sizeof(BoxMsgHead);
    if (box->online)
    {
        head->msgId = BoxMsgSmpl;
        smpl = (BoxSmplCmd *)(buf + sizeof(BoxMsgHead));
        smpl->lastErr = box->reTxSmplCmd ? True : False;
        smpl->rsvd = 0;
        head->msgLen += sizeof(BoxSmplCmd);
    }
    else
    {
        head->msgId = BoxMsgConn;
    }

    if (!box->boxHasSmplTry)
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
    if (TimerIsRun(&can->waitCanAckTmr)) /*can忙*/
    {
        return;
    }

    if (NULL != (box=canSmplBoxGet(can)))
    {
        boxSmplTx(can, box);
    }
    else /*can空闲但无可采box,就下一轮*/
    {
        timerStart(timer, TidCanSmplLoop, 1000, WoReset);
    }
    return;
}

void boxCtrlMix(Box *box, u16 chnSelectInd, u16 chnCtrlInd)
{
    BoxCtrlInd *ctrlInd;

    ctrlInd = &box->ctrlWaitSend.boxCtrlInd;
    if (ctrlInd->chnSelectInd & chnSelectInd) /*有交集以后者为准*/
    {
        u8 bit;

        for (bit=0; bit<16; bit++)
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
    else /*多数情况没有交集*/
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

/*todo,挪走*/
b8 stepCvFollowCcChk(Channel *chn)
{
    UpStepInfo *step;
    UpStepInfo *stepNxt;

    step = (UpStepInfo *)getChnStepInfo(chn->stepIdTmp);
    stepNxt = (UpStepInfo *)getChnStepInfo(chn->stepIdTmp+1);
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
    UpStepInfo *step;
    u16 chnSelectInd;
    u16 chnCtrlInd;
    u16 idx;

    buf = gBoxMsgBufTx;
    head = (BoxMsgHead *)buf;
    head->msgFlowSeq = can->canMsgFlowSeq;
    head->msgId = BoxMsgCtrl;
    head->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCtrlCmd);

    ctrlCmd = (BoxCtrlCmd *)(buf + sizeof(BoxMsgHead));
    ctrlCmd->isReTx = 0==box->reTxCtrlCnt ? False : True;
    ctrlCmd->chnSelectInd = box->ctrlWaitSend.boxCtrlInd.chnSelectInd;
    ctrlCmd->chnCtrlInd = box->ctrlWaitSend.boxCtrlInd.chnCtrlInd;
    for (idx=0,chnCtrl=ctrlCmd->chnCtrl; idx<box->boxChnAmt; idx++)
    {
        if (!BitIsSet(ctrlCmd->chnSelectInd, idx)) /*不操作*/
        {
            continue;
        }

        chn = &box->chn[idx];
        chn->dynStaCnt = 0;
        if (!BitIsSet(ctrlCmd->chnCtrlInd, idx)) /*停*/
        {
            continue;
        }

        step = (UpStepInfo *)getChnStepInfo(chn->stepIdTmp);
        chnCtrl->stepId = chn->stepIdTmp;
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
        else if (step->stepType > StepTypeCVD) /*todo,补充循环啥的*/
        {
            return;
        }
        else
        {
            chnCtrl->paramInd = step->paramEnable;
            memcpy(&chnCtrl->curSet, step->stepParam, sizeof(u32)*5);
            chnCtrl->timeStop = step->stepParam[2]-chn->stepRunTimeCtnu;
            chnCtrl->capStop = step->stepParam[4]-chn->capCtnu;
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

void canIdleNtfy(Can *can)
{
    Box *box;
    Tray *tray;

    if (!ListIsEmpty(&can->ctrlWaitList))  /*非空,有启停*/
    {
        BoxCtrlWaitSend *ctrl;

        ctrl = Container(BoxCtrlWaitSend, chain, can->ctrlWaitList.next);
        box = Container(Box, ctrlWaitSend, ctrl);
        boxCtrlTx(can, box);
        ChainDelSafeD(&ctrl->chain);
        return;
    }

    if (!ListIsEmpty(&can->auxWaitList))  /*有辅助维护*/
    {
        CanAuxCmdBuf *auxBuf;

        auxBuf = Container(CanAuxCmdBuf, chain, can->auxWaitList.next);
        boxAuxTx(can, auxBuf->box, auxBuf);
        return;
    }

    /*控制和维护都没有,只剩采样.*/
    if (TimerIsRun(&can->canSmplLoopTmr)) /*本轮采样未开始*/
    {
        return;
    }

    if (NULL != (box=canSmplBoxGet(can))) /*有需要采的就采*/
    {
        boxSmplTx(can, box);
        return;
    }

    /*到这里表明本轮采样结束,启动下轮采样定时*/
    timerStart(&can->canSmplLoopTmr, TidCanSmplLoop, 1000, WoReset);
    if (SmplModeTray == gDevMgr->smplMode)  /*轮询完成后尝试产生托盘采样*/
    {
        for (tray=gDevMgr->tray; tray<&gDevMgr->tray[gDevMgr->trayAmt]; tray++)
        {
            if (tray->smplMgr.smplTryBoxAmt == tray->boxAmt) /*每box都尝试了*/
            {
                if (tray->smplMgr.smplEnable)
                {
                    trayGenTraySmpl(tray);
                    if (gUpItfCb->upDiscExprHpn)
                    {
                        tray->smplMgr.smplEnable = False;
                    }
                }
                traySmplMgrRst(tray);
            }
        }
    }

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
    else  /*todo,加一条异常码数据,恢复通道状态*/
    {
        box->reTxCtrlCnt = 0;
        canIdleNtfy(can);
    }
    return;
}

void boxExprTraySmpl(Tray *tray, Box *box)
{
    Channel *chn;
    Channel *chnCri;

    for (chn=box->chn,chnCri=box->chn+box->boxChnAmt; chn<chnCri; chn++)
    {
        if (0==chn->smplAmtInLoop)
        {
            traySmplDefChnSmpl(tray, chn, ChnUpStateOffline, CcNone);
            chn->smplAmtInLoop = 1;
        }
    }

    return;
}

/*todo, 增加统计次数修改离线状态*/
void boxExprSmplAck(Timer *timer)
{
    Can *can;
    Box *box;

    can = Container(Can, waitCanAckTmr, timer);
    can->canMsgFlowSeq++;

    box = &gDevMgr->box[can->can2DevBoxIdx[can->smplCanBoxIdx]];
    box->boxHasSmplMore = False;
    if (box->online)
    {
        if (box->reTxSmplCmd)
        {
            box->online = False;
            sendUpConnNtfy();
        }
        else  /*立即接着采*/
        {
            box->reTxSmplCmd = True;
            goto canIdle;
        }
    }

    box->reTxSmplCmd = False;
    boxExprTraySmpl(box->tray, box);
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

void boxExprCfgReadAck(CanAuxCmdBuf *auxBuf)
{
    return;
}

void boxExprCfgSetAck(CanAuxCmdBuf *auxBuf)
{
    return;
}

/*todo, 增加重传,如果离开修调超时，则一直启定时器发送离开修调*/
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
        ChainDelSafeD(&auxBuf->chain);
        freeAuxCanBuf(auxBuf);
    }

    canIdleNtfy(can);
    return;
}

void _____begin_of_can_ack_recieve_____(){}

/*todo, 采集box信息*/
void boxRxConnAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxConnAck *connAck;
    TrayBoxCfg *cfg;
    SubBox *subBox;
    SubBoxInfo *subBoxAck;
    u16 idx;
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
        || connAck->chnModuAmt!=cfg->chnModuAmt || connAck->chnCellAmt!=cfg->chnCellAmt
        || connAck->volSmplAmt!=cfg->volSmplAmt || connAck->bypsSwAmt!=cfg->bypsSwAmt
        || connAck->current!=cfg->maxTrayCur || connAck->voltage!=cfg->maxTrayVol)
    {
        causeCode = Cc1LowCfg;
    }

    if (CcNone != causeCode)
    {
        Channel *chn;
        Channel *chnCri;

        /*维持不在线,且产生规格不匹配的异常码*/
        for (chn=box->chn,chnCri=box->chn+box->boxChnAmt; chn<chnCri; chn++)
        {
            if (0==chn->smplAmtInLoop)
            {
                traySmplDefChnSmpl(box->tray, chn, ChnUpStateIdle, causeCode);
                chn->smplAmtInLoop = 1;
            }
        }

        box->tray->trayWarnPres = True;
        return;
    }

    box->online = True;
    box->boxWorkMode = BoxModeManu;
    sendUpConnNtfy();
    for (idx=0,subBoxAck=connAck->boxSub; idx<cfg->volSmplAmt; idx++)
    {
        subBox = &box->tray->volSmpl[box->volSmplAmt*box->boxIdxInTray+idx];
        subBox->softVer = subBoxAck->softVer;
        subBox->online = subBoxAck->online;
        subBox->addr = subBoxAck->boxAddr;
        subBox->subType = subBoxAck->devType;
        subBoxAck++;
    }
    for (idx=0; idx<cfg->bypsSwAmt; idx++)
    {
        subBox = &box->tray->bypsSw[box->bypsSwAmt*box->boxIdxInTray+idx];
        subBox->softVer = subBoxAck->softVer;
        subBox->online = subBoxAck->online;
        subBox->addr = subBoxAck->boxAddr;
        subBox->subType = subBoxAck->devType;
        subBoxAck++;
    }

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

/*托盘采样模式的电源柜采样处理*/
/*电源柜数据直接写入上传上位机的采样,然后保护用上传采样是理想的*/
/*不过,托盘和通道两个模式的采样不同,保护要做两份*/
/*所以目前需要将数据也存本地,则每通道需要额外内存和时间开销*/
/*以后希望能固定一种模式采样,或者看看能否将两种采样调整一下*/
void boxRxSmplTray(Box *box, u16 chnIndBitMap, u8 *smpl)
{
    Tray *tray;
    Channel *chn;
    TraySmpl *traySmpl;
    u8 chnIdx;
    u16 lowSmplSize;
    u32 sysSec;

    tray = box->tray;
    traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
    lowSmplSize = getLowSmplSize(box->chn);
    sysSec = sysTimeSecGet();
    traySmplSetAux(tray);
    for (chnIdx=0; chnIdx<box->boxChnAmt; chnIdx++)
    {
        chn = &box->chn[chnIdx];
        if (!BitIsSet(chnIndBitMap, chnIdx)) /*没有数据就填充临时*/
        {
            traySmplDefChnSmpl(tray, chn, ChnUpStateTemp, CcNone);
            continue;
        }

        chnLowSmplProc(chn, &traySmpl->chnSmpl[chn->genIdxInTray], smpl, sysSec);
        smpl += lowSmplSize;
    }

    if (CcNone == tray->trayProtMgr.trayCausePre)
    {
        if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
        {
            boxCtrlTxTry(box);
        }
    }

    return;
}

/*收到采样应答,负载和长度*/
/*若下位机有堆积采样,中位机每个采样周期最多采两次*/
void boxRxSmplAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxSmplAck *ack;

    ack = (BoxSmplAck *)pld;
    box->reTxSmplCmd = False;
    gCanMgr->boxSmplProc[gDevMgr->smplMode](box, ack->chnSelectInd, (u8 *)ack->chnSmplParall);
    if (box->boxHasSmplMore)  /*一轮中最多采两次*/
    {
        box->boxHasSmplMore = False;
    }
    else
    {
        if (ack->moreSmpl)
        {
            box->boxHasSmplMore = True;
        }
    }

    canSmplBoxUpdate(can, can->smplCanBoxIdx+1);
    return;
}

/*todo, 增加下位机拒绝启动的处理*/
void boxRxCtrlAck(Can *can, Box *box, u8 *pld, u16 len)
{
    box->reTxCtrlCnt = 0;
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
    ChainDelSafeD(&auxBuf->chain);
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
        upAck->rspCode = RspPage; /*todo, 逻辑上缺少复原*/
    }
    upAck->familyVer = boxAck->bootVer;
    gUpItfCb->updMgr.pageSize = boxAck->pageSize;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpUpdSetupAck), UpCmdIdUpdSetup);
    ChainDelSafeD(&auxBuf->chain);
    freeAuxCanBuf(auxBuf);
    return;
}

void boxRxUpdDldAck(Can *can, Box *box, u8 *pld, u16 len)
{
    CanAuxCmdBuf *auxBuf;
    BoxUpdDldAck *boxAck;

    auxBuf = can->waitAckAuxCmd;
    boxAck = (BoxUpdDldAck *)pld;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    rspUpCmmn(auxBuf->upCanMsgId, box->tray->trayIdx, boxAck->rspCode);
    ChainDelSafeD(&auxBuf->chain);
    freeAuxCanBuf(auxBuf);
    return;
}

void boxRxUpdUpldAck(Can *can, Box *box, u8 *pld, u16 len)
{
    return;
}

/*todo,下位机回复错误咋办,尤其是成功一部分后突然给个失败*/
void boxRxCaliNtfyAck(Can *can, Box *box, u8 *pld, u16 len)
{
    CanAuxCmdBuf *auxBuf;
    BoxCaliNtfyAck *boxAck;

    auxBuf = can->waitAckAuxCmd;
    boxAck = (BoxCaliNtfyAck *)pld;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    rspUpCaliNtfyAck(box->tray->trayIdx, box->boxIdxInTray, boxAck->rspCode);
    if (boxAck->rspCode==RspOk && !boxAck->caliEnable)
    {
        box->boxWorkMode = BoxModeManu;    
        box->online = False;
    }
    ChainDelSafeD(&auxBuf->chain);
    freeAuxCanBuf(auxBuf);

    return;
}

void boxRxCaliCmmnAck(Can *can, Box *box, u8 *pld, u16 len)
{
    CanAuxCmdBuf *auxBuf;
    BoxCmmnAck *boxAck;

    auxBuf = can->waitAckAuxCmd;
    boxAck = (BoxCmmnAck *)pld;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    rspUpCmmn(auxBuf->upCanMsgId, box->tray->trayIdx, boxAck->rspCode);
    ChainDelSafeD(&auxBuf->chain);
    freeAuxCanBuf(auxBuf);

    return;
}

void boxRxCaliSmplAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxMsgHead *boxHead;
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

        cell = &chn->series->cell[boxAck->chnIdx-1];
        upAck->chnIdx = cell->genIdxInTray;
    }
    upAck->moduIdx = boxAck->moduIdx % box->chnModuAmt;
    upAck->smplOnlyMainChn = boxAck->smplAllSeries ? False : True;
    gUpItfCb->upMsgFlowSeq = auxBuf->upMsgFlowSeq;
    upPldLen = sizeof(UpCaliSmplAck) + sizeof(UpldCurVolSet);
    upAck->smplAmt = 1;
    if (0 == boxAck->chnIdx)  /*主通道*/
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
                upAck->curVolSet[idx+1].volInner = 0; /*子电芯没有内部电压*/
            }
        }
    }
    else
    {
        mem2Copy((u16 *)upAck->curVolSet, boxAck->smplVal, sizeof(BoxCellCurVol));
        upAck->curVolSet[0].volInner = 0; /*子电芯没有内部电压*/
    }

    sendUpMsg(buf, upPldLen, UpCmdIdCaliSmpl);
    ChainDelSafeD(&auxBuf->chain);
    freeAuxCanBuf(auxBuf);

    return;
}

void boxRxCfgReadAck(Can *can, Box *box, u8 *pld, u16 len)
{
    return;
}

void boxRxCfgSetAck(Can *can, Box *box, u8 *pld, u16 len)
{
    return;
}

/*
由于Can单工,等应答期间可能会堆积待发送消息。
处理收到消息的过程可能会产生控制需求,但不能抢占已排队控制消息
为此,收到消息后先不停止定时器以表明线路正忙,从而让接收处理过程
中可能产生的控制消息排队,即便是因保护而生的启停控制,也要排队.
*/
/*目前len可能不准,不作为消息真实长度的依据*/
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
        TimerStop(&can->waitCanAckTmr);  /*先处理, 后停定时器*/
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
    mgr->boxMsgProc[BoxMsgUpdCnfm] = boxRxUpdCnfmAck;
    mgr->boxMsgProc[BoxMsgUpdSetup] = boxRxUpdSetupAck;
    mgr->boxMsgProc[BoxMsgUpdDld] = boxRxUpdDldAck;
    mgr->boxMsgProc[BoxMsgUpdUpld] = boxRxUpdUpldAck;
    mgr->boxMsgProc[BoxMsgCaliNtfy] = boxRxCaliNtfyAck;
    mgr->boxMsgProc[BoxMsgCaliStart] = boxRxCaliCmmnAck;
    mgr->boxMsgProc[BoxMsgCaliSmpl] = boxRxCaliSmplAck;
    mgr->boxMsgProc[BoxMsgCaliKb] = boxRxCaliCmmnAck;
    mgr->boxMsgProc[BoxMsgCaliStop] = boxRxCaliCmmnAck;
    mgr->boxMsgProc[BoxMsgCfgRead] = boxRxCfgReadAck;
    mgr->boxMsgProc[BoxMsgCfgSet] = boxRxCfgSetAck;

    for (idx=0; idx<BoxMsgCri; idx++)
    {
        mgr->boxAuxAckExpr[idx] = (BoxExprRxAck)boxMsgIgnore;
    }
    mgr->boxAuxAckExpr[BoxMsgUpdCnfm] = boxExprUpdCnfmAck;
    mgr->boxAuxAckExpr[BoxMsgUpdSetup] = boxExprUpdSetupAck;
    mgr->boxAuxAckExpr[BoxMsgUpdDld] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgUpdUpld] = boxExprUpdUpldAck;
    mgr->boxAuxAckExpr[BoxMsgCaliNtfy] = boxExprCaliNtfyAck;
    mgr->boxAuxAckExpr[BoxMsgCaliStart] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgCaliSmpl] = boxExprCaliSmplAck;
    mgr->boxAuxAckExpr[BoxMsgCaliKb] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgCaliStop] = boxExprCmmnAuxAck;
    mgr->boxAuxAckExpr[BoxMsgCfgRead] = boxExprCfgReadAck;
    mgr->boxAuxAckExpr[BoxMsgCfgSet] = boxExprCfgSetAck;
    
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

