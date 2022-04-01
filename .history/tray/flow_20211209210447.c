

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"
#include "cause.h"
#include "func.h"
#include "log.h"
#include "entry.h"
#include "flow.h"
#include "host.h"
#include "box.h"
#include "protect.h"

FlowCb *gFlowCb;

#ifdef TmpStepSave
u8 *gTmpFlowInfo; /*todo,临时用，要删除*/
u8 *gTmpStepNpInfo;
u8 gTmpStepAmt; /*todo,临时用，要删除*/
u8 *gTmpProtGenInfo; /*todo,临时用，要删除*/
u8 gTmpProtGenAmt; /*todo,临时用，要删除*/
u8 *gTmpProtStepInfo; /*todo,临时用，要删除*/
u8 gTmpProtStepAmt; /*todo,临时用，要删除*/
u8eStepProtPolicy gTmpStepProtPolicy;
s32 gTmpReverseVol;
#endif

FlowRecvCtrl *recvCtrlGet()
{
    return gFlowCb->recvCtrl;
}

FlowRecvCtrl *recvCtrlSetup(u16 chnAmt)
{
    FlowRecvCtrl *ctrl;

    ctrl = memAlloc(sizeof(FlowRecvCtrl));
    if (NULL != ctrl)
    {
        ctrl->chnIdx = NULL;
        if (0 != chnAmt)
        {
            ctrl->chnIdx = memAlloc(chnAmt*2);
            if (NULL == ctrl->chnIdx)
            {
                memFree(ctrl);
                Warn("err: chn\r\n");
                return NULL;
            }
        }

        gFlowCb->recvCtrl = ctrl;
        return ctrl;
    }

    Warn("err: ctrl\r\n");
    return NULL;
}

void recvCtrlFree()
{
    if (NULL != gFlowCb->recvCtrl)
    {
        if (NULL != gFlowCb->recvCtrl->chnIdx)
        {
            memFree(gFlowCb->recvCtrl->chnIdx);
        }

        memFree(gFlowCb->recvCtrl);
        gFlowCb->recvCtrl = NULL;
    }

    return;
}

u16eRspCode upFlowSave(u8    *pld)
{
    UpStepInfo *step;
    FlowRecvCtrl *ctrl;
    UpFlowCmd *cmd;

    cmd = (UpFlowCmd *)pld;
    ctrl = recvCtrlGet();
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (cmd->stepAmt < cmd->stepAmtTtl)
        {
            return RspNoRes;  /*todo,暂时不允许多包,回头改成允许*/
            if (NULL == (ctrl=recvCtrlSetup(cmd->chnAmt)))
            {
                return RspBusy;
            }

            /*记录下来，用作下次收到时检查*/
            Trc("first flow\r\n");
            ctrl->recvMsgId = UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow);
            ctrl->total = cmd->stepAmtTtl;
            ctrl->offsetHope = cmd->stepAmt;
            ctrl->chnAmt = cmd->chnAmt;
            if (0 != cmd->chnAmt)
            {
                memcpy(ctrl->chnIdx, cmd->chnId, ctrl->chnAmt*2);
            }
        }
    }
    else
    {
        ctrl->offsetHope += cmd->stepAmt;
        if (ctrl->offsetHope == ctrl->total)  /*收完了*/
        {
            recvCtrlFree();
            Trc("multi over\r\n");
        }
    }

#ifdef TmpStepSave
    if (sizeof(UpStepInfo)*cmd->stepAmt < 1536)
    {
        memcpy(gTmpFlowInfo, &cmd->chnId[Align16(cmd->chnAmt)], sizeof(UpStepInfo)*cmd->stepAmt);
        gTmpStepAmt = cmd->stepAmt;
    }
    else
    {
        return RspNoRes;
    }
#else
#endif
    return Ok;
}

u16eRspCode upProtGenSave(u8 *pld)
{
    FlowRecvCtrl *ctrl;
    UpProtGenCmd *cmd;

    cmd = (UpProtGenCmd *)pld;
    ctrl = recvCtrlGet();
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (cmd->protAmt < cmd->protAmtTtl)
        {
            if (NULL == (ctrl=recvCtrlSetup(0)))
            {
                return RspBusy;
            }

            /*记录下来，用作下次收到时检查*/
            ctrl->recvMsgId = UpCmdIdManuProtGen;
            ctrl->total = cmd->protAmtTtl;
            ctrl->offsetHope = cmd->protAmt;
            Trc("recv first\r\n");
        }
    }
    else
    {
        ctrl->offsetHope += cmd->protAmt;
        if (ctrl->offsetHope == ctrl->total)  /*收完了*/
        {
            recvCtrlFree();
            Trc("multi over\r\n");
        }
    }

#ifdef TmpStepSave
    if (cmd->protSeq == 0)
    {
        gTmpProtGenAmt = 0;
        memset(&gDevMgr->tray[cmd->trayIdx].npMgr, 0, sizeof(TrayNpMgr));
    }
    if (sizeof(UpProtUnit)*(cmd->protAmt+gTmpProtGenAmt) < 512)
    {
        u8 *pos;
        UpFixProt *fixProt;
        u16 idx;

        pos = pld + sizeof(UpProtGenCmd);
        for (idx=0; idx<cmd->fixProtAmt; idx++)
        {
            fixProt = (UpFixProt *)pos;
            pos += sizeof(UpFixProt) + Align8(fixProt->fixProtlen);
        }
        memcpy(gTmpProtGenInfo+sizeof(UpProtUnit)*gTmpProtGenAmt, pos, cmd->protAmt*sizeof(UpProtUnit));
        gTmpProtGenAmt += cmd->protAmt;

        if (gTmpProtGenAmt == cmd->protAmtTtl)
        {
            saveGenProtCfg(cmd->trayIdx, gTmpProtGenAmt, gTmpProtGenInfo);
        }
    }
    else
    {
        return RspNoRes;
    }
#else
#endif
    return RspOk;
}

u16eRspCode upProtStepSave(u8 *pld)
{
    FlowRecvCtrl *ctrl;
    UpProtStepCmd *cmd;

    cmd = (UpProtStepCmd *)pld;
    ctrl = recvCtrlGet();
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (cmd->protAmt < cmd->protAmtTtl)
        {
            if (NULL == (ctrl=recvCtrlSetup(cmd->chnAmt)))
            {
                return RspBusy;
            }

            /*记录下来，用作下次收到时检查*/
            Trc("first prot\r\n");
            ctrl->recvMsgId = UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep);
            ctrl->total = cmd->protAmtTtl;
            ctrl->offsetHope = cmd->protAmt;
            ctrl->chnAmt = cmd->chnAmt;
            if (0 != ctrl->chnAmt)
            {
                memcpy(ctrl->chnIdx, cmd->chnId, ctrl->chnAmt*2);
            }
        }
    }
    else
    {
        ctrl->offsetHope += cmd->protAmt;
        if (ctrl->offsetHope == ctrl->total)  /*收完了*/
        {
            recvCtrlFree();
            Trc("multi over\r\n");
        }
    }

#ifdef TmpStepSave
    if (cmd->protSeq == 0)
    {
        gTmpProtStepAmt = 0;
        memset(gTmpStepNpInfo, 0, 512);
    }
    if (sizeof(UpProtUnit)*(cmd->protAmt+gTmpProtStepAmt) < 3072)
    {
        memcpy(gTmpProtStepInfo+sizeof(UpProtUnit)*gTmpProtStepAmt, &cmd->chnId[Align16(cmd->chnAmt)], cmd->protAmt*sizeof(UpProtUnit));
        gTmpProtStepAmt += cmd->protAmt;
        if (gTmpProtStepAmt == cmd->protAmtTtl)
        {
            UpProtUnit *prot;
            UpProtUnit *cri;
            StepNpReq *stepNp;

            for (prot=(UpProtUnit *)gTmpProtStepInfo,cri=prot+cmd->protAmtTtl; prot<cri; prot++)
            {
                if (0x0500 == prot->protId)  /*反接*/
                {
                    if (prot->paramEnable & 0x01)
                    {
                        gTmpStepProtPolicy = 0==prot->protParam[0] ? StepProtPause : StepProtStop;
                    }
                    else
                    {
                        gTmpStepProtPolicy = StepProtPause;
                    }

                    if (prot->paramEnable & 0x04)
                    {
                        gTmpReverseVol = prot->protParam[2];
                    }
                    else
                    {
                        gTmpReverseVol = -8000000;
                    }
                    break;
                }
                else if (0x0400 == prot->protId)
                {
                    stepNp = (StepNpReq *)(gTmpStepNpInfo + sizeof(StepNpReq)*prot->stepId);
                    stepNp->npType = prot->protParam[0];
                    stepNp->stepNpExpect = prot->protParam[1];
                    if (NpTypeRatio==stepNp->npType && 0==stepNp->stepNpExpect)
                    {
                        stepNp->npType = NpTypeNml; /*常压*/
                    }
                    stepNp->stepNpMax = prot->protParam[2];
                    stepNp->stepNpMin = prot->protParam[3];
                }
            }
        }
    }
    else
    {
        return RspNoRes;
    }
#else
#endif
    return RspOk;
}

void *getChnStepInfo(u8 stepId)
{
    if (stepId < gTmpStepAmt)
    {
        return gTmpFlowInfo + sizeof(UpStepInfo)*stepId;
    }

    return NULL;
}

Ret flowInit()
{
    FlowCb *cb;

    if (NULL == (cb = gFlowCb = sysMemAlloc(sizeof(FlowCb))))
    {
        return Nok;
    }

    gFlowCb->recvCtrl = NULL;
#ifdef TmpStepSave
    gTmpFlowInfo = sysMemAlloc(1536);
    gTmpStepNpInfo = sysMemAlloc(512);
    gTmpStepAmt = 0;
    gTmpProtGenInfo = sysMemAlloc(512);
    gTmpProtGenAmt = 0;
    gTmpProtStepInfo = sysMemAlloc(3072);
    gTmpProtStepAmt = 0;
    gTmpStepProtPolicy = StepProtPause;
    gTmpReverseVol = -8000000;  /*不使能*/
#else
#endif
    return Ok;
}


