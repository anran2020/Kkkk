#ifndef _ENUM_H_
#define _ENUM_H_

#include "basic.h"

typedef u8 u8eCmmuItfType;
#define ItfTypeCan 0x00
#define ItfTypeUart 0x01
#define ItfTypeTcp 0x02
#define ItfTypeUdp 0x03
#define ItfTypeCri 0x04

/*与上位机联机时信息类型*/
typedef u16 u16eSubDevType;
#define DevTypeTray       0x00
#define DevTypeDcdcBox    0x01
#define DevTypeVolSmpl  0x02    /*电压采样板*/
#define DevTypeTmprSmpl 0x03    /*温度采样板*/
#define DevTypePlc        0x04
#define DevTypeNdbd  0x05    /*针床控制单片机*/
#define DevTypeSeriesSw   0x06    /*串联旁路切换板*/
#define DevTypeCri        0x07    /*非法临界值*/

/*与下位机联机时设备类型*/
typedef u8 u8eBoxDevType;  /*u8eBoxType, u16eSubDevType*/
#define BoxDevNull   0x00
#define BoxDevParallel  0x01
#define BoxDevSeriesWiSw  0x02
#define BoxDevSeriesWoSw  0x03
#define BoxDevSwitch  0x04
#define BoxDevTmprSmpl  0x05
#define BoxDevVolSmplResa  0x06
#define BoxDevVolSmplDsp 0x0b
#define BoxDevCri  0x0c

/*电源箱设备类型*/
typedef u8 u8eBoxType;
#define BoxTypeParallel   0x00
#define BoxTypeSeriesWiSw 0x01    /*带切入切出功能的串联*/
#define BoxTypeSeriesWoSw 0x02    /*不带切入切出功能的串联,也即极简串联*/
#define BoxTypeCri        0x03

typedef u8 u8eSmplType;
#define SmplTypeTray      0x00  /*整盘模式采样*/
#define SmplTypeChn       0x01  /*通道模式电芯数据*/
#define SmplTypeNdbd    0x02  /*通道模式针床数据*/
#define SmplTypeRun    0x03  /*通道模式运行数据*/
#define SmplTypeCri       0x04

typedef u8 u8eChnType;
#define ChnTypeMainChn      0x00   /*并联通道，串联主通道*/
#define ChnTypeSeriesCell   0x01   /*串联电芯*/
#define ChnTypeCri          0x02

typedef u8 u8eUpChnState;  /*用于通知上位机的状态*/
#define ChnUpStateOffline     0x00
#define ChnUpStateIdle        0x01   /*流程跑完且新流程未开始*/
#define ChnUpStateStop        0x02
#define ChnUpStatePause       0x03
#define ChnUpStateNp          0x04   /*等待负压*/
#define ChnUpStateStart       0x05   /*尝试启动中*/
#define ChnUpStateRun         0x06
#define ChnUpStateTemp        0x07  /*托盘采样中用于填充*/
#define ChnUpStateCri         0x08

/*非稳定态均要防呆,闲时忙时均基于稳定态*/
/*漏洞是部分非稳定态不参与保护逻辑*/
/*由于非稳定态本身会做时间防呆,那么非稳定态不参与保护也可以*/
/*其实,如果假定下位机没问题,也即不考虑对下位机防呆的话*/
/*中位机的状态管理会很简单*/
/*但若做防呆,基于下位机缓存采样,中位机的状态管理比较艰难*/
/*多种因素综合,可以结论为:下位机缓存采样是个失败的决策*/
typedef u8 u8eMedChnState;  /*中位机维护的通道运行状态*/
#define ChnStaIdle        0x00   /*流程跑完且新流程未开始*/
#define ChnStaStop        0x01
#define ChnStaPause       0x02
#define ChnStaNp          0x03   /*等待负压*/
#define ChnStaRun         0x04
#define ChnStaFlowStart   0x05   /*上位机流程启动*/
#define ChnStaStartReqLow   0x06   /*下位机截止触发的工步切换*/
#define ChnStaStartReqMed   0x07   /*中位机触发工步切换,这个目前还没有*/
#define ChnStaStopReqUp  0x08  /*上位机触发停止*/
#define ChnStaPauseReqUp  0x09  /*上位机触发暂停*/
#define ChnStaStopReqMed  0x0a  /*中位机保护触发停止*/
#define ChnStaPauseReqMed  0x0b  /*中位机保护触发暂停*/
#define ChnStaCri         0x0c

typedef u8 u8eStepType;  /*上位机可下发的类型，无操除外*/
#define StepTypeNull        0x00   /*无操作*/
#define StepTypeQuiet      0x01   /*搁置*/
#define StepTypeCCC         0x02   /*恒流充 constant current charge*/
#define StepTypeCCD         0x03   /*恒流放 constant current dis-charge*/
#define StepTypeCCCVC       0x04   /*恒流恒压充 constant current-->constant voltage charge*/
#define StepTypeCCCVD       0x05   /*恒流恒压放 constant current-->constant voltage dis-charge*/
#define StepTypeCVC         0x06   /*恒压充 constant voltage charge*/
#define StepTypeCVD         0x07   /*恒压放 constant voltage dis-charge*/
#define StepTypeLoop        0x08   /*循环*/
#define StepTypeDCIR        0x09   /**/
#define StepTypeCri         0x0a

typedef u8 u8eStepSubType;  /*工步中可能分阶段的情况*/
#define StepSubTypeNull    0x00   /*工步不分阶段*/
#define StepSubTypeCC      0x01   /*恒流恒压中的恒流段*/
#define StepSubTypeCV      0x02   /*恒流恒压中的恒压段*/
#define StepSubTypeCri     0x03

typedef u8 u8eTrayEnterState;  /*入料状态*/
#define EnterStateWoTray      0x00   /*无料*/
#define EnterStateWiTray      0x01   /*有料*/
#define EnterStateCri         0x02

typedef u8 u8eTrayTouchState;  /*压接状态*/
#define TouchStateWiTouch      0x01   /*压接*/
#define TouchStateWoTouch      0x02   /*脱开*/
#define TouchStateMoving       0x02   /*动作中*/
#define TouchStateHover       0x03   /*悬停*/
#define TouchStateCri          0x04

typedef u8 u8eNpType;  /*负压类型*/
#define NpTypeNone    0x00   /*无负压需求*/
#define NpTypeHigh    0x01   /*高低负压之高负压*/
#define NpTypeLow     0x02   /*高低负压之低负压*/
#define NpTypeNml     0x03   /*常压*/
#define NpTypeRatio   0x04   /*比例阀*/
#define NpTypeCri     0x05

typedef u8 u8eProtGrp;  /*保护群组,废弃,todo*/
#define ProtGrpQuiet        0x00   /*搁置*/
#define ProtGrpCCC      0x01   /*恒流充 含恒流恒压充电之恒流段*/
#define ProtGrpCCD         0x02   /*恒流放 含恒流恒压放电之恒流段*/
#define ProtGrpCVC         0x03   /*恒压充 含恒流恒压充电之恒压段*/
#define ProtGrpCVD       0x04   /*恒压放 含恒流恒压放电之恒压段*/
#define ProtGrpCCCVC       0x05   /*恒流恒压充电之特定保护*/
#define ProtGrpCCCVD         0x06   /*恒流恒压放电之特定保护*/
#define ProtGrpCri         0x07

typedef u8 u8eStepProtPolicy;
#define StepProtPause  0x00
#define StepProtStop   0x01
#define StepProtCri    0x02

typedef u8 u8eNpTime;
#define NpTimeVacuoMake   0x00  /*抽真空时间，要达到预期，T0*/
#define NpTimeVacuoBrk   0x01  /*破真空时间，达预期，及常压保护，T10*/
#define NpTimeVacuoStable   0x02  /*负压稳定时间，稳定后起工步*/
#define NpTimeVacuoProt   0x03  /*负压保护时间，T6*/
#define NpTimeStartExpr   0x04  /*负压超时时间，启动负压后该时间要能起工步*/
#define NpTimeVacuoBrkUrg   0x05  /*紧急破真空时间上限，超时要脱开针床，T7*/
#define NpTimeNdbdBrkDelay   0x06  /*顺利常压后延迟脱开针床时间，T9*/
#define NpTimeVacuoBrkDelay   0x07  /*针床脱开后延时关闭破真空时间，T8*/
#define NpTimeCri   0x08

typedef u8 u8eMixSubId;  /*组合保护因子*/
#define MixSubCccVolDownAbnm    0x00  /*CC*/
#define MixSubCccVolBigDown    0x01  /*CC*/
#define MixSubBusyQuietVolFluct    0x02  /*Quiet*/
#define MixSubIdleVolFluctBig    0x03
#define MixSubIdleVolFluctSml    0x04
#define MixSubAllVolCrashStop    0x05
#define MixSubIdleChnTmprUpSud    0x06
#define MixSubIdleChnTmprUpCtnu    0x07
#define MixSubBusyChnTmprBigRise    0x08
#define MixSubAllTmprMax    0x09
#define MixSubAllAirPrsMax    0x0a
#define MixSubBusyNpBigRise    0x0b
#define MixSubAllSmoke    0x0c
#define MixSubIdleCurLeak    0x0d   /*闲时漏电流*/
#define MixSubVolIntvlOfst    0x0e  /*电压间隔偏差*/
#define MixSubCri    0x0f

typedef u8 u8eProtPolicy;  /*保护处理策略,调整,todo*/
#define PolicyNdbdBrk     0x00   /*弹开库位针床*/
#define PolicyFmsNtfy     0x01   /*通知fms*/
#define PolicyGasEnable     0x02   /*打开气消防*/
#define PolicyItfDisplay     0x03   /*弹屏*/
#define PolicyFireDoorClose     0x04   /*关消防门*/
#define PolicyFanStopTray   0x05   /*停风扇*/
#define PolicySmokeRemove   0x06   /*排烟*/
#define PolicyFireDoorCloseAll    0x07   /*关所有消防门*/
#define PolicyPowerOff      0x08   /*设备掉电*/
#define PolicyCri           0x09

typedef u8 u8eDbgMsgId;
#define DbgMsgConn  0x00
#define DbgMsgDisc  0x01
#define DbgMsgCmd   0x02
#define DbgMsgLog   0x03
#define DbgMsgCri   0x04

typedef u8 u8eCaliMode;
#define CaliModeFirst  0x00
#define CaliModeReChk  0x01
#define CaliModeMeasure  0x02
#define CaliModeCri  0x03

typedef u8 u8eCaliType;
#define CaliTypeCurCharge  0x00
#define CaliTypeCurDisCharge  0x01
#define CaliTypeVolCharge  0x02
#define CaliTypeVolDisCharge  0x03
#define CaliTypeLineOrder  0x04   /*工装有,电源箱没有*/
#define CaliTypeImpedance  0x05   /*工装有,电源箱没有*/
#define CaliTypeCri  0x06

typedef u8 u8eVolType;
#define VolTypeCell  0x00
#define VolTypeCur  0x01
#define VolTypePort  0x02
#define VolTypeInner  0x03
#define VolTypeCri  0x04

typedef u8 u8eFixtChnSwType;
#define FixtChnSwIdle  0x00
#define FixtChnSwSingle  0x01
#define FixtChnSwDouble  0x02
#define FixtChnSwMulti  0x03
#define FixtChnSwSeries  0x04
#define FixtChnSwCri  0x05

typedef u8 u8eFixtPrecSmplType;
#define FixtPrecSmplAglt  0x00
#define FixtPrecSmplFixt  0x01
#define FixtPrecSmplCri  0x02

typedef u8 u8eUpFixtDevType;
#define UpFixtTypePrecSeries   0x00  /*串联电流电压精度线序工装*/
#define UpFixtTypeTmpr    0x01  /*温度线序工装*/
#define UpFixtTypeClean   0x02  /*负压清洗工装*/
#define UpFixtTypeGas   0x03  /*气密性工装*/
#define UpFixtTypeFlow   0x04  /*负压流量工装*/
#define UpFixtTypeSuctIn   0x05  /*插吸嘴工装*/
#define UpFixtTypeSuctOut   0x06  /*拔吸嘴工装*/
#define UpFixtTypeLocat   0x07  /*定位工装*/
#define UpFixtTypePrecParal   0x08  /*并联精度线序工装,与串联共用协议*/
#define UpFixtTypeCri   0x09

typedef u8 u8eUpdFileType;
#define UpdateFileApp  0x00  /**/
#define UpdateFileBoot  0x01  /**/
#define UpdateFileCfg  0x02  /**/
#define UpdateFileCri  0x03

/*静态指设备上电即可初始化,动态指需要时才能初始化*/
/*目前,工装属于动态,其余属于静态*/
/*串口上的设备类型,有联动,中间有等值,维护需小心*/
typedef u8 u8eUartDevType;
#define UartTmprSmpl 0x00
#define UartTmprCtrl 0x01
#define UartNdbdCtrl  0x02
#define UartNeedSmplCri 0x03   /*采样与否分割线*/
#define UartStaticCri 0x03   /*静态和动态分隔线*/
#define UartFixtTmpr 0x03
#define UartFixtClean 0x04
#define UartFixtGas 0x05
#define UartFixtPrecParal 0x06
#define UartFixtPrecSeries 0x07
#define UartFixtSuctIn 0x08
#define UartFixtSuctOut 0x09
#define UartFixtLocat 0x0a
#define UartFixtFlow 0x0b
#define UartDevTypeCri 0x0c

/*通讯地址基址*/
#define UartAddrBaseTmprSmpl 0x00
#define UartAddrBaseTmprCtrl 0x10
#define UartAddrBaseNdbdCtrl 0x20
#define UartAddrBaseFixtTmpr 0xc0
#define UartAddrBaseFixtClean 0xc4
#define UartAddrBaseFixtGas 0xc8
#define UartAddrBaseFixtPrecParal 0xcc
#define UartAddrBaseFixtPrecSeries 0xd0
#define UartAddrBaseFixtSuctIn 0xd4
#define UartAddrBaseFixtSuctOut 0xd8
#define UartAddrBaseFixtLocat 0xdc
#define UartAddrBaseFixtFlow 0xe0

typedef u8 u8eUpUpdDevType;
#define UpUpdDevMedium  0x00  /*中位机*/
#define UpUpdDevBox  0x01  /*电源柜下位机*/
#define UpUpdDevNdbd  0x02  /*针床下位机*/
#define UpUpdDevTmpr  0x03  /*温度采样板*/
#define UpUpdDevVolResa  0x04  /*瑞萨电压采样板*/
#define UpUpdDevSw  0x05  /*串联旁路切换板*/
#define UpUpdDevVolDsp 0x06  /*dsp电压采样板*/
#define UpUpdDevCri  0x07

/*针床状态,传感器为主,少量逻辑状态*/
typedef u8 u8eNdbdStaType;
#define NdbdSenNpVal 0x00  /*负压 区分正负,单位百帕*/
#define NdbdStaEnter 0x01  /*入料,0--无 1--有*/
#define NdbdStaTouch 0x02  /*压合 1压合2分离3运动4悬停*/
#define NdbdSenFwdEnter 0x03  /*前入料微动0无感1有感应*/
#define NdbdSenBackEnter 0x04  /*后入料微动0无感1有感应*/
#define NdbdSenFowTouch 0x05  /*前压合微动0无感1有感应*/
#define NdbdSenBackTouch 0x06  /*后压合微动0无感1有感应*/
#define NdbdStaWorkMode 0x07  /*工作模式0工作1维护*/
#define NdbdSenRatioVal 0x08  /*比例阀模拟量 0~7000*/
#define NdbdSenSwValve 0x09  /*导通阀0开1关*/
#define NdbdSenBrkVacum 0x0a  /*破真空0关1开*/
#define NdbdSenFireDoorUp 0x0b  /*消防门上限感应0无感1有感*/
#define NdbdSenFireDoorDown 0x0c  /*消防门下限感应0无感1有感*/
#define NdbdSenFwdDoorMsw 0x0d  /*前面微动,库位门0关1开*/
#define NdbdSenBackDoor 0x0e  /*后门检测0关1开*/
#define NdbdSenCylinderDown 0x0f  /*气缸下感应0无感1有感*/
#define NdbdSenCylinderUp 0x10  /*气缸上感应0无感1有感*/
#define NdbdSenCri 0x11  /**/

/*针床告警状态或传感器*/
/*有告警时不能启动新的流程,部分告警发生时需中断流程*/
/*如非特别标注，均为0正常1异常*/
typedef u8 u8eNdbdWarnType;
#define NdbdWarnScram 0x00  /*急停,手消*/
#define NdbdWarnGas 0x01  /*气压,需中断,手消*/
#define NdbdWarnSmoke 0x02  /*需中断,手消,0正常1前2后3全*/
#define NdbdWarnCo 0x03  /*需中断但不参与消防,手消*/
#define NdbdWarnCylinder 0x04  /*气缸,手消,0正常1压合2分离3感应器*/
#define NdbdWarnFan 0x05  /*需中断,自消*/
#define NdbdWarnFireDoor 0x06  /*消防门,手消*/
#define NdbdWarnSlotDoorRast 0x07  /*库位门(前门)光栅对射,自消*/
#define NdbdWarnTray 0x08  /*托盘,手消*/
#define NdbdWarnFwdDoorMsw 0x09  /*库位门(前门)微动,自消*/
#define NdbdWarnBackDoor 0x0a  /*后门,自消*/
#define NdbdWarnRatio 0x0b  /*比例阀开度,自消*/
#define NdbdWarnScapeGoat 0x0c  /*替罪羊,自消*/
#define NdbdWarnCri 0x0d  /**/

/*配置类*/
typedef u8 u8eNdbdCtrlType;
#define NdbdSetTouch 0x00  /*压合控制0保持1压合2分离,中位机不写0*/
#define NdbdSetFireNtfy 0x01  /*0关1开,中位机不写0*/
#define NdbdSetWarnDel 0x02  /*0使能1消警,中位机不写0*/
#define NdbdSetFireDoor 0x03  /*0保持1打开2关闭,中位机不写0*/
#define NdbdSetRatioVal 0x04  /*plc要求:0~900的负压值,0表关*/
#define NdbdSetSwValve 0x05  /*导通阀0开1关*/
#define NdbdSetBrkVacum 0x06  /*破真空0关1开*/
#define NdbdSetFixtPower 0x07  /*工装供电0断1供,PLC需防呆*/
#define NdbdSetFan 0x08  /*风扇0停1转*/
#define NdbdSetTypeCri 0x09  /**/

/*针床数据上传时的bitmap*/
/*状态bitmap*/
#define BitSenFwdEnter 0x00  /*前入料微动0无感1有感应*/
#define BitSenBackEnter 0x01  /*后入料微动0无感1有感应*/
#define BitSenFwdTouch 0x02  /*前压合微动0无感1有感应*/
#define BitSenBackTouch 0x03  /*后压合微动0无感1有感应*/
#define BitSenWorkMode 0x04  /*工作模式0工作1维护*/
#define BitSenSwValve 0x05  /*导通阀0开1关*/
#define BitSenBrkVacum 0x06  /*破真空0关1开*/
#define BitSenFireDoorUp 0x07  /*消防门上限感应0无感1有感*/
#define BitSenFireDoorDown 0x08  /*消防门下限感应0无感1有感*/
#define BitSenFwdDoorMsw 0x09  /*前面微动,库位门0关1开*/
#define BitSenBackDoor 0x0a  /*后门检测0关1开*/
#define BitSenCylinderUp 0x0b  /*气缸上感应0无感1有感*/
#define BitSenCylinderDown 0x0c  /*气缸下感应0无感1有感*/
#define BitSenCri   0x0d

/*告警bitmap*/
#define BitWarnScram 0x00    /*急停*/
#define BitWarnGas 0x01  /*气压*/
#define BitWarnCo 0x02
#define BitWarnFan 0x03
#define BitWarnFireDoor 0x04  /*消防门,手消*/
#define BitWarnSlotDoorRast 0x05  /*库位门(前门)光栅对射*/
#define BitWarnTray 0x06
#define BitWarnFwdDoorMsw 0x07  /*库位门(前门)微动*/
#define BitWarnBackDoor 0x08
#define BitWarnRatio 0x09  /*比例阀开度*/
#define BitWarnScapeGoat 0x0a  /*替罪羊*/
#define BitWarnCri 0x0b

/*针床动作操作类型*/
typedef u8 u8eNdbdActType;
#define NdbdActTouch 0x00
#define NdbdActFan 0x01
#define NdbdActFireDoor 0x02
#define NdbdActSlotDoor 0x03
#define NdbdActHeat 0x04
#define NdbdActCri 0x05

/*比例阀负压操作类型*/
/*plc说常态应为破关比关导开,如此对设备较好*/
typedef u8 u8eNpOprCode;
#define NpOprRatioBrkVacum 0x00  /*破真空*/
#define NpOprRatioMkVacum 0x01  /*抽真空*/
#define NpOprRatioHold 0x02  /*保压*/
#define NpOprRatioSwRatio 0x03  /*比例阀*/
#define NpOprRatioSwValve 0x04  /*导通阀*/
#define NpOprRatioSwBrk 0x05  /*破真空阀*/
#define NpOprRatioReset 0x6  /*比例阀复位,破关比关导开*/
#define NpOprRatioCri  0x07


/*负压设备类型*/
typedef u8 u8eNpDevType;
#define NpDevRatio 0x00
#define NpDevHighLow 0x01
#define NpDevTypeCri 0x02

/*负压清洗工装动作类型*/
typedef u8 u8eFixtCleanActType;
#define FixtCleanActPush 0x00  /*压液至管道负压杯*/
#define FixtCleanActSuck 0x01  /*抽液至原位*/
#define FixtCleanActStop 0x02  /*停止*/
#define FixtCleanActCri 0x03

/*拔吸嘴工装动作类型*/
typedef u8 u8eFixtSuckOutStaType;
#define FixtSuckOutStaRelax 0x00  /*松开*/
#define FixtSuckOutStaHold 0x01  /*夹持*/
#define FixtSuckOutStaRun 0x02  /*运动中*/
#define FixtSuckOutStaAbnm 0x03  /*异常*/
#define FixtSuckOutStaCri 0x04

/*定位工装定位杆位置类型*/
typedef u8 u8eFixtLocatPos;
#define FixtLocatPosAbnm 0x00  /**/
#define FixtLocatPos1 0x01  /**/
#define FixtLocatPos2 0x02  /**/
#define FixtLocatPos3 0x03  /**/
#define FixtLocatPosRun 0x10
#define FixtLocatPosCri 0x11

/*定位工装转动类型*/
typedef u8 u8eFixtLocatTurnType;
#define FixtLocatTurnIgnore 0x00  /**/
#define FixtLocatTurnFwd 0x01  /*正转*/
#define FixtLocatTurnBkwd 0x02  /*反转*/
#define FixtLocatTurnCri 0x03  /**/

typedef u8 u8eCauseCodeLow;
#define CcLowTime  0x01
#define CcLowCap  0x02
#define CcLowVol  0x03
#define CcLowCur  0x04
#define CcLowUser  0x07


#endif

