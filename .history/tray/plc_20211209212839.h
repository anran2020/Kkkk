

#ifndef _PLC_H_
#define _PLC_H_
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
#include "timer.h"

typedef u8 u8ePlcPortoType;
#define PlcPortoModbusTcp 0x00
#define PlcPortoCri 0x01

#define PlcObjNameLenMax 15
typedef struct
{
    b8 enable;
    u8 amount;
    u16 plcAddr;
    s8 name[AlignStr(PlcObjNameLenMax)];
}PlcDataObj;

#define PlcBufAmt  64
typedef struct
{
    ChainD chain;
    u8 trayIdxInPlc;  /*plc-tray-idx*/
    u8eNdbdCtrlType regType;
    u16 regVal;
}PlcCmdBlkBuf;

#define PlcTrayAmtMax 8
typedef struct
{
    u32 ip;
    u8 plc2TrayIdx[PlcTrayAmtMax]; /*dev-tray-idx*/
    u16 trayAddrBase[PlcTrayAmtMax];
    u16 port;
    u16 transId;
    u16 smplRegIdx;  /*相对地址*/
    u8 smplRegAmt;
    u8 smplTrayIdx;  /*plc-tray-idx,当前采样*/
    u8 trayAmt;
    u8 plcIdx;
    b8 online;
    u8 version;
    u8 delaySmpCnt; /*延时采样,采样超时,因为不冲突,共用计数*/
    Timer plcSmplLoopTmr;
    Timer plcWaitAckTmr;
    PlcCmdBlkBuf *plcBlkBufWaitAck;
    ListD plcBlkListBusy;  /*PlcCmdBlkBuf,等待发送的控制消息*/
    ListD plcBlkListIdle;  /**/
}Plc;

typedef u8 u8eModbusTcpFuncCode;
#define ModbusTcpFcRdCoilSta  0x01  /*bit,*/
#define ModbusTcpFcRdInputSta  0x02  /*bit,*/
#define ModbusTcpFcRdHoldReg  0x03  /*word,*/
#define ModbusTcpFcRdInputReg  0x04  /*word,*/
#define ModbusTcpFcWtCoilSingle  0x05  /*bit,*/
#define ModbusTcpFcWtRegSingle  0x06  /*word,*/
#define ModbusTcpFcWtCoilMulti  0x0f  /*bit,*/
#define ModbusTcpFcWtRegMulti  0x10  /*word,*/

/*modbusTcp--涉及多字节的均为网络序*/
typedef struct
{
    u8 byteAmt;
    u8 regVal[0]; /*按字操作*/
}ModbusTcpVal;

#define ModbusTcpProtoId 0x00
#define WordToNet(a) (((u16)(a)>>8) + ((u16)(a)<<8))

/*公共头部分*/
/*要求网络序*/
typedef struct
{
    u16 transId; /*从机原样返回*/
    u16 protoId;  /*modbusTcp--0*/
    u16 length;  /*之后的长度,不含length自身*/
    u8 payload[0];
}ModbusTcpHead;
typedef struct
{
    u8 unitId;  /*大约可以对应于modbus的addr,从机原样返回*/
    u8eModbusTcpFuncCode funcCode;
    u16 regAddr;
    u16 oprVal; /*写单个时为reg值,读和写多个时为reg数量*/
    ModbusTcpVal val[0]; /*写多个的命令,有它*/
}ModbusTcpCmd;
typedef struct
{
    u8 unitId;  /*大约可以对应于modbus的addr,从机原样返回*/
    u8eModbusTcpFuncCode funcCode;
    u16 regAddr;
    u16 oprVal; /*写单个时为reg值,写多个时为reg数量*/
}ModbusTcpWtAck;
typedef struct
{
    u8 unitId;  /*大约可以对应于modbus的addr,从机原样返回*/
    u8eModbusTcpFuncCode funcCode;
    u8 byteAmt; /*错误时为错误码*/
    u8 val[0]; /*按字操作*/
}ModbusTcpRdAck;

typedef struct
{
    u8 plcAmt;
    u8ePlcPortoType plcProtoType;
    b8 addrIsWord;
    u8 regReadAmtMax;  /*单次读取最大数量*/
    u16 regReadMin;  /*最小有效地址*/
    u16 regReadMaxCri;  /*最大临界非法地址*/
    b8 plcInfoHasChg;
    PlcDataObj plcSta[NdbdSenCri];
    PlcDataObj plcWarn[NdbdWarnCri];
    PlcDataObj plcCtrl[NdbdSetTypeCri];
    Plc *plc;
}PlcMgr;

extern PlcMgr *gPlcMgr;


#ifdef __cplusplus
extern "C"  {
#endif

extern void plcInit();
extern void plcSmplBegin(Timer *timer);
extern void plcExprRxSmplAck(Timer *timer);
extern void plcExprRxCtrlAck(Timer *timer);
extern void plcRegWriteTry(u8 trayIdx, u8eNdbdCtrlType type, s16 val);
extern b8 plcBeOnline(u8 plcIdx);
#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译
#endif


