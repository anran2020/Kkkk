
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include "basic.h"
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
    uartHead->flowSeq = uart->uartMsgFlowSeq;
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

        if (0 != tmprSmpl->tmprAmt4Loc)
        {
            tmprSmplInd->tmprSmplChnIdx = tmprSmpl->tmprBase4Loc;
            tmprSmplInd->tmprSmplChnAmt = tmprSmpl->tmprAmt4Loc;
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

    uart = Container(Uart, waitUartAckTmr, timer);
    uart->uartMsgFlowSeq++;
    uartIdleNtfy(uart);
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
                sendUpConnNtfy();
            }
        }

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
    }

    uart->uartMsgFlowSeq++;
    uartSmplDevUpdate(uart);
    uartIdleNtfy(uart);
    return;
}

void _____begin_of_uart_ack_recieve_____(){}

/*todo,补充非工装*/
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
                tmprSmpl->online = True;
                tmprSmpl->smplExprCnt = 0;
                sendUpConnNtfy();
            }
            tmprSmpl->devVer = uartTmprSmplAck->devVer;
        }

        uartSmplDevUpdate(uart);
    }
    else if (UartNdbdCtrl == uartAck->devType)
    {
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
    TmprSmplSmplAck *tmprSmplAck;
    TmprSmplData *tmprSmplData;
    DevMgr *devMgr;
    u8 amount;

    tmprSmplAck = (TmprSmplSmplAck *)pld;
    devMgr = gDevMgr;
    if (UartTmprSmpl == tmprSmplAck->devType)
    {
        TmprSmpl *tmprSmpl;

        uartSmplDevUpdate(uart);
        tmprSmpl = &devMgr->tmprSmpl[uart->uartStaAddr2DevIdx[UartTmprSmpl][actAddr]];
        tmprSmpl->smplExprCnt = 0;
        tmprSmplData = tmprSmplAck->tmprSmplData;
        if (tmprSmpl->genCellTmprIdx < devMgr->cellTmprAmt)
        {
            amount = devMgr->cellTmprAmt - tmprSmpl->genCellTmprIdx;
            if (amount > tmprSmplData->tmprSmplChnAmt)
            {
                amount = tmprSmplData->tmprSmplChnAmt;
            }
            mem2Copy(&devMgr->genCellTmpr[tmprSmpl->genCellTmprIdx], tmprSmplData->tmprVal, amount*2);
        }
        if (2 == tmprSmplAck->tmprSmplGrpAmt)
        {
            tmprSmplData = (TmprSmplData *)(tmprSmplData->tmprVal+Align16(tmprSmplData->tmprSmplChnAmt));
            if (tmprSmpl->genSlotTmprIdx < devMgr->slotTmprAmt)
            {
                amount = devMgr->slotTmprAmt - tmprSmpl->genSlotTmprIdx;
                if (amount > tmprSmplData->tmprSmplChnAmt)
                {
                    amount = tmprSmplData->tmprSmplChnAmt;
                }
                mem2Copy(&devMgr->genSlotTmpr[tmprSmpl->genSlotTmprIdx], tmprSmplData->tmprVal, amount*2);
            }
        }
    }
    else /*UartNdbdCtrl*/
    {
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
    mgr->uartAddrBase[UartNdbdCtrl] = UartAddrBaseNdbdCtrl;
    mgr->uartAddrBase[UartFixtTmpr] = UartAddrBaseFixtTmpr;
    mgr->uartAddrBase[UartFixtClean] = UartAddrBaseFixtClean;
    mgr->uartAddrBase[UartFixtGas] = UartAddrBaseFixtGas;
    mgr->uartAddrBase[UartFixtPrecParal] = UartAddrBaseFixtPrecParal;
    mgr->uartAddrBase[UartFixtPrecSeries] = UartAddrBaseFixtPrecSeries;
    mgr->uartAddrBase[UartFixtSuctIn] = UartAddrBaseFixtSuctIn;
    mgr->uartAddrBase[UartFixtSuctOut] = UartAddrBaseFixtSuctOut;
    mgr->uartAddrBase[UartFixtLocat] = UartAddrBaseFixtLocat;
    mgr->uartAddrBase[UartFixtFlow] = UartAddrBaseFixtFlow;

    mgr->upFixt2UartType[UpFixtTypePrecSeries] = UartFixtPrecSeries;
    mgr->upFixt2UartType[UpFixtTypeTmpr] = UartFixtTmpr;
    mgr->upFixt2UartType[UpFixtTypeClean] = UartFixtClean;
    mgr->upFixt2UartType[UpFixtTypeGas] = UartFixtGas;
    mgr->upFixt2UartType[UpFixtTypeFlow] = UartFixtFlow;
    mgr->upFixt2UartType[UpFixtTypeSuctIn] = UartFixtSuctIn;
    mgr->upFixt2UartType[UpFixtTypeSuctOut] = UartFixtSuctOut;
    mgr->upFixt2UartType[UpFixtTypeLocat] = UartFixtLocat;
    mgr->upFixt2UartType[UpFixtTypePrecParal] = UartFixtPrecParal;

    for (idx=UartTmprSmpl; idx<UartNeedSmplCri; idx++)
    {
        mgr->uartSmplTx[idx] = (UartSmplTx)uartMsgIgnore;
    }
    mgr->uartSmplTx[UartTmprSmpl] = uartSmplTmprTx;

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
    mgr->uartSpecMsgDisp[UartFixtPrecSeries] = uartMsgFixtPrecDisp;
    mgr->uartSpecMsgDisp[UartFixtPrecParal] = uartMsgFixtPrecDisp;
    mgr->uartSpecMsgDisp[UartFixtTmpr] = uartMsgFixtTmprDisp;
    mgr->uartSpecMsgDisp[UartFixtClean] = uartMsgFixtCleanDisp;
    mgr->uartSpecMsgDisp[UartFixtGas] = uartMsgFixtGasDisp;
    mgr->uartSpecMsgDisp[UartFixtFlow] = uartMsgFixtFlowDisp;
    mgr->uartSpecMsgDisp[UartFixtSuctIn] = uartMsgFixtSuctInDisp;
    mgr->uartSpecMsgDisp[UartFixtSuctOut] = uartMsgFixtSuctOutDisp;
    mgr->uartSpecMsgDisp[UartFixtLocat] = uartMsgFixtLocatDisp;

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

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译
