
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "basic.h"
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
#ifdef DebugVersion
#else
#include "upper_ntp.h"
#endif


UpItfCb *gUpItfCb;

/*临时用的全局变量*/
u8 *sendUpBuf;
u8 *recvUpBuf;
s32 gAbsTimeSec;

Ret upStepChkErr(UpFlowCmd *cmd, UpStepInfo *step)
{
    return Nok;
}

Ret upStepChkQuiet(UpFlowCmd *cmd, UpStepInfo *step)
{
    if (0x01==step->paramEnable && step->stepParam[0]>=1000)
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
        && step->stepParam[0]>0 && step->stepParam[0]<tray->boxCfg.maxTrayCur
        && step->stepParam[1] >= 1000)
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
        && step->stepParam[0]>0 && step->stepParam[0]<tray->boxCfg.maxTrayCur
        && step->stepParam[1] >= 1000)
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
        && param[0]>0 && param[0]<tray->boxCfg.maxTrayCur
        && param[1]>0 && param[1]<tray->boxCfg.maxTrayVol
        && param[2] >= 1000)
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
        && param[0]>0 && param[0]<tray->boxCfg.maxTrayCur
        && param[1]>0 && param[1]<tray->boxCfg.maxTrayVol
        && param[2] >= 1000)
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
        && param[1]>0 && param[1]<tray->boxCfg.maxTrayVol
        && (!(step->paramEnable&0x01) || param[0]>0 && param[0]<tray->boxCfg.maxTrayCur)
        && param[2] >= 1000)
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
        && param[1]>0 && param[1]<tray->boxCfg.maxTrayVol
        && (!(step->paramEnable&0x01) || param[0]>0 && param[0]<tray->boxCfg.maxTrayCur)
        && param[2] >= 1000)
    {
        return Ok;
    }

    return Nok;
}

Ret upStepChkLoop(UpFlowCmd *cmd, UpStepInfo *step)
{
    return Ok;
}

Ret upStepChkDcir(UpFlowCmd *cmd, UpStepInfo *step)
{
    return Ok;
}

Ret upStepChk(UpFlowCmd *cmd, UpStepInfo *step)
{
    if (step->stepType>StepTypeNull && step->stepType<StepTypeCri)
    {
        return gUpItfCb->stepParamChk[step->stepType](cmd, step);
    }

    return Nok;
}

u16eRspCode upFlowChk(UpFlowCmd *cmd, u16 pldLen)
{
    UpStepInfo *step;
    DevMgr *dev;
    FlowRecvCtrl *ctrl;
    u16 pldLenHope;
    u16 idx;

    dev = gDevMgr;
    ctrl = recvCtrlGet();
    pldLenHope = sizeof(UpFlowCmd) + Align16(cmd->chnAmt)*2;
    pldLenHope += sizeof(UpStepInfo) * cmd->stepAmt;
    if (pldLenHope!=pldLen || 0==cmd->stepAmt || cmd->stepAmt>MaxStepTrans
        || cmd->stepSeq+cmd->stepAmt>cmd->stepAmtTtl || cmd->trayIdx>=dev->trayAmt
        || cmd->chnAmt > dev->tray[cmd->trayIdx].trayChnAmt)
    {
        Err("basic\r\n");
        goto errHandler;
    }

    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (0 != cmd->stepSeq)
        {
            Err("first step\r\n");
            goto errHandler;
        }

        for (idx=0; idx<cmd->chnAmt; idx++)
        {
            if (cmd->chnId[idx] >= dev->tray[cmd->trayIdx].trayChnAmt)
            {
                Err("chn-id\r\n");
                goto errHandler;
            }
        }
    }
    else  /*有多包*/
    {
        if (ctrl->recvMsgId != UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow)
            || ctrl->total != cmd->stepAmtTtl
            || ctrl->offsetHope != cmd->stepSeq
            || ctrl->chnAmt != cmd->chnAmt
            || memcmp(ctrl->chnIdx, cmd->chnId, cmd->chnAmt*2))
        {
            Err("multi match\r\n");
            goto errHandler;
        }
    }

    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        if (step->stepId!=cmd->stepSeq+idx || Ok!=upStepChk(cmd, step))
        {
            Err("step info\r\n");
            goto errHandler;
        }
    }

    return RspOk;

errHandler:
    recvCtrlFree();
    return RspParam;
}

u16eRspCode upProtGenChk(UpProtGenCmd *cmd, u16 pldLen)
{
    UpFixProt *fixProt;
    DevMgr *dev;
    FlowRecvCtrl *ctrl;
    u16 pldLenHope;
    u16 idx;

    dev = gDevMgr;
    ctrl = recvCtrlGet();
    pldLenHope = sizeof(UpProtGenCmd);
    for (idx=0; idx<cmd->fixProtAmt; idx++)
    {
        fixProt = (UpFixProt *)((u8 *)cmd + pldLenHope);
        pldLenHope += sizeof(UpFixProt) + Align8(fixProt->fixProtlen);
    }
    pldLenHope += sizeof(UpProtUnit) * cmd->protAmt;
    if (pldLenHope!=pldLen || cmd->protAmt>MaxProtTrans
        || cmd->protSeq+cmd->protAmt>cmd->protAmtTtl || cmd->trayIdx>=dev->trayAmt)
    {
        Err("basic\r\n");
        goto errHandler;
    }

    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (0 != cmd->protSeq) /*不管有无保护，序号必须为零*/
        {
            Err("prot seq\r\n");
            goto errHandler;
        }
    }
    else  /*有多包*/
    {
        if (ctrl->recvMsgId != UpCmdIdManuProtGen
            || ctrl->total!=cmd->protAmtTtl || 0==cmd->protAmt
            || ctrl->offsetHope != cmd->protSeq)
        {
            Err("multi match\r\n");
            goto errHandler;
        }
    }

    /*保护项的检查随用随加*/
    return RspOk;

errHandler:
    recvCtrlFree();
    return RspParam;
}

u16eRspCode upProtStepChk(UpProtStepCmd *cmd, u16 pldLen)
{
    DevMgr *dev;
    FlowRecvCtrl *ctrl;
    u16 pldLenHope;
    u16 idx;

    dev = gDevMgr;
    ctrl = recvCtrlGet();
    pldLenHope = sizeof(UpProtStepCmd) + Align16(cmd->chnAmt)*2;
    pldLenHope += sizeof(UpProtUnit) * cmd->protAmt;
    if (pldLenHope!=pldLen || cmd->protAmt>MaxProtTrans
        || cmd->protSeq+cmd->protAmt>cmd->protAmtTtl || cmd->trayIdx>=dev->trayAmt
        || cmd->chnAmt > dev->tray[cmd->trayIdx].trayChnAmt)
    {
        Err("basic\r\n");
        goto errHandler;
    }

    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (0 != cmd->protSeq)
        {
            Err("first prot\r\n");
            goto errHandler;
        }

        for (idx=0; idx<cmd->chnAmt; idx++)
        {
            if (cmd->chnId[idx] >= dev->tray[cmd->trayIdx].trayChnAmt)
            {
                Err("chn-id\r\n");
                goto errHandler;
            }
        }
    }
    else  /*有多包*/
    {
        if (ctrl->recvMsgId != UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep)
            || ctrl->total != cmd->protAmtTtl
            || ctrl->offsetHope != cmd->protSeq
            || ctrl->chnAmt != cmd->chnAmt
            || memcmp(ctrl->chnIdx, cmd->chnId, cmd->chnAmt*2))
        {
            Err("multi match\r\n");
            goto errHandler;
        }
    }

    /*保护项的检查随用随加*/
    return RspOk;

errHandler:
    recvCtrlFree();
    return RspParam;
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

void keepAliveChk(Timer *timer)
{
    if (0 == gUpItfCb->rxUpMsgCnt)
    {
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
    cmd->smplVer = 0x0001;
    cmd->stepVer = 0x0001;
    cmd->protectVer = 0x0001;
    cmd->isMultiConn = False;
    cmd->rsvd = 0;
    cmd->maxSmplSeq = gDevMgr->tray[0].smplMgr.smplSeqMax;
    pldLen = sizeof(ConnUpCmd);

    /*所有托盘*/
    for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
    {
        tlv = (ConnDevInfoTlv *)(pld+pldLen);
        tlv->devType = DevTypeTray;
        tlv->devInfoLen = sizeof(ConnTrayInfo);

        trayInfo = (ConnTrayInfo *)tlv->devInfo;
        trayInfo->cellAmt = devMgr->tray[trayIdx].trayCellAmt;
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
        tray = &devMgr->tray[trayIdx];
        bypsSw = tray->bypsSw;
        for (boxIdx=0; boxIdx<tray->boxAmt; boxIdx++)
        {
            box = &tray->box[boxIdx];
            for (subBoxIdx=0; subBoxIdx<tray->boxCfg.bypsSwAmt; subBoxIdx++, bypsSw++)
            {
                tlv = (ConnDevInfoTlv *)(pld+pldLen);
                tlv->devType = DevTypeVolSmpl;
                tlv->devInfoLen = sizeof(ConnCmmnDevInfo);

                cmmnDev = (ConnCmmnDevInfo *)tlv->devInfo;
                cmmnDev->trayIdx = trayIdx;
                cmmnDev->devIdx = bypsSw->idxInTray;
                cmmnDev->softVer = bypsSw->softVer;
                cmmnDev->online = bypsSw->online;

                pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnCmmnDevInfo);
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

    /*PLC板,目前假定一个中位机只有一个plc,随时可扩展*/
    for (trayIdx=0; trayIdx<devMgr->trayAmt; trayIdx++)
    {
        tray = &devMgr->tray[trayIdx];
        tlv = (ConnDevInfoTlv *)(pld+pldLen);
        tlv->devType = DevTypePlc;
        tlv->devInfoLen = sizeof(ConnPlcInfo);

        plc = gPlcMgr->plc;
        plcInfo = (ConnPlcInfo *)tlv->devInfo;
        plcInfo->trayIdx = trayIdx;
        plcInfo->devIdx = plc->plcIdx;
        plcInfo->softVer = plc->version;
        plcInfo->online = plc->online;
        plcInfo->ipAddr = plc->ip;

        pldLen += sizeof(ConnDevInfoTlv) + sizeof(ConnPlcInfo);
    }

    if (Ok == sendUpMsg(buf, pldLen, UpCmdIdManuConn))
    {
        devMgr->needConnUp = False;
        if (!gUpItfCb->upOnline)
        {
            gUpItfCb->upOnline = True;
            timerStart(&gUpItfCb->keepAliveTmr, TidKeepAlive, 20000, WiReset);
        }
    }

    return;
}

/*todo,目前只做了上线,需补充掉线*/
void sendUpConnNtfy()
{
    gDevMgr->needConnUp = True;
    return;
}

void upMsgConnAck(u8 *cmdPld, u16 cmdPldLen)
{
    ConnUpAck *ack;
    Times time;

    ack = (ConnUpAck *)cmdPld;
    if (0 == gAbsTimeSec)
    {
        sysTimeGet(&time);
        gAbsTimeSec = (s32)ack->absSec - time.sec;
    }
    return;
}

void upMsgFlowStep(u8 *cmdPld, u16 cmdPldLen)
{
    UpFlowCmd *cmd;
    u16eRspCode rspCode;

    cmd = (UpFlowCmd *)cmdPld;
    if (RspOk == (rspCode=upFlowChk(cmd, cmdPldLen)))
    {
        rspCode = upFlowSave(cmdPld);
    }

    rspUpCmmn(UpCmdIdManuFlow, cmd->trayIdx, rspCode);
    return;
}


/*客户叫全程或安全保护，我们也不叫global了，以免扯不清*/
void upMsgProtGen(u8 *cmdPld, u16 cmdPldLen)
{
    UpProtGenCmd *cmd;
    u16eRspCode rspCode;

    cmd = (UpProtGenCmd *)cmdPld;
    if (RspOk == (rspCode=upProtGenChk(cmd, cmdPldLen))
        && RspOk == (rspCode=protExpSave(cmdPld)))
    {
        rspCode = upProtGenSave(cmdPld);
    }
    rspUpCmmn(UpCmdIdManuProtGen, cmd->trayIdx, rspCode);
    return;
}

/*工步保护*/
void upMsgProtStep(u8 *cmdPld, u16 cmdPldLen)
{
    UpProtStepCmd *cmd;
    u16eRspCode rspCode;

    cmd = (UpProtStepCmd *)cmdPld;
    if (RspOk == (rspCode=upProtStepChk(cmd, cmdPldLen)))
    {
        rspCode = upProtStepSave(cmdPld);
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
void upMsgSmplStack(u8 *cmdPld, u16 cmdPldLen)
{
    UpSmplCmd *cmd;
    UpSmplAck *ack;
    RunStack *runStack;
    LoopDscr *loop;
    u8 *buf;
    u8 *ackPld;
    Times time;
    u16 ackPldLen;
    u8 cnt;

    if (cmdPldLen != sizeof(UpSmplCmd))
    {
        return;
    }

    buf = sendUpBuf;
    cmd = (UpSmplCmd *)cmdPld;

    ackPld = buf + sizeof(UpMsgHead);
    ack = (UpSmplAck *)ackPld;
    ack->rspCode = RspOk;
    ack->trayIdx = cmd->trayIdx;
    ack->smplAmt = 1;
    ack->firstSeq = cmd->smplSeq+1;
    ack->nextWriteSeq = cmd->smplSeq+1;
    ackPldLen = sizeof(UpSmplAck);

    runStack = ack->runStack;
    sysTimeGet(&time);
    runStack->timeStampSec = gAbsTimeSec + time.sec;
    runStack->timeStampMs = time.mSec;
    runStack->chnType = ChnTypeMainChn;
    runStack->flowLoopAmt = 1;
    runStack->chnIdx = 0;
    runStack->capacity = 3998;
    ackPldLen += sizeof(RunStack);
    
    for (cnt=0; cnt<runStack->flowLoopAmt; cnt++)
    {
        loop = (LoopDscr *)(ackPld + ackPldLen);
        loop->stepId = 3;
        loop->leftAmt = 6;
        ackPldLen += sizeof(LoopDscr);
    }

    sendUpMsg(buf, ackPldLen, UpCmdIdManuSmplStack);

    return;
}
void upMsgSmplTray(u8 *cmdPld, u16 cmdPldLen)
{
    UpSmplCmd *cmd;
    TraySmplAck *ack;
    SmplSaveMgr *smplMgr;
    Tray *tray;
    u8 *buf;
    u16 pldLenAck;

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
    smplMgr->smplSeqUpReq = cmd->smplSeq;

    ack->trayIdx = cmd->trayIdx;
    ack->rspSmplSeq = cmd->smplSeq;
    ack->nextGenSeq = smplMgr->smplSeqNext;
    pldLenAck = sizeof(TraySmplAck);
    if (cmd->smplSeq==smplMgr->smplSeqNext || (!smplMgr->isLooped && cmd->smplSeq>smplMgr->smplSeqNext))
    {
        ack->smplAmt = 0;
        sendUpMsg(buf, pldLenAck, UpCmdIdManuSmplTray);
        return;
    }

    ack->smplAmt = 1;
    memcpy(ack->traySmplRcd, smplMgr->smplBufAddrBase+cmd->smplSeq*smplMgr->smplItemSize, smplMgr->smplItemSize);
    pldLenAck += smplMgr->smplItemSize;
    sendUpMsg(buf, pldLenAck, UpCmdIdManuSmplTray);
    return;
}

u16eRspCode boxStartChk(Box *box)
{
    if (!box->online)
    {
        return RspDisc;
    }
    if (BoxModeManu != box->boxWorkMode)
    {
        return RspStatus;
    }

    /*todo,其它通道状态*/
    return RspOk;
}
u16eRspCode chnStartChk(Channel *chn)
{
    /*todo,其它通道状态*/
    return RspOk;
}
u16eRspCode cellStartChk(Cell *cell)
{
    /*todo,其它电芯判断*/
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

u16eRspCode upMsgStartParalChk(UpFlowCtrlCmd *upCmd, Tray *tray)
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

        for (chnInd=upCmd->startInd,sentry=&upCmd->startInd[upCmd->chnAmt]; chnInd<sentry; chnInd++)
        {
            if (chnInd->chnId >= tray->trayChnAmt)
            {
                return RspParam;
            }

            chn = &tray->chn[chnInd->chnId];
        #ifdef TmpStepSave
            if (chnInd->stepId >= gTmpStepAmt)
        #else
            if (chnInd->stepId >= chn->flowEntry->stepAmt)
        #endif
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

/*todo,目前这个检查包含并联,后续若无变化则合并*/
u16eRspCode upMsgStartSeriesWoSwChk(UpFlowCtrlCmd *upCmd, Tray *tray)
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
        u16 chnIdx;

        for (chnInd=upCmd->startInd,sentry=&upCmd->startInd[upCmd->chnAmt]; chnInd<sentry; chnInd++)
        {
            if (0 != chnInd->chnId % (tray->boxCfg.chnCellAmt+1))
            {
                return RspParam;
            }

            chnIdx = chnInd->chnId / (tray->boxCfg.chnCellAmt+1);
            if (chnIdx >= tray->trayChnAmt)
            {
                return RspParam;
            }

            chn = &tray->chn[chnIdx];
        #ifdef TmpStepSave
            if (chnInd->stepId >= gTmpStepAmt)
        #else
            if (chnInd->stepId >= chn->flowEntry->stepAmt)
        #endif
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

u16eRspCode upMsgStartSeriesWiSwChk(UpFlowCtrlCmd *upCmd, Tray *tray)
{
    return RspOk;
}

u16eRspCode upMsgFlowStartChk(UpFlowCtrlCmd *upCmd, u16 pldLen, DevMgr *dev)
{
    Tray *tray;
    TmprSmpl *tmprSmpl;

    if (pldLen != sizeof(UpFlowCtrlCmd) + upCmd->chnAmt*sizeof(UpStartChnInd)
        || upCmd->trayIdx>=dev->trayAmt)
    {
        return RspParam;
    }

    tray = &dev->tray[upCmd->trayIdx];
#ifdef NdbdStateWoChk
#else
    if (!tray->ndbdData.trayIsEnter || !tray->ndbdData.trayIsTouch)
    {
        return RspTouch;
    }
#endif

    if (trayHasWarn(tray))
    {
        return RspWarn;
    }

    tmprSmpl = &gDevMgr->tmprSmpl[tray->tmprSmplIdxBase];
    if (!tmprSmpl->online || !plcBeOnline(tray->plcIdx))
    {
        return RspDisc;
    }

    if (BoxTypeParallel == tray->boxCfg.boxType)
    {
        return upMsgStartParalChk(upCmd, tray);
    }
    if (BoxTypeSeriesWoSw == tray->boxCfg.boxType)
    {
        return upMsgStartSeriesWoSwChk(upCmd, tray);
    }
    else /*(BoxTypeSeriesWiSw == tray->boxCfg.boxType)*/
    {
        return upMsgStartSeriesWiSwChk(upCmd, tray);
    }

    return RspOk;
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
    u16 rspCode;

    dev = gDevMgr;
    upCmd = (UpFlowCtrlCmd *)pld;
    if (RspOk != (rspCode=upMsgFlowStartChk(upCmd, pldLen, dev)))
    {
        goto rspUp;
    }

    if (NULL != recvCtrlGet()) /*流程或保护未完成*/
    {
        recvCtrlFree();
        rspCode = RspStep;
        goto rspUp;
    }

    /*todo,负压申请*/

    tray = &dev->tray[upCmd->trayIdx];
    if (0 == upCmd->chnAmt)
    {
        Channel *chnCri;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (Ok == chnProtReverse(chn))
            {
                chn->stepIdTmp = 0;
                chn->smplSeqInStep = 0;
                if (Ok == trayNpChnAlloc(tray, chn, 0))
                {
                    chn->chnStateMed = ChnStaFlowStart;
                    chn->dynStaCnt = 0;
                    boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
                }
                else
                {
                    chn->chnStateMed = ChnStaNp;
                }
            }
        }
    }
    else
    {
        UpStartChnInd *chnInd;
        UpStartChnInd *sentry;

        for (chnInd=upCmd->startInd,sentry=&upCmd->startInd[upCmd->chnAmt]; chnInd<sentry; chnInd++)
        {
            chn = &tray->chn[chnInd->chnId / (tray->boxCfg.chnCellAmt+1)];
            if (Ok == chnProtReverse(chn))
            {
                chn->stepIdTmp = 0;
                chn->smplSeqInStep = 0;
                if (Ok == trayNpChnAlloc(tray, chn, 0))
                {
                    chn->chnStateMed = ChnStaFlowStart;
                    chn->dynStaCnt = 0;
                    boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStart);
                }
                else
                {
                    chn->chnStateMed = ChnStaNp;
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

void upMsgFlowPause(u8 *pldCmd, u16 pldLenCmd)
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

    /*通道检查以后补充,todo*/
    ack->rspCode = RspOk;

errHandler:
    sendUpMsg(buf, sizeof(UpCmmnAck), UpCmdId(UpMsgGrpManu, UpMsgIdManuPause));
    return;
}

u16eRspCode upMsgFlowStopChk(UpFlowCtrlCmd *upCmd, u16 pldLen, DevMgr *dev)
{
    Tray *tray;
    u16 idx;
    u16 chnIdx;

    if (pldLen != sizeof(UpFlowCtrlCmd) + Align16(upCmd->chnAmt)*2
        || upCmd->trayIdx>=dev->trayAmt)
    {
        return RspParam;
    }

    tray = &dev->tray[upCmd->trayIdx];
    for (idx=0; idx<upCmd->chnAmt; idx++) /*todo,旁路串联有不同*/
    {
        if (0 != upCmd->chnId[idx] % (tray->boxCfg.chnCellAmt+1))
        {
            return RspParam;
        }
        
        chnIdx = upCmd->chnId[idx] / (tray->boxCfg.chnCellAmt+1);
        if (chnIdx >= tray->trayChnAmt)
        {
            return RspParam;
        }
    }

    return RspOk;
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
    if (0 == upCmd->chnAmt) /*todo,识别出那些跑的去停而不是全停*/
    {
        Channel *chnCri;

        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (ChnStaRun==chn->chnStateMed || ChnStaFlowStart==chn->chnStateMed
                || ChnStaStartReqLow==chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaStopReqUp;
                chn->dynStaCnt = 0;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else
            {
                chn->chnStateMed = ChnStaIdle;
            }
            trayNpChnFree(tray, chn);
        }
    }
    else /*todo,识别出那些跑的去停而不是全停*/
    {
        u16 idx;

        for (idx=0; idx<upCmd->chnAmt; idx++)
        {
            chn = &tray->chn[upCmd->chnId[idx] / (tray->boxCfg.chnCellAmt+1)];
            if (ChnStaRun==chn->chnStateMed || ChnStaFlowStart==chn->chnStateMed
                || ChnStaStartReqLow==chn->chnStateMed)
            {
                chn->chnStateMed = ChnStaStopReqUp;
                chn->dynStaCnt = 0;
                boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            }
            else
            {
                chn->chnStateMed = ChnStaIdle;
            }
            trayNpChnFree(tray, chn);
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

void upMsgFlowCtnu(u8 *pldCmd, u16 pldLenCmd)
{
    UpFlowCtrlCmd *cmd;
    UpCtnuChnlInd *ctnuInd;
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

    if (cmd->trayIdx>=dev->trayAmt)
    {
        Err("basic\r\n");
        goto errHandler;
    }

    pldLenHope = sizeof(UpFlowCtrlCmd);
    for (idx=0; idx<cmd->chnAmt; idx++)
    {
        ctnuInd = (UpCtnuChnlInd *)(pldCmd + pldLenHope);
        if (ctnuInd->chnId >= dev->tray[cmd->trayIdx].trayChnAmt)
        {
            Err("chn id\r\n");
            goto errHandler;
        }

        pldLenHope += sizeof(UpCtnuChnlInd) + sizeof(LoopDscr)*ctnuInd->loopAmt;
    }

    if (pldLenHope != pldLenCmd)
    {
        Err("pld len\r\n");
        goto errHandler;
    }

    /*通道和其它检查以后补充,todo*/
    if (NULL != recvCtrlGet())
    {
        recvCtrlFree();
        Err("lack info\r\n");
        goto errHandler;
    }

    ack->rspCode = RspOk;

errHandler:
    sendUpMsg(buf, sizeof(UpCmmnAck), UpCmdIdManuCtnu);
    return;
}

/*todo,里面的reset都要挪走*/
void upMsgWarnDel(u8 *pldCmd, u16 pldLenCmd)
{
    Tray *tray;
    Channel *chn;
    Channel *chnCri;
    TrayProtMgr *trayProtMgr;
    ChnProtBuf *protBuf;
    Cell *cell;
    Cell *cellCri;
    u8 idx;

    tray = &gDevMgr->tray[*pldCmd];
    trayProtMgr = &tray->trayProtMgr;
    tray->trayWarnPres = False;
    memset(trayProtMgr->mixProtHpn, 0, MixExpAmt);
    memset(trayProtMgr->policyActNeed, 0, Align8(PolicyCri));
    memset(trayProtMgr->policyActOver, 0, Align8(PolicyCri));
    trayProtMgr->slotTmprCrnt.tmprInvalidCnt = 0;
    trayProtMgr->smokeHpnCnt = 0;
    trayProtMgr->allSlotTmprUpLmtCnt = 0;
    trayProtMgr->busySlotTmprLowLmtCnt = 0;

    TimerStop(&tray->trayProtMgr.protPolicyTmr);
    for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        protBuf = &chn->chnProtBuf;
        protBuf->preCauseCode = CcNone;
        protBuf->mixSubHpnBitmap = 0;
        protBuf->cellTmprCrnt.tmprInvalidCnt = 0;
        protBuf->idleVolFluctSmlCnt = 0;
        protBuf->idleTmprUpSmlCnt = 0;
        protBuf->idleCurLeakCnt = 0;
        protBuf->allChnTmprUpLmtCnt = 0;
        protBuf->busyChnTmprLowLmtCnt = 0;
        if (NULL != chn->series)
        {
            for (cell=chn->series->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                protBuf = &cell->chnProtBuf;
                protBuf->preCauseCode = CcNone;
                protBuf->mixSubHpnBitmap = 0;
                protBuf->cellTmprCrnt.tmprInvalidCnt = 0;
                protBuf->idleVolFluctSmlCnt = 0;
                protBuf->idleTmprUpSmlCnt = 0;
                protBuf->idleCurLeakCnt = 0;
                protBuf->allChnTmprUpLmtCnt = 0;
                protBuf->busyChnTmprLowLmtCnt = 0;
            }
        }
    }

    plcRegWriteTry(*pldCmd, NdbdSetWarnDel, 1);
    rspUpCmmn(UpCmdIdManuWarnDel, *pldCmd, RspOk);
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

void setCaliAuxCanBuf(CanAuxCmdBuf *auxBuf, Box *box, u16 upMsgId)
{
    auxBuf->box = box;
    auxBuf->reTxCnt = 0;
    auxBuf->upMsgFlowSeq = gUpItfCb->upMsgFlowSeq;
    auxBuf->upCanMsgId = upMsgId;
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

/*todo,增加防呆,例如模式切换,状态是否允许*/
u16eRspCode upMsgUpdSetupChk(UpUpdSetupCmd *upCmd, u16 pldLen)
{
    if (upCmd->trayIdx>=gDevMgr->trayAmt || upCmd->devType>=UpUpdDevCri
        || upCmd->fileType>=UpdateFileCri)
    {
        return RspParam;
    }

#if 0
    if (gUpItfCb->updMgr.updWorking)
    {
        return RspBusy;
    }
#endif

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
        uart = &devMgr->uart[devMgr->ndbdUartIdx];
        *mapAddr = 0;
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
        box->boxWorkMode = BoxModeMntn;
        traySmplMgrRst(tray);
    }
    else  /*todo,增加温度盒等uart设备的维护模式*/
    {
    #if 0  /*todo, 不是无效代码，只是临时注释掉*/
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
            rspUpUpdCnfm(rspCode, 0);
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

    if (NULL == (blockBuf = allocUartBlockBuf()))
    {
        rspUpCmmn(UpCmdIdFixtNtfy, fixtCmd->trayIdx, RspBusy);
        return;
    }

    uartMgr = gUartMgr;
    tray = &gDevMgr->tray[fixtCmd->trayIdx];
    uartDevType = uartMgr->upFixt2UartType[fixtCmd->fixtType];
    tray->fixtUartCmmuAddr = uartMgr->uartAddrBase[uartDevType] + fixtCmd->actAddr;
    setUartBlockBuf(blockBuf, fixtCmd->trayIdx, UpCmdIdFixtNtfy,
                    tray->fixtUartCmmuAddr, uartDevType, UartMsgIdCmmn(UartMsgConn));
    ((UartMsgHead *)blockBuf->msgBuf)->pldLen = sizeof(UartCmmnCmd);
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    tray->trayWorkMode = fixtCmd->enable ? BoxModeMntn : BoxModeManu;  /*todo, 起定时防呆,以及,去使能时不用通知工装*/
    return;
}

u16eRspCode upFixtPrecChnlSwChk(UpFixtPrecSwCmd *fixtCmd, u16 pldLen)
{
    if (pldLen!=sizeof(UpFixtPrecSwCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->caliType>=CaliTypeCri || fixtCmd->chnSwType>=FixtChnSwCri)
    {
        return RspParam;
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
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtPrecSmplChk(UpFixtPrecSmplCmd *fixtCmd, u16 pldLen)
{
    if (pldLen!=sizeof(UpFixtPrecSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->smplType>=FixtPrecSmplCri)
    {
        return RspParam;
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
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtPrecOutChk(UpFixtPrecOutCmd *fixtCmd, u16 pldLen)
{
    if (pldLen!=sizeof(UpFixtPrecOutCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
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
    if (pldLen!=sizeof(UpFixtTmprHeatCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
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
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtTmprSmplChk(UpFixtTmprSmplCmd *fixtCmd, u16 pldLen)
{
    if (pldLen!=sizeof(UpFixtTmprSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->chnAmt > MaxFixtTmprSmplAmt)
    {
        return RspParam;
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
    if (pldLen!=sizeof(UpFixtCleanActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->actType >= FixtCleanActCri)
    {
        return RspParam;
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
    if (pldLen!=sizeof(UpFixtGasSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
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
    if (pldLen!=sizeof(UpFixtFlowSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
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
    if (pldLen!=sizeof(UpFixtSuckInSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
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
    if (pldLen!=sizeof(UpFixtSuckOutActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->action > FixtSuckOutStaHold)
    {
        return RspParam;
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
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtSuctOutSmplChk(UpFixtSuckOutSmplCmd *fixtCmd, u16 pldLen)
{
    if (pldLen!=sizeof(UpFixtSuckOutSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
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
    if (pldLen!=sizeof(UpFixtLocatSmplCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt)
    {
        return RspParam;
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
    uartTransTxTry(gDevMgr->fixtUartIdx, blockBuf);
    return;
}

u16eRspCode upFixtLocatActChk(UpFixtLocatActCmd *fixtCmd, u16 pldLen)
{
    if (pldLen!=sizeof(UpFixtLocatActCmd) || fixtCmd->trayIdx>=gDevMgr->trayAmt
        || fixtCmd->anodeTurn > FixtSuckOutStaHold || fixtCmd->npTurn > FixtSuckOutStaHold
        || fixtCmd->cathodeTurn > FixtSuckOutStaHold)
    {
        return RspParam;
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
    uartCmd->delayTime = fixtCmd->delayTime;
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
        return RspDisc;
    }

    /*todo,检查是否允许进入*/
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
    Box *box;
    u16eRspCode rspCode;

    upCmd = (UpCaliNtfyCmd *)pld;
    if (RspOk != (rspCode = upCaliNtfyChk(upCmd, pldLen))
        || RspOk != (rspCode = NULL==(auxBuf=allocAuxCanBuf()) ? RspBusy : RspOk))
    {
        rspUpCaliNtfyAck(upCmd->trayIdx, upCmd->boxIdx, rspCode);
        return;
    }

    box = &gDevMgr->tray[upCmd->trayIdx].box[upCmd->boxIdx];
    setCaliAuxCanBuf(auxBuf, box, UpCmdIdCaliNtfy);
    boxHead = (BoxMsgHead *)auxBuf->msgBuf;
    boxHead->msgId = BoxMsgCaliNtfy;
    boxHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliNtfyCmd);
    boxCmd = (BoxCaliNtfyCmd *)boxHead->payload;
    boxCmd->caliEnable = upCmd->caliEnable;
    boxAuxTxTry(box->canIdx, auxBuf);
    if (upCmd->caliEnable)
    {
        box->boxWorkMode = BoxModeMntn;
        traySmplMgrRst(box->tray);
    }
    return;
}

/*托盘统排通道号到通道内索引的映射,不检查参数,只针对修调*/
Channel *caliChnIdx2Chn(Tray *tray, u16 chnIdx, u16 *genIdxInChn)
{
    Channel *chn;

    if (BoxTypeParallel == tray->boxCfg.boxType)
    {
        *genIdxInChn = 0;
        chn = &tray->chn[chnIdx];
    }
    else
    {
        ChnMap *chnMap;

        chnMap = &tray->chnMap[chnIdx];
        if (chnMap->isMainChn)
        {
            *genIdxInChn = 0;
            chn = &tray->chn[chnMap->typeIdxInTray];
        }
        else
        {
            Cell *cell;

            cell = &tray->cell[chnMap->typeIdxInTray];
            *genIdxInChn = cell->cellIdxInChnl + 1;
            chn = cell->chn;
        }
    }

    return chn;
}

/*todo,检查时记得区分启动和停止*/
u16eRspCode upCaliStartChk(UpCaliStartCmd *caliCmd, u16 pldLen)
{
    if (caliCmd->trayIdx >= gDevMgr->trayAmt)
    {
        return RspParam;
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
    chn = caliChnIdx2Chn(tray, upCmd->chnIdx, &genIdxInChn);
    setCaliAuxCanBuf(auxBuf, chn->box, UpCmdIdCaliStart);
    boxMsgHead = (BoxMsgHead *)auxBuf->msgBuf;
    if (upCmd->isStart)
    {
        boxMsgHead->msgId = BoxMsgCaliStart;
        boxCmd = (BoxCaliStartCmd *)boxMsgHead->payload;
        boxCmd->moduIdx = chn->box->chnModuAmt*chn->chnIdxInBox + upCmd->moduIdx;
        boxCmd->chnIdx = genIdxInChn;
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

    boxAuxTxTry(chn->box->canIdx, auxBuf);
    return;
}

u16eRspCode upCaliSmplChk(UpCaliSmplCmd *caliCmd, u16 pldLen)
{
    if (caliCmd->trayIdx >= gDevMgr->trayAmt)
    {
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
    chn = caliChnIdx2Chn(tray, upCmd->chnIdx, &genIdxInChn);
    setCaliAuxCanBuf(auxBuf, chn->box, UpCmdIdCaliSmpl);
    boxMsgHead = (BoxMsgHead *)auxBuf->msgBuf;
    boxMsgHead->msgId = BoxMsgCaliSmpl;
    boxMsgHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliSmplCmd);
    boxCmd = (BoxCaliSmplCmd *)boxMsgHead->payload;
    boxCmd->moduIdx = chn->box->chnModuAmt*chn->chnIdxInBox + upCmd->moduIdx;
    boxCmd->chnIdx = genIdxInChn;
    boxCmd->smplAllSeries = 0==genIdxInChn && !upCmd->smplOnlyMainChn ? True : False;
    boxAuxTxTry(chn->box->canIdx, auxBuf);
    return;
}

u16eRspCode upCaliKbChk(UpCaliKbCmd *caliCmd, u16 pldLen)
{
    if (caliCmd->trayIdx >= gDevMgr->trayAmt)
    {
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
    chn = caliChnIdx2Chn(tray, upCmd->chnIdx, &genIdxInChn);
    setCaliAuxCanBuf(auxBuf, chn->box, UpCmdIdCaliKb);
    boxMsgHead = (BoxMsgHead *)auxBuf->msgBuf;
    boxMsgHead->msgId = BoxMsgCaliKb;
    boxMsgHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliKbCmd);
    boxCmd = (BoxCaliKbCmd *)boxMsgHead->payload;
    boxCmd->moduIdx = chn->box->chnModuAmt*chn->chnIdxInBox + upCmd->moduIdx;
    boxCmd->chnIdx = genIdxInChn;
    boxCmd->caliType = upCmd->caliType;
    boxCmd->volType = upCmd->volType;
    boxCmd->caliKbAmt = upCmd->caliKbAmt;
    mem2Copy(boxCmd->caliKb, (u16 *)upCmd->caliKb, upCmd->caliKbAmt*sizeof(CaliKb));
    boxMsgHead->msgLen = sizeof(BoxMsgHead) + sizeof(BoxCaliKbCmd) + upCmd->caliKbAmt*sizeof(CaliKb);
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
        plcRegWriteTry(upCmd->trayIdx, NdbdSetTouch, 0==upCmd->oprPld[0] ? 1 : 2);
    }
    else if (NdbdActFireDoor == upCmd->oprType)
    {
        plcRegWriteTry(upCmd->trayIdx, NdbdSetFireDoor, upCmd->oprPld[0]);
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

    plcRegWriteTry(upCmd->trayIdx, NdbdSetFixtPower, upCmd->powerEnable ? 1 : 0);
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

            Show("\r\nrecv upper msg: 0x%04x\r\n", msgHead->msgId);
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
    mgr->upMsgProcManu[UpMsgIdManuProtGen] = upMsgProtGen;
    mgr->upMsgProcManu[UpMsgIdManuProtStep] = upMsgProtStep;
    mgr->upMsgProcManu[UpMsgIdManuStart] = upMsgFlowStart;
    mgr->upMsgProcManu[UpMsgIdManuPause] = upMsgFlowPause;
    mgr->upMsgProcManu[UpMsgIdManuStop] = upMsgFlowStop;
    mgr->upMsgProcManu[UpMsgIdManuJump] = upMsgFlowJump;
    mgr->upMsgProcManu[UpMsgIdManuCtnu] = upMsgFlowCtnu;
    mgr->upMsgProcManu[UpMsgIdManuWarnDel] = upMsgWarnDel;

    /*配置升级类*/
    for (idx=0; idx<UpMsgIdUpdCri; idx++)
    {
        mgr->upMsgProcUpdate[idx] = (UpMsgProc)upMsgIgnore;
    }
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

    /*插拔吸嘴工装*/
    for (idx=0; idx<UpMsgIdFixtSuctOutCri; idx++)
    {
        mgr->upMsgFixtSuctOutDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtSuctOutDisp[UpMsgIdFixtSuctOutAct] = upMsgFixtSuctOutAct;
    mgr->upMsgFixtSuctOutDisp[UpMsgIdFixtSuctOutSmpl] = upMsgFixtSuctOutSmpl;

    /*定位工装*/
    for (idx=0; idx<UpMsgIdFixtLocatCri; idx++)
    {
        mgr->upMsgFixtLocatDisp[idx] = (UpMsgProc)upMsgIgnore;
    }
    mgr->upMsgFixtLocatDisp[UpMsgIdFixtLocatSmpl] = upMsgFixtLocatSmpl;
    mgr->upMsgFixtLocatDisp[UpMsgIdFixtLocatAct] = upMsgFixtLocatAct;

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
    timerStart(&mgr->connUpTmr, TidConnUpper, 2000, WiReset);
    return;
}


