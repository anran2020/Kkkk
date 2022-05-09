

#ifndef __ENTRY_H__
#define __ENTRY_H__


#include <stdio.h>

#include "basic.h"
#if TRAY_ENABLE
#include "define.h"
#include "enum.h"
#include "type.h"
#include "func.h"
#include "timer.h"
#include "flow.h"

/*约束*/
/*每托盘最大32box*/
/*每box最大16通道*/
/*每通道最大32串*/
/*每can最大32box*/
/*设备最大48box??待确认*/
#define TrayBoxAmtMax  32
#define BoxChnAmtMax   16
#define ChnCellAmtMax  32
#define CanBoxAmtMax  32
#define DevBoxAmtMax  48

struct powerBoxTag;
struct trayTag;
struct channelTag;
struct cellTag;

/*以下结构中的指针都可改为索引,指针效率高但内存多,索引反之*/

/*并联的通道都是主通道.主通道指,并联通道,或,串联的主通道*/

typedef struct
{
    b8 tmprBeValid;
    u8 tmprInvalidCnt;
    u16 tmprVal;
}TmprData;

/*通道保护数据缓存,上次数据,必须记录,无犹豫,本次数据,是否记录?*/
/*记录的好处,通道模式和托盘模式采样的保护逻辑不用分开*/
/*不记录好处,每个通道节约20字节内存,也节省一丢拷贝时间*/
/*为避免麻烦,都记录.但若涉及内存紧张,这里可以权宜不记录*/
typedef struct
{
    u32 mixSubHpnBitmap;  /*bitmap,保护因子发生,下标MixSubCri,自维护,消警也清*/
    u32 mixSubHpnSec[MixSubCri];  /*保护因子时戳*/
    u16 newCauseCode;  /*记录后清零*/
    b8 prePowerBeValid;
    b8 newPowerBeValid;  /*确定采到样后置位,记录历史后清零*/
    TmprData cellTmprPre;
    TmprData cellTmprCrnt;
    s32 preCur;   /*做完保护后记录,指示上次电流*/
    s32 preVolCell;   /*做完保护后记录,指示上次采样电芯电压*/
    s32 newCur;
    s32 newVolCell;
    s32 newVolCur;
    s32 newVolPort;

    u32 idleTimeStampSec;  /*进入闲时的时间戳秒,自维护,消警更*/
    s32 idleVolBase;   /*闲时累加电压基点,自维护*/
    s32 quietVolDownBase;   /*静置电压下降电压基点,自维护*/
    s32 cccVolDownAbnmVolBase;
    u32 cccVolDownAbnmCapMin;
    s32 ccdVolRiseAbnmVolBase;
    u32 ccdVolRiseAbnmCapMin;
    s32 cvcCurRiseAbnmCurBase;
    u32 cvcCurRiseAbnmCapMin;
    u32 ccVolIntvlFluctTimeBaseMs;
    s32 ccVolIntvlFluctVolBase;
    u32 flowIdleVolIntvlRiseTimeBaseSec;
    s32 flowIdleVolIntvlRiseVolBase;
    u32 flowCcdcVolIntvlFluctTimeBaseMs;
    s32 flowCcdcVolIntvlFluctVolBase;
    b8 idleVolBaseValid;   /*闲时累加电压基点是否有效,消警空闲切换清,自维护*/
    u8 idleVolCtnuSmlRiseCnt;  /*消警切换压合清零,也自维护*/
    u8 idleVolCtnuSmlDownCnt;  /*消警切换压合清零,也自维护*/
    u8 idleTmprUpSmlCnt;  /*消警压合清零,也自维护*/
    u8 idleCurLeakCnt;  /*消警压合清零,也自维护*/
    u8 allChnTmprUpLmtCnt;  /*消警压合清零,也自维护*/
    u8 busyChnTmprLowLmtCnt;  /*消警压合清零,也自维护*/
    b8 quietVolDownBaseValid; /*静置电压下降基准电压是否有效,切换清,自维护*/
    b8 cccVolDownAbnmValid;  /*恒流充电压下降异常点是否有效,切换清,自维护*/
    u8 cccVolDownAbnmCnt;  /*自维护*/
    b8 ccVolIntvlFluctBaseValid;  /*电压间隔波动基点是否有效,切换清,自维护*/
    u8 cccVolRiseCtnuCnt;  /*切换清,也自维护*/
    u8 cccVolDownCtnuCnt;  /*切换清,也自维护*/
    b8 ccdVolRiseAbnmValid;  /*恒流放电压上升异常点是否有效,切换清,自维护*/
    u8 ccdVolRiseAbnmCnt;  /*自维护*/
    b8 cvcCurRiseAbnmValid;  /*恒压充电流上升异常点是否有效,切换清,自维护*/
    u8 cvcCurRiseAbnmCnt;  /*自维护*/
    b8 flowIdleVolIntvlRiseBaseValid;  /*流程闲时电压间隔上波动基点是否有效,消警空闲切换清,自维护*/
    b8 flowCcdcVolIntvlFluctBaseValid;  /*流程cc/dc电压间隔波动基点是否有效,切换清,自维护*/
    b8 ccdcVolBigDownValid;  /*是否进入容量区间,切换清,自维护*/
    b8 idleProtEna;  /*临时方案,压合后流程前不算闲时,不启动的通道不进闲时*/
}ChnProtBuf;

/*串联子电芯*/
typedef struct cellTag
{
    u16 cellIdxInTray;   /*tray内索引,不含主通道*/
    u16 genIdxInTray;  /*tray内子主统排索引*/
    u16 tmprIdx;  /*温度索引*/
    u8 cellIdxInChn;   /*通道内索引*/
    u8 lowCellIdxInChn;  /*下位机通道内电芯映射*/
    struct channelTag *chn;   /*所属通道*/
    u32 cellCapCtnu;   /*续接容量前工步已跑容量,旁路串联有效*/
    u32 cellCapFlowCrnt;   /*累积容量，不含当前工步,旁路串联有效*/
    u32 cellCapStep;   /*当前工步容量,含续接,旁路串联有效*/
    ChnProtBuf chnProtBuf;
    u32 cellStepRunTime;  /*旁路串联有效*/
    u8eLowBypsSwSta swState;  /*旁路串联有效*/
    u8 cellCause;  /*旁路串联有效*/
}Cell;

/*带旁路切换板的串联的控制数据*/
typedef struct
{
    b8 needStart;   /*仅用于启动/续接时筛选主通道,不参与流程*/
    b8 hasCtnuStepId;  /*用于续接时查找工步号*/
    u8 rsvd[2];
    u32 startCell;  /*上位机启动或续接的电芯*/
    u32 runCell;   /*预期切入运行的电芯,保护逻辑时以通道运行态为前提*/
    u32 waitCell;   /*等待参与流程的电芯*/
    u32 endingCell;  /*正在工步截止的电芯,最后一条运行数据,是runCell的特殊子集*/
    u32 nmlEndCell;  /*正常已工步截止的电芯,以通道运行态为前提*/
    u32 stopedCell;  /*保护或指令停止的电芯*/
    u32 pausedCell;  /*保护或指令暂停的电芯*/
    u32 stopingCell; /*一次命令下位机截止的电芯,流程内保持增量*/
    u32 idleSwCell;   /*仅用于空闲时切入切出,不参与流程*/
    u32 idleCutInCell;   /*空闲时切入记录,除判断闲时保护外不参与流程*/
}BypsSeriesInd;

/*循环运行数据*/
typedef struct
{
    u8 loopStepId; /*循环工步自身工步ID*/
    u8 jumpStepId; /*跳转到哪个工步ID,与上位机收发时,该字段保留*/
    u16 jumpAmt;   /*循环剩余次数*/
}LoopDscr;

/*主通道,含并联*/
typedef struct channelTag
{
    u16 chnIdxInTray;   /*tray内主通道索引*/
    u16 genIdxInTray;  /*tray内子主统排索引*/
    u16 tmprIdx;  /*温度索引*/
    u8 lowChnIdxInBox;  /*下位机通道映射*/
    u8 chnIdxInBox;    /*box内主通道索引*/
    u8 chnCellAmt;    /*通道内电芯数量,串联时有效,不含预留*/
    u8 chnStepId; /*当前工步号*/
    u8eStepType chnStepType;  /*工步类型*/
    u8eChgType chgTypePre;  /*记录充放电类型,用于累积*/
    u8eStepSubType stepSubType;
    u8eMedChnState chnStateMed;
    u8 lowCause;  /*下位机上送的异常码*/
    u8eStepType upStepType;  /*给上位机的工步类型,同上,跟着工步号走*/
    u8 dynStaCnt;  /*不稳定状态计数,用于防呆,进入非稳定态时清零*/
    u8 noDataCnt;  /*采样有回复但无数据次数*/
    b8 smplPres;  /*托盘采样有效,采样生成后清零,是否产生通道数据*/
    b8 beWiNp;  /*是否占用了负压资源*/
    u16 tmprPower;  /*功率管温度*/
    struct powerBoxTag *box;     /*所属电源箱*/
    s32 volInner;  /*内部电压*/
    s32 volBus;  /*母线电压*/
    u32 capCtnu;   /*续接前工步已跑容量*/
    u32 capLow;  /*下位机上传容量,不含续接*/
    u32 capStep;   /*当前工步容量,含续接*/
    u32 capFlowCrnt;   /*当前电流方向累积容量，不含当前工步容量*/
    u32 capChgTtl;   /*充电总容量,含累积容量,不含当前工步容量*/
    u32 capDisChgTtl;   /*放电总容量,含累积容量,不含当前工步容量*/
    u32 stepRunTime;  /*工步运行时间,不含续接*/
    u32 stepRunTimeCtnu;  /*续接时已跑时间*/
    u32 stepRunTimeBase;  /*记录工步转运行态的下位机时间戳ms*/
    Cell *cell;
    BypsSeriesInd *bypsSeriesInd;  /*带旁路切换时非空,否则为空*/
    FlowStepEntry *flowStepEntry;  /*流程入口*/
    FlowProtEntry *flowProtEntry;  /*流程保护入口,不含工步保护*/
    StepNode *crntStepNode;   /*上电后流程前以及流程正常结束后为空*/
    ChnProtBuf chnProtBuf; /*保护过程数据*/
    LoopDscr loopDscr[LoopAmtMax];  /*循环工步维护*/
}Channel;

typedef struct
{
    u16 chnSelectInd;  /*bitmap，通道选择指示，1--选，0--不选*/
    u16 chnCtrlInd;  /*bitmap,启停指示，1--起，0--停*/
}BoxCtrlInd;

typedef struct
{
    ChainD chain;
    BoxCtrlInd boxCtrlInd;
}BoxCtrlWaitSend;

/*库位维护标志*/
/*上位机发起的离开,通知级联并收到应答后清维护*/
/*防呆触发的离开,先清维护再通知级联*/
/*这个逻辑用于区分收到级联应答时是否转给上位机*/
typedef u8 u8eBoxWorkMode;
#define BoxModeManu  0x00  /*生产*/
#define BoxModeMntnFixt  0x01  /*工装维护*/
#define BoxModeMntnCali  0x02  /*修调维护*/
#define BoxModeMntnBoxUpd  0x04  /*电源升级*/
#define BoxModeMntnNdbdUpd  0x08  /*针床升级,待补充*/

typedef struct powerBoxTag
{
    u8 boxIdxInTray;   /*tray-box-idx*/
    u8 addr;     /*通讯地址,即拨码*/
    u8eBoxWorkMode boxWorkMode;  /*维护时不保护电源柜数据*/
    u8 canIdx;   /*所属can口*/
    struct trayTag *tray;  /*所属tray*/
    b8 online;
    u8 chnAmtTtl;  /*满配主通道数量*/
    u8 boxCellAmt;  /*实际电芯数量,串并都用*/
    u8 chnModuAmt;  /*每个通道的模块数量*/
    u8 softVer;
    u8eBoxType boxType;
    u8 bypsSwAmt;  /*旁路切换板数量,目前旁路串联有*/
    u8 volSmplAmt; /*电压采样板数量,目前极简串联有*/
    u8 reTxSmplCnt;  /*重传采样指令次数,每周期算1次,不含第1次*/
    b8 reTxSmplCmd;  /*重传采样指令标记*/
    u8 reTxCtrlCnt;  /*重传次数,不含第一次*/
    b8 boxHasSmplTry;  /*托盘采样有效,记录是否尝试采样,托盘采样生成后清零*/
    b8 moreSmplPres;  /*是否2次采样,应答或超时清零,当下机制最多采2次*/
    u32 maxVol;
    u32 maxCur;
    u8 boxChnAmt;  /*实际使用通道数量,不含预留*/
    Channel *chn;  /*非并箱时箱内通道，并箱时所属通道*/
    BoxCtrlWaitSend seriesSwWaitSend;  /*空闲时切入切出控制,入链有效,发送后出链复位*/
    BoxCtrlWaitSend ctrlWaitSend;  /*等待发送的控制,入链有效,发送后出链复位*/
    BoxCtrlInd ctrlWaitAck;  /*已发送但等待应答的控制*/
}Box;

/*按托盘的针床数据*/
typedef struct
{
    s16 status[Align16(NdbdSenCri)];
    s16 warn[Align16(NdbdWarnCri)];
    u8eNdbdStaType bit2IdxSta[Align8(BitSenCri)];
    u8eNdbdWarnType bit2IdxWarn[Align8(BitWarnCri)];
    u8 tmprAmtPerCell;  /*每电芯占据温度通道个数*/
    u8 slotTmprAmt;  /*库位温度个数*/
    u8 slotWaterTmprAmt;  /*库位水温个数*/
    b8 ndbdDataValid;  /*数据是否有效,与plc/单片机在线有关*/
    u32 ndbdSmplTsSec;  /*记录采到针床数据的时间戳,仅用于保护时判断是否同一个采样*/
    u16 *cellTmpr;
    u16 *slotTmpr;
    u16 *slotWaterTmpr;
}NdbdData;

typedef struct
{
    u8eCmmuItfType cmmuItfType;
    u8 devType;
    u8 rsvd[2];
}UpdTypeMap;

typedef struct
{
    u16 idxInTray;
    u8 softVer;
    b8 online;
    u8 addr;
    u8eBoxDevType subType;
    u8 rsvd[2];
}SubBox;  /*下位机级联子设备,电压采样板和旁路切换板*/

typedef struct
{
    u32 maxTrayVol;
    u32 maxTrayCur;
    u8eBoxType boxType;
    u8 chnAmtConn;  /*满配主通道数量*/
    u8 chnModuAmt;  /*每个通道的模块数量*/
    u8 bypsSwAmt;  /*旁路切换板数量,目前旁路串联有*/
    u8 volSmplAmt; /*电压采样板数量,目前极简串联有*/
    u8 protoVerMin;
    u8 chnCellAmt;  /*满配单通道电芯数*/
    u8 rsvd;
}TrayBoxCfg;

typedef struct
{
    u8eNpType npType;
    b8 beReach;
    u16 refCnt;
    s16 npExpect;
    s16 npMax;
    s16 npMin;
    u16 mkNpExprSecT00;  /*抽真空超时*/
    u16 npLmtExprSecT06;  /*工步负压异常*/
    u16 closeBrkDelaySecT08;  /*关闭微正压延时*/
    u16 brkNpExprSecT10;  /*破真空超时,常压判定*/
    u16 ndbdBrkWaitNpSecT07; /*针床脱开前等待破真空时间上限*/
    u32 baseTimeSecT07;  /*T07的基准起始时间*/
    u32 npReqTimeStampSec;  /*通知针床负压起始时间*/
    u32 npOverLmtTimeStampSec;  /*工步中负压超限起始时间*/
}TrayNpMgr;

/*关于脱机,以下3条满足其一即不产生采样,当然,要先保护*/
/*1/中位机上电后,连到上位机之前*/
/*2/脱机30分钟*/
/*3/上位机最后一条确认序号不能被覆盖,下次采样为上次的确认*/
/*上述3举例,上位机刚读走5,则4是最后一条确认序号,4不能被覆盖*/
typedef struct
{
    b8 isLooped;  /*磁盘存储是否转圈了*/
    u8 smplTryBoxAmt;  /*托盘采样有效,尝试采样的box数量,采样生成的逻辑依据*/
    b8 smplEnable;  /*是否允许产生采样,脱机相关*/
    b8 upDiscExpr;  /*上位机脱机超时*/
    u16 smplItemSize;  /*单采样实际大小*/
    u16 smplItemDiskSize;  /*单采样占据磁盘空间,可能大于实际*/
    u32 smplSeqNext;  /*待产生的采样序号*/
    u32 smplSeqUpReq;  /*上位机请求序号,即未确认序号,其前一条不能被覆盖*/
    u32 smplSeqMax;   /*合法的最大序号*/
    u32 smplDiskAddrBase;  /*磁盘存储基址*/
    u32 smplDiskAddrMax;  /*磁盘最后有效地址,之后回头*/
    u32 smplDiskAddr;  /*磁盘存储待写入地址*/
    u32 smplDiskAmt;  /*磁盘存储采样数量*/
    u8 *smplBufAddrBase;  /*缓存采样基址*/
    u8 *smplBufAddrMax;  /*缓存最后有效地址,之后回头*/
    u8 *smplBufAddr;  /*缓存采样待写入地址*/
    u32 smplBufAmt;  /*缓存采样数量*/
    u16 smplChnAmt;  /*通道计数,一轮下来没有通道数据就不产生采样*/
    b8 upSmplRstInd;  /*指示上位机采样复位*/
    b8 auxDataDone;  /*针床温度等辅助数据是否准备,托盘采样生成后清零*/
}SmplSaveMgr;

typedef struct
{
    u8eProtPolicy policyId;
    u8 rsvd;
    u16 delaySec;  /*出于空间存储为秒*/
}MixPolicy;

typedef struct
{
    u16 suffixOfst;
    u8 suffixLen;
    u8 policyAmt;
    u16 policyOfst;
    u16 rsvd;
}MixExp;

#define MixExpAmt  16
#define TtlExpSize  1024
#define TtlPolicySize  1024
typedef struct
{
    u8 suffix[TtlExpSize];
    u8 policy[TtlPolicySize];  /*MixPolicy*/
    MixExp mixExp[MixExpAmt];  /*配置的表达式*/
    u32 lifePeriodSec[MixSubCri];  /*生命周期,存储为秒*/
    u8 mixExpAmt;  /*上位机配置表达式数量*/
    u8 rsvd;
    u16 suffixOfst;  /*解析时存放位置*/
    u16 policyOfst;  /*解析时存放位置*/
}MixProtCfg;

typedef struct
{
    u32 mixProtHpn[MixExpAmt];  /*非零则组合发生,消警清零,低24bit分四组记录为真的因子*/
    b8 policyActOver[Align8(PolicyCri)];  /*已经做完,消警清零*/
    b8 policyActNeed[Align8(PolicyCri)];  /*需要做(含已做),消警清零*/
    u32 policyActSec[PolicyCri];
    u32 newNdbdNpTs;  /*负压时间戳*/
    u32 newNdbdNpTsBak;  /*应客户要求,负压突升3个点才算,自维护*/
    u16eCauseCode trayCauseNew;   /*整盘保护,生成采样后清零,不能在逻辑前清*/
    u16eCauseCode trayCausePre;   /*整盘保护,消警压合清零*/
    s16 npBigRiseBase;  /*应客户要求,负压突升3个点才算,自维护*/
    s16 preNdbdNp;  /*负压都带符号*/
    s16 newNdbdNp;
    s16 newFanWarn;
    s16 newGasWarn;
    s16 newSmokeWarn;
    s16 newCoWarn;
    b8 policyNdbdBrkNeed;    /*针床脱开特殊,自维护,保护策略执行后清零*/
    b8 preNdbdValid;    /*上次针床数据是否有效,自维护,压合延时后清零*/
    b8 newNdbdValid;    /*本次次针床数据是否有效,自维护*/
    b8 npWiSw;   /*是否正在负压切换,自维护*/
    u8 npBigRiseCnt;  /*应客户要求,负压突升3个点才算,自维护*/
    u8 smokeCtnuHpnCnt;  /*消警压合清零,也自维护*/
    u8 coCtnuHpnCnt;  /*消警压合清零,也自维护*/
    u8 allSlotTmprUpLmtCnt;  /*消警压合清零,也自维护*/
    u8 busySlotTmprLowLmtCnt;  /*消警压合清零,也自维护*/
    u8 allChnTmprUpLmtPointAmt;  /*自维护,生成托盘采样后清零,超限通道数*/
    TmprData slotTmprPre;  /*上一数据的有效性*/
    TmprData slotTmprCrnt;  /*保护逻辑前实时*/
    Timer protPolicyTmr;  /*保护动作延时*/
}TrayProtMgr;

#define SpecsMapAmt 8
typedef struct
{
    u16 noUsedChnAmt;
    u16 rsvd;
    u16 noUsedChnIdx[16];
}SpecsMap;


/*整体不保护的bitmap*/
typedef u8 u8eProtDisable;
#define ProtDisableTouch 0x01  /*未压合不保护*/
#define ProtDisableMntn 0x02  /*修调工装等维护不保护*/

typedef struct trayTag
{
    u8 trayIdx;
    u8 boxAmt;
    u8 canAmt;   /*tray的box未必都在一条can*/
    u8 fixtUartCmmuAddr;  /*工装通讯地址,使能时初始化*/
    u8 tmprSmplAmt;  /*温度盒数量*/
    u8 tmprSmplIdxBase;  /*托盘首个温度盒索引,dev-idx*/
    u8 ndbdCtrlByMcu;  /*0--plc,1--mcu*/
    u8 plcIdx;  /*所处plc-idx*/
    u8eProtDisable protDisable;  /*托盘压合后2秒开始保护*/
    b8 trayWarnPres;  /*保护或防呆后置位,只有上位机消警才清零*/
    u8eBoxWorkMode trayWorkMode;  /*维护时不保护*/
    u16 trayChnAmt;  /*实际主通道数量,不含预留*/
    u16 trayCellAmt;  /*实际电芯数量*/
    u16 genChnAmt; /*实际主/子不分的通道数量,不含预留*/
    Box *box;
    Channel *chn;  /*不含预留*/
    Cell *cell;
    NdbdData ndbdData;
    UpdTypeMap updTypeMap[UpUpdDevCri];
    SubBox *volSmpl;
    SubBox *bypsSw;
    TrayBoxCfg boxCfg;  /*托盘内的box统一配置信息*/
    TrayNpMgr npMgr;
    SmplSaveMgr smplMgr;
    MixProtCfg mixProtCfg;
    TrayProtMgr trayProtMgr;
    Timer protEnaTmr;  /*保护使能延时*/
    Timer npSwRstDelayTmr;  /*针床脱开恢复负压开关延时*/
    Timer npSwProtDelayTmr;  /*负压切换保护延时*/
    Timer ndbdBrkWaitNpTmr;  /*针床脱开等破真空延时*/
    Timer mntnExprTmr;  /*维护防呆定时*/
    TrayProtEntry *trayProtEntry;  /*托盘保护*/
    FlowRecvCtrl *flowRecvCtrl;  /*控制流程和保护接收*/
}Tray;

/*收到上位机消息后的应答有两类,立即应答和延后应答*/
/*立即应答,定义为与下位机无关，读取flash虽要阻塞延后，但属于立即类*/
/*延后应答,定义为与下位机有关，需等下位机应答后再应答上位机*/
/*对于延后应答,都用下结构来缓存下位机命令,即便能立即发送*/
/*can辅助消息缓存,综合现有特征,按设备整体缓存,不按can,tray,box等*/
#define CanAuxBufSize 1312
#define CanAuxBufAmt  4
typedef struct
{
    ChainD chain;
    Box *box;
    u8 reTxCnt;  /*剩余重传次数,0就不重传了*/
    u8 rsvd[3];
    u16 upMsgFlowSeq;
    u16 upCanMsgId; 
    u8 msgBuf[CanAuxBufSize];
}CanAuxCmdBuf;

#define CanAddrWidth 5  /*目前使用5bit位宽拨码地址*/
#define CanAddrMaxCri (1 << (CanAddrWidth))
typedef struct canTag
{
    u8 canIdx;  /*dev-can-idx*/
    u8 boxAmt;
    u8 smplCanBoxIdx;  /*can-box-idx, 应答或超时后更新*/
    u8 canMsgFlowSeq;  /*应答或超时后更新*/
    u8 msgIdHasTx;
    b8 smplAgainAct;  /*执行一轮中第二次采样*/
    b8 smplAgainNeed;  /*需要一轮中第二次采样*/
    u8 can2DevBoxIdx[CanBoxAmtMax];  /*can-box-idx到dev-box-idx映射*/
    u8 addr2DevBoxIdx[CanAddrMaxCri];  /*addr到dev-box-idx映射*/
    Timer waitCanAckTmr;
    Timer canSmplLoopTmr;  /*采样定时器每can独立*/
    ListD ctrlWaitList;  /*BoxCtrlWaitSend, 等待发送的控制*/
    ListD ctrlCellWaitList;  /*BoxCtrlCellWaitSend, 等待发送的电芯切入切出*/
    ListD auxWaitList;  /*CanAuxCmdBuf,等待发送的辅助类消息*/
    CanAuxCmdBuf *waitAckAuxCmd; /*已发送，等应答中*/
    Box *boxWaitCtrlAck;
}Can;

#define UartBufSize 256
#define UartBufAmt  4
typedef struct
{
    ChainD chain;
    u8 trayIdx;
    u8 reTxCnt;  /*剩余重传次数*/
    u8 uartCmmuAddr;  /*通讯地址*/
    u8 rsvd;
    u16 upMsgFlowSeq;
    u16 upUartMsgId;
    u8 msgBuf[UartBufSize];
}UartBlockCmdBuf;

typedef struct
{
    u8 tmprSmplDevIdx;   /*dev-idx*/
    u8 actAddr; /*实际拨码地址,不含基址*/
    u8 cmmuAddr;  /*协议通讯地址,含基址*/
    b8 online;  /*在线与否,不影响转发消息*/
    u8 onlineDelayCnt; /*联机后延时采样*/
    u8 smplExprCnt; /*采样超时次数*/
    u8 devVer;
    u8 tmprAmt4Cell;  /*实际有效数量,不含预留*/
    u8 tmprBase4Cell;
    u8 tmprAmt4Loc;  /*实际有效数量,不含预留*/
    u8 tmprBase4Loc;
    u8 tmprAmt4Water;  /*水温,实际有效数量,不含预留*/
    u8 tmprBase4Water;
    u8 genWaterTmprIdx;  /*温度盒到水温映射*/
    u8 genSlotTmprIdx;  /*温度盒到库温映射*/
    u16 genCellTmprIdx;  /*温度盒到电芯映射*/
    u16 tmprChnAmt;   /*温度盒通道数量,含预留*/
    u16 crntSmplChnIdx;   /*当前采样的温度盒通道索引*/
    u8 uartIdx;  /*所在uart索引*/
}TmprSmpl;

typedef struct
{
    u8 touchAct;  /*针床压合0--忽略,1--压合,2--分离*/
    u8 ndbdWarnDel; /*消警0--忽略,1--消警*/
    u8 fireDoorAct;  /*消防门0--忽略,1--关闭,2--打开*/
    u8 smokeAct;  /*烟道0--忽略,1--关闭,2--打开*/
    u8 slotFanAct;  /*库位风扇0--忽略,1--关闭,2--打开*/
    u8 npHigh;  /*高负压0--忽略,1--关闭,2--打开*/
    u8 npLow;  /*低负压0--忽略,1--关闭,2--打开*/
    u8 vacumBrk;  /*破真空0--忽略,1--关闭,2--打开*/
    u8 npRatio;  /*比例阀0--忽略,1--关闭,2--打开*/
    u8 fixtPower;  /*工装供电0--忽略,1--关闭,2--打开*/
    s16 npVal;  /*负压值,区分正负,npRatio打开有效*/
}NdbdMcuCtrl;

typedef struct
{
    ChainD chain;
    NdbdMcuCtrl ctrl;
}NdbdMcuCtrlNode;

/*针床主控盒*/
typedef struct
{
    u8 ndbdMcuIdx;   /*dev-idx,针床主控索引严格映射托盘索引*/
    u8 actAddr; /*实际拨码地址,不含基址*/
    u8 cmmuAddr;  /*协议通讯地址,含基址*/
    b8 online;  /*在线与否,不影响转发消息*/
    u8 onlineDelayCnt; /*联机延时后采样*/
    u8 smplExprCnt; /*超时次数,用于判断离线*/
    u8 ctrlReTxCnt; /*控制命令重传次数*/
    u8 softVer;
    u8 uartIdx;  /*所在uart索引*/
    NdbdMcuCtrl ctrlWaitAck;
    NdbdMcuCtrlNode ctrlWaitTx;
}NdbdMcu;

#define UartAddrWidth 4  /*最大位宽拨码地址*/
#define UartAddrCri (1 << (UartAddrWidth))
#define UartStaActAddrMask  0x0f
#define FixtAddrWidth 2
#define FixtAddrCri (1 << (FixtAddrWidth))
#define UartDynActAddrMask  0x03
typedef struct
{
    u8 uartIdx;
    u8 uartMsgFlowSeq; /*应答或超时后更新*/
    u8eUartDevType uartCrntSmplType;  /*当前采样设备类型,应答或超时更新*/
    u8 uartCrntSmplDevIdx;  /*当前采样设备索引,应答或超时更新*/
    u8 mcuIdxWaitCtrlAck;  /*发送控制等待应答的索引,自维护*/
    u8 uartSmplDevAmt[Align8(UartNeedSmplCri)];
    u8 uartStaAddr2DevIdx[UartStaticCri][UartAddrCri];
    Timer waitUartAckTmr;
    Timer uartSmplLoopTmr;  /*采样定时器每串口独立*/
    ListD uartBlockList;  /*UartBlockCmdBuf,等待发送的消息*/
    UartBlockCmdBuf *waitAckBlockBuf; /*已发送，等应答中*/
    ListD ctrlWaitList;  /*NdbdMcuCtrlWaitTx, 等待发送的控制*/
}Uart;

typedef struct
{
    b8 updWorking;
    u8eUpUpdDevType devType;
    u8eUpdFileType fileType;
    u8 isUpload;
    u8 trayIdx;
    u8 rsvd[3];
    u16 devIdx;
    u16 pageSize;
    u32 fileSize;
}UpdMgr;

typedef struct
{
    u8 trayAmt;
    u8 canAmt;
    u8 uartAmt;
    u8 fixtUartIdx;   /*工装所在串口*/
    u8eSmplMode smplMode;
    b8 needConnUp;  /*是否需要给上位机发联机*/
    u16 cellTmprAmt; /*按满配电芯计算的用于电芯的温度个数,含1芯多温*/
    u8 slotTmprAmt; /*同上*/
    u8 waterTmprAmt;  /*水温*/
    Cell *cell;
    Channel *chn;
    Box *box;
    Tray *tray;
    Can *can;
    Uart *uart;
    TmprSmpl *tmprSmpl;
    NdbdMcu *ndbdMcu;
    u16 *genCellTmpr;
    u16 *genSlotTmpr;
    u16 *genSlotWaterTmpr;
    u32 tmprSmplTsSec;  /*记录采到温度数据的时间戳,仅用于保护时判断是否同一个采样*/
    Timer devAppRstTmr;
    u8eUpChnState chnStaMapMed2Up[Align8(ChnMedStaCri)];
    SpecsMap specsMap[SpecsMapAmt];
}DevMgr;

extern DevMgr *gDevMgr;
extern u16 gProtoSoftVer;
extern u16 gMediumSoftVer;
extern u16 gProtoSmplVer;
extern u8 gLowProtoVer;
extern u8 gNdbdMcuProtoVer;
extern u8eBoxType gTrayBoxType;
extern u8 gUsedMode;

#ifdef __cplusplus
extern "C"  {
#endif

extern void ctrlInitApp(void);
extern void ctrlInitBoot(void);


#ifdef __cplusplus
}
#endif

#endif
#endif

