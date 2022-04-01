
#ifndef _UART_H_
#define _UART_H_
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"
#include "entry.h"

#define UartFuncCode 0x42  /*66*/

/*---------------串口设备消息类型，开始-----------------*/
/*串口设备通用消息类型*/
typedef u8 u8eUartMsgId;

#define UartMsgConn  0x00
#define UartMsgCfgRead  0x01
#define UartMsgCfgSet  0x02
#define UartMsgUpdSetup  0x03
#define UartMsgUpdDld  0x04
#define UartMsgUpdUpld  0x05
#define UartMsgUpdCnfm  0x06
#define UartMsgSmpl  0x07
#define UartMsgCmmnCri 0x08

#define UartMsgIdCmmn(msgId) (msgId)
#define UartMsgIdSpecBase 0x30
#define UartMsgIdSpec(msgId) (UartMsgIdSpecBase + (msgId))

/*精度线序工装特有消息类型,并联串联共用*/
#define UartMsgFixtPrecChnSw 0x00  /*工装通道切换*/
#define UartMsgFixtPrecSmpl 0x01  /*工装采样*/
#define UartMsgFixtPrecOut 0x02  /*工装输出*/
#define UartMsgFixtPrecCri 0x03

/*温度线序工装特有消息类型*/
#define UartMsgFixtTmprHeat 0x00  /**/
#define UartMsgFixtTmprSmpl 0x01  /**/
#define UartMsgFixtTmprCri 0x02

/*负压清洗工装特有消息类型*/
#define UartMsgFixtCleanAct 0x00  /**/
#define UartMsgFixtCleanCri 0x01

/*负压气密性工装特有消息类型*/
#define UartMsgFixtGasSmpl 0x00  /**/
#define UartMsgFixtGasCri 0x01

/*负压流量工装特有消息类型*/
#define UartMsgFixtFlowSmpl 0x00  /**/
#define UartMsgFixtFlowCri 0x01

/*插吸嘴工装特有消息类型*/
#define UartMsgFixtSuckInSmpl 0x00  /**/
#define UartMsgFixtSuckInCri 0x01

/*拔吸嘴工装特有消息类型*/
#define UartMsgFixtSuckOutSmpl 0x00  /**/
#define UartMsgFixtSuckOutAct 0x01  /**/
#define UartMsgFixtSuckOutCri 0x02

/*定位工装特有消息类型*/
#define UartMsgFixtLocatSmpl 0x00  /**/
#define UartMsgFixtLocatAct 0x01  /**/
#define UartMsgFixtLocatCri 0x02
/*---------------串口设备消息类型，结束-----------------*/

/*所有串口设备共用消息头*/
typedef struct
{
    u8 devAddr;
    u8 funcCode;  /*UartFuncCode*/
    u8 pldLen;  /*负载长度,不含消息头*/
    u8 flowSeq;
    u8 payload[0];
}UartMsgHead;

/*消息尾*/
typedef struct
{
    u16 crcChk;
}UartMsgTail;

/*通用命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u16 rsvd;
}UartCmmnCmd;

/*通用响应*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 rsvd;
}UartCmmnAck;

/*通用联机响应*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 devVer;
}UartCmmnConnAck;

/*---------------温度盒消息，开始-----------------*/
/*温度盒联机命令适用通用命令*/
/*温度盒联机响应*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 devVer;
    u8 chnAmt;
    b8 isInApp;
    u8 rsvd[2];
}TmprSmplConnAck;

#define TmprSmplChnAmtMax 64 /*单次采样最大通道数*/

/*温度盒采样命令*/
typedef struct
{
    u16 tmprSmplChnIdx;
    u8 tmprSmplChnAmt;
    u8 rsvd;
}TmprSmplInd;
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 tmprSmplGrpAmt;
    u8 rsvd;
    TmprSmplInd tmprSmplInd[0];
}TmprSmplSmplCmd;

/*温度盒采样响应*/
typedef struct
{
    u8 tmprSmplChnAmt;
    u8 rsvd;
    u16 tmprSmplChnIdx;
    u16 tmprVal[0];
}TmprSmplData;
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 tmprSmplGrpAmt;
    TmprSmplData tmprSmplData[0];
}TmprSmplSmplAck;

/*---------------温度盒消息，开始-----------------*/

/*---------------精度线序工装消息，开始-----------------*/
/*联机命令响应适用通用命令和通用联机响应*/

/*精度线序通道切换命令,响应适用通用响应*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8eCaliType caliType;
    u8eFixtChnSwType chnSwType;
    u16 firstChn;
    u16 lastChn;
}UartFixtPrecSwCmd;

/*精度线序工装采样命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8eFixtPrecSmplType smplType;
    u8 rsvd;
}UartFixtPrecSmplCmd;

/*精度线序工装采样应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 rsvd;
    s32 voltage;
    s32 current;
}UartFixtPrecSmplAck;

/*精度线序工装输出命令,应答适用通用应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u16 rsvd;
    s32 voltage;
    s32 current;
}UartFixtPrecOutCmd;

/*---------------精度线序工装消息，结束-----------------*/

/*---------------温度线序工装消息，开始-----------------*/
/*温度线序工装启停加热命令,应答适用通用应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    b8 isSingleChn;   /*是否操作单通道,false--全盘*/
    b8 heatStop;
    u16 chnIdx;  /*从零计数,全盘时保留*/
    u16 tmprVal;
}UartFixtTmprHeatCmd;

/*温度线序工装采样命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u16 firstChnIdx;
    u8 chnAmt;
    u8 rsvd[3];
}UartFixtTmprSmplCmd;

/*温度线序工装采样应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 chnAmt;
    u16 firstChnIdx;
    u16 tmprVal[1];
}UartFixtTmprSmplAck;
/*---------------温度线序工装消息，结束-----------------*/

/*---------------负压清洗工装消息，开始-----------------*/
/*负压清洗工装动作命令,应答适用通用*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8eFixtCleanActType actType;
    u8 rsvd;
}UartFixtCleanActCmd;

/*---------------负压清洗工装消息，结束-----------------*/

/*---------------负压气密性工装消息，开始-----------------*/
/*负压气密性工装采样命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u16 firstChnIdx;
    u8 chnAmt;
    u8 rsvd[3];
}UartFixtGasSmplCmd;

/*负压气密性工装采样应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 chnAmt;
    u16 firstChnIdx;
    u16 npVal[1];
}UartFixtGasSmplAck;

/*---------------负压气密性工装消息，结束-----------------*/

/*---------------负压流量工装消息，开始-----------------*/
/*负压流量工装采样命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u16 firstChnIdx;
    u8 chnAmt;
    u8 rsvd[3];
}UartFixtFlowSmplCmd;

/*负压流量工装采样应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 chnAmt;
    u16 firstChnIdx;
    u16 flowVal[1];
}UartFixtFlowSmplAck;

/*---------------负压流量工装消息，结束-----------------*/

/*---------------插吸嘴工装消息，开始-----------------*/
/*插吸嘴工装采样命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u16 delayTime; /*0--采存储,1--延时后采样保存*/
}UartFixtSuckInSmplCmd;

/*插吸嘴工装采样应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    b8 hasRemain;
}UartFixtSuckInSmplAck;

/*---------------插吸嘴工装消息，结束-----------------*/

/*---------------拔吸嘴工装消息，开始-----------------*/
/*拔吸嘴工装采样命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u16 rsvd;
}UartFixtSuckOutSmplCmd;

/*拔吸嘴工装采样应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8eFixtSuckOutStaType state;
    b8 hasRemain;
    u8 rsvd[3];
}UartFixtSuckOutSmplAck;

/*拔吸嘴工装动作命令,响应适用通用*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8eFixtSuckOutStaType action;
    u8 rsvd;
    u32 delayTime;
}UartFixtSuckOutActCmd;
/*---------------拔吸嘴工装消息，结束-----------------*/

/*---------------定位工装消息，开始-----------------*/
/*定位工装采样命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u16 rsvd;
}UartFixtLocatSmplCmd;

/*定位工装采样应答*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    b8 unlockSucc;
    u8eFixtLocatPos locatPos;
    u8 rsvd[3];
}UartFixtLocatSmplAck;

/*定位工装动作命令,响应适用通用*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 cellType;
    u8eFixtLocatTurnType anodeTurn;  /*正极电机*/
    u8eFixtLocatTurnType npTurn;  /*负压电机*/
    u8eFixtLocatTurnType cathodeTurn;  /*负极电机*/
    u16 delayTime;
}UartFixtLocatActCmd;
/*---------------定位工装消息，结束-----------------*/

/*---------------配置升级消息，开始-----------------*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    b8 isUpload;
    u8eUpdFileType fileType;
    u32 fileSize;
}UartUpdSetupCmd;

typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 pageSize;  /*最大传输分片*/
    u32 fileSize;  /*升级在boot内时为boot版本,否则上传有效*/
}UartUpdSetupAck;

/*下载文件命令,响应适用通用响应*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8eUpdFileType fileType;
    u8 pldSize;
    u32 fileSize;
    u32 offset;
    u8 payload[0];
}UartUpdDldCmd;

/*上传文件命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8eUpdFileType fileType;
    b8 upStopInd;
    u32 offset;
}UartUpdUpldCmd;

/*上传文件响应*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 rsvd0;
    u32 fileSize;
    u32 offset;
    u8 pldSize;
    u8 rsvd1[3];
    u8 payload[0];
}UartUpdUpldAck;

/*升级确认命令*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8eUpdFileType fileType;
    u8 rsvd;
}UartUpdCnfmCmd;

/*升级确认响应*/
typedef struct
{
    u8eUartDevType devType;
    u8eUartMsgId msgId;
    u8 rspCode;
    u8 cnfmVer;
}UartUpdCnfmAck;

/*---------------配置升级消息，结束-----------------*/

typedef void (*UartMsgProc)(Uart *, u8, u8 *, u16);
typedef void (*UartSmplTx)(Uart *);

typedef struct
{
    ListD blockBufList;
    u8 uartAddrBase[UartDevTypeCri];
    u8 uartDynAddr2TrayIdx[UartDevTypeCri-UartFixtTmpr][FixtAddrCri]; /*todo,不用就删掉*/
    u8eUartDevType upFixt2UartType[UpFixtTypeCri];
    UartSmplTx uartSmplTx[UartNeedSmplCri];
    UartMsgProc uartCmmnMsgProc[UartMsgCmmnCri];
    UartMsgProc uartSpecMsgDisp[UartDevTypeCri];
    UartMsgProc uartFixtPrecMsgProc[UartMsgFixtPrecCri];
    UartMsgProc uartFixtTmprMsgProc[UartMsgFixtTmprCri];
    UartMsgProc uartFixtCleanMsgProc[UartMsgFixtCleanCri];
    UartMsgProc uartFixtGasMsgProc[UartMsgFixtGasCri];
    UartMsgProc uartFixtFlowMsgProc[UartMsgFixtFlowCri];
    UartMsgProc uartFixtSuckInMsgProc[UartMsgFixtSuckInCri];
    UartMsgProc uartFixtSuckOutMsgProc[UartMsgFixtSuckOutCri];
    UartMsgProc uartFixtLocatMsgProc[UartMsgFixtLocatCri];
}UartMgr;

extern UartMgr *gUartMgr;

#ifdef __cplusplus
extern "C"  {
#endif

extern void uartInit();
extern void uartExprBlkAck(Timer *timer);
extern void uartExprCtrlAck(Timer *timer);
extern void uartExprSmplAck(Timer *timer);
extern void uartTransTxTry(u8 uartIdx, void *buf);
extern void *allocUartBlockBuf();
extern void uartSmplBegin(Timer *timer);

#ifdef __cplusplus
}
#endif


#endif
