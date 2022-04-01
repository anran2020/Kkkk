


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"
#include "log.h"
#include "timer.h"
#include "func.h"
#include "entry.h"
#include "flow.h"
#include "protect.h"
#include "channel.h"
#include "box.h"
#include "host.h"
#include "uart.h"
#include "plc.h"

#ifdef DebugVersion
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <sys/time.h>
#endif

DevMgr *gDevMgr;
u16 gProtoSoftVer = 1;
u16 gMediumSoftVer = 6;

/*临时量，以后替换为配置*/
#if 0
#if 1
u8eBoxType gTrayBoxType = BoxTypeParallel;
u8 gTmpTrayAmt = 2;
u8 gTmpTrayBoxAmt = 4;
u8 gTmpBoxChnAmt = 8;
u8 gTmpChnCellAmt = 0;
u8 gChnModuAmt = 1;
u8 gBoxAddr[] = { 0,1,2,3,4,5,6,7 }; /*所有box的addr都必须列出*/
u8 gCan0BoxIdx[] = { 0,1,2,3 }; /*dev-box-idx*/
u32 gVoltageMax = 500000;
u32 gCurrentMax = 12000000;
u16 gTmpTmprSmplCellAmt = 32; /*用于电芯的通道数量,不含预留*/
u8 gBoxVolSmplBoardAmt = 0; /*电压采样板数量,目前极简串联有*/
#else
u8eBoxType gTrayBoxType = BoxTypeParallel;
u8 gTmpTrayAmt = 2;
u8 gTmpTrayBoxAmt = 4;
u8 gTmpBoxChnAmt = 6;
u8 gTmpChnCellAmt = 0;
u8 gChnModuAmt = 2;
u8 gBoxAddr[] = { 0,1,2,3,4,5,6,7,8,9,10,11 }; /*所有box的addr都必须列出*/
u8 gCan0BoxIdx[] = { 0,1,2,3,4,5 }; /*dev-box-idx*/
u32 gVoltageMax = 500000;
u32 gCurrentMax = 20000000;
u16 gTmpTmprSmplCellAmt = 24; /*用于电芯的通道数量,不含预留*/
u8 gBoxVolSmplBoardAmt = 0; /*电压采样板数量,目前极简串联有*/
#endif
#else
#if 0
u8eBoxType gTrayBoxType = BoxTypeSeriesWoSw;
u8 gTmpTrayAmt = 2;
u8 gTmpTrayBoxAmt = 2;
u8 gTmpBoxChnAmt = 1;
u8 gTmpChnCellAmt = 12;
u8 gChnModuAmt = 2;
u8 gBoxAddr[] = { 0,1,2,3 }; /*所有box的addr都必须列出*/
u8 gCan0BoxIdx[] = { 0,1 }; /*dev-box-idx*/
u32 gVoltageMax = 6000000;
u32 gCurrentMax = 20000000;
u16 gTmpTmprSmplCellAmt = 24; /*用于电芯的通道数量,不含预留*/
u8 gBoxVolSmplBoardAmt = 1; /*单箱电压采样板数量,目前极简串联有*/
#else
u8eBoxType gTrayBoxType = BoxTypeSeriesWoSw;
u8 gTmpTrayAmt = 2;
u8 gTmpTrayBoxAmt = 4;
u8 gTmpBoxChnAmt = 1;
u8 gTmpChnCellAmt = 8;
u8 gChnModuAmt = 1;
u8 gBoxAddr[32] = { 0,1,2,3,4,5,6,7 }; /*所有box的addr都必须列出*/
u8 gCAN0BoxNum=4;
u8 gCan0BoxIdx[32] = { 0,1,2,3 }; /*dev-box-idx*/

u32 gVoltageMax = 6000000;
u32 gCurrentMax = 12000000;
u16 gTmpTmprSmplCellAmt = 32; /*温度板用于电芯的通道数量,不含预留*/
u8 gBoxVolSmplBoardAmt = 1; /*单box电压采样板数量,目前极简串联有*/
#endif
#endif

u8 gTmpFixtUartIdx = 0;

u8 gTmpTmprSmplAmt = 2;  /*温度采样板数量*/
u8 gTmprSmplAddr[] = { 0,1 }; /*所有的addr都必须列出*/
u16 gTmpTmprSmplChnAmt = 52;  /*温度盒通道总数量,含预留*/
//u16 gTmpTmprSmplCellAmt = 32; /*用于电芯的通道数量,不含预留*/
u8 gTmpTmprSmplCellBase = 0;
u8 gTmpTmprSmplLocAmt = 2;  /*用于库温的通道数量,不含预留*/
u8 gTmpTmprSmplLocBase = 48;
u8 gTmprAmtPerCell = 1;  /*每个电芯的温度个数*/
u8 gTmprAmtPerSlot = 2;  /*每个库位的库温个数*/
u8 gTmpTmprSmplUartIdx = 0;

b8 gIsTraySmpl = 1;
u32 gChnSmplBufAmt = 512;
u32 gTraySmplBufAmt = 16;

u8 gBoxBypsSwBoardAmt = 0;  /*旁路切换板数量,目前旁路串联有*/
int mainInit()
{
    DevMgr *devMgr;
    Tray *tray;
    Cell *cell;
    Channel *chn;
    ChnMap *chnMap;
    Box *box;
    Can *can;
    Uart *uart;
    TmprSmpl *tmprSmpl;
    NdbdData *ndbdData;
    SmplSaveMgr *smplMgr;
    u16 idx;
    u16 idx0;
    u8 addr;

    u8 trayAmt;
    u8 boxAmtTtl;
    u16 chnAmtTtl;
    u16 cellAmtTtl;
    u16 tmprAmtTtl;  /*电池个数*/
    u8 canAmt;
    u8 uartAmt;
    u16 subBoxAmt;
    u16 genChnAmtTtl;

    trayAmt = gTmpTrayAmt;
    boxAmtTtl = trayAmt * gTmpTrayBoxAmt;
    chnAmtTtl = boxAmtTtl * gTmpBoxChnAmt;
    if (BoxTypeParallel == gTrayBoxType)
    {
        cellAmtTtl = 0;
        genChnAmtTtl = 0;
        tmprAmtTtl = chnAmtTtl;
    }
    else
    {
        cellAmtTtl = chnAmtTtl * gTmpChnCellAmt;
        genChnAmtTtl = chnAmtTtl + cellAmtTtl;
        tmprAmtTtl = cellAmtTtl;
    }

    uartAmt = 2;
    canAmt = 1;
    if (boxAmtTtl > sizeof(gCan0BoxIdx))
    {
        canAmt = 2;
    }

    gDevMgr = devMgr = sysMemAlloc(sizeof(DevMgr));
    devMgr->chnStaMapMed2Up[ChnStaIdle] = ChnUpStateIdle;
    devMgr->chnStaMapMed2Up[ChnStaStop] = ChnUpStateStop;
    devMgr->chnStaMapMed2Up[ChnStaPause] = ChnUpStatePause;
    devMgr->chnStaMapMed2Up[ChnStaNp] = ChnUpStateNp;
    devMgr->chnStaMapMed2Up[ChnStaRun] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaFlowStart] = ChnUpStateStart;
    devMgr->chnStaMapMed2Up[ChnStaStartReqLow] = ChnUpStateStart;
    devMgr->chnStaMapMed2Up[ChnStaStartReqMed] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaStopReqUp] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaPauseReqUp] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaStopReqMed] = ChnUpStateStop;
    devMgr->chnStaMapMed2Up[ChnStaPauseReqMed] = ChnUpStatePause;
    devMgr->trayAmt = gTmpTrayAmt;
    devMgr->canAmt = canAmt;
    devMgr->uartAmt = uartAmt;
    devMgr->isTraySmpl = gIsTraySmpl;
    devMgr->needConnUp = True;
    devMgr->cellTmprAmt = tmprAmtTtl * gTmprAmtPerCell;
    devMgr->slotTmprAmt = gTmpTrayAmt * gTmprAmtPerSlot;
    devMgr->fixtUartIdx = gTmpFixtUartIdx;
    devMgr->tmprSmplUartIdx = gTmpTmprSmplUartIdx;
    devMgr->ndbdUartIdx = gTmpTmprSmplUartIdx;
    devMgr->tray = sysMemAlloc(sizeof(Tray) * trayAmt);
    devMgr->box = sysMemAlloc(sizeof(Box) * boxAmtTtl);
    devMgr->chn = sysMemAlloc(sizeof(Channel) * chnAmtTtl);
    devMgr->can = sysMemAlloc(sizeof(Can) * canAmt);
    devMgr->uart = sysMemAlloc(sizeof(Uart) * uartAmt);
    devMgr->genCellTmpr = sysMemAlloc(Align16(devMgr->cellTmprAmt)*sizeof(u16));
    for (idx=0; idx<devMgr->cellTmprAmt; idx++)
    {
        devMgr->genCellTmpr[idx] = 0; /*温度盒不在线,给0*/
    }
    devMgr->genSlotTmpr = sysMemAlloc(Align16(devMgr->slotTmprAmt)*sizeof(u16));
    for (idx=0; idx<devMgr->slotTmprAmt; idx++)
    {
        devMgr->genSlotTmpr[idx] = 0; /*温度盒不在线,给0*/
    }

    if (BoxTypeParallel == gTrayBoxType)
    {
        devMgr->cell = NULL;
        devMgr->chnMap = NULL;
    }
    else
    {
        devMgr->cell = sysMemAlloc(sizeof(Cell) * cellAmtTtl);
        devMgr->chnMap = sysMemAlloc(sizeof(ChnMap) * genChnAmtTtl);
    }

    devMgr->tmprSmpl = NULL;
    if (0 != gTmpTmprSmplAmt)
    {
        devMgr->tmprSmpl = sysMemAlloc(sizeof(TmprSmpl) * gTmpTmprSmplAmt);
    }

    for (idx=0; idx<trayAmt; idx++)
    {
        tray = &devMgr->tray[idx];
        tray->boxCfg.maxTrayVol = gVoltageMax;
        tray->boxCfg.maxTrayCur = gCurrentMax;
        tray->boxCfg.boxType = gTrayBoxType;
        tray->boxCfg.chnAmtConn = gTmpBoxChnAmt;
        tray->boxCfg.chnModuAmt = gChnModuAmt;
        tray->boxCfg.bypsSwAmt = gBoxBypsSwBoardAmt;
        tray->boxCfg.volSmplAmt = gBoxVolSmplBoardAmt;
        tray->boxCfg.protoVerMin = 1;
        tray->boxCfg.chnCellAmt = gTmpChnCellAmt;
        tray->trayIdx = idx;
        tray->boxAmt = gTmpTrayBoxAmt;
        tray->trayWorkMode = BoxModeManu;
        tray->protEnable = False;
        TimerInit(&tray->protEnaTmr);
        TimerInit(&tray->npSwProtDelayTmr);
        TimerInit(&tray->trayProtMgr.protPolicyTmr);
        tray->trayWarnPres = False;
        subBoxAmt = gTmpTrayBoxAmt * gBoxVolSmplBoardAmt;
        tray->volSmpl = sysMemAlloc(subBoxAmt * sizeof(SubBox));
        for (idx0=0; idx0<subBoxAmt; idx0++)
        {
            tray->volSmpl[idx0].idxInTray = idx0;
            tray->volSmpl[idx0].softVer = 0;
            tray->volSmpl[idx0].online = False;
        }
        subBoxAmt = gTmpTrayBoxAmt * gBoxBypsSwBoardAmt;
        tray->bypsSw = sysMemAlloc(subBoxAmt * sizeof(SubBox));
        for (idx0=0; idx0<subBoxAmt; idx0++)
        {
            tray->bypsSw[idx0].idxInTray = idx0;
            tray->bypsSw[idx0].softVer = 0;
            tray->bypsSw[idx0].online = False;
        }
        tray->box = &devMgr->box[idx * gTmpTrayBoxAmt];
        tray->trayChnAmt = tray->boxAmt * gTmpBoxChnAmt;
        tray->chn = &devMgr->chn[idx*tray->trayChnAmt];
        if (BoxTypeParallel == tray->boxCfg.boxType)
        {
            tray->trayCellAmt = tray->trayChnAmt;
            tray->genChnAmt = tray->trayChnAmt;
            tray->cell = NULL;
            tray->chnMap = NULL;
        }
        else
        {
            tray->trayCellAmt = tray->trayChnAmt * gTmpChnCellAmt;
            tray->genChnAmt = tray->trayChnAmt + tray->trayCellAmt;
            tray->cell = &devMgr->cell[idx*tray->trayCellAmt];
            tray->chnMap = &devMgr->chnMap[idx*tray->genChnAmt];
        }

        ndbdData = &tray->ndbdData;
        ndbdData->tmprAmtPerCell = gTmprAmtPerCell;
        ndbdData->slotTmprAmt = gTmprAmtPerSlot;
        ndbdData->cellTmpr = &devMgr->genCellTmpr[idx*tray->trayCellAmt*gTmprAmtPerCell];
        ndbdData->slotTmpr = &devMgr->genSlotTmpr[idx * ndbdData->slotTmprAmt];
        ndbdData->bit2IdxSta[BitSenFwdEnter] = NdbdSenFwdEnter;
        ndbdData->bit2IdxSta[BitSenBackEnter] = NdbdSenBackEnter;
        ndbdData->bit2IdxSta[BitSenFwdTouch] = NdbdSenFowTouch;
        ndbdData->bit2IdxSta[BitSenBackTouch] = NdbdSenBackTouch;
        ndbdData->bit2IdxSta[BitSenWorkMode] = NdbdStaWorkMode;
        ndbdData->bit2IdxSta[BitSenSwValve] = NdbdSenSwValve;
        ndbdData->bit2IdxSta[BitSenBrkVacum] = NdbdSenBrkVacum;
        ndbdData->bit2IdxSta[BitSenFireDoorUp] = NdbdSenFireDoorUp;
        ndbdData->bit2IdxSta[BitSenFireDoorDown] = NdbdSenFireDoorDown;
        ndbdData->bit2IdxSta[BitSenFwdDoorMsw] = NdbdSenFwdDoorMsw;
        ndbdData->bit2IdxSta[BitSenBackDoor] = NdbdSenBackDoor;
        ndbdData->bit2IdxSta[BitSenCylinderUp] = NdbdSenCylinderUp;
        ndbdData->bit2IdxSta[BitSenCylinderDown] = NdbdSenCylinderDown;
        ndbdData->bit2IdxWarn[BitWarnScram] = NdbdWarnScram;
        ndbdData->bit2IdxWarn[BitWarnGas] = NdbdWarnGas;
        ndbdData->bit2IdxWarn[BitWarnCo] = NdbdWarnCo;
        ndbdData->bit2IdxWarn[BitWarnFan] = NdbdWarnFan;
        ndbdData->bit2IdxWarn[BitWarnFireDoor] = NdbdWarnFireDoor;
        ndbdData->bit2IdxWarn[BitWarnSlotDoorRast] = NdbdWarnSlotDoorRast;
        ndbdData->bit2IdxWarn[BitWarnTray] = NdbdWarnTray;
        ndbdData->bit2IdxWarn[BitWarnFwdDoorMsw] = NdbdWarnFwdDoorMsw;
        ndbdData->bit2IdxWarn[BitWarnBackDoor] = NdbdWarnBackDoor;
        ndbdData->bit2IdxWarn[BitWarnRatio] = NdbdWarnRatio;
        ndbdData->bit2IdxWarn[BitWarnScapeGoat] = NdbdWarnScapeGoat;
        ndbdData->trayIsTouch = False;
        ndbdData->trayIsEnter = False;
        ndbdData->ndbdDataValid = False;

        smplMgr = &tray->smplMgr;
        smplMgr->isLooped = False;
        smplMgr->smplGenMore = False;
        smplMgr->smplTryBoxAmt = 0;
        if (devMgr->isTraySmpl)
        {
            smplMgr->smplItemSize = sizeof(TraySmplRcd) + sizeof(TraySmpl)
                + sizeof(TrayChnSmpl)*tray->genChnAmt + sizeof(TraySmpl) + sizeof(TrayNdbdSmpl)
                + sizeof(u16)*(Align16(gTmprAmtPerSlot)+Align16(tray->trayCellAmt*gTmprAmtPerCell));
            smplMgr->smplDiskAmt = gTraySmplBufAmt;
            smplMgr->smplBufAmt = gTraySmplBufAmt;
        }
        else
        {
            smplMgr->smplItemSize = sizeof(UpChnlSmpl) + sizeof(u16)*Align16(gTmprAmtPerSlot);
            smplMgr->smplDiskAmt = gChnSmplBufAmt;
            smplMgr->smplBufAmt = gChnSmplBufAmt;
        }
        smplMgr->smplSeqNext = 0;
        smplMgr->smplSeqUpReq = 0;
        smplMgr->smplSeqMax = smplMgr->smplDiskAmt - 1;
        smplMgr->smplItemDiskSize = smplMgr->smplItemSize;
        smplMgr->smplDiskAddrBase = 0;
        smplMgr->smplDiskAddr = 0;
        smplMgr->smplDiskAddrMax = 0;
        smplMgr->smplBufAddrBase = sysMemAlloc(smplMgr->smplBufAmt * smplMgr->smplItemSize);
        smplMgr->smplBufAddr = smplMgr->smplBufAddrBase;
        smplMgr->smplBufAddrMax = smplMgr->smplBufAddrBase + (smplMgr->smplBufAmt-1)*smplMgr->smplItemSize;

        mixProtCfgReset(&tray->mixProtCfg);
        tray->trayProtMgr.slotTmprCrnt.tmprBeValid = False;
        tray->trayProtMgr.slotTmprPre.tmprBeValid = False;
        tray->trayProtMgr.slotTmprCrnt.tmprInvalidCnt = 0;
        tray->trayProtMgr.preNdbdValid = False;
        tray->trayProtMgr.npWiSw = False;
        memset(tray->trayProtMgr.mixProtHpn, 0, sizeof(MixExpAmt));
        memset(tray->trayProtMgr.policyActNeed, 0, Align8(PolicyCri));
        memset(tray->trayProtMgr.policyActOver, 0, Align8(PolicyCri));

        /*以下映射只为联机上报,其余用不到*/
        tray->tmprSmplAmt = 0;
        tray->tmprSmplIdxBase = 0;
        if (0 != gTmpTmprSmplAmt)
        {
            u16 diff;
            u16 leftTmprChn;

            idx0 = tray->trayCellAmt * gTmprAmtPerCell * tray->trayIdx;
            tray->tmprSmplIdxBase = idx0 / gTmpTmprSmplCellAmt;
            leftTmprChn = tray->trayCellAmt * gTmprAmtPerCell;
            if (0 != idx0 % gTmpTmprSmplCellAmt)
            {
                tray->tmprSmplAmt += 1;
                diff = gTmpTmprSmplCellAmt - idx0%gTmpTmprSmplCellAmt;
                if (leftTmprChn > diff)
                {
                    leftTmprChn -= diff;
                }
                else
                {
                    leftTmprChn = 0;
                }
            }

            tray->tmprSmplAmt += leftTmprChn / gTmpTmprSmplCellAmt;
            if (0 != leftTmprChn % gTmpTmprSmplCellAmt)
            {
                tray->tmprSmplAmt += 1;
            }
        }

        tray->updTypeMap[UpUpdDevBox].cmmuItfType = ItfTypeCan;
        if (BoxTypeParallel == tray->boxCfg.boxType)
        {
            tray->updTypeMap[UpUpdDevBox].devType = BoxDevParallel;
        }
        else if (BoxTypeSeriesWiSw == tray->boxCfg.boxType)
        {
            tray->updTypeMap[UpUpdDevBox].devType = BoxDevSeriesWiSw;
        }
        else
        {
            tray->updTypeMap[UpUpdDevBox].devType = BoxDevSeriesWoSw;
        }
        tray->updTypeMap[UpUpdDevNdbd].cmmuItfType = ItfTypeUart;  /*todo,PLC时不升级*/
        tray->updTypeMap[UpUpdDevNdbd].devType = UartNdbdCtrl;
        tray->updTypeMap[UpUpdDevTmpr].cmmuItfType = ItfTypeUart;
        tray->updTypeMap[UpUpdDevTmpr].devType = UartTmprSmpl;
        tray->updTypeMap[UpUpdDevVolResa].cmmuItfType = ItfTypeCan;
        tray->updTypeMap[UpUpdDevVolResa].devType = BoxDevVolSmplResa;
        tray->updTypeMap[UpUpdDevSw].cmmuItfType = ItfTypeCan;
        tray->updTypeMap[UpUpdDevSw].devType = BoxDevSwitch;
        tray->updTypeMap[UpUpdDevVolDsp].cmmuItfType = ItfTypeCan;
        tray->updTypeMap[UpUpdDevVolDsp].devType = BoxDevVolSmplDsp;
    }

    for (idx=0; idx<canAmt; idx++)
    {
        can = &devMgr->can[idx];
        can->canIdx = idx;
        can->canMsgFlowSeq = 0;
        can->boxAmt = 0; /*解析box时候确定*/
        for (addr=0; addr<CanAddrMaxCri; addr++)
        {
            can->addr2DevBoxIdx[addr] = 0xff;
        }

        TimerInit(&can->waitCanAckTmr);
        ListInitD(&can->ctrlWaitList);
        can->smplCanBoxIdx = can->boxAmt;
        ListInitD(&can->auxWaitList);
        can->waitAckAuxCmd = NULL;
        TimerInit(&can->canSmplLoopTmr);
    }

    for (idx=0; idx<boxAmtTtl; idx++)
    {
        box = &devMgr->box[idx];
        box->tray = &devMgr->tray[idx/gTmpTrayBoxAmt];
        box->boxIdxInTray = idx % gTmpTrayBoxAmt;
        box->addr = gBoxAddr[idx];
        box->boxType = box->tray->boxCfg.boxType;
        box->online = False;
        box->reTxCmd = False;
        box->boxHasSmplTry = False;
        box->boxHasSmplMore = False;
        box->chnAmtTtl = box->tray->boxCfg.chnAmtConn;
        box->maxCur = box->tray->boxCfg.maxTrayCur;
        box->maxVol = box->tray->boxCfg.maxTrayVol;
        box->bypsSwAmt = box->tray->boxCfg.bypsSwAmt;
        box->volSmplAmt = box->tray->boxCfg.volSmplAmt;
        box->softVer = 0;
        box->chnModuAmt = box->tray->boxCfg.chnModuAmt;
        box->boxWorkMode = BoxModeManu;
        for (idx0=0; idx0<sizeof(gCan0BoxIdx) && idx!=gCan0BoxIdx[idx0]; idx0++);
        if (idx0 < sizeof(gCan0BoxIdx))
        {
            box->canIdx = 0;
            devMgr->can[0].addr2DevBoxIdx[gBoxAddr[idx]] = idx;
            devMgr->can[0].can2DevBoxIdx[devMgr->can[0].boxAmt++] = idx;
        }
        else
        {
            box->canIdx = 1;
            devMgr->can[1].addr2DevBoxIdx[gBoxAddr[idx]] = idx;
            devMgr->can[1].can2DevBoxIdx[devMgr->can[1].boxAmt++] = idx;
        }
        box->boxChnAmt = gTmpBoxChnAmt;
        box->boxCellAmt = box->boxChnAmt;
        if (BoxTypeParallel != box->boxType)
        {
            box->boxCellAmt *= gTmpChnCellAmt;
        }
        box->chn = &devMgr->chn[idx*box->boxChnAmt];
        ChainInitD(&box->ctrlWaitSend.chain);
    }

    for (idx=0; idx<canAmt; idx++)
    {
        /*确保--定时器超时是触发采样的必要条件*/
        can = &devMgr->can[idx];
        can->smplCanBoxIdx = can->boxAmt;
    }

    /*确定can上的box分布后，再计算tray上的can数量*/
    for (idx=0; idx<trayAmt; idx++)
    {
        tray = &devMgr->tray[idx];
        tray->canAmt = 1;
        for (idx0=1; idx0<tray->boxAmt; idx0++)
        {
            if (tray->box->canIdx != tray->box[idx0].canIdx)
            {
                tray->canAmt = 2;
                break;
            }
        }
    }

    for (idx=0; idx<chnAmtTtl; idx++)
    {
        chn = &devMgr->chn[idx];
        chn->chnIdxInTray = idx % (gTmpTrayBoxAmt*gTmpBoxChnAmt);
        chn->chnIdxInBox = idx % gTmpBoxChnAmt;
        chn->box = &devMgr->box[idx/gTmpBoxChnAmt];
        chn->chnProtBuf.cellTmprPre.tmprBeValid = False;
        chn->chnProtBuf.cellTmprCrnt.tmprBeValid = False;
        chn->chnProtBuf.cellTmprCrnt.tmprInvalidCnt = 0;
        chn->chnProtBuf.newPowerBeValid = False;
        chn->chnProtBuf.prePowerBeValid = False;
        chn->chnProtBuf.newCauseCode = CcNone;
        chn->chnProtBuf.preCauseCode = CcNone;
        chn->chnProtBuf.mixSubHpnBitmap = 0;
        chn->chnStateMed = ChnStaIdle;
        chn->dynStaCnt = 0;
        chn->chnCellAmt = 0;
        chn->series = NULL;
        chn->smplSeqInStep = 0;
        chn->capCtnu = 0;
        chn->capSum = 0;
        chn->stepRunTime = 0;
        chn->stepRunTimeBase = 0;
        chn->smplAmtInLoop = 0;
        if (BoxTypeParallel != chn->box->boxType)
        {
            chn->chnCellAmt = gTmpChnCellAmt;
            if (BoxTypeSeriesWoSw == chn->box->boxType)
            {
                chn->series = sysMemAlloc(sizeof(ChnSeries) + sizeof(ChnWoSw));
            }
            else /*if (BoxTypeSeriesWiSw == chn->box->boxType)*/
            {
                chn->series = sysMemAlloc(sizeof(ChnSeries) + sizeof(ChnWiSw));
            }

            chn->series->cell = &devMgr->cell[idx*gTmpChnCellAmt];
        }
        chn->genIdxInTray = chn->chnIdxInTray * (chn->chnCellAmt+1);
        chn->genIdxInBox = chn->chnIdxInBox * (chn->chnCellAmt+1);
    }

    if (BoxTypeParallel != gTrayBoxType)
    {
        for (idx=0; idx<cellAmtTtl; idx++)
        {
            cell = &devMgr->cell[idx];
            cell->cellIdxInTray = idx % (gTmpTrayBoxAmt*gTmpBoxChnAmt*gTmpChnCellAmt);
            cell->cellIdxInBox = idx % (gTmpBoxChnAmt*gTmpChnCellAmt);
            cell->cellIdxInChnl = idx % gTmpChnCellAmt;
            cell->chn = &devMgr->chn[idx/gTmpChnCellAmt];
            cell->genIdxInTray = cell->chn->genIdxInTray + 1 + cell->cellIdxInChnl;
            cell->genIdxInBox = cell->chn->genIdxInBox + 1 + cell->cellIdxInChnl;
            cell->chnProtBuf.cellTmprPre.tmprBeValid = False;
            cell->chnProtBuf.cellTmprCrnt.tmprBeValid = False;
            cell->chnProtBuf.cellTmprCrnt.tmprInvalidCnt = 0;
            cell->chnProtBuf.newPowerBeValid = False;
            cell->chnProtBuf.prePowerBeValid = False;
            cell->chnProtBuf.newCauseCode = CcNone;
            cell->chnProtBuf.preCauseCode = CcNone;
        }

        for (idx=0; idx<genChnAmtTtl; idx++)
        {
            chnMap = &devMgr->chnMap[idx];
            if (0 == idx%(gTmpChnCellAmt+1))
            {
                u16 chnIdx;

                chnMap->isMainChn = True;
                chnIdx = idx / (gTmpChnCellAmt+1);
                chnMap->typeIdxInBox = devMgr->chn[chnIdx].chnIdxInBox;
                chnMap->typeIdxInTray = devMgr->chn[chnIdx].chnIdxInTray;
            }
            else
            {
                u16 cellIdx;

                chnMap->isMainChn = False;
                cellIdx = idx - (idx/(gTmpChnCellAmt+1)+1);
                chnMap->typeIdxInBox = devMgr->cell[cellIdx].cellIdxInBox;
                chnMap->typeIdxInTray = devMgr->cell[cellIdx].cellIdxInTray;
            }
        }
    }

    for (idx=0; idx<uartAmt; idx++)
    {
        uart = &devMgr->uart[idx];
        uart->uartIdx = idx;
        uart->uartMsgFlowSeq = 0;
        uart->waitAckBlockBuf = NULL;
        ListInitD(&uart->uartBlockList);
        TimerInit(&uart->waitUartAckTmr);
        TimerInit(&uart->uartSmplLoopTmr);
        uart->uartCrntSmplType = UartNeedSmplCri;/*确保超时是采样必要条件*/
    }

    devMgr->uart[devMgr->tmprSmplUartIdx].uartSmplDevAmt[UartTmprSmpl] = gTmpTmprSmplAmt;
    for (idx=0; idx<gTmpTmprSmplAmt; idx++)
    {
        tmprSmpl = &devMgr->tmprSmpl[idx];
        tmprSmpl->actAddr = gTmprSmplAddr[idx];
        tmprSmpl->cmmuAddr = tmprSmpl->actAddr + UartAddrBaseTmprSmpl;
        tmprSmpl->tmprSmplDevIdx = idx;
        tmprSmpl->online = False;
        tmprSmpl->smplExprCnt = 0;
        tmprSmpl->tmprAmt4Cell = gTmpTmprSmplCellAmt;
        tmprSmpl->tmprBase4Cell = gTmpTmprSmplCellBase;
        tmprSmpl->tmprAmt4Loc = gTmpTmprSmplLocAmt;
        tmprSmpl->tmprBase4Loc = gTmpTmprSmplLocBase;
        tmprSmpl->genSlotTmprIdx = idx*gTmpTmprSmplLocAmt;
        tmprSmpl->genCellTmprIdx = idx*gTmpTmprSmplCellAmt;
        tmprSmpl->tmprChnAmt = gTmpTmprSmplChnAmt;
        tmprSmpl->crntSmplChnIdx = 0;
        tmprSmpl->uartIdx = gTmpTmprSmplUartIdx;
        devMgr->uart[devMgr->tmprSmplUartIdx].uartStaAddr2DevIdx[UartTmprSmpl][gTmprSmplAddr[idx]] = idx;
    }

    TimerInit(&devMgr->devAppRstTmr);
    return 0;
}

/*初始化顺序并非允许随意调整*/
void ctrlInitApp()
{
#ifdef DebugVersion
    MemDesc desc[] = { {128, 4096}, {512, 1024} };
#else
    MemDesc desc[] = { {64, 64}, {128, 32}, {256, 32}, {512, 16} };
#endif

    funcInit(desc, sizeof desc/sizeof *desc);
    timerInit();
    mainInit();
    logInit();

    upInitApp();
    flowInit();
    boxInit();
    uartInit();
    plcInit();

    return;
}
void ctrlInitBoot()
{
    timerInit();
    //mainInit();
    //logInit();
    upInitBoot();
    return;
}

#ifdef DebugVersion
#if 1
s32 main()
{
    ctrlInitApp();
    return 0;
}
#endif
#endif

