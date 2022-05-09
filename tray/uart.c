
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
#include "uart.h"
#include "host.h"
#include "tray.h"

#ifdef DebugVersion
#else
#include "utp.h"
#endif

/*不依赖上位机信息的用这个buf,依赖于记录上位机信息的用动态blockBuf*/
/*todo,可以考虑整合统一,也可不用统一,但这个buf需要调整*/
u8 *gUartSendBuf;
UartMgr *gUartMgr;

void uartMsgIgnore()
{
    return;
}

b8 tmprSmplBeOnline(u8 trayIdx)
{
    Tray *tray;
    u8 idx;

    tray = &gDevMgr->tray[trayIdx];
    for (idx=0; idx<tray->tmprSmplAmt; idx++)
    {
        if (!gDevMgr->tmprSmpl[tray->tmprSmplIdxBase+idx].online)
        {
            return False;
        }
    }

    return True;
}

void uartTxMsg(Uart *uart, u8 *buf)
{
    UartMsgHead *uartHead;

    uartHead = (UartMsgHead *)buf;
    uartHead->flowSeq = uart->uartMsgFlowSeq;
#ifdef DebugVersion
#else
    Utp_TxMessage(uart->uartIdx, buf, uartHead->pldLen+sizeof(UartMsgHead));
#endif
    return;
}

void *allocUartBlockBuf()
{
    UartMgr *mgr;
    UartBlockCmdBuf *blockBuf;
    ChainD *chain;

    mgr = gUartMgr;
    if (ListIsEmpty(&mgr->blockBufList))
    {
        return NULL;
    }

    chain = mgr->blockBufList.next;
    ChainDelSafeD(chain);
    blockBuf = Container(UartBlockCmdBuf, chain, chain);
    return blockBuf;
}

void freeUartBlockBuf(void *buf)
{
    ListD *list;
    ChainD *chain;

    list = &gUartMgr->blockBufList;
    chain = &((UartBlockCmdBuf *)buf)->chain;
    ChainInsertD(list, chain);
    return;
}

void setUartBlockBuf(UartBlockCmdBuf *blockBuf, u8 trayIdx, u16 upCmdId, u8 cmmuAddr,
                         u8eUartDevType uartDevType, u8eUartMsgId uartMsgId)
{
    UartMsgHead *uartHead;
    UartCmmnCmd *uartCmd;

    blockBuf->trayIdx = trayIdx;
    blockBuf->reTxCnt = 0;
    blockBuf->upMsgFlowSeq = gUpItfCb->upMsgFlowSeq;
    blockBuf->upUartMsgId = upCmdId;
    blockBuf->uartCmmuAddr = cmmuAddr;

    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->devAddr = cmmuAddr;
    uartHead->funcCode = UartFuncCode;

    uartCmd = (UartCmmnCmd *)uartHead->payload;
    uartCmd->devType = uartDevType;
    uartCmd->msgId = uartMsgId;
    return;
}

/*适配有点拖沓,有时间再整合*/
void uartNdbdMcuCtrlAdd(u8 trayIdx, u8eNdbdCtrlType type, s16 oprVal, s16 npVal)
{
    NdbdMcu *ndbdMcu;
    NdbdMcuCtrl *ctrl;

    ndbdMcu =  &gDevMgr->ndbdMcu[trayIdx];
    ctrl = &ndbdMcu->ctrlWaitTx.ctrl;
    if (NdbdSetTouch == type)
    {
        ctrl->touchAct = oprVal;
    }
    else if (NdbdSetWarnDel == type)
    {
        ctrl->ndbdWarnDel = oprVal;
    }
    else if (NdbdSetFireDoor == type)
    {
        ctrl->fireDoorAct = 1==oprVal ? 2 : 1;
    }
    else if (NdbdSetRatioVal == type)
    {
        ctrl->npRatio = oprVal;
        ctrl->npVal = npVal;
    }
    else if (NdbdSetBrkVacum == type)
    {
        ctrl->vacumBrk = oprVal;
    }
    else if (NdbdSetFixtPower == type)
    {
        ctrl->fixtPower = oprVal + 1;
    }
    else if (NdbdSetSlotFan == type)
    {
        ctrl->slotFanAct = oprVal + 1;
    }
    else if (NdbdSetSmokeRmv == type)  /*目前plc没有*/
    {
        ctrl->smokeAct = oprVal + 1;
    }
    else if (NdbdSetSwValve == type)  /*目前针床主控没有*/
    {
    }

    return;
}

void uartNdbdMcuCtrlTx(Uart *uart, NdbdMcu *ndbdMcu)
{
    u8 *buf;
    UartMsgHead *uartHead;
    NdbdMcuCtrlCmd *ctrlCmd;
    NdbdMcuCtrl *ctrl;

    buf = gUartSendBuf;
    uartHead = (UartMsgHead *)buf;
    uartHead->devAddr = ndbdMcu->cmmuAddr;
    uartHead->funcCode = UartFuncCode;
    uartHead->pldLen = sizeof(NdbdMcuCtrlCmd);

    ctrlCmd = (NdbdMcuCtrlCmd *)uartHead->payload;
    ctrlCmd->devType = UartNdbdMcu;
    ctrlCmd->msgId = UartMsgIdSpec(UartMsgNdbdMcuCtrl);

    ctrl = &ndbdMcu->ctrlWaitTx.ctrl;
    ctrlCmd->fixtPower = ctrl->fixtPower;
    ctrlCmd->touchAct = ctrl->touchAct;
    ctrlCmd->ndbdWarnDel = ctrl->ndbdWarnDel;
    ctrlCmd->fireDoorAct = ctrl->fireDoorAct;
    ctrlCmd->smokeAct = ctrl->smokeAct;
    ctrlCmd->slotFanAct = ctrl->slotFanAct;
    ctrlCmd->npHigh = ctrl->npHigh;
    ctrlCmd->npLow = ctrl->npLow;
    ctrlCmd->vacumBrk = ctrl->vacumBrk;
    ctrlCmd->npRatio = ctrl->npRatio;
    ctrlCmd->npVal = ctrl->npVal;

    uartTxMsg(uart, buf);
    uart->mcuIdxWaitCtrlAck = ndbdMcu->ndbdMcuIdx;
    memcpy(&ndbdMcu->ctrlWaitAck, ctrl, sizeof(NdbdMcuCtrl));
    memset(ctrl, 0, sizeof(NdbdMcuCtrl));
    timerStart(&uart->waitUartAckTmr, TidUartRxCtrlAck, 100, WiReset);
    return;
}

void uartNdbdMcuCtrlTxTry(u8 trayIdx)
{
    Uart *uart;
    NdbdMcu *ndbdMcu;

    ndbdMcu =  &gDevMgr->ndbdMcu[trayIdx];
    uart = &gDevMgr->uart[ndbdMcu->uartIdx];
    if (TimerIsRun(&uart->waitUartAckTmr))
    {
        if (!ChainInList(&ndbdMcu->ctrlWaitTx.chain))
        {
            ChainInsertD(&uart->ctrlWaitList, &ndbdMcu->ctrlWaitTx.chain);
        }
    }
    else
    {
        uartNdbdMcuCtrlTx(uart, ndbdMcu);
    }
    return;
}

void uartTransTxTry(u8 uartIdx, void *buf)
{
    Uart *uart;
    UartBlockCmdBuf *blockBuf;

    uart = &gDevMgr->uart[uartIdx];
    blockBuf = (UartBlockCmdBuf *)buf;
    if (TimerIsRun(&uart->waitUartAckTmr))
    {
        if (!ChainInList(&blockBuf->chain))
        {
            ChainInsertD(&uart->uartBlockList, &blockBuf->chain);
        }
        return;
    }

    uart->waitAckBlockBuf = blockBuf;
    uartTxMsg(uart, blockBuf->msgBuf);
    timerStart(&uart->waitUartAckTmr, TidUartRxBlkAck, 300, WiReset);
    return;
}

/*联机采样视同*/
/*todo,温度盒采样有分组优化,后续需落地,目前暂按一次一盘*/
void uartSmplTmprTx(Uart *uart)
{
    TmprSmpl *tmprSmpl;
    UartMsgHead *uartHead;
    u8 *buf;

    buf = gUartSendBuf;
    tmprSmpl = &gDevMgr->tmprSmpl[uart->uartCrntSmplDevIdx];
    uartHead = (UartMsgHead *)buf;
    uartHead->devAddr = tmprSmpl->cmmuAddr;
    uartHead->funcCode = UartFuncCode;
    if (tmprSmpl->online)
    {
        TmprSmplSmplCmd *uartSmplCmd;
        TmprSmplInd *tmprSmplInd;

        uartHead->pldLen = sizeof(TmprSmplSmplCmd);
        uartSmplCmd = (TmprSmplSmplCmd *)uartHead->payload;
        uartSmplCmd->devType = UartTmprSmpl;
        uartSmplCmd->msgId = UartMsgSmpl;
        uartSmplCmd->tmprSmplGrpAmt = 0;
        tmprSmplInd = uartSmplCmd->tmprSmplInd;
        if (0 != tmprSmpl->tmprAmt4Cell)
        {
            tmprSmplInd->tmprSmplChnIdx = tmprSmpl->tmprBase4Cell;
            tmprSmplInd->tmprSmplChnAmt = tmprSmpl->tmprAmt4Cell;
            uartSmplCmd->tmprSmplGrpAmt++;
            uartHead->pldLen += sizeof(TmprSmplInd);
            tmprSmplInd++;
        }

        /*水温和库温,现在是连续的,水温后面是库温,暂时简化处理*/
        /*todo,,以后如果不连续或库温在前或有水温无库温等情况,需要修正这里*/
        if (0 != tmprSmpl->tmprAmt4Loc)
        {
            if (0 != tmprSmpl->tmprAmt4Water)
            {
                tmprSmplInd->tmprSmplChnIdx = tmprSmpl->tmprBase4Water;
                tmprSmplInd->tmprSmplChnAmt = tmprSmpl->tmprAmt4Loc+tmprSmpl->tmprAmt4Water;
            }
            else
            {
                tmprSmplInd->tmprSmplChnIdx = tmprSmpl->tmprBase4Loc;
                tmprSmplInd->tmprSmplChnAmt = tmprSmpl->tmprAmt4Loc;
            }
            uartSmplCmd->tmprSmplGrpAmt++;
            uartHead->pldLen += sizeof(TmprSmplInd);
        }
    }
    else
    {
        UartCmmnCmd *uartCmmnCmd;

        uartHead->pldLen = sizeof(UartCmmnCmd);
        uartCmmnCmd = (UartCmmnCmd *)uartHead->payload;
        uartCmmnCmd->devType = UartTmprSmpl;
        uartCmmnCmd->msgId = UartMsgConn;
    }

    uartTxMsg(uart, buf);
    timerStart(&uart->waitUartAckTmr, TidUartRxSmplAck, 100, WiReset);
    return;
}

/*联机采样视同*/
void uartSmplNdbdMcu(Uart *uart)
{
    NdbdMcu *ndbdMcu;
    UartMsgHead *uartHead;
    UartCmmnCmd *uartCmmnCmd;
    u8 *buf;

    buf = gUartSendBuf;
    ndbdMcu = &gDevMgr->ndbdMcu[uart->uartCrntSmplDevIdx];
    uartHead = (UartMsgHead *)buf;
    uartHead->devAddr = ndbdMcu->cmmuAddr;
    uartHead->funcCode = UartFuncCode;
    uartHead->pldLen = sizeof(UartCmmnCmd);

    uartCmmnCmd = (UartCmmnCmd *)uartHead->payload;
    uartCmmnCmd->devType = UartNdbdMcu;
    uartCmmnCmd->msgId = ndbdMcu->online ? UartMsgSmpl : UartMsgConn;

    uartTxMsg(uart, buf);
    timerStart(&uart->waitUartAckTmr, TidUartRxSmplAck, 100, WiReset);
    return;
}

/*初始化时设置临界类型,以避免因非超时而进入采样*/
/*采样定时器超时是进入采样轮次的唯一入口,初始化采样目标*/
/*应答超时或收到应答后,更新采样目标*/
/*todo,需修正温度盒采样优化后分组,现在只做一次采完一个设备的逻辑*/
void uartSmplDevReset(Uart *uart)
{
    uart->uartCrntSmplDevIdx = 0;
    for (uart->uartCrntSmplType=UartTmprSmpl; uart->uartCrntSmplType<UartNeedSmplCri; uart->uartCrntSmplType++)
    {
        if (uart->uartCrntSmplDevIdx < uart->uartSmplDevAmt[uart->uartCrntSmplType])
        {
            return;
        }
        uart->uartCrntSmplDevIdx = 0;
    }
    return;
}
void uartSmplDevUpdate(Uart *uart)
{
    for (uart->uartCrntSmplDevIdx+=1; uart->uartCrntSmplType<UartNeedSmplCri; uart->uartCrntSmplType++)
    {
        if (uart->uartCrntSmplDevIdx < uart->uartSmplDevAmt[uart->uartCrntSmplType])
        {
            return;
        }
        uart->uartCrntSmplDevIdx = 0;
    }
    return;
}

void uartSmplBegin(Timer *timer)
{
    Uart *uart;

    uart = Container(Uart, uartSmplLoopTmr, timer);
    uartSmplDevReset(uart);
    if (TimerIsRun(&uart->waitUartAckTmr))
    {
        return;
    }

    if (uart->uartCrntSmplType < UartNeedSmplCri)
    {
        gUartMgr->uartSmplTx[uart->uartCrntSmplType](uart);
    }
    else /*空闲且没有可采,就准备下一轮*/
    {
        timerStart(timer, TidUartSmplLoop, 1000, WoReset);
    }
    return;
}

void uartIdleNtfy(Uart *uart)
{
    if (!ListIsEmpty(&uart->ctrlWaitList))
    {
        NdbdMcuCtrlNode *ctrlWaitTx;
        NdbdMcu *ndbdMcu;

        ctrlWaitTx = Container(NdbdMcuCtrlNode, chain, uart->ctrlWaitList.next);
        ndbdMcu = Container(NdbdMcu, ctrlWaitTx, ctrlWaitTx);
        uartNdbdMcuCtrlTx(uart, ndbdMcu);
        ChainDelSafeD(&ctrlWaitTx->chain);
        return;
    }

    if (!ListIsEmpty(&uart->uartBlockList))
    {
        UartBlockCmdBuf *blockBuf;

        blockBuf = Container(UartBlockCmdBuf, chain, uart->uartBlockList.next);
        uart->waitAckBlockBuf = blockBuf;
        uartTxMsg(uart, blockBuf->msgBuf);
        timerStart(&uart->waitUartAckTmr, TidUartRxBlkAck, 300, WiReset);
        return;
    }

    if (!TimerIsRun(&uart->uartSmplLoopTmr))  /*定时器空闲表明本轮采样已启动*/
    {
        if (uart->uartCrntSmplType < UartNeedSmplCri) /*本轮采样尚未结束*/
        {
            gUartMgr->uartSmplTx[uart->uartCrntSmplType](uart);
        }
        else
        {
            timerStart(&uart->uartSmplLoopTmr, TidUartSmplLoop, 1000, WoReset);
        }
    }

    return;
}

void _____begin_of_uart_ack_expire_____(){}

void uartExprBlkAck(Timer *timer)
{
    Uart *uart;
    UartBlockCmdBuf *blkBuf;

    uart = Container(Uart, waitUartAckTmr, timer);
    uart->uartMsgFlowSeq++;
    blkBuf = uart->waitAckBlockBuf;
    if (0 != blkBuf->reTxCnt)
    {
        blkBuf->reTxCnt--;
        if (!ChainInList(&blkBuf->chain))
        {
            ChainInsertD(&uart->uartBlockList, &blkBuf->chain);
        }
    }
    else
    {
        rspUpCmmn(blkBuf->upUartMsgId, blkBuf->trayIdx, RspExpr);
        ChainDelSafeD(&blkBuf->chain);
        freeUartBlockBuf(blkBuf);
        uart->waitAckBlockBuf = NULL;
    }

    uartIdleNtfy(uart);
    return;
}

/*todo,重发*/
void uartExprCtrlAck(Timer *timer)
{
    Uart *uart;
    NdbdMcu *ndbdMcu;
    
    uart = Container(Uart, waitUartAckTmr, timer);
    uart->uartMsgFlowSeq++;
    ndbdMcu = &gDevMgr->ndbdMcu[uart->mcuIdxWaitCtrlAck];
    if (ndbdMcu->ctrlReTxCnt < 1)
    {
        NdbdMcuCtrl ctrlWaitAckBak;

        ndbdMcu->ctrlReTxCnt++;
        memcpy(&ctrlWaitAckBak, &ndbdMcu->ctrlWaitTx.ctrl, sizeof(NdbdMcuCtrl));
        memcpy(&ndbdMcu->ctrlWaitTx.ctrl, &ndbdMcu->ctrlWaitAck, sizeof(NdbdMcuCtrl));
        if (ctrlWaitAckBak.touchAct)
        {
            ndbdMcu->ctrlWaitTx.ctrl.touchAct = ctrlWaitAckBak.touchAct;
        }
        if (ctrlWaitAckBak.ndbdWarnDel)
        {
            ndbdMcu->ctrlWaitTx.ctrl.ndbdWarnDel = ctrlWaitAckBak.ndbdWarnDel;
        }
        if (ctrlWaitAckBak.fireDoorAct)
        {
            ndbdMcu->ctrlWaitTx.ctrl.fireDoorAct = ctrlWaitAckBak.fireDoorAct;
        }
        if (ctrlWaitAckBak.smokeAct)
        {
            ndbdMcu->ctrlWaitTx.ctrl.smokeAct = ctrlWaitAckBak.smokeAct;
        }
        if (ctrlWaitAckBak.slotFanAct)
        {
            ndbdMcu->ctrlWaitTx.ctrl.slotFanAct = ctrlWaitAckBak.slotFanAct;
        }
        if (ctrlWaitAckBak.vacumBrk)
        {
            ndbdMcu->ctrlWaitTx.ctrl.vacumBrk = ctrlWaitAckBak.vacumBrk;
        }
        if (ctrlWaitAckBak.fixtPower)
        {
            ndbdMcu->ctrlWaitTx.ctrl.fixtPower = ctrlWaitAckBak.fixtPower;
        }
        if (ctrlWaitAckBak.npRatio)
        {
            ndbdMcu->ctrlWaitTx.ctrl.npRatio = ctrlWaitAckBak.npRatio;
            ndbdMcu->ctrlWaitTx.ctrl.npVal = ctrlWaitAckBak.npVal;
        }

        uartNdbdMcuCtrlTxTry(ndbdMcu->ndbdMcuIdx);
    }
    else
    {
        ndbdMcu->ctrlReTxCnt = 0;
        uartIdleNtfy(uart);
    }
    return;
}

void uartExprSmplAck(Timer *timer)
{
    DevMgr *devMgr;
    Uart *uart;

    devMgr = gDevMgr;
    uart = Container(Uart, waitUartAckTmr, timer);
    if (UartTmprSmpl == uart->uartCrntSmplType)
    {
        TmprSmpl *tmprSmpl;
        u8 amount;
        u8 idx;
        u8 idxCri;

        tmprSmpl = &devMgr->tmprSmpl[uart->uartCrntSmplDevIdx];
        if (tmprSmpl->online)
        {
            tmprSmpl->smplExprCnt++;
            if (tmprSmpl->smplExprCnt > 3)
            {
                tmprSmpl->online = False;
                tmprSmpl->smplExprCnt = 0;
                tmprSmpl->onlineDelayCnt = 0;
                sendUpConnNtfy();
            }
        }

    #if 0  /*采样超时,保持原温度,否则会影响容量温度补偿计算*/
        if (tmprSmpl->genCellTmprIdx < devMgr->cellTmprAmt)
        {
            amount = devMgr->cellTmprAmt - tmprSmpl->genCellTmprIdx;
            if (amount > tmprSmpl->tmprAmt4Cell)
            {
                amount = tmprSmpl->tmprAmt4Cell;
            }
            for (idx=tmprSmpl->genCellTmprIdx,idxCri=idx+amount; idx<idxCri; idx++)
            {
                devMgr->genCellTmpr[idx] = 0;
            }
        }
        if (0 != tmprSmpl->tmprAmt4Loc)
        {
            if (tmprSmpl->genSlotTmprIdx < devMgr->slotTmprAmt)
            {
                amount = devMgr->slotTmprAmt - tmprSmpl->genSlotTmprIdx;
                if (amount > tmprSmpl->tmprAmt4Loc)
                {
                    amount = tmprSmpl->tmprAmt4Loc;
                }
                for (idx=tmprSmpl->genSlotTmprIdx,idxCri=idx+amount; idx<idxCri; idx++)
                {
                    devMgr->genSlotTmpr[idx] = 0;
                }
            }
        }
        if (0 != tmprSmpl->tmprAmt4Water)
        {
            if (tmprSmpl->genWaterTmprIdx < devMgr->waterTmprAmt)
            {
                amount = devMgr->waterTmprAmt - tmprSmpl->genWaterTmprIdx;
                if (amount > tmprSmpl->tmprAmt4Water)
                {
                    amount = tmprSmpl->tmprAmt4Water;
                }
                for (idx=tmprSmpl->genWaterTmprIdx,idxCri=idx+amount; idx<idxCri; idx++)
                {
                    devMgr->genSlotWaterTmpr[idx] = 0;
                }
            }
        }
    #endif
    }
    else
    {
        NdbdMcu *ndbdMcu;

        ndbdMcu = &devMgr->ndbdMcu[uart->uartCrntSmplDevIdx];
        devMgr->tray[ndbdMcu->ndbdMcuIdx].ndbdData.ndbdDataValid = False;
        if (ndbdMcu->online)
        {
            ndbdMcu->smplExprCnt++;
            if (ndbdMcu->smplExprCnt > 3)
            {
                ndbdMcu->online = False;
                ndbdMcu->smplExprCnt = 0;
                ndbdMcu->onlineDelayCnt = 0;
                devMgr->tray[ndbdMcu->ndbdMcuIdx].ndbdData.status[NdbdStaTouch] = 0;
                sendUpConnNtfy();
            }
        }
    }

    uart->uartMsgFlowSeq++;
    uartSmplDevUpdate(uart);
    uartIdleNtfy(uart);
    return;
}

void _____begin_of_uart_ack_recieve_____(){}

void uartMsgConnAck(Uart *uart, u8 actAddr, u8 *pld, u16 pldLen)
{
    UartCmmnConnAck *uartAck;

    uartAck = (UartCmmnConnAck *)pld;
    if (UartTmprSmpl == uartAck->devType) /*todo,检查通道数量和联机响应*/
    {
        TmprSmpl *tmprSmpl;
        TmprSmplConnAck *uartTmprSmplAck;

        tmprSmpl = &gDevMgr->tmprSmpl[uart->uartCrntSmplDevIdx];
        if (actAddr == tmprSmpl->actAddr)
        {
            uartTmprSmplAck = (TmprSmplConnAck *)pld;
            if (uartTmprSmplAck->isInApp)
            {
                tmprSmpl->smplExprCnt = 0;
                tmprSmpl->devVer = uartTmprSmplAck->devVer;
                tmprSmpl->onlineDelayCnt++;
                if (tmprSmpl->onlineDelayCnt > 0)
                {
                    tmprSmpl->online = True;
                    tmprSmpl->onlineDelayCnt = 0;
                    sendUpConnNtfy();
                }
            }
        }

        uartSmplDevUpdate(uart);
    }
    else if (UartNdbdMcu == uartAck->devType)
    {
        NdbdMcu *ndbdMcu;
        NdbdMcuConnAck *ndbdMcuConnAck;

        ndbdMcu = &gDevMgr->ndbdMcu[uart->uartCrntSmplDevIdx];
        if (actAddr==ndbdMcu->actAddr)
        {
            ndbdMcuConnAck = (NdbdMcuConnAck *)pld;
            if (ndbdMcuConnAck->isInApp && ndbdMcuConnAck->protoVer>=gNdbdMcuProtoVer)
            {
                ndbdMcu->smplExprCnt = 0;
                ndbdMcu->ctrlReTxCnt = 0;
                ndbdMcu->softVer = ndbdMcuConnAck->devVer;
                ndbdMcu->onlineDelayCnt++;
                if (ndbdMcu->onlineDelayCnt > 0)
                {
                    ndbdMcu->online = True;
                    ndbdMcu->onlineDelayCnt = 0;
                    sendUpConnNtfy();
                }
            }
        }

        uartSmplDevUpdate(uart);
    }
    else
    {
        UpFixtNtfyAck *upAck;
        UartBlockCmdBuf *blkBuf;
        u8 *buf;

        buf = sendUpBuf;
        upAck = (UpFixtNtfyAck *)(buf + sizeof(UpMsgHead));
        blkBuf = uart->waitAckBlockBuf;
        upAck->rspCode = uartAck->rspCode;
        upAck->trayIdx = blkBuf->trayIdx;
        upAck->fixtVer = uartAck->devVer;
        gUpItfCb->upMsgFlowSeq = blkBuf->upMsgFlowSeq;
        sendUpMsg(buf, sizeof(UpFixtNtfyAck), UpCmdIdFixtNtfy);
    }

    return;
}

void uartMsgCfgReadAck(Uart *uart, u8 addr, u8 *pld, u16 len)
{
    return;
}

void uartMsgCfgSetAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    return;
}

void uartMsgUpdSetupAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    return;
}

void uartMsgUpdDldAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    return;
}

void uartMsgUpdUpldAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    return;
}

void uartMsgUpdCnfmAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    return;
}

/*todo,目前按一次一库,需要扩展,例如一次多库或一库多次,温度类型识别等*/
/*库位温度,永远在二组*/
void uartMsgSmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *uartCmmnAck;
    DevMgr *devMgr;

    uartCmmnAck = (UartCmmnAck *)pld;
    devMgr = gDevMgr;
    if (UartTmprSmpl == uartCmmnAck->devType)
    {
        TmprSmplSmplAck *tmprSmplAck;
        TmprSmplData *tmprSmplData;
        TmprSmpl *tmprSmpl;
        u8 amount;

        tmprSmplAck = (TmprSmplSmplAck *)pld;
        tmprSmpl = &devMgr->tmprSmpl[uart->uartStaAddr2DevIdx[UartTmprSmpl][actAddr]];
        tmprSmpl->smplExprCnt = 0;
        tmprSmplData = tmprSmplAck->tmprSmplData;
        devMgr->tmprSmplTsSec = sysTimeSecGet();
        if (tmprSmpl->genCellTmprIdx < devMgr->cellTmprAmt)
        {
            amount = devMgr->cellTmprAmt - tmprSmpl->genCellTmprIdx;
            if (amount > tmprSmplData->tmprSmplChnAmt)
            {
                amount = tmprSmplData->tmprSmplChnAmt;
            }
            mem2Copy(&devMgr->genCellTmpr[tmprSmpl->genCellTmprIdx], tmprSmplData->tmprVal, amount*2);
        }

        /*以下适用于水温后面跟库温的情况,,todo,,通用算法还待调整*/
        if (2 == tmprSmplAck->tmprSmplGrpAmt)
        {
            u8 slotTmprOfst;

            tmprSmplData = (TmprSmplData *)(tmprSmplData->tmprVal+Align16(tmprSmplData->tmprSmplChnAmt));
            slotTmprOfst = tmprSmpl->tmprAmt4Water;
            if (0 != tmprSmpl->tmprAmt4Water)
            {
                if (tmprSmpl->genWaterTmprIdx < devMgr->waterTmprAmt)
                {
                    amount = devMgr->waterTmprAmt - tmprSmpl->genWaterTmprIdx;
                    if (amount > tmprSmpl->tmprAmt4Water)
                    {
                        amount = tmprSmpl->tmprAmt4Water;
                    }
                    mem2Copy(&devMgr->genSlotWaterTmpr[tmprSmpl->genWaterTmprIdx], tmprSmplData->tmprVal, amount*2);
                }
            }

            if (tmprSmpl->genSlotTmprIdx < devMgr->slotTmprAmt)
            {
                amount = devMgr->slotTmprAmt - tmprSmpl->genSlotTmprIdx;
                if (amount > tmprSmpl->tmprAmt4Loc)
                {
                    amount = tmprSmpl->tmprAmt4Loc;
                }
                mem2Copy(&devMgr->genSlotTmpr[tmprSmpl->genSlotTmprIdx], tmprSmplData->tmprVal+slotTmprOfst, amount*2);
            }
        }
    }
    else /*UartNdbdMcu==uartCmmnAck->devType*/
    {
        Tray *tray;
        NdbdData *ndbdData;
        NdbdMcuSmplAck *smplAck;
        NdbdMcu *ndbdMcu;
        b8 trayBeTouchPre;
        b8 ndbdBeAutoPre;

        ndbdMcu = &gDevMgr->ndbdMcu[uart->uartCrntSmplDevIdx];
        ndbdMcu->smplExprCnt = 0;

        tray = &devMgr->tray[uart->uartStaAddr2DevIdx[UartNdbdMcu][actAddr]];
        ndbdData = &tray->ndbdData;
        smplAck = (NdbdMcuSmplAck *)pld;
        ndbdBeAutoPre = 0==ndbdData->status[NdbdStaWorkMode] ? True : False;
        trayBeTouchPre = 1==ndbdData->status[NdbdStaTouch] ? True : False;

        ndbdData->status[NdbdStaEnter] = smplAck->trayBeEnter;
        ndbdData->status[NdbdStaTouch] = smplAck->trayStaTouch;
        ndbdData->status[NdbdSenFwdEnter] = BitVal(smplAck->sensorBitMap, 0);
        ndbdData->status[NdbdSenBackEnter] = BitVal(smplAck->sensorBitMap, 1);
        ndbdData->status[NdbdSenFowTouch] = BitVal(smplAck->sensorBitMap, 2);
        ndbdData->status[NdbdSenBackTouch] = BitVal(smplAck->sensorBitMap, 3);
        ndbdData->status[NdbdStaWorkMode] = BitVal(smplAck->sensorBitMap, 4);
        ndbdData->status[NdbdSenSwValve] = BitVal(smplAck->sensorBitMap, 5);
        ndbdData->status[NdbdSenBrkVacum] = BitVal(smplAck->sensorBitMap, 6);
        ndbdData->status[NdbdSenFireDoorUp] = BitVal(smplAck->sensorBitMap, 7);
        ndbdData->status[NdbdSenFireDoorDown] = BitVal(smplAck->sensorBitMap, 8);
        ndbdData->status[NdbdSenFwdDoorMsw] = BitVal(smplAck->sensorBitMap, 9);
        ndbdData->status[NdbdSenBackDoor] = BitVal(smplAck->sensorBitMap, 10);
        ndbdData->status[NdbdSenCylinderUp] = BitVal(smplAck->sensorBitMap, 11);
        ndbdData->status[NdbdSenCylinderDown] = BitVal(smplAck->sensorBitMap, 12);
        if (len > sizeof(NdbdMcuSmplAck))
        {
            NdbdMcuTlv *tlv;
            u8 cnt;

            len -= sizeof(NdbdMcuSmplAck);
            tlv = (NdbdMcuTlv *)smplAck->tlvData;
            for (cnt=0; cnt<2; cnt++)
            {
                if (len >= sizeof(NdbdMcuTlv)+sizeof(s16))
                {
                    if (0 == tlv->dataType)  /*负压*/
                    {
                        ndbdData->status[NdbdSenNpVal] = tlv->data[0];
                    }
                    else if (1 == tlv->dataType)  /*比例阀开度*/
                    {
                        ndbdData->status[NdbdSenRatioVal] = tlv->data[0];
                    }

                    len -= sizeof(NdbdMcuTlv)+sizeof(s16);
                    tlv = (NdbdMcuTlv *)((u8 *)tlv + sizeof(NdbdMcuTlv)+sizeof(s16));
                }
            }
        }

        if (!trayBeTouchPre && 1==ndbdData->status[NdbdStaTouch])  /*脱开变压合*/
        {
            timerStart(&tray->protEnaTmr, TidTrayProtEna, 2000, WiReset);
            trayNpReset(tray->trayIdx);  /*todo,临时防呆,以后要去掉*/
        }
        else if (trayBeTouchPre && 1!=ndbdData->status[NdbdStaTouch])  /*压合变脱开*/
        {
            tray->protDisable |= ProtDisableTouch;
            TimerStop(&tray->protEnaTmr);
            if (0 != tray->npMgr.closeBrkDelaySecT08)
            {
                timerStart(&tray->npSwRstDelayTmr, TidNpSwRstDelay,
                    tray->npMgr.closeBrkDelaySecT08*1000, WiReset);
            }
            trayStopByProt(tray, Cc0NdbdBrkAbnml);
        }
        
        if (ndbdBeAutoPre && 1==ndbdData->status[NdbdStaWorkMode]) /*自动打维护*/
        {
            trayStopByProt(tray, Cc0NdbdMntn);
        }
        
        ndbdData->warn[NdbdWarnScram] = BitVal(smplAck->abnmlBitMap, 0);
        ndbdData->warn[NdbdWarnGas] = BitVal(smplAck->abnmlBitMap, 1);
        ndbdData->warn[NdbdWarnSmoke] = BitVal(smplAck->abnmlBitMap, 11);
        ndbdData->warn[NdbdWarnSmoke] = ndbdData->warn[NdbdWarnSmoke] << 1;
        ndbdData->warn[NdbdWarnSmoke] |= BitVal(smplAck->abnmlBitMap, 12);
        ndbdData->warn[NdbdWarnCo] = BitVal(smplAck->abnmlBitMap, 2);
        ndbdData->warn[NdbdWarnCylinder] = BitVal(smplAck->abnmlBitMap, 18);
        ndbdData->warn[NdbdWarnFan] = BitVal(smplAck->abnmlBitMap, 3);
        ndbdData->warn[NdbdWarnFireDoor] = BitVal(smplAck->abnmlBitMap, 4);
        ndbdData->warn[NdbdWarnSlotDoorRast] = BitVal(smplAck->abnmlBitMap, 5);
        ndbdData->warn[NdbdWarnTray] = BitVal(smplAck->abnmlBitMap, 6);
        ndbdData->warn[NdbdWarnFwdDoorMsw] = BitVal(smplAck->abnmlBitMap, 7);
        ndbdData->warn[NdbdWarnBackDoor] = BitVal(smplAck->abnmlBitMap, 8);
        ndbdData->warn[NdbdWarnRatio] = BitVal(smplAck->abnmlBitMap, 9);
        ndbdData->warn[NdbdWarnScapeGoat] = BitVal(smplAck->abnmlBitMap, 10);
        
        ndbdData->ndbdDataValid = True;
        ndbdData->ndbdSmplTsSec = sysTimeSecGet();
        trayNpReachChk(tray);
    }

    uartSmplDevUpdate(uart);
    return;
}

void ndbdMcuCtrlRst(NdbdMcuCtrl *ndbdMcuCtrl)
{
    ndbdMcuCtrl->touchAct = 0;
    ndbdMcuCtrl->ndbdWarnDel = 0;
    ndbdMcuCtrl->fireDoorAct = 0;
    ndbdMcuCtrl->smokeAct = 0;
    ndbdMcuCtrl->slotFanAct = 0;
    ndbdMcuCtrl->npHigh = 0;
    ndbdMcuCtrl->npLow = 0;
    ndbdMcuCtrl->vacumBrk = 0;
    ndbdMcuCtrl->npRatio = 0;
    ndbdMcuCtrl->fixtPower = 0;
    return;
}

/*todo,可能要加返回错误的处理,暂时不做*/
void uartMsgNdbdMcuCtrlAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    return;
}

void uartMsgNdbdMcuDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgNdbdMcuCri)
    {
        gUartMgr->uartNdbdMcuMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtCmmnAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *uartAck;
    UartBlockCmdBuf *blockBuf;

    uartAck = (UartCmmnAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    rspUpCmmn(blockBuf->upUartMsgId, blockBuf->trayIdx, uartAck->rspCode);
    return;
}

void uartMsgFixtPrecSmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtPrecSmplAck *uartAck;
    UpFixtPrecSmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtPrecSmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtPrecSmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->voltage = uartAck->voltage;
    upAck->current = uartAck->current;
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtPrecSmplAck), UpCmdIdFixtPrecSmpl);
    return;
}

void uartMsgFixtPrecDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtPrecCri)
    {
        gUartMgr->uartFixtPrecMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtTmprSmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtTmprSmplAck *uartAck;
    UpFixtTmprSmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtTmprSmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtTmprSmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->chnAmt = uartAck->chnAmt;
    if (RspOk == uartAck->rspCode)
    {
        mem2Copy(upAck->tmprVal, uartAck->tmprVal, uartAck->chnAmt*2);
    }
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtTmprSmplAck)+Align16(upAck->chnAmt)*2, UpCmdIdFixtTmprSmpl);
    return;
}

void uartMsgFixtTmprDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtTmprCri)
    {
        gUartMgr->uartFixtTmprMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtCleanDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtCleanCri)
    {
        gUartMgr->uartFixtCleanMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtGasSmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtGasSmplAck *uartAck;
    UpFixtGasSmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtGasSmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtGasSmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->chnAmt = uartAck->chnAmt;
    if (RspOk == uartAck->rspCode)
    {
        mem2Copy(upAck->npVal, uartAck->npVal, uartAck->chnAmt*2);
    }
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtGasSmplAck)+Align16(upAck->chnAmt)*2, UpCmdIdFixtGasSmpl);
    return;
}

void uartMsgFixtGasDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtGasCri)
    {
        gUartMgr->uartFixtGasMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtFlowSmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtFlowSmplAck *uartAck;
    UpFixtFlowSmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtFlowSmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtFlowSmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->chnAmt = uartAck->chnAmt;
    if (RspOk == uartAck->rspCode)
    {
        mem2Copy(upAck->flowVal, uartAck->flowVal, uartAck->chnAmt*2);
    }
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtFlowSmplAck)+Align16(upAck->chnAmt)*2, UpCmdIdFixtFlowSmpl);
    return;
}

void uartMsgFixtFlowDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtFlowCri)
    {
        gUartMgr->uartFixtFlowMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtSuckInSmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtSuckInSmplAck *uartAck;
    UpFixtSuckInSmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtSuckInSmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtSuckInSmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->hasRemain = uartAck->hasRemain;
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtSuckInSmplAck), UpCmdIdFixtSuctInSmpl);
    return;
}

void uartMsgFixtSuctInDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtSuckInCri)
    {
        gUartMgr->uartFixtSuckInMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtSuckOutSmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtSuckOutSmplAck *uartAck;
    UpFixtSuckOutSmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtSuckOutSmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtSuckOutSmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->hasRemain = uartAck->hasRemain;
    upAck->state = uartAck->state;
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtSuckOutSmplAck), UpCmdIdFixtSuctOutSmpl);
    return;
}

void uartMsgFixtSuctOutDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtSuckOutCri)
    {
        gUartMgr->uartFixtSuckOutMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtLocatSmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtLocatSmplAck *uartAck;
    UpFixtLocatSmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtLocatSmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtLocatSmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->unlockSucc = uartAck->unlockSucc;
    upAck->locatPos = uartAck->locatPos;
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtLocatSmplAck), UpCmdIdFixtLocatSmpl);
    return;
}

void uartMsgFixtLocatDisp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtLocatCri)
    {
        gUartMgr->uartFixtLocatMsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtSuckIn2SmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtSuckIn2SmplAck *uartAck;
    UpFixtSuckIn2SmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtSuckIn2SmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtSuckIn2SmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->hasRemain = uartAck->hasRemain;
    upAck->topUpPos = uartAck->topUpPos;
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtSuckIn2SmplAck), UpCmdIdFixtSuctIn2Smpl);
    return;
}

void uartMsgFixtSuctIn2Disp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtSuckIn2Cri)
    {
        gUartMgr->uartFixtSuckIn2MsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtSuckOut2SmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtSuckOut2SmplAck *uartAck;
    UpFixtSuckOut2SmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtSuckOut2SmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtSuckOut2SmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->hasRemain = uartAck->hasRemain;
    upAck->topUpPos = uartAck->topUpPos;
    upAck->state = uartAck->state;
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtSuckOut2SmplAck), UpCmdIdFixtSuctOut2Smpl);
    return;
}

void uartMsgFixtSuctOut2Disp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtSuckOut2Cri)
    {
        gUartMgr->uartFixtSuckOut2MsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartMsgFixtLocat2SmplAck(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartFixtLocat2SmplAck *uartAck;
    UpFixtLocat2SmplAck *upAck;
    UartBlockCmdBuf *blockBuf;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpFixtLocat2SmplAck *)(buf + sizeof(UpMsgHead));
    uartAck = (UartFixtLocat2SmplAck *)pld;
    blockBuf = uart->waitAckBlockBuf;
    upAck->rspCode = uartAck->rspCode;
    upAck->trayIdx = blockBuf->trayIdx;
    upAck->unlockSucc = uartAck->unlockSucc;
    upAck->locatPos = uartAck->locatPos;
    upAck->topUpPos = uartAck->topUpPos;
    gUpItfCb->upMsgFlowSeq = blockBuf->upMsgFlowSeq;
    sendUpMsg(buf, sizeof(UpFixtLocat2SmplAck), UpCmdIdFixtLocat2Smpl);
    return;
}

void uartMsgFixtLocat2Disp(Uart *uart, u8 actAddr, u8 *pld, u16 len)
{
    UartCmmnAck *tmpAck;
    u8eUartMsgId msgId;

    tmpAck = (UartCmmnAck *)pld;
    msgId = tmpAck->msgId - UartMsgIdSpecBase;
    if (msgId < UartMsgFixtLocat2Cri)
    {
        gUartMgr->uartFixtLocat2MsgProc[msgId](uart, actAddr, pld, len);
    }
    return;
}

void uartRxMsg(u8 port, u8 *msg, u16 len)
{
    Uart *uart;
    UartMsgHead *uartHead;
    UartCmmnAck *tmpAck;
    u8 actAddr;

    uart = &gDevMgr->uart[port];
    uartHead = (UartMsgHead *)msg;
    tmpAck = (UartCmmnAck *)uartHead->payload;
    if (len != uartHead->pldLen+sizeof(UartMsgHead)
        || uartHead->flowSeq!=uart->uartMsgFlowSeq || tmpAck->devType>=UartDevTypeCri)
    {
        return;
    }

    uart->uartMsgFlowSeq++;
    if (uartHead->devAddr < UartAddrBaseFixtTmpr)
    {
        actAddr = uartHead->devAddr & UartStaActAddrMask;
    }
    else
    {
        actAddr = uartHead->devAddr & UartDynActAddrMask;
    }

    if (tmpAck->msgId < UartMsgCmmnCri)
    {
        gUartMgr->uartCmmnMsgProc[tmpAck->msgId](uart, actAddr, uartHead->payload, uartHead->pldLen);
    }
    else
    {
        gUartMgr->uartSpecMsgDisp[tmpAck->devType](uart, actAddr, uartHead->payload, uartHead->pldLen);
    }

    if (NULL != uart->waitAckBlockBuf)
    {
        ChainDelSafeD(&uart->waitAckBlockBuf->chain);
        freeUartBlockBuf(uart->waitAckBlockBuf);
        uart->waitAckBlockBuf = NULL;
    }

    TimerStop(&uart->waitUartAckTmr);  /*先处理, 后停定时器*/
    uartIdleNtfy(uart);
}

void uartInit()
{
    UartMgr *mgr;
    UartBlockCmdBuf *blockBuf;
    Uart *uart;
    u8 idx;

#ifdef DebugVersion
#else
    utp_init(0, uartRxMsg);
    utp_init(1, uartRxMsg);
#endif

    gUartSendBuf = sysMemAlloc(512);

    mgr = gUartMgr = sysMemAlloc(sizeof(UartMgr));
    ListInitD(&mgr->blockBufList);
    blockBuf = sysMemAlloc(sizeof(UartBlockCmdBuf) * UartBufAmt);
    for (idx=0; idx<UartBufAmt; idx++,blockBuf++)
    {
        ChainInsertD(&mgr->blockBufList, &blockBuf->chain);
    }

    mgr->uartAddrBase[UartTmprSmpl] = UartAddrBaseTmprSmpl;
    mgr->uartAddrBase[UartTmprCtrl] = UartAddrBaseTmprCtrl;
    mgr->uartAddrBase[UartNdbdMcu] = UartAddrBaseNdbdCtrl;
    mgr->uartAddrBase[UartFixtTmpr] = UartAddrBaseFixtTmpr;
    mgr->uartAddrBase[UartFixtClean] = UartAddrBaseFixtClean;
    mgr->uartAddrBase[UartFixtGas] = UartAddrBaseFixtGas;
    mgr->uartAddrBase[UartFixtPrecParal] = UartAddrBaseFixtPrecParal;
    mgr->uartAddrBase[UartFixtPrecSeries] = UartAddrBaseFixtPrecSeries;
    mgr->uartAddrBase[UartFixtSuctIn] = UartAddrBaseFixtSuctIn;
    mgr->uartAddrBase[UartFixtSuctOut] = UartAddrBaseFixtSuctOut;
    mgr->uartAddrBase[UartFixtLocat] = UartAddrBaseFixtLocat;
    mgr->uartAddrBase[UartFixtFlow] = UartAddrBaseFixtFlow;
    mgr->uartAddrBase[UartFixtSuctIn2] = UartAddrBaseFixtSuctIn2;
    mgr->uartAddrBase[UartFixtSuctOut2] = UartAddrBaseFixtSuctOut2;
    mgr->uartAddrBase[UartFixtLocat2] = UartAddrBaseFixtLocat2;

    mgr->upFixt2UartType[UpFixtTypePrecSeries] = UartFixtPrecSeries;
    mgr->upFixt2UartType[UpFixtTypeTmpr] = UartFixtTmpr;
    mgr->upFixt2UartType[UpFixtTypeClean] = UartFixtClean;
    mgr->upFixt2UartType[UpFixtTypeGas] = UartFixtGas;
    mgr->upFixt2UartType[UpFixtTypeFlow] = UartFixtFlow;
    mgr->upFixt2UartType[UpFixtTypeSuctIn] = UartFixtSuctIn;
    mgr->upFixt2UartType[UpFixtTypeSuctOut] = UartFixtSuctOut;
    mgr->upFixt2UartType[UpFixtTypeLocat] = UartFixtLocat;
    mgr->upFixt2UartType[UpFixtTypePrecParal] = UartFixtPrecParal;
    mgr->upFixt2UartType[UpFixtTypeSuctIn2] = UartFixtSuctIn2;
    mgr->upFixt2UartType[UpFixtTypeSuctOut2] = UartFixtSuctOut2;
    mgr->upFixt2UartType[UpFixtTypeLocat2] = UartFixtLocat2;

    for (idx=UartTmprSmpl; idx<UartNeedSmplCri; idx++)
    {
        mgr->uartSmplTx[idx] = (UartSmplTx)uartMsgIgnore;
    }
    mgr->uartSmplTx[UartTmprSmpl] = uartSmplTmprTx;
    mgr->uartSmplTx[UartNdbdMcu] = uartSmplNdbdMcu;

    for (idx=UartMsgConn; idx<UartMsgCmmnCri; idx++)
    {
        mgr->uartCmmnMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartCmmnMsgProc[UartMsgConn] = uartMsgConnAck;
    mgr->uartCmmnMsgProc[UartMsgCfgRead] = uartMsgCfgReadAck;
    mgr->uartCmmnMsgProc[UartMsgCfgSet] = uartMsgCfgSetAck;
    mgr->uartCmmnMsgProc[UartMsgUpdSetup] = uartMsgUpdSetupAck;
    mgr->uartCmmnMsgProc[UartMsgUpdDld] = uartMsgUpdDldAck;
    mgr->uartCmmnMsgProc[UartMsgUpdUpld] = uartMsgUpdUpldAck;
    mgr->uartCmmnMsgProc[UartMsgUpdCnfm] = uartMsgUpdCnfmAck;
    mgr->uartCmmnMsgProc[UartMsgSmpl] = uartMsgSmplAck;

    for (idx=UartTmprSmpl; idx<UartDevTypeCri; idx++)
    {
        mgr->uartSpecMsgDisp[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartSpecMsgDisp[UartNdbdMcu] = uartMsgNdbdMcuDisp;
    mgr->uartSpecMsgDisp[UartFixtPrecSeries] = uartMsgFixtPrecDisp;
    mgr->uartSpecMsgDisp[UartFixtPrecParal] = uartMsgFixtPrecDisp;
    mgr->uartSpecMsgDisp[UartFixtTmpr] = uartMsgFixtTmprDisp;
    mgr->uartSpecMsgDisp[UartFixtClean] = uartMsgFixtCleanDisp;
    mgr->uartSpecMsgDisp[UartFixtGas] = uartMsgFixtGasDisp;
    mgr->uartSpecMsgDisp[UartFixtFlow] = uartMsgFixtFlowDisp;
    mgr->uartSpecMsgDisp[UartFixtSuctIn] = uartMsgFixtSuctInDisp;
    mgr->uartSpecMsgDisp[UartFixtSuctOut] = uartMsgFixtSuctOutDisp;
    mgr->uartSpecMsgDisp[UartFixtLocat] = uartMsgFixtLocatDisp;
    mgr->uartSpecMsgDisp[UartFixtSuctIn2] = uartMsgFixtSuctIn2Disp;
    mgr->uartSpecMsgDisp[UartFixtSuctOut2] = uartMsgFixtSuctOut2Disp;
    mgr->uartSpecMsgDisp[UartFixtLocat2] = uartMsgFixtLocat2Disp;

    for (idx=UartMsgNdbdMcuCtrl; idx<UartMsgNdbdMcuCri; idx++)
    {
        mgr->uartNdbdMcuMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartNdbdMcuMsgProc[UartMsgNdbdMcuCtrl] = uartMsgNdbdMcuCtrlAck;

    for (idx=UartMsgFixtPrecChnSw; idx<UartMsgFixtPrecCri; idx++)
    {
        mgr->uartFixtPrecMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtPrecMsgProc[UartMsgFixtPrecChnSw] = uartMsgFixtCmmnAck;
    mgr->uartFixtPrecMsgProc[UartMsgFixtPrecSmpl] = uartMsgFixtPrecSmplAck;
    mgr->uartFixtPrecMsgProc[UartMsgFixtPrecOut] = uartMsgFixtCmmnAck;

    for (idx=UartMsgFixtTmprHeat; idx<UartMsgFixtTmprCri; idx++)
    {
        mgr->uartFixtTmprMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtTmprMsgProc[UartMsgFixtTmprHeat] = uartMsgFixtCmmnAck;
    mgr->uartFixtTmprMsgProc[UartMsgFixtTmprSmpl] = uartMsgFixtTmprSmplAck;

    for (idx=UartMsgFixtCleanAct; idx<UartMsgFixtCleanCri; idx++)
    {
        mgr->uartFixtCleanMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtCleanMsgProc[UartMsgFixtCleanAct] = uartMsgFixtCmmnAck;

    for (idx=UartMsgFixtGasSmpl; idx<UartMsgFixtGasCri; idx++)
    {
        mgr->uartFixtGasMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtGasMsgProc[UartMsgFixtGasSmpl] = uartMsgFixtGasSmplAck;

    for (idx=UartMsgFixtFlowSmpl; idx<UartMsgFixtFlowCri; idx++)
    {
        mgr->uartFixtFlowMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtFlowMsgProc[UartMsgFixtFlowSmpl] = uartMsgFixtFlowSmplAck;

    for (idx=UartMsgFixtSuckInSmpl; idx<UartMsgFixtSuckInCri; idx++)
    {
        mgr->uartFixtSuckInMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtSuckInMsgProc[UartMsgFixtSuckInSmpl] = uartMsgFixtSuckInSmplAck;

    for (idx=UartMsgFixtSuckOutSmpl; idx<UartMsgFixtSuckOutCri; idx++)
    {
        mgr->uartFixtSuckOutMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtSuckOutMsgProc[UartMsgFixtSuckOutSmpl] = uartMsgFixtSuckOutSmplAck;
    mgr->uartFixtSuckOutMsgProc[UartMsgFixtSuckOutAct] = uartMsgFixtCmmnAck;

    for (idx=UartMsgFixtLocatSmpl; idx<UartMsgFixtLocatCri; idx++)
    {
        mgr->uartFixtLocatMsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtLocatMsgProc[UartMsgFixtLocatSmpl] = uartMsgFixtLocatSmplAck;
    mgr->uartFixtLocatMsgProc[UartMsgFixtLocatAct] = uartMsgFixtCmmnAck;

    for (idx=UartMsgFixtSuckIn2Smpl; idx<UartMsgFixtSuckIn2Cri; idx++)
    {
        mgr->uartFixtSuckIn2MsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtSuckIn2MsgProc[UartMsgFixtSuckIn2Smpl] = uartMsgFixtSuckIn2SmplAck;
    mgr->uartFixtSuckIn2MsgProc[UartMsgFixtSuckIn2Act] = uartMsgFixtCmmnAck;

    for (idx=UartMsgFixtSuckOut2Smpl; idx<UartMsgFixtSuckOut2Cri; idx++)
    {
        mgr->uartFixtSuckOut2MsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtSuckOut2MsgProc[UartMsgFixtSuckOut2Smpl] = uartMsgFixtSuckOut2SmplAck;
    mgr->uartFixtSuckOut2MsgProc[UartMsgFixtSuckOut2Act] = uartMsgFixtCmmnAck;

    for (idx=UartMsgFixtLocat2Smpl; idx<UartMsgFixtLocat2Cri; idx++)
    {
        mgr->uartFixtLocat2MsgProc[idx] = (UartMsgProc)uartMsgIgnore;
    }
    mgr->uartFixtLocat2MsgProc[UartMsgFixtLocat2Smpl] = uartMsgFixtLocat2SmplAck;
    mgr->uartFixtLocat2MsgProc[UartMsgFixtLocat2Act] = uartMsgFixtCmmnAck;

    for (uart=gDevMgr->uart; uart<&gDevMgr->uart[gDevMgr->uartAmt]; uart++)
    {
        for (idx=UartTmprSmpl; idx<UartNeedSmplCri; idx++)
        {
            if (0 != uart->uartSmplDevAmt[idx])
            {
                timerStart(&uart->uartSmplLoopTmr, TidUartSmplLoop, 1000, WiReset);
                break;
            }
        }
    }
    return;
}

#endif
