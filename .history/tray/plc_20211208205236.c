
#include <string.h>

#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"
#include "cause.h"
#include "entry.h"
#include "plc.h"
#include "host.h"
#include "u_setting.h"
#ifdef DebugVersion
#else
#include "ntp_w5500.h"
#include "pb.h"
#endif

u8 *gPlcSendBuf;


/*临时配置,以后改为配置文件*/
u8 gPlcAmt = 1;
#if 1
u8 gPlcIp[] = { 192, 168, 0, 57 };
#else
u8 gPlcIp[] = { 192, 168, 1, 1 };
#endif
u16 gPlcPort = 502;
u8 gPlcMapTrayNum=2;//plc映射的托盘数
u8 gPlcTrayIdx[4] = { 0,1 };
u16 gPlcTrayAddrBase[4] = { 0,150 };
u8 gPlcReadAmtMax = 120;

PlcMgr *gPlcMgr;


//加载配置信息
void plc_setting_load(void )
{
    u8 i,addr;
    //判断配置文件是否存在
    if (!setting_in_being())
    {
        return ;
    }
    gPlcAmt=mySetting->plcNum;
    //gPlcTrayIdx[] = { 0,1 };
    gPlcMapTrayNum=mySetting->plcMapTrayNum;
    addr=mySetting->plcMapTrayStartAddr;
    for (i = 0; i < gPlcMapTrayNum; i++)
    {
       gPlcTrayIdx[i]=addr++;
       gPlcTrayAddrBase[i]=mySetting->plcReadBaseAddr[i];
    }
    gPlcTrayAddrBase[0]=
    gPlcTrayAddrBase[1]=mySetting->plcReadEndAddr;
    gPlcReadAmtMax=mySetting->plcReadMaxAddr;

    gPlcPort=mySetting->plcPort;

    for (i = 0; i < 4; i++)
    {
        gPlcIp[i]=mySetting->plcip[i];
    }
}

void *plcBlkBufAlloc(Plc *plc)
{
    PlcCmdBlkBuf *blkBuf;
    ChainD *chain;

    if (ListIsEmpty(&plc->plcBlkListIdle))
    {
        return NULL;
    }

    chain = plc->plcBlkListIdle.next;
    ChainDelSafeD(chain);
    blkBuf = Container(PlcCmdBlkBuf, chain, chain);
    return blkBuf;
}

void plcBlkBufFree(Plc *plc)
{
    ListD *list;
    ChainD *chain;

    if (NULL != plc->plcBlkBufWaitAck)
    {
        list = &plc->plcBlkListIdle;
        chain = &plc->plcBlkBufWaitAck->chain;
        ChainDelSafeD(chain);
        ChainInsertD(list, chain);
        plc->plcBlkBufWaitAck = NULL;
    }
    return;
}

void plcSmplAddrReset(Plc *plc)
{
    PlcMgr *mgr;
    u8 regAmt;

    mgr = gPlcMgr;
    plc->smplTrayIdx = 0;
    plc->smplRegIdx = mgr->regReadMin;
    regAmt = mgr->regReadMaxCri - mgr->regReadMin;
    plc->smplRegAmt = regAmt<mgr->regReadAmtMax ? regAmt : mgr->regReadAmtMax;
    return;
}

void plcSmplAddrUpdate(Plc *plc)
{
    PlcMgr *mgr;
    u8 regAmt;
    u8 idx;

    mgr = gPlcMgr;
    for (; plc->smplTrayIdx<plc->trayAmt; plc->smplTrayIdx++)
    {
        if (plc->smplRegIdx+plc->smplRegAmt < mgr->regReadMaxCri)
        {
            plc->smplRegIdx += plc->smplRegAmt;
            regAmt = mgr->regReadMaxCri - plc->smplRegIdx;
            plc->smplRegAmt = regAmt<mgr->regReadAmtMax ? regAmt : mgr->regReadAmtMax;
            return;
        }
        plc->smplRegIdx = mgr->regReadMin;
        plc->smplRegAmt = 0;
    }
    return;
}

Plc *devTrayIdx2Plc(u8 trayIdx, PlcMgr *mgr, u8 *idxInPlc)
{
    Plc *plc;
    u8 idx;

    for (plc=mgr->plc; plc<&mgr->plc[mgr->plcAmt]; plc++)
    {
        for (idx=0; idx<plc->trayAmt; idx++)
        {
            if (plc->plc2TrayIdx[idx] == trayIdx)
            {
                *idxInPlc = idx;
                return plc;
            }
        }
    }

    return NULL;
}

void plcRegWrite(Plc *plc, PlcCmdBlkBuf *blkBuf)
{
    ModbusTcpHead *head;
    ModbusTcpCmd *cmd;
    u8 *buf;
    u16 regAddr;

#ifdef DebugVersion
    buf = NULL;
#else
    buf = Pb_txbuf_malloc(sizeof(ModbusTcpHead) + sizeof(ModbusTcpCmd));
#endif
    if (NULL == buf)
    {
        if (!ChainInList(&blkBuf->chain))
        {
            ChainInsertD(&plc->plcBlkListBusy, &blkBuf->chain);
        }
        return;
    }

    head = (ModbusTcpHead *)buf;
    head->protoId = WordToNet(ModbusTcpProtoId);
    head->length = WordToNet(sizeof(ModbusTcpCmd));
    head->transId = WordToNet(plc->transId);
    cmd = (ModbusTcpCmd *)head->payload;
    cmd->unitId = 0;
    cmd->funcCode = ModbusTcpFcWtRegSingle;
    regAddr = plc->trayAddrBase[blkBuf->trayIdxInPlc] + gPlcMgr->plcCtrl[blkBuf->regType].plcAddr;
    cmd->regAddr = WordToNet(regAddr);
    cmd->oprVal = WordToNet(blkBuf->regVal);
    plc->plcBlkBufWaitAck = blkBuf;
    timerStart(&plc->plcWaitAckTmr, TidPlcRxCtrlAck, 100, WiReset);
    return;
}

void plcRegWriteTry(u8 trayIdx, u8eNdbdCtrlType type, s16 val)
{
    PlcCmdBlkBuf *blkBuf;
    Plc *plc;
    u8 trayIdxInPlc;

    if (NULL==(plc=devTrayIdx2Plc(trayIdx, gPlcMgr, &trayIdxInPlc))
        || !plc->online || NULL==(blkBuf=plcBlkBufAlloc(plc)))
    {
        return;
    }

    blkBuf->trayIdxInPlc = trayIdxInPlc;
    blkBuf->regType = type;
    blkBuf->regVal = (u16)val;
    if (TimerIsRun(&plc->plcWaitAckTmr))
    {
        ChainInsertD(&plc->plcBlkListBusy, &blkBuf->chain);
        return;
    }

    plcRegWrite(plc, blkBuf);
    return;
}

void plcSmplTx(Plc *plc)
{
    ModbusTcpHead *head;
    ModbusTcpCmd *cmd;
    u8 *buf;
    u16 regAddr;

#ifdef DebugVersion
    buf = NULL;
#else
    buf = Pb_txbuf_malloc(sizeof(ModbusTcpHead) + sizeof(ModbusTcpCmd));
#endif
    if (NULL == buf)
    {
        timerStart(&plc->plcSmplLoopTmr, TidPlcSmplLoop, 1000, WoReset);
        return;
    }

    head = (ModbusTcpHead *)buf;
    head->transId = WordToNet(plc->transId);
    head->protoId = WordToNet(ModbusTcpProtoId);
    head->length = WordToNet(sizeof(ModbusTcpCmd));
    cmd = (ModbusTcpCmd *)head->payload;
    cmd->unitId = 0;
    cmd->funcCode = ModbusTcpFcRdHoldReg;
    regAddr = plc->smplRegIdx + plc->trayAddrBase[plc->smplTrayIdx];
    cmd->regAddr = WordToNet(regAddr);
    cmd->oprVal = WordToNet(plc->smplRegAmt);
    timerStart(&plc->plcWaitAckTmr, TidPlcRxSmplAck, 100, WiReset);
    return;
}

void plcSmplBegin(Timer *timer)
{
    Plc *plc;

    plc = Container(Plc, plcSmplLoopTmr, timer);
    plcSmplAddrReset(plc);
    if (plc->smplTrayIdx == plc->trayAmt)
    {
        timerStart(timer, TidPlcSmplLoop, 1000, WoReset);
        return;
    }

    if (TimerIsRun(&plc->plcWaitAckTmr))
    {
        return;
    }

    plcSmplTx(plc);
    return;
}

void plcIdleNtfy(Plc *plc)
{
    if (!ListIsEmpty(&plc->plcBlkListBusy))
    {
        PlcCmdBlkBuf *blkBuf;

        blkBuf = Container(PlcCmdBlkBuf, chain, plc->plcBlkListBusy.next);
        plcRegWrite(plc, blkBuf);
        return;
    }

    if (!TimerIsRun(&plc->plcSmplLoopTmr))  /*定时器空闲表明本轮采样已启动*/
    {
        if (plc->smplTrayIdx < plc->trayAmt) /*本轮采样尚未结束*/
        {
            plcSmplTx(plc);
        }
        else
        {
            timerStart(&plc->plcSmplLoopTmr, TidPlcSmplLoop, 1000, WoReset);
        }
    }

    return;
}

/*重连,通知上位机,todo,还需要保护*/
void plcExprAckNtfy(Plc *plc)
{
    if (plc->online)
    {
        plc->delaySmpCnt++;
        if (plc->delaySmpCnt > plc->trayAmt*3)
        {
            gDevMgr->tray[plc->smplTrayIdx].ndbdData.ndbdDataValid = False;
            plc->online = False;
            plc->delaySmpCnt = 0;
            sendUpConnNtfy();
    #ifdef DebugVersion
    #else
            plc_reconn();
    #endif
        }
    }

    return;
}

/*plc采样超时的数据,是保持上次还是怎样?todo,都不合适*/
void plcExprRxSmplAck(Timer *timer)
{
    Plc *plc;

    plc = Container(Plc, plcWaitAckTmr, timer);
    plcExprAckNtfy(plc);
    plc->transId++;
    plcSmplAddrUpdate(plc);
    plcIdleNtfy(plc);
    return;
}

/*todo,清空--blockBuf*/
void plcExprRxCtrlAck(Timer *timer)
{
    Plc *plc;

    plc = Container(Plc, plcWaitAckTmr, timer);
    plcExprAckNtfy(plc);
    plc->transId++;
    plcBlkBufFree(plc);
    plcIdleNtfy(plc);
    return;
}

/*plc上电后的状态不准,需要延时采样*/
/*但是网口驱动几乎没有应用模型*/
/*目前暂用计数机制,完善驱动模型后改为定时器*/
void plcRxSmplAck(Plc *plc, u8 *buf)
{
    Tray *tray;
    NdbdData *ndbdData;
    PlcMgr *mgr;
    u8 *dst;
    u8 *src;
    u8 idx;
    b8 trayIsTouchLst;

    tray = &gDevMgr->tray[plc->smplTrayIdx];
    ndbdData = &tray->ndbdData;
    if (!plc->online)
    {
        if (plc->delaySmpCnt < plc->trayAmt*3) /*延时3秒后采样有效*/
        {
            plc->delaySmpCnt++;
            return;
        }
        plc->delaySmpCnt = 0;
        plc->online = True;
        ndbdData->ndbdDataValid = True;
        sendUpConnNtfy();
    }

    mgr = gPlcMgr;
    for (idx=0; idx<NdbdSenCri; idx++)
    {
        if (mgr->plcSta[idx].enable)
        {
            dst = (u8 *)&ndbdData->status[idx];
            src = buf + mgr->plcSta[idx].plcAddr*sizeof(u16);
            dst[0] = src[1];
            dst[1] = src[0];
        }
    }

    trayIsTouchLst = ndbdData->trayIsTouch;
    ndbdData->trayIsEnter = 1==ndbdData->status[NdbdStaEnter] ? True : False;
    ndbdData->trayIsTouch = 1==ndbdData->status[NdbdStaTouch] ? True : False;
    if (ndbdData->trayIsTouch && !trayIsTouchLst)
    {
        timerStart(&tray->protEnaTmr, TidTrayProtEna, 2000, WiReset);
    }
    else if (!ndbdData->trayIsTouch && trayIsTouchLst)
    {
        tray->protEnable = False;
        TimerStop(&tray->protEnaTmr);
    }

    for (idx=0; idx<NdbdWarnCri; idx++)
    {
        if (mgr->plcWarn[idx].enable)
        {
            dst = (u8 *)&ndbdData->warn[idx];
            src = buf + mgr->plcWarn[idx].plcAddr*sizeof(u16);
            dst[0] = src[1];
            dst[1] = src[0];
        }
    }
    return;
}

u16 plcRxMsg(u8 *buf, u16 size)
{
    ModbusTcpHead *ackHead;
    ModbusTcpRdAck *ack;
    PlcMgr *plcMgr;
    Plc *plc;
    u8 *pos;
    u8 *head;
    u16 len;
    u16 pldLen;
    u8 funcCode;
    u16 transId;

    plc = gPlcMgr->plc;
    for (head=buf,pos=buf+size; ;head+=sizeof(ModbusTcpHead)+pldLen)
    {
        len = pos - head;
        if (len < sizeof(ModbusTcpHead))
        {
            goto recvContinue;
        }

        ackHead = (ModbusTcpHead *)head;
        pldLen = WordToNet(ackHead->length);
        transId = WordToNet(ackHead->transId);
        if (plc->transId != transId)
        //if (plc->transId != WordToNet(ackHead->transId))
        {
            return 0;  /*丢弃，清空*/
        }

        if (len < pldLen + sizeof(ModbusTcpHead))
        {
            goto recvContinue;
        }

        ack = (ModbusTcpRdAck *)ackHead->payload;
        funcCode = ack->funcCode;
        if (funcCode & 0x80) /*plc拒绝了指令*/
        {
            if (sizeof(ModbusTcpRdAck) != pldLen)
            {
                return 0;  /*丢弃，清空*/
            }

            funcCode &= 0x7f;
            if (ModbusTcpFcRdHoldReg == funcCode)
            {
                plcSmplAddrUpdate(plc);
            }
            else if (ModbusTcpFcWtRegSingle == funcCode)
            {
                plcBlkBufFree(plc);
            }

            plc->transId++;
            TimerStop(&plc->plcWaitAckTmr);  /*先处理, 后停定时器*/
            plcIdleNtfy(plc);
            continue;
        }

        if (ModbusTcpFcWtRegSingle == funcCode)
        {
            if (sizeof(ModbusTcpWtAck) != pldLen)
            {
                return 0;  /*丢弃，清空*/
            }

            plcBlkBufFree(plc);
            plc->transId++;
            TimerStop(&plc->plcWaitAckTmr);  /*先处理, 后停定时器*/
            plcIdleNtfy(plc);
            continue;
        }

        if (pldLen != plc->smplRegAmt*2+sizeof(ModbusTcpRdAck))
        {
            return 0;  /*丢弃，清空*/
        }

        plcRxSmplAck(plc, ack->val);
        plcSmplAddrUpdate(plc);
        plc->transId++;
        TimerStop(&plc->plcWaitAckTmr);  /*先处理, 后停定时器*/
        plcIdleNtfy(plc);
    }

recvContinue:
    if (0!=len && head!=buf)
    {
        memcpy(buf, head, len);
    }
    return len;
}

void plcInitAddr(PlcMgr *mgr)
{
    mgr->plcSta[NdbdSenNpVal].enable = True;
    mgr->plcSta[NdbdSenNpVal].amount = 1;
    mgr->plcSta[NdbdSenNpVal].plcAddr = 0;
    strcpy(mgr->plcSta[NdbdSenNpVal].name, "npVal");
    mgr->plcSta[NdbdStaEnter].enable = True;
    mgr->plcSta[NdbdStaEnter].amount = 1;
    mgr->plcSta[NdbdStaEnter].plcAddr = 3;
    strcpy(mgr->plcSta[NdbdStaEnter].name, "trayEnter");
    mgr->plcSta[NdbdStaTouch].enable = True;
    mgr->plcSta[NdbdStaTouch].amount = 1;
    mgr->plcSta[NdbdStaTouch].plcAddr = 4;
    strcpy(mgr->plcSta[NdbdStaTouch].name, "trayTouch");
    mgr->plcSta[NdbdSenFwdEnter].enable = True;
    mgr->plcSta[NdbdSenFwdEnter].amount = 1;
    mgr->plcSta[NdbdSenFwdEnter].plcAddr = 8;
    strcpy(mgr->plcSta[NdbdSenFwdEnter].name, "fwdEnter");
    mgr->plcSta[NdbdSenBackEnter].enable = True;
    mgr->plcSta[NdbdSenBackEnter].amount = 1;
    mgr->plcSta[NdbdSenBackEnter].plcAddr = 9;
    strcpy(mgr->plcSta[NdbdSenBackEnter].name, "backEnter");
    mgr->plcSta[NdbdSenFowTouch].enable = True;
    mgr->plcSta[NdbdSenFowTouch].amount = 1;
    mgr->plcSta[NdbdSenFowTouch].plcAddr = 10;
    strcpy(mgr->plcSta[NdbdSenFowTouch].name, "fwdTouch");
    mgr->plcSta[NdbdSenBackTouch].enable = True;
    mgr->plcSta[NdbdSenBackTouch].amount = 1;
    mgr->plcSta[NdbdSenBackTouch].plcAddr = 11;
    strcpy(mgr->plcSta[NdbdSenBackTouch].name, "backTouch");
    mgr->plcSta[NdbdStaWorkMode].enable = True;
    mgr->plcSta[NdbdStaWorkMode].amount = 1;
    mgr->plcSta[NdbdStaWorkMode].plcAddr = 16;
    strcpy(mgr->plcSta[NdbdStaWorkMode].name, "workMode");
    mgr->plcSta[NdbdSenRatioVal].enable = True;
    mgr->plcSta[NdbdSenRatioVal].amount = 1;
    mgr->plcSta[NdbdSenRatioVal].plcAddr = 17;
    strcpy(mgr->plcSta[NdbdSenRatioVal].name, "ratioVal");
    mgr->plcSta[NdbdSenSwValve].enable = True;
    mgr->plcSta[NdbdSenSwValve].amount = 1;
    mgr->plcSta[NdbdSenSwValve].plcAddr = 18;
    strcpy(mgr->plcSta[NdbdSenSwValve].name, "swValve");
    mgr->plcSta[NdbdSenBrkVacum].enable = True;
    mgr->plcSta[NdbdSenBrkVacum].amount = 1;
    mgr->plcSta[NdbdSenBrkVacum].plcAddr = 19;
    strcpy(mgr->plcSta[NdbdSenBrkVacum].name, "brkVacum");
    mgr->plcSta[NdbdSenFireDoorUp].enable = True;
    mgr->plcSta[NdbdSenFireDoorUp].amount = 1;
    mgr->plcSta[NdbdSenFireDoorUp].plcAddr = 21;
    strcpy(mgr->plcSta[NdbdSenFireDoorUp].name, "fireDoorUp");
    mgr->plcSta[NdbdSenFireDoorDown].enable = True;
    mgr->plcSta[NdbdSenFireDoorDown].amount = 1;
    mgr->plcSta[NdbdSenFireDoorDown].plcAddr = 22;
    strcpy(mgr->plcSta[NdbdSenFireDoorDown].name, "fireDoorDown");
    mgr->plcSta[NdbdSenFwdDoorMsw].enable = True;
    mgr->plcSta[NdbdSenFwdDoorMsw].amount = 1;
    mgr->plcSta[NdbdSenFwdDoorMsw].plcAddr = 23;
    strcpy(mgr->plcSta[NdbdSenFwdDoorMsw].name, "fwdDoor");
    mgr->plcSta[NdbdSenBackDoor].enable = True;
    mgr->plcSta[NdbdSenBackDoor].amount = 1;
    mgr->plcSta[NdbdSenBackDoor].plcAddr = 24;
    strcpy(mgr->plcSta[NdbdSenBackDoor].name, "backDoor");
    mgr->plcSta[NdbdSenCylinderDown].enable = True;
    mgr->plcSta[NdbdSenCylinderDown].amount = 1;
    mgr->plcSta[NdbdSenCylinderDown].plcAddr = 25;
    strcpy(mgr->plcSta[NdbdSenCylinderDown].name, "cyldDown");
    mgr->plcSta[NdbdSenCylinderUp].enable = True;
    mgr->plcSta[NdbdSenCylinderUp].amount = 1;
    mgr->plcSta[NdbdSenCylinderUp].plcAddr = 26;
    strcpy(mgr->plcSta[NdbdSenCylinderUp].name, "cyldUp");

    mgr->plcWarn[NdbdWarnScram].enable = True;
    mgr->plcWarn[NdbdWarnScram].amount = 1;
    mgr->plcWarn[NdbdWarnScram].plcAddr = 42;
    strcpy(mgr->plcWarn[NdbdWarnScram].name, "scram");
    mgr->plcWarn[NdbdWarnGas].enable = True;
    mgr->plcWarn[NdbdWarnGas].amount = 1;
    mgr->plcWarn[NdbdWarnGas].plcAddr = 43;
    strcpy(mgr->plcWarn[NdbdWarnGas].name, "gasPres");
    mgr->plcWarn[NdbdWarnSmoke].enable = True;
    mgr->plcWarn[NdbdWarnSmoke].amount = 1;
    mgr->plcWarn[NdbdWarnSmoke].plcAddr = 44;
    strcpy(mgr->plcWarn[NdbdWarnSmoke].name, "smoke");
    mgr->plcWarn[NdbdWarnCo].enable = True;
    mgr->plcWarn[NdbdWarnCo].amount = 1;
    mgr->plcWarn[NdbdWarnCo].plcAddr = 45;
    strcpy(mgr->plcWarn[NdbdWarnCo].name, "co");
    mgr->plcWarn[NdbdWarnCylinder].enable = True;
    mgr->plcWarn[NdbdWarnCylinder].amount = 1;
    mgr->plcWarn[NdbdWarnCylinder].plcAddr = 46;
    strcpy(mgr->plcWarn[NdbdWarnCylinder].name, "cylinder");
    mgr->plcWarn[NdbdWarnFan].enable = True;
    mgr->plcWarn[NdbdWarnFan].amount = 1;
    mgr->plcWarn[NdbdWarnFan].plcAddr = 47;
    strcpy(mgr->plcWarn[NdbdWarnFan].name, "fan");
    mgr->plcWarn[NdbdWarnFireDoor].enable = True;
    mgr->plcWarn[NdbdWarnFireDoor].amount = 1;
    mgr->plcWarn[NdbdWarnFireDoor].plcAddr = 49;
    strcpy(mgr->plcWarn[NdbdWarnFireDoor].name, "fireDoor");
    mgr->plcWarn[NdbdWarnSlotDoorRast].enable = True;
    mgr->plcWarn[NdbdWarnSlotDoorRast].amount = 1;
    mgr->plcWarn[NdbdWarnSlotDoorRast].plcAddr = 50;
    strcpy(mgr->plcWarn[NdbdWarnSlotDoorRast].name, "slotDoorRast");
    mgr->plcWarn[NdbdWarnTray].enable = True;
    mgr->plcWarn[NdbdWarnTray].amount = 1;
    mgr->plcWarn[NdbdWarnTray].plcAddr = 51;
    strcpy(mgr->plcWarn[NdbdWarnTray].name, "tray");
    mgr->plcWarn[NdbdWarnFwdDoorMsw].enable = True;
    mgr->plcWarn[NdbdWarnFwdDoorMsw].amount = 1;
    mgr->plcWarn[NdbdWarnFwdDoorMsw].plcAddr = 52;
    strcpy(mgr->plcWarn[NdbdWarnFwdDoorMsw].name, "fwdDoor");
    mgr->plcWarn[NdbdWarnBackDoor].enable = True;
    mgr->plcWarn[NdbdWarnBackDoor].amount = 1;
    mgr->plcWarn[NdbdWarnBackDoor].plcAddr = 53;
    strcpy(mgr->plcWarn[NdbdWarnBackDoor].name, "backDoor");
    mgr->plcWarn[NdbdWarnRatio].enable = True;
    mgr->plcWarn[NdbdWarnRatio].amount = 1;
    mgr->plcWarn[NdbdWarnRatio].plcAddr = 54;
    strcpy(mgr->plcWarn[NdbdWarnRatio].name, "ratio");
    mgr->plcWarn[NdbdWarnScapeGoat].enable = True;
    mgr->plcWarn[NdbdWarnScapeGoat].amount = 1;
    mgr->plcWarn[NdbdWarnScapeGoat].plcAddr = 55;
    strcpy(mgr->plcWarn[NdbdWarnScapeGoat].name, "scapeGoat");

    mgr->plcCtrl[NdbdSetTouch].enable = True;
    mgr->plcCtrl[NdbdSetTouch].amount = 1;
    mgr->plcCtrl[NdbdSetTouch].plcAddr = 68;
    strcpy(mgr->plcCtrl[NdbdSetTouch].name, "touchCtrl");
    mgr->plcCtrl[NdbdSetFireNtfy].enable = True;
    mgr->plcCtrl[NdbdSetFireNtfy].amount = 1;
    mgr->plcCtrl[NdbdSetFireNtfy].plcAddr = 70;
    strcpy(mgr->plcCtrl[NdbdSetFireNtfy].name, "fireNtfy");
    mgr->plcCtrl[NdbdSetWarnDel].enable = True;
    mgr->plcCtrl[NdbdSetWarnDel].amount = 1;
    mgr->plcCtrl[NdbdSetWarnDel].plcAddr = 71;
    strcpy(mgr->plcCtrl[NdbdSetWarnDel].name, "warnDel");
    mgr->plcCtrl[NdbdSetFireDoor].enable = True;
    mgr->plcCtrl[NdbdSetFireDoor].amount = 1;
    mgr->plcCtrl[NdbdSetFireDoor].plcAddr = 75;
    strcpy(mgr->plcCtrl[NdbdSetFireDoor].name, "fireDoor");
    mgr->plcCtrl[NdbdSetRatioVal].enable = True;
    mgr->plcCtrl[NdbdSetRatioVal].amount = 1;
    mgr->plcCtrl[NdbdSetRatioVal].plcAddr = 77;
    strcpy(mgr->plcCtrl[NdbdSetRatioVal].name, "ratio");
    mgr->plcCtrl[NdbdSetSwValve].enable = True;
    mgr->plcCtrl[NdbdSetSwValve].amount = 1;
    mgr->plcCtrl[NdbdSetSwValve].plcAddr = 79;
    strcpy(mgr->plcCtrl[NdbdSetSwValve].name, "valve");
    mgr->plcCtrl[NdbdSetBrkVacum].enable = True;
    mgr->plcCtrl[NdbdSetBrkVacum].amount = 1;
    mgr->plcCtrl[NdbdSetBrkVacum].plcAddr = 80;
    strcpy(mgr->plcCtrl[NdbdSetBrkVacum].name, "brkVacum");
    mgr->plcCtrl[NdbdSetFixtPower].enable = True;
    mgr->plcCtrl[NdbdSetFixtPower].amount = 1;
    mgr->plcCtrl[NdbdSetFixtPower].plcAddr = 82;
    strcpy(mgr->plcCtrl[NdbdSetFixtPower].name, "fixtPower");
    mgr->plcCtrl[NdbdSetFan].enable = True;
    mgr->plcCtrl[NdbdSetFan].amount = 1;
    mgr->plcCtrl[NdbdSetFan].plcAddr = 76;
    strcpy(mgr->plcCtrl[NdbdSetFan].name, "fan");

    mgr->regReadMin = 0;
    mgr->regReadMaxCri = mgr->plcWarn[NdbdWarnScapeGoat].plcAddr + 1;

    return;
}

void plcInit()
{
    PlcMgr *mgr;
    PlcCmdBlkBuf *blkBuf;
    Plc *plc;
    u16 idx;
    u16 idx2;

    gPlcMgr = NULL;
    if (0 == gPlcAmt)
    {
        return;
    }

    gPlcSendBuf = sysMemAlloc(256);
    gPlcMgr = mgr = sysMemAlloc(sizeof(PlcMgr));
    mgr->plcAmt = gPlcAmt;
    mgr->plcInfoHasChg = False;
    mgr->plc = sysMemAlloc(sizeof(Plc) * gPlcAmt);
    mgr->plcProtoType = PlcPortoModbusTcp;
    mgr->regReadAmtMax = gPlcReadAmtMax;
    plcInitAddr(mgr);

    for (idx=0; idx<gPlcAmt; idx++)
    {
        plc = &mgr->plc[idx];
        plc->plcIdx = idx;
        plc->online = False;
        plc->delaySmpCnt = 0;
        plc->trayAmt = gPlcMapTrayNum;
        plc->version = 1;
        plc->ip = (gPlcIp[0]<<24)+(gPlcIp[1]<<16)+(gPlcIp[2]<<8)+gPlcIp[3];
        plc->port = gPlcPort;
        plc->transId = 0;
        plc->smplRegIdx = 0;
        plc->smplRegAmt = 0;
        plc->smplTrayIdx = plc->trayAmt; /*定时器启动*/
        for (idx2=0; idx2<plc->trayAmt; idx2++)
        {
            plc->plc2TrayIdx[idx2] = gPlcTrayIdx[idx2];
            plc->trayAddrBase[idx2] = gPlcTrayAddrBase[idx2];
        }

        ListInitD(&plc->plcBlkListIdle);
        ListInitD(&plc->plcBlkListBusy);
        plc->plcBlkBufWaitAck = NULL;
        blkBuf = sysMemAlloc(sizeof(PlcCmdBlkBuf) * PlcBufAmt);
        for (idx=0; idx<PlcBufAmt; idx++,blkBuf++)
        {
            ChainInsertD(&plc->plcBlkListIdle, &blkBuf->chain);
        }

    #ifdef DebugVersion
    #else
        Pb_init(w5500_network_card(), gPlcIp, gPlcPort, plcRxMsg);
    #endif

        TimerInit(&plc->plcSmplLoopTmr);
        TimerInit(&plc->plcWaitAckTmr);
        timerStart(&plc->plcSmplLoopTmr, TidPlcSmplLoop, 1000, WiReset);
    }

    return;
}



