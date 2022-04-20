

#ifndef _ITF_UPPER_H_
#define _ITF_UPPER_H_

#include "basic.h"
#if TRAY_ENABLE
#include "define.h"
#include "enum.h"
#include "type.h"
#include "cause.h"
#include "flow.h"
#include "timer.h"
#include "entry.h"


/*上位机消息类型定义：msg group + (msg type +) msg id*/

typedef u8 u8eUpMsgGrp;
#define UpMsgGrpManu     0x00   /*电源柜生产*/
#define UpMsgGrpUpdate    0x01   /*配置升级*/
#define UpMsgGrpCali      0x02   /*电源柜修调*/
#define UpMsgGrpNdbd    0x03   /*针床控制*/
#define UpMsgGrpFixt   0x04   /*工装控制*/
#define UpMsgGrpDbg   0x05   /*调试消息*/
#define UpMsgGrpCri       0x06

#define UpMsgGrpOffset   8  /*消息类别偏移*/
#define UpMsgIdMask        0xff  /*消息类型掩码,工装为高4bit type+低4bit id*/
#define UpMsgGrpBase(grp)  ((grp)<<UpMsgGrpOffset)

/*上位机消息类型*/
/*量产电源柜命令类*/
/*对应协议*/
typedef u8 u8eUpMsgIdManu;
#define UpMsgIdManuConn         0x00   /*联机*/
#define UpMsgIdManuFlow         0x01
#define UpMsgIdManuProtGen     0x02   /*全程或全时保护*/
#define UpMsgIdManuProtStep     0x03  /*特定工步和全局工步*/
#define UpMsgIdManuSmplChnl     0x04
#define UpMsgIdManuSmplNdbd     0x05
#define UpMsgIdManuSmplStack   0x06
#define UpMsgIdManuSmplTray     0x07
#define UpMsgIdManuStart        0x08
#define UpMsgIdManuPause        0x09
#define UpMsgIdManuStop         0x0a
#define UpMsgIdManuJump         0x0b
#define UpMsgIdManuCtnuStack      0x0c
#define UpMsgIdManuKeepAlive      0x0d
#define UpMsgIdManuWarnDel        0x0e
#define UpMsgIdManuCtnu         0x0f
#define UpMsgIdManuCri          0x10

/*配置升级命令类*/
typedef u8 u8eUpMsgIdUpdate;
#define UpMsgIdUpdCfgRead    0x00
#define UpMsgIdUpdCfgSet     0x01
#define UpMsgIdUpdSetup      0x02
#define UpMsgIdUpdDld        0x03
#define UpMsgIdUpdUpld        0x04
#define UpMsgIdUpdCnfm      0x05
#define UpMsgIdUpdCri        0x06

/*电源柜修调命令类*/
typedef u8 u8eUpMsgIdCali;
#define UpMsgIdCaliNtfy   0x00
#define UpMsgIdCaliStart    0x01
#define UpMsgIdCaliSmpl     0x02
#define UpMsgIdCaliKb   0x03
#define UpMsgIdCaliCri      0x04

/*针床外设控制命令类*/
typedef u8 u8eUpMsgIdNdbd;
#define UpMsgIdNdbdAct   0x00
#define UpMsgIdNdbdNp    0x01
#define UpMsgIdFixtPower  0x02
#define UpMsgIdNdbdCri   0x03

/*工装控制命令类，工装协议类型,工装有细分，便于扩展*/
typedef u8 u8eUpFixtProtoType;
#define FixtProtoPrec   0x00  /*精度线序工装,并联串联统一*/
#define FixtProtoTmpr    0x01  /*温度线序工装*/
#define FixtProtoClean   0x02  /*负压清洗工装*/
#define FixtProtoGas   0x03  /*气密性工装*/
#define FixtProtoFlow   0x04  /*负压流量工装*/
#define FixtProtoSuctIn   0x05  /*插吸嘴工装*/
#define FixtProtoSuctOut   0x06  /*拔吸嘴工装*/
#define FixtProtoLocat   0x07  /*定位工装*/
#define FixtProtoCri   0x08

#define UpMsgFixtTypeOffset   4  /*消息类别偏移*/
#define UpMsgIdFixtMask        0x0f  /*消息类型掩码*/
#define UpMsgIdFixtBase(type) ((type)<<UpMsgFixtTypeOffset)

typedef u8 u8eUpMsgIdFixtPrec;
#define UpMsgIdFixtNtfy   0x00   /*所有工装使能通用*/
#define UpMsgIdFixtPrecChnlSw    0x01  /*精度工装通道切换*/
#define UpMsgIdFixtPrecSmpl     0x02  /*采样*/
#define UpMsgIdFixtPrecOut   0x03  /*精度工装输出*/
#define UpMsgIdFixtPrecCri      0x04

typedef u8 u8eUpMsgIdFixtTmpr;
#define UpMsgIdFixtTmprHeat   0x00  /*加热启停*/
#define UpMsgIdFixtTmprSmpl    0x01
#define UpMsgIdFixtTmprCri    0x02

typedef u8 u8eUpMsgIdFixtClean;
#define UpMsgIdFixtCleanAct    0x00  /*清洗动作*/
#define UpMsgIdFixtCleanCri    0x01

typedef u8 u8eUpMsgIdFixtGas;
#define UpMsgIdFixtGasSmpl   0x00  /*气密性工装采样*/
#define UpMsgIdFixtGasCri    0x01

typedef u8 u8eUpMsgIdFixtFlow;
#define UpMsgIdFixtFlowSmpl   0x00  /*流量工装采样*/
#define UpMsgIdFixtFlowCri    0x01

typedef u8 u8eUpMsgIdFixtSuctIn;
#define UpMsgIdFixtSuctInSmpl   0x00  /*插吸嘴采样*/
#define UpMsgIdFixtSuctInCri    0x01

typedef u8 u8eUpMsgIdFixtSuctOut;
#define UpMsgIdFixtSuctOutAct    0x00  /*拔吸嘴动作*/
#define UpMsgIdFixtSuctOutSmpl   0x01  /*拔吸嘴采样*/
#define UpMsgIdFixtSuctOutCri    0x02

typedef u8 u8eUpMsgIdFixtLocat;  /*定位工装*/
#define UpMsgIdFixtLocatSmpl   0x00  /*采样*/
#define UpMsgIdFixtLocatAct    0x01
#define UpMsgIdFixtLocatCri    0x02

#define UpMsgIdFixt(type, id) (UpMsgIdFixtBase(type) + (id))
#define UpCmdId(group, id) (UpMsgGrpBase(group) + (id))

#define UpCmdIdManuConn  (UpCmdId(UpMsgGrpManu, UpMsgIdManuConn))
#define UpCmdIdManuFlow  (UpCmdId(UpMsgGrpManu, UpMsgIdManuFlow))
#define UpCmdIdManuProtGen  (UpCmdId(UpMsgGrpManu, UpMsgIdManuProtGen))
#define UpCmdIdManuProtStep  (UpCmdId(UpMsgGrpManu, UpMsgIdManuProtStep))
#define UpCmdIdManuSmplChnl  (UpCmdId(UpMsgGrpManu, UpMsgIdManuSmplChnl))
#define UpCmdIdManuSmplNdbd  (UpCmdId(UpMsgGrpManu, UpMsgIdManuSmplNdbd))
#define UpCmdIdManuSmplStack  (UpCmdId(UpMsgGrpManu, UpMsgIdManuSmplStack))
#define UpCmdIdManuSmplTray  (UpCmdId(UpMsgGrpManu, UpMsgIdManuSmplTray))
#define UpCmdIdManuStart  (UpCmdId(UpMsgGrpManu, UpMsgIdManuStart))
#define UpCmdIdManuPause  (UpCmdId(UpMsgGrpManu, UpMsgIdManuPause))
#define UpCmdIdManuStop  (UpCmdId(UpMsgGrpManu, UpMsgIdManuStop))
#define UpCmdIdManuJump  (UpCmdId(UpMsgGrpManu, UpMsgIdManuJump))
#define UpCmdIdManuCtnu  (UpCmdId(UpMsgGrpManu, UpMsgIdManuCtnu))
#define UpCmdIdManuCtnuStack  (UpCmdId(UpMsgGrpManu, UpMsgIdManuCtnuStack))
#define UpCmdIdManuWarnDel  (UpCmdId(UpMsgGrpManu, UpMsgIdManuWarnDel))
#define UpCmdIdUpdCfgRead  (UpCmdId(UpMsgGrpUpdate, UpMsgIdUpdCfgRead))
#define UpCmdIdUpdCfgSet  (UpCmdId(UpMsgGrpUpdate, UpMsgIdUpdCfgSet))
#define UpCmdIdUpdSetup  (UpCmdId(UpMsgGrpUpdate, UpMsgIdUpdSetup))
#define UpCmdIdUpdDld  (UpCmdId(UpMsgGrpUpdate, UpMsgIdUpdDld))
#define UpCmdIdUpdUpld  (UpCmdId(UpMsgGrpUpdate, UpMsgIdUpdUpld))
#define UpCmdIdUpdCnfm  (UpCmdId(UpMsgGrpUpdate, UpMsgIdUpdCnfm))
#define UpCmdIdCaliNtfy  (UpCmdId(UpMsgGrpCali, UpMsgIdCaliNtfy))
#define UpCmdIdCaliStart  (UpCmdId(UpMsgGrpCali, UpMsgIdCaliStart))
#define UpCmdIdCaliSmpl  (UpCmdId(UpMsgGrpCali, UpMsgIdCaliSmpl))
#define UpCmdIdCaliKb  (UpCmdId(UpMsgGrpCali, UpMsgIdCaliKb))
#define UpCmdIdNdbdAct  (UpCmdId(UpMsgGrpNdbd, UpMsgIdNdbdAct))
#define UpCmdIdNdbdNp  (UpCmdId(UpMsgGrpNdbd, UpMsgIdNdbdNp))
#define UpCmdIdFixtPower  (UpCmdId(UpMsgGrpNdbd, UpMsgIdFixtPower))
#define UpCmdIdFixtNtfy  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoPrec, UpMsgIdFixtNtfy)))
#define UpCmdIdFixtPrecChnlSw  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoPrec, UpMsgIdFixtPrecChnlSw)))
#define UpCmdIdFixtPrecSmpl  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoPrec, UpMsgIdFixtPrecSmpl)))
#define UpCmdIdFixtPrecOut  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoPrec, UpMsgIdFixtPrecOut)))
#define UpCmdIdFixtTmprHeat  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoTmpr, UpMsgIdFixtTmprHeat)))
#define UpCmdIdFixtTmprSmpl  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoTmpr, UpMsgIdFixtTmprSmpl)))
#define UpCmdIdFixtCleanAct  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoClean, UpMsgIdFixtCleanAct)))
#define UpCmdIdFixtGasSmpl  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoGas, UpMsgIdFixtGasSmpl)))
#define UpCmdIdFixtFlowSmpl  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoFlow, UpMsgIdFixtFlowSmpl)))
#define UpCmdIdFixtSuctInSmpl  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoSuctIn, UpMsgIdFixtSuctInSmpl)))
#define UpCmdIdFixtSuctOutAct  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoSuctOut, UpMsgIdFixtSuctOutAct)))
#define UpCmdIdFixtSuctOutSmpl  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoSuctOut, UpMsgIdFixtSuctOutSmpl)))
#define UpCmdIdFixtLocatSmpl  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoLocat, UpMsgIdFixtLocatSmpl)))
#define UpCmdIdFixtLocatAct  (UpCmdId(UpMsgGrpFixt, UpMsgIdFixt(FixtProtoLocat, UpMsgIdFixtLocatAct)))

/*---------------整体消息结构，开始-----------------*/
typedef struct
{
    u32 magicTag;
    u16 pldLen;  /*负载长度*/
    u16 msgId;
}UpMsgHead;
typedef struct
{
    u16 msgFlowSeq;
    u16 crcChk;
}UpMsgTail;

#if 0   /*完整消息格式*/
typedef struct upMsgTag
{
    UpMsgHead msgHead;
    u8 msgPld[0];
    UpMsgTail msgTail;
}UpMsg;
#endif
/*---------------整体消息结构，结束-----------------*/

/*---------------联机消息，开始-----------------*/
/*联机命令负载,对应"上中协议"之联机协议部分*/
typedef struct
{
    u16 protocolVer;
    u16 mediumSoftVer;
    u16 smplVer;
    u16 stepVer;
    u16 protectVer;
    u8eConnType connType;
    u8 rsvd;
    u32 maxSmplSeq;   /*最大有效序号*/
    u8 devInfoTlv[0];
}ConnUpCmd;

typedef struct
{
    u16eSubDevType devType;    /*t*/
    u16 devInfoLen;    /*l*/
    u8 devInfo[0];    /*v*/
}ConnDevInfoTlv;

/*子设备单元--托盘*/
typedef struct
{
    u8 trayIdx;
    b8 smplSeqRst;  /*是否复位,复位指示采样序号归零*/
    u16 cellAmt; /*实际电芯数量*/
}ConnTrayInfo;

/*子设备单元--电源箱*/
typedef struct
{
    u8 trayIdx;      /*所属托盘*/
    u8 devIdx;       /*电源箱索引，托盘内全局*/
    u8eBoxType boxType;
    u8 chnAmtConn;            /*最大回路数量，并非电芯数量*/
    u8 chnModuAmt;
    u8 isCombine;       /*跨箱并机标识*/
    u16 boxCellAmt;     /*实际电芯数量*/
    u32 maxVol;
    u32 maxCur;
    u16 softVer;
    u8 online;
    u8 rsvd;
}ConnBoxInfo;

/*子设备单元--电压采样板,针床控制板,温度采样板*/
typedef struct
{
    u8 trayIdx;      /*所属托盘*/
    u8 devIdx;       /*设备索引，托盘内全局*/
    u16 softVer;
    u8 online;
    u8 subType;  /*0--resa,1--dsp*/
    u8 rsvd[2];
}ConnCmmnDevInfo;

/*子设备单元--PLC控制板*/
typedef struct
{
    u8 devIdx;      /*PLC索引*/
    u8 online;
    u16 softVer;
    u32 ipAddr;
    u8 trayIdx;      /*所属托盘*/
    u8 rsvd[3];
}ConnPlcInfo;

/*子设备单元--串联旁路切换板*/
typedef struct
{
    u8 trayIdx;      /*所属托盘*/
    u8 online;
    u16 devIdx;      /*旁路板索引，托盘内全局*/
    u16 chnIdx;      /*旁路板所属通道*/
    u16 softVer;
}ConnSerielSwInfo;

/*联机命令的响应负载*/
typedef struct
{
    u16 rspCode;
    u16 rsvd;
    u32 absSec;     /*绝对时间秒数 */
}ConnUpAck;
/*---------------联机消息，结束-----------------*/

/*---------------采样消息，开始-----------------*/
/*采样命令*/
/*通道模式下通道,针床,运行栈统一采样命令*/
typedef struct
{
    u8 trayIdx;
    u8 smplAmt;
    u16 rsvd;
    u32 smplSeq;
}UpSmplCmd;

/*单通道数据*/
typedef struct
{
    u32 timeStampSec;
    u16 timeStampMs;
    u16 chnIdx;
    u8eChnType chnType;
    u8eUpChnState chnUpState;
    u8 stepId; /*0-254合法,255无效*/
    u8eStepType stepType;
    u8eStepSubType stepSubType;
    u8 inLoop;   /*串联时，电芯是否在回路中*/
    u16 stepSmplSeq;   /*工步内采样序号，工步切换时清零*/
    s32 volCell;    /*电池电压，极柱电压，电压采样线电压*/
    s32 volCur;    /*电流探针电压*/
    s32 volPort;    /*设备端口电压*/
    s32 volInner;    /*内部电压*/
    s32 current;
    u32 capacity;
    u16 cellTmpr[2];
    u16eCauseCode causeCode;
    s16 npVal;
    u8 smokePres;
    u8 slotTmprAmt;
    u8 rsvd[2];
    u16 slotTmpr[0];
}UpChnlSmpl;

typedef struct
{
    u32 timeStampSec;
    u16 timeStampMs;
    u8eTrayEnterState enterState;    /*入料状态*/
    u8eTrayTouchState touchState;    /*压合状态*/
    s16 npVal;
    u16 ratioAnalog;
    u32 bitmapState;
    u32 bitmapAlarm;
    u8 cylinderWarn; /*0无1压合2分离3感应器*/
    u8 smokeWarn;  /*0无1前2后3都*/
    u8 slotTmprAmt;
    u8 rsvd;
    u16 slotTmpr[0];
}UpNdbdSmpl;

/*通道运行数据*/
typedef struct
{
    u8 stepId;
    u8 rsvd;
    u16 leftAmt;   /*循环剩余次数*/
}LoopDscr;
typedef struct
{
    u32 timeStampSec;
    u16 timeStampMs;
    u8eChnType chnType;
    u8 flowLoopAmt;
    u16 chnIdx;
    u16 rsvd;
    u32 capacity;   /*累积容量，不含当前工步容量*/
    LoopDscr loopDscr[0];   /*loop describe*/
}RunStack;

/*采样响应*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 smplAmt;
    u32 firstSeq;   /*本消息内第一条采样的序号*/
    u32 nextWriteSeq;    /*中位机下次生成采样的序号*/
    u8 smpl[0];
    UpChnlSmpl chnlSmpl[0];
    RunStack runStack[0];
    UpNdbdSmpl ndbdSmpl[0];
}UpSmplAck;

typedef struct
{
    u8eChnType chnType;
    u8eUpChnState chnUpState;
    u8 stepId; /*0-254合法,255无效*/
    u8eStepType stepType;
    u8eStepSubType stepSubType;
    u8 inLoop;   /*串联时，电芯是否在回路中*/
    u16eCauseCode causeCode;   /**/
    u32 stepRunTime;  /*毫秒*/
    s32 volCell;    /*电池电压，电压采样线电压*/
    s32 volCur;    /*电流探针电压*/
    s32 volPort;    /*设备端口电压*/
    s32 volInner;    /*内部电容电压*/
    s32 current;
    u32 capacity;  /*工步容量,含续接*/
}TrayChnSmpl;

typedef struct
{
    u8eTrayEnterState enterState;    /*入料状态*/
    u8eTrayTouchState touchState;    /*压合状态*/
    s16 npVal;
    u16 ratioAnalog;
    u16 cellTmprAmt;  /*托盘的电芯温度总数*/
    u32 bitmapState;
    u32 bitmapAlarm;
    u8 cylinderWarn; /*0无1压合2分离3感应器*/
    u8 smokeWarn;  /*0无1前2后3都*/
    u8 slotTmprAmt;
    u8 tmprAmtPerCell;
    u16 slotTmpr[0];
    u16 cellTmpr[0];
}TrayNdbdSmpl;

typedef u8 u8eTraySmplType;
#define TraySmplChn 0x00
#define TraySmplNdbd 0x01
#define TraySmplCri 0x02

typedef struct
{
    u8eTraySmplType smplType;
    u8 smplSize;  /*单个通道采样长度*/
    u16 smplAmt;  /*托盘采样中的通道采样条数*/
    TrayChnSmpl chnSmpl[0];
    TrayNdbdSmpl ndbdSmpl[0];
}TraySmpl;

/*存储的采样*/
typedef struct
{
    u32 timeStampSec;  /*时间戳,秒数*/
    TraySmpl traySmpl[0];
}TraySmplRcd;

typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 smplAmt;
    u32 rspSmplSeq;  /*首条采样序号*/
    u32 nextGenSeq;    /*中位机下次生成采样的序号*/
    TraySmplRcd traySmplRcd[0];
}TraySmplAck;

/*---------------采样消息，结束-----------------*/

/*---------------流程消息，开始-----------------*/

/*单工步数据*/
typedef struct upStepInfoTag
{
    u8 stepId;
    u8eStepType stepType;
    u8 paramEnable;  /*bitmap,参数使能指示*/
    u8 rsvd;
    s32 stepParam[ParamAmt];
}UpStepInfo;


/*命令*/
typedef struct
{
    u8 trayIdx;
    u8 stepAmtTtl;  /*流程中的工步总数*/
    u8 stepSeq;  /*本次传输的起始工步号*/
    u8 stepAmt;   /*本次传输的工步数量*/
    u16 chnAmt;   /*0--整盘，否则表示通道电芯/数量*/
    u16 rsvd;
    u16 chnId[0];
    UpStepInfo upStep[0];
}UpFlowCmd;


/*响应,多数上位机命令的通用响应消息*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 rsvd;
}UpCmmnAck;
/*---------------流程消息，结束-----------------*/

/*---------------保护消息，开始-----------------*/

/*保护单元*/
typedef struct upProtUnitTag
{
    u16 protId;
    u8 stepId;   /*适用工步号，全局保护时保留*/
    u8 paramEnable;  /*bitmap,参数使能指示*/
    s32 protParam[ParamAmt];
}UpProtUnit;

/*组合保护*/
typedef struct upFixProtTag
{
    u16 fixProtlen;
    u16 rsvd;
    u8 fixProtAby[0];
}UpFixProt;

/*全程保护命令*/
typedef struct upProtGenCmdTag
{
    u8 trayIdx;
    u8 fixProtAmt;
    u16 protAmtTtl;
    u16 protSeq;
    u16 protAmt;
    UpFixProt fixProt[0];
    UpProtUnit protUnit[0];
}UpProtGenCmd;

/*工步保护命令*/
typedef struct upProtStepCmdTag
{
    u8 trayIdx;
    u8 rsvd0;
    u16 protAmtTtl;
    u16 protSeq;
    u16 protAmt;
    u16 chnAmt;   /*0--整盘，否则表示通道电芯/数量*/
    u16 rsvd1;
    u16 chnId[0];
    UpProtUnit protUnit[0];
}UpProtStepCmd;

/*保护消息的响应适用通用上位机响应*/
/*---------------保护消息，结束-----------------*/

/*---------------启停控制消息，开始-----------------*/
/*启动电芯指示*/
typedef struct upStartChnlIndTag
{
    u16 chnId;
    u8 stepId;
    u8 rsvd;
}UpStartChnInd;

/*续接指示*/
typedef struct
{
    u16 chnId;
    u8 ctnuStepId;
    u8 loopAmt;
    u32 stepRestTime;   /*当前工步剩余时间*/
    u32 capStep;   /*当前工步已跑容量*/
    u32 capFlowCrnt;   /*累积容量，不含当前工步*/
    LoopDscr loopDscr[0];
}UpCtnuChnlInd;

/*停止，暂停，跳转*/
typedef struct upFlowCtrlCmdTag
{
    u8 trayIdx;
    u8 jumpStepId;    /*跳转命令有效，其余保留*/
    u16 chnAmt;   /*0--整盘，否则表示通道电芯/数量*/
    u16 chnId[0];   /*暂停，停止，跳转*/
    UpStartChnInd startInd[0];
    UpCtnuChnlInd ctnuInd[0];
}UpFlowCtrlCmd;

/*启停消息的响应适用通用上位机响应*/
/*---------------启停控制消息，结束-----------------*/

/*---------------配置升级消息，开始-----------------*/
/*配置读取*/

/*配置下发设置*/

/*启动传输*/
typedef struct
{
    u8eUpUpdDevType devType;
    u8eUpdFileType fileType;
    u8 isUpload;
    u8 rsvd1;
    u32 fileSize;  /*下载有效*/
    u8 trayIdx;   /*非中位机有效*/
    u8 rsvd2;
    u16 devId;   /*非中位机有效*/
}UpUpdSetupCmd;

typedef struct
{
    u16 rspCode;
    u16 pageSize;  /*传输最大分片*/
    u32 fileSize;  /*上传文件时有效*/
    u16 familyVer;  /*升级app时有效,boot版本号*/
    u16 rsvd;
}UpUpdSetupAck;

/*下载命令，响应适用通用*/
typedef struct
{
    u8eUpUpdDevType devType;
    u8eUpdFileType fileType;
    u16 pldSize;
    u8 trayIdx;   /*非中位机有效*/
    u8 rsvd;   /*非中位机有效*/
    u16 devId;   /*非中位机有效*/
    u32 fileSize;
    u32 offset;   /*本次传输偏移*/
    u8 payload[0];
}UpUpdDldCmd;

/*上传命令*/
typedef struct
{
    u8eUpUpdDevType devType;
    u8eUpdFileType fileType;
    u8 upStopInd;   /*终止指示*/
    u8 trayIdx;   /*非中位机有效*/
    u16 devId;   /*非中位机有效*/
    u16 rsvd;
    u32 offset;   /*本次传输偏移*/
}UpUpdUpldCmd;

/*上传命令的响应*/
typedef struct
{
    u16 rspCode;
    u16 pldSize;
    u32 fileSize;
    u32 offset;   /*本次传输偏移*/
    u8 payload[0];
}UpUpdUpldAck;

/*升级确认命令,读取升级后版本号*/
typedef struct
{
    u8eUpUpdDevType devType;
    u8 trayIdx;   /*非中位机有效*/
    u16 devId;   /*非中位机有效*/
    u8eUpdFileType fileType;
    u8 rsvd[3];
}UpUpdCnfmCmd;

/*升级确认命令响应,返回升级后版本号*/
typedef struct
{
    u16 rspCode;
    u16 updateVer;
}UpUpdCnfmAck;

/*---------------配置升级消息，结束-----------------*/

/*---------------电源柜修调消息，开始-----------------*/
/*修调指示:进入或离开修调*/
typedef struct
{
    u8 trayIdx;
    b8 caliEnable; /*0--离开修调，1--进入修调*/
    u8 boxIdx;
    u8 rsvd;
}UpCaliNtfyCmd;

typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 boxIdx;
}UpCaliNtfyAck;

/*修调启动指示，适用通用应答*/
typedef struct
{
    u8 trayIdx;
    u8 moduIdx;  /*模块号*/
    u16 chnIdx; /*托盘内索引*/
    u8eCaliMode caliMode;
    u8eCaliType caliType;
    b8 isStart;  /*真起假停,假时trayIdx和chnIdx有效，其余保留*/
    u8 rsvd;
    u32 caliKbPoint;
}UpCaliStartCmd;

typedef struct
{
    u32 caliKbPoint;
    s32 valAdK;
    s32 valAdB;
    s32 valDaK;
    s32 valDaB;
}CaliKb;

/*修调参数下发，适用通用应答*/
typedef struct
{
    u8 trayIdx;
    u8 moduIdx;
    u16 chnIdx; /*托盘内索引*/
    u8eCaliType caliType;
    u8eVolType volType;  /*caliType为电压时有效*/
    u8 caliKbAmt;
    u8 rsvd;
    CaliKb caliKb[0];
}UpCaliKbCmd;

/*修调采样命令*/
typedef struct
{
    u8 trayIdx;
    u8 moduIdx;
    u16 chnIdx; /*托盘内索引*/
    b8 smplOnlyMainChn; /*串联且通道为主通道时有效*/
    u8 rsvd[3];
}UpCaliSmplCmd;

/*上行方向电流电压属性五件套装*/
typedef struct
{
    s32 current;
    s32 volCell;    /*电池电压，极柱电压，电压采样线电压*/
    s32 volCur;    /*电流探针电压*/
    s32 volPort;    /*设备端口电压*/
    s32 volInner;    /*内部电压*/
}UpldCurVolSet;

/*修调采样应答*/
typedef struct
{
    u16 rspCode;
    u16 chnIdx; /*托盘内索引*/
    u8 trayIdx;
    u8 moduIdx;
    b8 smplOnlyMainChn; /*串联且通道为主通道时有效*/
    u8 smplAmt;
    UpldCurVolSet curVolSet[0];
}UpCaliSmplAck;

/*---------------电源柜修调消息，结束-----------------*/

/*---------------工装消息，开始-----------------*/

/*工装统一使能命令*/
typedef struct
{
    u8 trayIdx;
    u8eUpFixtDevType fixtType;
    b8 enable;
    u8 actAddr;  /*工装实际拨码*/
}UpFixtNtfyCmd;

/*工装统一使能响应*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 fixtVer;
}UpFixtNtfyAck;

/*精度线序工装通道切换命令,响应适用通用响应*/
typedef struct
{
    u8 trayIdx;
    u8eCaliType caliType;
    u8eFixtChnSwType chnSwType;
    u8 rsvd;
    u16 firstChn;
    u16 lastChn;
}UpFixtPrecSwCmd;

/*精度线序工装采样命令*/
typedef struct
{
    u8 trayIdx;
    u8eFixtPrecSmplType smplType;
    u16 rsvd;
}UpFixtPrecSmplCmd;

/*精度线序工装采样应答*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 rsvd;
    s32 voltage;
    s32 current;
}UpFixtPrecSmplAck;

/*精度线序工装输出命令,应答适用通用应答*/
typedef struct
{
    u8 trayIdx;
    u8 rsvd[3];
    s32 voltage;
    s32 current;
}UpFixtPrecOutCmd;

/*温度线序工装启停加热命令,应答适用通用应答*/
typedef struct
{
    u8 trayIdx;
    b8 heatStop;
    b8 isSingleChn;  /*0--全盘,1--单通道*/
    u8 rsvd;
    u16 tmprVal;
    u16 chnIdx;  /*全盘时保留*/
}UpFixtTmprHeatCmd;

/*温度线序工装采样命令*/
typedef struct
{
    u8 trayIdx;
    u8 chnAmt;   /*采样通道数量,最大64个*/
    u16 firstChnIdx;
}UpFixtTmprSmplCmd;

/*温度线序工装采样应答*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 chnAmt;
    u16 tmprVal[0];
}UpFixtTmprSmplAck;

/*负压清洗工装动作命令,响应适用通用*/
typedef struct
{
    u8 trayIdx;
    u8eFixtCleanActType actType;   /**/
    u16 rsvd;
}UpFixtCleanActCmd;

/*负压气密性工装采样命令*/
typedef struct
{
    u8 trayIdx;
    u8 chnAmt;   /**/
    u16 chnIdx;
}UpFixtGasSmplCmd;

/*负压气密性工装采样应答*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 chnAmt;
    u16 npVal[0];
}UpFixtGasSmplAck;

/*负压流量工装采样命令*/
typedef struct
{
    u8 trayIdx;
    u8 chnAmt;   /**/
    u16 chnIdx;
}UpFixtFlowSmplCmd;

/*负压流量工装采样应答*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 chnAmt;
    u16 flowVal[0];
}UpFixtFlowSmplAck;

/*插吸嘴工装采样命令*/
typedef struct
{
    u8 trayIdx;
    u8 rsvd;   /**/
    u16 delayTime;
}UpFixtSuckInSmplCmd;

/*插吸嘴工装采样应答*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    b8 hasRemain;
}UpFixtSuckInSmplAck;

/*拔吸嘴工装采样命令*/
typedef struct
{
    u8 trayIdx;
    u8 rsvd[3];   /**/
}UpFixtSuckOutSmplCmd;

/*拔吸嘴工装采样应答*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 rsvd1;
    u8eFixtSuckOutStaType state;
    b8 hasRemain;
    u8 rsvd[2];
}UpFixtSuckOutSmplAck;

/*拔吸嘴工装动作命令,应答适用通用*/
typedef struct
{
    u8 trayIdx;
    u8eFixtSuckOutStaType action;   /**/
    u16 delayTime;
}UpFixtSuckOutActCmd;

/*定位工装采样命令*/
typedef struct
{
    u8 trayIdx;
    u8 rsvd[3];   /**/
}UpFixtLocatSmplCmd;

/*定位工装采样应答*/
typedef struct
{
    u16eRspCode rspCode;
    u8 trayIdx;
    u8 rsvd1;
    b8 unlockSucc;  /*解锁成功*/
    u8eFixtLocatPos locatPos;
    u8 rsvd[2];
}UpFixtLocatSmplAck;

/*定位工装动作命令,应答适用通用*/
typedef struct
{
    u8 trayIdx;
    u8 cellType;
    u8eFixtLocatTurnType anodeTurn;  /*正极电机*/
    u8eFixtLocatTurnType npTurn;  /*负压电机*/
    u8eFixtLocatTurnType cathodeTurn;  /*负极电机*/
    u8 rsvd;
    u16 delayTime;
}UpFixtLocatActCmd;

/*---------------工装消息，结束-----------------*/

/*---------------针床操作消息，结束-----------------*/

/*负压操作命令,响应适用通用*/
typedef struct
{
    u8 trayIdx;
    u8eNdbdActType oprType;
    u8 oprPldLen;
    u8 rsvd;
    u8 oprPld[0];
}UpNdbdActOprCmd;

/*负压操作命令,响应适用通用*/
typedef struct
{
    u8 trayIdx;
    u8eNpOprCode npOpr;
    u8 oprPldLen;
    u8eNpDevType npDevType;
    u8 oprPld[0];
}UpNdbdNpOprCmd;

/*工装供电命令,响应适用通用*/
typedef struct
{
    u8 trayIdx;
    b8 powerEnable;
    u16 rsvd;
}UpNdbdFixtPowerCmd;
/*---------------针床操作消息，结束-----------------*/

/*接收缓存尺寸,也是最大消息长度*/
#define UpMsgMagic  (0x6e6e7968)    /*"hynn"的小字节序*/
#define UpMsgBufSize 2048
#define UpMsgMandLen (sizeof(UpMsgHead)+sizeof(UpMsgTail)) /*定长(最小)长度*/
#define UpMsgPldLenMax (UpMsgBufSize-UpMsgMandLen)
#define UpMsgMandCrcLen (sizeof(u16)+sizeof(u16)+sizeof(u16)) /*定长部分参与校验的长度*/
#define MaxStepTrans 45
#define MaxProtTrans 45

typedef void (*UpMsgDisp)(u8, u8 *, u16);
typedef void (*UpMsgProc)(u8 *, u16);
typedef Ret (*UpStepParamChk)(UpFlowCmd *, UpStepInfo *);

typedef struct
{
    UpMsgDisp upMsgDisp[UpMsgGrpCri];
    UpMsgDisp upMsgFixtDisp[FixtProtoCri];
    UpMsgProc upMsgProcManu[UpMsgIdManuCri];
    UpMsgProc upMsgProcUpdate[UpMsgIdUpdCri];
    UpMsgProc upMsgProcCali[UpMsgIdCaliCri];
    UpMsgProc upMsgProcNdbd[UpMsgIdNdbdCri];

    /*工装命令字组成:grp+type+id,需要二次分发*/
    UpMsgProc upMsgFixtPrecDisp[UpMsgIdFixtPrecCri];
    UpMsgProc upMsgFixtTmprDisp[UpMsgIdFixtTmprCri];
    UpMsgProc upMsgFixtCleanDisp[UpMsgIdFixtCleanCri];
    UpMsgProc upMsgFixtGasDisp[UpMsgIdFixtGasCri];
    UpMsgProc upMsgFixtFlowDisp[UpMsgIdFixtFlowCri];
    UpMsgProc upMsgFixtSuctInDisp[UpMsgIdFixtSuctInCri];
    UpMsgProc upMsgFixtSuctOutDisp[UpMsgIdFixtSuctOutCri];
    UpMsgProc upMsgFixtLocatDisp[UpMsgIdFixtLocatCri];

    UpStepParamChk stepParamChk[StepTypeCri];

    u16 upMsgFlowSeq;  /*消息流水号*/
    u16 rxUpMsgCnt;
    b8 upOnline;  /*发送联机成功表示上位机在线*/
    u8eConnType connType;

    Timer keepAliveTmr;
    Timer connUpTmr;
    Timer bootTmr;  /*boot内使用*/
    Timer upDiscExprTmr;
    UpdMgr updMgr;
}UpItfCb;

extern u8 *sendUpBuf;
extern u8 *recvUpBuf;
extern UpItfCb *gUpItfCb;
extern s32 gAbsTimeSec;

#ifdef __cplusplus
extern "C"  {
#endif

extern void sendUpConnCmd(Timer *);
extern void sendUpConnNtfy(void);
extern void keepAliveChk(Timer *timer);
extern void upInitApp(void);
extern void upInitBoot(void);
extern u8 sendUpMsg(u8 *msg, u16 pldLen, u16 msgId);
extern void rspUpCmmn(u16 msgId, u8 trayIdx, u16 rspCode);
extern void rspUpCaliNtfyAck(u8 trayIdx, u8 boxIdx, u16eRspCode rspCode);
extern void trayUpDiscExpr(Timer *timer);

#ifdef __cplusplus
}
#endif

#endif
#endif
