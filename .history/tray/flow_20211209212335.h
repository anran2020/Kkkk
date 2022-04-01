

#ifndef _FLOW_H_
#define _FLOW_H_
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
#include "cause.h"

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
    ChainD *family;  /*闲时母体*/
    u8 refCnt;
    u8eNpType npType;
    s16 stepNpExpect;
    s16 stepNpMax;
    s16 stepNpMin;
}StepNpCfg;

/*流程保护配置*/
typedef struct
{
    ChainD chain;  /*忙时双链，闲时单链*/
    ChainD *family;  /*闲时母体*/
    u8 refCnt;
    u8eStepProtPolicy protPolicy;
    u8 rsvd[2];
    u32 sampleInterval;
    s32 reverseProtVol;
}FlowProtCfg;

/*流程保护入口*/
typedef struct flowProtEntryTag
{
    ChainD chain;  /*忙时双链，闲时单链*/
    u16 refCnt;
    u8 rsvd[2];
    FlowProtCfg *flowProtCfg;   /*流程保护中的配置参数*/
    ListS protList;  /*单链表*/
}FlowProtEntry;

/*工步保护入口*/
typedef struct stepProtEntryTag
{
    ChainD chain;  /*忙时双链，闲时单链*/
    u16 refCnt;
    u8 rsvd[2];
    ListS protList;
}StepProtEntry;

/*托盘保护入口*/
typedef struct trayProtEntryTag
{
    ChainD chain;  /*忙时双链，闲时单链*/
    u16 refCnt;
    u8 rsvd[2];
    ListS protList;  /*单链表*/
}TrayProtEntry;

/*流程入口*/
typedef struct flowStepEntryTag
{
    ChainD chain;  /*忙时双链，闲时单链*/
    u16 refCnt;
    u8 stepAmt;
    u8 rsvd;
    ListD stepList;  /*单链*/
}FlowStepEntry;

/*流程,工步或流程保护,及配置共用内存,名字不会了*/
typedef union
{
    TrayProtEntry trayProtEntry;
    FlowProtEntry flowProtEntry;
    StepProtEntry stepProtEntry;
    FlowStepEntry flowStepEntry;
}FlowEntry;

/*工步存储数据*/
typedef struct
{
    ChainD chain;  /*忙时双链，闲时单链*/
    ChainD *family;  /*闲时母体*/
    u16 refCnt;
    u8eStepType stepType;
    u8 paramEnable;
    StepNpCfg *stepNpCfg;
    s32 stepparam[0];   /*ParamAmt*/
}StepObj;

/*保护存储数据*/
typedef struct
{
    ChainD chain;  /*忙时双链，闲时单链*/
    ChainD *family;  /*闲时母体*/
    u16 refCnt;
    u16 protId;
    u8 rsvd[3];
    u8 paramEnable;
    s32 protParam[0];
}ProtObj;

/*工步保护参数,工步流程配置,共用内存,但配置不能参与联合*/
typedef union
{
    StepObj stepObj;
    ProtObj protObj;
}FlowObj;

/*保护和保护信息的通用结构*/
typedef struct
{
    ChainS chain;
    u16 objIdx;  /*工步或保护实体索引*/
    u16 protEntryIdx;  /*工步时保护入口索引，保护时无效*/
}FlowNode;

/*全程安全保护配置,代码内叫做托盘保护*/
typedef struct
{
    u32 npTime[NpTimeCri];
    u32 liftTime[MixSubCri];
    TrayProtEntry *protEntry;
}TrayProt;

/*资源均可独立修改，不知瓶颈可能是哪个*/
#define FlowObjAmt8  128    /*(sizeof(FlowObj)+32)*128 = 6k*/
#define FlowObjAmt6  384    /*(sizeof(FlowObj)+24)*384 = 16k*/
#define FlowObjAmt4  1024    /*(sizeof(FlowObj)+16)*1024 = 36k*/
#define FlowObjAmt2  768    /*(sizeof(FlowObj)+8)*768 = 21k*/
#define FlowNodeAmt  8192  /*sizeof(FlowNode)*8192 = 64k*/
#define FlowEntryAmt 640   /*sizeof(FlowEntry)*768 = 12k*/

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
    ListD idleObjList;  /*闲时单链用*/
    ListD busyStepObjList[StepTypeCri];
    ListD busyProtObjList[ProtGrpCri];

    ListS idleNodeList;

    ListD idleEntryList;  /*闲时单链用*/
    ListD busyStepEntryList;
    ListD busyProtEntryList[ProtGrpCri];

    FlowRecvCtrl *recvCtrl;  /*不用防呆,起定时防呆也行*/
}FlowCb;


extern FlowCb *gFlowCb;
extern u8 *gTmpFlowInfo;
extern u8 gTmpStepAmt;
extern u8eStepProtPolicy gTmpStepProtPolicy;
extern s32 gTmpReverseVol;
extern u8 *gTmpProtGenInfo; /*todo,临时用，要删除*/
extern u8 gTmpProtGenAmt; /*todo,临时用，要删除*/
extern u8 *gTmpProtStepInfo; /*todo,临时用，要删除*/
extern u8 gTmpProtStepAmt; /*todo,临时用，要删除*/
extern u8 *gTmpStepNpInfo;

#ifdef __cplusplus
extern "C"  {
#endif

extern FlowRecvCtrl *recvCtrlGet(void);
extern void recvCtrlFree(void);
extern u16eRspCode upFlowSave(u8 *flow);
extern u16eRspCode upProtGenSave(u8 *protGen);
extern u16eRspCode upProtStepSave(u8 *protStep);
extern void *getChnStepInfo(u8 stepId);
extern Ret flowInit(void);

#ifdef __cplusplus
}
#endif


#endif


