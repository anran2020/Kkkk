

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
#include "upper_ntp.h"
#include "alarm_led.h"
#endif

/*本文件包括所有上中协议的处理入口*/

UpItfCb *gUpItfCb;

/*临时用的全局变量*/
u8 *sendUpBuf;
u8 *recvUpBuf;
u32 gAbsTimeSec;

Ret upStepChkErr(UpFlowCmd *cmd, UpStepInfo *step)
{
    return Nok;
}

Ret upStepChkQuiet(UpFlowCmd *cmd, UpStepInfo *step)
{
    if (0x01 == step->paramEnable)
    {
        return Ok;
    }

    return Nok;
}

Ret upStepChkCCC(UpFlowCmd *cmd, UpStepInfo *step)
{
    Tray *tray;

    tray = &gDevMgr->tray[cmd->trayIdx];
    if (0x0f==step->paramEnable
        && step->stepParam[0]>0 && step->stepParam[0]<=tray->boxCfg.maxTrayCur)
    {
        return Ok;
    }

    return Nok;
}

Ret upStepChkCCD(UpFlowCmd *cmd, UpStepInfo *step)
{
    Tray *tray;

    tray = &gDevMgr->tray[cmd->trayIdx];
    if (0x0f==step->paramEnable
        && step->stepParam[0]>0 && step->stepParam[0]<=tray->boxCfg.maxTrayCur)
    {
        return Ok;
    }

    return Nok;
}

Ret upStepChkCCCVC(UpFlowCmd *cmd, UpStepInfo *step)
{
    Tray *tray;
    s32 *param;

    tray = &gDevMgr->tray[cmd->trayIdx];
    param = step->stepParam;
    if (0x1f==step->paramEnable
        && param[0]>0 && param[0]<=tray->boxCfg.maxTrayCur
        && param[1]>0 && param[1]<=tray->boxCfg.maxTrayVol)
    {
        return Ok;
    }

    return Nok;
}

Ret upStepChkCCCVD(UpFlowCmd *cmd, UpStepInfo *step)
{
    Tray *tray;
    s32 *param;

    tray = &gDevMgr->tray[cmd->trayIdx];
    param = step->stepParam;
    if (0x1f==step->paramEnable
        && param[0]>0 && param[0]<=tray->boxCfg.maxTrayCur
        && param[1]>0 && param[1]<=tray->boxCfg.maxTrayVol)
    {
        return Ok;
    }

    return Nok;
}

Ret upStepChkCVC(UpFlowCmd *cmd, UpStepInfo *step)
{
    Tray *tray;
    s32 *param;

    tray = &gDevMgr->tray[cmd->trayIdx];
    param = step->stepParam;
    if (0x1e == (step->paramEnable & 0x1e)
        && param[1]>0 && param[1]<=tray->boxCfg.maxTrayVol
        && (!(step->paramEnable&0x01) || param[0]>0 && param[0]<=tray->boxCfg.maxTrayCur))
    {
        return Ok;
    }

    return Nok;
}

Ret upStepChkCVD(UpFlowCmd *cmd, UpStepInfo *step)
{
    Tray *tray;
    s32 *param;

    tray = &gDevMgr->tray[cmd->trayIdx];
    param = step->stepParam;
    if (0x1e == (step->paramEnable & 0x1e)
        && param[1]>0 && param[1]<=tray->boxCfg.maxTrayVol
        && (!(step->paramEnable&0x01) || param[0]>0 && param[0]<=tray->boxCfg.maxTrayCur))
    {
        return Ok;
    }

    return Nok;
}

/*允许跳转次数为零*/
Ret upStepChkLoop(UpFlowCmd *cmd, UpStepInfo *step)
{
    if (0x03==step->paramEnable && step->stepParam[0]<StepIdNull && step->stepParam[1]<65536)
    {
        return Ok;
    }
    return Nok;
}

Ret upStepChkDcir(UpFlowCmd *cmd, UpStepInfo *step)
{
    return Nok;
}

Ret upStepChk(UpFlowCmd *cmd, UpStepInfo *step)
{
    if (step->stepType>StepTypeNull && step->stepType<StepTypeCri)
    {
        return gUpItfCb->stepParamChk[step->stepType](cmd, step);
    }

    return Nok;
}

/*托盘统排通道号到通道的映射,不检查参数*/
/*返回主通道指针,以及通道内子主统排索引(genIdxInChn)*/
Channel *upChnMap(Tray *tray, u16 chnIdx, u16 *genIdxInChn)
{
    Channel *chn;

    if (BoxTypeParallel == tray->boxCfg.boxType)
    {
        *genIdxInChn = 0;
        chn = &tray->chn[chnIdx];
    }
    else
    {
        Channel *chnCri;

        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            if (chnIdx <= chn->chnCellAmt)
            {
                *genIdxInChn = chnIdx;
                break;
            }
            chnIdx -= chn->chnCellAmt+1;
        }
    }
    return chn;
}

u16eRspCode upFlowStepChk(UpFlowCmd *cmd, u16 pldLen)
{
    UpStepInfo *step;
    DevMgr *dev;
    FlowRecvCtrl *ctrl;
    Tray *tray;
    u16 pldLenHope;
    u16 idx;

    dev = gDevMgr;
    pldLenHope = sizeof(UpFlowCmd) + Align16(cmd->chnAmt)*2;
    pldLenHope += sizeof(UpStepInfo) * cmd->stepAmt;
    if (pldLenHope!=pldLen || cmd->trayIdx>=dev->trayAmt)
    {
        /*基本检查,直接返回错误,不清除之前*/
        return RspParam;
    }

    tray = &dev->tray[cmd->trayIdx];
    if (0==cmd->stepAmt || cmd->stepAmt>MaxStepTrans
        || cmd->stepSeq+cmd->stepAmt > cmd->stepAmtTtl
        || cmd->chnAmt > dev->tray[cmd->trayIdx].trayChnAmt)
    {
        recvCtrlFree(tray);
        return RspParam;
    }

    ctrl = tray->flowRecvCtrl;
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        Channel *chn;
        u16 genChnIdx;

        if (0 != cmd->stepSeq)
        {
            if (cmd->stepAmtTtl == cmd->stepSeq+cmd->stepAmt) /*重发*/
            {
                return RspRepeatDrop;
            }
            return RspStepFst;
        }

        if (0 == cmd->chnAmt)  /*整盘*/
        {
            Channel *chnCri;
            
            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                if (ChnStaIdle!=chn->chnStateMed && ChnStaPause!=chn->chnStateMed)
                {
                    return RspChnWiRun;
                }
            }
        }
        else
        {
            for (idx=0; idx<cmd->chnAmt; idx++)
            {
                if (cmd->chnId[idx] >= tray->genChnAmt)
                {
                    return RspChnIdx;
                }

                chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
                if (0 != genChnIdx)
                {
                    return RspChnIdx;
                }

                if (ChnStaIdle!=chn->chnStateMed && ChnStaPause!=chn->chnStateMed)
                {
                    return RspChnWiRun;
                }
            }
        }
    }
    else  /*有多包*/
    {
        if (ctrl->recvMsgId != UpCmdIdManuFlow
            || ctrl->total != cmd->stepAmtTtl
            || ctrl->chnAmt != cmd->chnAmt
            || memcmp(ctrl->chnIdx, cmd->chnId, cmd->chnAmt*2))
        {
            recvCtrlFree(tray);
            return RspMultiSeq;
        }
        else if (ctrl->offsetHope != cmd->stepSeq)
        {
            if (ctrl->offsetHope == cmd->stepSeq+cmd->stepAmt) /*重发*/
            {
                return RspRepeatDrop;
            }
            else
            {
                recvCtrlFree(tray);
                return RspMultiSeq;
            }
        }
    }

    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        if (step->stepId!=cmd->stepSeq+idx || Ok!=upStepChk(cmd, step))
        {
            recvCtrlFree(tray);
            return RspParam;
        }
    }

    return RspOk;
}

u16eRspCode upTrayProtChk(UpProtGenCmd *cmd, u16 pldLen)
{
    UpMixProt *mixProt;
    DevMgr *dev;
    Tray *tray;
    FlowRecvCtrl *ctrl;
    u16 pldLenHope;
    u16 idx;

    dev = gDevMgr;
    pldLenHope = sizeof(UpProtGenCmd);
    for (idx=0; idx<cmd->mixProtAmt; idx++)
    {
        mixProt = (UpMixProt *)((u8 *)cmd + pldLenHope);
        pldLenHope += sizeof(UpMixProt) + Align8(mixProt->mixProtLen);
    }
    pldLenHope += sizeof(UpProtUnit) * cmd->protAmt;
    if (pldLenHope!=pldLen || cmd->trayIdx>=dev->trayAmt)
    {
        /*基本合法性检查,不操作*/
        return RspParam;
    }

    tray = &dev->tray[cmd->trayIdx];
    if (cmd->protAmt>MaxProtTrans || cmd->protSeq+cmd->protAmt>cmd->protAmtTtl
        || (0!=cmd->mixProtAmt && 0!=cmd->protSeq))
    {
        recvCtrlFree(tray);
        return RspParam;
    }

    ctrl = tray->flowRecvCtrl;
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        Channel *chn;
        Channel *chnCri;

        if (0 != cmd->protSeq) /*不管有无保护，序号必须为零*/
        {
            if (cmd->protAmtTtl == cmd->protSeq+cmd->protAmt) /*重发*/
            {
                return RspRepeatDrop;
            }
            return RspProtFst;
        }

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            /*这里难搞,全程保护适用于托盘,假设一个通道在跑*/
            /*此时上位机下发第二个通道的全程保护的时候会报错*/
            /*但上位机若不发全程保护,又不符合约定*/
            /*跟上位机权宜的办法:收到全程保护时,有通道再跑就丢弃,无通道在跑就覆盖*/
            if (ChnStaIdle!=chn->chnStateMed && ChnStaPause!=chn->chnStateMed)
            {
                return RspRepeatDrop;
            }
        }
    }
    else  /*有多包*/
    {
        if (ctrl->recvMsgId != UpCmdIdManuProtGen
            || ctrl->total!=cmd->protAmtTtl || 0==cmd->protAmt)
        {
            recvCtrlFree(tray);
            return RspMultiSeq;
        }
        else if (ctrl->offsetHope != cmd->protSeq)
        {
            if (ctrl->offsetHope == cmd->protSeq+cmd->protAmt)
            {
                return RspRepeatDrop; /*重发*/
            }
            else
            {
                recvCtrlFree(tray);
                return RspMultiSeq;
            }
        }
    }

    return RspOk;
}

u16eRspCode upStepProtChk(UpProtStepCmd *cmd, u16 pldLen)
{
    DevMgr *dev;
    FlowRecvCtrl *ctrl;
    Tray *tray;
    u16 pldLenHope;
    u16 idx;

    dev = gDevMgr;
    pldLenHope = sizeof(UpProtStepCmd) + Align16(cmd->chnAmt)*2;
    pldLenHope += sizeof(UpProtUnit) * cmd->protAmt;
    if (pldLenHope!=pldLen || cmd->trayIdx>=dev->trayAmt)
    {
        /*基本合法性检查,不操作*/
        return RspParam;
    }

    tray = &dev->tray[cmd->trayIdx];
    if (cmd->protAmt>MaxProtTrans || cmd->protSeq+cmd->protAmt>cmd->protAmtTtl
        || cmd->chnAmt > dev->tray[cmd->trayIdx].trayChnAmt)
    {
        recvCtrlFree(tray);
        return RspParam;
    }

    ctrl = tray->flowRecvCtrl;
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        Channel *chn;
        u16 genChnIdx;

        if (0 != cmd->protSeq)
        {
            if (cmd->protAmtTtl == cmd->protSeq+cmd->protAmt) /*重发*/
            {
                return RspRepeatDrop;
            }
            return RspProtFst;
        }

        if (0 == cmd->chnAmt)  /*整盘*/
        {
            Channel *chnCri;
            
            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                if (ChnStaIdle!=chn->chnStateMed && ChnStaPause!=chn->chnStateMed)
                {
                    return RspChnWiRun;
                }
            }
        }
        else
        {
            for (idx=0; idx<cmd->chnAmt; idx++)
            {
                if (cmd->chnId[idx] >= tray->genChnAmt)
                {
                    return RspChnIdx;
                }

                chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
                if (0 != genChnIdx)
                {
                    return RspChnIdx;
                }

                if (ChnStaIdle!=chn->chnStateMed && ChnStaPause!=chn->chnStateMed)
                {
                    return RspChnWiRun;
                }
            }
        }
    }
    else  /*有多包*/
    {
        if (ctrl->recvMsgId != UpCmdIdManuProtStep
            || ctrl->total != cmd->protAmtTtl
            || ctrl->chnAmt != cmd->chnAmt
            || memcmp(ctrl->chnIdx, cmd->chnId, cmd->chnAmt*2))
        {
            recvCtrlFree(tray);
            return RspMultiSeq;
        }
        else if (ctrl->offsetHope != cmd->protSeq)
        {
            if (ctrl->offsetHope == cmd->protSeq+cmd->protAmt)
            {
                return RspRepeatDrop; /*重发*/
            }
            else
            {
                recvCtrlFree(tray);
                return RspMultiSeq;
            }
        }
    }

    return RspOk;
}

/*
func:发送消息给上位机
param:
    msg--指向消息头的消息
    pldLen--消息负载长度，不含头尾
return:
*/
u8 sendUpMsg(u8 *msg, u16 pldLen, u16 msgId)
{
    UpMsgHead *head;
    UpMsgTail *tail;

    head = (UpMsgHead *)msg;
    head->magicTag = UpMsgMagic;
    head->pldLen = pldLen;
    head->msgId = msgId;

    tail = (UpMsgTail *)(msg+sizeof(UpMsgHead)+pldLen);
    tail->msgFlowSeq = gUpItfCb->upMsgFlowSeq;
    tail->crcChk = crc16Modbus(msg+sizeof(u32), pldLen+UpMsgMandCrcLen);

#ifdef DebugVersion
    Show("send upper msg--0x%04x,rsp--0x%04x\r\n", msgId, *(u16 *)(head+1));
#else
    return upper_data_send(msg, pldLen + UpMsgMandLen);
#endif
}

void rspUpCmmn(u16 msgId, u8 trayIdx, u16 rspCode)
{
    UpCmmnAck *ack;
    u8 *buf;

    buf = sendUpBuf;
    ack = (UpCmmnAck *)(buf + sizeof(UpMsgHead));
    ack->trayIdx = trayIdx;
    ack->rsvd = 0;
    ack->rspCode = rspCode;

    sendUpMsg(buf, sizeof(UpCmmnAck), msgId);
    return;
}

void upMsgIgnore(void)
{
    return;
}

void _____begin_of_product_msg_____(){}

/*定时(半小时)查询是否触发上位机脱机保护,若触发则保护且停止采样*/
void trayUpDiscExpr(Timer *timer)
{
    Tray *tray;

    if (gUpItfCb->upOnline)
    {
        return; /*上位机若在线,就当没发生过*/
    }

    for (tray=gDevMgr->tray; tray<&gDevMgr->tray[gDevMgr->trayAmt]; tray++)
    {
        tray->smplMgr.upDiscExpr = True;
    }
    return;
}

/*定时查询与上位机是否有交互*/
void keepAliveChk(Timer *timer)
{
    if (0 == gUpItfCb->rxUpMsgCnt)
    {
        if (gUpItfCb->upOnline)
        {
            timerStart(&gUpItfCb->upDiscExprTmr, TidUpDisc, 30*60*1000, WiReset);
        }
        gUpItfCb->upOnline = False;
        gDevMgr->needConnUp = True;
    #ifdef DebugVersion
    #else
        upper_keepalive_reconn();
    #endif
    }
    else
    {
        gUpItfCb->rxUpMsgCnt = 0;
        timerStart(timer, TidKeepAlive, 20000, WiReset);
    }
    return;
}

/*发送联机给上位机,包括*/
/*1.中位机上电延迟几秒(等查询级联设备)*/
/*2.任何级联设备联机变更*/
void sendUpConnCmd(Timer *timer)
{
    ConnUpCmd *cmd;
    ConnDevInfoTlv *tlv;
    ConnTrayInfo *trayInfo;
    ConnBoxInfo *boxInfo;
    ConnCmmnDevInfo *cmmnDev;
    ConnPlcInfo *plcInfo;
    u8 *buf;
    u8 *pld;
    Tray *tray;
    Box *box;
    SubBox *volSmpl;
    SubBox *bypsSw;
    TmprSmpl *tmprSmpl;
    Plc *plc;
    DevMgr *devMgr;
    u16 pldLen;
    u16 chnlIdx;
    u8 trayIdx;
    u8 boxIdx;
    u16 subBoxIdx;

    timerStart(timer, TidConnUpper, 2000, WiReset);  /*周期检查*/
    devMgr = gDevMgr;
    if (!devMgr->needConnUp)
    {
        return;
    }

    buf = sendUpBuf;
    pld = buf + sizeof(UpMsgHead);
    cmd = (ConnUpCmd *)pld;
    cmd->protocolVer = gProtoSoftVer;
    cmd->mediumSoftVer = gMediumSoftVer;
    cmd->smplVer = gProtoSmplVer;
    cmd->stepVer = 0x0000;
    cmd->protectVer = 0x0000;
    cmd->connType = gUpItfCb->connType;
    cmd->rsvd = 0;
    cmd->maxSmplSeq = gDevMgr->tray[0].smplMgr.smplSeqMax;
    pldLen = sizeof(ConnUpCmd);

    /*所有托盘*/
    for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
    {
        tray = &devMgr->tray[trayIdx];
        tlv = (ConnDevInfoTlv *)(pld+pldLen);
        tlv->devType = DevTypeTray;
        tlv->devInfoLen = sizeof(ConnTrayInfo);

        trayInfo = (ConnTrayInfo *)tlv->devInfo;
        trayInfo->cellAmt = tray->trayCellAmt;
        trayInfo->smplSeqRst = tray->smplMgr.upSmplRstInd;
        trayInfo->trayIdx = trayIdx;

        pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnTrayInfo);
    }

    /*所有电源箱*/
    for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
    {
        tray = &devMgr->tray[trayIdx];
        for (boxIdx=0; boxIdx<tray->boxAmt; boxIdx++)
        {
            box = &tray->box[boxIdx];
            tlv = (ConnDevInfoTlv *)(pld+pldLen);
            tlv->devType = DevTypeDcdcBox;
            tlv->devInfoLen = sizeof(ConnBoxInfo);
        
            boxInfo = (ConnBoxInfo *)tlv->devInfo;
            boxInfo->trayIdx = tray->trayIdx;
            boxInfo->devIdx = box->boxIdxInTray;
            boxInfo->boxType = box->boxType;
            boxInfo->chnAmtConn = box->chnAmtTtl;
            boxInfo->chnModuAmt = box->chnModuAmt;
            boxInfo->isCombine = False;
            boxInfo->boxCellAmt = box->boxCellAmt;
            boxInfo->maxVol = box->maxVol;
            boxInfo->maxCur = box->maxCur;
            boxInfo->softVer = box->softVer;
            boxInfo->online = box->online;
        
            pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnBoxInfo);
        }
    }

    /*电压采样板*/
    for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
    {
        tray = &devMgr->tray[trayIdx];
        volSmpl = tray->volSmpl;
        for (boxIdx=0; boxIdx<tray->boxAmt; boxIdx++)
        {
            box = &tray->box[boxIdx];
            for (subBoxIdx=0; subBoxIdx<tray->boxCfg.volSmplAmt; subBoxIdx++, volSmpl++)
            {
                tlv = (ConnDevInfoTlv *)(pld+pldLen);
                tlv->devType = DevTypeVolSmpl;
                tlv->devInfoLen = sizeof(ConnCmmnDevInfo);

                cmmnDev = (ConnCmmnDevInfo *)tlv->devInfo;
                cmmnDev->trayIdx = trayIdx;
                cmmnDev->devIdx = volSmpl->idxInTray;
                cmmnDev->softVer = volSmpl->softVer;
                cmmnDev->online = volSmpl->online;
                cmmnDev->subType = BoxDevVolSmplResa==volSmpl->subType ? 0 : 1;

                pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnCmmnDevInfo);
            }
        }
    }

    /*旁路切换板*/
    for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
    {
        ConnSerielSwInfo *connSerielSwInfo;

        tray = &devMgr->tray[trayIdx];
        bypsSw = tray->bypsSw;
        for (boxIdx=0; boxIdx<tray->boxAmt; boxIdx++)
        {
            box = &tray->box[boxIdx];
            for (subBoxIdx=0; subBoxIdx<tray->boxCfg.bypsSwAmt; subBoxIdx++, bypsSw++)
            {
                tlv = (ConnDevInfoTlv *)(pld+pldLen);
                tlv->devType = DevTypeSeriesSw;
                tlv->devInfoLen = sizeof(ConnSerielSwInfo);
    
                connSerielSwInfo = (ConnSerielSwInfo *)tlv->devInfo;
                connSerielSwInfo->trayIdx = trayIdx;
                connSerielSwInfo->devIdx = bypsSw->idxInTray;
                connSerielSwInfo->softVer = bypsSw->softVer;
                connSerielSwInfo->chnIdx = bypsSw->idxInTray;
                connSerielSwInfo->online = bypsSw->online;
    
                pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnSerielSwInfo);
            }
        }
    }

    /*温度采样板*/
    for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
    {
        tray = &devMgr->tray[trayIdx];
        for (boxIdx=0; boxIdx<tray->tmprSmplAmt; boxIdx++)
        {
            tlv = (ConnDevInfoTlv *)(pld+pldLen);
            tlv->devType = DevTypeTmprSmpl;
            tlv->devInfoLen = sizeof(ConnCmmnDevInfo);

            tmprSmpl = &devMgr->tmprSmpl[tray->tmprSmplIdxBase+boxIdx];
            cmmnDev = (ConnCmmnDevInfo *)tlv->devInfo;
            cmmnDev->trayIdx = trayIdx;
            cmmnDev->devIdx = tmprSmpl->tmprSmplDevIdx;
            cmmnDev->softVer = tmprSmpl->devVer;
            cmmnDev->online = tmprSmpl->online;

            pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnCmmnDevInfo);
        }
    }

    for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
    {
        tray = &devMgr->tray[trayIdx];
        if (tray->ndbdCtrlByMcu)
        {
            NdbdMcu *ndbdMcu;

            tlv = (ConnDevInfoTlv *)(pld+pldLen);
            tlv->devType = DevTypeNdbd;
            tlv->devInfoLen = sizeof(ConnCmmnDevInfo);

            ndbdMcu = &devMgr->ndbdMcu[trayIdx];
            cmmnDev = (ConnCmmnDevInfo *)tlv->devInfo;
            cmmnDev->trayIdx = trayIdx;
            cmmnDev->devIdx = 0;
            cmmnDev->softVer = ndbdMcu->softVer;
            cmmnDev->online = ndbdMcu->online;

            pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnCmmnDevInfo);
        }
    }

    if (0 != gPlcMgr->plcAmt)
    {
        for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
        {
            tray = &devMgr->tray[trayIdx];
            tlv = (ConnDevInfoTlv *)(pld+pldLen);
            tlv->devType = DevTypePlc;
            tlv->devInfoLen = sizeof(ConnPlcInfo);

            plc = &gPlcMgr->plc[tray->plcIdx];
            plcInfo = (ConnPlcInfo *)tlv->devInfo;
            plcInfo->trayIdx = trayIdx;
            plcInfo->devIdx = plc->plcIdx;
            plcInfo->softVer = plc->version;
            plcInfo->online = plc->online;
            plcInfo->ipAddr = plc->ip;

            pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnPlcInfo);
        }
    }

    if (Ok == sendUpMsg(buf, pldLen, UpCmdIdManuConn))
    {
        devMgr->needConnUp = False;
        gUpItfCb->connType = ConnTypeNml;
        if (!gUpItfCb->upOnline)
        {
            gUpItfCb->upOnline = True;
            timerStart(&gUpItfCb->keepAliveTmr, TidKeepAlive, 20000, WiReset);
        }
        for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
        {
            devMgr->tray[trayIdx].smplMgr.upSmplRstInd = False;
        }
    }

    return;
}

/*通知给上位机发联机*/
void sendUpConnNtfy()
{
    gDevMgr->needConnUp = True;
    return;
}

/*收到上位机的联机响应*/
void upMsgConnAck(u8 *cmdPld, u16 cmdPldLen)
{
    ConnUpAck *ack;

    ack = (ConnUpAck *)cmdPld;
    if (0 == gAbsTimeSec)
    {
        gAbsTimeSec = ack->absSec - sysTimeSecGet();
    }
    return;
}

/*上位机下发顺序: 工步流程-->全程保护-->工步保护(含流程保护)*/
void upMsgFlowStep(u8 *cmdPld, u16 cmdPldLen)
{
    UpFlowCmd *cmd;
    u16eRspCode rspCode;

    cmd = (UpFlowCmd *)cmdPld;
    if (RspOk == (rspCode=upFlowStepChk(cmd, cmdPldLen)))
    {
        rspCode = upFlowStepSave(cmdPld);
    }

    if (RspRepeatDrop == rspCode)
    {
        rspCode = RspOk;
    }
    rspUpCmmn(UpCmdIdManuFlow, cmd->trayIdx, rspCode);
    return;
}


/*客户叫全程或安全保护,保护针对托盘,代码中叫托盘保护*/
void upMsgTrayProt(u8 *cmdPld, u16 cmdPldLen)
{
    UpProtGenCmd *cmd;
    u16eRspCode rspCode;

    cmd = (UpProtGenCmd *)cmdPld;
    if (RspOk == (rspCode=upTrayProtChk(cmd, cmdPldLen)))
    {
        rspCode = upTrayProtSave(cmdPld);
    }

    if (RspRepeatDrop == rspCode)
    {
        rspCode = RspOk;
    }
    rspUpCmmn(UpCmdIdManuProtGen, cmd->trayIdx, rspCode);
    return;
}

/*工步保护,含流程保护*/
void upMsgStepProt(u8 *cmdPld, u16 cmdPldLen)
{
    UpProtStepCmd *cmd;
    u16eRspCode rspCode;

    cmd = (UpProtStepCmd *)cmdPld;
    if (RspOk == (rspCode=upStepProtChk(cmd, cmdPldLen)))
    {
        rspCode = upStepProtSave(cmdPld);
    }

    if (RspRepeatDrop == rspCode)
    {
        rspCode = RspOk;
    }
    rspUpCmmn(UpCmdIdManuProtStep, cmd->trayIdx, rspCode);
    return;
}

u16eRspCode upSmplChnChk(UpSmplCmd *cmd, u16 cmdPldLen)
{
    if (cmdPldLen != sizeof(UpSmplCmd) || cmd->trayIdx>=gDevMgr->trayAmt
        || cmd->smplSeq > gDevMgr->tray[cmd->trayIdx].smplMgr.smplSeqMax)
    {
        return RspParam;
    }

    return RspOk;
}

void upMsgSmplChnl(u8 *cmdPld, u16 cmdPldLen)
{
    UpSmplCmd *cmd;
    UpSmplAck *ack;
    u8 *smpl;
    Tray *tray;
    u8 *buf;
    u32 seq;
    u16 pldLenAck;
    u8 amt;
    u8 cnt;

    cmd = (UpSmplCmd *)cmdPld;
    buf = sendUpBuf;
    ack = (UpSmplAck *)(buf + sizeof(UpMsgHead));
    if (RspOk != (ack->rspCode=upSmplChnChk(cmd, cmdPldLen)))
    {
        rspUpCmmn(UpCmdIdManuSmplChnl, cmd->trayIdx, ack->rspCode);
        return;
    }

    tray = &gDevMgr->tray[cmd->trayIdx];
    ack->trayIdx = cmd->trayIdx;
    ack->firstSeq = cmd->smplSeq;
    tray->smplMgr.smplSeqUpReq = cmd->smplSeq;
    ack->nextWriteSeq = tray->smplMgr.smplSeqNext;
    pldLenAck = sizeof(UpSmplAck);
    amt = cmd->smplAmt>MsgSmplAmtMax ? MsgSmplAmtMax : (u8)cmd->smplAmt;
    for (cnt=0,seq=cmd->smplSeq,smpl=ack->smpl; cnt<amt&&seq!=tray->smplMgr.smplSeqNext; cnt++,smpl+=tray->smplMgr.smplItemSize)
    {
        memcpy(smpl, tray->smplMgr.smplBufAddrBase+seq*tray->smplMgr.smplItemSize, tray->smplMgr.smplItemSize);
        pldLenAck += tray->smplMgr.smplItemSize;
        seq = seq==tray->smplMgr.smplDiskAmt-1 ? 0 : seq+1;
    }

    ack->smplAmt = cnt;
    if (0 != cnt)
    {
        tray->smplMgr.smplSeqUpReq = cmd->smplSeq;  /*确实有采样上传才更改记录*/
    }
    sendUpMsg(buf, pldLenAck, UpCmdIdManuSmplChnl);
    return;
}

void upMsgSmplNdbd(u8 *cmdPld, u16 cmdPldLen)
{
    UpSmplCmd *cmd;
    UpSmplAck *ack;
    UpNdbdSmpl *smpl;
    NdbdData *ndbd;
    Tray *tray;
    Times time;
    u8 *buf;
    u16 pldLenAck;
    u8 idx;

    cmd = (UpSmplCmd *)cmdPld;
    if (cmdPldLen != sizeof(UpSmplCmd) || cmd->trayIdx>=gDevMgr->trayAmt)
    {
        rspUpCmmn(UpCmdIdManuSmplNdbd, cmd->trayIdx, RspParam);
        return;
    }

    buf = sendUpBuf;
    ack = (UpSmplAck *)(buf + sizeof(UpMsgHead));
    ack->rspCode = RspOk;
    ack->trayIdx = cmd->trayIdx;
    ack->smplAmt = 1;
    ack->firstSeq = cmd->smplSeq;
    ack->nextWriteSeq = cmd->smplSeq;

    smpl = ack->ndbdSmpl;
    ndbd = &gDevMgr->tray[cmd->trayIdx].ndbdData;
    sysTimeGet(&time);
    smpl->timeStampSec = gAbsTimeSec + time.sec;
    smpl->timeStampMs = time.mSec;
    smpl->enterState = ndbd->status[NdbdStaEnter];
    smpl->touchState = ndbd->status[NdbdStaTouch];
    smpl->npVal = ndbd->status[NdbdSenNpVal];
    smpl->ratioAnalog = ndbd->status[NdbdSenRatioVal];
    smpl->bitmapState = 0;
    smpl->bitmapAlarm = 0;
    for (idx=0; idx<BitSenCri; idx++)
    {
        if (ndbd->status[ndbd->bit2IdxSta[idx]])
        {
            BitSet(smpl->bitmapState, idx);
        }
    }
    for (idx=0; idx<BitWarnCri; idx++)
    {
        if (ndbd->warn[ndbd->bit2IdxWarn[idx]])
        {
            BitSet(smpl->bitmapAlarm, idx);
        }
    }

    smpl->cylinderWarn = ndbd->warn[NdbdWarnCylinder];
    smpl->smokeWarn = ndbd->warn[NdbdWarnSmoke];
    smpl->slotTmprAmt = ndbd->slotTmprAmt;
    smpl->rsvd = 0;
    for (idx=0; idx<ndbd->slotTmprAmt; idx++)
    {
        smpl->slotTmpr[idx] = ndbd->slotTmpr[idx];
    }

    pldLenAck = sizeof(UpSmplAck) + sizeof(UpNdbdSmpl) + Align16(idx)*sizeof(u16);
    sendUpMsg(buf, pldLenAck, UpCmdIdManuSmplNdbd);
    return;
}

u16eRspCode upSmplStackChk(UpSmplStackCmd *cmd, u16 pldLen)
{
    if (pldLen != sizeof(UpSmplStackCmd) + Align16(cmd->chnAmt)*2
        || cmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    if (0 != cmd->chnAmt)
    {
        Tray *tray;
        Channel *chn;
        u16 idx;
        u16 genChnIdx;

        tray = &gDevMgr->tray[cmd->trayIdx];
        for (idx=0; idx<cmd->chnAmt; idx++)
        {
            if (cmd->chnId[idx] >= tray->genChnAmt)
            {
                return RspChnIdx;
            }
        
            chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
            if (BoxTypeSeriesWiSw == tray->boxCfg.boxType)
            {
                if (0 == genChnIdx)
                {
                    return RspChnIdx;
                }
            }
            else
            {
                if (0 != genChnIdx)
                {
                    return RspChnIdx;
                }
            }
        }
    }
    return RspOk;
}

void upMsgSmplStack(u8 *cmdPld, u16 pldLen)
{
    UpSmplStackCmd *cmd;
    UpSmplStackAck *ack;
    RunStack *runStack;
    Tray *tray;
    Channel *chn;
    Cell *cell;
    u8 *buf;
    u16 ackPldLen;
    u16 loopDscrLen;

    buf = sendUpBuf;
    cmd = (UpSmplStackCmd *)cmdPld;
    ack = (UpSmplStackAck *)(buf + sizeof(UpMsgHead));
    ack->trayIdx = cmd->trayIdx;
    ack->rsvd2 = ack->rsvd1 = 0;
    ack->timeStamp = gAbsTimeSec + sysTimeSecGet();
    ackPldLen = sizeof(UpSmplStackAck);
    if (RspOk != (ack->rspCode=upSmplStackChk(cmd, pldLen)))
    {
        ack->chnAmt = 0;
        sendUpMsg(buf, ackPldLen, UpCmdIdManuSmplStack);
        return;
    }

    tray = &gDevMgr->tray[cmd->trayIdx];
    runStack = ack->runStack;
    ack->chnAmt = 0;
    if (0 == cmd->chnAmt)
    {
        Channel *chnCri;
        Cell *cellCri;

        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            if (NULL == chn->flowStepEntry)
            {
                continue;
            }

            if (NULL == chn->bypsSeriesInd)
            {
                runStack->chnType = ChnTypeMainChn;
                runStack->flowLoopAmt = chn->flowStepEntry->loopStepAmt;
                runStack->chnIdx = chn->genIdxInTray;
                loopDscrLen = runStack->flowLoopAmt*sizeof(LoopDscr);
                memcpy(runStack->loopDscr, chn->loopDscr, loopDscrLen);
                ackPldLen += sizeof(RunStack) + loopDscrLen;
                runStack = (RunStack *)((u8 *)runStack + sizeof(RunStack) + loopDscrLen);
                ack->chnAmt++;
            }
            else
            {
                for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
                {
                    runStack->chnType = ChnTypeSeriesCell;
                    runStack->flowLoopAmt = chn->flowStepEntry->loopStepAmt;
                    runStack->chnIdx = cell->genIdxInTray;
                    loopDscrLen = runStack->flowLoopAmt*sizeof(LoopDscr);
                    memcpy(runStack->loopDscr, chn->loopDscr, loopDscrLen);
                    ackPldLen += sizeof(RunStack) + loopDscrLen;
                    runStack = (RunStack *)((u8 *)runStack + sizeof(RunStack) + loopDscrLen);
                    ack->chnAmt++;
                }
            }
        }
    }
    else
    {
        u16 genChnIdx;
        u16 idx;

        for (idx=0; idx<cmd->chnAmt; idx++)
        {
            chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
            if (NULL == chn->flowStepEntry)
            {
                continue;
            }

            runStack->chnType = NULL==chn->bypsSeriesInd ? ChnTypeMainChn : ChnTypeSeriesCell;
            runStack->flowLoopAmt = chn->flowStepEntry->loopStepAmt;
            runStack->chnIdx = cmd->chnId[idx];
            loopDscrLen = runStack->flowLoopAmt*sizeof(LoopDscr);
            memcpy(runStack->loopDscr, chn->loopDscr, loopDscrLen);
            ackPldLen += sizeof(RunStack) + loopDscrLen;
            runStack = (RunStack *)((u8 *)runStack + sizeof(RunStack) + loopDscrLen);
            ack->chnAmt++;
        }
    }

    sendUpMsg(buf, ackPldLen, UpCmdIdManuSmplStack);
    return;
}

u32 findSmplBufSeq(u32 upSeq, SmplSaveMgr *smplMgr)
{
    u32 bufSeq;
    u32 offset;
#ifdef DebugVersion
    return upSeq;
#else
    if (smplMgr->smplSeqNext < upSeq)
    {
        offset = smplMgr->smplDiskAmt - upSeq + smplMgr->smplSeqNext;
    }
    else
    {
        offset = smplMgr->smplSeqNext - upSeq;
    }

    if (offset >= smplMgr->smplBufAmt)
    {
        return smplMgr->smplBufAmt;
    }

    bufSeq = (smplMgr->smplBufAddr-smplMgr->smplBufAddrBase)/smplMgr->smplItemSize;
    if (offset > bufSeq)
    {
        return smplMgr->smplBufAmt - offset + bufSeq;
    }
    else
    {
        return bufSeq - offset;
    }
#endif
}

void upMsgSmplTray(u8 *cmdPld, u16 cmdPldLen)
{
    UpSmplCmd *cmd;
    TraySmplAck *ack;
    SmplSaveMgr *smplMgr;
    Tray *tray;
    u8 *buf;
    u16 pldLenAck;
    u32 bufSeq;

    cmd = (UpSmplCmd *)cmdPld;
    buf = sendUpBuf;
    ack = (TraySmplAck *)(buf + sizeof(UpMsgHead));
    if (RspOk != (ack->rspCode=upSmplChnChk(cmd, cmdPldLen)))
    {
        rspUpCmmn(UpCmdIdManuSmplTray, cmd->trayIdx, ack->rspCode);
        return;
    }

    tray = &gDevMgr->tray[cmd->trayIdx];
    smplMgr = &tray->smplMgr;
    ack->trayIdx = cmd->trayIdx;
    ack->rspSmplSeq = cmd->smplSeq;
    ack->nextGenSeq = smplMgr->smplSeqNext;
    pldLenAck = sizeof(TraySmplAck);
    if (cmd->smplSeq==smplMgr->smplSeqNext || (!smplMgr->isLooped && cmd->smplSeq>smplMgr->smplSeqNext))
    {
        ack->smplAmt = 0;
        smplMgr->smplEnable = True;
        smplMgr->upDiscExpr = False;
        sendUpMsg(buf, pldLenAck, UpCmdIdManuSmplTray);

        /*上位机采到后面了,通知归零*/
        if (!smplMgr->isLooped && cmd->smplSeq>smplMgr->smplSeqNext)
        {
            smplMgr->upSmplRstInd = True;
            sendUpConnNtfy();
        }
        return;
    }

    smplMgr->smplSeqUpReq = cmd->smplSeq;  /*确实有采样上传才更改记录*/
    ack->smplAmt = 1;
    bufSeq = findSmplBufSeq(cmd->smplSeq, smplMgr);
    if (bufSeq < smplMgr->smplBufAmt)
    {
        memcpy(ack->traySmplRcd, smplMgr->smplBufAddrBase+bufSeq*smplMgr->smplItemSize, smplMgr->smplItemSize);
    }
    else
    {
    #ifdef DebugVersion
    #else
    #if 1
        if (Ok != smplDiskRead(tray->trayIdx, 1, ack->traySmplRcd, cmd->smplSeq))
        {
            ack->rspCode = RspDiskRead;
        }
    #else
        ds_read_file(tray->trayIdx, cmd->smplSeq, 1, ack->traySmplRcd);
    #endif
    #endif
    }
    pldLenAck += smplMgr->smplItemSize;
    sendUpMsg(buf, pldLenAck, UpCmdIdManuSmplTray);
    if (!smplMgr->smplEnable)
    {
        u32 upLastSureSeq; /*上位机确认的最后采样,这里不管转圈与否*/

        upLastSureSeq = 0==cmd->smplSeq ? smplMgr->smplSeqMax : cmd->smplSeq-1;
        if (upLastSureSeq != smplMgr->smplSeqNext) /*只要不相等就解禁,不用管其它*/
        {
            smplMgr->smplEnable = True;
            smplMgr->upDiscExpr = False;
        }
    }
    return;
}

u16eRspCode boxStartChk(Box *box)
{
    if (!box->online)
    {
        return RspDiscBox;
    }
    if (BoxModeManu != box->boxWorkMode)
    {
        return RspStatusBox;
    }

    /*todo,其它状态*/
    return RspOk;
}
u16eRspCode chnStartChk(Channel *chn)
{
    if (NULL==chn->flowStepEntry || 0==chn->flowStepEntry->stepAmt)
    {
        return RspStep;
    }

    if (NULL == chn->flowProtEntry)
    {
        return RspProt;
    }

    if (chn->chnStateMed >= ChnStaNpWait)
    {
        return RspChnBeRun;
    }
    return RspOk;
}

/*针床有些告警属于自消,需单独判断*/
/*单通道告警,算不算托盘告警?是个问题,目前暂定算*/
b8 trayHasWarn(Tray *tray)
{
    u8eNdbdWarnType idx;

    if (tray->trayWarnPres)
    {
        return True;
    }

    for (idx=0; idx<NdbdWarnCri; idx++)
    {
        if (tray->ndbdData.warn[NdbdWarnFan])
        {
            return True;
        }
    }

    return False;
}

u16eRspCode cmmnStartTrayChk(Tray *tray)
{
    if (NULL != tray->flowRecvCtrl) /*流程或保护未完成*/
    {
        recvCtrlFree(tray);
        return RspFlow;
    }

    if (NULL == tray->trayProtEntry)
    {
        return RspProt;
    }

    if (trayHasWarn(tray))
    {
        return RspWarn;
    }

    /*这里plcidx未必有效,不影响.todo,调整为传trayidx*/
    if (!tmprSmplBeOnline(tray->trayIdx))
    {
        return RspDiscTmpr;
    }

    if (BoxModeManu != tray->trayWorkMode)
    {
        return RspNdbdMntn;
    }

    if (!trayNdbdBeOnline(tray->plcIdx))
    {
        return RspDiscNdbd;
    }
    else
    {
        if (1 == tray->ndbdData.status[NdbdStaWorkMode])
        {
            return RspNdbdMntn;
        }
        else
        {
            u8eNdbdWarnType ndbdWarnIdx;

            for (ndbdWarnIdx=0; ndbdWarnIdx<NdbdWarnCri; ndbdWarnIdx++)
            {
                if (tray->ndbdData.warn[ndbdWarnIdx])
                {
                    return RspWarn;
                }
            }
        }
    }

    return RspOk;
}

u16eRspCode upMsgStartChnChk(UpFlowCtrlCmd *upCmd, Tray *tray)
{
    Channel *chn;
    Box *box;
    u16eRspCode rspCode;

    if (0 == upCmd->chnAmt)
    {
        Channel *sentry;
        Box *boxCri;

        for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
        {
            if (RspOk != (rspCode = boxStartChk(box)))
            {
                return rspCode;
            }
        }

        for (chn=tray->chn,sentry=&tray->chn[tray->trayChnAmt]; chn<sentry; chn++)
        {
            if (RspOk != (rspCode = chnStartChk(chn)))
            {
                return rspCode;
            }
        }
    }
    else
    {
        UpStartChnInd *chnInd;
        UpStartChnInd *sentry;
        u16 genChnIdx;

        for (chnInd=upCmd->startInd,sentry=chnInd+upCmd->chnAmt; chnInd<sentry; chnInd++)
        {
            if (chnInd->chnId >= tray->genChnAmt)
            {
                return RspChnIdx;
            }

            chn = upChnMap(tray, chnInd->chnId, &genChnIdx);
            if (BoxTypeSeriesWiSw == tray->boxCfg.boxType)
            {
                if (0 == genChnIdx)
                {
                    return RspChnIdx;
                }
            }
            else
            {
                if (0 != genChnIdx)
                {
                    return RspChnIdx;
                }
            }

            if (NULL==chn->flowStepEntry || chnInd->stepId>=chn->flowStepEntry->stepAmt)
            {
                return RspStep;
            }

            if (RspOk!=(rspCode = boxStartChk(chn->box)) || RspOk!=(rspCode = chnStartChk(chn)))
            {
                return rspCode;
            }
        }
    }

    return RspOk;
}

u16eRspCode upMsgFlowStartChk(UpFlowCtrlCmd *upCmd, u16 pldLen, DevMgr *dev)
{
    Tray *tray;
    u16eRspCode rspCode;
    u8 idx;

    if (pldLen != sizeof(UpFlowCtrlCmd) + upCmd->chnAmt*sizeof(UpStartChnInd)
        || upCmd->trayIdx>=dev->trayAmt)
    {
        return RspParam;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (RspOk != (rspCode = cmmnStartTrayChk(tray)))
    {
        return rspCode;
    }

    return upMsgStartChnChk(upCmd, tray);
}

void upMsgFlowStart(u8 *pld, u16 pldLen)
{
    UpFlowCtrlCmd *upCmd;
    UpStartChnInd *start;
    DevMgr *dev;
    Channel *chn;
    Tray *tray;
    Box *box;
    Box *boxCri;
    ChainS *chain;
    StepNode *stepNode;
    StepObj *step;
    u16 rspCode;

    dev = gDevMgr;
    upCmd = (UpFlowCtrlCmd *)pld;
    if (RspOk != (rspCode=upMsgFlowStartChk(upCmd, pldLen, dev)))
    {
        goto rspUp;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (0 == upCmd->chnAmt)  /*整盘*/
    {
        Channel *chnCri;

        if (BoxTypeParallel != tray->boxCfg.boxType)
        {
            Cell *cell;
            Cell *cellCri;

            for (cell=tray->cell,cellCri=cell+tray->trayCellAmt; cell<cellCri; cell++)
            {
                cell->chnProtBuf.idleProtEna = True;
            }
        }

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            chain = chn->flowStepEntry->stepList.next;
            stepNode = Container(StepNode, chain, chain);
            step = stepNode->stepObj;
            chn->chnProtBuf.idleProtEna = True;
            if (NULL != chn->bypsSeriesInd)  /*旁路*/
            {
                BypsSeriesInd *seriesInd;
                u8 idx;
            
                seriesInd = chn->bypsSeriesInd;
                memset(seriesInd, 0, sizeof(BypsSeriesInd));
                seriesInd->needStart = True;
                for (idx=0; idx<chn->chnCellAmt; idx++)
                {
                    BitSet(seriesInd->startCell, idx);
                    BitSet(seriesInd->runCell, idx);
                }
            }
            
            if (Ok == chnProtReverse(chn))
            {
                chn->chnStepId = 0;
                chn->crntStepNode = stepNode;
                chn->chnStepType = chn->upStepType = step->stepType;
                chn->capFlowCrnt = 0;
                chn->capLow = 0;
                chn->capStep = 0;
                chn->capCtnu = 0;
                chn->stepRunTimeCtnu = 0;
                chn->capChgTtl = 0;
                chn->capDisChgTtl = 0;
                chn->chgTypePre = ChgTypeCri;
                chnStepSwPre(chn);
                if (0 == getStepEndTime(stepNode->stepObj))
                {
                    /*如果工步截止时间为零,需要跳下个工步,并生成一条数据*/
                    /*不能直接发下个工步,等下条采样生成工步数据,再发下个工步*/
                    chn->chnStateMed = ChnStaMedEnd;
                    continue;
                }

                if (Ok == trayNpChnAlloc(tray, chn))
                {
                    chn->chnStateMed = ChnStaStart;
                    boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
                }
                else
                {
                    chn->chnStateMed = ChnStaNpWait;
                }

                if (NULL != chn->bypsSeriesInd)  /*旁路*/
                {
                    Cell *cell;
                    Cell *cellCri;
                
                    for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
                    {
                        cell->cellUpStepId = chn->chnStepId;
                        cell->cellUpStepType = chn->upStepType;
                        cell->cellCapCtnu = 0;
                        cell->cellCapFlowCrnt = 0;
                        cell->cellCapStep = 0;
                        cell->cellCapLow = 0;
                        cell->cellCapChg = 0;
                        cell->cellCapDisChg = 0;
                    }
                }
            }
        }
    }
    else if (BoxTypeSeriesWiSw != tray->boxCfg.boxType)
    {
        UpStartChnInd *chnInd;
        UpStartChnInd *sentry;
        u16 genChnIdx;

        for (chnInd=upCmd->startInd,sentry=&upCmd->startInd[upCmd->chnAmt]; chnInd<sentry; chnInd++)
        {
            chn = upChnMap(tray, chnInd->chnId, &genChnIdx);
            chain = chn->flowStepEntry->stepList.next;
            stepNode = Container(StepNode, chain, chain);
            step = stepNode->stepObj;
            chn->chnProtBuf.idleProtEna = True;
            if (NULL != chn->cell)
            {
                Cell *cell;
                Cell *cellCri;

                for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
                {
                    cell->chnProtBuf.idleProtEna = True;
                }
            }
            if (Ok == chnProtReverse(chn))
            {
                chn->chnStepId = 0;
                chn->crntStepNode = stepNode;
                chn->chnStepType = chn->upStepType = step->stepType;
                chn->capFlowCrnt = 0;
                chn->capLow = 0;
                chn->capStep = 0;
                chn->capCtnu = 0;
                chn->stepRunTimeCtnu = 0;
                chn->capChgTtl = 0;
                chn->capDisChgTtl = 0;
                chn->chgTypePre = ChgTypeCri;
                chnStepSwPre(chn);
                if (0 == getStepEndTime(stepNode->stepObj))
                {
                    /*如果工步截止时间为零,需要跳下个工步,并生成一条数据*/
                    /*不能直接发下个工步,等下条采样生成工步数据,再发下个工步*/
                    chn->chnStateMed = ChnStaMedEnd;
                    continue;
                }
                if (Ok == trayNpChnAlloc(tray, chn))
                {
                    chn->chnStateMed = ChnStaStart;
                    boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
                }
                else
                {
                    chn->chnStateMed = ChnStaNpWait;
                }
            }
        }
    }
    else  /*旁路串联的按电芯启动*/
    {
        UpStartChnInd *chnInd;
        UpStartChnInd *sentry;
        Channel *chnCri;
        BypsSeriesInd *seriesInd;
        u16 genChnIdx;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            chn->bypsSeriesInd->needStart = False;
        }

        for (chnInd=upCmd->startInd,sentry=&upCmd->startInd[upCmd->chnAmt]; chnInd<sentry; chnInd++)
        {
            chn = upChnMap(tray, chnInd->chnId, &genChnIdx);
            seriesInd = chn->bypsSeriesInd;
            if (!seriesInd->needStart)
            {
                memset(seriesInd, 0, sizeof(BypsSeriesInd));
                seriesInd->needStart = True;
            }

            genChnIdx -= 1;
            BitSet(seriesInd->startCell, genChnIdx);
            BitSet(seriesInd->runCell, genChnIdx);
            chn->cell[genChnIdx].chnProtBuf.idleProtEna = True;
        }

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            seriesInd = chn->bypsSeriesInd;
            if (seriesInd->needStart)
            {
                chn->chnProtBuf.idleProtEna = True;
                chain = chn->flowStepEntry->stepList.next;
                stepNode = Container(StepNode, chain, chain);
                step = stepNode->stepObj;
                if (Ok == chnProtReverse(chn))
                {
                    Cell *cell;
                    Cell *cellCri;

                    chn->chnStepId = 0;
                    chn->crntStepNode = stepNode;
                    chn->chnStepType = chn->upStepType = step->stepType;
                    chn->capFlowCrnt = 0;
                    chn->capLow = 0;
                    chn->capStep = 0;
                    chn->capCtnu = 0;
                    chn->stepRunTimeCtnu = 0;
                    chn->capChgTtl = 0;
                    chn->capDisChgTtl = 0;
                    chn->chgTypePre = ChgTypeCri;
                    chnStepSwPre(chn);
                    if (0 == getStepEndTime(stepNode->stepObj))
                    {
                        /*如果工步截止时间为零,需要跳下个工步,并生成一条数据*/
                        /*不能直接发下个工步,等下条采样生成工步数据,再发下个工步*/
                        chn->chnStateMed = ChnStaMedEnd;
                        continue;
                    }
                    if (Ok == trayNpChnAlloc(tray, chn))
                    {
                        chn->chnStateMed = ChnStaStart;
                        boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
                    }
                    else
                    {
                        chn->chnStateMed = ChnStaNpWait;
                    }

                    for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
                    {
                        if (BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
                        {
                            cell->cellUpStepId = chn->chnStepId;
                            cell->cellUpStepType = chn->upStepType;
                            cell->cellCapCtnu = 0;
                            cell->cellCapFlowCrnt = 0;
                            cell->cellCapStep = 0;
                            cell->cellCapLow = 0;
                            cell->cellCapChg = 0;
                            cell->cellCapDisChg = 0;
                        }
                    }
                }
            }
        }
    }

    for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
    {
        if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
        {
            boxCtrlTxTry(box);
        }
    }

rspUp:
    rspUpCmmn(UpCmdIdManuStart, upCmd->trayIdx, rspCode);
    return;
}

u16eRspCode upMsgFlowStopChk(UpFlowCtrlCmd *upCmd, u16 pldLen, DevMgr *dev)
{
    Tray *tray;
    Channel *chn;
    u16 idx;
    u16 chnIdx;

    if (pldLen != sizeof(UpFlowCtrlCmd) + Align16(upCmd->chnAmt)*2
        || upCmd->trayIdx>=dev->trayAmt)
    {
        return RspParam;
    }

    tray = &dev->tray[upCmd->trayIdx];
    for (idx=0; idx<upCmd->chnAmt; idx++)
    {
        if (upCmd->chnId[idx] >= tray->genChnAmt)
        {
            return RspChnIdx;
        }

        chn = upChnMap(tray, upCmd->chnId[idx], &chnIdx);
        if (BoxTypeSeriesWiSw == tray->boxCfg.boxType)
        {
            if (0 == chnIdx)
            {
                return RspChnIdx;
            }
            if (ChnStaRun!=chn->chnStateMed && ChnStaIdle!=chn->chnStateMed && ChnStaPause!=chn->chnStateMed)
            {
                return RspChnWoRun;
            }
        }
        else
        {
            if (0 != chnIdx)
            {
                return RspChnIdx;
            }
        }
    }

    return RspOk;
}
void upMsgFlowPause(u8 *pld, u16 pldLen)
{
    UpFlowCtrlCmd *upCmd;
    DevMgr *dev;
    Channel *chn;
    Tray *tray;
    Box *box;
    Box *boxCri;
    u16 rspCode;

    dev = gDevMgr;
    upCmd = (UpFlowCtrlCmd *)pld;
    if (RspOk != (rspCode=upMsgFlowStopChk(upCmd, pldLen, dev)))
    {
        goto rspUp;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (0 == upCmd->chnAmt)
    {
        Channel *chnCri;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (ChnStaRun == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpPauseReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaStart == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpPauseStartReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaNpWait == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpPauseNpReq;
            }

            if (NULL != chn->bypsSeriesInd)
            {
                BypsSeriesInd *seriesInd;

                seriesInd = chn->bypsSeriesInd;
                seriesInd->pausedCell = seriesInd->startCell & ~seriesInd->stopedCell;
                seriesInd->stopingCell = seriesInd->startCell;
            }
            trayNpChnFree(tray, chn);
        }
    }
    else if (BoxTypeSeriesWiSw != tray->boxCfg.boxType)
    {
        u16 idx;
        u16 chnIdx;

        for (idx=0; idx<upCmd->chnAmt; idx++)
        {
            chn = upChnMap(tray, upCmd->chnId[idx], &chnIdx);
            if (ChnStaRun == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpPauseReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaStart == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpPauseStartReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaNpWait==chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpPauseNpReq;
            }
            trayNpChnFree(tray, chn);
        }
    }
    else  /*暂停指定电芯视同暂停整串,目前也限定在运行态允许暂停*/
    {
        BypsSeriesInd *seriesInd;
        u16 idx;
        u16 genChnIdx;

        for (idx=0; idx<upCmd->chnAmt; idx++)
        {
            chn = upChnMap(tray, upCmd->chnId[idx], &genChnIdx);
            if (ChnStaRun == chn->chnStateMed)
            {
                seriesInd = chn->bypsSeriesInd;
                seriesInd->pausedCell = seriesInd->startCell & ~seriesInd->stopedCell;
                seriesInd->stopingCell = seriesInd->startCell;
                chn->chnStateMed = ChnStaUpPauseReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
                trayNpChnFree(tray, chn);
            }
        }
    }

    for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
    {
        if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
        {
            boxCtrlTxTry(box);
        }
    }

rspUp:
    rspUpCmmn(UpCmdIdManuPause, upCmd->trayIdx, rspCode);
    return;
}
void upMsgFlowStop(u8 *pld, u16 pldLen)
{
    UpFlowCtrlCmd *upCmd;
    DevMgr *dev;
    Tray *tray;
    Channel *chn;
    Box *box;
    Box *boxCri;
    u16 rspCode;

    dev = gDevMgr;
    upCmd = (UpFlowCtrlCmd *)pld;
    if (RspOk != (rspCode=upMsgFlowStopChk(upCmd, pldLen, dev)))
    {
        goto rspUp;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (0 == upCmd->chnAmt)
    {
        Channel *chnCri;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (ChnStaRun == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpStopReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaStart == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpStopStartReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaNpWait == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpStopNpReq;
            }
            else if (ChnStaPause == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaIdle;
                chn->chnStepId = StepIdNull;
                chn->chnStepType = StepTypeNull;
            }

            if (NULL != chn->bypsSeriesInd)
            {
                BypsSeriesInd *seriesInd;

                seriesInd = chn->bypsSeriesInd;
                seriesInd->stopingCell = seriesInd->stopedCell = seriesInd->startCell;
                seriesInd->pausedCell = 0;
            }
            trayNpChnFree(tray, chn);
        }
    }
    else if (BoxTypeSeriesWiSw != tray->boxCfg.boxType)
    {
        u16 idx;
        u16 chnIdx;

        for (idx=0; idx<upCmd->chnAmt; idx++)
        {
            chn = upChnMap(tray, upCmd->chnId[idx], &chnIdx);
            if (ChnStaRun == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpStopReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaStart == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpStopStartReq;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else if (ChnStaNpWait == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaUpStopNpReq;
            }
            else if (ChnStaPause == chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaIdle;
                chn->chnStepId = StepIdNull;
                chn->chnStepType = StepTypeNull;
            }
            trayNpChnFree(tray, chn);
        }
    }
    else  /*指定电芯停止目前只用于测试,所以限定在运行态才允许*/
    {
        u16 idx;
        u16 genChnIdx;
        BypsSeriesInd *seriesInd;

        for (idx=0; idx<upCmd->chnAmt; idx++)
        {
            chn = upChnMap(tray, upCmd->chnId[idx], &genChnIdx);
            if (ChnStaRun == chn->chnStateMed)
            {
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            genChnIdx -= 1;
            seriesInd = chn->bypsSeriesInd;
            BitSet(seriesInd->stopingCell, genChnIdx);
            BitSet(seriesInd->stopedCell, genChnIdx);
            BitClr(seriesInd->pausedCell, genChnIdx);
            BitClr(seriesInd->runCell, genChnIdx);
            BitClr(seriesInd->nmlEndCell, genChnIdx);
            BitClr(seriesInd->endingCell, genChnIdx);
            BitClr(seriesInd->waitCell, genChnIdx);
        }
    }

    for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
    {
        if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
        {
            boxCtrlTxTry(box);
        }
    }

rspUp:
    rspUpCmmn(UpCmdIdManuStop, upCmd->trayIdx, rspCode);
    return;
}

void upMsgFlowJump(u8 *pldCmd, u16 pldLenCmd)
{
    UpFlowCtrlCmd *cmd;
    UpCmmnAck *ack;
    u8 *buf;
    DevMgr *dev;
    u16 pldLenHope;
    u16 idx;

    cmd = (UpFlowCtrlCmd *)pldCmd;
    buf = sendUpBuf;
    dev = gDevMgr;

    ack = (UpCmmnAck *)(buf + sizeof(UpMsgHead));
    ack->trayIdx = cmd->trayIdx;
    ack->rsvd = 0;
    ack->rspCode = RspErr;

    pldLenHope = sizeof(UpFlowCtrlCmd) + Align16(cmd->chnAmt)*2;
    if (cmd->trayIdx>=dev->trayAmt || pldLenHope!=pldLenCmd)
    {
        Warn("err: basic\r\n");
        goto errHandler;
    }

    for (idx=0; idx<cmd->chnAmt; idx++)
    {
        if (cmd->chnId[idx] >= dev->tray[cmd->trayIdx].trayChnAmt)
        {
            Warn("err: chn id\r\n");
            goto errHandler;
        }
    }

    /*通道和跳转工步检查以后补充,todo*/
    ack->rspCode = RspOk;

errHandler:
    sendUpMsg(buf, sizeof(UpCmmnAck), UpCmdId(UpMsgGrpManu, UpMsgIdManuJump));
    return;
}

/*旁串,目前仅支持主通道下需要续接的电芯在同一个节奏,,todo,,以后完善不同节奏也能续接*/
u16eRspCode upMsgFlowStackChk(UpFlowCtrlCmd *upCmd, u16 pldLen, DevMgr *dev)
{
    Tray *tray;
    UpCtnuChnlInd *chnInd;
    UpCtnuChnlInd *fstInd;
    Channel *chn;
    Channel *chnCri;
    StepObj *step;
    StepNode *stepNode;
    ChainS *chain;
    u16eRspCode rspCode;
    u16 hopePldLen;
    u8 loopIdx;

    hopePldLen = sizeof(UpFlowCtrlCmd) + upCmd->chnAmt*sizeof(UpCtnuChnlInd);
    if (pldLen<hopePldLen || upCmd->trayIdx>=dev->trayAmt)
    {
        return RspParam;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (RspOk != (rspCode = cmmnStartTrayChk(tray)))
    {
        return rspCode;
    }

    if (0 == upCmd->chnAmt)
    {
        return RspChnIdx; /*todo,增加整盘续接,,不过貌似不会用到*/
    }
    else
    {
        u16 chnIdx;
        u16 genIdxTmp;
        u16 chnCnt;

        for (fstInd=chnInd=upCmd->ctnuInd, chnCnt=0; chnCnt<upCmd->chnAmt; chnCnt++)
        {
            if (chnInd->chnId >= tray->genChnAmt)
            {
                return RspChnIdx;
            }

            chn = upChnMap(tray, chnInd->chnId, &chnIdx);
            if (BoxTypeSeriesWiSw == tray->boxCfg.boxType)
            {
                if (0 == chnIdx)
                {
                    return RspChnIdx;
                }

            #if 1
                /*旁串的节奏检查,,不严谨,,不过目前够用,,todo,,支持后删除*/
                if (chn != upChnMap(tray, fstInd->chnId, &genIdxTmp))
                {
                    fstInd = chnInd;
                }

                if (fstInd->ctnuStepId!=chnInd->ctnuStepId || fstInd->loopAmt!=chnInd->loopAmt
                    || fstInd->stepRestTime!=chnInd->stepRestTime || fstInd->capStep!=chnInd->capStep)
                {
                    return RspParam;
                }

                if (chnInd->loopAmt && memcmp(fstInd->loopDscr, chnInd->loopDscr, sizeof(LoopDscr)*chnInd->loopAmt))
                {
                    return RspParam;
                }
            #endif
            }
            else
            {
                if (0 != chnIdx)
                {
                    return RspChnIdx;
                }
            }

            if (RspOk!=(rspCode = boxStartChk(chn->box)) || RspOk!=(rspCode = chnStartChk(chn)))
            {
                return rspCode;
            }

            if (chnInd->loopAmt != chn->flowStepEntry->loopStepAmt)
            {
                return RspLoopAmt;
            }

            hopePldLen += chnInd->loopAmt * sizeof(LoopDscr);
            if (pldLen < hopePldLen)
            {
                return RspParam;
            }

            for (loopIdx=0; loopIdx<chnInd->loopAmt; loopIdx++)
            {
                stepNode = findStepNode(chn->flowStepEntry, chn->loopDscr[loopIdx].loopStepId);
                step = stepNode->stepObj;
                if (chnInd->loopDscr[loopIdx].loopStepId != chn->loopDscr[loopIdx].loopStepId
                    || chnInd->loopDscr[loopIdx].jumpAmt > step->stepParam[1])
                {
                    return RspLoopParam;
                }
            }

            stepNode = findStepNode(chn->flowStepEntry, chnInd->ctnuStepId);
            if (NULL == stepNode)
            {
                return RspStep;
            }
            step = stepNode->stepObj;
            if (StepTypeLoop==step->stepType || getStepEndTime(step)<chnInd->stepRestTime)
            {
                return RspParam;
            }
            if (0 == chnInd->stepRestTime)
            {
                if (NULL == getNxtStep(chn, stepNode, False))
                {
                    return RspStep;
                }
            }

            chnInd = (UpCtnuChnlInd *)(&chnInd->loopDscr[chnInd->loopAmt]);
        }

        if (pldLen != hopePldLen)
        {
            return RspParam;
        }
    }
    return RspOk;
}

/*不检查暂停态,空闲态也能续接,,只要上位机愿意*/
void upMsgFlowStack(u8 *pld, u16 pldLen)
{
    UpFlowCtrlCmd *upCmd;
    UpCtnuChnlInd *chnInd;
    Box *box;
    Box *boxCri;
    DevMgr *dev;
    Channel *chn;
    Tray *tray;
    u16 rspCode;
    u8 stepIdx;
    u8 loopIdx;

    dev = gDevMgr;
    upCmd = (UpFlowCtrlCmd *)pld;
    if (RspOk != (rspCode=upMsgFlowStackChk(upCmd, pldLen, dev)))
    {
        goto rspUp;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (0 == upCmd->chnAmt)
    {
    }
    else if (BoxTypeSeriesWiSw != tray->boxCfg.boxType)
    {
        StepNode *stepNode;
        u16 genChnIdx;
        u16 chnCnt;

        for (chnInd=upCmd->ctnuInd, chnCnt=0; chnCnt<upCmd->chnAmt; chnCnt++)
        {
            chn = upChnMap(tray, chnInd->chnId, &genChnIdx);
            if (0 == chnInd->stepRestTime)
            {
                stepNode = findStepNode(chn->flowStepEntry, chnInd->ctnuStepId);
                chn->crntStepNode = getNxtStep(chn, stepNode, True);
                chn->chnStepId = chn->crntStepNode->stepId;
                chn->stepRunTimeCtnu = 0;
                chn->capStep = chn->capCtnu = 0;
            }
            else
            {
                chn->chnStepId = chnInd->ctnuStepId;
                chn->crntStepNode = findStepNode(chn->flowStepEntry, chn->chnStepId);
                chn->stepRunTimeCtnu = getStepEndTime(chn->crntStepNode->stepObj) - chnInd->stepRestTime;
                chn->capStep = chn->capCtnu = chnInd->capStep;
            }

            for (loopIdx=0; loopIdx<chnInd->loopAmt; loopIdx++)
            {
                chn->loopDscr[loopIdx].jumpAmt = chnInd->loopDscr[loopIdx].jumpAmt;
            }

            chn->capFlowCrnt = chnInd->capFlowCrnt;
            for (stepIdx=chn->chnStepId; StepIdNull!=stepIdx; stepIdx--)
            {
                stepNode = findStepNode(chn->flowStepEntry, stepIdx);
                chn->chgTypePre = stepType2ChgType(stepNode->stepObj->stepType);
                if (ChgTypeCri != chn->chgTypePre)
                {
                    break;
                }
            }

            chn->chnProtBuf.idleProtEna = True;
            if (NULL != chn->cell)
            {
                Cell *cell;
                Cell *cellCri;

                for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
                {
                    cell->chnProtBuf.idleProtEna = True;
                }
            }
            if (Ok == chnProtReverse(chn))
            {
                chn->chnStepType = chn->upStepType = chn->crntStepNode->stepObj->stepType;
                chn->capLow = 0;
                chn->capChgTtl = 0;
                chn->capDisChgTtl = 0;
                chnStepSwPre(chn);
                if (Ok == trayNpChnAlloc(tray, chn))
                {
                    chn->chnStateMed = ChnStaStart;
                    boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
                }
                else
                {
                    chn->chnStateMed = ChnStaNpWait;
                }
            }

            chnInd = (UpCtnuChnlInd *)(&chnInd->loopDscr[chnInd->loopAmt]);
        }
    }
    else  /*todo,,先完全独立以免影响,之后合并上面*/
    {
        StepNode *stepNode;
        Channel *chnCri;
        Cell *cell;
        Cell *cellCri;
        BypsSeriesInd *seriesInd;
        u16 genChnIdx;
        u16 chnCnt;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            chn->bypsSeriesInd->needStart = False;
        }

        for (chnInd=upCmd->ctnuInd, chnCnt=0; chnCnt<upCmd->chnAmt; chnCnt++)
        {
            chn = upChnMap(tray, chnInd->chnId, &genChnIdx);
            genChnIdx -= 1;
            seriesInd = chn->bypsSeriesInd;
            cell = chn->cell + genChnIdx;
            cell->chnProtBuf.idleProtEna = True;
            cell->cellCapCtnu = chnInd->capFlowCrnt;
            if (!seriesInd->needStart)
            {
                memset(seriesInd, 0, sizeof(BypsSeriesInd));
                seriesInd->needStart = True;
                seriesInd->hasCtnuStepId = False;
            }

            BitSet(seriesInd->startCell, genChnIdx);
            if (0 == chnInd->stepRestTime)
            {
                BitSet(seriesInd->nmlEndCell, genChnIdx);
            }
            else
            {
                BitSet(seriesInd->runCell, genChnIdx);
            }

            for (loopIdx=0; loopIdx<chnInd->loopAmt; loopIdx++)
            {
                chn->loopDscr[loopIdx].jumpAmt = chnInd->loopDscr[loopIdx].jumpAmt;
            }

            if (!seriesInd->hasCtnuStepId)
            {
                chn->chnStepId = chnInd->ctnuStepId;
                chn->capFlowCrnt = chnInd->capFlowCrnt;
                if (0 != chnInd->stepRestTime)
                {
                    chn->crntStepNode = findStepNode(chn->flowStepEntry, chn->chnStepId);
                    chn->stepRunTimeCtnu = getStepEndTime(chn->crntStepNode->stepObj) - chnInd->stepRestTime;
                    chn->capStep = chn->capCtnu = chnInd->capStep;
                    seriesInd->hasCtnuStepId = True;
                }
            }

            chnInd = (UpCtnuChnlInd *)(&chnInd->loopDscr[chnInd->loopAmt]);
        }

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (chn->bypsSeriesInd->needStart)
            {
                chn->chnProtBuf.idleProtEna = True;
                if (!seriesInd->hasCtnuStepId)
                {
                    stepNode = findStepNode(chn->flowStepEntry, chn->chnStepId);
                    chn->crntStepNode = getNxtStep(chn, stepNode, True);
                    chn->chnStepId = chn->crntStepNode->stepId;
                    chn->stepRunTimeCtnu = 0;
                    chn->capStep = chn->capCtnu = 0;
                    chn->bypsSeriesInd->runCell = chn->bypsSeriesInd->nmlEndCell;
                    chn->bypsSeriesInd->nmlEndCell = 0;
                }

                for (stepIdx=chn->chnStepId; StepIdNull!=stepIdx; stepIdx--)
                {
                    stepNode = findStepNode(chn->flowStepEntry, stepIdx);
                    chn->chgTypePre = stepType2ChgType(stepNode->stepObj->stepType);
                    if (ChgTypeCri != chn->chgTypePre)
                    {
                        break;
                    }
                }

                if (Ok == chnProtReverse(chn))
                {
                    chn->chnStepType = chn->upStepType = chn->crntStepNode->stepObj->stepType;
                    chn->capLow = 0;
                    chn->capChgTtl = 0;
                    chn->capDisChgTtl = 0;
                    chnStepSwPre(chn);
                    if (Ok == trayNpChnAlloc(tray, chn))
                    {
                        chn->chnStateMed = ChnStaStart;
                        boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
                    }
                    else
                    {
                        chn->chnStateMed = ChnStaNpWait;
                    }
                }

                for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
                {
                    if (BitIsSet(seriesInd->runCell, cell->cellIdxInChn))
                    {
                        cell->cellUpStepId = chn->chnStepId;
                        cell->cellUpStepType = chn->upStepType;
                        cell->cellCapCtnu = chn->capCtnu;
                        cell->cellCapStep = chn->capStep;
                        cell->cellCapLow = 0;
                        cell->cellCapChg = 0;
                        cell->cellCapDisChg = 0;
                    }
                }
            }
        }
    }

    for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
    {
        if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
        {
            boxCtrlTxTry(box);
        }
    }

rspUp:
    rspUpCmmn(UpCmdIdManuCtnuStack, upCmd->trayIdx, rspCode);
    return;
}

u16eRspCode upMsgFlowCtnuChk(UpFlowCtrlCmd *upCmd, u16 pldLen, DevMgr *dev)
{
    Tray *tray;
    Channel *chn;

    return RspParam;
    if (pldLen != sizeof(UpFlowCtrlCmd) + Align16(upCmd->chnAmt)*sizeof(u16)
        || upCmd->trayIdx>=dev->trayAmt)
    {
        return RspParam;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (trayHasWarn(tray))
    {
        return RspWarn;
    }

    /*这里plcidx未必有效,不影响.todo,调整为传trayidx*/
    if (!tmprSmplBeOnline(tray->trayIdx))
    {
        return RspDiscTmpr;
    }

    if (!trayNdbdBeOnline(tray->plcIdx))
    {
        return RspDiscNdbd;
    }

    if (0 == upCmd->chnAmt)
    {
        Channel *chnCri;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
        }
    }
    else
    {
        u16 idx;
        u16 chnIdx;

        for (idx=0; idx<upCmd->chnAmt; idx++)
        {
            if (upCmd->chnId[idx] >= tray->genChnAmt)
            {
                return RspChnIdx;
            }
            
            chn = upChnMap(tray, upCmd->chnId[idx], &chnIdx);
        }
    }
    return RspOk;
}

void upMsgFlowCtnu(u8 *pld, u16 pldLen)
{
    UpFlowCtrlCmd *upCmd;
    DevMgr *dev;
    u16 rspCode;

    dev = gDevMgr;
    upCmd = (UpFlowCtrlCmd *)pld;
    if (RspOk != (rspCode=upMsgFlowCtnuChk(upCmd, pldLen, dev)))
    {
        goto rspUp;
    }

rspUp:
    rspUpCmmn(UpCmdIdManuCtnu, upCmd->trayIdx, rspCode);
    return;
}

void upMsgWarnDel(u8 *pldCmd, u16 pldLenCmd)
{
    Tray *tray;
    Channel *chn;
    Channel *chnCri;
    TrayProtMgr *trayProtMgr;
    ChnProtBuf *protBuf;
    Cell *cell;
    Cell *cellCri;
    u8 trayIdx;

    trayIdx = *pldCmd;
    tray = &gDevMgr->tray[trayIdx];
    trayProtMgr = &tray->trayProtMgr;
    tray->trayWarnPres = False;
    memset(trayProtMgr->mixProtHpn, 0, MixExpAmt*sizeof(u32));
    memset(trayProtMgr->policyActNeed, 0, Align8(PolicyCri));
    memset(trayProtMgr->policyActOver, 0, Align8(PolicyCri));
    trayProtMgr->slotTmprCrnt.tmprInvalidCnt = 0;
    trayProtMgr->smokeCtnuHpnCnt = 0;
    trayProtMgr->coCtnuHpnCnt = 0;
    trayProtMgr->allSlotTmprUpLmtCnt = 0;
    trayProtMgr->trayCausePre = CcNone;
    trayProtMgr->busySlotTmprLowLmtCnt = 0;

    TimerStop(&tray->trayProtMgr.protPolicyTmr);
    for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        protBuf = &chn->chnProtBuf;
        protBuf->mixSubHpnBitmap = 0;
        protBuf->cellTmprCrnt.tmprInvalidCnt = 0;
        protBuf->idleVolCtnuSmlRiseCnt = 0;
        protBuf->idleVolCtnuSmlDownCnt = 0;
        protBuf->idleTmprUpSmlCnt = 0;
        protBuf->idleCurLeakCnt = 0;
        protBuf->allChnTmprUpLmtCnt = 0;
        protBuf->busyChnTmprLowLmtCnt = 0;
        protBuf->idleTimeStampSec = sysTimeSecGet();
        protBuf->idleVolBaseValid = False;
        protBuf->flowIdleVolIntvlRiseBaseValid = False;
        if (NULL != chn->cell)
        {
            for (cell=chn->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                protBuf = &cell->chnProtBuf;
                protBuf->mixSubHpnBitmap = 0;
                protBuf->cellTmprCrnt.tmprInvalidCnt = 0;
                protBuf->idleVolCtnuSmlRiseCnt = 0;
                protBuf->idleVolCtnuSmlDownCnt = 0;
                protBuf->idleTmprUpSmlCnt = 0;
                protBuf->idleCurLeakCnt = 0;
                protBuf->allChnTmprUpLmtCnt = 0;
                protBuf->busyChnTmprLowLmtCnt = 0;
                protBuf->idleTimeStampSec = sysTimeSecGet();
                protBuf->idleVolBaseValid = False;
                protBuf->flowIdleVolIntvlRiseBaseValid = False;
            }
        }
    }

    trayNdbdCtrl(trayIdx, NdbdSetWarnDel, 1);
    rspUpCmmn(UpCmdIdManuWarnDel, trayIdx, RspOk);
    return;
}

u16eRspCode upMsgSeriesCellSwChk(UpSeriesSwCmd *upCmd, u16 pldLen, DevMgr *dev)
{
    Tray *tray;
    Channel *chn;
    u16 idx;
    u16 chnIdx;

    if (pldLen != sizeof(UpFlowCtrlCmd) + Align16(upCmd->chnAmt)*2
        || upCmd->trayIdx>=dev->trayAmt)
    {
        return RspParam;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (BoxTypeSeriesWiSw != tray->boxCfg.boxType)
    {
        return RspParam;
    }

    for (idx=0; idx<upCmd->chnAmt; idx++)
    {
        if (upCmd->chnId[idx] >= tray->genChnAmt)
        {
            return RspChnIdx;
        }

        chn = upChnMap(tray, upCmd->chnId[idx], &chnIdx);
        if (0 == chnIdx)
        {
            return RspChnIdx;
        }
        else if (chn->chnStateMed > ChnStaPause)
        {
            return RspChnWiRun;
        }
    }

    return RspOk;
}

void upMsgSeriesCellSw(u8 *pld, u16 pldLen)
{
    UpSeriesSwCmd *upCmd;
    DevMgr *dev;
    Tray *tray;
    Channel *chn;
    Box *box;
    Box *boxCri;
    BypsSeriesInd *seriesInd;
    u16 rspCode;
    u8 idx;

    dev = gDevMgr;
    upCmd = (UpSeriesSwCmd *)pld;
    if (RspOk != (rspCode=upMsgSeriesCellSwChk(upCmd, pldLen, dev)))
    {
        goto rspUp;
    }

    tray = &dev->tray[upCmd->trayIdx];
    if (0 == upCmd->chnAmt)
    {
        Channel *chnCri;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            seriesInd = chn->bypsSeriesInd;
            boxCtrlAddSeriesCell(chn->box, chn->chnIdxInBox, upCmd->swInd);
            if (1 == upCmd->swInd)  /*切入*/
            {
                for (idx=0; idx<chn->chnCellAmt; idx++)
                {
                    BitSet(seriesInd->idleSwInCell, idx);
                    BitSet(seriesInd->idleSwCell, idx);
                }
            }
            else
            {
                seriesInd->idleSwInCell = 0;
                for (idx=0; idx<chn->chnCellAmt; idx++)
                {
                    BitSet(seriesInd->idleSwCell, idx);
                }
            }
        }
    }
    else
    {
        u16 genChnIdx;

        for (idx=0; idx<upCmd->chnAmt; idx++)
        {
            chn = upChnMap(tray, upCmd->chnId[idx], &genChnIdx);
            boxCtrlAddSeriesCell(chn->box, chn->chnIdxInBox, upCmd->swInd);
            seriesInd = chn->bypsSeriesInd;
            BitSet(seriesInd->idleSwCell, genChnIdx-1);
            if (1 == upCmd->swInd)  /*切入*/
            {
                BitSet(seriesInd->idleSwInCell, idx);
            }
            else
            {
                BitClr(seriesInd->idleSwInCell, idx);
            }
        }
    }

    for (box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
    {
        if (0 != box->seriesSwWaitSend.boxCtrlInd.chnSelectInd)
        {
            boxCtrlCellTxTry(box);
        }
    }

rspUp:
    rspUpCmmn(UpCmdIdManuCellSw, upCmd->trayIdx, rspCode);
    return;
}

u16eRspCode upMsgRgbCtrlChk(UpRgbCtrlCmd *upCmd, u16 pldLen)
{
    u8 rgbInd;

    if (pldLen != sizeof(UpRgbCtrlCmd))
    {
        return RspParam;
    }

    rgbInd = upCmd->rgbInd;
    if (BitIsSet(rgbInd, 3))
    {
        if (BitIsSet(rgbInd, 0) || BitIsSet(rgbInd, 1))
        {
            return RspRgbCtrl;
        }
    }
    else
    {
        if (!(0==rgbInd || 1==rgbInd || 2==rgbInd || 4==rgbInd))
        {
            return RspRgbCtrl;
        }
    }

    return RspOk;
}

void upMsgRgbCtrl(u8 *pld, u16 pldLen)
{
    UpRgbCtrlCmd *upCmd;
    u16 rspCode;
    b8 buzzer;
    b8 green;
    b8 yellow;
    b8 red;

    upCmd=(UpRgbCtrlCmd *)pld;
    if (RspOk != (rspCode=upMsgRgbCtrlChk(upCmd, pldLen)))
    {
        goto rspUp;
    }

    green = BitVal(upCmd->rgbInd, 0);
    yellow = BitVal(upCmd->rgbInd, 1);
    red = BitVal(upCmd->rgbInd, 2);
    buzzer = BitVal(upCmd->rgbInd, 3);

    /*目前板子有三组三色灯,一列一灯情况下，三组都点*/
#ifdef DebugVersion
#else
    AlarmLight_Switch(0, yellow, green, red);
    Buzzer_Switch(0, buzzer);

    AlarmLight_Switch(1, yellow, green, red);
    Buzzer_Switch(1, buzzer);

    AlarmLight_Switch(2, yellow, green, red);
    Buzzer_Switch(2, buzzer);
#endif

rspUp:
    rspUpCmmn(UpCmdIdManuRgbCtrl, 0, rspCode);
    return;
}

/*目前仅支持并联*/
u16eRspCode upMsgBoxChkChk(UpBoxChkCmd *upCmd, u16 pldLen)
{
    Tray *tray;
    Box *box;
    u16 idx;

    if (pldLen!=sizeof(UpBoxChkCmd)+sizeof(BoxChkParam)*upCmd->chnAmt || upCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = gDevMgr->tray + upCmd->trayIdx;
    if (upCmd->boxIdx>=tray->boxAmt || upCmd->chkStage>=BoxChkStageCri)
    {
        return RspParam;
    }

    box = tray->box + upCmd->boxIdx;
    if (upCmd->chnAmt>box->boxChnAmt || BoxTypeParallel!=tray->box->boxType)
    {
        return RspParam;
    }

    for (idx=0; idx<upCmd->chnAmt; idx++)
    {
        if (upCmd->param[idx].chnIdx >= box->boxChnAmt)
        {
            return RspParam;
        }
    }

    return RspOk;
}

/*下位机自检,属于透传类型,目前仅限于并联*/
void upMsgBoxChk(u8 *pld, u16 pldLen)
{
    UpBoxChkCmd *upCmd;
    CanAuxCmdBuf *auxBuf;
    BoxMsgHead *boxHead;
    BoxChkCmd *boxCmd;
    Tray *tray;
    Box *box;
    u16eRspCode rspCode;

    upCmd = (UpBoxChkCmd *)pld;
    if (RspOk != (rspCode = upMsgBoxChkChk(upCmd, pldLen))
        || RspOk != (rspCode = NULL==(auxBuf=allocAuxCanBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdManuBoxChk, upCmd->trayIdx, rspCode);
        return;
    }

    tray = gDevMgr->tray + upCmd->trayIdx;
    box = tray->box + upCmd->boxIdx;
    setCaliAuxCanBuf(auxBuf, box, UpCmdIdCaliNtfy);
    boxHead = (BoxMsgHead *)auxBuf->msgBuf;
    boxHead->msgId = BoxMsgBoxChk;
    boxHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxChkCmd) + sizeof(BoxChkParam)*upCmd->chnAmt;
    boxCmd = (BoxChkCmd *)boxHead->payload;
    boxCmd->chkStage = upCmd->chkStage;
    boxCmd->beTouch = upCmd->beTouch;
    boxCmd->chnAmt = upCmd->chnAmt;
    memcpy(boxCmd->param, upCmd->param, sizeof(BoxChkParam)*upCmd->chnAmt);
    boxAuxTxTry(box->canIdx, auxBuf);
    return;
}

void upMsgDispManu(u8eUpMsgIdManu msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdManuCri)
    {
        gUpItfCb->upMsgProcManu[msgId](pld, pldLen);
    }

    return;
}

void _____begin_of_upgrade_msg_____(){}

b8 trayBoxMntnAble(u8    trayIdx)
{
    Tray *tray;
    Channel *chn;
    Channel *chnCri;

    tray = &gDevMgr->tray[trayIdx];
    for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        if (ChnStaIdle != chn->chnStateMed)
        {
            return False;
        }
    }
    return True;
}
                         
u16eRspCode upCfgReadChk(UpCfgReadCmd *upCmd, u16 pldLen)
{
    if (pldLen!=sizeof(UpCfgReadCmd) || upCmd->devType>=UpUpdDevCri)
    {
        return RspParam;
    }

    if (UpUpdDevBox == upCmd->devType)
    {
        Tray *tray;

        if (upCmd->trayIdx >= gDevMgr->trayAmt)
        {
            return RspParam;
        }

        tray = &gDevMgr->tray[upCmd->trayIdx];
        if (upCmd->devId >= tray->boxAmt)
        {
            return RspParam;
        }
    }

    return RspOk;
}

void upMsgCfgRead(u8 *pld, u16 pldLen)
{
    UpCfgReadCmd *upCmd;
    Tray *tray;
    u16eRspCode rspCode;

    upCmd = (UpCfgReadCmd *)pld;
    if (RspOk != (rspCode=upCfgReadChk(upCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdUpdCfgRead, upCmd->trayIdx, rspCode);
        return;
    }

    if (UpUpdDevBox == upCmd->devType)
    {
        CanAuxCmdBuf *auxBuf;
        BoxMsgHead *boxHead;
        Box *box;

        if (NULL == (auxBuf = allocAuxCanBuf()))
        {
            rspUpCmmn(UpCmdIdUpdCfgRead, upCmd->trayIdx, RspNoRes);
            return;
        }

        tray = &gDevMgr->tray[upCmd->trayIdx];
        box = &tray->box[upCmd->devId];
        setCaliAuxCanBuf(auxBuf, box, UpCmdIdUpdCfgRead);
        boxHead = (BoxMsgHead *)auxBuf->msgBuf;
        boxHead->msgId = BoxMsgCfgRead;
        boxHead->msgLen = sizeof(BoxMsgHead);
        boxAuxTxTry(box->canIdx, auxBuf);
    }
    else if (UpUpdDevMedium == upCmd->devType)
    {
        rspUpCmmn(UpCmdIdUpdCfgRead, upCmd->trayIdx, rspCode); /*todo,,待补充*/
    }

    return;
}

u16eRspCode upCfgSetChk(upCfgSetCmd *upCmd, u16 pldLen)
{
    u16 hopePldLen;

    hopePldLen = sizeof(upCfgSetCmd);
    if (pldLen<=hopePldLen || upCmd->devType>=UpUpdDevCri)
    {
        return RspParam;
    }

    if (UpUpdDevBox == upCmd->devType)
    {
        Tray *tray;
        UpCfgTlv *cfgTlv;
        u8 tlvSize;
        u8eBoxCfgType cfgType;

        if (upCmd->trayIdx >= gDevMgr->trayAmt)
        {
            return RspParam;
        }

        tray = &gDevMgr->tray[upCmd->trayIdx];
        if (upCmd->devId >= tray->boxAmt)
        {
            return RspParam;
        }

        for (; hopePldLen<pldLen; hopePldLen+=tlvSize)
        {
            cfgTlv = (UpCfgTlv *)((u8 *)upCmd + hopePldLen);
            if (cfgTlv->cfgType<0x0100 || cfgTlv->cfgType>0x0109)
            {
                return RspParam;
            }

            cfgType = cfgTlv->cfgType - BoxCfgBase;
            tlvSize = BoxCfgVolRise==cfgType||BoxCfgCurDown==cfgType ? 16 : 12;
            if (tlvSize != cfgTlv->cfgLen+sizeof(UpCfgTlv))
            {
                return RspParam;
            }
            if (16 == tlvSize)
            {
                if (cfgTlv->cfgVal[1]>65535 || cfgTlv->cfgVal[2]>65535)
                {
                    return RspParam;
                }
            }
        }

        if (pldLen != hopePldLen)
        {
            return RspParam;
        }
    }

    return RspOk;
}

void upMsgCfgSet(u8 *pld, u16 pldLen)
{
    upCfgSetCmd *upCmd;
    Tray *tray;
    u16eRspCode rspCode;

    upCmd = (upCfgSetCmd *)pld;
    if (RspOk != (rspCode=upCfgSetChk(upCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdUpdCfgSet, upCmd->trayIdx, rspCode);
        return;
    }

    if (UpUpdDevBox == upCmd->devType)
    {
        CanAuxCmdBuf *auxBuf;
        BoxMsgHead *boxHead;
        BoxCfgInd *boxCmd;
        Box *box;
        UpCfgTlv *tlv;
        UpCfgTlv *sentry;
        u32 *param;
        u8eBoxCfgType cfgType;

        if (NULL == (auxBuf = allocAuxCanBuf()))
        {
            rspUpCmmn(UpCmdIdUpdCfgSet, upCmd->trayIdx, RspNoRes);
            return;
        }

        tray = &gDevMgr->tray[upCmd->trayIdx];
        box = &tray->box[upCmd->devId];
        setCaliAuxCanBuf(auxBuf, box, UpCmdIdUpdCfgSet);
        boxHead = (BoxMsgHead *)auxBuf->msgBuf;
        boxHead->msgId = BoxMsgCfgSet;
        boxHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCfgInd);
        boxCmd = (BoxCfgInd *)boxHead->payload;
        boxCmd->funcEnable = boxCmd->funcSelect = 0;
        sentry = (UpCfgTlv *)(pld+pldLen);
        for (tlv=upCmd->cfgTlv,param=boxCmd->param; tlv<sentry; tlv=(UpCfgTlv *)((u8 *)(tlv+1) + tlv->cfgLen))
        {
            cfgType = tlv->cfgType - BoxCfgBase;
            BitSet(boxCmd->funcSelect, cfgType);
            if (0 != (tlv->cfgVal[0] & 0xff))
            {
                BitSet(boxCmd->funcEnable, cfgType);
                *param = tlv->cfgVal[1];
                if (12 == tlv->cfgLen) /*使能+两个参数*/
                {
                    *param /= 1000;  /*ms-->s*/
                    *param &= 0xffff;  /*前两个字节时间*/
                    *param += tlv->cfgVal[2] << 16;  /*后两字节压差或流差*/
                }

                param++;
                boxHead->msgLen += sizeof(u32);
            }
        }

        boxAuxTxTry(box->canIdx, auxBuf);
    }
    else if (UpUpdDevMedium == upCmd->devType)
    {
        rspUpCmmn(UpCmdIdUpdCfgSet, upCmd->trayIdx, rspCode); /*todo,,待补充*/
    }

    return;
}

/*todo,增加防呆,例如模式切换,状态是否允许*/
u16eRspCode upMsgUpdSetupChk(UpUpdSetupCmd *upCmd, u16 pldLen)
{
    if (upCmd->trayIdx>=gDevMgr->trayAmt || upCmd->devType>=UpUpdDevCri
        || upCmd->fileType>=UpdateFileCri)
    {
        return RspParam;
    }

    if (gUpItfCb->updMgr.updWorking)
    {
        return RspBusy;
    }

    if (!trayBoxMntnAble(upCmd->trayIdx))
    {
        return RspChnBeRun;
    }
    return RspOk;
}

/*含临时应答*/
void rspUpUpdSetup(u16eRspCode rspCode)
{
    UpUpdSetupAck *upAck;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpUpdSetupAck *)(buf + sizeof(UpMsgHead));
    upAck->rspCode = rspCode;
    upAck->pageSize = upAck->fileSize = upAck->familyVer = 0;
    sendUpMsg(buf, sizeof(UpUpdSetupAck), UpCmdIdUpdSetup);
    return;
}

/*配置升级类的设备地址映射*/
/*todo,目前没有切换板,采样板与主下一比一,不用换算,以后要补充切换板*/
Box *UpUpdCanDevMap(Tray *tray, u8eBoxDevType devType, u16 idxInTray, u16 *mapIdx)
{
    *mapIdx = idxInTray;
    return &tray->box[idxInTray];
}

/*mapAddr--通讯地址*/
Uart *UpUpdUartDevMap(Tray *tray, u8eUartDevType devType, u16 idxInTray, u8 *mapAddr)
{
    Uart *uart;
    DevMgr *devMgr;

    devMgr = gDevMgr;
    if (UartTmprSmpl == devType)
    {
        TmprSmpl *tmprSmpl;

        tmprSmpl = &devMgr->tmprSmpl[tray->tmprSmplIdxBase+idxInTray];
        uart = &devMgr->uart[tmprSmpl->uartIdx];
        *mapAddr = tmprSmpl->cmmuAddr;
    }
    else /*todo,目前没有针床下位机,项目需要时候加*/
    {
        NdbdMcu *ndbdMcu;

        ndbdMcu = &devMgr->ndbdMcu[tray->trayIdx];
        uart = &devMgr->uart[ndbdMcu->uartIdx];
        *mapAddr = ndbdMcu->cmmuAddr;
    }

    return uart;
}

void upMsgUpdSetup(u8 *pld, u16 pldLen)
{
    UpUpdSetupCmd *upCmd;
    UpdTypeMap *updTypeMap;
    Tray *tray;
    UpdMgr *updMgr;
    u16eRspCode rspCode;

    upCmd = (UpUpdSetupCmd *)pld;
    if (RspOk != (rspCode=upMsgUpdSetupChk(upCmd, pldLen)))
    {
        rspUpUpdSetup(rspCode);
        return;
    }

    if (UpUpdDevMedium == upCmd->devType)
    {
        rspUpUpdSetup(RspRetry); /*临时应答,留出发包的时间后重启*/
        timerStart(&gDevMgr->devAppRstTmr, TidAppSw2Boot, 100, WiReset);
        return;
    }

    updMgr = &gUpItfCb->updMgr;
    updMgr->devIdx = upCmd->devId;
    updMgr->devType = upCmd->devType;
    updMgr->fileSize = upCmd->fileSize;
    updMgr->fileType = upCmd->fileType;
    updMgr->isUpload = upCmd->isUpload;
    updMgr->trayIdx = upCmd->trayIdx;

    tray = &gDevMgr->tray[upCmd->trayIdx];
    updTypeMap = &tray->updTypeMap[upCmd->devType];
    if (ItfTypeCan == updTypeMap->cmmuItfType)
    {
        CanAuxCmdBuf *auxBuf;
        BoxMsgHead *boxHead;
        BoxUpdSetupCmd *boxCmd;
        Box *box;
        u16 mapIdx;

        if (NULL == (auxBuf = allocAuxCanBuf()))
        {
            rspUpUpdSetup(RspNoRes);
            return;
        }

        box = UpUpdCanDevMap(tray, updTypeMap->devType, upCmd->devId, &mapIdx);
        setCaliAuxCanBuf(auxBuf, box, UpCmdIdUpdSetup);
        boxHead = (BoxMsgHead *)auxBuf->msgBuf;
        boxHead->msgId = BoxMsgUpdSetup;
        boxHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxUpdSetupCmd);
        boxCmd = (BoxUpdSetupCmd *)boxHead->payload;
        boxCmd->boxDevType = updTypeMap->devType;
        boxCmd->boxDevIdx = mapIdx;
        boxCmd->fileType = upCmd->fileType;
        boxCmd->isUpload = upCmd->isUpload;
        mem2Copy(boxCmd->fileSize, (u16 *)&upCmd->fileSize, sizeof(u32));
        boxAuxTxTry(box->canIdx, auxBuf);
        box->boxWorkMode = BoxModeMntnBoxUpd;
        traySmplMgrRst(tray);
    }
    else  /*todo,增加温度盒等uart设备的维护模式*/
    {
    #if 0  /*todo, 不是无效代码,只是目前未用*/
        UartBlockCmdBuf *blkBuf;
        UartMsgHead *uartHead;
        UartUpdSetupCmd *uartCmd;
        Uart *uart;
        u8 cmmuAddr;

        if (NULL == (blkBuf = allocUartBlockBuf()))
        {
            rspUpUpdSetup(RspNoRes);
            return;
        }

        uart = UpUpdUartDevMap(tray, updTypeMap->devType, upCmd->devId, &cmmuAddr);
        setUartBlockBuf(blkBuf, upCmd->trayIdx, UpCmdIdUpdSetup, cmmuAddr,
                        updTypeMap->devType, UartMsgIdCmmn(UartMsgUpdSetup));
        uartHead = (UartMsgHead *)blkBuf->msgBuf;
        uartHead->pldLen = sizeof(UartUpdSetupCmd);
        uartCmd = (UartUpdSetupCmd *)uartHead->payload;
        uartCmd->isUpload = upCmd->isUpload;
        uartCmd->fileType = upCmd->fileType;
        uartCmd->fileSize = upCmd->fileSize;
        uartTransTxTry(uart->uartIdx, blkBuf);
    #endif
    }

    updMgr->updWorking = True;
    return;
}

u16eRspCode upMsgUpdDldChk(UpUpdDldCmd *upCmd, u16 pldLen)
{
    if (upCmd->trayIdx>=gDevMgr->trayAmt || upCmd->pldSize>gUpItfCb->updMgr.pageSize
        || upCmd->devType>=UpUpdDevCri || upCmd->fileType>=UpdateFileCri)
    {
        return RspParam;
    }

    return RspOk;
}

void upMsgUpdDld(u8 *pld, u16 pldLen)
{
    UpUpdDldCmd *upCmd;
    UpdTypeMap *updTypeMap;
    Tray *tray;
    UpdMgr *updMgr;
    u16eRspCode rspCode;

    upCmd = (UpUpdDldCmd *)pld;
    if (RspOk != (rspCode=upMsgUpdDldChk(upCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdUpdDld, upCmd->trayIdx, rspCode);
        return;
    }

    if (UpUpdDevMedium == upCmd->devType)
    {
        return;
    }

    tray = &gDevMgr->tray[upCmd->trayIdx];
    updMgr = &gUpItfCb->updMgr;
    updTypeMap = &tray->updTypeMap[upCmd->devType];
    if (ItfTypeCan == updTypeMap->cmmuItfType)
    {
        CanAuxCmdBuf *auxBuf;
        BoxMsgHead *boxHead;
        BoxUpdDldCmd *boxCmd;
        Box *box;
        u16 mapIdx;

        if (NULL == (auxBuf = allocAuxCanBuf()))
        {
            rspUpCmmn(UpCmdIdUpdDld, upCmd->trayIdx, RspBusy);
            return;
        }

        box = UpUpdCanDevMap(tray, updTypeMap->devType, upCmd->devId, &mapIdx);
        setCaliAuxCanBuf(auxBuf, box, UpCmdIdUpdDld);
        boxHead = (BoxMsgHead *)auxBuf->msgBuf;
        boxHead->msgId = BoxMsgUpdDld;
        boxHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxUpdDldCmd);
        boxCmd = (BoxUpdDldCmd *)boxHead->payload;
        boxCmd->boxDevType = updTypeMap->devType;
        boxCmd->boxDevIdx = mapIdx;
        boxCmd->pldSeq = upCmd->offset / updMgr->pageSize;
        boxCmd->pldSize = upCmd->pldSize;
        mem2Copy((u16 *)boxCmd->payload, (u16 *)&upCmd->payload, upCmd->pldSize&0xfffe);
        if (upCmd->pldSize & 1)
        {
            boxCmd->payload[upCmd->pldSize-1] = upCmd->payload[upCmd->pldSize-1];
        }
        if (upCmd->pldSize < updMgr->pageSize)
        {
            memset(&boxCmd->payload[upCmd->pldSize], 0xff, updMgr->pageSize-upCmd->pldSize);
        }
        boxHead->msgLen += updMgr->pageSize;
        boxAuxTxTry(box->canIdx, auxBuf);
    }
    else  /*todo,增加温度盒等uart设备的维护模式*/
    {
    }

    return;
}

void upMsgUpdUpld(u8 *pld, u16 pldLen)
{
    return;
}

u16eRspCode upMsgUpdCnfmChk(UpUpdCnfmCmd *upCmd, u16 pldLen)
{
    if (upCmd->trayIdx>=gDevMgr->trayAmt || upCmd->devType>=UpUpdDevCri
        || upCmd->fileType>=UpdateFileCri)
    {
        return RspParam;
    }
    return RspOk;
}

/*含临时应答*/
void rspUpUpdCnfm(u16eRspCode rspCode, u16 version)
{
    UpUpdCnfmAck *upAck;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpUpdCnfmAck *)(buf + sizeof(UpMsgHead));
    upAck->rspCode = rspCode;
    upAck->updateVer = version;
    sendUpMsg(buf, sizeof(UpUpdCnfmAck), UpCmdIdUpdCnfm);
    return;
}

void upMsgUpdCnfm(u8 *pld, u16 pldLen)
{
    UpUpdCnfmCmd *upCmd;
    UpdTypeMap *updTypeMap;
    Tray *tray;
    u16eRspCode rspCode;

    upCmd = (UpUpdCnfmCmd *)pld;
    if (RspOk != (rspCode=upMsgUpdCnfmChk(upCmd, pldLen)))
    {
        rspUpUpdCnfm(rspCode, 0);
        return;
    }

    if (UpUpdDevMedium == upCmd->devType)
    {
        rspUpUpdCnfm(rspCode, gMediumSoftVer);
        return;
    }

    tray = &gDevMgr->tray[upCmd->trayIdx];
    updTypeMap = &tray->updTypeMap[upCmd->devType];
    if (ItfTypeCan == updTypeMap->cmmuItfType)
    {
        CanAuxCmdBuf *auxBuf;
        BoxMsgHead *boxHead;
        BoxUpdCnfmCmd *boxCmd;
        Box *box;
        u16 mapIdx;

        if (NULL == (auxBuf = allocAuxCanBuf()))
        {
            rspUpUpdCnfm(RspNoRes, 0);
            return;
        }

        box = UpUpdCanDevMap(tray, updTypeMap->devType, upCmd->devId, &mapIdx);
        setCaliAuxCanBuf(auxBuf, box, UpCmdIdUpdCnfm);
        boxHead = (BoxMsgHead *)auxBuf->msgBuf;
        boxHead->msgId = BoxMsgUpdCnfm;
        boxHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxUpdCnfmCmd);
        boxCmd = (BoxUpdCnfmCmd *)boxHead->payload;
        boxCmd->boxDevType = updTypeMap->devType;
        boxCmd->boxDevIdx = mapIdx;
        boxCmd->fileType = upCmd->fileType;
        boxAuxTxTry(box->canIdx, auxBuf);
    }
    else
    {
    }

    return;
}

void upMsgDispUpdate(u8eUpMsgIdUpdate msgId, u8 *pld, u16 pldLen)
{
    /*todo, 入口把控*/
    if (msgId < UpMsgIdUpdCri)
    {
        gUpItfCb->upMsgProcUpdate[msgId](pld, pldLen);
    }

    return;
}

void _____begin_fixture_precision_msg_____(){}

u16eRspCode upFixtNtfyChk(UpFixtNtfyCmd *fixtCmd, u16 pldLen)
{
    if (pldLen!=sizeof(UpFixtNtfyCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->fixtType>=UpFixtTypeCri || fixtCmd->actAddr>=FixtAddrCri)
    {
        return RspParam;
    }

    return RspOk;
}

/*工装使能,建立映射*/
void upMsgFixtNtfy(u8 *pld, u16 pldLen)
{
    UpFixtNtfyCmd *fixtCmd;
    UartBlockCmdBuf *blockBuf;
    Tray *tray;
    UartMgr *uartMgr;
    u16eRspCode rspCode;
    u8eUartDevType uartDevType;

    fixtCmd = (UpFixtNtfyCmd *)pld;
    if (RspOk != (rspCode=upFixtNtfyChk(fixtCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdFixtNtfy, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!fixtCmd->enable)
    {
        /*离开工装维护,不需要通知级联,直接回复上位机*/
        trayMntnEnd(tray, BoxModeMntnFixt);
        rspUpCmmn(UpCmdIdFixtNtfy, fixtCmd->trayIdx, RspOk);
        return;
    }

    if (NULL == (blockBuf = allocUartBlockBuf()))
    {
        rspUpCmmn(UpCmdIdFixtNtfy, fixtCmd->trayIdx, RspBusy);
        return;
    }

    uartMgr = gUartMgr;
    uartDevType = uartMgr->upFixt2UartType[fixtCmd->fixtType];
    tray->fixtUartCmmuAddr = uartMgr->uartAddrBase[uartDevType] + fixtCmd->actAddr;
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtNtfy,
                    tray->fixtUartCmmuAddr, uartDevType, UartMsgIdCmmn(UartMsgConn));
    ((UartMsgHead *)blockBuf->msgBuf)->pldLen = sizeof(UartCmmnCmd);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    trayMntnEnter(tray, BoxModeMntnFixt);
    return;
}

u16eRspCode upFixtPrecChnlSwChk(UpFixtPrecSwCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtPrecSwCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->caliType>=CaliTypeCri || fixtCmd->chnSwType>=FixtChnSwCri)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtPrecChnlSw(u8 *pld, u16 pldLen)
{
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    UartFixtPrecSwCmd *uartCmd;
    UpFixtPrecSwCmd *fixtCmd;
    Tray *tray;
    u16eRspCode rspCode;
    u8eUartDevType uartDevType;

    fixtCmd = (UpFixtPrecSwCmd *)pld;
    if (RspOk != (rspCode=upFixtPrecChnlSwChk(fixtCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdFixtPrecChnlSw, fixtCmd->trayIdx, rspCode);
        return;
    }

    if (NULL == (blockBuf = allocUartBlockBuf()))
    {
        rspUpCmmn(UpCmdIdFixtPrecChnlSw, fixtCmd->trayIdx, RspBusy);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    uartDevType = (tray->fixtUartCmmuAddr-UartAddrBaseFixtTmpr >> FixtAddrWidth) + UartFixtTmpr;
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtPrecChnlSw,
                    tray->fixtUartCmmuAddr, uartDevType, UartMsgIdSpec(UartMsgFixtPrecChnSw));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtPrecSwCmd);
    uartCmd = (UartFixtPrecSwCmd *)uartHead->payload;
    uartCmd->caliType = fixtCmd->caliType;
    uartCmd->chnSwType = fixtCmd->chnSwType;
    uartCmd->firstChn = fixtCmd->firstChn;
    uartCmd->lastChn = fixtCmd->lastChn;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtPrecSmplChk(UpFixtPrecSmplCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtPrecSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->smplType>=FixtPrecSmplCri)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }
        
    return RspOk;
}

void upMsgFixtPrecSmpl(u8 *pld, u16 pldLen)
{
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    UartFixtPrecSmplCmd *uartCmd;
    UpFixtPrecSmplCmd *fixtCmd;
    Tray *tray;
    u16eRspCode rspCode;
    u8eUartDevType uartDevType;

    fixtCmd = (UpFixtPrecSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtPrecSmplChk(fixtCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdFixtPrecSmpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    if (NULL == (blockBuf = allocUartBlockBuf()))
    {
        rspUpCmmn(UpCmdIdFixtPrecSmpl, fixtCmd->trayIdx, RspBusy);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    uartDevType = (tray->fixtUartCmmuAddr-UartAddrBaseFixtTmpr >> FixtAddrWidth) + UartFixtTmpr;
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtPrecSmpl,
                    tray->fixtUartCmmuAddr, uartDevType, UartMsgIdSpec(UartMsgFixtPrecSmpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtPrecSmplCmd);
    uartCmd = (UartFixtPrecSmplCmd *)uartHead->payload;
    uartCmd->smplType = fixtCmd->smplType;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtPrecOutChk(UpFixtPrecOutCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtPrecOutCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtPrecOut(u8 *pld, u16 pldLen)
{
    UartBlockCmdBuf *blockBuf;
    UartFixtPrecOutCmd *uartCmd;
    UartMsgHead *uartHead;
    UpFixtPrecOutCmd *fixtCmd;
    Tray *tray;
    u16eRspCode rspCode;
    u8eUartDevType uartDevType;

    fixtCmd = (UpFixtPrecOutCmd *)pld;
    if (RspOk != (rspCode=upFixtPrecOutChk(fixtCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdFixtPrecOut, fixtCmd->trayIdx, rspCode);
        return;
    }

    if (NULL == (blockBuf = allocUartBlockBuf()))
    {
        rspUpCmmn(UpCmdIdFixtPrecOut, fixtCmd->trayIdx, RspBusy);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    uartDevType = (tray->fixtUartCmmuAddr-UartAddrBaseFixtTmpr >> FixtAddrWidth) + UartFixtTmpr;
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtPrecOut,
                    tray->fixtUartCmmuAddr, uartDevType, UartMsgIdSpec(UartMsgFixtPrecOut));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtPrecOutCmd);
    uartCmd = (UartFixtPrecOutCmd *)uartHead->payload;
    uartCmd->voltage = fixtCmd->voltage;
    uartCmd->current = fixtCmd->current;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtPrecDisp(u8 msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdFixtPrecCri)
    {
        gUpItfCb->upMsgFixtPrecDisp[msgId](pld, pldLen);
    }

    return;
}

void _____begin_fixture_temperature_____(){}

u16eRspCode upFixtTmprHeatChk(UpFixtTmprHeatCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtTmprHeatCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtTmprHeat(u8 *pld, u16 pldLen)
{
    UpFixtTmprHeatCmd *fixtCmd;
    UartBlockCmdBuf *blockBuf;
    UartFixtTmprHeatCmd *uartCmd;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtTmprHeatCmd *)pld;
    if (RspOk != (rspCode=upFixtTmprHeatChk(fixtCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdFixtTmprHeat, fixtCmd->trayIdx, rspCode);
        return;
    }

    if (NULL == (blockBuf = allocUartBlockBuf()))
    {
        rspUpCmmn(UpCmdIdFixtTmprHeat, fixtCmd->trayIdx, RspBusy);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtTmprHeat,
                    tray->fixtUartCmmuAddr, UartFixtTmpr, UartMsgIdSpec(UartMsgFixtTmprHeat));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtTmprHeatCmd);
    uartCmd = (UartFixtTmprHeatCmd *)uartHead->payload;
    uartCmd->isSingleChn = fixtCmd->isSingleChn;
    uartCmd->heatStop = fixtCmd->heatStop;
    uartCmd->tmprVal = fixtCmd->tmprVal;
    uartCmd->chnIdx = fixtCmd->chnIdx;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtTmprSmplChk(UpFixtTmprSmplCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtTmprSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->chnAmt > MaxFixtTmprSmplAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }
        
    return RspOk;
}

void upMsgFixtTmprSmpl(u8 *pld, u16 pldLen)
{
    UpFixtTmprSmplCmd *fixtCmd;
    UartFixtTmprSmplCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtTmprSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtTmprSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtTmprSmpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtTmprSmpl,
                    tray->fixtUartCmmuAddr, UartFixtTmpr, UartMsgIdSpec(UartMsgFixtTmprSmpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtTmprSmplCmd);
    uartCmd = (UartFixtTmprSmplCmd *)uartHead->payload;
    uartCmd->chnAmt = fixtCmd->chnAmt;
    uartCmd->firstChnIdx = fixtCmd->firstChnIdx;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtTmprDisp(u8 msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdFixtTmprCri)
    {
        gUpItfCb->upMsgFixtTmprDisp[msgId](pld, pldLen);
    }

    return;
}

void _____begin_fixture_neg_pressure_____(){}

u16eRspCode upFixtCleanActChk(UpFixtCleanActCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtCleanActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->actType >= FixtCleanActCri)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }
        
    return RspOk;
}

void upMsgFixtCleanAct(u8 *pld, u16 pldLen)
{
    UpFixtCleanActCmd *fixtCmd;
    UartFixtCleanActCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtCleanActCmd *)pld;
    if (RspOk != (rspCode=upFixtCleanActChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtCleanAct, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtCleanAct,
                    tray->fixtUartCmmuAddr, UartFixtClean, UartMsgIdSpec(UartMsgFixtCleanAct));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtCleanActCmd);
    uartCmd = (UartFixtCleanActCmd *)uartHead->payload;
    uartCmd->actType = fixtCmd->actType;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtCleanDisp(u8 msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdFixtCleanCri)
    {
        gUpItfCb->upMsgFixtCleanDisp[msgId](pld, pldLen);
    }

    return;
}

u16eRspCode upFixtGasSmplChk(UpFixtGasSmplCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtGasSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtGasSmpl(u8 *pld, u16 pldLen)
{
    UpFixtGasSmplCmd *fixtCmd;
    UartFixtGasSmplCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtGasSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtGasSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtGasSmpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtGasSmpl,
                    tray->fixtUartCmmuAddr, UartFixtGas, UartMsgIdSpec(UartMsgFixtGasSmpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtGasSmplCmd);
    uartCmd = (UartFixtGasSmplCmd *)uartHead->payload;
    uartCmd->chnAmt = fixtCmd->chnAmt;
    uartCmd->firstChnIdx = fixtCmd->chnIdx;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtGasDisp(u8 msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdFixtGasCri)
    {
        gUpItfCb->upMsgFixtGasDisp[msgId](pld, pldLen);
    }

    return;
}

u16eRspCode upFixtFlowSmplChk(UpFixtFlowSmplCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtFlowSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtFlowSmpl(u8 *pld, u16 pldLen)
{
    UpFixtFlowSmplCmd *fixtCmd;
    UartFixtFlowSmplCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtFlowSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtFlowSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtFlowSmpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtFlowSmpl,
                    tray->fixtUartCmmuAddr, UartFixtFlow, UartMsgIdSpec(UartMsgFixtFlowSmpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtFlowSmplCmd);
    uartCmd = (UartFixtFlowSmplCmd *)uartHead->payload;
    uartCmd->chnAmt = fixtCmd->chnAmt;
    uartCmd->firstChnIdx = fixtCmd->chnIdx;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtFlowDisp(u8 msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdFixtFlowCri)
    {
        gUpItfCb->upMsgFixtFlowDisp[msgId](pld, pldLen);
    }

    return;
}

void _____begin_fixture_suction_____(){}

u16eRspCode upFixtSuctInSmplChk(UpFixtSuckInSmplCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtSuckInSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtSuctInSmpl(u8 *pld, u16 pldLen)
{
    UpFixtSuckInSmplCmd *fixtCmd;
    UartFixtSuckInSmplCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtSuckInSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtSuctInSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtSuctInSmpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtSuctInSmpl,
                    tray->fixtUartCmmuAddr, UartFixtSuctIn, UartMsgIdSpec(UartMsgFixtSuckInSmpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtSuckInSmplCmd);
    uartCmd = (UartFixtSuckInSmplCmd *)uartHead->payload;
    uartCmd->delayTime = fixtCmd->delayTime;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtSuctIn2Smpl(u8 *pld, u16 pldLen)
{
    UpFixtSuckInSmplCmd *fixtCmd;
    UartFixtSuckInSmplCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtSuckInSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtSuctInSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtSuctIn2Smpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtSuctIn2Smpl, tray->fixtUartCmmuAddr, 
                    UartFixtSuctIn2, UartMsgIdSpec(UartMsgFixtSuckIn2Smpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtSuckInSmplCmd);
    uartCmd = (UartFixtSuckInSmplCmd *)uartHead->payload;
    uartCmd->delayTime = fixtCmd->delayTime;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtSuctIn2ActChk(UpFixtSuckIn2ActCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtSuckIn2ActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtSuctIn2Act(u8 *pld, u16 pldLen)
{
    UpFixtSuckIn2ActCmd *fixtCmd;
    UartFixtSuckIn2ActCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtSuckIn2ActCmd *)pld;
    if (RspOk != (rspCode=upFixtSuctIn2ActChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtSuctIn2Act, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtSuctIn2Act,
                    tray->fixtUartCmmuAddr, UartFixtSuctIn2, UartMsgIdSpec(UartMsgFixtSuckIn2Act));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtSuckIn2ActCmd);
    uartCmd = (UartFixtSuckIn2ActCmd *)uartHead->payload;
    uartCmd->topUpAct = fixtCmd->topUpAct;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtSuctInDisp(u8 msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdFixtSuctInCri)
    {
        gUpItfCb->upMsgFixtSuctInDisp[msgId](pld, pldLen);
    }

    return;
}

u16eRspCode upFixtSuctOutActChk(UpFixtSuckOutActCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtSuckOutActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->action > FixtSuckOutStaHold)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }
        
    return RspOk;
}

void upMsgFixtSuctOutAct(u8 *pld, u16 pldLen)
{
    UpFixtSuckOutActCmd *fixtCmd;
    UartFixtSuckOutActCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtSuckOutActCmd *)pld;
    if (RspOk != (rspCode=upFixtSuctOutActChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtSuctOutAct, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtSuctOutAct,
                    tray->fixtUartCmmuAddr, UartFixtSuctOut, UartMsgIdSpec(UartMsgFixtSuckOutAct));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtSuckOutActCmd);
    uartCmd = (UartFixtSuckOutActCmd *)uartHead->payload;
    uartCmd->action = fixtCmd->action;
    uartCmd->delayTime = fixtCmd->delayTime;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtSuctOutSmplChk(UpFixtSuckOutSmplCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtSuckOutSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtSuctOutSmpl(u8 *pld, u16 pldLen)
{
    UpFixtSuckOutSmplCmd *fixtCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtSuckOutSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtSuctOutSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtSuctOutSmpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtSuctOutSmpl,
                tray->fixtUartCmmuAddr, UartFixtSuctOut, UartMsgIdSpec(UartMsgFixtSuckOutSmpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtSuckOutSmplCmd);
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtSuctOut2Smpl(u8 *pld, u16 pldLen)
{
    UpFixtSuckOutSmplCmd *fixtCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtSuckOutSmplCmd *)pld;
    if (RspOk != (rspCode = upFixtSuctOutSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtSuctOut2Smpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtSuctOut2Smpl,
                tray->fixtUartCmmuAddr, UartFixtSuctOut2, UartMsgIdSpec(UartMsgFixtSuckOut2Smpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtSuckOutSmplCmd);
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtSuctOut2ActChk(UpFixtSuckOut2ActCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtSuckOut2ActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->action > FixtSuckOutStaHold)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }
        
    return RspOk;
}

void upMsgFixtSuctOut2Act(u8 *pld, u16 pldLen)
{
    UpFixtSuckOut2ActCmd *fixtCmd;
    UartFixtSuckOut2ActCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtSuckOut2ActCmd *)pld;
    if (RspOk != (rspCode=upFixtSuctOut2ActChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtSuctOut2Act, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtSuctOut2Act,
                    tray->fixtUartCmmuAddr, UartFixtSuctOut2, UartMsgIdSpec(UartMsgFixtSuckOut2Act));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtSuckOut2ActCmd);
    uartCmd = (UartFixtSuckOut2ActCmd *)uartHead->payload;
    uartCmd->action = fixtCmd->action;
    uartCmd->delayTime = fixtCmd->delayTime;
    uartCmd->topUpAct = fixtCmd->topUpAct;
    uartCmd->rsvd = 0;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtSuctOutDisp(u8 msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdFixtSuctOutCri)
    {
        gUpItfCb->upMsgFixtSuctOutDisp[msgId](pld, pldLen);
    }

    return;
}

void _____begin_fixture_location_____(){}

u16eRspCode upFixtLocatSmplChk(UpFixtLocatSmplCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtLocatSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgFixtLocatSmpl(u8 *pld, u16 pldLen)
{
    UpFixtLocatSmplCmd *fixtCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtLocatSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtLocatSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtLocatSmpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtLocatSmpl,
                    tray->fixtUartCmmuAddr, UartFixtLocat, UartMsgIdSpec(UartMsgFixtLocatSmpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtLocatSmplCmd);
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtLocatActChk(UpFixtLocatActCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtLocatActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->anodeTurn > FixtLocatTurnCri-1 || fixtCmd->npTurn > FixtLocatTurnCri-1
        || fixtCmd->cathodeTurn > FixtLocatTurnCri-1)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }
        
    return RspOk;
}

void upMsgFixtLocatAct(u8 *pld, u16 pldLen)
{
    UpFixtLocatActCmd *fixtCmd;
    UartFixtLocatActCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtLocatActCmd *)pld;
    if (RspOk != (rspCode=upFixtLocatActChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtLocatAct, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtLocatAct,
                    tray->fixtUartCmmuAddr, UartFixtLocat, UartMsgIdSpec(UartMsgFixtLocatAct));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtLocatActCmd);
    uartCmd = (UartFixtLocatActCmd *)uartHead->payload;
    uartCmd->cellType = fixtCmd->cellType;
    uartCmd->anodeTurn = fixtCmd->anodeTurn;
    uartCmd->npTurn = fixtCmd->npTurn;
    uartCmd->cathodeTurn = fixtCmd->cathodeTurn;
    uartCmd->delayTime = fixtCmd->delayTime;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtLocat2Smpl(u8 *pld, u16 pldLen)
{
    UpFixtLocatSmplCmd *fixtCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtLocatSmplCmd *)pld;
    if (RspOk != (rspCode=upFixtLocatSmplChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtLocat2Smpl, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtLocat2Smpl,
                    tray->fixtUartCmmuAddr, UartFixtLocat2, UartMsgIdSpec(UartMsgFixtLocat2Smpl));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtLocatSmplCmd);
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtLocat2ActChk(UpFixtLocat2ActCmd *fixtCmd, u16 pldLen)
{
    Tray *tray;

    if (pldLen!=sizeof(UpFixtLocat2ActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->anodeTurn > FixtLocatTurnCri-1 || fixtCmd->npTurn > FixtLocatTurnCri-1
        || fixtCmd->cathodeTurn > FixtLocatTurnCri-1)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnFixt))
    {
        return RspWoMntn;
    }
        
    return RspOk;
}

void upMsgFixtLocat2Act(u8 *pld, u16 pldLen)
{
    UpFixtLocat2ActCmd *fixtCmd;
    UartFixtLocat2ActCmd *uartCmd;
    UartBlockCmdBuf *blockBuf;
    UartMsgHead *uartHead;
    Tray *tray;
    u16eRspCode rspCode;

    fixtCmd = (UpFixtLocat2ActCmd *)pld;
    if (RspOk != (rspCode=upFixtLocat2ActChk(fixtCmd, pldLen))
        || RspOk != (rspCode = NULL==(blockBuf=allocUartBlockBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdFixtLocat2Act, fixtCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtLocat2Act,
                    tray->fixtUartCmmuAddr, UartFixtLocat2, UartMsgIdSpec(UartMsgFixtLocat2Act));
    uartHead = (UartMsgHead *)blockBuf->msgBuf;
    uartHead->pldLen = sizeof(UartFixtLocat2ActCmd);
    uartCmd = (UartFixtLocat2ActCmd *)uartHead->payload;
    uartCmd->cellType = fixtCmd->cellType;
    uartCmd->anodeTurn = fixtCmd->anodeTurn;
    uartCmd->npTurn = fixtCmd->npTurn;
    uartCmd->cathodeTurn = fixtCmd->cathodeTurn;
    uartCmd->topUpAct = fixtCmd->topUpAct;
    uartCmd->delayTime = fixtCmd->delayTime;
    trayMntnKeep(tray);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

void upMsgFixtLocatDisp(u8 msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdFixtLocatCri)
    {
        gUpItfCb->upMsgFixtLocatDisp[msgId](pld, pldLen);
    }

    return;
}

void upMsgDispFixt(u8 typeId, u8 *pld, u16 pldLen)
{
    u8 type;

    /*todo, 入口把控*/
    type = typeId >> UpMsgFixtTypeOffset;
    if (type < FixtProtoCri)
    {
        gUpItfCb->upMsgFixtDisp[type](typeId&UpMsgIdFixtMask, pld, pldLen);
    }

    return;
}
void _____begin_of_calibration__msg_____(){}

u16eRspCode upCaliNtfyChk(UpCaliNtfyCmd *caliCmd, u16 pldLen)
{
    Box *box;

    if (pldLen!=sizeof(UpCaliNtfyCmd) || caliCmd->trayIdx>=gDevMgr->trayAmt
        || caliCmd->boxIdx>=gDevMgr->tray[caliCmd->trayIdx].boxAmt)
    {
        return RspParam;
    }

    box = &gDevMgr->tray[caliCmd->trayIdx].box[caliCmd->boxIdx];
    if (!box->online)
    {
        return RspDiscBox;
    }

    if (!trayBoxMntnAble(caliCmd->trayIdx))
    {
        return RspChnBeRun;
    }
    return RspOk;
}

/*进入或者离开修调*/
void rspUpCaliNtfyAck(u8 trayIdx, u8 boxIdx, u16eRspCode rspCode)
{
    UpCaliNtfyAck *upAck;
    u8 *buf;

    buf = sendUpBuf;
    upAck = (UpCaliNtfyAck *)(buf + sizeof(UpMsgHead));
    upAck->trayIdx = trayIdx;
    upAck->rspCode = rspCode;
    upAck->boxIdx = boxIdx;

    sendUpMsg(buf, sizeof(UpCaliNtfyAck), UpCmdIdCaliNtfy);
    return;
}

void upMsgCaliNtfy(u8 *pld, u16 pldLen)
{
    UpCaliNtfyCmd *upCmd;
    CanAuxCmdBuf *auxBuf;
    BoxMsgHead *boxHead;
    BoxCaliNtfyCmd *boxCmd;
    Tray *tray;
    Box *box;
    u16eRspCode rspCode;

    upCmd = (UpCaliNtfyCmd *)pld;
    if (RspOk != (rspCode = upCaliNtfyChk(upCmd, pldLen))
        || RspOk != (rspCode = NULL==(auxBuf=allocAuxCanBuf()) ? RspBusy : RspOk))
    {
        rspUpCaliNtfyAck(upCmd->trayIdx, upCmd->boxIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[upCmd->trayIdx];
    box = &tray->box[upCmd->boxIdx];
    setCaliAuxCanBuf(auxBuf, box, UpCmdIdCaliNtfy);
    boxHead = (BoxMsgHead *)auxBuf->msgBuf;
    boxHead->msgId = BoxMsgCaliNtfy;
    boxHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliNtfyCmd);
    boxCmd = (BoxCaliNtfyCmd *)boxHead->payload;
    boxCmd->caliEnable = upCmd->caliEnable;
    boxAuxTxTry(box->canIdx, auxBuf);
    if (upCmd->caliEnable)
    {
        box->boxWorkMode = BoxModeMntnCali;
        trayMntnEnter(tray, BoxModeMntnCali);
    }
    else
    {
        trayMntnKeep(tray);
    }
    return;
}

/*todo,区分启动和停止,再检查详细些*/
u16eRspCode upCaliStartChk(UpCaliStartCmd *caliCmd, u16 pldLen)
{
    Tray *tray;

    if (caliCmd->trayIdx >= gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[caliCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnCali))
    {
        return RspWoMntn;
    }

    return RspOk;
}

/*中下协议只针对2对齐,赋值4字节整数时小心点儿*/
void upMsgCaliStart(u8 *pld, u16 pldLen)
{
    UpCaliStartCmd *upCmd;
    CanAuxCmdBuf *auxBuf;
    BoxMsgHead *boxMsgHead;
    BoxCaliStartCmd *boxCmd;
    Tray *tray;
    Channel *chn;
    u16eRspCode rspCode;
    u16 genIdxInChn;

    upCmd = (UpCaliStartCmd *)pld;
    if (RspOk != (rspCode = upCaliStartChk(upCmd, pldLen))
        || RspOk != (rspCode = NULL==(auxBuf=allocAuxCanBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdCaliStart, upCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[upCmd->trayIdx];
    chn = upChnMap(tray, upCmd->chnIdx, &genIdxInChn);
    setCaliAuxCanBuf(auxBuf, chn->box, UpCmdIdCaliStart);
    boxMsgHead = (BoxMsgHead *)auxBuf->msgBuf;
    if (upCmd->isStart)
    {
        boxMsgHead->msgId = BoxMsgCaliStart;
        boxCmd = (BoxCaliStartCmd *)boxMsgHead->payload;
        boxCmd->moduIdx = chn->box->chnModuAmt*chn->lowChnIdxInBox + upCmd->moduIdx;
        boxCmd->chnIdx = 0==genIdxInChn ? 0 : chn->cell[genIdxInChn-1].lowCellIdxInChn+1;
        boxCmd->caliMode = upCmd->caliMode;
        boxCmd->caliType = upCmd->caliType;
        mem2Copy(boxCmd->caliKbPoint, (u16 *)&upCmd->caliKbPoint, sizeof(u32));
        boxMsgHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliStartCmd) + sizeof(u32);
    }
    else
    {
        boxMsgHead->msgId = BoxMsgCaliStop;
        boxMsgHead->msgLen = sizeof(BoxMsgHead);
    }

    trayMntnKeep(tray);
    boxAuxTxTry(chn->box->canIdx, auxBuf);
    return;
}

/*todo,区分启动和停止,再检查详细些*/
u16eRspCode upCaliSmplChk(UpCaliSmplCmd *caliCmd, u16 pldLen)
{
    Tray *tray;

    if (caliCmd->trayIdx >= gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[caliCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnCali))
    {
        return RspWoMntn;
    }

    return RspOk;
}

void upMsgCaliSmpl(u8 *pld, u16 pldLen)
{
    UpCaliSmplCmd *upCmd;
    CanAuxCmdBuf *auxBuf;
    BoxMsgHead *boxMsgHead;
    BoxCaliSmplCmd *boxCmd;
    Tray *tray;
    Channel *chn;
    u16eRspCode rspCode;
    u16 genIdxInChn;  /*通道内子主统排索引*/

    upCmd = (UpCaliSmplCmd *)pld;
    if (RspOk != (rspCode = upCaliSmplChk(upCmd, pldLen))
        || RspOk != (rspCode = NULL==(auxBuf=allocAuxCanBuf()) ? RspBusy : RspOk))
    {
        UpCaliSmplAck *upAck;
        u8 *buf;

        buf = sendUpBuf;
        upAck = (UpCaliSmplAck *)(buf + sizeof(UpMsgHead));
        upAck->trayIdx = upCmd->trayIdx;
        upAck->smplAmt = 0;
        upAck->rspCode = rspCode;
        upAck->moduIdx = upCmd->moduIdx;
        upAck->chnIdx = upCmd->chnIdx;
        upAck->smplOnlyMainChn = upCmd->smplOnlyMainChn;
        sendUpMsg(buf, sizeof(UpCaliSmplAck), UpCmdIdCaliSmpl);
        return;
    }

    tray = &gDevMgr->tray[upCmd->trayIdx];
    chn = upChnMap(tray, upCmd->chnIdx, &genIdxInChn);
    setCaliAuxCanBuf(auxBuf, chn->box, UpCmdIdCaliSmpl);
    boxMsgHead = (BoxMsgHead *)auxBuf->msgBuf;
    boxMsgHead->msgId = BoxMsgCaliSmpl;
    boxMsgHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliSmplCmd);
    boxCmd = (BoxCaliSmplCmd *)boxMsgHead->payload;
    boxCmd->moduIdx = chn->box->chnModuAmt*chn->lowChnIdxInBox + upCmd->moduIdx;
    boxCmd->chnIdx = 0==genIdxInChn ? 0 : chn->cell[genIdxInChn-1].lowCellIdxInChn+1;
    boxCmd->smplAllSeries = 0==genIdxInChn && !upCmd->smplOnlyMainChn ? True : False;
    trayMntnKeep(tray);
    boxAuxTxTry(chn->box->canIdx, auxBuf);
    return;
}

/*todo,再检查详细些*/
u16eRspCode upCaliKbChk(UpCaliKbCmd *caliCmd, u16 pldLen)
{
    Tray *tray;

    if (caliCmd->trayIdx >= gDevMgr->trayAmt)
    {
        return RspParam;
    }

    tray = &gDevMgr->tray[caliCmd->trayIdx];
    if (!(tray->trayWorkMode & BoxModeMntnCali))
    {
        return RspWoMntn;
    }

    return RspOk;
}

/*中下协议只针对2对齐,赋值4字节整数时小心点儿*/
void upMsgCaliKb(u8 *pld, u16 pldLen)
{
    UpCaliKbCmd *upCmd;
    CanAuxCmdBuf *auxBuf;
    BoxMsgHead *boxMsgHead;
    BoxCaliKbCmd *boxCmd;
    Tray *tray;
    Channel *chn;
    u16eRspCode rspCode;
    u16 genIdxInChn;

    upCmd = (UpCaliKbCmd *)pld;
    if (RspOk != (rspCode = upCaliKbChk(upCmd, pldLen))
        || RspOk != (rspCode = NULL==(auxBuf=allocAuxCanBuf()) ? RspBusy : RspOk))
    {
        rspUpCmmn(UpCmdIdCaliKb, upCmd->trayIdx, rspCode);
        return;
    }

    tray = &gDevMgr->tray[upCmd->trayIdx];
    chn = upChnMap(tray, upCmd->chnIdx, &genIdxInChn);
    setCaliAuxCanBuf(auxBuf, chn->box, UpCmdIdCaliKb);
    boxMsgHead = (BoxMsgHead *)auxBuf->msgBuf;
    boxMsgHead->msgId = BoxMsgCaliKb;
    boxMsgHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliKbCmd);
    boxCmd = (BoxCaliKbCmd *)boxMsgHead->payload;
    boxCmd->moduIdx = chn->box->chnModuAmt*chn->lowChnIdxInBox + upCmd->moduIdx;
    boxCmd->chnIdx = 0==genIdxInChn ? 0 : chn->cell[genIdxInChn-1].lowCellIdxInChn+1;
    boxCmd->caliType = upCmd->caliType;
    boxCmd->volType = upCmd->volType;
    boxCmd->caliKbAmt = upCmd->caliKbAmt;
    mem2Copy(boxCmd->caliKb, (u16 *)upCmd->caliKb, upCmd->caliKbAmt*sizeof(CaliKb));
    boxMsgHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliKbCmd) + upCmd->caliKbAmt*sizeof(CaliKb);
    trayMntnKeep(tray);
    boxAuxTxTry(chn->box->canIdx, auxBuf);
    return;
}

void upMsgDispCali(u8eUpMsgIdCali msgId, u8 *pld, u16 pldLen)
{
    /*todo, 入口把控, 模式切换*/
    if (msgId < UpMsgIdCaliCri)
    {
        gUpItfCb->upMsgProcCali[msgId](pld, pldLen);
    }

    return;
}

void _____begin_of_needlebed__msg_____(){}

u16eRspCode upNdbdActChk(UpNdbdActOprCmd *upCmd, u16 pldLen)
{
    if (upCmd->trayIdx>=gDevMgr->trayAmt || upCmd->oprType>=NdbdActCri)
    {
        return RspParam;
    }

    if (NdbdActTouch==upCmd->oprType && 1==upCmd->oprPld[0]) /*针床脱开*/
    {
        Tray *tray;
        Channel *chn;
        Channel *chnCri;
        
        tray = &gDevMgr->tray[upCmd->trayIdx];
        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            if (chn->chnStateMed >= ChnStaNpWait)
            {
                return RspChnBeRun;
            }
        }
    }
    return RspOk;
}

void upMsgNdbdAct(u8 *pld, u16 pldLen)
{
    UpNdbdActOprCmd *upCmd;
    u16eRspCode rspCode;

    upCmd = (UpNdbdActOprCmd *)pld;
    if (RspOk != (rspCode = upNdbdActChk(upCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdNdbdAct, upCmd->trayIdx, rspCode);
        return;
    }

    if (NdbdActTouch == upCmd->oprType) /*0压合1脱开*/
    {
        if (0 == upCmd->oprPld[0])
        {
            trayNdbdCtrl(upCmd->trayIdx, NdbdSetTouch, 1);
        }
        else
        {
            trayNdbdBreak(upCmd->trayIdx);
        }
    }
    else if (NdbdActFireDoor == upCmd->oprType)
    {
        trayNdbdCtrl(upCmd->trayIdx, NdbdSetFireDoor, upCmd->oprPld[0]);
    }

    rspUpCmmn(UpCmdIdNdbdAct, upCmd->trayIdx, RspOk);
    return;
}

u16eRspCode upNdbdNpChk(UpNdbdNpOprCmd *npCmd, u16 pldLen)
{
    if (npCmd->trayIdx>=gDevMgr->trayAmt || npCmd->npDevType>=NpDevTypeCri)
    {
        return RspParam;
    }

    if (NpDevRatio == npCmd->npDevType)
    {
        if (npCmd->npOpr>=NpOprRatioCri)
        {
            return RspParam;
        }
        if (NpOprRatioBrkVacum==npCmd->npOpr || NpOprRatioHold==npCmd->npOpr)
        {
            if (4 != pldLen)
            {
                return RspParam;
            }
        }
        else if (8 != pldLen)
        {
            return RspParam;
        }
    }
    else if (NpDevHighLow == npCmd->npDevType)
    {
        return RspParam;
    }
    else
    {
        return RspParam;
    }

    return RspOk;
}

void upMsgNdbdNp(u8 *pld, u16 pldLen)
{
    UpNdbdNpOprCmd *upCmd;
    u8eNpOprCode npOpr;
    u16eRspCode rspCode;
    s16 npVal;

    upCmd = (UpNdbdNpOprCmd *)pld;
    if (RspOk != (rspCode = upNdbdNpChk(upCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdNdbdNp, upCmd->trayIdx, rspCode);
        return;
    }

    npOpr = upCmd->npOpr;
    if (NpOprRatioMkVacum == npOpr)  /*抽真空*/
    {
        npVal = *(s16 *)upCmd->oprPld;
    }
    else if (NpOprRatioSwRatio == npOpr) /*独立比例阀*/
    {
        npVal = *(s16 *)&upCmd->oprPld[2];
    }
    trayNpRatioSet(upCmd->trayIdx, npOpr, upCmd->oprPld[0], npVal);
    trayNpReset(upCmd->trayIdx);
    rspUpCmmn(UpCmdIdNdbdNp, upCmd->trayIdx, RspOk);
    return;
}

u16eRspCode upNdbdFixtPowerChk(UpNdbdFixtPowerCmd *upCmd, u16 pldLen)
{
    if (upCmd->trayIdx>=gDevMgr->trayAmt || pldLen!=sizeof(UpNdbdFixtPowerCmd))
    {
        return RspParam;
    }

    if (upCmd->powerEnable
        && TouchStateWiTouch!=gDevMgr->tray[upCmd->trayIdx].ndbdData.status[NdbdStaTouch])
    {
        return RspTouch;
    }

    return RspOk;
}

void upMsgFixtPower(u8 *pld, u16 pldLen)
{
    UpNdbdFixtPowerCmd *upCmd;
    u16eRspCode rspCode;

    upCmd = (UpNdbdFixtPowerCmd *)pld;
    if (RspOk != (rspCode = upNdbdFixtPowerChk(upCmd, pldLen)))
    {
        rspUpCmmn(UpCmdIdFixtPower, upCmd->trayIdx, rspCode);
        return;
    }

    trayNdbdCtrl(upCmd->trayIdx, NdbdSetFixtPower, upCmd->powerEnable ? 1 : 0);
    rspUpCmmn(UpCmdIdFixtPower, upCmd->trayIdx, RspOk);
    return;
}

void upMsgDispNdbd(u8eUpMsgIdNdbd msgId, u8 *pld, u16 pldLen)
{
    if (msgId < UpMsgIdNdbdCri)
    {
        gUpItfCb->upMsgProcNdbd[msgId](pld, pldLen);
    }

    return;
}

void upMsgDispDbg(u8eDbgMsgId msgId, u8 *pld, u16 pldLen)
{
    if (DbgMsgConn == msgId)
    {
        return;
    }
    else if (DbgMsgDisc == msgId)
    {
        return;
    }
    return;
}


/*
与以太驱动接口，网口有新数据到达，驱动接收后调用此函数
buf:预设的接收缓存，大小为UpMsgBufSize，起始地址按4对齐。
pos:本次数据接收位置。连接建立时初始化为buf，之后动态
size:本次接收数据大小,也即pos之后的数据大小。
return:本函数结束后，接收数据的起始位置。
*/
u8 *upDataRecv(u8 *buf, u8 *pos, u16 size)
{
    UpMsgHead *msgHead;
    UpMsgTail *msgTail;
    u8 *head;
    u16 len;

    for (head=buf,pos+=size; ;head+=UpMsgMandLen+msgHead->pldLen)
    {
        len = pos - head;
        if (len < UpMsgMandLen)
        {
            goto recvContinue;
        }

        msgHead = (UpMsgHead *)head;
        if (UpMsgMagic!=msgHead->magicTag || msgHead->pldLen>UpMsgPldLenMax)
        {
            return buf;  /*丢弃，清空*/
        }

        if (len < UpMsgMandLen+msgHead->pldLen)
        {
            goto recvContinue;
        }

        msgTail = (UpMsgTail *)(head+sizeof(UpMsgHead)+msgHead->pldLen);
        if (msgTail->crcChk == crc16Modbus(head+sizeof(u32), msgHead->pldLen+UpMsgMandCrcLen))
        {
            u8eUpMsgGrp msgGrp;

            /*Show("\r\nrecv upper msg: 0x%04x\r\n", msgHead->msgId);*/
            /*outHex(head, msgHead->pldLen+UpMsgMandLen);*/
            gUpItfCb->rxUpMsgCnt++;
            msgGrp = msgHead->msgId >> UpMsgGrpOffset;
            if (msgGrp < UpMsgGrpCri)
            {
                gUpItfCb->upMsgFlowSeq = msgTail->msgFlowSeq;
                gUpItfCb->upMsgDisp[msgGrp](msgHead->msgId&UpMsgIdMask, head+sizeof(UpMsgHead),
                                            msgHead->pldLen);
            }
        }
        else
        {
            Warn("recv upper msg err crc\r\n");
            return buf;  /*丢弃，清空*/
        }
    }

recvContinue:
    if (0!=len && head!=buf)
    {
        memcpy(buf, head, len);
    }
    return buf+len;
}

void upInitBoot()
{
    UpItfCb *mgr;
    u8 idx;
    u8eStepType stepType;

    mgr = gUpItfCb = sysMemAlloc(sizeof(UpItfCb));
    if (NULL == mgr)
    {
        return;
    }
    sendUpBuf = sysMemAlloc(UpMsgBufSize);
    if (NULL == sendUpBuf)
    {
        return;
    }
    recvUpBuf = sysMemAlloc(UpMsgBufSize);
    if (NULL == recvUpBuf)
    {
        return;
    }

    for (idx=0; idx<UpMsgGrpCri; idx++)
    {
        mgr->upMsgDisp[idx] = (UpMsgDisp )upMsgIgnore;
    }
    mgr->upMsgDisp[UpMsgGrpUpdate] = upMsgDispUpdate;

    /*配置升级类*/
    for (idx=0; idx<UpMsgIdUpdCri; idx++)
    {
        mgr->upMsgProcUpdate[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgProcUpdate[UpMsgIdUpdSetup] = upMsgUpdSetup;
    mgr->upMsgProcUpdate[UpMsgIdUpdDld] = upMsgUpdDld;
    mgr->upMsgProcUpdate[UpMsgIdUpdUpld] = upMsgUpdUpld;
    mgr->upMsgProcUpdate[UpMsgIdUpdCnfm] = upMsgUpdCnfm;

    mgr->updMgr.updWorking = False;
    TimerInit(&mgr->bootTmr);
    timerStart(&mgr->bootTmr, TidBootIdleStay, 3000, WiReset);
}

void upInitApp()
{
    UpItfCb *mgr;
    u8 idx;
    u8eStepType stepType;

    mgr = gUpItfCb = sysMemAlloc(sizeof(UpItfCb));
    if (NULL == mgr)
    {
        return;
    }
    sendUpBuf = sysMemAlloc(UpMsgBufSize);
    if (NULL == sendUpBuf)
    {
        return;
    }
    recvUpBuf = sysMemAlloc(UpMsgBufSize);
    if (NULL == recvUpBuf)
    {
        return;
    }

    mgr->updMgr.updWorking = False;
    mgr->rxUpMsgCnt = 0;
    mgr->upOnline = False;
    mgr->connType = ConnTypeFst;
#ifdef DebugVersion
#else
    upper_datagram_process_port_set(upDataRecv);
#endif

    for (idx=0; idx<UpMsgGrpCri; idx++)
    {
        mgr->upMsgDisp[idx] = (UpMsgDisp)upMsgIgnore;
    }
    mgr->upMsgDisp[UpMsgGrpManu] = upMsgDispManu;
    mgr->upMsgDisp[UpMsgGrpUpdate] = upMsgDispUpdate;
    mgr->upMsgDisp[UpMsgGrpCali] = upMsgDispCali;
    mgr->upMsgDisp[UpMsgGrpNdbd] = upMsgDispNdbd;
    mgr->upMsgDisp[UpMsgGrpFixt] = upMsgDispFixt;
    mgr->upMsgDisp[UpMsgGrpDbg] = upMsgDispDbg;

    /*生产类*/
    for (idx=0; idx<UpMsgIdManuCri; idx++)
    {
        mgr->upMsgProcManu[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgProcManu[UpMsgIdManuConn] = upMsgConnAck;
    mgr->upMsgProcManu[UpMsgIdManuSmplChnl] = upMsgSmplChnl;
    mgr->upMsgProcManu[UpMsgIdManuSmplNdbd] = upMsgSmplNdbd;
    mgr->upMsgProcManu[UpMsgIdManuSmplStack] = upMsgSmplStack;
    mgr->upMsgProcManu[UpMsgIdManuSmplTray] = upMsgSmplTray;
    mgr->upMsgProcManu[UpMsgIdManuFlow] = upMsgFlowStep;
    mgr->upMsgProcManu[UpMsgIdManuProtGen] = upMsgTrayProt;
    mgr->upMsgProcManu[UpMsgIdManuProtStep] = upMsgStepProt;
    mgr->upMsgProcManu[UpMsgIdManuStart] = upMsgFlowStart;
    mgr->upMsgProcManu[UpMsgIdManuPause] = upMsgFlowPause;
    mgr->upMsgProcManu[UpMsgIdManuStop] = upMsgFlowStop;
    mgr->upMsgProcManu[UpMsgIdManuJump] = upMsgFlowJump;
    mgr->upMsgProcManu[UpMsgIdManuCtnu] = upMsgFlowCtnu;
    mgr->upMsgProcManu[UpMsgIdManuCtnuStack] = upMsgFlowStack;
    mgr->upMsgProcManu[UpMsgIdManuWarnDel] = upMsgWarnDel;
    mgr->upMsgProcManu[UpMsgIdManuCellSw] = upMsgSeriesCellSw;
    mgr->upMsgProcManu[UpMsgIdManuRgbCtrl] = upMsgRgbCtrl;
    mgr->upMsgProcManu[UpMsgIdManuBoxChk] = upMsgBoxChk;

    /*配置升级类*/
    for (idx=0; idx<UpMsgIdUpdCri; idx++)
    {
        mgr->upMsgProcUpdate[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgProcUpdate[UpMsgIdUpdCfgRead] = upMsgCfgRead;
    mgr->upMsgProcUpdate[UpMsgIdUpdCfgSet] = upMsgCfgSet;
    mgr->upMsgProcUpdate[UpMsgIdUpdSetup] = upMsgUpdSetup;
    mgr->upMsgProcUpdate[UpMsgIdUpdDld] = upMsgUpdDld;
    mgr->upMsgProcUpdate[UpMsgIdUpdUpld] = upMsgUpdUpld;
    mgr->upMsgProcUpdate[UpMsgIdUpdCnfm] = upMsgUpdCnfm;

    /*电源柜修调类*/
    for (idx=0; idx<UpMsgIdCaliCri; idx++)
    {
        mgr->upMsgProcCali[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgProcCali[UpMsgIdCaliNtfy] = upMsgCaliNtfy;
    mgr->upMsgProcCali[UpMsgIdCaliStart] = upMsgCaliStart;
    mgr->upMsgProcCali[UpMsgIdCaliSmpl] = upMsgCaliSmpl;
    mgr->upMsgProcCali[UpMsgIdCaliKb] = upMsgCaliKb;

    /*针床操作类*/
    for (idx=0; idx<UpMsgIdNdbdCri; idx++)
    {
        mgr->upMsgProcNdbd[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgProcNdbd[UpMsgIdNdbdAct] = upMsgNdbdAct;
    mgr->upMsgProcNdbd[UpMsgIdNdbdNp] = upMsgNdbdNp;
    mgr->upMsgProcNdbd[UpMsgIdFixtPower] = upMsgFixtPower;

    /*工装类*/
    for (idx=0; idx<FixtProtoCri; idx++)
    {
        mgr->upMsgFixtDisp[idx] = (UpMsgDisp)upMsgIgnore;
    }
    mgr->upMsgFixtDisp[FixtProtoPrec] = upMsgFixtPrecDisp;
    mgr->upMsgFixtDisp[FixtProtoTmpr] = upMsgFixtTmprDisp;
    mgr->upMsgFixtDisp[FixtProtoClean] = upMsgFixtCleanDisp;
    mgr->upMsgFixtDisp[FixtProtoGas] = upMsgFixtGasDisp;
    mgr->upMsgFixtDisp[FixtProtoFlow] = upMsgFixtFlowDisp;
    mgr->upMsgFixtDisp[FixtProtoSuctIn] = upMsgFixtSuctInDisp;
    mgr->upMsgFixtDisp[FixtProtoSuctOut] = upMsgFixtSuctOutDisp;
    mgr->upMsgFixtDisp[FixtProtoLocat] = upMsgFixtLocatDisp;

    /*精度线序工装*/
    for (idx=0; idx<UpMsgIdFixtPrecCri; idx++)
    {
        mgr->upMsgFixtPrecDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtPrecDisp[UpMsgIdFixtNtfy] = upMsgFixtNtfy;
    mgr->upMsgFixtPrecDisp[UpMsgIdFixtPrecChnlSw] = upMsgFixtPrecChnlSw;
    mgr->upMsgFixtPrecDisp[UpMsgIdFixtPrecSmpl] = upMsgFixtPrecSmpl;
    mgr->upMsgFixtPrecDisp[UpMsgIdFixtPrecOut] = upMsgFixtPrecOut;

    /*温度线序工装*/
    for (idx=0; idx<UpMsgIdFixtTmprCri; idx++)
    {
        mgr->upMsgFixtTmprDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtTmprDisp[UpMsgIdFixtTmprHeat] = upMsgFixtTmprHeat;
    mgr->upMsgFixtTmprDisp[UpMsgIdFixtTmprSmpl] = upMsgFixtTmprSmpl;

    /*负压杯清洗工装*/
    for (idx=0; idx<UpMsgIdFixtCleanCri; idx++)
    {
        mgr->upMsgFixtCleanDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtCleanDisp[UpMsgIdFixtCleanAct] = upMsgFixtCleanAct;

    /*负压气密性工装*/
    for (idx=0; idx<UpMsgIdFixtGasCri; idx++)
    {
        mgr->upMsgFixtGasDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtGasDisp[UpMsgIdFixtGasSmpl] = upMsgFixtGasSmpl;

    /*负压流量工装*/
    for (idx=0; idx<UpMsgIdFixtFlowCri; idx++)
    {
        mgr->upMsgFixtFlowDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtFlowDisp[UpMsgIdFixtFlowSmpl] = upMsgFixtFlowSmpl;

    /*插拔吸嘴工装*/
    for (idx=0; idx<UpMsgIdFixtSuctInCri; idx++)
    {
        mgr->upMsgFixtSuctInDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtSuctInDisp[UpMsgIdFixtSuctInSmpl] = upMsgFixtSuctInSmpl;
    mgr->upMsgFixtSuctInDisp[UpMsgIdFixtSuctIn2Smpl] = upMsgFixtSuctIn2Smpl;
    mgr->upMsgFixtSuctInDisp[UpMsgIdFixtSuctIn2Act] = upMsgFixtSuctIn2Act;

    /*插拔吸嘴工装*/
    for (idx=0; idx<UpMsgIdFixtSuctOutCri; idx++)
    {
        mgr->upMsgFixtSuctOutDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtSuctOutDisp[UpMsgIdFixtSuctOutSmpl] = upMsgFixtSuctOutSmpl;
    mgr->upMsgFixtSuctOutDisp[UpMsgIdFixtSuctOutAct] = upMsgFixtSuctOutAct;
    mgr->upMsgFixtSuctOutDisp[UpMsgIdFixtSuctOut2Smpl] = upMsgFixtSuctOut2Smpl;
    mgr->upMsgFixtSuctOutDisp[UpMsgIdFixtSuctOut2Act] = upMsgFixtSuctOut2Act;

    /*定位工装*/
    for (idx=0; idx<UpMsgIdFixtLocatCri; idx++)
    {
        mgr->upMsgFixtLocatDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtLocatDisp[UpMsgIdFixtLocatSmpl] = upMsgFixtLocatSmpl;
    mgr->upMsgFixtLocatDisp[UpMsgIdFixtLocatAct] = upMsgFixtLocatAct;
    mgr->upMsgFixtLocatDisp[UpMsgIdFixtLocat2Smpl] = upMsgFixtLocat2Smpl;
    mgr->upMsgFixtLocatDisp[UpMsgIdFixtLocat2Act] = upMsgFixtLocat2Act;

    /*工步内容合法性检查*/
    for (stepType=StepTypeNull; stepType<StepTypeCri; stepType++)
    {
        mgr->stepParamChk[stepType] = upStepChkErr;
    }
    mgr->stepParamChk[StepTypeQuiet] = upStepChkQuiet;
    mgr->stepParamChk[StepTypeCCC] = upStepChkCCC;
    mgr->stepParamChk[StepTypeCCD] = upStepChkCCD;
    mgr->stepParamChk[StepTypeCCCVC] = upStepChkCCCVC;
    mgr->stepParamChk[StepTypeCCCVD] = upStepChkCCCVD;
    mgr->stepParamChk[StepTypeCVC] = upStepChkCVC;
    mgr->stepParamChk[StepTypeCVD] = upStepChkCVD;
    mgr->stepParamChk[StepTypeLoop] = upStepChkLoop;
    mgr->stepParamChk[StepTypeDCIR] = upStepChkDcir;

    TimerInit(&mgr->bootTmr);
    TimerInit(&mgr->keepAliveTmr);
    TimerInit(&mgr->connUpTmr);
    TimerInit(&mgr->upDiscExprTmr);
    timerStart(&mgr->connUpTmr, TidConnUpper, 2000, WiReset);
    return;
}

#endif
