

#ifndef _CAUSE_H_
#define _CAUSE_H_

#include "basic.h"
#if TRAY_ENABLE

/*原因码分为命令响应码和通道原因码*/

/*---------------命令响应码，开始-----------------*/
/*0~255留给其它下位机,中位机定义的原因码从256开始*/
typedef u16 u16eRspCode;
#define RspOk 0x0000
#define RspErr 0x0100
#define RspRetry 0x0101  /*临时应答*/
#define RspTouch 0x0102  /*托盘未压合*/
#define RspAddr 0x0103  /*寻址错误*/
#define RspStatusBox 0x0104  /*电源箱维护状态*/
#define RspParam 0x0105  /*参数错误*/
#define RspNoRes 0x0106  /*资源不足*/
#define RspWarn 0x0107  /*存在告警*/
#define RspBusy 0x0108  /*等应答后再发命令*/
#define RspDiscBox 0x0109  /*电源箱不在线*/
#define RspExpr 0x010a  /*级联超时*/
#define RspStep 0x010b  /*工步未下发*/
#define RspProt 0x010c  /*保护未下发*/
#define RspPage 0x010d  /*升级传输协商失败*/
#define RspDiscFixt 0x010e  /*工装不在线*/
#define RspDiscTmpr 0x010f  /*温度板不在线*/
#define RspDiscNdbd 0x0110  /*针床设备不在线*/
#define RspChnBeRun 0x0111  /*通道运行中*/
#define RspNdbdMntn 0x0112  /*针床维护中*/
#define RspResAbnml 0x0113  /*异常错误*/
#define RspChnIdx 0x0114  /*通道号错误*/
#define RspStepFst 0x0115  /*工步首包错误*/
#define RspProtFst 0x0116  /*保护首包错误*/
#define RspMultiSeq 0x0117  /*多包序号错误*/
#define RspFlow 0x0118  /*流程不完整*/
#define RspChnWiRun 0x0119  /*主通道忙*/
#define RspChnWoRun 0x011a  /*主通道未运行*/
#define RspRgbCtrl 0x011b  /*三色灯指示错误*/
#define RspUnknown 0x011c  /*不识别信息*/
#define RspRepeatDrop 0x011d  /*重发报文,不算错误*/
#define RspMissInfo 0x011e  /*必选信息缺失*/
#define RspMissFlow 0x011f  /*保护通道缺少流程*/
#define RspRepeatParam 0x0120  /*信息重复*/
#define RspLoopAmt 0x0121  /*循环工步超限*/
#define RspLoopParam 0x0122  /*循环参数错误*/
#define RspDiskRead 0x0123  /*读磁盘错误*/
#define RspWoMntn 0x0124  /*不在维护状态*/
/*---------------响应码，结束-----------------*/


/*---------------采样通道原因码，开始-----------------*/
/*0~255留给电源柜下位机异常码,256~512留给电源柜下位机响应码*/
/*中位机定义的原因码从1024开始*/
/*中位机异常码划分优先级,优先级高的可以替换优先级低的*/
/*但是触发高级保护的通道的异常码不能被替换*/
typedef u16 u16eCauseCode;
#define CcNone 0x0000
#define CcLowTimeEnd  0x0001
#define CcLowCcVolEnd  0x0002
#define CcLowCurEnd  0x0003
#define CcLowCapEnd  0x0004
#define CcLowCvVolEnd  0x0005
#define CcFlowPauseEnd 0x0006
#define CcFlowStopEnd  0x0007  /*收到命令而截止*/
#define Cclv0Base 0x0400   /*托盘(不涉组合)*/
#define Cclv1Base 0x0800  /*通道(不涉组合不涉全盘)*/
#define Cclv2Base 0x0a00  /*通道(不涉组合但涉全盘)*/
#define Cclv3Base 0x0c00  /*托盘级因子*/
#define Cclv4Base 0x0d00  /*单通道因子+组合保护波及*/
#define CcMixBase 0x0e00  /*组合*/
#define CcBaseGet(cc) (0xff00 & (cc))
#define CcChkModify(oldCc, newCc) \
do \
{ \
    if (CcNone==(oldCc) || (oldCc)<CcBaseGet(newCc)) \
    { \
        (oldCc) = (newCc); \
    } \
}while(0)

/*受(非组合)托盘保护波及或者普通托盘保护,优先级最低*/
#define Cc0TrayTrig 0x0400  /*(非组合非托盘级因子)托盘级保护波及*/
#define Cc0SeriesTrig 0x0401  /*串联波及保护*/
#define Cc0BusySlotTmprLowLmt 0x0402  /*忙时库温下限*/
#define Cc0TmprBoxOffline   0x0403
#define Cc0MkNpExpr   0x0404   /*工步前抽负压超时*/
#define Cc0StepNpOverLmt   0x0405  /*流程中负压超限*/
#define Cc0TrayGas   0x0406
#define Cc0TrayCo   0x0407  /*co告警停流程但不参与消防*/
#define Cc0TrayFan   0x0408
#define Cc0SlotTmprBad   0x0409
#define Cc0NdbdDisc   0x040a  /*针床数据无效--PLC断线*/
#define Cc0AllSlotTmprUpLmt 0x040b  /*全时库位温度上限*/
#define Cc0NdbdBrkAbnml 0x040c  /*针床异常脱开*/
#define Cc0NdbdMntn 0x040d  /*针床维护*/

/*(非因子)通道保护,且不涉全盘,优先级高于前者*/
#define Cc1LowCfg  0x0800   /*,下位机规格不匹配*/
#define Cc1CellTmprBad 0x0801  /*温度无效--温度盒断线*/
#define Cc1LowProto 0x0802  /*下位机协议不匹配*/
#define Cc1Reverse 0x0803  /*反接保护*/
#define Cc1MedReset 0x0804  /*中位机重启*/
#define Cc1QuietCurLeak 0x0805  /**/
#define Cc1StepQuietVolUpLmt 0x0806  /**/
#define Cc1StepQuietVolLowLmt 0x0807  /**/
#define Cc1StepQuietChnTmprBigRise 0x0808  /**/
#define Cc1CccSetTimeVolUpLmt 0x0809  /**/
#define Cc1CccSetTimeVolLowLmt 0x080a  /**/
#define Cc1CccCurOfst 0x080b  /**/
#define Cc1CccVolRiseRateUpLmt 0x080c  /**/
#define Cc1CccVolRiseRateLowLmt 0x080d  /**/
#define Cc1CcdVolIntvlFluctUpLmt 0x080e  /**/
#define Cc1StepCccCapUpLmt 0x080f  /**/
#define Cc1StepCccCapLowLmt 0x0810  /**/
#define Cc1CccVolIntvlFluctUpLmt 0x0811  /**/
#define Cc1CccVolIntvlFluctLowLmt 0x0812  /**/
#define Cc1CccVolRiseCtnu 0x0813  /**/
#define Cc1CccVolDownCtnu 0x0814  /**/
#define Cc1CcdVolIntvlFluctLowLmt 0x0815  /**/
#define Cc1CvcVolOfst 0x0816  /*恒压充电电压超差*/
#define Cc1CvcCurRiseAbnm 0x0817  /*恒压充电电流上升异常点*/
#define Cc1CvcCurBigRise 0x0818  /*恒压充电电流突升*/
#define Cc1QuietVolChkBack 0x0819  /*电压回检即电流电芯双压差*/
#define Cc1QuietVolFluct 0x081a  /**/
#define Cc1StepCccChnTmprBigRise 0x081b  /**/
#define Cc1StepCcdChnTmprBigRise 0x081c  /**/
#define Cc1StepCccvcChnTmprBigRise 0x081d  /**/
#define Cc1StepCccvdChnTmprBigRise 0x081e  /**/
#define Cc1StepCvcChnTmprBigRise 0x081f  /**/
#define Cc1StepCvdChnTmprBigRise 0x0820  /**/
#define Cc1StepCccVolUpLmt 0x0821  /**/
#define Cc1StepCcdVolUpLmt 0x0822  /**/
#define Cc1StepCccvcVolUpLmt 0x0823  /**/
#define Cc1StepCccvdVolUpLmt 0x0824  /**/
#define Cc1StepCvcVolUpLmt 0x0825  /**/
#define Cc1StepCvdVolUpLmt 0x0826  /**/
#define Cc1StepCccVolLowLmt 0x0827  /**/
#define Cc1StepCcdVolLowLmt 0x0828  /**/
#define Cc1StepCccvcVolLowLmt 0x0829  /**/
#define Cc1StepCccvdVolLowLmt 0x082a  /**/
#define Cc1StepCvcVolLowLmt 0x082b  /**/
#define Cc1StepCvdVolLowLmt 0x082c  /**/
#define Cc1CcdCurOfst 0x082d  /**/
#define Cc1StepCcdCapUpLmt 0x082e  /**/
#define Cc1StepCccvcCapUpLmt 0x082f  /**/
#define Cc1StepCccvdCapUpLmt 0x0830  /**/
#define Cc1StepCvcCapUpLmt 0x0831  /**/
#define Cc1StepCvdCapUpLmt 0x0832  /**/
#define Cc1StepCcdCapLowLmt 0x0833  /**/
#define Cc1StepCccvcCapLowLmt 0x0834  /**/
#define Cc1StepCccvdCapLowLmt 0x0835  /**/
#define Cc1StepCvcCapLowLmt 0x0836  /**/
#define Cc1StepCvdCapLowLmt 0x0837  /**/
#define Cc1LowStartExpr 0x0838  /*工步启动超时*/
#define Cc1LowEndExpr 0x0839  /*工步停止超时*/
#define Cc1LowAbnmlEnd 0x083a  /*运行中异常截止*/
#define Cc1BoxOffline 0x083b  /*离线异常*/
#define Cc1ChnNoData 0x083c  /*通道无数据*/
#define Cc1DvVolOfst 0x083d  /*恒压放电电压超差*/
#define Cc1DvCurRiseAbnm 0x083e  /*恒压放电电流上升异常点*/
#define Cc1DvCurBigRise 0x083f  /*恒压放电电流突升*/

/*(非因子)通道保护,但涉全盘,优先级高于前者*/
#define Cc2BusyChnTmprLowLmt 0x0a01  /*忙时通道温度下限*/
#define Cc2FlowChnTmprUpLmt 0x0a02  /*流程通道温度上限*/
#define Cc2FlowChnTmprLowLmt 0x0a03  /*流程通道温度下限*/
#define Cc2FlowChnChgOvldVol 0x0a04  /**/
#define Cc2FlowChnChgOvldCap 0x0a05  /**/
#define Cc2FlowChnChgOvldCur 0x0a06  /**/
#define Cc2FlowChnDisChgOvldVol 0x0a07  /**/
#define Cc2FlowChnDisChgOvldCap 0x0a08  /**/
#define Cc2FlowChnDisChgOvldCur 0x0a09  /**/
#define Cc2FlowChnVolCell2Port 0x0a0a  /*接触不良*/
#define Cc2FlowChnVolCell2Cur 0x0a0b  /*极耳不良*/
#define Cc2FlowChnContactResis 0x0a0c  /*接触阻抗*/
#define Cc2FlowChnLoopResis 0x0a0d  /*回路阻抗*/
#define Cc2FlowChnCapTtl 0x0a0e  /*充放电总容量*/

/*因子托盘保护,优先级高于前者*/
#define Cc3UpDiscExpr 0x0c00  /*上位机脱机*/
#define Cc3AllSmoke 0x0c02  /*全时烟感*/
#define Cc3AllNpUpLmt 0x0c03  /*全时气压上限*/
#define Cc3BusyNpBigRise 0x0c04  /*忙时气压突升*/
#define Cc3AllCo 0x0c05  /*全时co*/

/*因子通道保护+组合保护波及,优先级仅次于触发组合的通道*/
/*单项保护时,忙时闲时全时流程这4类涉及全盘,其余只涉通道不涉全盘*/
#define Cc4TrayMixTrag 0x0d00 /*组合保护波及*/
#define Cc4IdleVolBigRise1 0x0d01  /*闲时电压突升1*/
#define Cc4IdleVolBigRise2 0x0d02  /*闲时电压突升2*/
#define Cc4IdleVolCtnuSmlRise 0x0d03  /*闲时电压连续小幅上升*/
#define Cc4IdleVolBigDown1 0x0d04  /*闲时电压突降1*/
#define Cc4IdleVolBigDown2 0x0d05  /*闲时电压突降2*/
#define Cc4IdleVolCtnuSmlDown 0x0d06  /*闲时电压连续小幅下降*/
#define Cc4IdleTmprBigRise 0x0d07  /*闲时温度突升*/
#define Cc4IdleTmprCtnuSmlRise 0x0d08  /*闲时温度连续小幅上升*/
#define Cc4FlowIdleCurLeak 0x0d09  /*流程闲时漏电流*/
#define Cc4FlowIdleVolRiseTtl 0x0d0a  /*流程闲时累加电压*/
#define Cc4AllCellTmprUpLmt 0x0d0b  /*全时通道温度上限*/
#define Cc4BusyChnTmprBigRise 0x0d0c  /*忙时通道温度突升*/
#define Cc4FlowCrashStopVolUpLmt 0x0d0d  /**/
#define Cc4FlowCrashStopVolLowLmt 0x0d0e  /**/
#define Cc4FlowIdleVolIntvlRiseUpLmt 0x0d0f  /**/
#define Cc4FlowCcdcVolIntvlFluctLowLmt 0x0d10  /**/
#define Cc4QuietVolDown 0x0d12  /*静置电压下降*/
#define Cc4CccVolDownAbnm 0x0d13  /**/
#define Cc4CccVolBigDown 0x0d14  /**/
#define Cc4CcdVolBigDown 0x0d15  /* 恒流放电电压突降*/
#define Cc4CcdVolRiseAbnm 0x0d16  /**/

/*优先级最高的是触发组合的通道异常码:CcMixBase+表达式id*/

/*---------------通道原因码，结束-----------------*/
#endif
#endif



