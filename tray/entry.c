


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic.h"
#if TRAY_ENABLE
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
#include "tray.h"

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
#else
#include "u_setting.h"
#include "ds.h"
#endif

DevMgr *gDevMgr;
u16 gProtoSoftVer = 1;  /*上中协议版本*/
u16 gProtoSmplVer = 0x0101;  /*上中采样版本*/
u16 gMediumSoftVer = 41;  /*中位机软件版本*/
u8 gLowProtoVer = 2;  /*中下协议版本*/
u8 gNdbdMcuProtoVer = 1;  /*针床主控协议版本*/
u8 gSmplStoreVer = 4;  /*磁盘存储版本*/

/*临时量，以后替换为配置*/
u8eBoxType gTrayBoxType = BoxTypeParallel;
u8 gTmpTrayAmt = 1;
u8 gTmpTrayBoxAmt = 8;
u8 gTmpBoxChnAmt = 4;  /*满配主通道数*/
u8 gTmpChnCellAmt = 0;  /*满配电芯数量*/
u8 gChnModuAmt = 1;
u8 gBoxAddr[32] = { 0,1,2,3,4,5,6,7 }; /*所有box的addr都必须列出*/
u8 gCAN0BoxNum=4;
u8 gCan0BoxIdx[32] = { 0,1,2,3 }; /*dev-box-idx*/
u32 gVoltageMax = 6000000;
u32 gCurrentMax = 12000000;
u16 gTmpTmprSmplCellAmt = 32; /*温度板用于电芯的通道数量,按满配*/
u8 gBoxVolSmplBoardAmt = 0; /*单box电压采样板数量,目前极简串联有*/

u8 gTmpFixtUartIdx = 0;

u8 gTmpTmprSmplAmt = 0;  /*温度采样板数量*/
u8 gTmprSmplAddr[16] = { 0,1 }; /*所有的addr都必须列出*/
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
//u32 gTrayDiskBufAmt = 64;  /*即便临时验证,磁盘数也要大于内存数*/
u32 gTrayDiskBufAmt = 2000;  /*平均1秒多于1条数据,给30分钟留点余量*/

u8 gBoxBypsSwBoardAmt = 0;  /*旁路切换板数量,目前旁路串联有*/

u8 gSpecsMapId = 0;
u8 gNdbdCtrlByMcu = 0;  /*针床控制0--plc,1--单片机主控*/
u8 gNdbdMcuUartIdx = 0;

#ifdef Fd3CapSpec  /*fd3出货特例,,32路温度板*/
u8 gSlotWaterTmprAmt = 2;  /*每个库位的水温个数*/
u8 gWaterTmprSmplAmt = 0;  /*温度盒用于水温的通道数量*/
u8 gWaterTmprSmplBase = 0;  /*温度盒水温基址*/
#else
#if 1
u8 gSlotWaterTmprAmt = 0;  /*每个库位的水温个数*/
u8 gWaterTmprSmplAmt = 0;  /*温度盒用于水温的通道数量*/
u8 gWaterTmprSmplBase = 0;  /*温度盒水温基址*/
#else  /*fd3和psl的容量样机使用52通道温度板时候*/
u8 gSlotWaterTmprAmt = 2;  /*每个库位的水温个数*/
u8 gWaterTmprSmplAmt = 2;  /*温度盒用于水温的通道数量*/
u8 gWaterTmprSmplBase = 46;  /*温度盒水温基址*/
#endif
#endif

#if 1   /*出货版本打开,等升级工具做好后固化这里*/
u8 gUsedMode = 0;  /*使用模式,0--出货(客户用),1--备货(家里生产用)*/
#else
u8 gUsedMode = 1;  /*使用模式,0--出货(客户用),1--备货(家里生产用)*/
#endif
void tray_setting_load(void)
{
#ifdef DebugVersion
#else
    u8 i,addr;
    u16 u16temp;
    //判断配置文件是否存在
    if (!setting_in_being())
    {
        return ;
    }

    gTrayBoxType =mySetting->traypwrBoxType ;
    gTmpTrayAmt =mySetting->trayNum;
    gTmpTrayBoxAmt = mySetting->traypwrBoxNum;      //托盘的箱数
    gTmpBoxChnAmt = mySetting->trayPwrBoxChnlNum;   //箱的通道数
    gTmpChnCellAmt = mySetting->trayChnlCellNum;    //通道的电芯数
    gChnModuAmt = mySetting->trayChnlMdlNum;            //通道的模块数

     //gBoxAddr[32] = { 0,1,2,3,4,5,6,7 }; /*所有box的addr都必须列出*/
    //gCan0BoxIdx[32] = { 0,1,2,3 }; /*dev-box-idx*/
    u16temp=gTmpTrayAmt;
    u16temp*=gTmpTrayBoxAmt;
    addr=mySetting->pwrBoxStartAddr;
    for (i = 0; i < u16temp; i++)
    {
       gBoxAddr[i]=addr++;
    }
    gCAN0BoxNum=mySetting->CNA1PwrBoxNum;
    for (i = 0; i < gCAN0BoxNum; i++)
    {
    	gCan0BoxIdx[i]=gBoxAddr[i];
    }
   
    gVoltageMax =mySetting->pwrChnlVolRange;
    gCurrentMax = mySetting->pwrChnlCurRange;

    gTmpTmprSmplCellAmt = mySetting->tmprBoxCellChnlNum; /*温度板用于电芯的通道数量,不含预留*/
    gBoxVolSmplBoardAmt = mySetting->volBoxPerPwrBox; /*单box电压采样板数量,目前极简串联有*/    


    gTmpFixtUartIdx = mySetting->toolingUartIdx;//工装的串口号

    gTmpTmprSmplAmt = mySetting->tmprBoxNum;  /*温度采样板数量*/
   //gTmprSmplAddr[10] = { 0,1 }; //所有的addr都必须列出
   addr=mySetting->tmprBoxStartAddr;
    for (i = 0; i < mySetting->tmprBoxNum; i++)
    {
        gTmprSmplAddr[i]= addr++;
    }

    gTmpTmprSmplChnAmt = mySetting->tmprBoxChnlTotal;  /*温度盒通道总数量,含预留*/

    gTmpTmprSmplCellBase = 0;
    gTmpTmprSmplLocAmt = mySetting->tmprLocChnlNUm;  /*用于库温的通道数量,不含预留*/
    gTmpTmprSmplLocBase = mySetting->tmprLocChnlStart;
    gTmprAmtPerCell = mySetting->tmprChnlPerCell;  /*每个电芯的温度个数*/
    gTmprAmtPerSlot = mySetting->tmprChnlPerLoction;  /*每个库位的库温个数*/
    gTmpTmprSmplUartIdx = mySetting->tmprBoxUartIdx;

    gSpecsMapId = mySetting->devSpecsId;
    gNdbdCtrlByMcu = mySetting->ndbdCtrlByMcu;
    gNdbdMcuUartIdx = mySetting->ndbdMcuOnUartIdx;
    gBoxBypsSwBoardAmt = mySetting->bypsBoxPerPwrBox;  /*每电源箱的旁路切换板数量*/

#if 0
    gSlotWaterTmprAmt = mySetting->slotWaterTmprAmt;  /*每个库位的水温个数*/
    gWaterTmprSmplAmt = mySetting->tmprSmplWaterAmt;  /*温度盒的水温个数*/
    gWaterTmprSmplBase = mySetting->tmprSmplWaterBase;  /*温度盒的水温起始序号*/
#endif
#endif
    return;
}

void traySpecsMap(Tray *tray, SpecsMap *specsMap)
{
    Box *box;
    Box *boxCri;
    Channel *chn;
    Channel *chnCri;
    u16 idx;
    u16 noUsedIdx;
    u8 chnIdxInBox;
    u8 boxIdx;
    u8 offset;
    u8 delCnt;

    if (BoxTypeParallel == tray->boxCfg.boxType)
    {
        for (box=tray->box,boxCri=box+tray->boxAmt; box<boxCri; box++)
        {
            box->boxChnAmt = gTmpBoxChnAmt;  /*先按满配*/
        }

        for (idx=0; idx<specsMap->noUsedChnAmt; idx++) /*再计算实际数量*/
        {
            boxIdx = specsMap->noUsedChnIdx[idx] / gTmpBoxChnAmt; /*离席通道所在box*/
            tray->box[boxIdx].boxChnAmt--;
        }

        for (chn=tray->chn,box=tray->box,boxCri=box+tray->boxAmt; box<boxCri; box++)
        {
            box->chn = chn;  /*最后确定box的起始通道*/
            chn += box->boxChnAmt;
        }

        for (chnIdxInBox=0,box=tray->box,idx=0; idx<tray->trayChnAmt; idx++)
        {
            chn = &tray->chn[idx];
            chn->chnIdxInTray = idx;
            chn->genIdxInTray = idx;
            chn->tmprIdx = idx;  /*先按此,后修正*/
            chn->box = box;
            chn->chnIdxInBox = chnIdxInBox++;
            chn->lowChnIdxInBox = chn->chnIdxInBox;  /*先按相等,下面调整*/
            if (chnIdxInBox == box->boxChnAmt)
            {
                chnIdxInBox = 0;
                box++;
            }
        }

        /*修正通道号到下位机的映射*/
        for (delCnt=0,box=tray->box,idx=0; idx<specsMap->noUsedChnAmt; idx++)
        {
            noUsedIdx = specsMap->noUsedChnIdx[idx];
            boxIdx = noUsedIdx / gTmpBoxChnAmt; /*离席通道所在box*/
            offset = noUsedIdx % gTmpBoxChnAmt; /*离席通道的box内偏移*/
            if (box != &tray->box[boxIdx])
            {
                box = &tray->box[boxIdx];
                delCnt = 0;
            }
            for (chn=&box->chn[offset-delCnt],chnCri=&box->chn[box->boxChnAmt]; chn<chnCri; chn++)
            {
                chn->lowChnIdxInBox++;
            }
            delCnt++;

            for (chn=&tray->chn[noUsedIdx-idx],chnCri=&tray->chn[tray->trayChnAmt]; chn<chnCri; chn++)
            {
                chn->tmprIdx++;
            }
        }

        for (box=tray->box,boxCri=box+tray->boxAmt; box<boxCri; box++)
        {
            box->boxCellAmt = box->boxChnAmt;
        }
    }
    else
    {
        Cell *cell;
        Cell *cellCri;
        u16 chnIdx;
        u8 cellIdxInChn;

        for (chn=tray->chn,box=tray->box,boxCri=box+tray->boxAmt; box<boxCri; box++)
        {
            box->boxChnAmt = gTmpBoxChnAmt;
            box->chn = chn;
            chn += box->boxChnAmt;
        }

        for (chnIdxInBox=0,box=tray->box,idx=0; idx<tray->trayChnAmt; idx++)
        {
            chn = &tray->chn[idx];
            chn->chnIdxInTray = idx;
            chn->genIdxInTray = idx;  /*先按相等,下面调整*/
            chn->chnCellAmt = gTmpChnCellAmt;  /*先按满配,下面调整*/
            chn->tmprIdx = 0;
            chn->box = box;
            chn->chnIdxInBox = chnIdxInBox++;
            chn->lowChnIdxInBox = chn->chnIdxInBox;
            if (chnIdxInBox == box->boxChnAmt)
            {
                chnIdxInBox = 0;
                box++;
            }
        }

        for (idx=0; idx<specsMap->noUsedChnAmt; idx++) /*修正通道实际电芯数量*/
        {
            chnIdx = specsMap->noUsedChnIdx[idx] / gTmpChnCellAmt; /*离席通道所在chn*/
            tray->chn[chnIdx].chnCellAmt--;
        }

        for (cell=tray->cell,chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            chn->cell = cell;  /*确定chn起始电芯*/
            chn->genIdxInTray += cell-tray->cell;  /*修正通道的统排索引*/
            cell += chn->chnCellAmt;
        }

        for (cellIdxInChn=0,chn=tray->chn,idx=0; idx<tray->trayCellAmt; idx++)
        {
            cell = &tray->cell[idx];
            cell->cellIdxInTray = idx;
            cell->tmprIdx = idx;  /*先按此,后修正*/
            cell->chn = chn;
            cell->genIdxInTray = chn->genIdxInTray + 1 + cellIdxInChn;
            cell->cellIdxInChn = cellIdxInChn++;
            cell->lowCellIdxInChn = cell->cellIdxInChn;  /*先按相等,下面调整*/
            if (cellIdxInChn == chn->chnCellAmt)
            {
                cellIdxInChn = 0;
                chn++;
            }
        }

        for (delCnt=0,chn=tray->chn,idx=0; idx<specsMap->noUsedChnAmt; idx++) /*修正电芯号到下位机的映射*/
        {
            noUsedIdx = specsMap->noUsedChnIdx[idx];
            chnIdx = noUsedIdx / gTmpChnCellAmt; /*离席cell所在chn*/
            offset = noUsedIdx % gTmpChnCellAmt; /*离席cell的chn内偏移*/
            if (chn != &tray->chn[chnIdx])
            {
                chn = &tray->chn[chnIdx];
                delCnt = 0;
            }
            for (cell=&chn->cell[offset-delCnt],cellCri=&chn->cell[chn->chnCellAmt]; cell<cellCri; cell++)
            {
                cell->lowCellIdxInChn++;
            }
            delCnt++;

            for (cell=&tray->cell[noUsedIdx-idx],cellCri=&tray->cell[tray->trayCellAmt]; cell<cellCri; cell++)
            {
                cell->tmprIdx++;
            }
        }

        for (box=tray->box,boxCri=box+tray->boxAmt; box<boxCri; box++)
        {
            box->boxCellAmt = 0;
            for (chn=box->chn,chnCri=chn+box->boxChnAmt; chn<chnCri; chn++)
            {
                box->boxCellAmt += chn->chnCellAmt;
            }
        }
    }
    return;
}

/*启动后从flash恢复数据,以满足续接为目的,并非完全恢复*/
void trayRestoreData(u8 trayAmt, u8 *restoreSmplCnt)
{
    TraySmplRcd *traySmplRcd;
    TrayChnSmpl *trayChnSmpl;
    TrayChnSmpl *newChnSmpl;
    Tray *tray;
    SmplSaveMgr *smplMgr;
    Channel *chn;
    Channel *chnCri;
    Cell *cell;
    Cell *cellCri;
    u8 smplCnt;
    u8 trayIdx;
    u8 smplIdx;
    b8 needNewSmpl; /*掉电前是否有负压等待,启动,运行,是则需要产生新采样*/
    b8 allowNewSmpl; /*掉电前是否上位机脱机超时,是则不能产生新采样*/

    for (trayIdx=0; trayIdx<trayAmt; trayIdx++)
    {
        tray = &gDevMgr->tray[trayIdx];
        smplMgr = &tray->smplMgr;
        if (0 == restoreSmplCnt[trayIdx])
        {
            continue;
        }

        if (restoreSmplCnt[trayIdx] == smplMgr->smplBufAmt)
        {
            restoreSmplCnt[trayIdx] -= 1;  /*留出第一条缓存准备存储新采样*/
        }

        /*看看掉电前是否上位机脱机超时,若是则不能生成新采样*/
        traySmplRcd = (TraySmplRcd *)smplMgr->smplBufAddrMax;
        for (allowNewSmpl=True,chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            trayChnSmpl = &traySmplRcd->traySmpl->chnSmpl[chn->genIdxInTray];
            if (Cc3UpDiscExpr == trayChnSmpl->causeCode)
            {
                allowNewSmpl = False;
                break;
            }
        }

        if (BoxTypeSeriesWiSw == tray->box->boxType)
        {
            for (needNewSmpl=False,cell=tray->cell,cellCri=cell+tray->trayCellAmt; cell<cellCri; cell++)
            {
                for (smplCnt=0; smplCnt<restoreSmplCnt[trayIdx]; smplCnt++)
                {
                    traySmplRcd = (TraySmplRcd *)(smplMgr->smplBufAddrMax-smplMgr->smplItemSize*smplCnt);
                    trayChnSmpl = &traySmplRcd->traySmpl->chnSmpl[cell->genIdxInTray];
                    if (ChnUpStateOffline==trayChnSmpl->chnUpState || ChnUpStateTemp==trayChnSmpl->chnUpState)
                    {
                        continue;
                    }

                    /*从后往前找的第一条有效数据*/
                    if (ChnUpStateIdle != trayChnSmpl->chnUpState)
                    {
                        BitSet(cell->chn->bypsSeriesInd->startCell, cell->cellIdxInChn);
                        BitSet(cell->chn->bypsSeriesInd->pausedCell, cell->cellIdxInChn);
                        cell->cellUpStepId = trayChnSmpl->stepId;
                        cell->cellUpStepType = trayChnSmpl->stepType;
                        cell->cellStepRunTime = trayChnSmpl->stepRunTime;
                        cell->cellCapStep = trayChnSmpl->capacity;
                        if (ChnUpStatePause != trayChnSmpl->chnUpState) /*负压启动运行三态*/
                        {
                            needNewSmpl = True;
                        }
                    }
            
                    break;
                }
            }
        }
        else
        {
            for (needNewSmpl=False,chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
            {
                for (smplCnt=0; smplCnt<restoreSmplCnt[trayIdx]; smplCnt++)
                {
                    traySmplRcd = (TraySmplRcd *)(smplMgr->smplBufAddrMax-smplMgr->smplItemSize*smplCnt);
                    trayChnSmpl = &traySmplRcd->traySmpl->chnSmpl[chn->genIdxInTray];
                    if (ChnUpStateOffline==trayChnSmpl->chnUpState || ChnUpStateTemp==trayChnSmpl->chnUpState)
                    {
                        continue;
                    }
            
                    /*从后往前找的第一条有效数据*/
                    if (ChnUpStateIdle != trayChnSmpl->chnUpState)
                    {
                        chn->chnStateMed = ChnStaPause;
                        chn->chnStepId = trayChnSmpl->stepId;
                        chn->chnStepType = trayChnSmpl->stepType;
                        chn->stepSubType = trayChnSmpl->stepSubType;
                        chn->stepRunTime = trayChnSmpl->stepRunTime;
                        chn->capStep = trayChnSmpl->capacity;
                        if (ChnUpStatePause != trayChnSmpl->chnUpState) /*负压启动运行三态*/
                        {
                            needNewSmpl = True;
                        }
                    }
            
                    break;
                }
            }
        }

        if (!needNewSmpl || !allowNewSmpl)  /*不需要或不允许产生新采样就算了*/
        {
            continue;
        }

        /*生成一条全部设为临时的新数据*/
        memcpy(smplMgr->smplBufAddr, smplMgr->smplBufAddrMax, smplMgr->smplItemSize);
        traySmplRcd = (TraySmplRcd *)(smplMgr->smplBufAddr);
        traySmplRcd->timeStampSec = 0;
        for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
        {
            traySmplDefChnSmpl(tray, chn, ChnUpStateTemp, CcNone);
        }

        /*找到负压启动运行三态的通道修改对应新数据*/
        if (BoxTypeSeriesWiSw == tray->box->boxType)
        {
            for (cell=tray->cell,cellCri=cell+tray->trayCellAmt; cell<cellCri; cell++)
            {
                for (smplCnt=0; smplCnt<restoreSmplCnt[trayIdx]; smplCnt++)
                {
                    traySmplRcd = (TraySmplRcd *)(smplMgr->smplBufAddrMax-smplMgr->smplItemSize*smplCnt);
                    trayChnSmpl = &traySmplRcd->traySmpl->chnSmpl[cell->genIdxInTray];
                    if (ChnUpStateOffline==trayChnSmpl->chnUpState || ChnUpStateTemp==trayChnSmpl->chnUpState)
                    {
                        continue;
                    }

                    /*从后往前找的第一条有效数据,若是三态的就修改到新数据*/
                    if (ChnUpStateIdle!=trayChnSmpl->chnUpState && ChnUpStatePause!=trayChnSmpl->chnUpState)
                    {
                        traySmplRcd = (TraySmplRcd *)(smplMgr->smplBufAddr);
                        newChnSmpl = &traySmplRcd->traySmpl->chnSmpl[cell->genIdxInTray];
                        memcpy(newChnSmpl, trayChnSmpl, sizeof(TrayChnSmpl));
                        newChnSmpl->causeCode = Cc1MedReset;
                        newChnSmpl->chnUpState = ChnUpStateRun;
                    }
            
                    break;
                }
            }
        }
        else
        {
            for (chn=tray->chn,chnCri=chn+tray->trayChnAmt; chn<chnCri; chn++)
            {
                for (smplCnt=0; smplCnt<restoreSmplCnt[trayIdx]; smplCnt++)
                {
                    traySmplRcd = (TraySmplRcd *)(smplMgr->smplBufAddrMax-smplMgr->smplItemSize*smplCnt);
                    trayChnSmpl = &traySmplRcd->traySmpl->chnSmpl[chn->genIdxInTray];
                    if (ChnUpStateOffline==trayChnSmpl->chnUpState || ChnUpStateTemp==trayChnSmpl->chnUpState)
                    {
                        continue;
                    }
            
                    /*从后往前找的第一条有效数据,若是三态的就修改到新数据*/
                    if (ChnUpStateIdle!=trayChnSmpl->chnUpState && ChnUpStatePause!=trayChnSmpl->chnUpState)
                    {
                        traySmplRcd = (TraySmplRcd *)(smplMgr->smplBufAddr);
                        newChnSmpl = &traySmplRcd->traySmpl->chnSmpl[chn->genIdxInTray];
                        memcpy(newChnSmpl, trayChnSmpl, sizeof(TrayChnSmpl)*(chn->chnCellAmt+1));
                        for (smplIdx=0; smplIdx<chn->chnCellAmt+1; smplIdx++,newChnSmpl++)
                        {
                            newChnSmpl->causeCode = Cc1MedReset;
                            newChnSmpl->chnUpState = ChnUpStateRun;
                        }
                    }
            
                    break;
                }
            }
        }

        /*存储新采样到flash,并更新采样控制变量*/
        smplDiskWrite(trayIdx, 1, smplMgr->smplBufAddr, smplMgr->smplSeqNext);
        trayUpdSmplSeq(smplMgr);
        trayUpdSmplBuf(smplMgr);
    }
    
    return;
}

int mainInit()
{
    DevMgr *devMgr;
    Tray *tray;
    Cell *cell;
    Channel *chn;
    Box *box;
    Can *can;
    Uart *uart;
    TmprSmpl *tmprSmpl;
    NdbdData *ndbdData;
    SmplSaveMgr *smplMgr;
    SpecsMap *specsMap;
    u8 *restoreSmplCnt;
    u16 idx;
    u16 idx0;
    u8 addr;
    u16 actChnAmtTtl;  /*实际使用通道数*/
    u8 trayAmt;
    u8 boxAmtTtl;
    u16 chnAmtTtl;  /*满配通道数*/
    u16 cellAmtTtl;  /*满配串联电芯数*/
    u16 tmprAmtTtl;  /*电池个数,按满配*/
    u8 canAmt;
    u8 uartAmt;
    u16 subBoxAmt;

    trayAmt = gTmpTrayAmt;
    boxAmtTtl = trayAmt * gTmpTrayBoxAmt;
    chnAmtTtl = boxAmtTtl * gTmpBoxChnAmt;
    if (BoxTypeParallel == gTrayBoxType)
    {
        cellAmtTtl = 0;
        tmprAmtTtl = chnAmtTtl;
    }
    else
    {
        cellAmtTtl = chnAmtTtl * gTmpChnCellAmt;
        tmprAmtTtl = cellAmtTtl;
    }

    uartAmt = 2;
    canAmt = 1;
    if (boxAmtTtl > gCAN0BoxNum)
    {
        canAmt = 2;
    }

    gDevMgr = devMgr = sysMemAlloc(sizeof(DevMgr));
    specsMap = gDevMgr->specsMap;
    specsMap->noUsedChnAmt = 0;
    specsMap = &gDevMgr->specsMap[1];
    specsMap->noUsedChnAmt = 4;
    specsMap->noUsedChnIdx[0] = 3;
    specsMap->noUsedChnIdx[1] = 8;
    specsMap->noUsedChnIdx[2] = 15;
    specsMap->noUsedChnIdx[3] = 20;
    specsMap = &gDevMgr->specsMap[2];
    specsMap->noUsedChnAmt = 8;
    specsMap->noUsedChnIdx[0] = 1;
    specsMap->noUsedChnIdx[1] = 4;
    specsMap->noUsedChnIdx[2] = 7;
    specsMap->noUsedChnIdx[3] = 10;
    specsMap->noUsedChnIdx[4] = 13;
    specsMap->noUsedChnIdx[5] = 16;
    specsMap->noUsedChnIdx[6] = 19;
    specsMap->noUsedChnIdx[7] = 22;
    if (gSpecsMapId > 2)
    {
        gSpecsMapId = 0;
    }
    specsMap = &gDevMgr->specsMap[gSpecsMapId];

    devMgr->chnStaMapMed2Up[ChnStaIdle] = ChnUpStateIdle;
    devMgr->chnStaMapMed2Up[ChnStaPause] = ChnUpStatePause;
    devMgr->chnStaMapMed2Up[ChnStaNpWait] = ChnUpStateNp;
    devMgr->chnStaMapMed2Up[ChnStaStart] = ChnUpStateStart;
    devMgr->chnStaMapMed2Up[ChnStaUpStopStartReq] = ChnUpStateStart;
    devMgr->chnStaMapMed2Up[ChnStaUpPauseStartReq] = ChnUpStateStart;
    devMgr->chnStaMapMed2Up[ChnStaUpStopNpReq] = ChnUpStateNp;  /*实际走不到*/
    devMgr->chnStaMapMed2Up[ChnStaUpPauseNpReq] = ChnUpStateNp;  /*实际走不到*/
    devMgr->chnStaMapMed2Up[ChnStaRun] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaUpStopReq] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaUpPauseReq] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaUpStopEnd] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaUpPauseEnd] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaLowProtEnd] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaLowNmlEnd] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaMedEnd] = ChnUpStateRun;
    devMgr->chnStaMapMed2Up[ChnStaMedIdleWait] = ChnUpStateIdle;
    devMgr->chnStaMapMed2Up[ChnStaMedPauseWait] = ChnUpStatePause;

    devMgr->trayAmt = trayAmt;
    devMgr->canAmt = canAmt;
    devMgr->uartAmt = uartAmt;
    devMgr->smplMode = gIsTraySmpl ? SmplModeTray : SmplModeChn;
    devMgr->needConnUp = True;
    devMgr->cellTmprAmt = tmprAmtTtl * gTmprAmtPerCell;
    devMgr->slotTmprAmt = trayAmt * gTmprAmtPerSlot;
    devMgr->waterTmprAmt = trayAmt * gSlotWaterTmprAmt;
    devMgr->fixtUartIdx = gTmpFixtUartIdx;

    restoreSmplCnt = 0==trayAmt ? NULL : memAlloc(trayAmt);
    devMgr->tray = sysMemAlloc(sizeof(Tray) * trayAmt);
    devMgr->box = sysMemAlloc(sizeof(Box) * boxAmtTtl);
    actChnAmtTtl = BoxTypeParallel==gTrayBoxType ? chnAmtTtl-specsMap->noUsedChnAmt*trayAmt : chnAmtTtl;
    devMgr->chn = sysMemAlloc(sizeof(Channel) * actChnAmtTtl);
    devMgr->can = sysMemAlloc(sizeof(Can) * canAmt);
    devMgr->uart = sysMemAlloc(sizeof(Uart) * uartAmt);
    devMgr->genCellTmpr = 0==devMgr->cellTmprAmt ? NULL : sysMemAlloc(Align16(devMgr->cellTmprAmt)*sizeof(u16));
    for (idx=0; idx<devMgr->cellTmprAmt; idx++)
    {
        devMgr->genCellTmpr[idx] = 0; /*温度盒不在线,给0*/
    }
    devMgr->genSlotTmpr = 0==devMgr->slotTmprAmt ? NULL : sysMemAlloc(Align16(devMgr->slotTmprAmt)*sizeof(u16));
    for (idx=0; idx<devMgr->slotTmprAmt; idx++)
    {
        devMgr->genSlotTmpr[idx] = 0; /*温度盒不在线,给0*/
    }
    devMgr->genSlotWaterTmpr = 0==devMgr->waterTmprAmt ? NULL : sysMemAlloc(Align16(devMgr->waterTmprAmt)*sizeof(u16));
    for (idx=0; idx<devMgr->waterTmprAmt; idx++)
    {
        devMgr->genSlotWaterTmpr[idx] = 0; /*温度盒不在线,给0*/
    }


    devMgr->cell = NULL;
    if (BoxTypeParallel != gTrayBoxType)
    {
        devMgr->cell = sysMemAlloc(sizeof(Cell) * (cellAmtTtl-specsMap->noUsedChnAmt*trayAmt));
    }

    devMgr->tmprSmpl = NULL;
    if (0 != gTmpTmprSmplAmt)
    {
        devMgr->tmprSmpl = sysMemAlloc(sizeof(TmprSmpl) * gTmpTmprSmplAmt);
    }

    devMgr->ndbdMcu = NULL;
    if (gNdbdCtrlByMcu)
    {
        devMgr->ndbdMcu = sysMemAlloc(sizeof(NdbdMcu) * trayAmt);
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
        tray->protDisable = ProtDisableTouch;
        tray->ndbdCtrlByMcu = gNdbdCtrlByMcu;
        TimerInit(&tray->protEnaTmr);
        TimerInit(&tray->npSwRstDelayTmr);
        TimerInit(&tray->npSwProtDelayTmr);
        TimerInit(&tray->ndbdBrkWaitNpTmr);
        TimerInit(&tray->mntnExprTmr);
        TimerInit(&tray->trayProtMgr.protPolicyTmr);
        tray->trayProtEntry = NULL;
        tray->flowRecvCtrl = NULL;
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
        if (BoxTypeParallel == gTrayBoxType)
        {
            tray->trayChnAmt = tray->boxAmt*gTmpBoxChnAmt - specsMap->noUsedChnAmt;
        }
        else
        {
            tray->trayChnAmt = tray->boxAmt * gTmpBoxChnAmt;
        }
        tray->chn = &devMgr->chn[idx*tray->trayChnAmt];
        if (BoxTypeParallel == tray->boxCfg.boxType)
        {
            tray->trayCellAmt = tray->trayChnAmt;
            tray->genChnAmt = tray->trayChnAmt;
            tray->cell = NULL;
        }
        else
        {
            tray->trayCellAmt = tray->trayChnAmt*gTmpChnCellAmt - specsMap->noUsedChnAmt;
            tray->genChnAmt = tray->trayChnAmt + tray->trayCellAmt;
            tray->cell = &devMgr->cell[idx*tray->trayCellAmt];
        }

        ndbdData = &tray->ndbdData;
        ndbdData->tmprAmtPerCell = gTmprAmtPerCell;
        ndbdData->slotTmprAmt = gTmprAmtPerSlot;
        ndbdData->slotWaterTmprAmt = gSlotWaterTmprAmt;
        ndbdData->cellTmpr = 0==gTmprAmtPerCell ? NULL : &devMgr->genCellTmpr[idx*tmprAmtTtl/trayAmt*gTmprAmtPerCell];
        ndbdData->slotTmpr = 0==gTmprAmtPerSlot ? NULL : &devMgr->genSlotTmpr[idx * ndbdData->slotTmprAmt];
        ndbdData->slotWaterTmpr = 0==gSlotWaterTmprAmt ? NULL : &devMgr->genSlotWaterTmpr[idx * ndbdData->slotWaterTmprAmt];
        ndbdData->bit2IdxSta[BitSenFwdEnter] = NdbdSenFwdEnter;
        ndbdData->bit2IdxSta[BitSenBackEnter] = NdbdSenBackEnter;
        ndbdData->bit2IdxSta[BitSenFwdTouch] = NdbdSenFowTouch;
        ndbdData->bit2IdxSta[BitSenBackTouch] = NdbdSenBackTouch;
        ndbdData->bit2IdxSta[BitSenWorkMode] = NdbdStaWorkMode;
        ndbdData->bit2IdxSta[BitSenNpGate] = NdbdSenNpGate;
        ndbdData->bit2IdxSta[BitSenBrkVacum] = NdbdSenBrkVacum;
        ndbdData->bit2IdxSta[BitSenFireDoorUp] = NdbdSenFireDoorUp;
        ndbdData->bit2IdxSta[BitSenFireDoorDown] = NdbdSenFireDoorDown;
        ndbdData->bit2IdxSta[BitSenFwdDoorMsw] = NdbdSenFwdDoorMsw;
        ndbdData->bit2IdxSta[BitSenBackDoor] = NdbdSenBackDoor;
        ndbdData->bit2IdxSta[BitSenCylinderUp] = NdbdSenCylinderUp;
        ndbdData->bit2IdxSta[BitSenCylinderDown] = NdbdSenCylinderDown;
        ndbdData->bit2IdxSta[BitSenBollOpen] = NdbdSenBollOpen;
        ndbdData->bit2IdxSta[BitSenBollClose] = NdbdSenBollClose;
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
        ndbdData->ndbdDataValid = False;

        smplMgr = &tray->smplMgr;
        smplMgr->isLooped = False;
        smplMgr->upDiscExpr = False;
        smplMgr->smplEnable = False;
        smplMgr->smplTryBoxAmt = 0;
        smplMgr->auxDataDone = False;
        if (SmplModeTray == gDevMgr->smplMode)
        {
        #if 0
            smplMgr->smplItemSize = sizeof(TraySmplRcd) + sizeof(TraySmpl)
                + sizeof(TrayChnSmpl)*tray->genChnAmt + sizeof(TraySmpl) + sizeof(TrayNdbdSmpl)
                + sizeof(u16)*(Align16(gTmprAmtPerSlot)+Align16(tray->trayCellAmt*gTmprAmtPerCell));
        #else
            smplMgr->smplItemSize = sizeof(TraySmplRcd) + sizeof(TraySmpl)
                + sizeof(TrayChnSmpl)*tray->genChnAmt + sizeof(TraySmpl) + sizeof(TrayNdbdSmpl);
            if (0 != gTmprAmtPerSlot)
            {
                smplMgr->smplItemSize += sizeof(NdbdDynData) + sizeof(u16)*Align16(gTmprAmtPerSlot);
            }
            if (0 != gTmprAmtPerCell)
            {
                smplMgr->smplItemSize += sizeof(NdbdDynData) + sizeof(u16)*Align16(tray->trayCellAmt*gTmprAmtPerCell);
            }
            if (0 != gSlotWaterTmprAmt)
            {
                smplMgr->smplItemSize += sizeof(NdbdDynData) + sizeof(u16)*Align16(gSlotWaterTmprAmt);
            }
        #endif
            smplMgr->smplDiskAmt = gTrayDiskBufAmt;
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

        tray->npMgr.npType = NpTypeNone;
        tray->npMgr.beReach = False;
        tray->npMgr.refCnt = 0;
        tray->npMgr.npExpect = 0;
        tray->npMgr.npMax = 0;
        tray->npMgr.npMin = 0;

        tray->mixProtCfg.mixExpAmt = 0;
        tray->mixProtCfg.suffixOfst = 0;
        tray->mixProtCfg.policyOfst = 0;
        memset(tray->mixProtCfg.lifePeriodSec, 0, sizeof(u32)*MixSubCri);
        tray->trayProtMgr.slotTmprCrnt.tmprBeValid = False;
        tray->trayProtMgr.slotTmprPre.tmprBeValid = False;
        tray->trayProtMgr.slotTmprCrnt.tmprInvalidCnt = 0;
        tray->trayProtMgr.preNdbdValid = False;
        tray->trayProtMgr.npWiSw = False;
        memset(tray->trayProtMgr.mixProtHpn, 0, MixExpAmt*sizeof(u32));
        memset(tray->trayProtMgr.policyActNeed, 0, Align8(PolicyCri));
        memset(tray->trayProtMgr.policyActOver, 0, Align8(PolicyCri));

        /*以下映射只为联机上报,其余用不到*/
        tray->tmprSmplAmt = 0;
        tray->tmprSmplIdxBase = 0;
        if (0 != gTmpTmprSmplAmt)
        {
            u16 diff;
            u16 leftTmprChn;

            idx0 = tmprAmtTtl/trayAmt * gTmprAmtPerCell * tray->trayIdx;
            tray->tmprSmplIdxBase = idx0 / gTmpTmprSmplCellAmt;
            leftTmprChn = tmprAmtTtl/trayAmt * gTmprAmtPerCell;
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
        tray->updTypeMap[UpUpdDevNdbd].devType = UartNdbdMcu;
        tray->updTypeMap[UpUpdDevTmpr].cmmuItfType = ItfTypeUart;
        tray->updTypeMap[UpUpdDevTmpr].devType = UartTmprSmpl;
        tray->updTypeMap[UpUpdDevVolResa].cmmuItfType = ItfTypeCan;
        tray->updTypeMap[UpUpdDevVolResa].devType = BoxDevVolSmplResa;
        tray->updTypeMap[UpUpdDevSw].cmmuItfType = ItfTypeCan;
        tray->updTypeMap[UpUpdDevSw].devType = BoxDevSwitch;
        tray->updTypeMap[UpUpdDevVolDsp].cmmuItfType = ItfTypeCan;
        tray->updTypeMap[UpUpdDevVolDsp].devType = BoxDevVolSmplDsp;
    }

#ifdef DebugVersion
#else
    if (0 != trayAmt)
    {
    #if 1
        SmplDiskHead *diskHead;
        
        memset(restoreSmplCnt, 0, trayAmt);
        diskHead = memAlloc(sizeof(SmplDiskHead) + sizeof(SmplDiskTrayHead)*trayAmt);
        if (NULL != diskHead)
        {
            diskHead->trayAmt = trayAmt;
            diskHead->version = gSmplStoreVer;
            for (idx=0; idx<trayAmt; idx++)
            {
                diskHead->trayHead[idx].trayCellAmt = gDevMgr->tray[idx].trayCellAmt;
                diskHead->trayHead[idx].trayBoxAmt = gDevMgr->tray[idx].boxAmt;
                diskHead->trayHead[idx].smplSize = gDevMgr->tray[idx].smplMgr.smplItemSize;
                diskHead->trayHead[idx].smplAmt = gDevMgr->tray[idx].smplMgr.smplDiskAmt;
            }

            smplDiskInit(diskHead);
            for (idx=0; idx<trayAmt; idx++)
            {
                smplMgr = &gDevMgr->tray[idx].smplMgr;
                smplMgr->isLooped = diskHead->trayHead[idx].diskLooped;
                smplMgr->smplSeqNext = diskHead->trayHead[idx].smplSeq;
                smplMgr->upSmplRstInd = !smplMgr->isLooped&&0==smplMgr->smplSeqNext ? True : False;
                if (!smplMgr->upSmplRstInd)  /*有数据就尝试读取到内存*/
                {
                    u8 *buf;
                    u32 seq;

                    seq = 0==smplMgr->smplSeqNext ? smplMgr->smplSeqMax : smplMgr->smplSeqNext-1;
                    for (buf=smplMgr->smplBufAddrMax; buf!=smplMgr->smplBufAddrBase; buf-=smplMgr->smplItemSize)
                    {
                        if (Ok == smplDiskRead(idx, 1, buf, seq))
                        {
                            restoreSmplCnt[idx]++;
                            seq = 0==seq ? smplMgr->smplSeqMax : seq-1;
                            if (seq>smplMgr->smplSeqNext && !smplMgr->isLooped)
                            {
                                break;
                            }
                        }
                        else  /*读磁盘错误,就归零.关键是不能错,错了怎么处理都难*/
                        {
                            smplMgr->isLooped = False;
                            smplMgr->smplSeqNext = 0;
                            smplMgr->upSmplRstInd = True;
                            break;
                        }
                    }
                }
            }
            memFree(diskHead);
        }
    #else
        smplMgr = &gDevMgr->tray->smplMgr;
        ds_tray_file_init(trayAmt, gSmplStoreVer, smplMgr->smplItemSize, smplMgr->smplDiskAmt);

        for (idx=0; idx<trayAmt; idx++)
        {
            smplMgr = &gDevMgr->tray[idx].smplMgr;
            smplMgr->isLooped = ds_file_seek_end(idx, &smplMgr->smplSeqNext, &smplMgr->smplSeqUpReq);
            smplMgr->upSmplRst = !smplMgr->isLooped&&0==smplMgr->smplSeqNext ? True : False;
            if (!smplMgr->upSmplRst)
            {
                u8 *buf;
                u32 seq;
        
                seq = 0==smplMgr->smplSeqNext ? smplMgr->smplSeqMax : smplMgr->smplSeqNext-1;
                for (buf=smplMgr->smplBufAddrMax; buf!=smplMgr->smplBufAddrBase; buf-=smplMgr->smplItemSize)
                {
                    if (!smplMgr->isLooped && seq>smplMgr->smplSeqNext)
                    {
                        break;
                    }
        
                    ds_read_file(idx, seq, 1, buf);
                    seq = 0==seq ? smplMgr->smplSeqMax : seq-1;
                }
            }
        }
    #endif
    }
#endif

    for (idx=0; idx<canAmt; idx++)
    {
        can = &devMgr->can[idx];
        can->canIdx = idx;
        can->canMsgFlowSeq = 0;
        can->smplAgainAct = False;
        can->smplAgainNeed = False;
        can->boxAmt = 0; /*解析box时候确定*/
        for (addr=0; addr<CanAddrMaxCri; addr++)
        {
            can->addr2DevBoxIdx[addr] = 0xff;
        }

        TimerInit(&can->waitCanAckTmr);
        ListInitD(&can->ctrlWaitList);
        ListInitD(&can->ctrlCellWaitList);
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
        box->reTxSmplCmd = False;
        box->reTxSmplCnt = 0;
        box->reTxCtrlCnt = 0;
        box->reTxConnCnt = 0;
        box->boxHasSmplTry = False;
        box->moreSmplPres = False;
        box->chnAmtTtl = box->tray->boxCfg.chnAmtConn;
        box->maxCur = box->tray->boxCfg.maxTrayCur;
        box->maxVol = box->tray->boxCfg.maxTrayVol;
        box->bypsSwAmt = box->tray->boxCfg.bypsSwAmt;
        box->volSmplAmt = box->tray->boxCfg.volSmplAmt;
        box->softVer = 0;
        box->chnModuAmt = box->tray->boxCfg.chnModuAmt;
        box->boxWorkMode = BoxModeManu;
        for (idx0=0; idx0<gCAN0BoxNum && idx!=gCan0BoxIdx[idx0]; idx0++);
        if (idx0 < gCAN0BoxNum)
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
        ChainInitD(&box->ctrlWaitSend.chain);
        ChainInitD(&box->seriesSwWaitSend.chain);
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

    /*初始化与换型无关的内容,与换型相关的内容在换型时候初始化*/
    for (idx=0; idx<actChnAmtTtl; idx++)
    {
        chn = &devMgr->chn[idx];
        chnProtBufInit(&chn->chnProtBuf);
        chn->chnStateMed = ChnStaIdle;
        chn->dynStaCnt = 0;
        chn->noDataCnt = 0;
        chn->stepCapCalced = False;
        chn->chnCellAmt = 0;
        chn->cell = NULL;
        chn->bypsSeriesInd = NULL;
        if (BoxTypeSeriesWiSw == gTrayBoxType)
        {
            chn->bypsSeriesInd = sysMemAlloc(sizeof(BypsSeriesInd));
            memset(chn->bypsSeriesInd, 0, sizeof(BypsSeriesInd));
        }
        chn->beWiNp = False;
        chn->capStep = 0;
        chn->capLow = 0;
        chn->capCtnu = 0;
        chn->capFlowCrnt = 0;
        chn->volInner = 0;
        chn->stepRunTime = 0;
        chn->stepRunTimeCtnu = 0;
        chn->stepRunTimeBase = 0;
        chn->smplPres = False;
        chn->chnStepType = StepTypeNull;
        chn->chnStepId = StepIdNull;
        chn->flowStepEntry = NULL;
        chn->flowProtEntry = NULL;
        chn->crntStepNode = NULL;
    }

    if (BoxTypeParallel != gTrayBoxType)
    {
        for (idx=0; idx<(cellAmtTtl-specsMap->noUsedChnAmt*trayAmt); idx++)
        {
            cell = &devMgr->cell[idx];
            chnProtBufInit(&cell->chnProtBuf);
            cell->bypsSwState = LowBypsSwOut;
            cell->cellLowCause = CcNone;
            cell->cellUpStepId = StepIdNull;
            cell->cellUpStepType = StepTypeNull;
            cell->cellStateMed = ChnStaIdle;
            cell->cellLowCause = CcNone;
        }
    }

    /*换型相关内容*/
    for (idx=0; idx<trayAmt; idx++)
    {
        traySpecsMap(&devMgr->tray[idx], specsMap);
    }

    /*恢复通道相关数据,,用于续接*/
    if (NULL != restoreSmplCnt)
    {
        trayRestoreData(trayAmt, restoreSmplCnt);
        memFree(restoreSmplCnt);
    }

    for (idx=0; idx<uartAmt; idx++)
    {
        uart = &devMgr->uart[idx];
        uart->uartIdx = idx;
        uart->uartMsgFlowSeq = 0;
        uart->waitAckBlockBuf = NULL;
        ListInitD(&uart->uartBlockList);
        ListInitD(&uart->ctrlWaitList);
        TimerInit(&uart->waitUartAckTmr);
        TimerInit(&uart->uartSmplLoopTmr);
        uart->uartCrntSmplType = UartNeedSmplCri;/*确保超时是采样必要条件*/
    }

    devMgr->uart[gTmpTmprSmplUartIdx].uartSmplDevAmt[UartTmprSmpl] = gTmpTmprSmplAmt;
    for (idx=0; idx<gTmpTmprSmplAmt; idx++)
    {
        tmprSmpl = &devMgr->tmprSmpl[idx];
        tmprSmpl->actAddr = gTmprSmplAddr[idx];
        tmprSmpl->cmmuAddr = tmprSmpl->actAddr + UartAddrBaseTmprSmpl;
        tmprSmpl->tmprSmplDevIdx = idx;
        tmprSmpl->online = False;
        tmprSmpl->onlineDelayCnt = 0;
        tmprSmpl->smplExprCnt = 0;
        tmprSmpl->tmprAmt4Cell = gTmpTmprSmplCellAmt;
        tmprSmpl->tmprBase4Cell = gTmpTmprSmplCellBase;
        tmprSmpl->tmprAmt4Loc = gTmpTmprSmplLocAmt;
        tmprSmpl->tmprBase4Loc = gTmpTmprSmplLocBase;
        tmprSmpl->tmprAmt4Water = gWaterTmprSmplAmt;
        tmprSmpl->tmprBase4Water = gWaterTmprSmplBase;
        tmprSmpl->genWaterTmprIdx = idx*gWaterTmprSmplAmt;
        tmprSmpl->genSlotTmprIdx = idx*gTmpTmprSmplLocAmt;
        tmprSmpl->genCellTmprIdx = idx*gTmpTmprSmplCellAmt;
        tmprSmpl->tmprChnAmt = gTmpTmprSmplChnAmt;
        tmprSmpl->crntSmplChnIdx = 0;
        tmprSmpl->uartIdx = gTmpTmprSmplUartIdx;
        devMgr->uart[gTmpTmprSmplUartIdx].uartStaAddr2DevIdx[UartTmprSmpl][gTmprSmplAddr[idx]] = idx;
    }

    devMgr->uart[gNdbdMcuUartIdx].uartSmplDevAmt[UartNdbdMcu] = 0;
    if (gNdbdCtrlByMcu)
    {
        NdbdMcu *ndbdMcu;

        devMgr->uart[gNdbdMcuUartIdx].uartSmplDevAmt[UartNdbdMcu] = trayAmt;
        for (idx=0; idx<trayAmt; idx++)
        {
            ndbdMcu = &devMgr->ndbdMcu[idx];
            ndbdMcu->ndbdMcuIdx = idx;
            ndbdMcu->actAddr = idx;  /*与box相同,强制0开始排*/
            ndbdMcu->cmmuAddr = ndbdMcu->actAddr + UartAddrBaseNdbdCtrl;
            ndbdMcu->online = False;
            ndbdMcu->onlineDelayCnt = 0;
            ndbdMcu->smplExprCnt = 0;
            ndbdMcu->ctrlReTxCnt = 0;
            ndbdMcu->softVer = 0;
            ndbdMcu->uartIdx = gNdbdMcuUartIdx;
            ndbdMcuCtrlRst(&ndbdMcu->ctrlWaitAck);
            ndbdMcuCtrlRst(&ndbdMcu->ctrlWaitTx.ctrl);
            ChainInitD(&ndbdMcu->ctrlWaitTx.chain);
            devMgr->uart[gNdbdMcuUartIdx].uartStaAddr2DevIdx[UartNdbdMcu][idx] = idx;
        }
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

    tray_setting_load();
    funcInit(desc, sizeof desc/sizeof *desc);
    timerInit();
    logInit();
    mainInit();

    upInitApp();
    flowInit(gTmpTrayAmt);
    boxInit();
    uartInit();
    plcInit();
    chnInit();

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
extern u8 *upDataRecv(u8 *buf, u8 *pos, u16 size);
void simuSendMdMsg(u8 *msg, u16 pldLen, u16 msgId)
{
    UpMsgHead *head;
    UpMsgTail *tail;
    u16 pktLen;

    head = (UpMsgHead *)msg;
    head->magicTag = UpMsgMagic;
    head->pldLen = pldLen;
    head->msgId = msgId;

    tail = (UpMsgTail *)(msg+sizeof(UpMsgHead)+pldLen);
    tail->msgFlowSeq = 0;
    tail->crcChk = crc16Modbus(msg+sizeof(u32), pldLen+UpMsgMandCrcLen);

    pktLen = pldLen + UpMsgMandLen;
    upDataRecv(msg, msg, pktLen);
    return;
}
void testTrayFlow()
{
    UpFlowCmd *cmd;
    UpProtStepCmd *stepProtCmd;
    UpProtUnit *protUnit;
    UpStepInfo *step;
    UpMsgHead *head;
    UpMsgTail *tail;
    u8 txBuf[2048];
    u8 idx;

    head = (UpMsgHead *)txBuf;
    cmd = (UpFlowCmd *)(head + 1);
    stepProtCmd = (UpProtStepCmd *)cmd;

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 0;
    cmd->stepAmt = 2;
    cmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 2;
    cmd->stepAmt = 2;
    cmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 4;
    cmd->stepAmt = 2;
    cmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 30000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 0;
    stepProtCmd->protAmt = 3;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = idx % 2;
        protUnit->protId = 0x0403;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 3;
    stepProtCmd->protAmt = 3;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = idx % 2;
        protUnit->protId = 0x0403;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 6;
    stepProtCmd->protAmt = 1;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = 0;
        protUnit->protId = 0x0500;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 0;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 7;
    stepProtCmd->protAmt = 1;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = 0;
        protUnit->protId = 0x0501;
        protUnit->paramEnable = 0x03;
        protUnit->protParam[0] = 600;
        protUnit->protParam[0] = 300;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    printf("second send------\r\n");

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 0;
    cmd->stepAmt = 2;
    cmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 2;
    cmd->stepAmt = 2;
    cmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 4;
    cmd->stepAmt = 2;
    cmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 30000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 0;
    stepProtCmd->protAmt = 3;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = idx % 2;
        protUnit->protId = 0x0403;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 3;
    stepProtCmd->protAmt = 3;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = idx % 2;
        protUnit->protId = 0x0403;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 6;
    stepProtCmd->protAmt = 1;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = 0;
        protUnit->protId = 0x0500;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 0;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 7;
    stepProtCmd->protAmt = 1;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = 0;
        protUnit->protId = 0x0501;
        protUnit->paramEnable = 0x03;
        protUnit->protParam[0] = 600;
        protUnit->protParam[0] = 300;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));
}

void testChnFlow()
{
    UpFlowCmd *cmd;
    UpProtStepCmd *stepProtCmd;
    UpProtGenCmd *upProtGenCmd;
    UpProtUnit *protUnit;
    UpStepInfo *step;
    UpMixProt *mixProt;
    UpMsgHead *head;
    UpMsgTail *tail;
    u8 txBuf[2048];
    u8 idx;

    head = (UpMsgHead *)txBuf;
    cmd = (UpFlowCmd *)(head + 1);
    stepProtCmd = (UpProtStepCmd *)cmd;
    upProtGenCmd = (UpProtGenCmd *)cmd;

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 0;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 1;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 2;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 1;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 4;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 1;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 30000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 0;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 2;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 2;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 2;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 4;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 2;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 30000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 0;
    stepProtCmd->protAmt = 3;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = idx % 2;
        protUnit->protId = 0x0403;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 3;
    stepProtCmd->protAmt = 3;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = idx % 2;
        protUnit->protId = 0x0403;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 6;
    stepProtCmd->protAmt = 1;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = 0;
        protUnit->protId = 0x0500;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 0;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 7;
    stepProtCmd->protAmt = 1;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = 0;
        protUnit->protId = 0x0501;
        protUnit->paramEnable = 0x03;
        protUnit->protParam[0] = 600;
        protUnit->protParam[0] = 300;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*托盘保护*/
    upProtGenCmd->trayIdx = 0;
    upProtGenCmd->mixProtAmt = 2;
    upProtGenCmd->protAmtTtl = 4;
    upProtGenCmd->protSeq = 0;
    upProtGenCmd->protAmt = 4;

    head->pldLen = sizeof(UpProtGenCmd);
    for (idx=0; idx<upProtGenCmd->mixProtAmt; idx++)
    {
        mixProt = (UpMixProt *)((u8 *)upProtGenCmd + head->pldLen);
        strcpy(mixProt->mixProtAby, "1&2@2");
        mixProt->mixProtLen = strlen(mixProt->mixProtAby);
        head->pldLen += sizeof(UpMixProt) + Align8(mixProt->mixProtLen);
    }
    
    protUnit = (UpProtUnit *)((u8 *)upProtGenCmd + head->pldLen);
    for (idx=0; idx<upProtGenCmd->protAmt; idx++,protUnit++)
    {
        protUnit->protId = 0x0300;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 600+idx;
    }
    head->pldLen += sizeof(UpProtUnit)*upProtGenCmd->protAmt;

    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtGen));
    printf("second send------\r\n");

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 0;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 1;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 2;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 1;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 4;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 1;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 30000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 0;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 2;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 2;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 2;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    cmd->trayIdx = 0;
    cmd->rsvd = 0;
    cmd->stepAmtTtl = 6;
    cmd->stepSeq = 4;
    cmd->stepAmt = 2;
    cmd->chnAmt = 1;
    cmd->chnId[0] = 2;
    
    head->pldLen = sizeof(UpFlowCmd);
    head->pldLen += Align16(cmd->chnAmt) * 2;
    head->pldLen += sizeof(UpStepInfo)*cmd->stepAmt;
    
    step = (UpStepInfo *)&cmd->chnId[Align16(cmd->chnAmt)];
    for (idx=0; idx<cmd->stepAmt; idx++,step++)
    {
        step->stepId = cmd->stepSeq+idx;
        step->stepType = StepTypeQuiet;
        step->paramEnable = 0x01;
        step->stepParam[0] = 30000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 0;
    stepProtCmd->protAmt = 3;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = idx % 2;
        protUnit->protId = 0x0403;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 10000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 3;
    stepProtCmd->protAmt = 3;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = idx % 2;
        protUnit->protId = 0x0403;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 20000 + idx*2000;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 6;
    stepProtCmd->protAmt = 1;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = 0;
        protUnit->protId = 0x0500;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 0;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*保护*/
    stepProtCmd->trayIdx = 0;
    stepProtCmd->protAmtTtl = 8;
    stepProtCmd->protSeq = 7;
    stepProtCmd->protAmt = 1;
    stepProtCmd->chnAmt = 0;
    
    head->pldLen = sizeof(UpProtStepCmd);
    head->pldLen += Align16(stepProtCmd->chnAmt) * 2;
    head->pldLen += sizeof(UpProtUnit)*stepProtCmd->protAmt;
    
    protUnit = (UpProtUnit *)&stepProtCmd->chnId[Align16(stepProtCmd->chnAmt)];
    for (idx=0; idx<stepProtCmd->protAmt; idx++,protUnit++)
    {
        protUnit->stepId = 0;
        protUnit->protId = 0x0501;
        protUnit->paramEnable = 0x03;
        protUnit->protParam[0] = 600;
        protUnit->protParam[0] = 300;
    }
    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep));

    /*托盘保护*/
    upProtGenCmd->trayIdx = 0;
    upProtGenCmd->mixProtAmt = 2;
    upProtGenCmd->protAmtTtl = 4;
    upProtGenCmd->protSeq = 0;
    upProtGenCmd->protAmt = 4;

    head->pldLen = sizeof(UpProtGenCmd);
    for (idx=0; idx<upProtGenCmd->mixProtAmt; idx++)
    {
        mixProt = (UpMixProt *)((u8 *)upProtGenCmd + head->pldLen);
        strcpy(mixProt->mixProtAby, "1&2@2");
        mixProt->mixProtLen = strlen(mixProt->mixProtAby);
        head->pldLen += sizeof(UpMixProt) + Align8(mixProt->mixProtLen);
    }
    
    protUnit = (UpProtUnit *)((u8 *)upProtGenCmd + head->pldLen);
    for (idx=0; idx<upProtGenCmd->protAmt; idx++,protUnit++)
    {
        protUnit->protId = 0x0300;
        protUnit->paramEnable = 0x01;
        protUnit->protParam[0] = 600+idx;
    }
    head->pldLen += sizeof(UpProtUnit)*upProtGenCmd->protAmt;

    simuSendMdMsg(txBuf, head->pldLen, UpCmdId(UpMsgGrpManu, UpMsgIdManuProtGen));
}

void testRowData()
{
    u8 rowData[] = {0x68, 0x79, 0x6e, 0x6e, 0xc0, 0x00, 0x01, 0x00, 0x00, 0x05,
    0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x88, 0x13,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
    0x0f, 0x00, 0xa0, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x93, 0x04, 0x00, 0x20, 0xc9,
    0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x01, 0x01, 0x00, 0x88, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x0f, 0x00, 0xa0, 0x86, 0x01, 0x00, 0x10, 0x27,
    0x00, 0x00, 0x60, 0xcc, 0x05, 0x00, 0x20, 0xc9, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x00, 0x88, 0x13,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
    0x3c, 0xbf };

    //upDataRecv(rowData, rowData, sizeof(rowData));

    u8 rowDataStep[] = {
        0x68, 0x79, 0x6E, 0x6E, 0x2C, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x01, 0x01, 0x00, 0x30, 0xE6, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0xED
    };

    upDataRecv(rowDataStep, rowDataStep, sizeof(rowDataStep));

    u8 rowDataProtGen[] = {
        0x68, 0x79, 0x6E, 0x6E, 0x3C, 0x01, 0x02, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 
        0x10, 0x00, 0x00, 0x00, 0x32, 0x30, 0x26, 0x31, 0x36, 0x40, 0x30, 0x3D, 0x30, 0x2C, 0x33, 0x3D, 
        0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x30, 0x75, 0x00, 0x00, 0x30, 0x75, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x30, 0x75, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0xD0, 0x07, 0x00, 0x00, 
        0x30, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x00, 0x87, 0x93, 0x03, 
        0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 
        0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x02, 0x00, 0x00, 0xFF, 
        0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 
        0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 
        0x03, 0x00, 0x00, 0xFF, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 
        0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 0x00, 0x87, 0x93, 0x03, 
        0x00, 0x87, 0x93, 0x03, 0x01, 0x01, 0x00, 0x01, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x0D, 0x64, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x00, 0x01, 
        0x22, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x03, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x9A, 0x8B
    };

    upDataRecv(rowDataProtGen, rowDataProtGen, sizeof(rowDataProtGen));

    u8 rowDataProtStep[] = {
        0x68, 0x79, 0x6E, 0x6E, 0x78, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x01, 0xC8, 0x0E, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x05, 0xFF, 0x01, 
        0xB0, 0x8F, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x04, 0x00, 0xBE, 0xEB
    };

    upDataRecv(rowDataProtStep, rowDataProtStep, sizeof(rowDataProtStep));

    u8 rowDataStack[] = {
        0x68, 0x79, 0x6E, 0x6E, 0x04, 0x02, 0x0C, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0xCF, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
        0xE1, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
        0xE6, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
        0xD4, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
        0xC3, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 
        0xCF, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 
        0xEB, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 
        0xEB, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
        0xF0, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 
        0xF6, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 
        0xEA, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 
        0xF0, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 
        0xE9, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 
        0xFB, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 
        0xDF, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 
        0x0D, 0x3E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 
        0x0D, 0x3E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 
        0x0C, 0x3E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 
        0x0D, 0x3E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 
        0x0D, 0x3E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 
        0x0C, 0x3E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 
        0x0D, 0x3E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 
        0x0D, 0x3E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 
        0x9E, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 
        0x98, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 
        0x82, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 
        0x98, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 
        0x93, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x00, 0x00, 0x00, 
        0x93, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 
        0x87, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 
        0x8D, 0x3D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x86
    };

    upDataRecv(rowDataStack, rowDataStack, sizeof(rowDataStack));

    return;
}

s32 main()
{

    ctrlInitApp();

    //printf("testChn---------------------\r\n");
    //testChnFlow();

    //printf("testTray----------------\r\n");
    //testTrayFlow();

    printf("testData----------------\r\n");
    testRowData();

    printf("test quit ok\r\n");
    return 0;
}
#endif

#endif
