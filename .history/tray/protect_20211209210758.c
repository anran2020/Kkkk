
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include "box.h"
#include "host.h"
#include "uart.h"
#include "plc.h"
#include "channel.h"
#include "tray.h"

void mixProtCfgReset(MixProtCfg *mixProtCfg)
{
    u8 idx;

    mixProtCfg->mixExpAmt = 0;
    mixProtCfg->suffixOfst = 0;
    mixProtCfg->policyOfst = 0;
    for (idx=0; idx<MixSubCri; idx++)
    {
        mixProtCfg->lifePeriodSnd[idx] = 0;
    }

    return;
}

Ret policyParse(u8 *exp, u8 expLen, u8 numCri, MixPolicy *policy, MixPolicy *policyCri, u8 *policyAmt)
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

/*策略可以为空,但不能没有@字符*/
Ret protExpParse(u8 *exp, u16 expLen, MixProtCfg *mixProtCfg)
{
    u8 *pos;
    u8 *cri;
    u8 *suffix;
    MixExp *prot;
    MixPolicy *policy;
    MixPolicy *policyCri;
    u8 result;
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
        || Ok != expSuffixCalc(suffix, prot->suffixLen, tmp, &result)
        || Ok != policyParse(pos+1, expLen-(pos-exp)-1, PolicyCri, policy, policyCri, &prot->policyAmt))
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

u16eRspCode protExpSave(u8 *cmdPld)
{
    UpProtGenCmd *cmd;
    Tray *tray;
    UpFixProt *fixProt;
    MixProtCfg *mixProtCfg;
    u8 *pos;
    u8 idx;

    cmd = (UpProtGenCmd *)cmdPld;
    tray = &gDevMgr->tray[cmd->trayIdx];
    mixProtCfg = &tray->mixProtCfg;
    if (0 == cmd->protSeq)
    {
        mixProtCfgReset(mixProtCfg);
    }

    if (0 == cmd->fixProtAmt)
    {
        return RspOk;
    }

    for (idx=0,pos=cmdPld+sizeof(UpProtGenCmd); idx<cmd->fixProtAmt; idx++)
    {
        fixProt = (UpFixProt *)pos;
        if (Ok != protExpParse(fixProt->fixProtAby, fixProt->fixProtlen, mixProtCfg))
        {
            return RspParam;
        }
        pos += sizeof(UpFixProt) + Align8(fixProt->fixProtlen);
    }

    return RspOk;
}

/*存储负压时间参数和因子生命周期*/
void saveGenProtCfg(u8 trayIdx, u8 amt, u8 *buf)
{
    Tray *tray;
    MixProtCfg *mixProtCfg;
    UpProtUnit *prot;
    UpProtUnit *cri;
    u8 idx;

    tray = &gDevMgr->tray[trayIdx];
    mixProtCfg = &tray->mixProtCfg;
    for (prot=(UpProtUnit *)buf,cri=prot+amt; prot<cri; prot++)
    {
        if (0x0000 == prot->protId)  /*负压相关时间*/
        {
            TrayNpMgr *npMgr;

            npMgr = &tray->npMgr;
            npMgr->mkNpExprSecT00 = prot->protParam[0]/1000;
            npMgr->brkNpExprSecT10 = prot->protParam[1]/1000;
            npMgr->npLmtExprSecT06 = prot->protParam[3]/1000;
            npMgr->closeBrkDelaySecT08 = prot->protParam[6]/1000;
        }
        else if (0x0001 == prot->protId) /*因子生命周期*/
        {
            for (idx=0; idx<8; idx++)
            {
                if (prot->paramEnable & 1<<idx)
                {
                    mixProtCfg->lifePeriodSnd[idx] = prot->protParam[idx]/1000;
                }
            }
        }
        else if (0x0002 == prot->protId) /*因子生命周期*/
        {
            for (idx=0; idx<8; idx++)
            {
                if (prot->paramEnable & 1<<idx)
                {
                    mixProtCfg->lifePeriodSnd[idx+8] = prot->protParam[idx]/1000;
                }
            }
        }
    }

    return;
}

void trayProtEnable(Timer *timer)
{
    Tray *tray;
    Channel *chn;
    Channel *chnCri;
    TrayProtMgr *trayProtMgr;
    ChnProtBuf *protBuf;
    Cell *cell;
    Cell *cellCri;
    u8 idx;

    tray = Container(Tray, protEnaTmr, timer);
    trayProtMgr = &tray->trayProtMgr;
    tray->protEnable = True;
    trayProtMgr->slotTmprPre.tmprBeValid = False;
    trayProtMgr->slotTmprCrnt.tmprInvalidCnt = 0;
    trayProtMgr->preNdbdValid = False;
    trayProtMgr->smokeHpnCnt = 0;
    trayProtMgr->allSlotTmprUpLmtCnt = 0;
    trayProtMgr->busySlotTmprLowLmtCnt = 0;

    for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
    {
        protBuf = &chn->chnProtBuf;
        protBuf->cellTmprPre.tmprBeValid = False;
        protBuf->prePowerBeValid = False;
        protBuf->newCauseCode = CcNone;
        protBuf->idleVolFluctSmlCnt = 0;
        protBuf->idleTmprUpSmlCnt = 0;
        protBuf->idleCurLeakCnt = 0;
        protBuf->allChnTmprUpLmtCnt = 0;
        protBuf->busyChnTmprLowLmtCnt = 0;
        chnEnterIdle(chn);
        if (NULL != chn->series)
        {
            for (cell=chn->series->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                protBuf = &cell->chnProtBuf;
                protBuf->cellTmprPre.tmprBeValid = False;
                protBuf->prePowerBeValid = False;
                protBuf->newCauseCode = CcNone;
                protBuf->idleVolFluctSmlCnt = 0;
                protBuf->idleTmprUpSmlCnt = 0;
                protBuf->idleCurLeakCnt = 0;
                protBuf->allChnTmprUpLmtCnt = 0;
                protBuf->busyChnTmprLowLmtCnt = 0;
            }
        }
    }
    return;
}

/*todo,TmpStepSave,修正反接配置获取*/
/*发生反接保护时通道一定是空闲/暂停/停止的,所以来自下位机的采样均可覆盖*/
/*反接保护是相对比较特殊的保护,处理也不具备一致性*/
Ret chnProtReverse(Channel *chn)
{
    Tray *tray;
    TraySmpl *traySmpl;
    TrayChnSmpl *trayChnSmpl;
    Ret ret;
    b8 beFirst;

    tray = chn->box->tray;
    traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
    ret = Ok;
    if (gDevMgr->isTraySmpl)
    {
        if (NULL == chn->series)  /*并联*/
        {
            if (chn->chnProtBuf.newVolCell < gTmpReverseVol)
            {
                ret = Nok;
                tray->trayWarnPres = True;
                trayChnSmpl = &traySmpl->chnSmpl[chn->chnIdxInTray];
                chn->chnStateMed = ChnStaIdle;
                /*chn->chnStateMed = StepProtPause==gTmpStepProtPolicy ? ChnStaPause : ChnStaStop;*/
                traySmplDefChnSmpl(tray, chn, chnStaMapMed2Up(chn), Cc1Reverse);
                trayChnSmpl->volCell = chn->chnProtBuf.newVolCell;
                trayChnSmpl->volCur = chn->chnProtBuf.newVolCur;
                trayChnSmpl->volPort = chn->chnProtBuf.newVolPort;
                if (0 == chn->smplAmtInLoop)
                {
                    chn->smplAmtInLoop = 1;
                }
            }
        }
        else
        {
            Cell *cell;
            Cell *cellCri;

            for (beFirst=True,cell=chn->series->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                if (cell->chnProtBuf.newVolCell < gTmpReverseVol)
                {
                    ret = Nok;
                    tray->trayWarnPres = True;
                    chn->chnStateMed = ChnStaIdle;
                    /*chn->chnStateMed = StepProtPause==gTmpStepProtPolicy ? ChnStaPause : ChnStaStop;*/
                    if (beFirst)
                    {
                        if (0 == chn->smplAmtInLoop)
                        {
                            chn->smplAmtInLoop = 1;
                        }
                        traySmplDefChnSmpl(tray, chn, ChnUpStateTemp, Cc1Reverse);
                        beFirst = False;
                    }
                    trayChnSmpl = &traySmpl->chnSmpl[cell->genIdxInTray];
                    trayChnSmpl->chnUpState = chnStaMapMed2Up(chn);
                    trayChnSmpl->volCell = cell->chnProtBuf.newVolCell;
                    trayChnSmpl->volCur = cell->chnProtBuf.newVolCur;
                }
            }
        }
    }
    else
    {
    }

    return ret;
}

/*设置导致整盘保护的原因码*/
void setTrayProtCause(Tray *tray, u16eCauseCode causeCode)
{
    TrayProtMgr *trayProtMgr;

    trayProtMgr = &tray->trayProtMgr;
    CcChkModify(trayProtMgr->trayCauseCode, causeCode);
    return;
}

/*记录通道发生的非因子保护*/
void setChnNmlProt(Channel *chn, ChnProtBuf *chnProtBuf, u16eCauseCode causeCode)
{
    if (CcBaseGet(causeCode) > chnProtBuf->preCauseCode)
    {
        CcChkModify(chnProtBuf->newCauseCode, causeCode);
        if (NULL != chn->series)
        {
            if (CcNone == chn->chnProtBuf.preCauseCode)
            {
                CcChkModify(chn->chnProtBuf.newCauseCode, Cc0SeriesTrig);
            }
        }
    }

    return;
}

/*记录通道发生的保护因子*/
void setChnMixSubProt(Channel *chn, ChnProtBuf *chnProtBuf, u8eMixSubId mixSubId, u16eCauseCode causeCode)
{
    BitSet(chnProtBuf->mixSubHpnBitmap, mixSubId);
    chnProtBuf->mixSubHpnSec[mixSubId] = sysTimeSecGet();
    if (CcBaseGet(causeCode) > chnProtBuf->preCauseCode)
    {
        CcChkModify(chnProtBuf->newCauseCode, causeCode);
    }

    if (NULL != chn->series)
    {
        if (CcNone == chn->chnProtBuf.preCauseCode)
        {
            CcChkModify(chn->chnProtBuf.newCauseCode, Cc0SeriesTrig);
        }
    }
    return;
}

u32 resistanceCalc(Channel *chn, s32 vol01, s32 vol02, s32 current)
{
    u32 dif;
    u32 curAbs;

    curAbs = AbsVal(current);
    if (curAbs < 1000000) /*小于10A的恒压充放电就不计算接触阻抗了不准*/
    {
        if (StepTypeCVC==chn->stepTypeTmp
            || (StepTypeCCCVC==chn->stepTypeTmp && StepSubTypeCV==chn->stepSubType)
            || (StepTypeCCCVD==chn->stepTypeTmp && StepSubTypeCV==chn->stepSubType))
        {
            return 0;
        }
    }

    dif = AbsDifVal(vol01, vol02);
    dif *= 1000;
    curAbs /= 10;

    return 0==curAbs ? 0 : dif/curAbs;
}

/*除了反接保护,所有保护项对应的函数,只做逻辑运算和记录*/
/*忙时气压突升,因子*/
void trayBusyNpBigRise0100(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    TrayProtMgr *trayProtMgr;

    /*不稳定态也算忙时*/
    if (chn->chnStateMed < ChnStaRun)  /*忙闲判断,暂时先如此*/
    {
        return;
    }

    trayProtMgr = &tray->trayProtMgr;
    if (trayProtMgr->npWiSw || !tray->ndbdData.ndbdDataValid || !trayProtMgr->preNdbdValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (tray->ndbdData.status[NdbdSenNpVal]-trayProtMgr->preNdbdNp > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc3BusyNpBigRise);
            setChnMixSubProt(chn, chnProtBuf, MixSubBusyNpBigRise, Cc3BusyNpBigRise);
        }
    }

    return;
}

/*忙时通道温度突升,因子*/
void trayBusyChnTmprBigRise0101(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    TmprData *tmprPre;
    TmprData *tmprCrnt;

    /*不稳定态也算忙时*/
    if (chn->chnStateMed < ChnStaRun)  /*忙闲判断,暂时先如此*/
    {
        return;
    }

    tmprPre = &chnProtBuf->cellTmprPre;
    tmprCrnt = &chnProtBuf->cellTmprCrnt;
    if (!tmprPre->tmprBeValid || !tmprCrnt->tmprBeValid || tmprPre->tmprVal>tmprCrnt->tmprVal)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (tmprCrnt->tmprVal-tmprPre->tmprVal > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubBusyChnTmprBigRise, Cc4BusyChnTmprBigRise);
        }
    }

    return;
}

/*忙时温度下限,非因子*/
void trayBusyTmprLowLmt0103(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    TmprData *tmpr;

    if (chn->chnStateMed < ChnStaRun)  /*忙闲判断,暂时先如此*/
    {
        return;
    }

    if (prot->paramEnable & 1<<2)  /*库温*/
    {
        TrayProtMgr *trayProtMgr;

        trayProtMgr = &tray->trayProtMgr;
        tmpr = &trayProtMgr->slotTmprCrnt;
        if (tmpr->tmprBeValid && tmpr->tmprVal<prot->protParam[2])
        {
            if (prot->paramEnable & 1<<3)
            {
                trayProtMgr->busySlotTmprLowLmtCnt++;
            }
            if (!(prot->paramEnable & 1<<3) || trayProtMgr->busySlotTmprLowLmtCnt>=prot->protParam[3])
            {
                setTrayProtCause(tray, Cc0BusySlotTmprLowLmt);
                trayProtMgr->busySlotTmprLowLmtCnt = 0;
            }
        }
        else
        {
            trayProtMgr->busySlotTmprLowLmtCnt = 0;
        }
    }

    if (prot->paramEnable & 1<<0)  /*通道温度*/
    {
        tmpr = &chnProtBuf->cellTmprCrnt;
        if (tmpr->tmprBeValid && tmpr->tmprVal<prot->protParam[0])
        {
            if (prot->paramEnable & 1<<1)
            {
                chnProtBuf->busyChnTmprLowLmtCnt++;
            }
            if (!(prot->paramEnable & 1<<1) || chnProtBuf->busyChnTmprLowLmtCnt>=prot->protParam[1])
            {
                setTrayProtCause(tray, Cc0TrayTrig);
                setChnNmlProt(chn, chnProtBuf, Cc2BusyChnTmprLowLmt);
                chnProtBuf->busyChnTmprLowLmtCnt = 0;
            }
        }
        else
        {
            chnProtBuf->busyChnTmprLowLmtCnt = 0;
        }
    }

    return;
}

/*闲时电压上波动,因子*/
void chnIdleVolUpFluct0200(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    u32 cnt;
    s32 dif;
    
    if (chn->chnStateMed > ChnStaNp)  /*忙闲判断,暂时先如此*/
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    dif = chnProtBuf->newVolCell - chnProtBuf->preVolCell;
    if (prot->paramEnable & 1<<0)  /*突升*/
    {
        if (dif > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubIdleVolFluctBig, Cc4IdleVolUpFluctBig);
        }
    }

    if (prot->paramEnable & 1<<2) /*连续小幅上升*/
    {
        cnt = prot->paramEnable & 1<<3 ? prot->protParam[3] : 1;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->idleVolFluctSmlCnt++;
            if (chnProtBuf->idleVolFluctSmlCnt >= cnt)
            {
                setTrayProtCause(tray, Cc0TrayTrig);
                setChnMixSubProt(chn, chnProtBuf, MixSubIdleVolFluctSml, Cc4IdleVolUpFluctSml);
                chnProtBuf->idleVolFluctSmlCnt = 0;
            }
        }
        else
        {
            chnProtBuf->idleVolFluctSmlCnt = 0;
        }
    }

    return;
}

/*闲时温度上波动,因子*/
void chnIdleTmprUpFluct0201(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    u32 cnt;
    s32 dif;
    
    if (chn->chnStateMed > ChnStaNp)
    {
        return;
    }

    if (!chnProtBuf->cellTmprCrnt.tmprBeValid || !chnProtBuf->cellTmprPre.tmprBeValid)
    {
        return;
    }

    if (chnProtBuf->cellTmprCrnt.tmprVal > chnProtBuf->cellTmprPre.tmprVal)
    {
        dif = chnProtBuf->cellTmprCrnt.tmprVal - chnProtBuf->cellTmprPre.tmprVal;
    }
    else
    {
        chnProtBuf->idleTmprUpSmlCnt = 0;
        return;
    }

    if (prot->paramEnable & 1<<0)  /*突升*/
    {
        if (dif > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubIdleChnTmprUpSud, Cc4IdleTmprUpFluctBig);
        }
    }

    if (prot->paramEnable & 1<<2) /*连续小幅上升*/
    {
        cnt = prot->paramEnable & 1<<3 ? prot->protParam[3] : 1;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->idleTmprUpSmlCnt++;
            if (chnProtBuf->idleTmprUpSmlCnt >= cnt)
            {
                setTrayProtCause(tray, Cc0TrayTrig);
                setChnMixSubProt(chn, chnProtBuf, MixSubIdleChnTmprUpCtnu, Cc4IdleTmprUpFluctSml);
                chnProtBuf->idleTmprUpSmlCnt = 0;
            }
        }
        else
        {
            chnProtBuf->idleTmprUpSmlCnt = 0;
        }
    }

    return;
}

/*闲时漏电流,因子*/
void chnIdleCurLeak0202(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 cur;
    u32 cnt;

    if (chn->chnStateMed > ChnStaNp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        cur = chnProtBuf->newCur<0 ? 0-chnProtBuf->newCur : chnProtBuf->newCur;
        if (cur > prot->protParam[0])
        {
            chnProtBuf->idleCurLeakCnt++;
            cnt = prot->paramEnable & 1<<1 ? prot->protParam[1] : 1;
            if (chnProtBuf->idleCurLeakCnt >= cnt)
            {
                setTrayProtCause(tray, Cc0TrayTrig);
                setChnMixSubProt(chn, chnProtBuf, MixSubIdleCurLeak, Cc4IdleCurLeak);
            }
        }
        else
        {
            chnProtBuf->idleCurLeakCnt = 0;
        }
    }

    return;
}

/*闲时电压累加上限,非因子*/
/*进入闲时后,以首采样或延时设定时间采样为基准,以后时间的电压上升超过设定值则保护*/
void chnIdleVolTtlRise0203(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 dif;

    if (chn->chnStateMed > ChnStaNp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->idleVolBaseValid)
    {
        if (!(prot->paramEnable & 1<<1) || sysTimeSecGet()-chnProtBuf->idleTimeStampSec>=prot->protParam[1]/1000)
        {
            chnProtBuf->idleVolBase = chnProtBuf->newVolCell;
            chnProtBuf->idleVolBaseValid = True;
        }
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newVolCell < chnProtBuf->idleVolBase)
        {
            return;
        }

        dif = chnProtBuf->newVolCell - chnProtBuf->idleVolBase;
        if (dif > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2IdleVolRiseTtl);
        }
    }

    return;
}

/*全时气压上限,因子*/
void trayAllNpUpLmt0300(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (prot->paramEnable & 1<<0)
    {
        if (tray->ndbdData.status[NdbdSenNpVal] > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc3AllNpUpLmt);
            setChnMixSubProt(chn, chnProtBuf, MixSubAllAirPrsMax, Cc3AllNpUpLmt);
        }
    }
    return;
}

/*全时温度上限,因子*/
void trayAllTmprUpLmt0302(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    TmprData *tmpr;

    if (prot->paramEnable & 1<<2)  /*库温*/
    {
        TrayProtMgr *trayProtMgr;

        trayProtMgr = &tray->trayProtMgr;
        tmpr = &trayProtMgr->slotTmprCrnt;
        if (tmpr->tmprBeValid && tmpr->tmprVal>prot->protParam[2])
        {
            if (prot->paramEnable & 1<<3)
            {
                trayProtMgr->allSlotTmprUpLmtCnt++;
            }
            if (!(prot->paramEnable & 1<<3) || trayProtMgr->allSlotTmprUpLmtCnt>=prot->protParam[3])
            {
                setTrayProtCause(tray, Cc3AllSlotTmprUpLmt);
                setChnMixSubProt(chn, chnProtBuf, MixSubAllTmprMax, Cc3AllSlotTmprUpLmt);
                trayProtMgr->allSlotTmprUpLmtCnt = 0;
            }
        }
        else
        {
            trayProtMgr->allSlotTmprUpLmtCnt = 0;
        }
    }

    if (prot->paramEnable & 1<<0)  /*通道温度*/
    {
        tmpr = &chnProtBuf->cellTmprCrnt;
        if (tmpr->tmprBeValid && tmpr->tmprVal>prot->protParam[0])
        {
            if (prot->paramEnable & 1<<1)
            {
                chnProtBuf->allChnTmprUpLmtCnt++;
            }
            if (!(prot->paramEnable & 1<<1) || chnProtBuf->allChnTmprUpLmtCnt>=prot->protParam[1])
            {
                setTrayProtCause(tray, Cc0TrayTrig);
                setChnMixSubProt(chn, chnProtBuf, MixSubAllTmprMax, Cc3AllCellTmprUpLmt);
                chnProtBuf->allChnTmprUpLmtCnt = 0;
            }
        }
        else
        {
            chnProtBuf->allChnTmprUpLmtCnt = 0;
        }
    }

    return;
}

/*全时烟感,因子*/
void trayAllSmoke0303(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    u8 amtCfg;  /*数量*/
    u8 amtHpn;
    u8 cntCfg;  /*连续次数*/

    amtCfg = cntCfg = 1;
    amtHpn = 0;
    if (NULL != prot)
    {
        if (prot->paramEnable & 1<<0)
        {
            amtCfg = prot->protParam[0];
        }
        if (prot->paramEnable & 1<<1)
        {
            cntCfg = prot->protParam[1];
        }
    }

    if (tray->ndbdData.warn[NdbdWarnSmoke])
    {
        amtHpn = 3==tray->ndbdData.warn[NdbdWarnSmoke] ? 2 : 1;
    }

    if (amtHpn >= amtCfg)
    {
        tray->trayProtMgr.smokeHpnCnt++;
        if (tray->trayProtMgr.smokeHpnCnt >= cntCfg)
        {
            setTrayProtCause(tray, Cc3AllSmoke);
            setChnMixSubProt(chn, chnProtBuf, MixSubAllSmoke, Cc3AllSmoke);
            tray->trayProtMgr.smokeHpnCnt = 0;
        }
    }
    else
    {
        tray->trayProtMgr.smokeHpnCnt = 0;
    }

    return;
}

/*电压波动,静置工步,因子但不影响全盘*/
void chnQuietVolFluct0401(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 dif;
    
    if (ChnStaRun!=chn->chnStateMed || StepTypeQuiet!=chn->stepTypeTmp || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<1)  /*延时*/
    {
        if (chn->stepRunTime < prot->protParam[1])
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        dif = AbsDifVal(chnProtBuf->newVolCell,chnProtBuf->preVolCell);
        if (dif > prot->protParam[0])
        {
            /*setTrayProtCause(tray, Cc0TrayTrig);*//*暂定工步因子不影响全盘*/
            setChnMixSubProt(chn, chnProtBuf, MixSubBusyQuietVolFluct, Cc4QuietVolFluct);
        }
    }

    return;
}

/*电压下降,静置工步,非因子*/
/*基准时间以后的采样,与工步首条采样,压降大于设定则触发*/
void chnQuietVolDown0402(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || StepTypeQuiet!=chn->stepTypeTmp || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->quietVolDownBaseValid)
    {
        chnProtBuf->quietVolDownBase = chnProtBuf->newVolCell;
        chnProtBuf->quietVolDownBaseValid = True;
        return;
    }

    if (!(prot->paramEnable & 1<<0) || chn->stepRunTime>prot->protParam[0])
    {
        if (prot->paramEnable & 1<<1)  /**/
        {
            if (chnProtBuf->newVolCell < chnProtBuf->quietVolDownBase)
            {
                s32 dif;

                dif = chnProtBuf->quietVolDownBase - chnProtBuf->newVolCell;
                if (dif > prot->protParam[1])
                {
                    setChnNmlProt(chn, chnProtBuf, Cc1QuietVolDown);
                }
            }
        
        }
    }

    return;
}

/*静置漏电流,非因子*/
void chnQuietCurLeak0403(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 cur;

    if (ChnStaRun!=chn->chnStateMed || StepTypeQuiet!=chn->stepTypeTmp || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        cur = chnProtBuf->newCur<0 ? 0-chnProtBuf->newCur : chnProtBuf->newCur;
        if (cur > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1QuietCurLeak);
        }
    }

    return;
}

/*工步电压上下限,非因子*/
void chnStepVolLmt0404(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newVolCell > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1StepVolUpLmt);
            return;
        }
    }

    if (prot->paramEnable & 1<<1)
    {
        if (chnProtBuf->newVolCell < prot->protParam[1])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1StepVolLowLmt);
        }
    }

    return;
}

/*工步通道温度突升,非因子*/
void chnStepChnTmprBigRise0405(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    TmprData *tmprPre;
    TmprData *tmprCrnt;

    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    tmprPre = &chnProtBuf->cellTmprPre;
    tmprCrnt = &chnProtBuf->cellTmprCrnt;
    if (!tmprPre->tmprBeValid || !tmprCrnt->tmprBeValid || tmprPre->tmprVal>tmprCrnt->tmprVal)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (tmprCrnt->tmprVal-tmprPre->tmprVal > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1StepChnTmprBigRise);
        }
    }

    return;
}

/*,非因子*/
void chnCccSetTimeVol0406(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }
    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chn->stepRunTime <= prot->protParam[0])
        {
            if (chnProtBuf->newVolCell > prot->protParam[1])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccSetTimeVolUpLmt);
            }
        }
    }

    if (prot->paramEnable & 1<<2)
    {
        if (chn->stepRunTime > prot->protParam[2])
        {
            if (chnProtBuf->newVolCell < prot->protParam[3])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccSetTimeVolLowLmt);
            }
        }
    }

    return;
}

/*,非因子*/
void chnCcCurOfst0407(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType)
        && StepTypeCCD!=chn->stepTypeTmp && (StepTypeCCCVD!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<1)  /*延时*/
    {
        if (chn->stepRunTime <= prot->protParam[1])
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        UpStepInfo *step;
        s32 curAbs;
        s32 dif;

        step = (UpStepInfo *)getChnStepInfo(chn->stepIdTmp);
        curAbs = AbsVal(chnProtBuf->newCur);
        dif = AbsDifVal(curAbs, step->stepParam[0]);
        if (dif > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1CcCurOfst);
        }
    }

    return;
}

/*,因子*/
void chnCccVolDownAbnm0408(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->cccVolDownAbnmValid)
    {
        if (!(prot->paramEnable & 1<<0)
            || chnProtBuf->newCap>=prot->protParam[0]&&chnProtBuf->newCap<=prot->protParam[1])
        {
            chnProtBuf->cccVolDownAbnmValid = True;
            chnProtBuf->cccVolDownAbnmVolBase = chnProtBuf->newVolCell;
            chnProtBuf->cccVolDownAbnmCapMin = prot->protParam[0];
            chnProtBuf->cccVolDownAbnmCnt = 0;
        }

        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->cccVolDownAbnmCapMin != prot->protParam[0])
        {
            if (chnProtBuf->newCap>=prot->protParam[0]&&chnProtBuf->newCap<=prot->protParam[1])
            {
                chnProtBuf->cccVolDownAbnmVolBase = chnProtBuf->newVolCell;
                chnProtBuf->cccVolDownAbnmCapMin = prot->protParam[0];
                chnProtBuf->cccVolDownAbnmCnt = 0;
            }
            return;
        }
        else
        {
            if (chnProtBuf->newCap > prot->protParam[1])
            {
                chnProtBuf->cccVolDownAbnmValid = False;
                return;
            }
        }
    }

    if (chnProtBuf->newVolCell > chnProtBuf->cccVolDownAbnmVolBase)
    {
        chnProtBuf->cccVolDownAbnmVolBase = chnProtBuf->newVolCell;
        chnProtBuf->cccVolDownAbnmCnt = 0;
        return;
    }

    if (!chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (chnProtBuf->newVolCell >= chnProtBuf->preVolCell)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)
    {
        s32 dif;

        dif = chnProtBuf->cccVolDownAbnmVolBase - chnProtBuf->newVolCell;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->cccVolDownAbnmCnt++;
            if (!(prot->paramEnable & 1<<3) || chnProtBuf->cccVolDownAbnmCnt >= prot->protParam[3])
            {
                /*setTrayProtCause(tray, Cc0TrayTrig);*//*暂定工步因子不影响全盘*/
                setChnMixSubProt(chn, chnProtBuf, MixSubCccVolDownAbnm, Cc4CccVolDownAbnm);
            }
        }
    }

    return;
}

/*,非因子*/
void chnCccVolRiseRate0409(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 dif;

    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newCap<=prot->protParam[0] || chnProtBuf->newCap>prot->protParam[1])
        {
            return;
        }
    }

    if (!(prot->paramEnable & 1<<0) || 0==prot->protParam[0])
    {
        if (chn->stepRunTime <= 2000)
        {
            return;
        }
    }

    dif = chnProtBuf->newVolCell - chnProtBuf->preVolCell;
    if (prot->paramEnable & 1<<2)
    {
        if (dif > prot->protParam[2])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1CccVolRiseRateUpLmt);
        }
    }

    if (prot->paramEnable & 1<<3)
    {
        if (dif < prot->protParam[3])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1CccVolRiseRateLowLmt);
        }
    }
    return;
}

/*电压突降,恒流,其中恒流充时作为因子*/
void chnCcVolBigDown040a(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType)
        && StepTypeCCD!=chn->stepTypeTmp && (StepTypeCCCVD!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newCap<=prot->protParam[0] || chnProtBuf->newCap>prot->protParam[1])
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<3)  /*延时*/
    {
        if (chn->stepRunTime <= prot->protParam[3])
        {
            return;
        }
    }

    if ((!(prot->paramEnable & 1<<0) || 0==prot->protParam[0])
        && (!(prot->paramEnable & 1<<3) || prot->protParam[3]<=1000))
    {
        if (chn->stepRunTime <= 1000)
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<2)  /**/
    {
        if (chnProtBuf->newVolCell < chnProtBuf->preVolCell)
        {
            s32 dif;

            dif = chnProtBuf->preVolCell - chnProtBuf->newVolCell;
            if (dif > prot->protParam[2])
            {
                if (StepTypeCCC==chn->stepTypeTmp || StepTypeCCCVC==chn->stepTypeTmp)
                {
                    /*setTrayProtCause(tray, Cc0TrayTrig);*//*暂定工步因子不影响全盘*/
                    setChnMixSubProt(chn, chnProtBuf, MixSubCccVolBigDown, Cc4CccVolBigDown);
                }
                else
                {
                    setChnNmlProt(chn, chnProtBuf, Cc1CcdVolBigDown);
                }
            }
        }
    }

    return;
}

/*上下限,非因子*/
void chnStepCapLmt040b(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || StepTypeQuiet==chn->stepTypeTmp || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newCap > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1StepCapUpLmt);
            return;
        }
    }

    if (prot->paramEnable & 1<<1)
    {
        UpStepInfo *step;
        u32 stepCfgTimeMs;

        step = (UpStepInfo *)getChnStepInfo(chn->stepIdTmp);
        stepCfgTimeMs = step->stepParam[2];
        if (StepTypeCCC==chn->stepTypeTmp || StepTypeCCD==chn->stepTypeTmp)
        {
            stepCfgTimeMs = step->stepParam[1];
        }
        if (chn->stepRunTime >= stepCfgTimeMs-2000)
        //if (chnStepBeJustNmlEnd(chn))
        {
            if (chnProtBuf->newCap < prot->protParam[1])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1StepCapLowLmt);
            }
        }
    }

    return;
}

/*电压间隔波动,恒流工步,非因子*/
void chnCcVolIntvlFluct040c(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 dif;
    u32 intvl;
    
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType)
        && StepTypeCCD!=chn->stepTypeTmp && (StepTypeCCCVD!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->ccVolIntvlFluctBaseValid)
    {
        chnProtBuf->ccVolIntvlFluctVolBase = chnProtBuf->newVolCell;
        chnProtBuf->ccVolIntvlFluctTimeBaseMs = chn->stepRunTime;
        chnProtBuf->ccVolIntvlFluctBaseValid = True;
        return;
    }

    dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->ccVolIntvlFluctVolBase);
    if (prot->paramEnable & 1<<1)  /**/
    {
        if (dif > prot->protParam[1])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1CcVolIntvlFluctUpLmt);
        }
    }

    intvl = (prot->paramEnable & 1<<0) ? prot->protParam[0] : 10000;
    if (chn->stepRunTime - chnProtBuf->ccVolIntvlFluctTimeBaseMs < intvl)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)  /**/
    {
        if (dif < prot->protParam[2])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1CcVolIntvlFluctLowLmt);
        }
    }

    chnProtBuf->ccVolIntvlFluctVolBase = chnProtBuf->newVolCell;
    chnProtBuf->ccVolIntvlFluctTimeBaseMs = chn->stepRunTime;
    return;
}

/*ccc电压连续上升,非因子*/
void chnCccVolRiseCtnu040d(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    u32 cnt;
    s32 dif;
    
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)  /*延时*/
    {
        if (chn->stepRunTime <= prot->protParam[2])
        {
            return;
        }
    }

    if (!(prot->paramEnable & 1<<0))
    {
        return;
    }
    
    if (chnProtBuf->newVolCell > chnProtBuf->preVolCell)
    {
        dif = chnProtBuf->newVolCell - chnProtBuf->preVolCell;
        if (dif > prot->protParam[0])
        {
            chnProtBuf->cccVolRiseCtnuCnt++;
            cnt = prot->paramEnable & 1<<1 ? prot->protParam[1] : 1;
            if (chnProtBuf->cccVolRiseCtnuCnt >= cnt)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccVolRiseCtnu);
                chnProtBuf->cccVolRiseCtnuCnt = 0;
            }
            return;
        }
    }

    chnProtBuf->cccVolRiseCtnuCnt = 0;
    return;
}

/*ccc电压连续下降,非因子*/
void chnCccVolDownCtnu040e(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    u32 cnt;
    s32 dif;
    
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)  /*延时*/
    {
        if (chn->stepRunTime <= prot->protParam[2])
        {
            return;
        }
    }

    if (!(prot->paramEnable & 1<<0))
    {
        return;
    }
    
    if (chnProtBuf->preVolCell > chnProtBuf->newVolCell)
    {
        dif = chnProtBuf->preVolCell - chnProtBuf->newVolCell;
        if (dif > prot->protParam[0])
        {
            chnProtBuf->cccVolDownCtnuCnt++;
            cnt = prot->paramEnable & 1<<1 ? prot->protParam[1] : 1;
            if (chnProtBuf->cccVolDownCtnuCnt >= cnt)
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CccVolDownCtnu);
                chnProtBuf->cccVolDownCtnuCnt = 0;
            }
            return;
        }
    }

    chnProtBuf->cccVolDownCtnuCnt = 0;
    return;
}

/*电压上升异常点,恒流放电,非因子*/
void chnCcdVolRiseAbnm040f(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCCD!=chn->stepTypeTmp && (StepTypeCCCVD!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->ccdVolRiseAbnmValid)
    {
        if (!(prot->paramEnable & 1<<0)
            || chnProtBuf->newCap>=prot->protParam[0]&&chnProtBuf->newCap<=prot->protParam[1])
        {
            chnProtBuf->ccdVolRiseAbnmValid = True;
            chnProtBuf->ccdVolRiseAbnmVolBase = chnProtBuf->newVolCell;
            chnProtBuf->ccdVolRiseAbnmCapMin = prot->protParam[0];
            chnProtBuf->ccdVolRiseAbnmCnt = 0;
        }

        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->ccdVolRiseAbnmCapMin != prot->protParam[0])
        {
            if (chnProtBuf->newCap>=prot->protParam[0]&&chnProtBuf->newCap<=prot->protParam[1])
            {
                chnProtBuf->ccdVolRiseAbnmVolBase = chnProtBuf->newVolCell;
                chnProtBuf->ccdVolRiseAbnmCapMin = prot->protParam[0];
                chnProtBuf->ccdVolRiseAbnmCnt = 0;
            }
            return;
        }
        else
        {
            if (chnProtBuf->newCap > prot->protParam[1])
            {
                chnProtBuf->ccdVolRiseAbnmValid = False;
                return;
            }
        }
    }

    if (chnProtBuf->newVolCell < chnProtBuf->ccdVolRiseAbnmVolBase)
    {
        chnProtBuf->ccdVolRiseAbnmVolBase = chnProtBuf->newVolCell;
        chnProtBuf->ccdVolRiseAbnmCnt = 0;
        return;
    }

    if (!chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (chnProtBuf->newVolCell <= chnProtBuf->preVolCell)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)
    {
        s32 dif;

        dif = chnProtBuf->newVolCell - chnProtBuf->ccdVolRiseAbnmVolBase;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->ccdVolRiseAbnmCnt++;
            if (!(prot->paramEnable & 1<<3) || chnProtBuf->ccdVolRiseAbnmCnt >= prot->protParam[3])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CcdVolRiseAbnm);
            }
        }
    }

    return;
}

/*,非因子*/
void chnCvcVolOfst0410(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun != chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCVC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCV!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<1)  /*延时*/
    {
        if (chn->stepRunTime < prot->protParam[1])
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        UpStepInfo *step;
        s32 dif;

        step = (UpStepInfo *)getChnStepInfo(chn->stepIdTmp);
        dif = AbsDifVal(chnProtBuf->newVolCell, step->stepParam[1]);
        if (dif > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1CvcVolOfst);
        }
    }

    return;
}

/*电流上升异常点,恒压充电,非因子*/
void chnCvcCurRiseAbnm0411(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCVC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCV!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->cvcCurRiseAbnmValid)
    {
        if (!(prot->paramEnable & 1<<0)
            || chnProtBuf->newCap>=prot->protParam[0]&&chnProtBuf->newCap<=prot->protParam[1])
        {
            chnProtBuf->cvcCurRiseAbnmValid = True;
            chnProtBuf->cvcCurRiseAbnmCurBase = chnProtBuf->newCur;
            chnProtBuf->cvcCurRiseAbnmCapMin = prot->protParam[0];
            chnProtBuf->cvcCurRiseAbnmCnt = 0;
        }

        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->cvcCurRiseAbnmCapMin != prot->protParam[0])
        {
            if (chnProtBuf->newCap>=prot->protParam[0]&&chnProtBuf->newCap<=prot->protParam[1])
            {
                chnProtBuf->cvcCurRiseAbnmCurBase = chnProtBuf->newCur;
                chnProtBuf->cvcCurRiseAbnmCapMin = prot->protParam[0];
                chnProtBuf->cvcCurRiseAbnmCnt = 0;
            }
            return;
        }
        else
        {
            if (chnProtBuf->newCap > prot->protParam[1])
            {
                chnProtBuf->cvcCurRiseAbnmValid = False;
                return;
            }
        }
    }

    if (chnProtBuf->newCur < chnProtBuf->cvcCurRiseAbnmCurBase)
    {
        chnProtBuf->cvcCurRiseAbnmCurBase = chnProtBuf->newCur;
        chnProtBuf->cvcCurRiseAbnmCnt = 0;
        return;
    }

    if (!chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (chnProtBuf->newCur <= chnProtBuf->preCur)
    {
        return;
    }

    if (prot->paramEnable & 1<<2)
    {
        s32 dif;

        dif = chnProtBuf->newCur - chnProtBuf->cvcCurRiseAbnmCurBase;
        if (dif > prot->protParam[2])
        {
            chnProtBuf->cvcCurRiseAbnmCnt++;
            if (!(prot->paramEnable & 1<<3) || chnProtBuf->cvcCurRiseAbnmCnt >= prot->protParam[3])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CvcCurRiseAbnm);
            }
        }
    }

    return;
}

/*电流突升,恒压充电,非因子*/
void chnCvcCurBigRise0412(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (StepTypeCVC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCV!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid || !chnProtBuf->prePowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newCap<=prot->protParam[0] || chnProtBuf->newCap>prot->protParam[1])
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<3)  /*延时*/
    {
        if (chn->stepRunTime <= prot->protParam[3])
        {
            return;
        }
    }

    if ((!(prot->paramEnable & 1<<0) || 0==prot->protParam[0])
        && (!(prot->paramEnable & 1<<3) || prot->protParam[3]<=1000))
    {
        if (chn->stepRunTime <= 1000)
        {
            return;
        }
    }

    if (prot->paramEnable & 1<<2)  /**/
    {
        if (chnProtBuf->newCur > chnProtBuf->preCur)
        {
            s32 dif;

            dif = chnProtBuf->newCur - chnProtBuf->preCur;
            if (dif > prot->protParam[2])
            {
                setChnNmlProt(chn, chnProtBuf, Cc1CvcCurBigRise);
            }
        }
    }

    return;
}

/*电压回检,静置工步,非因子*/
void chnQuietVolChkBack0413(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 dif;
    
    if (ChnStaRun!=chn->chnStateMed || StepTypeQuiet!=chn->stepTypeTmp || prot->stepId!=chn->stepIdTmp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /**/
    {
        dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->newVolCur);
        if (dif > prot->protParam[0])
        {
            setChnNmlProt(chn, chnProtBuf, Cc1QuietVolChkBack);
        }
    }

    return;
}

/*流程温度上下限,非因子但影响全盘*/
void flowChnTmprLmt0501(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    TmprData *tmpr;

    tmpr = &chnProtBuf->cellTmprCrnt;
    if (!tmpr->tmprBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /*通道温度上限*/
    {
        if (tmpr->tmprVal>prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnTmprUpLmt);
        }
    }

    if (prot->paramEnable & 1<<1)  /*通道温度上限*/
    {
        if (tmpr->tmprVal<prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnTmprLowLmt);
        }
    }

    return;
}

/*过充,非因子但影响全盘*/
void flowChgOvld0502(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || (StepTypeCCC!=chn->stepTypeTmp
        && StepTypeCCCVC!=chn->stepTypeTmp && StepTypeCVC!=chn->stepTypeTmp))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /*电压上限*/
    {
        if (chnProtBuf->newVolCell > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnChgOvldVol);
        }
    }

    if (prot->paramEnable+chn->capSum & 1<<1)  /*容量上限*/
    {
        if (chnProtBuf->newCap > prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnChgOvldCap);
        }
    }

    if (prot->paramEnable & 1<<2)  /*电流上限*/
    {
        s32 curAbs;

        curAbs = chnProtBuf->newCur;
        if (curAbs > prot->protParam[2])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnChgOvldCur);
        }
    }

    return;
}

/*过放,非因子但影响全盘*/
void flowDisChgOvld0503(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (ChnStaRun!=chn->chnStateMed || (StepTypeCCD!=chn->stepTypeTmp
        && StepTypeCCCVD!=chn->stepTypeTmp && StepTypeCVD!=chn->stepTypeTmp))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)  /*电压下限*/
    {
        if (chnProtBuf->newVolCell < prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnDisChgOvldVol);
        }
    }

    if (prot->paramEnable & 1<<1)  /*容量上限*/
    {
        if (chnProtBuf->newCap+chn->capSum > prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnDisChgOvldCap);
        }
    }

    if (prot->paramEnable & 1<<2)  /*电流上限*/
    {
        s32 curAbs;

        curAbs = AbsVal(chnProtBuf->newCur);
        if (curAbs > prot->protParam[2])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnDisChgOvldCur);
        }
    }

    return;
}

/*接触,非因子但影响全盘*/
void flowContactChk0504(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 dif;

    if (ChnStaRun!=chn->chnStateMed || StepTypeQuiet==chn->stepTypeTmp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (NULL == chn->series)
    {
        if (prot->paramEnable & 1<<0)  /*接触不良压差电芯端口*/
        {
            dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->newVolPort);
            if (dif > prot->protParam[0])
            {
                setTrayProtCause(tray, Cc0TrayTrig);
                setChnNmlProt(chn, chnProtBuf, Cc2FlowChnVolCell2Port);
            }
        }
    }

    if (prot->paramEnable & 1<<1)  /*极耳不良压差电芯探针*/
    {
        dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->newVolCur);
        if (dif > prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnVolCell2Cur);
        }
    }

    if (prot->paramEnable & 1<<2)  /*接触阻抗上限*/
    {
        s32 resis;

        resis = resistanceCalc(chn, chnProtBuf->newVolCell, chnProtBuf->newVolCur, chnProtBuf->newCur);
        if (resis > prot->protParam[2])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnNmlProt(chn, chnProtBuf, Cc2FlowChnContactResis);
        }
    }

    return;
}

/*流程闲时电压间隔上波动,因子,首个间隔不计*/
void flowIdleVolIntvlRise0505(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 dif;
    u32 intvlSec;
    u32 intvlCfg;
    u32 sysSec;
    
    if (chn->chnStateMed > ChnStaNp)
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    sysSec = sysTimeSecGet();
    intvlSec = sysSec - chnProtBuf->idleTimeStampSec;
    intvlCfg = (prot->paramEnable & 1<<0) ? prot->protParam[0]/1000 : 10;
    if (!chnProtBuf->flowIdleVolIntvlRiseBaseValid)
    {
        if (intvlSec >= intvlCfg)
        {
            chnProtBuf->flowIdleVolIntvlRiseBaseValid = True;
            chnProtBuf->flowIdleVolIntvlRiseTimeBaseSec = sysSec;
            chnProtBuf->flowIdleVolIntvlRiseVolBase = chnProtBuf->newVolCell;
        }
        return;
    }

    if (!(prot->paramEnable & 1<<1))
    {
        return;
    }

    if (chnProtBuf->newVolCell > chnProtBuf->flowIdleVolIntvlRiseVolBase)
    {
        dif = chnProtBuf->newVolCell - chnProtBuf->flowIdleVolIntvlRiseVolBase;
        if (dif > prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubVolIntvlOfst, Cc4FlowIdleVolIntvlRiseUpLmt);
        }
    }

    if (intvlSec >= intvlCfg)
    {
        chnProtBuf->flowIdleVolIntvlRiseVolBase = chnProtBuf->newVolCell;
        chnProtBuf->flowIdleVolIntvlRiseTimeBaseSec = sysSec;
    }
    return;
}

/*流程电压间隔波动,恒流工步,因子*/
void flowCcdcVolIntvlFluct0506(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    s32 dif;
    u32 intvl;
    
    if (ChnStaRun!=chn->chnStateMed)
    {
        return;
    }

    if (StepTypeCCC!=chn->stepTypeTmp && (StepTypeCCCVC!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType)
        && StepTypeCCD!=chn->stepTypeTmp && (StepTypeCCCVD!=chn->stepTypeTmp || StepSubTypeCC!=chn->stepSubType))
    {
        return;
    }

    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (!chnProtBuf->flowCcdcVolIntvlFluctBaseValid)
    {
        chnProtBuf->flowCcdcVolIntvlFluctVolBase = chnProtBuf->newVolCell;
        chnProtBuf->flowCcdcVolIntvlFluctTimeBaseMs = chn->stepRunTime;
        chnProtBuf->flowCcdcVolIntvlFluctBaseValid = True;
        return;
    }

    intvl = (prot->paramEnable & 1<<0) ? prot->protParam[0] : 10000;
    if (chn->stepRunTime - chnProtBuf->flowCcdcVolIntvlFluctTimeBaseMs < intvl)
    {
        return;
    }

    dif = AbsDifVal(chnProtBuf->newVolCell, chnProtBuf->flowCcdcVolIntvlFluctVolBase);
    if (prot->paramEnable & 1<<1)  /**/
    {
        if (dif < prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubVolIntvlOfst, Cc4FlowCcdcVolIntvlFluctLowLmt);
        }
    }

    chnProtBuf->flowCcdcVolIntvlFluctVolBase = chnProtBuf->newVolCell;
    chnProtBuf->flowCcdcVolIntvlFluctTimeBaseMs = chn->stepRunTime;
    return;
}

/*流程急停电压即电压上下限,因子*/
void flowChnCrashVol0507(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf, UpProtUnit *prot)
{
    if (!chnProtBuf->newPowerBeValid)
    {
        return;
    }

    if (prot->paramEnable & 1<<0)
    {
        if (chnProtBuf->newVolCell > prot->protParam[0])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubAllVolCrashStop, Cc4FlowCrashStopVolUpLmt);
            return;
        }
    }

    if (prot->paramEnable & 1<<1)
    {
        if (chnProtBuf->newVolCell < prot->protParam[1])
        {
            setTrayProtCause(tray, Cc0TrayTrig);
            setChnMixSubProt(chn, chnProtBuf, MixSubAllVolCrashStop, Cc4FlowCrashStopVolLowLmt);
        }
    }

    return;
}

/*温度盒掉线,和,单通道温度异常,先做温度盒掉线,todo:单通道,单库位多温度盒*/
void tmprBoxProt(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    TmprSmpl *tmprSmpl;

    tmprSmpl = &gDevMgr->tmprSmpl[tray->tmprSmplIdxBase];
    if (!tmprSmpl->online)
    {
        setTrayProtCause(tray, Cc0TmprBoxOffline);
    }

    if (tray->trayProtMgr.slotTmprCrnt.tmprBeValid > 3)
    {
        setTrayProtCause(tray, Cc0SlotTmprBad);
    }

    if (chnProtBuf->cellTmprCrnt.tmprBeValid > 3)
    {
        setChnNmlProt(chn, chnProtBuf, Cc1CellTmprBad);
    }

    return;
}

void npExprProt(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    TrayNpMgr *npMgr;

    npMgr = &tray->npMgr;
    if (0 == npMgr->refCnt)
    {
        return;
    }
    
    if (npMgr->beReach)
    {
        if (tray->ndbdData.ndbdDataValid)
        {
            if (tray->ndbdData.status[NdbdSenNpVal] > npMgr->npMax
                || tray->ndbdData.status[NdbdSenNpVal] < npMgr->npMin)
            {
                if (0 == npMgr->npOverLmtSec)
                {
                    npMgr->npOverLmtSec = sysTimeSecGet();
                }
                else
                {
                    if (sysTimeSecGet() > npMgr->npOverLmtSec+npMgr->npLmtExprSecT06)
                    {
                        setTrayProtCause(tray, Cc0StepNpOverLmt);
                    }
                }
            }
            else
            {
                npMgr->npOverLmtSec = 0;
            }
        }
    }
    else
    {
        if (sysTimeSecGet() > npMgr->npReqStartSec+npMgr->mkNpExprSecT00)
        {
            setTrayProtCause(tray, Cc0MkNpExpr);
        }
    }

    return;
}

/**/
void ndbdDevProt(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    NdbdData *ndbdData;

    if (!plcBeOnline(tray->plcIdx))
    {
        setTrayProtCause(tray, Cc0NdbdDisc);
    }

    /*并非所有针床设备告警都要中断流程,以下几项中断,另烟感单做*/
    ndbdData = &tray->ndbdData;
    if (!ndbdData->ndbdDataValid)
    {
        return;
    }

    if (ndbdData->warn[NdbdWarnGas])
    {
        setTrayProtCause(tray, Cc0TrayGas);
    }

    if (ndbdData->warn[NdbdWarnCo])
    {
        setTrayProtCause(tray, Cc0TrayCo);
    }

    if (ndbdData->warn[NdbdWarnFan])
    {
        setTrayProtCause(tray, Cc0TrayFan);
    }
}

/*电源柜数据保护*/
void chnProtWorkBox(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    UpProtUnit *prot;
    UpProtUnit *protCri;

    for (prot=(UpProtUnit *)gTmpProtGenInfo,protCri=prot+gTmpProtGenAmt; prot<protCri; prot++)
    {
        switch (prot->protId)
        {
            case 0x0200:
                chnIdleVolUpFluct0200(tray, chn, chnProtBuf, prot);
                break;
            case 0x0202:
                chnIdleCurLeak0202(tray, chn, chnProtBuf, prot);
                break;
            case 0x0203:
                chnIdleVolTtlRise0203(tray, chn, chnProtBuf, prot);
                break;
            default:
                break;
        }
    }

    for (prot=(UpProtUnit *)gTmpProtStepInfo,protCri=prot+gTmpProtStepAmt; prot<protCri; prot++)
    {
        switch (prot->protId)
        {
            case 0x0401:
                chnQuietVolFluct0401(tray, chn, chnProtBuf, prot);
                break;
            case 0x0402:
                chnQuietVolDown0402(tray, chn, chnProtBuf, prot);
                break;
            case 0x0403:
                chnQuietCurLeak0403(tray, chn, chnProtBuf, prot);
                break;
            case 0x0404:
                chnStepVolLmt0404(tray, chn, chnProtBuf, prot);
                break;
            case 0x0406:
                chnCccSetTimeVol0406(tray, chn, chnProtBuf, prot);
                break;
            case 0x0407:
                chnCcCurOfst0407(tray, chn, chnProtBuf, prot);
                break;
            case 0x0408:
                chnCccVolDownAbnm0408(tray, chn, chnProtBuf, prot);
                break;
            case 0x0409:
                chnCccVolRiseRate0409(tray, chn, chnProtBuf, prot);
                break;
            case 0x040a:
                chnCcVolBigDown040a(tray, chn, chnProtBuf, prot);
                break;
            case 0x040b:
                chnStepCapLmt040b(tray, chn, chnProtBuf, prot);
                break;
            case 0x040c:
                chnCcVolIntvlFluct040c(tray, chn, chnProtBuf, prot);
                break;
            case 0x040d:
                chnCccVolRiseCtnu040d(tray, chn, chnProtBuf, prot);
                break;
            case 0x040e:
                chnCccVolDownCtnu040e(tray, chn, chnProtBuf, prot);
                break;
            case 0x040f:
                chnCcdVolRiseAbnm040f(tray, chn, chnProtBuf, prot);
                break;
            case 0x0410:
                chnCvcVolOfst0410(tray, chn, chnProtBuf, prot);
                break;
            case 0x0411:
                chnCvcCurRiseAbnm0411(tray, chn, chnProtBuf, prot);
                break;
            case 0x0412:
                chnCvcCurBigRise0412(tray, chn, chnProtBuf, prot);
                break;
            case 0x0413:
                chnQuietVolChkBack0413(tray, chn, chnProtBuf, prot);
                break;
            case 0x0502:
                flowChgOvld0502(tray, chn, chnProtBuf, prot);
                break;
            case 0x0503:
                flowDisChgOvld0503(tray, chn, chnProtBuf, prot);
                break;
            case 0x0504:
                flowContactChk0504(tray, chn, chnProtBuf, prot);
                break;
            case 0x0505:
                flowIdleVolIntvlRise0505(tray, chn, chnProtBuf, prot);
                break;
            case 0x0506:
                flowCcdcVolIntvlFluct0506(tray, chn, chnProtBuf, prot);
                break;
            case 0x0507:
                flowChnCrashVol0507(tray, chn, chnProtBuf, prot);
                break;
            default:
                break;
        }
    }
    return;
}

/*针床数据保护*/
void chnProtWorkNdbd(Tray *tray, Channel *chn, ChnProtBuf *chnProtBuf)
{
    UpProtUnit *prot;
    UpProtUnit *protCri;
    b8 smokeChked;

    smokeChked = False;
    for (prot=(UpProtUnit *)gTmpProtGenInfo,protCri=prot+gTmpProtGenAmt; prot<protCri; prot++)
    {
        switch (prot->protId)
        {
            case 0x0100:
                trayBusyNpBigRise0100(tray, chn, chnProtBuf, prot);
                break;
            case 0x0101:
                trayBusyChnTmprBigRise0101(tray, chn, chnProtBuf, prot);
                break;
            case 0x0103:
                trayBusyTmprLowLmt0103(tray, chn, chnProtBuf, prot);
                break;
            case 0x0201:
                chnIdleTmprUpFluct0201(tray, chn, chnProtBuf, prot);
                break;
            case 0x0300:
                trayAllNpUpLmt0300(tray, chn, chnProtBuf, prot);
                break;
            case 0x0302:
                trayAllTmprUpLmt0302(tray, chn, chnProtBuf, prot);
                break;
            case 0x0303:
                trayAllSmoke0303(tray, chn, chnProtBuf, prot);
                smokeChked = True;
                break;
            default:
                break;
        }
    }

    if (!smokeChked)  /*无条件检查烟感*/
    {
        trayAllSmoke0303(tray, chn, chnProtBuf, NULL);
    }

    tmprBoxProt(tray, chn, chnProtBuf);
    npExprProt(tray, chn, chnProtBuf);
    ndbdDevProt(tray, chn, chnProtBuf);

    for (prot=(UpProtUnit *)gTmpProtStepInfo,protCri=prot+gTmpProtStepAmt; prot<protCri; prot++)
    {
        switch (prot->protId)
        {
            case 0x0405:
                chnStepChnTmprBigRise0405(tray, chn, chnProtBuf, prot);
                break;
            case 0x0501:
                flowChnTmprLmt0501(tray, chn, chnProtBuf, prot);
                break;
            default:
                break;
        }
    }
    return;
}

/*剔除过时因子*/
void mixSubRmvExpr(MixProtCfg *mixProtCfg, ChnProtBuf *chnProtBuf, u32 crntSec)
{
    u8eMixSubId idx;

    for (idx=0; idx<MixSubCri; idx++)
    {
        if (BitIsSet(chnProtBuf->mixSubHpnBitmap, idx))
        {
            if (chnProtBuf->mixSubHpnSec[idx] + mixProtCfg->lifePeriodSnd[idx] < crntSec)
            {
                BitClr(chnProtBuf->mixSubHpnBitmap, idx);
            }
        }
    }

    return;
}

/*设置策略动作*/
void mixProtPolicySet(TrayProtMgr *trayProtMgr, MixProtCfg *mixProtCfg, MixExp *mixExp, u32 crntSec)
{
    MixPolicy *policy;
    MixPolicy *policyCri;

    policy = (MixPolicy *)&mixProtCfg->policy[mixExp->policyOfst];
    for (policyCri=policy+mixExp->policyAmt; policy<policyCri; policy++)
    {
        if (!trayProtMgr->policyActNeed[policy->policyId])
        {
            trayProtMgr->policyActNeed[policy->policyId] = True;
            trayProtMgr->policyActSec[policy->policyId] = crntSec + policy->delaySec;
            if (PolicyNdbdBrk == policy->policyId)
            {
                trayProtMgr->policyNdbdBrkNeed = True;
            }
        }
    }

    return;
}

/*计算组合保护*/
void mixProtWork(Tray *tray, ChnProtBuf *chnProtBuf, u32 crntSec)
{
    MixProtCfg *mixProtCfg;
    TrayProtMgr *trayProtMgr;
    MixExp *mixExp;
    u8 idx;

    mixProtCfg = &tray->mixProtCfg;
    trayProtMgr = &tray->trayProtMgr;
    mixSubRmvExpr(mixProtCfg, chnProtBuf, crntSec);
    for (idx=0; idx<mixProtCfg->mixExpAmt; idx++)
    {
        if (trayProtMgr->mixProtHpn[idx])
        {
            continue;
        }

        mixExp = &mixProtCfg->mixExp[idx];
        if (Ok != expSuffixCalc(&mixProtCfg->suffix[mixExp->suffixOfst],
            mixExp->suffixLen, chnProtBuf->mixSubHpnBitmap, &trayProtMgr->mixProtHpn[idx])
            || !trayProtMgr->mixProtHpn[idx])
        {
            continue;
        }

        setTrayProtCause(tray, Cc4TrayMixTrag);
        CcChkModify(chnProtBuf->newCauseCode, CcMixBase+idx);
        mixProtPolicySet(trayProtMgr, mixProtCfg, mixExp, crntSec);
    }

    return;
}

void mixProtPolicyAct(Timer *timer)
{
    TrayProtMgr *trayProtMgr;
    Tray *tray;
    u32 crntSec;
    u32 delaySec;
    u8eProtPolicy policyIdx;

    trayProtMgr = Container(TrayProtMgr, protPolicyTmr, timer);
    tray = Container(Tray, trayProtMgr, trayProtMgr);
    crntSec = sysTimeSecGet();

    /*先做到时间的*/
    for (delaySec=0,policyIdx=0; policyIdx<PolicyCri; policyIdx++)
    {
        if (trayProtMgr->policyActNeed[policyIdx] && !trayProtMgr->policyActOver[policyIdx])
        {
            if (trayProtMgr->policyActSec[policyIdx] <= crntSec)
            {
                trayProtMgr->policyActOver[policyIdx] = True;
                if (PolicyNdbdBrk == policyIdx)
                {
                    plcRegWriteTry(tray->trayIdx, NdbdSetTouch, 2);
                }
                else if (PolicyFireDoorClose == policyIdx)
                {
                    plcRegWriteTry(tray->trayIdx, NdbdSetFireDoor, 2);
                }
                else if (PolicyFanStopTray == policyIdx)
                {
                    plcRegWriteTry(tray->trayIdx, NdbdSetFan, 0);
                }
                else if (PolicySmokeRemove == policyIdx)
                {
                    plcRegWriteTry(tray->trayIdx, NdbdSetFan, 1);
                }
            }
            else
            {
                if (0 == delaySec)
                {
                    delaySec = trayProtMgr->policyActSec[policyIdx] - crntSec;
                }
                else
                {
                    delaySec = Miner(delaySec, (trayProtMgr->policyActSec[policyIdx]-crntSec));
                }
            }
        }
    }

    /*再做未到时间的,如果定时器在转就等下次或超时*/
    /*定时器在转的情况例如,上轮触发组合,本轮又触发不同组合*/
    if (0!=delaySec && !TimerIsRun(&trayProtMgr->protPolicyTmr))
    {
        timerStart(&trayProtMgr->protPolicyTmr, TidMixProtPolicyAct, delaySec*1000, WiReset);
    }

    return;
}

/*三个异常码,下位机异常码,通道异常码,消防(整盘)异常码*/
/*先记录下位机异常码*/
/*其次执行保护逻辑,产生通道异常码,可覆盖下位机异常码*/
/*若产生托盘级保护,若由托盘引发则所有通道相同*/
/*若由通道产生则该通道异常码异于托盘内其它通道异常码*/
/*中位机需要理解的下位机异常码有:0无状况,1~5正常截止,7用户截止*/
/*一些特殊情况如下,需要分别处理,部分逻辑可能存在不完整*/
/*通道所在box不在线*/
/*通道所在box量程不匹配*/
/*做保护时通道没有数据,为插入临时采样(上位机会忽略)*/
/*通道所在box在修调*/
/*托盘跑工装(其中校准工装与修调重叠)*/
void trayProtWork(Tray *tray, b8 wiNdbd)
{
    TrayProtMgr *trayProtMgr;
    ChnProtBuf *chnProtBuf;
    Channel *chn;
    Channel *chnCri;
    Cell *cell;
    Cell *cellCri;
    Box *box;
    Box *boxCri;
    TraySmpl *traySmpl;
    TrayChnSmpl *trayChnSmpl;
    u32 crntSec;
    b8 hasRunChn;

    trayProtMgr = &tray->trayProtMgr;
    trayProtMgr->trayCauseCode = CcNone;
    trayProtMgr->policyNdbdBrkNeed = False;

    /*先获取时间戳,否则本轮产生且周期为零的因子可能参与不了表达式*/
    crntSec = sysTimeSecGet();
    for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
    {
        if (BoxTypeParallel == tray->boxCfg.boxType)
        {
            chnProtBuf = &chn->chnProtBuf;
            if (wiNdbd)
            {
                chnProtWorkNdbd(tray, chn, chnProtBuf);
            }
            chnProtWorkBox(tray, chn, chnProtBuf);
            mixProtWork(tray, chnProtBuf, crntSec);
        }
        else
        {
            for (cell=chn->series->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
            {
                chnProtBuf = &cell->chnProtBuf;
                if (wiNdbd)
                {
                    chnProtWorkNdbd(tray, chn, chnProtBuf);
                }
                chnProtWorkBox(tray, chn, chnProtBuf);
                mixProtWork(tray, chnProtBuf, crntSec);
            }
        }
    }

    /*保护逻辑完成后的处理,todo,也可另建函数*/
    /*如果有托盘级保护,修正通道异常码*/
    if (CcNone != trayProtMgr->trayCauseCode)
    {
        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (CcBaseGet(trayProtMgr->trayCauseCode) > chn->chnProtBuf.preCauseCode)
            {
                CcChkModify(chn->chnProtBuf.newCauseCode, trayProtMgr->trayCauseCode);
            }

            if (NULL == chn->series)
            {
                continue;
            }

            for (cell=chn->series->cell,cellCri=&chn->series->cell[chn->chnCellAmt]; cell<cellCri; cell++)
            {
                if (CcBaseGet(trayProtMgr->trayCauseCode) > cell->chnProtBuf.preCauseCode)
                {
                    CcChkModify(cell->chnProtBuf.newCauseCode, trayProtMgr->trayCauseCode);
                }
            }
        }
    }
    else if (BoxTypeParallel != tray->boxCfg.boxType)
    {
        for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
        {
            if (CcNone != chn->chnProtBuf.newCauseCode)
            {
                for (cell=chn->series->cell,cellCri=&chn->series->cell[chn->chnCellAmt]; cell<cellCri; cell++)
                {
                    if (CcBaseGet(chn->chnProtBuf.newCauseCode) > cell->chnProtBuf.preCauseCode)
                    {
                        CcChkModify(cell->chnProtBuf.newCauseCode, chn->chnProtBuf.newCauseCode);
                    }
                }
            }
        }
    }

    /*修正通道状态,确定是否下发停止,确定上报采样异常码*/
    traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
    for (chn=tray->chn,chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
    {
        if (CcNone == chn->chnProtBuf.newCauseCode)
        {
            continue;
        }

        if (ChnStaRun==chn->chnStateMed || ChnStaStartReqLow==chn->chnStateMed
            || ChnStaStartReqMed==chn->chnStateMed || ChnStaFlowStart==chn->chnStateMed)
        {
            chn->chnStateMed = StepProtPause==gTmpStepProtPolicy ? ChnStaPauseReqMed : ChnStaStopReqMed;
            chn->dynStaCnt = 0;
            boxCtrlAddChn(chn->box, chn->chnIdxInBox, ChnStop);
            trayNpChnFree(tray, chn);
        }
        else if (ChnStaNp == chn->chnStateMed)
        {
            chn->chnStateMed = StepProtPause==gTmpStepProtPolicy ? ChnStaPause : ChnStaStop;
            chnEnterIdle(chn);
            trayNpChnFree(tray, chn);
        }
        else if (ChnStaStopReqUp == chn->chnStateMed || ChnStaPauseReqUp == chn->chnStateMed)
        {
            chn->chnStateMed = StepProtPause==gTmpStepProtPolicy ? ChnStaPauseReqMed : ChnStaStopReqMed;
        }

        trayChnSmpl = &traySmpl->chnSmpl[chn->genIdxInTray];
        CcChkModify(trayChnSmpl->causeCode, chn->chnProtBuf.newCauseCode);
        if (NULL == chn->series)
        {
            continue;
        }

        for (cell=chn->series->cell,cellCri=&chn->series->cell[chn->chnCellAmt]; cell<cellCri; cell++)
        {
            trayChnSmpl = &traySmpl->chnSmpl[cell->genIdxInTray];
            CcChkModify(trayChnSmpl->causeCode, cell->chnProtBuf.newCauseCode);
        }
    }

    /*如果有停止下发,表明托盘忙且有异常*/
    for (hasRunChn=False,box=tray->box,boxCri=&tray->box[tray->boxAmt]; box<boxCri; box++)
    {
        if (0 != box->ctrlWaitSend.boxCtrlInd.chnSelectInd)
        {
            boxCtrlTxTry(box);
            tray->trayWarnPres = True;
            hasRunChn = True;
        }
    }

    /*最后执行组合保护策略动作风扇消防门等*/
    if (Cc4TrayMixTrag != trayProtMgr->trayCauseCode)
    {
        return;
    }

    /*脱开针床要注意忙闲和负压*/
    if (trayProtMgr->policyNdbdBrkNeed)
    {
        if (tray->ndbdData.status[NdbdSenNpVal] < -200) /*破真空1秒*/
        {
            trayProtMgr->policyActSec[PolicyNdbdBrk] += 1;
            trayNpRatioSet(tray->trayIdx, NpOprRatioBrkVacum, 0, 0);
        }
        else if (hasRunChn) /*前面已停,这里只延时*/
        {
            trayProtMgr->policyActSec[PolicyNdbdBrk] += 1;
        }
    }

    mixProtPolicyAct(&trayProtMgr->protPolicyTmr);
    return;
}

#if 0
int main()
{
    MemDesc desc[] = { {64, 64}, {128, 32}, {256, 32}, {512, 16} };
    s8 *src = "1-4,7,9-16";
    u8 dst[128];
    u8 amt;
    u8 *infix = "(3|4|5)&6&9&12|13|10&7";
    u8 infix1[] = "(4|4|5)&6&9&12|13|10&7@2=6000,3,5=500";
    u8 infix2[] = "(3|5|9)&6&10&12|15|10&7@";
    u8 infix3[] = "(3|5|9)&6&10&12|15|10&7@6=300000";
    u8 suffix[128];
    u32 elemVal;
    u8 suffixLen;
    u8 result;
    MixProtCfg mixProtCfg;
    MixExp *item;
    u8 *suffix1;
    MixPolicy *policy;
    u8 idx;

    funcInit(desc, sizeof desc/sizeof *desc);
    logInit();
#if 0
    if (Ok == arrayParse(src, dst, 32, &amt))
    {
        printf("amt:%d\r\n", amt);
        outHex(dst, amt);
    }
#endif
#if 0
    //elemVal[4] = elemVal[6] = elemVal[9] = elemVal[12] = 1;
    if (Ok == expInfix2Suffix(infix, infix+strlen(infix), 32, suffix, suffix+128, &suffixLen)
        && Ok == expSuffixCalc(suffix, suffixLen, elemVal, &result))
    {
        printf("infix:%s\r\n", infix);
        printf("suffixLen:%d, result:%d\r\n", suffixLen, result);
        for (amt=0; amt<suffixLen; amt++)
        {
            if (suffix[amt] > 128)
            {
                printf("%2c ", suffix[amt]-128);
            }
            else
            {
                printf("%2d ", suffix[amt]);
            }
        }
        printf("\r\n");
    }
#endif
#if 1
    mixProtCfgReset(&mixProtCfg);
    if (Ok == protExpParse(infix1, strlen(infix1), &mixProtCfg)
        && Ok == protExpParse(infix2, strlen(infix2), &mixProtCfg)
        && Ok == protExpParse(infix3, strlen(infix3), &mixProtCfg))
    {
        printf("protect amt:%d\r\n", mixProtCfg.mixExpAmt);
        for (idx=0; idx<3; idx++)
        {
            item = &mixProtCfg.mixExp[idx];
            printf("item[%d] policy amt:%d\r\n", idx, item->policyAmt);
            suffix1 = &mixProtCfg.suffix[item->suffixOfst];
            for (amt=0; amt<item->suffixLen; amt++)
            {
                if (suffix1[amt] > 128)
                {
                    printf("%2c ", suffix1[amt]-128);
                }
                else
                {
                    printf("%2d ", suffix1[amt]);
                }
            }
            printf("\r\n");
            policy = (MixPolicy *)&mixProtCfg.policy[item->policyOfst];
            for (amt=0; amt<item->policyAmt; amt++,policy++)
            {
                printf("p:%d,t:%d    ", policy->policyId, policy->delaySec);
            }
            printf("\r\n");
        }
    }
    else
    {
        printf("error\r\n");
    }
#endif
    return 0;
}
#endif



