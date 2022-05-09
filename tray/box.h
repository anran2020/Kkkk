
#ifndef _ITF_BOX_H
#define _ITF_BOX_H

#include "basic.h"
#if TRAY_ENABLE
#include "define.h"
#include "enum.h"
#include "type.h"
#include "func.h"
#include "log.h"
#include "flow.h"
#include "timer.h"
#include "entry.h"

/*下位机命令字,从下、主下、中位机命令字混排*/
/*暂时跟随，若下位机命令字太多，则需要hash映射,否则空函数太多*/
typedef u8 u8eBoxMsgId;
#define BoxMsgNull  0x00
#define BoxMsgConn  0x01
#define BoxMsgSmpl  0x02
#define BoxMsgCtrl  0x03
#define BoxMsgCaliNtfy 0x09
#define BoxMsgCaliStart 0x0a
#define BoxMsgCaliSmpl  0x0b
#define BoxMsgCaliKb 0x0c
#define BoxMsgCaliStop 0x0d
#define BoxMsgCfgRead 0x0e
#define BoxMsgCfgSet 0x0f
#define BoxMsgUpdCnfm  0x17
#define BoxMsgUpdSetup  0x18
#define BoxMsgUpdDld  0x19
#define BoxMsgUpdUpld  0x1a
#define BoxMsgCellSw 0x1d
#define BoxMsgCri  0x1e

typedef u8 u8eLowChnState;
#define LowChnStateIdle  0x00
#define LowChnStateRun  0x01
#define LowChnStateStart 0x02
#define LowChnStateStop 0x03
#define LowChnStateFault 0x04
#define LowChnStateCri  0x05

/*所有can消息通用消息头*/
typedef struct
{
    u8 msgFlowSeq;
    u8eBoxMsgId msgId;
    u16 msgLen;   /*自流水号(含)至负载最后字节(含)的总长度*/
    u8 payload[0];
}BoxMsgHead;

/*下位机通用应答*/
typedef struct
{
    u16 rspCode;
}BoxCmmnAck;

/*-----------------电源箱联机,开始------------------*/

typedef struct
{
    u8 ttlCellAmt;
    u8 useCellAmt;
    u16 rsvd;
    u32 useCellInd;  /*实际使用电芯指示bitmap*/
}ChnCellInd;

typedef struct
{
    u8 chnAmt;
    u8 lowWorkMode;  /*0--出厂出货模式,1--厂内备货模式*/
    u8 rsvd[2];
    ChnCellInd chnCellInd[0];
}BoxConnCmd;

typedef struct
{
    u8eBoxDevType devType;
    u8 softVer;
    u8 boxAddr;
    b8 online;
}SubBoxInfo;

typedef struct
{
    u32 timeStamp;
    u8eBoxDevType devType;
    u8 protoVer;
    u8 softVer;
    u8 chnAmt;   /*box内主通道数*/
    u8 chnModuAmt;   /*单通道模块数*/
    u8 chnCellAmt;   /*串联有效，box内总电芯数*/
    u8 volSmplAmt;
    u8 bypsSwAmt;
    u32 current;
    u32 voltage;
    SubBoxInfo boxSub[0];
}BoxConnAck;

/*-----------------电源箱联机负载,结束---------------------*/

/*-----------------电源箱采样命令响应负载,开始---------------------*/
typedef struct
{
    u16 lastErr;   /*0--succ,适用于偶尔丢帧但连接尚在，联机后复位*/
    u8 ndbdState;  /*针床状态,adbt下位机需求,不排除以后删除,1--压合,其余非压合*/
    u8 rsvd;
}BoxSmplCmd;

typedef struct
{
    u8 stepId;
    u8 stepType;  /*bitmap,高5bit为类型 u8eStepType,低3bit为子类型 u8eStepSubType*/
    u8eLowChnState chnLowState;
    u8 cause;
    u32 timeStamp;  /*ms*/
    s32 current;
    u32 capacity;
    s32 volCell;   /**/
    s32 volCur;   /*电流线探针连接处电压*/
    s32 volPort;
    s32 volInner;  /*电容电压*/
    s32 volBus;  /*母线电压,generatrix*/
    u32 tempPower;  /*功率管温度*/
}ChnSmplParall;

typedef struct
{
    s32 volCell;
    s32 volCur;
}CellSmplSeriesWoSw;

typedef struct
{
    u8eLowBypsSwSta swState;
    u8 cause;
    u16 rsvd;
    s32 current;
    u32 capacity;
    s32 volCell;
    s32 volCur;
}CellSmplSeriesWiSw;

typedef struct
{
    u8 stepId;
    u8 stepType;  /*bitmap,高5bit为类型 u8eStepType,低3bit为子类型 u8eStepSubType*/
    u8eLowChnState chnLowState;
    u8 cause;
    u32 timeStamp;  /*ms*/
    s32 current;
    u32 capacity;
    s32 volPort;
    s32 volInner;
    s32 volBus;
    u32 tempPower;
    CellSmplSeriesWoSw cellSmplWoSw[0];
    CellSmplSeriesWiSw cellSmplWiSw[0];
}ChnSmplSeries;

typedef struct
{
    u8eBoxDevType devType;
    b8 moreSmpl;  /*另有更多采样缓存可采*/
    u16 chnSelectInd;   /*bitmap指明哪些通道数据*/
    ChnSmplParall chnSmplParall[0];
    ChnSmplSeries chnSmplSeries[0];
}BoxSmplAck;
/*-----------------电源箱采样命令响应负载,结束---------------------*/

/*-----------------电源箱启停命令负载,开始---------------------*/
typedef struct
{
    u8 stepId;
    u8eStepType stepType;
    u8 paramInd;  /*bitmap,参数有效指示，1表明参数有效*/
    b8 cvFollowCc;  /*cc工步后是否为点位相同的cv工步,1为跟,0为不跟*/
    s32 curSet;
    s32 volSet;
    u32 timeStop;
    s32 volStop;   /*截止电流或截止电压，取决于工步类型*/
    u32 capStop;
}ChnCtrl;

/*下位机启停控制命令,适用通用应答*/
typedef struct boxCtrlCmdTag
{
    b8 isReTx;
    u8 rsvd[3];
    u16 chnSelectInd;  /*bitmap，通道选择指示，1--选，0--不选*/
    u16 chnCtrlInd;  /*bitmap,启停指示，1--起，0--停*/
    u8 cellCtrlInd[0];   /*bitmap,旁路串联有效,1--选,0--不选,按4对齐*/
    ChnCtrl chnCtrl[0];
}BoxCtrlCmd;

/*旁路串联主通道空闲时电芯切入切出命令,适用通用应答*/
typedef struct
{
    u16 chnSelectInd;  /*bitmap，通道选择指示，1--选，0--不选*/
    u16 chnCtrlInd;  /*bitmap,启停指示，1--切入，0--切出*/
    u8 cellCtrlInd[0];   /*bitmap,旁路串联有效,1--选,0--不选,按4对齐*/
}BoxSeriesSwCmd;
#endif
/*-----------------电源箱启停命令负载,结束---------------------*/

/*-----------------电源箱配置升级,开始---------------------*/

/*电源柜下位机配置读应答和写命令,读命令无负载,写应答适用通用*/
typedef u8 u8eBoxCfgType;
#define BoxCfgCurOfst 0x00  /*电流超差*/
#define BoxCfgPowerRevs 0x01  /*功率线反接*/
#define BoxCfgCellRevs 0x02  /*电芯采样线反接*/
#define BoxCfgCurRevs 0x03  /*探针采样线反接*/
#define BoxCfgVolUpLmt 0x04  /*电压上限*/
#define BoxCfgCell2Cur 0x05  /*电池-探针压差*/
#define BoxCfgCurPrec 0x06  /*电池-探针精度*/
#define BoxCfgPortPrec 0x07  /*电池-端口精度*/
#define BoxCfgVolRise 0x08  /*电压上升趋势,俩参数*/
#define BoxCfgCurDown 0x09  /*电流下降趋势,俩参数*/
#define BoxCfgCri 0x0a  /**/

#define BoxCfgBase 0x0100

typedef struct
{
    u32 funcSelect;
    u32 funcEnable;
    u32 param[0];
}BoxCfgInd;

typedef struct
{
    u8eBoxDevType boxDevType;
    u8 boxDevIdx;
    u8eUpdFileType fileType;
    u8 isUpload;
    u16 fileSize[2];
}BoxUpdSetupCmd;

typedef struct
{
    u8eBoxDevType boxDevType;
    u8 boxDevIdx;
    u8 bootVer;
    u8 rspCode;
    u16 pageSize;  /*最大传输分片*/
}BoxUpdSetupAck;

typedef struct
{
    u8eBoxDevType boxDevType;
    u8 boxDevIdx;
    u16 pldSeq;
    u16 pldSize;  /*有效数据长度*/
    u8 payload[0];
}BoxUpdDldCmd;

typedef struct
{
    u8eBoxDevType boxDevType;
    u8 boxDevIdx;
    u8 rspCode;
    u8 rsvd;
}BoxUpdDldAck;

typedef struct
{
    u8eBoxDevType boxDevType;
    u8 boxDevIdx;
    u16 fileSize[2];
    u16 pldSeq;
    u16 pldSize;
    u8 payload[0];
}BoxUpdUpldCmd;

typedef struct
{
    u8eBoxDevType boxDevType;
    u8 boxDevIdx;
    b8 upStopInd;
    u8 rsvd;
    u16 pldSeq;
}BoxUpdUpldAck;

typedef struct
{
    u8eBoxDevType boxDevType;
    u8 boxDevIdx;
    u8eUpdFileType fileType;
    u8 rsvd;
}BoxUpdCnfmCmd;

typedef struct
{
    u8eBoxDevType boxDevType;
    u8 boxDevIdx;
    u8 cnfmVer;
    u8 rspCode;
}BoxUpdCnfmAck;

/*-----------------电源箱配置升级,结束---------------------*/

/*-----------------电源箱(含子设备)修调,开始---------------------*/
/*修调进入和退出命令*/
typedef struct
{
    b8 caliEnable;
    u8 rsvd;
}BoxCaliNtfyCmd;

/*修调进入和退出响应*/
typedef struct
{
    u16 rspCode;
    b8 caliEnable;
    u8 rsvd;
}BoxCaliNtfyAck;

/*启动修调命令,适用通用响应*/
typedef struct
{
    u8 rsvd;
    u8 moduIdx;  /*box内全局模块号*/
    u16 chnIdx;  /*通道内子主统排索引*/
    u8eCaliMode caliMode;
    u8eCaliType caliType;
    u16 caliKbPoint[0];
}BoxCaliStartCmd;

/*停止修调命令和响应,均使用通用消息头和响应*/

/*修调采样命令*/
typedef struct
{
    u8 moduIdx;  /*box内全局模块号*/
    b8 smplAllSeries;
    u16 chnIdx;  /*通道内子主统排索引*/
}BoxCaliSmplCmd;

/*下位机串联子串电流电压套件,没有内部电压*/
typedef struct
{
    s32 current;
    s32 volCell;    /*电池电压，极柱电压，电压采样线电压*/
    s32 volCur;    /*电流探针电压*/
    s32 volPort;    /*设备端口电压*/
}BoxCellCurVol;

/*修调采样响应*/
typedef struct
{
    u16 rspCode;
    u8 moduIdx;  /*box内全局模块号*/
    b8 smplAllSeries;
    u16 chnIdx;  /*通道内子主统排索引*/
    u16 smplVal[0];
}BoxCaliSmplAck;

/*修调码更新命令,适用通用响应*/
typedef struct
{
    u8 rsvd1;
    u8 moduIdx;
    u16 chnIdx;  /*子主统排索引*/
    u8eCaliType caliType;
    u8eVolType volType;
    u8 caliKbAmt;
    u8 rsvd2;
    u16 caliKb[0];
}BoxCaliKbCmd;

typedef void (*BoxMsgProc)(Can *can, Box *, u8 *, u16);
typedef void (*BoxExprRxAck)(CanAuxCmdBuf *);
typedef void (*BoxSmplRcd)(Box *, u16, void *);
typedef void (*BoxSmplProc)(Box *, u16, u8 *);

typedef struct
{
    ListD auxBufList;
    BoxMsgProc boxMsgProc[BoxMsgCri];
    BoxExprRxAck boxAuxAckExpr[BoxMsgCri];
    BoxSmplProc boxSmplProc[SmplModeCri];
}CanMgr;

/*-----------------电源箱(含子设备)修调,结束---------------------*/

#ifdef __cplusplus
extern "C"  {
#endif

extern void boxSmplTxTry(Timer *timer);
extern void boxRxMsg(Box *box, u8 *data, u16 len);
extern void boxInit(void);
extern void boxCtrlTxTry(Box *box);
extern void boxCtrlCellTxTry(Box *box);
extern void boxCtrlAddChn(Box *box, u8 chnIdx, b8 isStart);
extern void boxCtrlAddSeriesCell(Box *box, u8 chnIdx, b8 isStart);
extern void boxAuxTxTry(u8 canIdx, void *buf);
extern void canTxMsg(u8 canId, u8 addr, u8 *data, u16 len);
extern void boxExprCtrlAck(Timer *timer);
extern void boxExprCellSwAck(Timer *timer);
extern void boxExprSmplAck(Timer *timer);
extern void boxExprAuxAck(Timer *timer);
extern void *allocAuxCanBuf(void);
extern void freeAuxCanBuf(void *buf);
extern void setCaliAuxCanBuf(CanAuxCmdBuf *auxBuf, Box *box, u16 upMsgId);
extern void canRxMsg(u8 canId, u8 addr, u8 *data, u16 len);
extern void traySmplMgrRst(Tray *tray);
extern void traySmplDefChnSmpl(Tray *tray, Channel *chn, u8eUpChnState state, u16eCauseCode cause);

#ifdef __cplusplus
}
#endif

#endif

