//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic.h"
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
    //trayChnSmpl->volInner = 0;
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
        //trayChnSmpl->volInner = 0;
        trayChnSmpl->current = 0;
        trayChnSmpl->capacity = 0;
    }

    return;
}

void trayUpdSmplSeq(SmplSaveMgr *smplMgr, u8 amt)
{
    smplMgr->smplSeqNext += amt;
    if (smplMgr->smplSeqNext > smplMgr->smplSeqMax)
    {
        smplMgr->smplSeqNext -= smplMgr->smplDiskAmt;
        smplMgr->smplDiskAddr = smplMgr->smplDiskAddrBase + smplMgr->smplSeqNext*smplMgr->smplItemDiskSize;
        smplMgr->isLooped = True;
    }
    else
    {
        smplMgr->smplDiskAddr += smplMgr->smplItemDiskSize * amt;
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

void traySmplSaveNdbdSmpl(TraySmpl *traySmpl, Tray *tray)
{
    TrayNdbdSmpl *trayNdbdSmpl;
    NdbdData *ndbdData;
    u16 *cellTmpr;
    u16 idx;

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
    for (idx=0; idx<trayNdbdSmpl->cellTmprAmt; idx++)
    {
        *cellTmpr++ = ndbdData->cellTmpr[idx];
    }
    traySmpl->smplSize += sizeof(u16) * Align16(cellTmpr-trayNdbdSmpl->slotTmpr);
    return;
}

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

/*1个通道或者1个库位可能不止1个温度,保护只用1个最高的*/
/*注意从采样拿还是从托盘针床数据拿,目前可以用后者,但不绝对*/
/*从后者拿稍简单,todo,以后还是要改为从前者拿,以往万一*/
void trayProtTmprGet(TraySmpl *traySmpl, Tray *tray)
{
    TmprData *tmprRcd;
    NdbdData *ndbdData;
    u16 *tmpr;

    ndbdData = &tray->ndbdData;
    tmprGetSingle(ndbdData->slotTmpr, ndbdData->slotTmprAmt, &tray->trayProtMgr.slotTmprCrnt);
    tmpr = ndbdData->cellTmpr;
    if (BoxTypeParallel == tray->boxCfg.boxType)
    {
        Channel *chn;
        Channel *chnCri;

        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            tmprGetSingle(tmpr, ndbdData->tmprAmtPerCell, &chn->chnProtBuf.cellTmprCrnt);
            tmpr += ndbdData->tmprAmtPerCell;
        }
    }
    else  /*串联时候,主通道不需要针床数据*/
    {
        Cell *cell;
        Cell *cellCri;

        for (cell=tray->cell,cellCri=cell+tray->trayCellAmt; cell<cellCri; cell++)
        {
            tmprGetSingle(tmpr, ndbdData->tmprAmtPerCell, &cell->chnProtBuf.cellTmprCrnt);
            tmpr += ndbdData->tmprAmtPerCell;
        }
    }

    return;
}

/*从确定为采样的数据中获取通道电源柜数据,以做保护*/
void trayProtChnPowerGet(TraySmpl *traySmpl, Tray *tray, Channel *chn)
{
    TrayChnSmpl *chnSmpl;
    TrayChnSmpl *smplCri;
    ChnProtBuf *chnProtBuf;
    Cell *cell;

    chnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
    chnProtBuf = &chn->chnProtBuf;
    chnProtBuf->newCur = chnSmpl->current;
    chnProtBuf->newVolCell = chnSmpl->volCell;
    chnProtBuf->newVolCur = chnSmpl->volCur;
    chnProtBuf->newVolPort = chnSmpl->volPort;
    chnProtBuf->newCap = chnSmpl->capacity;
    
    if (BoxTypeParallel == tray->boxCfg.boxType)
    {
        return;
    }

    chnSmpl++;
    for (smplCri=chnSmpl+chn->chnCellAmt,cell=chn->series->cell; chnSmpl<smplCri; chnSmpl++,cell++)
    {
        chnProtBuf = &cell->chnProtBuf;
        chnProtBuf->newCur = chnSmpl->current;
        chnProtBuf->newVolCell = chnSmpl->volCell;
        chnProtBuf->newVolCur = chnSmpl->volCur;
        chnProtBuf->newVolPort = chnSmpl->volPort;
        chnProtBuf->newCap = chnSmpl->capacity;
    }

    return;
}

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

void traySetChnPowerValid(Channel *chn)
{
    chn->chnProtBuf.newPowerBeValid = True;
    if (NULL != chn->series)
    {
        Cell *cell;
        Cell *cri;

        for (cell=chn->series->cell,cri=&chn->series->cell[chn->chnCellAmt]; cell<cri; cell++)
        {
            cell->chnProtBuf.newPowerBeValid = True;
        }
    }

    return;
}

/*仅用于托盘采样的第二条采样的保护逻辑之前*/
/*下位机采用缓存机制,中位机一轮可能采两条数据*/
/*第二条采样生成时候第一条采样还未做保护逻辑*/
/*例如下位机某通道缓存两条运行态采样,中位机凑整盘数据后做保护*/
/*如果第一条触发保护,那么需要将第二条数据的状态进行修正*/
void traySmplReWtChnSta(TraySmpl *traySmpl, Channel *chn)
{
    TrayChnSmpl *chnSmpl;
    u8eUpChnState chnUpState;

    chnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
    chnUpState = chnStaMapMed2Up(chn);
    if (CcNone != chn->chnProtBuf.preCauseCode)
    {
        chnSmpl->chnUpState = chnUpState;
        if (NULL != chn->series)
        {
            TrayChnSmpl *smplCri;
        
            chnSmpl++;
            for (smplCri=chnSmpl+chn->chnCellAmt; chnSmpl<smplCri; chnSmpl++)
            {
                chnSmpl->chnUpState = chnUpState;
            }
        }
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

    smplMgr = &tray->smplMgr;
    traySmplRcd = (TraySmplRcd *)smplMgr->smplBufAddr;
    traySmplRcd->timeStampSec = gAbsTimeSec + sysTimeSecGet();

    traySmpl = traySmplRcd->traySmpl;
    traySmpl->smplType = TraySmplChn;
    traySmpl->smplSize = sizeof(TrayChnSmpl);
    traySmpl->smplAmt = tray->genChnAmt;
    for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        if (0 == chn->smplAmtInLoop)  /*没采到样的用填充*/
        {
            traySmplDefChnSmpl(tray, chn, ChnUpStateTemp, CcNone);
        }
        else
        {
            trayProtChnPowerGet(traySmpl, tray, chn);  /*准备电源柜数据*/
        }
    }

    /*填充针床数据*/
    traySmpl = (TraySmpl *)&traySmpl->chnSmpl[tray->genChnAmt];
    traySmpl->smplType = TraySmplNdbd;
    traySmpl->smplSize = sizeof(TrayNdbdSmpl); /*填温度时追加温度长度*/
    traySmpl->smplAmt = 1;
    traySmplSaveNdbdSmpl(traySmpl, tray);

    if (tray->protEnable)
    {
        trayProtTmprGet(traySmpl, tray);  /*准备针床数据*/
        trayProtWork(tray, True);
        trayRcdLastData(tray);  /*做完保护之后记录相关数据*/
    }

    trayUpdSmplBuf(smplMgr);
    trayUpdSmplSeq(smplMgr, 1);
    if (!smplMgr->smplGenMore)
    {
        return;  /*绝大多数到此为止*/
    }

    /*第二条采样*/
    /*下位机堆积的采样,保护要怎么做?真的要疯*/
    /*如果保护后的运行态数据都扔掉,这样数据不对*/
    /*如果不扔,那么保护后可能还跟着1~2条运行态数据,这算啥?*/
    /*如果中位机强行改运行态为其它状态,数据也不对,不过也只能如此*/
    /*个人觉得保护之后有1或2条运行态数据才是合理的,但怕用户不接受*/
    traySmplRcd = (TraySmplRcd *)smplMgr->smplBufAddr;
    traySmplRcd->timeStampSec = gAbsTimeSec + sysTimeSecGet();

    traySmpl = traySmplRcd->traySmpl;
    traySmpl->smplType = TraySmplChn;
    traySmpl->smplSize = sizeof(TrayChnSmpl);
    traySmpl->smplAmt = tray->genChnAmt;
    for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        if (chn->smplAmtInLoop < 2)  /*没有二次采样的用填充*/
        {
            traySmplDefChnSmpl(tray, chn, ChnUpStateTemp, CcNone);
        }
        else
        {
            /*第二条采样保护逻辑之前,todo,先要根据第一次的保护结果修正通道状态*/
            traySmplReWtChnSta(traySmpl, chn);
            trayProtChnPowerGet(traySmpl, tray, chn);  /*准备二次采样有效的电源柜数据*/
            traySetChnPowerValid(chn); /*第一次保护逻辑后清零,这里需要单独置位*/
        }
    }

    /*填充针床数据*/
    traySmpl = (TraySmpl *)&traySmpl->chnSmpl[tray->genChnAmt];
    traySmpl->smplType = TraySmplNdbd;
    traySmpl->smplSize = sizeof(TrayNdbdSmpl); /*填温度时追加温度长度*/
    traySmpl->smplAmt = 1;
    traySmplSaveNdbdSmpl(traySmpl, tray);

    /*第二条采样的保护,不需要分析针床数据,所以也不用准备*/
    if (tray->protEnable)
    {
        trayProtWork(tray, False);

        /*第二条采样后的数据记录,只记录有效通道,那些填充的不需要*/
        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            if (2 == chn->smplAmtInLoop)
            {
                trayRcdChnLastData(chn);
            }
        }
    }

    trayUpdSmplBuf(smplMgr);
    trayUpdSmplSeq(smplMgr, 1);
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
        smpl->lastErr = False;  /*todo, 补充错误可能*/
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

/*todo, 这个可能有用,先留着*/
void boxCtrlMix(Box *box, u16 chnSelectInd, u16 chnCtrlInd)
{
    BoxCtrlInd *ctrlInd;

    ctrlInd = &box->ctrlWaitSend.boxCtrlInd;
    if (ctrlInd->chnSelectInd & chnSelectInd) /*有交集以后者为准*/
    {
        u8 bit;

        for (bit=0; bit<16; bit++)
        {
            if (1<<bit & chnSelectInd)
            {
                BitEqualSet(ctrlInd->chnCtrlInd, bit, 1<<bit & chnCtrlInd);
            }
        }
    }
    else /*多数情况没有交集,简化*/
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
#ifdef CanCtrlWoReTx
#else
    ctrlCmd->isReTx = False;
#endif
    ctrlCmd->chnSelectInd = box->ctrlWaitSend.boxCtrlInd.chnSelectInd;
    ctrlCmd->chnCtrlInd = box->ctrlWaitSend.boxCtrlInd.chnCtrlInd;
    for (idx=0,chnCtrl=ctrlCmd->chnCtrl; idx<box->boxChnAmt; idx++)
    {
        if (!(1<<idx & ctrlCmd->chnCtrlInd)) /*停*/
        {
            /*todo,状态*/
            continue;
        }

        chn = &box->chn[idx]; /*todo, 修正工步信息获取*/
        step = (UpStepInfo *)getChnStepInfo(chn->stepIdTmp);
        chnCtrl->stepId = chn->stepIdTmp;
        chn->stepTypeTmp = chnCtrl->stepType = step->stepType;
        if (StepTypeQuiet == step->stepType)
        {
            chnCtrl->paramInd = 0x04;
            chnCtrl->timeStop = step->stepParam[0];
        }
        else if (StepTypeCCC==step->stepType || StepTypeCCD==step->stepType)
        {
            chnCtrl->paramInd = 0x1d;
            chnCtrl->curSet = step->stepParam[0];
            memcpy(&chnCtrl->timeStop, &step->stepParam[1], sizeof(u32)*3);
        }
        else if (step->stepType > StepTypeCVD) /*todo,补充循环啥的*/
        {
            return;
        }
        else
        {
            chnCtrl->paramInd = step->paramEnable;
            memcpy(&chnCtrl->curSet, step->stepParam, sizeof(u32)*5);
        }

        chnStepSwPre(chn);

        head->msgLen += sizeof(ChnCtrl);
        chnCtrl++;
    }

    memcpy(&box->ctrlWaitAck, &box->ctrlWaitSend.boxCtrlInd, sizeof(BoxCtrlInd));
    memset(&box->ctrlWaitSend.boxCtrlInd, 0, sizeof(BoxCtrlInd));
    can->msgIdHasTx = head->msgId;
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
        return;
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
    if (gDevMgr->isTraySmpl)  /*轮询完成后尝试产生托盘采样*/
    {
        for (tray=gDevMgr->tray; tray<&gDevMgr->tray[gDevMgr->trayAmt]; tray++)
        {
            if (tray->smplMgr.smplTryBoxAmt == tray->boxAmt) /*每box都尝试了*/
            {
                trayGenTraySmpl(tray);
                traySmplMgrRst(tray);
            }
        }
    }

    return;
}

void _____begin_of_can_ack_expire_____(){}

/*todo, 暂时视同采样,需增加重传*/
void boxExprCtrlAck(Timer *timer)
{
    Can *can;

    can = Container(Can, waitCanAckTmr, timer);
    can->canMsgFlowSeq++;
    canIdleNtfy(can);
    return;
}

void boxExprChnSmplParal(Tray *tray, Box *box)
{
    Channel *chn;
    UpChnlSmpl *smplRcd;
    NdbdData *ndbdData;
    SmplSaveMgr *smplMgr;
    Times timeStamp;
    u8 chnIdx;
    u16 tmprIdx;

    ndbdData = &tray->ndbdData;
    smplMgr = &tray->smplMgr;
    sysTimeGet(&timeStamp);
    for (chnIdx=0; chnIdx<box->boxChnAmt; chnIdx++)
    {
        smplRcd = (UpChnlSmpl *)smplMgr->smplBufAddr;
        trayUpdSmplBuf(smplMgr);

        chn = &box->chn[chnIdx];
        smplRcd->timeStampSec = gAbsTimeSec + timeStamp.sec;
        smplRcd->timeStampMs = timeStamp.mSec;
        smplRcd->chnIdx = chn->chnIdxInTray;
        smplRcd->chnType = ChnTypeMainChn;
        smplRcd->stepId = StepIdNull;
        smplRcd->stepType = StepTypeNull;
        smplRcd->stepSubType = StepSubTypeNull;
        smplRcd->chnUpState = ChnUpStateOffline;
        smplRcd->stepSmplSeq = 0;
        smplRcd->causeCode = 0;
        smplRcd->inLoop = True;
        smplRcd->volCell = 0;
        smplRcd->volCur = 0;
        smplRcd->volPort = 0;
        smplRcd->volInner = 0;
        smplRcd->current = 0;
        smplRcd->capacity = 0;
        tmprIdx = chn->chnIdxInTray * ndbdData->tmprAmtPerCell;
        smplRcd->cellTmpr[0] = ndbdData->cellTmpr[tmprIdx];
        smplRcd->cellTmpr[1] = 1==ndbdData->tmprAmtPerCell ? 0 : ndbdData->cellTmpr[tmprIdx+1];
        smplRcd->npVal = ndbdData->status[NdbdSenNpVal];
        smplRcd->smokePres = ndbdData->warn[NdbdWarnSmoke];
        for (tmprIdx=0; tmprIdx<ndbdData->slotTmprAmt; tmprIdx++)
        {
            smplRcd->slotTmpr[tmprIdx] = ndbdData->slotTmpr[tmprIdx];
        }
    }

    trayUpdSmplSeq(smplMgr, box->boxChnAmt);
    return;
}
void boxExprChnSmplSeriesWoSw(Tray *tray, Box *box)
{
    Channel *chn;
    Cell *cell;
    NdbdData *ndbdData;
    UpChnlSmpl *smplRcd;
    UpChnlSmpl *smplSubRcd;
    SmplSaveMgr *smplMgr;
    Times timeStamp;
    u8 chnIdx;
    u8 cellIdx;
    u16 tmprIdx;

    ndbdData = &tray->ndbdData;
    smplMgr = &tray->smplMgr;
    sysTimeGet(&timeStamp);
    for (chnIdx=0; chnIdx<box->boxChnAmt; chnIdx++)
    {
        smplRcd = (UpChnlSmpl *)smplMgr->smplBufAddr;
        trayUpdSmplBuf(smplMgr);

        chn = &box->chn[chnIdx];
        smplRcd->timeStampSec = gAbsTimeSec + timeStamp.sec;
        smplRcd->timeStampMs = timeStamp.mSec;
        smplRcd->chnIdx = chn->genIdxInTray;
        smplRcd->chnType = ChnTypeMainChn;
        smplRcd->stepId = StepIdNull;
        smplRcd->stepType = StepTypeNull;
        smplRcd->stepSubType = StepSubTypeNull;
        smplRcd->chnUpState = ChnUpStateOffline;
        smplRcd->stepSmplSeq = 0;
        smplRcd->causeCode = 0;
        smplRcd->inLoop = True;
        smplRcd->volCell = 0;
        smplRcd->volCur = 0;
        smplRcd->volPort = 0;
        smplRcd->volInner = 0;
        smplRcd->current = 0;
        smplRcd->capacity = 0;
        smplRcd->cellTmpr[0] = smplRcd->cellTmpr[1] = 0;
        smplRcd->npVal = ndbdData->status[NdbdSenNpVal];
        smplRcd->smokePres = ndbdData->warn[NdbdWarnSmoke];
        for (tmprIdx=0; tmprIdx<ndbdData->slotTmprAmt; tmprIdx++)
        {
            smplRcd->slotTmpr[tmprIdx] = ndbdData->slotTmpr[tmprIdx];
        }

        for (cellIdx=0; cellIdx<chn->chnCellAmt; cellIdx++)
        {
            smplSubRcd = (UpChnlSmpl *)smplMgr->smplBufAddr;
            trayUpdSmplBuf(smplMgr);

            cell = &chn->series->cell[cellIdx];
            memcpy(smplSubRcd, smplRcd, tray->smplMgr.smplItemSize);
            smplSubRcd->chnIdx = cell->genIdxInTray;
            smplSubRcd->chnType = ChnTypeSeriesCell;
            tmprIdx = cell->cellIdxInTray * ndbdData->tmprAmtPerCell;
            smplSubRcd->cellTmpr[0] = ndbdData->cellTmpr[tmprIdx];
            smplSubRcd->cellTmpr[1] = 1==ndbdData->tmprAmtPerCell ? 0 : ndbdData->cellTmpr[tmprIdx+1];
        }
        
        trayUpdSmplSeq(smplMgr, chn->chnCellAmt+1);
    }

    return;
}

void boxExprTraySmpl(Tray *tray, Box *box)
{
    Channel *chn;
    Channel *chnCri;

    /*超时的采样不替换任何其它采样,也不因此产生第二条采样*/
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

void genExprSmplRcd(Box *box)
{
    Tray *tray;

    tray = box->tray;
    if (BoxTypeParallel == box->boxType)
    {
        if (gDevMgr->isTraySmpl)
        {
            boxExprTraySmpl(tray, box);
        }
        else
        {
            boxExprChnSmplParal(tray, box);
        }
    }
    else if (BoxTypeSeriesWoSw == box->boxType)
    {
        if (gDevMgr->isTraySmpl)
        {
            boxExprTraySmpl(tray, box);
        }
        else
        {
            boxExprChnSmplSeriesWoSw(tray, box);
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
        if (box->reTxCmd)
        {
            box->online = False;
        }
        else  /*立即接着采*/
        {
            box->reTxCmd = True;
            goto canIdle;
        }
    }

    box->reTxCmd = False;
    genExprSmplRcd(box);
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
    if (connAck->devType-1!=cfg->boxType || connAck->chnAmt!=cfg->chnAmtConn
        || connAck->chnModuAmt!=cfg->chnModuAmt || connAck->chnCellAmt!=cfg->chnCellAmt
        || connAck->volSmplAmt!=cfg->volSmplAmt || connAck->bypsSwAmt!=cfg->bypsSwAmt
        || connAck->current!=cfg->maxTrayCur || connAck->voltage!=cfg->maxTrayVol)
    {
        Channel *chn;
        Channel *chnCri;

        /*维持不在线,且产生规格不匹配的异常码*/
        for (chn=box->chn,chnCri=box->chn+box->boxChnAmt; chn<chnCri; chn++)
        {
            if (0==chn->smplAmtInLoop)
            {
                traySmplDefChnSmpl(box->tray, chn, ChnUpStateIdle, Cc1LowCfg);
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

void boxRxSmplParalChn(Box *box, u16 chnIndBitMap, void *smpl)
{
    Channel *chn;
    ChnSmplParall *chnSmpl;
    UpChnlSmpl *smplRcd;
    NdbdData *ndbdData;
    Tray *tray;
    SmplSaveMgr *smplMgr;
    Times timeStamp;
    u8 chnIdx;
    u16 tmprIdx;

    tray = box->tray;
    ndbdData = &tray->ndbdData;
    smplMgr = &tray->smplMgr;
    for (chnIdx=0,chnSmpl=(ChnSmplParall *)smpl; chnIdx<box->boxChnAmt; chnIdx++)
    {
        if (0 == (1<<chnIdx & chnIndBitMap))
        {
            continue;
        }

        smplRcd = (UpChnlSmpl *)smplMgr->smplBufAddr;
        trayUpdSmplBuf(smplMgr);
        trayUpdSmplSeq(smplMgr, 1);

        chn = &box->chn[chnIdx];
        sysTimeGet(&timeStamp);
        smplRcd->timeStampSec = gAbsTimeSec+timeStamp.sec;
        smplRcd->timeStampMs = timeStamp.mSec;
        smplRcd->chnIdx = chn->chnIdxInTray;
        smplRcd->chnType = ChnTypeMainChn;
        smplRcd->stepId = chnSmpl->stepId;
        smplRcd->stepType = chnSmpl->stepType >> 3;
        smplRcd->stepSubType = chnSmpl->stepType & 0x07;
        if (LowChnStateRun == chnSmpl->chnLowState)
        {
            smplRcd->chnUpState = ChnUpStateRun;
        }
        else if (LowChnStateStart == chnSmpl->chnLowState)
        {
            smplRcd->chnUpState = ChnUpStateStart;
        }
        else
        {
            smplRcd->chnUpState = ChnUpStateIdle;
        }
        smplRcd->stepSmplSeq = chn->smplSeqInStep++;
        smplRcd->causeCode = chnSmpl->cause;
        smplRcd->inLoop = True;
        smplRcd->volCell = chnSmpl->volCell;
        smplRcd->volCur = chnSmpl->volCur;
        smplRcd->volPort = 0;
        smplRcd->volInner = 0;
        smplRcd->current = chnSmpl->current;
        smplRcd->capacity = chnSmpl->capacity;
        tmprIdx = chn->chnIdxInTray * ndbdData->tmprAmtPerCell;
        smplRcd->cellTmpr[0] = ndbdData->cellTmpr[tmprIdx];
        smplRcd->cellTmpr[1] = 1==ndbdData->tmprAmtPerCell ? 0 : ndbdData->cellTmpr[tmprIdx+1];
        smplRcd->npVal = ndbdData->status[NdbdSenNpVal];
        smplRcd->smokePres = ndbdData->warn[NdbdWarnSmoke];
        smplRcd->slotTmprAmt = ndbdData->slotTmprAmt;
        for (tmprIdx=0; tmprIdx<ndbdData->slotTmprAmt; tmprIdx++)
        {
            smplRcd->slotTmpr[tmprIdx] = ndbdData->slotTmpr[tmprIdx];
        }

        /*记录采样完成，下面是控制*/
        if (0 != chnSmpl->cause)
        {
            chn->smplSeqInStep = 0;
            if (chnSmpl->cause < 5) /*正常截止*/
            {
                u8 stepId;

                stepId = chnSmpl->stepId + 1;
            #ifdef TmpStepSave
                if (stepId < gTmpStepAmt)
            #else
            #endif
                {
                    chn->stepIdTmp = stepId;
                    boxCtrlAddChn(box, chn->chnIdxInBox, ChnStart);
                    boxCtrlTxTry(box);
                }
            }
        }

        chnSmpl++;
    }

    return;
}

void boxRxSmplSeriesWiSwChn(Box *box, u16 chnIndBitMap, void *data)
{
}

void boxRxSmplSeriesWoSwChn(Box *box, u16 chnIndBitMap, void *smpl)
{
    Channel *chn;
    Cell *cell;
    NdbdData *ndbdData;
    ChnSmplSeries *chnSmpl;
    CellSmplSeriesWoSw *cellSmpl;
    UpChnlSmpl *smplRcd;
    UpChnlSmpl *smplSubRcd;
    Tray *tray;
    SmplSaveMgr *smplMgr;
    Times timeStamp;
    u8 chnIdx;
    u8 cellIdx;
    u16 tmprIdx;

    tray = box->tray;
    smplMgr = &tray->smplMgr;
    ndbdData = &tray->ndbdData;
    for (chnIdx=0,chnSmpl=(ChnSmplSeries *)smpl; chnIdx<box->boxChnAmt; chnIdx++)
    {
        if (0 == (1<<chnIdx & chnIndBitMap))
        {
            continue;
        }

        smplRcd = (UpChnlSmpl *)smplMgr->smplBufAddr;
        trayUpdSmplBuf(smplMgr);

        chn = &box->chn[chnIdx];
        sysTimeGet(&timeStamp);
        smplRcd->timeStampSec = gAbsTimeSec+timeStamp.sec;
        smplRcd->timeStampMs = timeStamp.mSec;
        smplRcd->chnIdx = chn->genIdxInTray;
        smplRcd->chnType = ChnTypeMainChn;
        smplRcd->stepId = chnSmpl->stepId;
        smplRcd->stepType = chnSmpl->stepType >> 3;
        smplRcd->stepSubType = chnSmpl->stepType & 0x07;
        if (LowChnStateRun == chnSmpl->chnLowState)
        {
            smplRcd->chnUpState = ChnUpStateRun;
        }
        else if (LowChnStateStart == chnSmpl->chnLowState)
        {
            smplRcd->chnUpState = ChnUpStateStart;
        }
        else
        {
            smplRcd->chnUpState = ChnUpStateIdle;
        }
        smplRcd->stepSmplSeq = chn->smplSeqInStep++;
        smplRcd->causeCode = chnSmpl->cause;
        smplRcd->inLoop = True;
        smplRcd->volCell = 0;
        smplRcd->volCur = 0;
        smplRcd->volPort = chnSmpl->volPort;
        smplRcd->volInner = 0;
        smplRcd->current = chnSmpl->current;
        smplRcd->capacity = chnSmpl->capacity;
        smplRcd->cellTmpr[0] = smplRcd->cellTmpr[1] = 0;
        smplRcd->npVal = ndbdData->status[NdbdSenNpVal];
        smplRcd->smokePres = ndbdData->warn[NdbdWarnSmoke];
        smplRcd->slotTmprAmt = ndbdData->slotTmprAmt;
        for (tmprIdx=0; tmprIdx<ndbdData->slotTmprAmt; tmprIdx++)
        {
            smplRcd->slotTmpr[tmprIdx] = ndbdData->slotTmpr[tmprIdx];
        }

        for (cellIdx=0,cellSmpl=chnSmpl->cellSmplWoSw; cellIdx<chn->chnCellAmt; cellIdx++,cellSmpl++)
        {
            smplSubRcd = (UpChnlSmpl *)smplMgr->smplBufAddr;
            trayUpdSmplBuf(smplMgr);

            cell = &chn->series->cell[cellIdx];
            memcpy(smplSubRcd, smplRcd, tray->smplMgr.smplItemSize);
            smplSubRcd->chnIdx = cell->genIdxInTray;
            smplSubRcd->chnType = ChnTypeSeriesCell;
            smplSubRcd->volCell = cellSmpl->volCell;
            smplSubRcd->volCur = cellSmpl->volCur;
            smplSubRcd->volPort = 0;
            tmprIdx = cell->cellIdxInTray * ndbdData->tmprAmtPerCell;
            smplSubRcd->cellTmpr[0] = ndbdData->cellTmpr[tmprIdx];
            smplSubRcd->cellTmpr[1] = 1==ndbdData->tmprAmtPerCell ? 0 : ndbdData->cellTmpr[tmprIdx+1];
        }

        trayUpdSmplSeq(smplMgr, chn->chnCellAmt+1);

        /*记录采样完成，下面是控制*/
        if (0 != chnSmpl->cause)
        {
            chn->smplSeqInStep = 0;
            if (chnSmpl->cause < 5) /*正常截止*/
            {
                u8 stepId;

                stepId = chnSmpl->stepId + 1;
            #ifdef TmpStepSave
                if (stepId < gTmpStepAmt)
            #else
            #endif
                {
                    chn->stepIdTmp = stepId;
                    boxCtrlAddChn(box, chn->chnIdxInBox, ChnStart);
                    boxCtrlTxTry(box);
                }
            }
        }

        chnSmpl++;
        chnSmpl = (ChnSmplSeries *)((u8 *)chnSmpl + sizeof(CellSmplSeriesWoSw)*chn->chnCellAmt);
    }
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

u8eUpChnState chnStaMapMed2Up(Channel *chn)
{
    return gDevMgr->chnStaMapMed2Up[chn->chnStateMed];
}

/*todo,暂时简单做,需改为函数矩阵,且带着工步号工步类型一起,否则状态与数据不符也不太好*/
/*只有中位机在非稳定态时候,才可能随着下位机状态变化而变化*/
/*对于中位机稳定态,不受下位机影响*/
void chnStaMapLow2Med(Channel *chn, u8eLowChnState chnLowState, u32 timeStampMs)
{
    if (ChnStaFlowStart == chn->chnStateMed)
    {
        if (LowChnStateRun == chnLowState)
        {
            chn->chnStateMed = ChnStaRun;
            chn->stepRunTime = 0;
            chn->stepRunTimeBase = timeStampMs;
        }
        else
        {
            chn->dynStaCnt++; /*保持状态同时防呆计数,达到阈值防呆触发,todo*/
        }
    }
    else if (ChnStaStartReqMed == chn->chnStateMed)
    {
        if (LowChnStateRun == chnLowState)
        {
            chn->chnStateMed = ChnStaRun;
            chn->stepRunTime = 0;
            chn->stepRunTimeBase = timeStampMs;
        }
        else if (LowChnStateStart == chnLowState)
        {
            chn->chnStateMed = ChnStaStartReqLow;
            chn->dynStaCnt = 0;
        }
        else
        {
            chn->dynStaCnt++; /*保持状态同时防呆计数,达到阈值防呆触发,todo*/
        }
    }
    else if (ChnStaStartReqLow == chn->chnStateMed)
    {
        if (LowChnStateRun == chnLowState)
        {
            chn->chnStateMed = ChnStaRun;
            chn->stepRunTime = 0;
            chn->stepRunTimeBase = timeStampMs;
        }
        else
        {
            chn->dynStaCnt++; /*保持状态同时防呆计数,达到阈值防呆触发,todo*/
        }
    }
    else if (ChnStaStopReqUp == chn->chnStateMed)
    {
        if (LowChnStateRun != chnLowState)
        {
            chn->chnStateMed = ChnStaIdle;
            chnEnterIdle(chn);
        }
        else
        {
            chn->dynStaCnt++; /*保持状态同时防呆计数,达到阈值防呆触发,todo*/
        }
    }
    else if (ChnStaPauseReqUp==chn->chnStateMed || ChnStaPauseReqMed==chn->chnStateMed)
    {
        if (LowChnStateRun != chnLowState)
        {
            chn->chnStateMed = ChnStaPause;
            chnEnterIdle(chn);
        }
        else
        {
            chn->dynStaCnt++; /*保持状态同时防呆计数,达到阈值防呆触发,todo*/
        }
    }
    else if (ChnStaPauseReqMed == chn->chnStateMed)
    {
        if (LowChnStateRun != chnLowState)
        {
            chn->chnStateMed = ChnStaStop;
            chnEnterIdle(chn);
        }
        else
        {
            chn->dynStaCnt++; /*保持状态同时防呆计数,达到阈值防呆触发,todo*/
        }
    }

    return;
}

void boxRxSmplParalTray(Box *box, u16 chnIndBitMap, void *smpl)
{
    Tray *tray;
    Channel *chn;
    ChnSmplParall *lowChnSmpl;
    SmplSaveMgr *smplMgr;
    TraySmpl *traySmpl;
    TraySmpl *traySmplSnd;
    TrayChnSmpl *trayChnSmpl;
    u8 chnIdx;

    tray = box->tray;
    smplMgr = &tray->smplMgr;
    traySmpl = ((TraySmplRcd *)smplMgr->smplBufAddr)->traySmpl;
    for (chnIdx=0,lowChnSmpl=(ChnSmplParall *)smpl; chnIdx<box->boxChnAmt; chnIdx++)
    {
        if (0 == (1<<chnIdx & chnIndBitMap))
        {
            continue;
        }

        chn = &box->chn[chnIdx];
        trayChnSmpl = &traySmpl->chnSmpl[chn->chnIdxInTray];
        if (0 == chn->smplAmtInLoop)
        {
            chn->smplAmtInLoop = 1;
        }
        else if (ChnUpStateRun==trayChnSmpl->chnUpState || CcNone!=trayChnSmpl->causeCode)
        {
            if (LowChnStateRun!=lowChnSmpl->chnLowState && CcNone==lowChnSmpl->cause)
            {
                continue;  /*可以丢弃*/
            }

            chn->smplAmtInLoop = 2;  /*不覆盖不丢弃那就第二条*/
            smplMgr->smplGenMore = True;
            if (smplMgr->smplBufAddr == smplMgr->smplBufAddrMax)
            {
                trayChnSmpl = (TrayChnSmpl *)(smplMgr->smplBufAddrBase + ((u8 *)trayChnSmpl - smplMgr->smplBufAddr));
            }
            else
            {
                trayChnSmpl = (TrayChnSmpl *)((u8 *)trayChnSmpl + smplMgr->smplItemSize);
            }
        }

        trayChnSmpl->chnType = ChnTypeMainChn;
        chnStaMapLow2Med(chn, lowChnSmpl->chnLowState, lowChnSmpl->timeStamp);
        trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);
        if (ChnStaRun == chn->chnStateMed)
        {
            chnRunTimeUpd(chn, lowChnSmpl->timeStamp);
        }
        trayChnSmpl->stepId = lowChnSmpl->stepId;
        trayChnSmpl->stepType = lowChnSmpl->stepType >> 3;
        trayChnSmpl->stepSubType = lowChnSmpl->stepType & 0x07;
        chn->stepSubType = trayChnSmpl->stepSubType;
        trayChnSmpl->inLoop = True;
        trayChnSmpl->causeCode = lowChnSmpl->cause;
        trayChnSmpl->stepRunTime = chn->stepRunTime;
        trayChnSmpl->volCell = lowChnSmpl->volCell;
        trayChnSmpl->volCur = lowChnSmpl->volCur;
        trayChnSmpl->volPort = 0;
        //trayChnSmpl->volInner = 0;
        trayChnSmpl->current = lowChnSmpl->current;
        trayChnSmpl->capacity = lowChnSmpl->capacity + chn->capCtnu;

        /*准备保护需要的数据*/
        traySetChnPowerValid(chn);

        /*记录采样完成，下面是控制,1~5是正常条件截止*/
        if (0 != lowChnSmpl->cause)
        {
            trayNpChnFree(tray, chn);
            if (lowChnSmpl->cause < 6)
            {
                if (ChnStaRun==chn->chnStateMed || ChnStaFlowStart==chn->chnStateMed
                    || ChnStaStartReqLow==chn->chnStateMed || ChnStaStartReqMed==chn->chnStateMed)
                {
                    u8 stepId;

                    stepId = lowChnSmpl->stepId + 1;
                    if (stepId < gTmpStepAmt)
                    {
                        chn->stepIdTmp = stepId;
                        trayNpChnFree(tray, chn);
                        if (Ok == trayNpChnAlloc(tray, chn, stepId))
                        {
                            chn->chnStateMed = ChnStaStartReqLow;
                            chn->dynStaCnt = 0;
                            boxCtrlAddChn(box, chn->chnIdxInBox, ChnStart);
                            /*boxCtrlTxTry(box);*/
                        }
                        else
                        {
                            chn->chnStateMed = ChnStaNp;
                        }
                    }
                    else  /*todo,这里的闲时时戳不完整,例如第一步下位机直接截止,此时属于一直闲时而不应该记录时戳*/
                    {
                        chn->chnStateMed = ChnStaIdle;
                        chnEnterIdle(chn);
                        chn->stepRunTime = 0;
                    }
                }
            }
            else  /*下位机保护*/
            {
                if (ChnStaRun==chn->chnStateMed || ChnStaFlowStart==chn->chnStateMed
                    || ChnStaStartReqLow==chn->chnStateMed || ChnStaStartReqMed==chn->chnStateMed)
                {
                    chn->chnStateMed = StepProtPause==gTmpStepProtPolicy ? ChnStaPause : ChnStaStop;
                    chnEnterIdle(chn);
                    chn->stepRunTime = 0;
                }
            }
        }

        lowChnSmpl++;
    }

    if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
    {
        boxCtrlTxTry(box);
    }

    return;
}

void boxRxSmplSeriesWiSwTray(Box *box, u16 chnIndBitMap, void *data)
{
}

/*一轮采样每箱最多采2次*/
/*第2次,看看能否覆盖,不能覆盖就看看能否丢弃,拿不准就不覆盖不丢弃*/
void boxRxSmplSeriesWoSwTray(Box *box, u16 chnIndBitMap, void *smpl)
{
    Tray *tray;
    Channel *chn;
    Cell *cell;
    ChnSmplSeries *lowChnSmpl;
    CellSmplSeriesWoSw *cellSmpl;
    CellSmplSeriesWoSw *cellSmplCri;
    SmplSaveMgr *smplMgr;
    TraySmpl *traySmpl;
    TrayChnSmpl *trayChnSmpl;
    u8 chnIdx;

    tray = box->tray;
    smplMgr = &tray->smplMgr;
    traySmpl = ((TraySmplRcd *)smplMgr->smplBufAddr)->traySmpl;
    for (chnIdx=0,lowChnSmpl=(ChnSmplSeries *)smpl; chnIdx<box->boxChnAmt; chnIdx++)
    {
        if (0 == (1<<chnIdx & chnIndBitMap))
        {
            continue;
        }

        chn = &box->chn[chnIdx];
        trayChnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
        if (0 == chn->smplAmtInLoop)
        {
            chn->smplAmtInLoop = 1;
        }
        else if (ChnUpStateRun==trayChnSmpl->chnUpState || CcNone!=trayChnSmpl->causeCode)
        {
            if (LowChnStateRun!=lowChnSmpl->chnLowState && CcNone==lowChnSmpl->cause)
            {
                continue; /*可以的话就丢弃*/
            }
            
            chn->smplAmtInLoop = 2;
            smplMgr->smplGenMore = True;
            if (smplMgr->smplBufAddr == smplMgr->smplBufAddrMax)
            {
                trayChnSmpl = (TrayChnSmpl *)(smplMgr->smplBufAddrBase + ((u8 *)trayChnSmpl - smplMgr->smplBufAddr));
            }
            else
            {
                trayChnSmpl = (TrayChnSmpl *)((u8 *)trayChnSmpl + smplMgr->smplItemSize);
            }
        }

        trayChnSmpl->chnType = ChnTypeMainChn;
        chnStaMapLow2Med(chn, lowChnSmpl->chnLowState, lowChnSmpl->timeStamp);
        trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);
        if (ChnStaRun == chn->chnStateMed)
        {
            chnRunTimeUpd(chn, lowChnSmpl->timeStamp);
        }
        trayChnSmpl->stepId = lowChnSmpl->stepId;
        trayChnSmpl->stepType = lowChnSmpl->stepType >> 3;
        trayChnSmpl->stepSubType = lowChnSmpl->stepType & 0x07;
        chn->stepSubType = trayChnSmpl->stepSubType;
        trayChnSmpl->inLoop = True;
        trayChnSmpl->causeCode = lowChnSmpl->cause;
        trayChnSmpl->stepRunTime = chn->stepRunTime;
        trayChnSmpl->volCell = 0;
        trayChnSmpl->volCur = 0;
        trayChnSmpl->volPort = lowChnSmpl->volPort;
        //trayChnSmpl->volInner = 0;
        trayChnSmpl->current = lowChnSmpl->current;
        trayChnSmpl->capacity = lowChnSmpl->capacity + chn->capCtnu;

        cellSmpl = lowChnSmpl->cellSmplWoSw;
        cellSmplCri = cellSmpl + chn->chnCellAmt;
        for (trayChnSmpl++; cellSmpl<cellSmplCri; trayChnSmpl++,cellSmpl++)
        {
            trayChnSmpl->chnType = ChnTypeSeriesCell;
            trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);
            trayChnSmpl->stepId = lowChnSmpl->stepId;
            trayChnSmpl->stepType = lowChnSmpl->stepType >> 3;
            trayChnSmpl->stepSubType = lowChnSmpl->stepType & 0x07;
            trayChnSmpl->inLoop = True;
            trayChnSmpl->causeCode = lowChnSmpl->cause;
            trayChnSmpl->stepRunTime = chn->stepRunTime;
            trayChnSmpl->volCell = cellSmpl->volCell;
            trayChnSmpl->volCur = cellSmpl->volCur;
            trayChnSmpl->volPort = 0;
            //trayChnSmpl->volInner = 0;
            trayChnSmpl->current = lowChnSmpl->current;
            trayChnSmpl->capacity = lowChnSmpl->capacity + chn->capCtnu;  /*极简容量相同*/
        }

        /*准备保护需要的数据*/
        traySetChnPowerValid(chn);

        /*记录采样完成，下面是控制*/
        if (0 != lowChnSmpl->cause)
        {
            trayNpChnFree(tray, chn);
            if (lowChnSmpl->cause < 6)
            {
                if (ChnStaRun==chn->chnStateMed || ChnStaFlowStart==chn->chnStateMed
                    || ChnStaStartReqLow==chn->chnStateMed || ChnStaStartReqMed==chn->chnStateMed)
                {
                    u8 stepId;

                    stepId = lowChnSmpl->stepId + 1;
                    if (stepId < gTmpStepAmt)
                    {
                        chn->stepIdTmp = stepId;
                        if (Ok == trayNpChnAlloc(tray, chn, stepId))
                        {
                            chn->chnStateMed = ChnStaStartReqLow;
                            chn->dynStaCnt = 0;
                            boxCtrlAddChn(box, chn->chnIdxInBox, ChnStart);
                            /*boxCtrlTxTry(box);*/
                        }
                        else
                        {
                            chn->chnStateMed = ChnStaNp;
                        }
                    }
                    else
                    {
                        chn->chnStateMed = ChnStaIdle;
                        chnEnterIdle(chn);
                        chn->stepRunTime = 0;
                    }
                }
            }
            else  /*下位机保护*/
            {
                if (ChnStaRun==chn->chnStateMed || ChnStaFlowStart==chn->chnStateMed
                    || ChnStaStartReqLow==chn->chnStateMed || ChnStaStartReqMed==chn->chnStateMed)
                {
                    chn->chnStateMed = StepProtPause==gTmpStepProtPolicy ? ChnStaPause : ChnStaStop;
                    chnEnterIdle(chn);
                    chn->stepRunTime = 0;
                }
            }
        }

        lowChnSmpl++;
        lowChnSmpl = (ChnSmplSeries *)((u8 *)lowChnSmpl + sizeof(CellSmplSeriesWoSw)*chn->chnCellAmt);
    }

    if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
    {
        boxCtrlTxTry(box);
    }
    return;
}

/*收到采样应答,负载和长度*/
/*若下位机有堆积采样,中位机每个采样周期最多采两次*/
void boxRxSmplAck(Can *can, Box *box, u8 *pld, u16 len)
{
    BoxSmplAck *ack;

    ack = (BoxSmplAck *)pld;
    gCanMgr->boxSmplRcd[gDevMgr->isTraySmpl][box->boxType](box, ack->chnSelectInd, ack->chnSmplParall);
    if (ack->moreSmpl && !box->boxHasSmplMore) /*若有更多缓存就继续采该box,但最多2次*/
    {
        box->boxHasSmplMore = True;
        return;  /*不更新,继续采本箱*/
    }

    box->boxHasSmplMore = False;
    canSmplBoxUpdate(can, can->smplCanBoxIdx+1);
    return;
}

void boxRxCtrlAck(Can *can, Box *box, u8 *pld, u16 len)
{
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
        box->reTxCmd = False;
        gCanMgr->boxMsgProc[head->msgId](can, box, head->payload, head->msgLen-sizeof(BoxMsgHead));
        TimerStop(&can->waitCanAckTmr);  /*先处理, 后停定时器*/
        canIdleNtfy(can);
    }
    return;
}

void boxInit()
{
    CanMgr *mgr;
    CanAuxCmdBuf *auxBuf;
    Can *can;
    u8 idx;

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
    
    for (idx=0; idx<BoxTypeCri; idx++)
    {
        mgr->boxSmplRcd[False][idx] = (BoxSmplRcd)boxMsgIgnore;
        mgr->boxSmplRcd[True][idx] = (BoxSmplRcd)boxMsgIgnore;
    }
    mgr->boxSmplRcd[False][BoxTypeParallel] = boxRxSmplParalChn;
    mgr->boxSmplRcd[False][BoxTypeSeriesWiSw] = boxRxSmplSeriesWiSwChn;
    mgr->boxSmplRcd[False][BoxTypeSeriesWoSw] = boxRxSmplSeriesWoSwChn;
    mgr->boxSmplRcd[True][BoxTypeParallel] = boxRxSmplParalTray;
    mgr->boxSmplRcd[True][BoxTypeSeriesWiSw] = boxRxSmplSeriesWiSwTray;
    mgr->boxSmplRcd[True][BoxTypeSeriesWoSw] = boxRxSmplSeriesWoSwTray;

    for (idx=0; idx<gDevMgr->canAmt; idx++)
    {
        can = &gDevMgr->can[idx];
        if (0 != can->boxAmt)
        {
            timerStart(&can->canSmplLoopTmr, TidCanSmplLoop, 1000, WiReset);
        }
    }

    return;
}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译

