

#ifndef _FLOW_H_
#define _FLOW_H_

#include "basic.h"
#if TRAY_ENABLE
#include "define.h"
#include "enum.h"
#include "type.h"
#include "cause.h"

struct channelTag;
struct trayTag;

/*
内存有限，不能敞开存流程保护，对外宣称又不能磕碜。
优化内存效率，1，结构离散化，为主，2，索引替代指针，为辅。
机制会有内存及性能消耗。方案占据内存约160kB。

是否支持单点，区别只在于工步和保护的存储。由于资源限制，支持单点
需要做性能的牺牲，虽然牺牲非常有限，但毕竟不支持单点的性能要高于
支持单点，所以是否可以考虑通过配置做成两种工作模式，只在用户要跑
单点时切换为支持单点模式。模式切换需要重启中位机，不知道客户是否
接受，现在先按支持单点做，如果确实需要优化这一丢丢性能，再按区分
模式做。
*/

/*
工步保护--工步独有保护，每个工步可以不同
流程保护--流程统一保护，流程内所有工步都适用
托盘保护--托盘统一保护，托盘内所有流程都适用，且包含闲时
托盘VS工步、流程，前者用全程保护命令，后者用工步保护命令
工步VS流程，后者适用工步号为0xff，而前者不是。 
*/

/*工步负压需求，即工步保护配置*/
typedef struct
{
    ChainD chain;  /*忙时双链，闲时单链*/
    u16 refCnt;
    s16 stepNpExpect;
    u8eNpType npType;
    u8 rsvd[3];
    s16 stepNpMax;
    s16 stepNpMin;
}StepNpCfg;

/*流程保护配置*/
typedef struct
{
    ChainD chain;
    u16 refCnt;
    u8eChnProtPolicy protPolicy;
    u8 rsvd;
    u32 sampleInterval;
    s32 reverseProtVol;
}FlowProtCfg;

/*工步配置和流程配置*/
typedef union
{
    StepNpCfg stepNpCfg;
    FlowProtCfg flowProtCfg;
}FlowStepCfg;

/*流程保护入口*/
typedef struct flowProtEntryTag
{
    ChainD chain;  /*忙时双链*/
    u16 refCnt;
    u8 rsvd[2];
    FlowProtCfg *flowProtCfg;   /*流程保护中的配置参数*/
    ListS protList;  /*单链表*/
    ChainS *tail;
}FlowProtEntry;

/*工步保护入口*/
typedef struct stepProtEntryTag
{
    ChainD chain;  /*忙时双链*/
    u16 refCnt;
    u8 rsvd[2];
    ListS protList;
    ChainS *tail;
}StepProtEntry;

/*托盘保护入口*/
typedef struct trayProtEntryTag
{
    ChainD chain;  /*忙时双链*/
    u16 refCnt;
    u8 rsvd[2];
    ListS protList;  /*单链表*/
    ChainS *tail;
}TrayProtEntry;

/*流程入口*/
typedef struct flowStepEntryTag
{
    ChainD chain;  /*忙时双链*/
    u16 refCnt;
    u8 stepAmt;
    u8 loopStepAmt;  /*流程中的循环工步数量*/
    ListS stepList;  /*单链*/
    ChainS *tail;
}FlowStepEntry;

/*流程,工步或流程保护,及配置共用内存,名字不会了*/
typedef union
{
    TrayProtEntry trayProtEntry;
    FlowProtEntry flowProtEntry;
    StepProtEntry stepProtEntry;
    FlowStepEntry flowStepEntry;
}FlowEntry;

typedef u8 u8eObjAmtId;
#define ObjAmtId2  0
#define ObjAmtId4  1
#define ObjAmtId6  2
#define ObjAmtId8  3
#define ObjAmtIdCri  4

/*工步存储数据*/
typedef struct
{
    ChainD chain;  /*忙时双链*/
    u8eObjAmtId objAmtId;  /*标记闲时母体,用于释放*/
    u8 rsvd01;
    u16 refCnt;
    u8 paramEnable;
    u8eStepType stepType;
    u16 rsvd02;
    s32 stepParam[0];   /*ParamAmt*/
}StepObj;

/*保护存储数据*/
typedef struct
{
    ChainD chain;  /*忙时双链*/
    u8eObjAmtId objAmtId;  /*标记闲时母体,用于释放*/
    u8 rsvd01;
    u16 refCnt;
    u8 paramEnable;
    u8 rsvd02;
    u16 protId;
    s32 protParam[0];
}ProtObj;

/*工步保护参数,工步流程配置,共用内存,但配置不能参与联合*/
typedef union
{
    StepObj stepObj;
    ProtObj protObj;
}FlowObj;

/*工步流程节点*/
typedef struct
{
    ChainS chain;  /*用单链,出于节约4字节*/
    StepObj *stepObj;  /*工步实体*/
    StepProtEntry *stepProtEntry;  /*工步保护入口*/
    StepNpCfg *stepNpCfg;  /*工步配置,目前只有负压*/
    u8 stepId;
    u8 rsvd[3];
}StepNode;

/*保护信息节点*/
typedef struct
{
    ChainS chain;  /*用单链,出于节约4字节*/
    ProtObj *protObj;  /*保护实体*/
}ProtNode;

/*全程安全保护配置,代码内叫做托盘保护*/
typedef struct
{
    u32 npTime[NpTimeCri];
    u32 liftTime[MixSubCri];
    TrayProtEntry *trayProtEntry;
}TrayProt;

/*资源均可独立修改，不知瓶颈可能是哪个*/
#if 1
#define FlowObjAmt8  64    /*(sizeof(FlowObj)+32)*128 = 6k*/
#define FlowObjAmt6  384    /*(sizeof(FlowObj)+24)*384 = 16k*/
#define FlowObjAmt4  1024    /*(sizeof(FlowObj)+16)*1024 = 36k*/
#define FlowObjAmt2  768    /*(sizeof(FlowObj)+8)*768 = 21k*/
#define StepNodeAmt  1024  /*sizeof(StepNode)*4096 = 48k*/
#define ProtNodeAmt  3072  /*sizeof(ProtNode)*4096 = 48k*/
#define FlowEntryAmt 768   /*sizeof(FlowEntry)*768 = 15k*/
#define FlowStepCfgAmt 16  /*sizeof(FlowStepCfg)*16 <= 1k*/
#else  /*临时调试用*/
#define FlowObjAmt8  64    /*(sizeof(FlowObj)+32)*128 = 6k*/
#define FlowObjAmt6  384    /*(sizeof(FlowObj)+24)*384 = 16k*/
#define FlowObjAmt4  1024    /*(sizeof(FlowObj)+16)*1024 = 36k*/
#define FlowObjAmt2  17    /*(sizeof(FlowObj)+8)*768 = 21k*/
#define StepNodeAmt  12  /*sizeof(StepNode)*4096 = 48k*/
#define ProtNodeAmt  11  /*sizeof(ProtNode)*4096 = 48k*/
#define FlowEntryAmt 6   /*sizeof(FlowEntry)*768 = 15k*/
#define FlowStepCfgAmt 1  /*sizeof(FlowStepCfg)*16 <= 1k*/
#endif

typedef struct
{
    u16 recvMsgId;
    u16 total;
    u16 offsetHope;
    u16 chnAmt;   /*全程保护无效*/
    u16 *chnIdx;   /*全程保护无效*/
}FlowRecvCtrl;

typedef struct
{
    u16 flowObjAmt[ObjAmtIdCri];  /*FlowObjAmt2,FlowObjAmt4,FlowObjAmt6,FlowObjAmt8*/
    u8 objParamAmt[ObjAmtIdCri]; /*2,4,6,8*/

    ListD idleObjList[ObjAmtIdCri];  /*存放工步/保护实体的内存集*/
    ListD busyStepObjList[StepTypeCri];  /*忙时双链,存放工步实体*/
    ListD busyProtObjList[ProtGrpCri];  /*忙时双链,存放保护实体*/

    ListS idleStepNodeList;    /*单链,存放工步节点*/
    ListS idleProtNodeList;    /*单链,存放保护节点*/

    ListD idleEntryList;  /*存放流程和保护入口的内存集*/
    ListD busyStepEntryList;  /*忙时双链,存放流程入口*/
    ListD busyProtEntryList[ProtGrpCri];  /*忙时双链,存放保护入口*/

    ListD idleProtCfgList;  /*流程中的配置项,含流程保护配置项和工步负压配置*/
    ListD busyFlowProtCfgList;  /*流程保护中的配置项*/
    ListD busyStepProtCfgList;  /*流程工步中的配置项,目前只有负压*/

    FlowRecvCtrl *recvCtrl[3];  /*不用防呆,起定时防呆也行*/
}FlowCb;

#ifdef __cplusplus
extern "C"  {
#endif

struct channelTag;

extern FlowRecvCtrl *recvCtrlGet(u8);
extern void recvCtrlFree(u8);
extern u16eRspCode upFlowStepSave(u8 *flow);
extern u16eRspCode upTrayProtSave(u8 *protGen);
extern u16eRspCode upStepProtSave(u8 *protStep);
extern u32 getStepEndTime(StepObj *step);
extern Ret flowInit(u8 trayAmt);
extern StepNode *getNxtStep(struct channelTag *chn, StepNode *base, b8 realSw);
extern StepNode *findStepNode(FlowStepEntry *flowStepEntry, u8 stepId);
extern u16eRspCode protExpSave(u8 *cmd);

#ifdef __cplusplus
}
#endif

#endif
#endif


