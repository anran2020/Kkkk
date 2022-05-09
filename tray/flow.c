
/*本文件完成保护和工步的解析存储,并提供工步参数的查询*/
/*本属于中上协议,但存储相对独立,故单列文件*/
/*目前只存储到内存,不存储到flash*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basic.h"
#if TRAY_ENABLE
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

FlowRecvCtrl *recvCtrlGet(u8 trayIdx)
{
    return gFlowCb->recvCtrl[trayIdx];
}

FlowRecvCtrl *recvCtrlSetup(u8 trayIdx, u16 chnAmt)
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
                return NULL;
            }
        }

        gFlowCb->recvCtrl[trayIdx] = ctrl;
        return ctrl;
    }

    return NULL;
}

void recvCtrlFree(u8 trayIdx)
{
    FlowRecvCtrl *ctrl;

    ctrl = gFlowCb->recvCtrl[trayIdx];
    if (NULL != ctrl)
    {
        if (NULL != ctrl->chnIdx)
        {
            memFree(ctrl->chnIdx);
        }

        memFree(ctrl);
        gFlowCb->recvCtrl[trayIdx] = NULL;
    }

    return;
}

/*工步到参数群组的映射*/
u16eRspCode stepType2ObjAmtId(u8eStepType stepType, u8eObjAmtId *id)
{
    switch (stepType)
    {
        case StepTypeQuiet:
        case StepTypeLoop:
            *id = ObjAmtId2;
            break;
        case StepTypeCCC:
        case StepTypeCCD:
            *id = ObjAmtId4;
            break;
        case StepTypeCCCVC:
        case StepTypeCCCVD:
        case StepTypeCVC:
        case StepTypeCVD:
            *id = ObjAmtId6;
            break;
        default:
            return RspUnknown; /*不能走到这里*/
    }
    return RspOk;
}

/*工步保护和流程保护到保护群组和参数群组的映射*/
u16eRspCode stepProtId2ObjAmtId(u16 protId, u8eObjAmtId *id, u8eProtGrp *protGrp)
{
    if (protId > 0x0500)  /*流程保护*/
    {
        *protGrp = ProtGrpFlow;
        switch (protId)
        {
            case 0x0501:
            case 0x0505:
            case 0x0506:
            case 0x0507:
            case 0x0508:
            case 0x0509:
            case 0x050a:
                *id = ObjAmtId2;
                break;
            case 0x0502:
            case 0x0503:
            case 0x0504:
                *id = ObjAmtId4;
                break;
            default:
                return RspUnknown;
                break;
        }
    }
    else  /*工步保护*/
    {
        if (protId < ProtGrpStep1IdCri)
        {
            *protGrp = ProtGrpStep2;
            switch (protId)
            {
                case 0x0401:
                case 0x0402:
                case 0x0403:
                case 0x0404:
                case 0x0405:
                case 0x0407:
                    *id = ObjAmtId2;
                    break;
                case 0x0406:
                case 0x0408:
                case 0x0409:
                case 0x040a:
                    *id = ObjAmtId4;
                    break;
                default:
                    return RspUnknown;
                    break;
            }
        }
        else
        {
            *protGrp = ProtGrpStep1;
            switch (protId)
            {
                case 0x040b:
                case 0x0410:
                case 0x0413:
                    *id = ObjAmtId2;
                    break;
                case 0x040c:
                case 0x040d:
                case 0x040e:
                case 0x040f:
                case 0x0411:
                case 0x0412:
                    *id = ObjAmtId4;
                    break;
                default:
                    return RspUnknown;
                    break;
            }
        }
    }
    return RspOk;
}

/*托盘即全程保护id到参数群组的映射*/
u16eRspCode trayProtId2ObjAmtId(u16 protId, u8eObjAmtId *id)
{
    switch (protId)
    {
        case 0x0100:
        case 0x0101:
        case 0x0300:
        case 0x0303:
            *id = ObjAmtId2;
            break;
        case 0x0103:
        case 0x0201:
            *id = ObjAmtId4;
            break;
        case 0x0200:
        case 0x0204:
        case 0x0302:
            *id = ObjAmtId6;
            break;
        default:
            return RspUnknown; /*不能走到这里*/
    }
    return RspOk;
}

/*获取工步截止时间,不检查参数合法性,若需要则调用之前检查*/
u32 getStepEndTime(StepObj *step)
{
    if (StepTypeQuiet == step->stepType)
    {
        return step->stepParam[0];
    }
    else if (StepTypeCCC==step->stepType || StepTypeCCD==step->stepType)
    {
        return step->stepParam[1];
    }
    else
    {
        return step->stepParam[2];
    }
}

/*找到工步号对应的工步节点*/
StepNode *findStepNode(FlowStepEntry *flowStepEntry, u8 stepId)
{
    StepNode *stepNode;
    ChainS *chain;
    u8 idx;

    if (NULL==flowStepEntry || stepId>=flowStepEntry->stepAmt)
    {
        return NULL;
    }

    chain = flowStepEntry->stepList.next;
    for (idx=0; idx<stepId; idx++)
    {
        chain = chain->next;
    }

    stepNode = Container(StepNode, chain, chain);
    return stepNode;
}

/*查找base的下一个非循环工步*/
/*realSw:False--只查找而不跳,True--查找且跳*/
StepNode *getNxtStep(Channel *chn, StepNode *base, b8 realSw)
{
    ChainS *chain;
    StepNode *stepNode;
    StepObj *stepObj;
    ListS *stepList;
    LoopDscr *loopDscr;
    LoopDscr *loopDscrCri;

    stepList = &chn->flowStepEntry->stepList;
    for (chain=base->chain.next; chain!=stepList; chain=chain->next)
    {
        stepNode = Container(StepNode, chain, chain);
        stepObj = stepNode->stepObj;
        if (StepTypeLoop != stepObj->stepType)
        {
            return stepNode;
        }

        loopDscrCri = &chn->loopDscr[chn->flowStepEntry->loopStepAmt];
        for (loopDscr=chn->loopDscr; loopDscr<loopDscrCri; loopDscr++)
        {
            if (loopDscr->loopStepId == stepNode->stepId)
            {
                break;
            }
        }

        if (loopDscr == loopDscrCri)
        {
            return NULL;
        }

        if (0 != loopDscr->jumpAmt)
        {
            if (realSw)
            {
                loopDscr->jumpAmt--;
            }

            /*跳转的目标工步一定是非循环工步,直接返回*/
            return findStepNode(chn->flowStepEntry, loopDscr->jumpStepId);
        }
        else
        {
            if (realSw)
            {
                loopDscr->jumpAmt = stepObj->stepParam[1]; /*复原跳数*/
            }
        }
    }

    return NULL;
}

b8 flowProtCfgChkSame(FlowProtCfg *flowProtCfg, UpProtUnit *upProt)
{
    u8eChnProtPolicy policy;
    u32 sampleInterval;
    s32 reverseProtVol;

    policy = ((upProt->paramEnable&0x01) && 1==upProt->protParam[0]) ? ChnProtStop : ChnProtPause;
    sampleInterval = upProt->paramEnable&0x02 ? upProt->protParam[1] : 1000;
    reverseProtVol = upProt->paramEnable&0x04 ? upProt->protParam[2] : -80000000;

    if (policy == flowProtCfg->protPolicy
        && sampleInterval == flowProtCfg->sampleInterval
        && reverseProtVol == flowProtCfg->reverseProtVol)
    {
        return True;
    }

    return False;
}

FlowProtCfg *flowProtCfgNew(UpProtUnit *upProt)
{
    ChainD *chain;
    FlowProtCfg *flowProtCfg;

    /*先查重*/
    ListForEach(&gFlowCb->busyFlowProtCfgList, chain)
    {
        flowProtCfg = Container(FlowProtCfg, chain, chain);
        if (flowProtCfgChkSame(flowProtCfg, upProt))
        {
            return flowProtCfg;
        }
    }

    /*找不到就新建*/
    if (ListIsEmpty(&gFlowCb->idleProtCfgList))
    {
        return NULL;
    }

    chain = gFlowCb->idleProtCfgList.next;
    ChainDeleteD(chain);
    ChainInsertD(&gFlowCb->busyFlowProtCfgList, chain);
    flowProtCfg = Container(FlowProtCfg, chain, chain);
    flowProtCfg->refCnt = 0;
    flowProtCfg->protPolicy = upProt->paramEnable&0x01 ? upProt->protParam[0] : ChnProtPause;
    flowProtCfg->sampleInterval = upProt->paramEnable&0x02 ? upProt->protParam[1] : 1000;
    flowProtCfg->reverseProtVol = upProt->paramEnable&0x04 ? upProt->protParam[2] : -80000000;
    return flowProtCfg;
}

void flowProtCfgRel(FlowProtCfg *flowProtCfg)
{
    if (NULL == flowProtCfg)
    {
        return;
    }

    flowProtCfg->refCnt--;
    if (0 != flowProtCfg->refCnt)
    {
        return;
    }

    ChainDeleteD(&flowProtCfg->chain);
    ChainInsertD(&gFlowCb->idleProtCfgList, &flowProtCfg->chain);
    return;
}

b8 stepProtCfgChkSame(StepNpCfg *stepNpCfg, UpProtUnit *upProt)
{
    u8eNpType npType;
    s16 stepNpExpect;

    stepNpExpect = upProt->protParam[1];
    npType = upProt->protParam[0];
    if (NpTypeRatio==npType && 0==stepNpExpect)
    {
        npType = NpTypeNml; /*当常压处理*/
    }

    if (npType==stepNpCfg->npType && stepNpExpect==stepNpCfg->stepNpExpect
        && stepNpCfg->stepNpMax==upProt->protParam[2]
        && stepNpCfg->stepNpMin==upProt->protParam[3])
    {
        return True;
    }

    return False;
}

StepNpCfg *stepProtCfgNew(UpProtUnit *upProt)
{
    ChainD *chain;
    StepNpCfg *stepNpCfg;

    /*先查重*/
    ListForEach(&gFlowCb->busyStepProtCfgList, chain)
    {
        stepNpCfg = Container(StepNpCfg, chain, chain);
        if (stepProtCfgChkSame(stepNpCfg, upProt))
        {
            return stepNpCfg;
        }
    }

    /*找不到就新建*/
    if (ListIsEmpty(&gFlowCb->idleProtCfgList))
    {
        return NULL;
    }

    chain = gFlowCb->idleProtCfgList.next;
    ChainDeleteD(chain);
    ChainInsertD(&gFlowCb->busyStepProtCfgList, chain);
    stepNpCfg = Container(StepNpCfg, chain, chain);
    stepNpCfg->refCnt = 0;
    stepNpCfg->stepNpExpect = upProt->protParam[1];
    stepNpCfg->npType = upProt->protParam[0];
    if (NpTypeRatio==stepNpCfg->npType && 0==stepNpCfg->stepNpExpect)
    {
        stepNpCfg->npType = NpTypeNml; /*当常压处理*/
    }
    stepNpCfg->stepNpMax = upProt->protParam[2];
    stepNpCfg->stepNpMin = upProt->protParam[3];
    return stepNpCfg;
}

void stepProtCfgRel(StepNpCfg *stepNpCfg)
{
    if (NULL == stepNpCfg)
    {
        return;
    }

    stepNpCfg->refCnt--;
    if (0 != stepNpCfg->refCnt)
    {
        return;
    }

    ChainDeleteD(&stepNpCfg->chain);
    ChainInsertD(&gFlowCb->idleProtCfgList, &stepNpCfg->chain);
    return;
}

StepNode *stepNodeNew(u8 stepId)
{
    ChainS *chain;
    StepNode *stepNode;

    if (ListIsEmpty(&gFlowCb->idleStepNodeList))
    {
        return NULL;
    }

    chain = gFlowCb->idleStepNodeList.next;
    ChainDelSafeS(&gFlowCb->idleStepNodeList, chain);
    stepNode = Container(StepNode, chain, chain);
    stepNode->stepObj = NULL;
    stepNode->stepProtEntry = NULL;
    stepNode->stepNpCfg = NULL;
    stepNode->stepId = stepId;
    return stepNode;
}

ProtNode *protNodeNew()
{
    ChainS *chain;
    ProtNode *protNode;

    if (ListIsEmpty(&gFlowCb->idleProtNodeList))
    {
        return NULL;
    }

    chain = gFlowCb->idleProtNodeList.next;
    ChainDelSafeS(&gFlowCb->idleProtNodeList, chain);
    protNode = Container(ProtNode, chain, chain);
    protNode->protObj = NULL;
    return protNode;
}

b8 protObjChkSame(ProtObj *protObj, UpProtUnit *upProt)
{
    u8 bitIdx;
    u8 bitIdxCri;

    if (protObj->protId!=upProt->protId || protObj->paramEnable!=upProt->paramEnable)
    {
        return False;
    }

    bitIdxCri = gFlowCb->objParamAmt[protObj->objAmtId];
    for (bitIdx=0; bitIdx<bitIdxCri; bitIdx++)
    {
        if (BitIsSet(upProt->paramEnable, bitIdx))
        {
            if (protObj->protParam[bitIdx] != upProt->protParam[bitIdx])
            {
                return False;
            }
        }
    }

    return True;
}

ProtObj *protObjNew(UpProtUnit *upProt, u8eObjAmtId objAmtId, u8eProtGrp protGrp)
{
    ChainD *chain;
    ProtObj *protObj;
    u8 paramIdx;
    u8 paramIdxCri;

    /*先查重*/
    ListForEach(&gFlowCb->busyProtObjList[protGrp], chain)
    {
        protObj = Container(ProtObj, chain, chain);
        if (protObjChkSame(protObj, upProt))
        {
            return protObj;
        }
    }

    /*查不到则新建*/
    if (ListIsEmpty(&gFlowCb->idleObjList[objAmtId]))
    {
        return NULL;
    }

    chain = gFlowCb->idleObjList[objAmtId].next;
    ChainDelSafeD(chain);
    ChainInsertD(&gFlowCb->busyProtObjList[protGrp], chain);
    protObj = Container(ProtObj, chain, chain);
    protObj->refCnt = 0;
    protObj->objAmtId = objAmtId;
    protObj->paramEnable = upProt->paramEnable;
    protObj->protId = upProt->protId;
    paramIdxCri = gFlowCb->objParamAmt[objAmtId];
    for (paramIdx=0; paramIdx<paramIdxCri; paramIdx++)
    {
        protObj->protParam[paramIdx] = upProt->protParam[paramIdx];
    }
    return protObj;
}

void protObjRel(ProtObj *protObj)
{
    if (NULL == protObj)
    {
        return;
    }

    protObj->refCnt--;
    if (0 != protObj->refCnt)
    {
        return;
    }

    ChainDeleteD(&protObj->chain);
    ChainInsertD(&gFlowCb->idleObjList[protObj->objAmtId], &protObj->chain);
    return;
}

b8 stepObjChkSame(StepObj *stepObj, UpStepInfo *upStepInfo)
{
    u8 bitIdx;
    u8 bitIdxCri;

    if (stepObj->paramEnable != upStepInfo->paramEnable)
    {
        return False;
    }

    bitIdxCri = gFlowCb->objParamAmt[stepObj->objAmtId];
    for (bitIdx=0; bitIdx<bitIdxCri; bitIdx++)
    {
        if (BitIsSet(upStepInfo->paramEnable, bitIdx))
        {
            if (stepObj->stepParam[bitIdx] != upStepInfo->stepParam[bitIdx])
            {
                return False;
            }
        }
    }

    return True;
}

StepObj *stepObjNew(UpStepInfo *upStepInfo, u8eObjAmtId objAmtId)
{
    ChainD *chain;
    StepObj *stepObj;
    u8 paramIdx;
    u8 paramIdxCri;

    /*先查重*/
    ListForEach(&gFlowCb->busyStepObjList[upStepInfo->stepType], chain)
    {
        stepObj = Container(StepObj, chain, chain);
        if (stepObjChkSame(stepObj, upStepInfo))
        {
            return stepObj;
        }
    }

    /*查不到则新建*/
    if (ListIsEmpty(&gFlowCb->idleObjList[objAmtId]))
    {
        return NULL;
    }

    chain = gFlowCb->idleObjList[objAmtId].next;
    ChainDelSafeD(chain);
    ChainInsertD(&gFlowCb->busyStepObjList[upStepInfo->stepType], chain);
    stepObj = Container(StepObj, chain, chain);
    stepObj->refCnt = 0;
    stepObj->objAmtId = objAmtId;
    stepObj->paramEnable = upStepInfo->paramEnable;
    stepObj->stepType = upStepInfo->stepType;
    paramIdxCri = gFlowCb->objParamAmt[objAmtId];
    for (paramIdx=0; paramIdx<paramIdxCri; paramIdx++)
    {
        stepObj->stepParam[paramIdx] = upStepInfo->stepParam[paramIdx];
    }
    return stepObj;
}

void stepObjRel(StepObj * stepObj)
{
    if (NULL == stepObj)
    {
        return;
    }

    stepObj->refCnt--;
    if (0 != stepObj->refCnt)
    {
        return;
    }

    ChainDeleteD(&stepObj->chain);
    ChainInsertD(&gFlowCb->idleObjList[stepObj->objAmtId], &stepObj->chain);
    return;
}

/*创建托盘也即全程保护入口,与工步保护一样,可以合并*/
TrayProtEntry *trayProtEntryNew(u8eProtGrp protGrpId)
{
    ChainD *chain;
    TrayProtEntry *trayProtEntry;

    if (ListIsEmpty(&gFlowCb->idleEntryList))
    {
        return NULL;
    }

    chain = gFlowCb->idleEntryList.next;
    ChainDeleteD(chain);
    ChainInsertD(&gFlowCb->busyProtEntryList[protGrpId], chain);
    trayProtEntry = Container(TrayProtEntry, chain, chain);
    trayProtEntry->refCnt = 0;
    ListInitS(&trayProtEntry->protList);
    trayProtEntry->tail = &trayProtEntry->protList;
    return trayProtEntry;
}

/*释放托盘也即全程保护,与工步保护一样,可以合并*/
void trayProtEntryRel(TrayProtEntry *trayProtEntry)
{
    ChainS *chain;
    ChainS *tmp;
    ProtNode *protNode;

    if (NULL == trayProtEntry)
    {
        return;
    }

    trayProtEntry->refCnt--;
    if (0 != trayProtEntry->refCnt)
    {
        return;
    }

    /*释放所有保护节点*/
    ListForEachSafe(&trayProtEntry->protList, chain, tmp)
    {
        protNode = Container(ProtNode, chain, chain);
        protObjRel(protNode->protObj);  /*先释放节点所指实体*/
        ChainInsertS(&gFlowCb->idleProtNodeList, chain); /*再释放节点自身*/
    }

    /*释放入口自身*/
    ChainDeleteD(&trayProtEntry->chain);
    ChainInsertD(&gFlowCb->idleEntryList, &trayProtEntry->chain);
    return;
}

/*创建流程保护*/
FlowProtEntry *flowProtEntryNew(u8eProtGrp protGrpId)
{
    ChainD *chain;
    FlowProtEntry *flowProtEntry;

    if (ListIsEmpty(&gFlowCb->idleEntryList))
    {
        return NULL;
    }

    chain = gFlowCb->idleEntryList.next;
    ChainDeleteD(chain);
    ChainInsertD(&gFlowCb->busyProtEntryList[protGrpId], chain);
    flowProtEntry = Container(FlowProtEntry, chain, chain);
    flowProtEntry->refCnt = 0;
    flowProtEntry->flowProtCfg = NULL;
    ListInitS(&flowProtEntry->protList);
    flowProtEntry->tail = &flowProtEntry->protList;
    return flowProtEntry;
}

/*释放流程保护*/
void flowProtEntryRel(FlowProtEntry *flowProtEntry)
{
    ChainS *chain;
    ChainS *tmp;
    ProtNode *protNode;

    if (NULL == flowProtEntry)
    {
        return;
    }

    flowProtEntry->refCnt--;
    if (0 != flowProtEntry->refCnt)
    {
        return;
    }

    /*释放流程保护中的配置项*/
    flowProtCfgRel(flowProtEntry->flowProtCfg);

    /*释放所有保护节点*/
    ListForEachSafe(&flowProtEntry->protList, chain, tmp)
    {
        protNode = Container(ProtNode, chain, chain);
        protObjRel(protNode->protObj);  /*先释放节点所指实体*/
        ChainInsertS(&gFlowCb->idleProtNodeList, chain); /*再释放节点自身*/
    }

    /*释放入口自身*/
    ChainDeleteD(&flowProtEntry->chain);
    ChainInsertD(&gFlowCb->idleEntryList, &flowProtEntry->chain);
    return;
}

/*创建工步保护入口*/
StepProtEntry *stepProtEntryNew(u8eProtGrp protGrpId)
{
    ChainD *chain;
    StepProtEntry *stepProtEntry;

    if (ListIsEmpty(&gFlowCb->idleEntryList))
    {
        return NULL;
    }

    chain = gFlowCb->idleEntryList.next;
    ChainDeleteD(chain);
    ChainInsertD(&gFlowCb->busyProtEntryList[protGrpId], chain);
    stepProtEntry = Container(StepProtEntry, chain, chain);
    stepProtEntry->refCnt = 0;
    ListInitS(&stepProtEntry->protList);
    stepProtEntry->tail = &stepProtEntry->protList;
    return stepProtEntry;
}

/*释放工步保护*/
void stepProtEntryRel(StepProtEntry *stepProtEntry)
{
    ChainS *chain;
    ChainS *tmp;
    ProtNode *protNode;

    if (NULL == stepProtEntry)
    {
        return;
    }

    stepProtEntry->refCnt--;
    if (0 != stepProtEntry->refCnt)
    {
        return;
    }

    /*释放所有保护节点*/
    ListForEachSafe(&stepProtEntry->protList, chain, tmp)
    {
        protNode = Container(ProtNode, chain, chain);
        protObjRel(protNode->protObj);  /*先释放节点所指实体*/
        ChainInsertS(&gFlowCb->idleProtNodeList, chain); /*再释放节点自身*/
    }

    /*释放入口自身*/
    ChainDeleteD(&stepProtEntry->chain);
    ChainInsertD(&gFlowCb->idleEntryList, &stepProtEntry->chain);
    return;
}

/*建立流程*/
FlowStepEntry *flowStepEntryNew()
{
    ChainD *chain;
    FlowStepEntry *flowStepEntry;

    if (ListIsEmpty(&gFlowCb->idleEntryList))
    {
        return NULL;
    }

    chain = gFlowCb->idleEntryList.next;
    ChainDeleteD(chain);
    ChainInsertD(&gFlowCb->busyStepEntryList, chain);
    flowStepEntry = Container(FlowStepEntry, chain, chain);
    flowStepEntry->refCnt = 0;
    flowStepEntry->stepAmt = 0;
    flowStepEntry->loopStepAmt = 0;
    ListInitS(&flowStepEntry->stepList);
    flowStepEntry->tail = &flowStepEntry->stepList;
    return flowStepEntry;
}

/*释放流程,包括流程中的工步以及工步的工步保护*/
void flowStepEntryRel(FlowStepEntry *flowStepEntry)
{
    ChainS *chain;
    ChainS *tmp;
    StepNode *stepNode;

    if (NULL == flowStepEntry)
    {
        return;
    }

    flowStepEntry->refCnt--;
    if (0 != flowStepEntry->refCnt)
    {
        return;
    }

    /*释放所有工步节点*/
    ListForEachSafe(&flowStepEntry->stepList, chain, tmp)
    {
        stepNode = Container(StepNode, chain, chain);
        stepProtEntryRel(stepNode->stepProtEntry); /*释放工步保护*/
        stepProtCfgRel(stepNode->stepNpCfg); /*释放工步配置,目前只有负压*/
        stepObjRel(stepNode->stepObj);  /*释放工步实体*/
        ChainInsertS(&gFlowCb->idleStepNodeList, chain); /*最后释放节点自身*/
    }

    /*释放入口自身*/
    ChainDeleteD(&flowStepEntry->chain);
    ChainInsertD(&gFlowCb->idleEntryList, &flowStepEntry->chain);
    return;
}

/*清除通道的流程，含工步和工步保护和流程保护*/
void chnFlowRel(Channel *chn)
{
    flowProtEntryRel(chn->flowProtEntry);
    chn->flowProtEntry = NULL;

    flowStepEntryRel(chn->flowStepEntry);
    chn->flowStepEntry = NULL;

    chn->crntStepNode = NULL;
    return;
}

/*释放通道的工步保护和流程保护,含工步负压配置*/
void chnProtRel(Channel *chn)
{
    StepNode *stepNode;
    ChainS *chain;

    if (NULL != chn->flowStepEntry)
    {
        ListForEach(&chn->flowStepEntry->stepList, chain)
        {
            stepNode = Container(StepNode, chain, chain);
        
            stepProtEntryRel(stepNode->stepProtEntry);
            stepNode->stepProtEntry = NULL;
        
            stepProtCfgRel(stepNode->stepNpCfg);
            stepNode->stepNpCfg = NULL;
        }
    }

    flowProtEntryRel(chn->flowProtEntry);
    chn->flowProtEntry = NULL;
    return;
}

/*todo,现在这些存储的函数都比较大,可以再扇出,例如区分全盘与否*/
/*与上位机约束先流程后全局工步保护,收到工步首包后全清*/
u16eRspCode upFlowStepSave(u8    *pld)
{
    UpStepInfo *step;
    FlowRecvCtrl *ctrl;
    UpFlowCmd *cmd;
    Tray *tray;
    Channel *chn;
    Channel *chnCri;
    FlowStepEntry *flowStepEntry;
    UpStepInfo *upStepInfo;
    StepNode *stepNode;
    StepObj *stepObj;
    u16 idx;
    u16 genChnIdx;
    u16eRspCode ret;
    u8eObjAmtId objAmtId;

    ret = RspOk;
    cmd = (UpFlowCmd *)pld;
    ctrl = recvCtrlGet(cmd->trayIdx);
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (cmd->stepAmt < cmd->stepAmtTtl)
        {
            if (NULL == (ctrl=recvCtrlSetup(cmd->trayIdx, cmd->chnAmt)))
            {
                return RspBusy;
            }

            /*记录下来，用作下次收到时检查*/
            ctrl->recvMsgId = UpCmdIdManuFlow;
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
            recvCtrlFree(cmd->trayIdx);
        }
    }

    tray = &gDevMgr->tray[cmd->trayIdx];

    /*由于工步或保护都有可能分包传输,所以收到单个报文时无法做完整查重*/
    /*所以目前的策略是,首包时先清除,再生成,收到最后报文时查重归并*/
    /*策略不是最优逻辑,主要受分包传输机制所限*/

    /*首个工步报文时*/
    if (0 == cmd->stepSeq)
    {
        /*清除原有流程和工步保护和流程保护*/
        if (0 == cmd->chnAmt)  /*整盘*/
        {
            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                chnFlowRel(chn);
            }
        }
        else
        {
            for (idx=0; idx<cmd->chnAmt; idx++)
            {
                chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
                chnFlowRel(chn);
            }
        }

        /*再申请创建新流程*/
        flowStepEntry = flowStepEntryNew();
        if (NULL == flowStepEntry)
        {
            ret = RspNoRes;
            goto endHandler;
        }
        if (0 == cmd->chnAmt)  /*整盘*/
        {
            flowStepEntry->refCnt = tray->trayChnAmt;
            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                chn->flowStepEntry = flowStepEntry;
            }
        }
        else
        {
            flowStepEntry->refCnt = cmd->chnAmt;
            for (idx=0; idx<cmd->chnAmt; idx++)
            {
                chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
                chn->flowStepEntry = flowStepEntry;
            }
        }
    }
    else  /*不是首包,就找到首包建立的流程,哪个通道的都行*/
    {
        if (0 == cmd->chnAmt)  /*整盘*/
        {
            flowStepEntry = tray->chn->flowStepEntry;
        }
        else
        {
            chn = upChnMap(tray, cmd->chnId[0], &genChnIdx);
            flowStepEntry = chn->flowStepEntry;
        }
    }

    /*存储流程中的工步*/
    upStepInfo = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,upStepInfo++)
    {
        /*先创建节点*/
        stepNode = stepNodeNew(upStepInfo->stepId);
        if (NULL == stepNode)
        {
            ret = RspNoRes;
            goto endHandler;
        }
        ChainInsertS(flowStepEntry->tail, &stepNode->chain);
        flowStepEntry->tail = &stepNode->chain;

        /*再创建实体*/
        ret = stepType2ObjAmtId(upStepInfo->stepType, &objAmtId);
        if (RspOk != ret)
        {
            goto endHandler;
        }

        stepNode->stepObj = stepObjNew(upStepInfo, objAmtId);
        if (NULL == stepNode->stepObj)
        {
            ret = RspNoRes;
            goto endHandler;
        }
        stepNode->stepObj->refCnt++;
        flowStepEntry->stepAmt++;
    }

    /*收到最后一包时,搜集循环信息,查重归并.后者在工步保护中*/
    if (cmd->stepSeq+cmd->stepAmt == cmd->stepAmtTtl)
    {
        StepNode *stepNodeTmp;
        StepObj *stepObjTmp;
        ChainS *chainTmp;
        LoopDscr *loopDscr;
        LoopDscr *loopDscrPrev;
        u8 stepId;
        u8 loopStepAmt;
        u8 loopStepIdx;
        u8 loopStepIdxPrev;

        /*搜集循环工步信息*/
        stepId = loopStepAmt = 0;
        ListForEach(&flowStepEntry->stepList, chainTmp)
        {
            stepNodeTmp = Container(StepNode, chain, chainTmp);
            stepObjTmp = stepNodeTmp->stepObj;
            if (StepTypeLoop == stepObjTmp->stepType)
            {
                if (LoopAmtMax == loopStepAmt)
                {
                    ret = RspLoopAmt;
                    goto endHandler;
                }

                if (0 == cmd->chnAmt)  /*整盘*/
                {
                    for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
                    {
                        chn->loopDscr[loopStepAmt].loopStepId = stepId;
                        chn->loopDscr[loopStepAmt].jumpStepId = stepObjTmp->stepParam[0];
                        chn->loopDscr[loopStepAmt].jumpAmt = stepObjTmp->stepParam[1];
                    }
                }
                else
                {
                    for (idx=0; idx<cmd->chnAmt; idx++)
                    {
                        chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
                        chn->loopDscr[loopStepAmt].loopStepId = stepId;
                        chn->loopDscr[loopStepAmt].jumpStepId = stepObjTmp->stepParam[0];
                        chn->loopDscr[loopStepAmt].jumpAmt = stepObjTmp->stepParam[1];
                    }
                }
                loopStepAmt++;
            }
            stepId++;
        }

        /*搜集完毕后查错,todo,可以扇出*/
        /*只能往前跳到非循环工步,允许嵌套但不允许交叉*/
        if (0 == cmd->chnAmt)  /*整盘*/
        {
            chn = tray->chn;
        }
        else
        {
            chn = upChnMap(tray, cmd->chnId[0], &genChnIdx);
        }
        for (loopStepIdx=0; loopStepIdx<loopStepAmt; loopStepIdx++)
        {
            loopDscr = &chn->loopDscr[loopStepIdx];
            if (loopDscr->jumpStepId >= loopDscr->loopStepId) /*只能往前跳*/
            {
                ret = RspLoopParam;
                goto endHandler;
            }

            for (loopStepIdxPrev=0; loopStepIdxPrev<loopStepIdx; loopStepIdxPrev++)
            {
                loopDscrPrev = &chn->loopDscr[loopStepIdxPrev];
                if (loopDscr->jumpStepId == loopDscrPrev->loopStepId) /*只能跳到非循环*/
                {
                    ret = RspLoopParam;
                    goto endHandler;
                }
                if (loopDscr->jumpStepId > loopDscrPrev->jumpStepId  /*不允许嵌套*/
                    && loopDscr->jumpStepId < loopDscrPrev->loopStepId)
                {
                    ret = RspLoopParam;
                    goto endHandler;
                }
            }
        }

        /*无误后设置工步流程的循环工步数量*/
        flowStepEntry->loopStepAmt = loopStepAmt;
    }

endHandler:
    if (RspOk != ret)
    {
        recvCtrlFree(cmd->trayIdx);
        if (0 == cmd->chnAmt)  /*整盘*/
        {
            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                chnFlowRel(chn);
            }
        }
        else
        {
            for (idx=0; idx<cmd->chnAmt; idx++)
            {
                chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
                chnFlowRel(chn);
            }
        }
    }
    return ret;
}

/*清除托盘保护单项,负压时间,因子生命周期*/
void trayProtRelNml(Tray *tray)
{
    memset(&tray->npMgr, 0, sizeof(TrayNpMgr));
    memset(tray->mixProtCfg.lifePeriodSec, 0, sizeof(u32)*MixSubCri);
    trayProtEntryRel(tray->trayProtEntry);
    tray->trayProtEntry = NULL;
    return;
}
/*清除托盘组合保护*/
void trayProtRelMix(Tray *tray)
{
    tray->mixProtCfg.mixExpAmt = 0;
    tray->mixProtCfg.suffixOfst = 0;
    tray->mixProtCfg.policyOfst = 0;
    return;
}

/*清除托盘保护,含保护单项,负压时间,因子生命周期,组合保护*/
void trayProtRel(Tray *tray)
{
    trayProtRelNml(tray);
    trayProtRelMix(tray);
    return;
}

Ret policyParse(u8 *exp, u8 expLen, MixPolicy *policy, MixPolicy *policyCri, u8 *policyAmt)
{
    void *handle;
    void *subHdl;
    s8 *part;
    u32 numb;
    Ret ret;
    u8 tmpRcd;
    u8 amt;

    *policyAmt = 0;
    if (0 == expLen)
    {
        return Ok;
    }

    ret = Nok;
    tmpRcd = exp[expLen];  /*临时改变返回时复原*/
    exp[expLen] = 0;
    handle = subHdl = NULL;
    if (NULL == (handle=strTokSetup(exp, ',')))
    {
        goto endHandler;
    }
    for (amt=0; NULL!=(part=strTokGet(handle)); )
    {
        if ('=' == *part || NULL == (subHdl=strTokSetup(part, '='))
            || NULL==(part=strTokGet(subHdl)) || Ok!=strToHex(part, &numb)
            || numb>=PolicyCri || policy>=policyCri)
        {
            goto endHandler;
        }

        policy->policyId = numb;
        policy->delaySec = 0;
        if (NULL != (part=strTokGet(subHdl)))
        {
            if (Ok != strToHex(part, &numb))
            {
                goto endHandler;
            }

            policy->delaySec = numb/1000;
            if (numb%1000)
            {
                policy->delaySec += 1;
            }
        }

        strTokRel(subHdl);
        subHdl = NULL;

        /*过滤掉不能执行的动作*/
        if (PolicyFmsNtfy!=policy->policyId && PolicyGasEnable!=policy->policyId
            && PolicyItfDisplay != policy->policyId)
        {
            amt++;
            policy++;
        }
    }

    *policyAmt = amt;
    ret = Ok;
endHandler:
    if (NULL != handle) strTokRel(handle);
    if (NULL != subHdl) strTokRel(subHdl);
    exp[expLen] = tmpRcd;
    return ret;
}

/*组合保护解析,含条件和策略*/
/*策略可以为空,但不能没有@字符*/
Ret protExpParse(u8 *exp, u16 expLen, MixProtCfg *mixProtCfg)
{
    u8 *pos;
    u8 *cri;
    u8 *suffix;
    MixExp *prot;
    MixPolicy *policy;
    MixPolicy *policyCri;
    u32 tmp;  /*仅用于验证,不需赋值*/

    for (pos=exp,cri=exp+expLen; pos<cri && '@'!=*pos; pos++);

    if ('@' != *pos || mixProtCfg->mixExpAmt >= MixExpAmt)
    {
        return Nok;
    }

    prot = &mixProtCfg->mixExp[mixProtCfg->mixExpAmt];
    suffix = mixProtCfg->suffix + mixProtCfg->suffixOfst;
    policy = (MixPolicy *)(mixProtCfg->policy + mixProtCfg->policyOfst);
    policyCri = (MixPolicy *)(mixProtCfg->policy+TtlPolicySize);
    if (Ok != expInfix2Suffix(exp, pos, MixSubCri, suffix, mixProtCfg->suffix+TtlExpSize, &prot->suffixLen)
        || Ok != expSuffixCalc(suffix, prot->suffixLen, tmp, &tmp)
        || Ok != policyParse(pos+1, expLen-(pos-exp)-1, policy, policyCri, &prot->policyAmt))
    {
        return Nok;
    }

    prot->suffixOfst = mixProtCfg->suffixOfst;
    mixProtCfg->suffixOfst += prot->suffixLen;
    prot->policyOfst = mixProtCfg->policyOfst;
    mixProtCfg->policyOfst += prot->policyAmt * sizeof(MixPolicy);
    mixProtCfg->mixExpAmt++;
    return Ok;
}

/*约束:含组合保护的必是第一包,清除原有保护*/
u16eRspCode protExpSave(u8 *cmdPld)
{
    UpProtGenCmd *cmd;
    UpMixProt *mixProt;
    Tray *tray;
    u8 *pos;
    u8 idx;

    cmd = (UpProtGenCmd *)cmdPld;
    if (0 == cmd->mixProtAmt)
    {
        return RspOk;
    }

    tray = &gDevMgr->tray[cmd->trayIdx];
    trayProtRelMix(tray);
    for (idx=0,pos=cmdPld+sizeof(UpProtGenCmd); idx<cmd->mixProtAmt; idx++)
    {
        mixProt = (UpMixProt *)pos;
        if (Ok != protExpParse(mixProt->mixProtAby, mixProt->mixProtLen, &tray->mixProtCfg))
        {
            return RspParam;
        }
        pos += sizeof(UpMixProt) + Align8(mixProt->mixProtLen);
    }

    return RspOk;
}

u16eRspCode upTrayProtSave(u8 *pld)
{
    FlowRecvCtrl *ctrl;
    UpProtGenCmd *cmd;
    Tray *tray;
    TrayProtEntry *trayProtEntry;
    u8 *pos;
    UpMixProt *mixProt;
    UpProtUnit *upProt;
    ProtNode *protNode;
    u16 idx;
    u16eRspCode ret;
    u8eObjAmtId objAmtId;

    ret = RspOk;
    cmd = (UpProtGenCmd *)pld;
    tray = &gDevMgr->tray[cmd->trayIdx];
    ctrl = recvCtrlGet(cmd->trayIdx);
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (cmd->protAmt < cmd->protAmtTtl)
        {
            if (NULL == (ctrl=recvCtrlSetup(cmd->trayIdx, 0)))
            {
                return RspBusy;
            }

            /*记录下来，用作下次收到时检查*/
            ctrl->recvMsgId = UpCmdIdManuProtGen;
            ctrl->total = cmd->protAmtTtl;
            ctrl->offsetHope = cmd->protAmt;
        }
        trayProtRel(tray);  /*清除原来*/
    }
    else
    {
        ctrl->offsetHope += cmd->protAmt;
        if (ctrl->offsetHope == ctrl->total)  /*收完了*/
        {
            recvCtrlFree(cmd->trayIdx);
        }
    }

    /*解析组合保护的条件和策略*/
    if (RspOk != (ret=protExpSave(pld)))
    {
        goto endHandler;
    }

    if (0 == cmd->protSeq)  /*首包,清除之前保护(不含组合),并重新建立*/
    {
        trayProtRelNml(tray);
        tray->trayProtEntry = trayProtEntryNew(ProtGrpTray);
        if (NULL == tray->trayProtEntry)
        {
            ret = RspNoRes;
            goto endHandler;
        }
        tray->trayProtEntry->refCnt++;
    }

    /*配置和组合保护在其它处理,暂时不动,以后可以放这里*/
    /*跳过组合保护表达式,找到非组合保护位置*/
    pos = pld + sizeof(UpProtGenCmd);
    for (idx=0; idx<cmd->mixProtAmt; idx++)
    {
        mixProt = (UpMixProt *)pos;
        pos += sizeof(UpMixProt) + Align8(mixProt->mixProtLen);
    }
    upProt = (UpProtUnit *)pos;

    /*非组合保护*/
    for (idx=0; idx<cmd->protAmt; idx++,upProt++)
    {
        /*小于0x0100的都是配置:负压时间和生命周期*/
        if (0x0000 == upProt->protId) /*负压相关时间*/
        {
            TrayNpMgr *npMgr;

            npMgr = &tray->npMgr;
            npMgr->mkNpExprSecT00 = upProt->protParam[0]/1000;
            npMgr->brkNpExprSecT10 = upProt->protParam[1]/1000;
            npMgr->npLmtExprSecT06 = upProt->protParam[3]/1000;
            npMgr->ndbdBrkWaitNpSecT07 = upProt->protParam[4]/1000;
            npMgr->closeBrkDelaySecT08 = upProt->protParam[6]/1000;
            continue;
        }
        else if (upProt->protId < 0x0005)  /*因子生命周期*/
        {
            u8 arrayIdx;

            for (arrayIdx=0; arrayIdx<8; arrayIdx++)
            {
                if (BitIsSet(upProt->paramEnable, arrayIdx))
                {
                    tray->mixProtCfg.lifePeriodSec[arrayIdx+(upProt->protId-1)*8] = upProt->protParam[arrayIdx]/1000;
                }
            }
            continue;
        }

        /*其余保护id不是配置*/
        ret = trayProtId2ObjAmtId(upProt->protId, &objAmtId);
        if (RspOk != ret)
        {
            goto endHandler;
        }

        /*先创建保护节点*/
        protNode = protNodeNew();
        if (NULL == protNode)
        {
            ret = RspNoRes;
            goto endHandler;
        }
        ChainInsertS(tray->trayProtEntry->tail, &protNode->chain);
        tray->trayProtEntry->tail = &protNode->chain;

        /*再创建保护实体*/
        protNode->protObj = protObjNew(upProt, objAmtId, ProtGrpTray);
        if (NULL == protNode->protObj)
        {
            ret = RspNoRes;
            goto endHandler;
        }
        protNode->protObj->refCnt++;
    }

    /*最后一条全程保护消息后查重归并*/
    if (cmd->protSeq+cmd->protAmt == cmd->protAmtTtl)
    {
        /*todo,查重归并暂时也可不做,但以后一定要做*/
    }

endHandler:
    if (RspOk != ret)
    {
        recvCtrlFree(cmd->trayIdx);
        trayProtRel(tray);
    }
    return ret;
}

/*拷贝通道的工步保护到另一个通道,含负压配置*/
void chnStepProtCopy(Channel *chnSrc, Channel *chnDst)
{
    StepNode *stepNodeSrc;
    StepNode *stepNodeDst;
    ListS *stepList;
    ChainS *chain;
    ListS *stepListDst;
    ChainS *chainDst;

    if (ListIsEmpty(&chnDst->flowStepEntry->stepList))
    {
        return;
    }

    stepListDst = &chnDst->flowStepEntry->stepList;
    chainDst = stepListDst->next;
    stepNodeDst = Container(StepNode, chain, chainDst);

    stepList = &chnSrc->flowStepEntry->stepList;
    ListForEach(stepList, chain)
    {
        stepNodeSrc = Container(StepNode, chain, chain);
        if (NULL != stepNodeSrc->stepNpCfg)
        {
            stepNodeDst->stepNpCfg = stepNodeSrc->stepNpCfg;
            stepNodeSrc->stepNpCfg->refCnt++;
        }
        if (NULL != stepNodeSrc->stepProtEntry)
        {
            stepNodeDst->stepProtEntry = stepNodeSrc->stepProtEntry;
            stepNodeSrc->stepProtEntry->refCnt++;
        }

        chainDst = chainDst->next;
        if (chainDst == stepListDst)
        {
            return;
        }
        stepNodeDst = Container(StepNode, chain, chainDst);
    }
}

u16eRspCode upStepProtSave(u8 *pld)
{
    FlowRecvCtrl *ctrl;
    UpProtStepCmd *cmd;
    UpProtUnit *upProt;
    Tray *tray;
    Channel *chn;
    Channel *chnCri;
    Channel *fstChn;  /*保护指定的通道中,第一个有流程的*/
    FlowProtEntry *flowProtEntry;
    FlowStepEntry *flowStepEntry;
    StepNode *stepNode;
    ProtNode *protNode;
    u16eRspCode ret;
    u16 idx;
    u16 genChnIdx;
    u8 crntStepIdTag;
    u8eObjAmtId objAmtId;
    u8eProtGrp protGrp;

    ret = RspOk;
    cmd = (UpProtStepCmd *)pld;
    ctrl = recvCtrlGet(cmd->trayIdx);
    if (NULL == ctrl)  /*必是单包或多包之首包*/
    {
        if (cmd->protAmt < cmd->protAmtTtl)
        {
            if (NULL == (ctrl=recvCtrlSetup(cmd->trayIdx, cmd->chnAmt)))
            {
                return RspBusy;
            }

            /*记录下来，用作下次收到时检查*/
            ctrl->recvMsgId = UpCmdIdManuProtStep;
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
            recvCtrlFree(cmd->trayIdx);
        }
    }

    tray = &gDevMgr->tray[cmd->trayIdx];

    /*由于定容的特殊性(参见本函数最后的查重归并逻辑中的描述),*/
    /*可能出现流程对应的通道与保护对应的通道不一致的情况*/
    /*所以,虽然收到流程时候释放了相关保护,但这里还需要再释放*/
    /*并且,保护的载体也需要找到有流程的通道*/

    /*先找到存放保护的载体,也即第一个有流程的通道*/
    flowStepEntry = NULL;
    if (0 == cmd->chnAmt)  /*整盘*/
    {
        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (NULL != chn->flowStepEntry)
            {
                flowStepEntry = chn->flowStepEntry;
                fstChn = chn;
                break;
            }
        }
    }
    else
    {
        for (idx=0; idx<cmd->chnAmt; idx++)
        {
            chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
            if (NULL != chn->flowStepEntry)
            {
                flowStepEntry = chn->flowStepEntry;
                fstChn = chn;
                break;
            }
        }
    }

    if (NULL == flowStepEntry)  /*找不到载体就报错*/
    {
        ret = RspMissFlow;
        goto endHandler;
    }

    /*工步保护和流程保护*/
    if (0 == cmd->protSeq)  /*首包*/
    {
        /*首包就创建新的流程保护*/
        flowProtEntry = flowProtEntryNew(ProtGrpFlow);
        if (NULL == flowProtEntry)
        {
            ret = RspNoRes;
            goto endHandler;
        }

        if (0 == cmd->chnAmt)  /*整盘*/
        {
            flowProtEntry->refCnt = tray->trayChnAmt;
            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                /*先释放早先的保护,再建立*/
                chnProtRel(chn);
                chn->flowProtEntry = flowProtEntry;
            }
        }
        else
        {
            flowProtEntry->refCnt = cmd->chnAmt;
            for (idx=0; idx<cmd->chnAmt; idx++)
            {
                chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);

                /*先释放早先的保护,再建立*/
                chnProtRel(chn);
                chn->flowProtEntry = flowProtEntry;
            }
        }
    }
    else  /*不是首包,就找到首包建立的流程保护,哪个通道的都行*/
    {
        if (0 == cmd->chnAmt)  /*整盘*/
        {
            flowProtEntry = tray->chn->flowProtEntry;
        }
        else
        {
            chn = upChnMap(tray, cmd->chnId[0], &genChnIdx);
            flowProtEntry = chn->flowProtEntry;
        }
    }
    
    /*存储消息中的保护*/
    crntStepIdTag = 255;  /*无效工步id*/
    stepNode = NULL;
    upProt = (UpProtUnit *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->protAmt; idx++,upProt++)
    {
        /*通过保护id来区分真保护还是配置,二者存储不同*/
        if (0x0400 == upProt->protId) /*各工步的负压配置*/
        {
            if (crntStepIdTag != upProt->stepId)
            {
                stepNode = findStepNode(flowStepEntry, upProt->stepId);
                crntStepIdTag = upProt->stepId;
            }
            if (NULL == stepNode)
            {
                ret = RspMissFlow;
                goto endHandler;
            }
            if (NULL != stepNode->stepNpCfg)
            {
                ret = RspRepeatParam;
                goto endHandler;
            }

            stepNode->stepNpCfg = stepProtCfgNew(upProt);
            if (NULL == stepNode->stepNpCfg)
            {
                ret = RspNoRes;
                goto endHandler;
            }

            stepNode->stepNpCfg->refCnt++;
            continue;
        }
        else if (0x0500 == upProt->protId) /*流程的反接/采样间隔/处理策略配置*/
        {
            if (NULL != flowProtEntry->flowProtCfg)
            {
                ret = RspRepeatParam;
                goto endHandler;
            }

            flowProtEntry->flowProtCfg = flowProtCfgNew(upProt);
            if (NULL == flowProtEntry->flowProtCfg)
            {
                ret = RspNoRes;
                goto endHandler;
            }
            
            flowProtEntry->flowProtCfg->refCnt++;
            continue;
        }

        /*其余保护id是真保护而不是配置*/
        ret = stepProtId2ObjAmtId(upProt->protId, &objAmtId, &protGrp);
        if (RspOk != ret)
        {
            goto endHandler;
        }

        /*若是工步保护,需要确定保护所在链表母体,为工步创建保护入口*/
        if (upProt->protId < 0x0500)  /*工步保护*/
        {
            /*找到工步节点*/
            if (crntStepIdTag != upProt->stepId)
            {
                stepNode = findStepNode(flowStepEntry, upProt->stepId);
                crntStepIdTag = upProt->stepId;
            }
            if (NULL==stepNode)
            {
                ret = RspMissFlow;
                goto endHandler;
            }

            if (NULL == stepNode->stepProtEntry) /*给单工步建立工步保护入口*/
            {
                stepNode->stepProtEntry = stepProtEntryNew(protGrp);
                if (NULL == stepNode->stepProtEntry)
                {
                    ret = RspNoRes;
                    goto endHandler;
                }
                stepNode->stepProtEntry->refCnt++;
            }
        }

        /*先创建保护节点*/
        protNode = protNodeNew();
        if (NULL == protNode)
        {
            ret = RspNoRes;
            goto endHandler;
        }
        if (upProt->protId < 0x0500)  /*工步保护*/
        {
            ChainInsertS(stepNode->stepProtEntry->tail, &protNode->chain);
            stepNode->stepProtEntry->tail = &protNode->chain;
        }
        else  /*流程保护*/
        {
            ChainInsertS(flowProtEntry->tail, &protNode->chain);
            flowProtEntry->tail = &protNode->chain;
        }

        /*再创建保护实体*/
        protNode->protObj = protObjNew(upProt, objAmtId, protGrp);
        if (NULL == protNode->protObj)
        {
            ret = RspNoRes;
            goto endHandler;
        }
        protNode->protObj->refCnt++;
    }

    /*收到最后一条工步保护消息时,查错,查重归并*/
    if (cmd->protSeq+cmd->protAmt == cmd->protAmtTtl)
    {
        /*查错,目前只检查配置缺失*/
        if (NULL == flowProtEntry->flowProtCfg)
        {
            ret = RspMissInfo;
            goto endHandler;
        }

        /*1.正常情况下,流程与保护一对一映射*/
        /*2.定容特殊,特征是,多个流程相比,除时间截止参数之外全部相同*/
        /*3.满足该特征的情况下(如定容),允许多个流程对应一个保护,好处是节约保护交互次数*/
        /*4.以下代码兼容该特殊情况(如定容)*/
        /*5.但是,中位机做这种兼容就无法报告部分错误*/
        /*6.所以,上位机必须保证,除定容之外,流程与保护必须一对一映射*/
        /*7.若上位机无法保证6,那么要严格遵循流程与保护一对一映射*/
        /*8.若7,则以下这些代码删除*/
        if (0 == cmd->chnAmt)  /*整盘*/
        {
            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                if (chn==fstChn || NULL==chn->flowStepEntry || fstChn->flowStepEntry==chn->flowStepEntry)
                {
                    continue;
                }

                chnStepProtCopy(fstChn, chn);
            }
        }
        else
        {
            for (idx=0; idx<cmd->chnAmt; idx++)
            {
                chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
                if (chn==fstChn || NULL==chn->flowStepEntry || fstChn->flowStepEntry==chn->flowStepEntry)
                {
                    continue;
                }

                chnStepProtCopy(fstChn, chn);
            }
        }
        /*9.涉及兼容定容的代码到此为止*/

        /*todo,查重归并暂时也可不做,但以后一定要做*/
    }
    
endHandler:
    if (RspOk != ret)
    {
        recvCtrlFree(cmd->trayIdx);
        if (0 == cmd->chnAmt)  /*整盘*/
        {
            for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                chnProtRel(chn);
            }
        }
        else
        {
            for (idx=0; idx<cmd->chnAmt; idx++)
            {
                chn = upChnMap(tray, cmd->chnId[idx], &genChnIdx);
                chnProtRel(chn);
            }
        }
    }
    return ret;
}


Ret flowInit(u8 trayAmt)
{
    FlowCb *cb;
    u8 *tmpMem;
    u8 stepIdx;
    u8 trayIdx;
    u16 objIdx;
    u8 listIdx;

    if (NULL == (cb = gFlowCb = sysMemAlloc(sizeof(FlowCb))))
    {
        return Nok;
    }

    for (trayIdx=0; trayIdx<trayAmt; trayIdx++)
    {
        gFlowCb->recvCtrl[trayIdx] = NULL;
    }

    gFlowCb->flowObjAmt[ObjAmtId2] = FlowObjAmt2;
    gFlowCb->flowObjAmt[ObjAmtId4] = FlowObjAmt4;
    gFlowCb->flowObjAmt[ObjAmtId6] = FlowObjAmt6;
    gFlowCb->flowObjAmt[ObjAmtId8] = FlowObjAmt8;
    gFlowCb->objParamAmt[ObjAmtId2] = 2;
    gFlowCb->objParamAmt[ObjAmtId4] = 4;
    gFlowCb->objParamAmt[ObjAmtId6] = 6;
    gFlowCb->objParamAmt[ObjAmtId8] = 8;
    for (listIdx=0; listIdx<ObjAmtIdCri; listIdx++)  /*工步/保护实体*/
    {
        StepObj *flowObj;  /*StepObj和ProtObj都包括*/
        u8 objSize;

        ListInitD(&gFlowCb->idleObjList[listIdx]);
        objSize = sizeof(FlowObj) + gFlowCb->objParamAmt[listIdx]*sizeof(u32);
        tmpMem = sysMemAlloc(objSize*gFlowCb->flowObjAmt[listIdx]);
        for (objIdx=0; objIdx<gFlowCb->flowObjAmt[listIdx]; objIdx++)
        {
            flowObj = (StepObj *)(tmpMem + objIdx*objSize);
            ChainInsertD(&gFlowCb->idleObjList[listIdx], &flowObj->chain);
        }
    }
    for (listIdx=0; listIdx<StepTypeCri; listIdx++)
    {
        ListInitD(&gFlowCb->busyStepObjList[listIdx]);
    }
    for (listIdx=0; listIdx<ProtGrpCri; listIdx++)
    {
        ListInitD(&gFlowCb->busyProtObjList[listIdx]);
    }

    {  /*工步流程节点,单链*/
        StepNode *stepNode;

        ListInitS(&gFlowCb->idleStepNodeList);
        tmpMem = sysMemAlloc(StepNodeAmt*sizeof(StepNode));
        for (objIdx=0; objIdx<StepNodeAmt; objIdx++)
        {
            stepNode = (StepNode *)(tmpMem + objIdx*sizeof(StepNode));
            ChainInsertS(&gFlowCb->idleStepNodeList, &stepNode->chain);
        }
    }

    {  /*保护流程节点,单链*/
        ProtNode *protNode;

        ListInitS(&gFlowCb->idleProtNodeList);
        tmpMem = sysMemAlloc(ProtNodeAmt*sizeof(ProtNode));
        for (objIdx=0; objIdx<ProtNodeAmt; objIdx++)
        {
            protNode = (ProtNode *)(tmpMem + objIdx*sizeof(ProtNode));
            ChainInsertS(&gFlowCb->idleProtNodeList, &protNode->chain);
        }
    }

    {  /*流程/保护的入口*/
        FlowStepEntry *flowStepEntry;

        ListInitD(&gFlowCb->idleEntryList);
        tmpMem = sysMemAlloc(FlowEntryAmt*sizeof(FlowEntry));
        for (objIdx=0; objIdx<FlowEntryAmt; objIdx++)
        {
            flowStepEntry = (FlowStepEntry *)(tmpMem + objIdx*sizeof(FlowEntry));
            ChainInsertD(&gFlowCb->idleEntryList, &flowStepEntry->chain);
        }
    }

    ListInitD(&gFlowCb->busyStepEntryList);
    for (listIdx=0; listIdx<ProtGrpCri; listIdx++)
    {
        ListInitD(&gFlowCb->busyProtEntryList[listIdx]);
    }

    {  /*保护的配置项*/
        StepNpCfg *stepCfg;

        ListInitD(&gFlowCb->idleProtCfgList);
        tmpMem = sysMemAlloc(FlowStepCfgAmt*sizeof(FlowStepCfg));
        for (objIdx=0; objIdx<FlowStepCfgAmt; objIdx++)
        {
            stepCfg = (StepNpCfg *)(tmpMem + objIdx*sizeof(FlowStepCfg));
            ChainInsertD(&gFlowCb->idleProtCfgList, &stepCfg->chain);
        }
    }

    ListInitD(&gFlowCb->busyFlowProtCfgList);
    ListInitD(&gFlowCb->busyStepProtCfgList);

    return Ok;
}

#if 0
typedef struct
{
    u32 baseAddr;  /*采样存储基址*/
    u32 trayAddrOfst;  /*每盘空间大小,上层由此计算每个单盘的基址*/
    u32 smplOfst;  /*每单条采样空间大小,未必等于实际采样大小,上层由此计算采样地址*/
}StoreAddr;
/*trayAmt-托盘数量,smplSize-单采样实际大小,traySmplAmt-每盘需存储采样条数,result-返回信息*/
u8 smplStoreInit(u8 trayAmt, u16 smplSize, u16 traySmplAmt, StoreAddr *result)
{
}
void smpSizeNtfy(u16 smplSize)  /*通知单条采样大小*/
{
}
u8 smplWrite(u32 addr, u8 smplAmt, u8 *smplBuf)  /*smplAmt--采样条数*/
{
}
u8 smplRead(u32 addr, u8 smplAmt, u8 *smplBuf)
{
}
/*逻辑:上层读取管理头,若匹配则调用init,否则调用ntfy,避免下层每次都要对齐计算*/

/*trayAmt-托盘数量,smplSize-单采样实际大小,traySmplAmt-每盘需存储采样条数*/
u8 smplStoreNtfy(u8 trayAmt, u16 smplSize, u16 traySmplAmt)  /*上电调用1次*/
{/*成功返回0,否则返回1,下同*/
}
u8 smplWrite(u8 trayIdx, u32 smplSeq, u8 smplAmt, u8 *smplBuf)  /*smplAmt--采样条数*/
{/*smplSeq--序号,从零开始计数*//*秒级写,不阻塞,立即返回*/
}
u8 smplRead(u8 trayIdx, u32 smplSeq, u8 smplAmt, u8 *smplBuf)
{/*阻塞等返回*/
}

/*读写存储管理头*/
/*size--不大于256B,若sd卡读写最小单位为128B,则改成不大于128B也可以*/
 /*idx--目前不用,保留*/
u8 smplHeadWrite(u8 idx, u16 size, u8 *headBuf)
{/*秒级写,不阻塞,立即返回*/
}
/*idx--索引标识,取值0或1,防呆AB区*/
u8 smplHeadRead(u8 idx, u16 size, u8 *headBuf)
{/*阻塞等返回*/
}
#endif
#endif
