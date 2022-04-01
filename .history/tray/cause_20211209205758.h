

#ifndef _CAUSE_H_
#define _CAUSE_H_

#include "basic.h"

/*原因码分为命令响应码和通道原因码*/

/*---------------命令响应码，开始-----------------*/
/*0~255留给其它下位机,中位机定义的原因码从256开始*/
typedef u16 u16eRspCode;
#define RspOk 0x0000
#define RspErr 0x0100
#define RspRetry 0x0101  /*临时应答*/
#define RspTouch 0x0102  /*托盘未压合*/
#define RspAddr 0x0103  /*寻址错误*/
#define RspStatus 0x0104  /*状态不匹配*/
#define RspParam 0x0105  /*参数错误*/
#define RspNoRes 0x0106  /*资源不足*/
#define RspWarn 0x0107  /*存在告警*/
#define RspBusy 0x0108  /*等应答后再发命令*/
#define RspDisc 0x0109  /*级联不在线*/
#define RspExpr 0x010a  /*级联超时*/
#define RspStep 0x010b  /*工步未下发*/
#define RspProt 0x010c  /*保护未下发*/
#define RspPage 0x010d  /*升级传输协商失败*/
/*---------------响应码，结束-----------------*/


/*---------------采样通道原因码，开始-----------------*/
/*0~255留给电源柜下位机异常码,256~512留给电源柜下位机响应码*/
/*中位机定义的原因码从1024开始*/
/*中位机异常码划分优先级,优先级高的可以替换优先级低的*/
/*但是触发高级保护的通道的异常码不能被替换*/
typedef u16 u16eCauseCode;
#define CcNone 0x0000
#define Cclv0Base 0x0400   /*托盘(不涉组合)*/
#define Cclv1Base 0x0800  /*通道(不涉组合不涉全盘)*/
#define Cclv2Base 0x0a00  /*通道(不涉组合但涉全盘)*/
#define Cclv3Base 0x0c00  /*托盘级因子*/
#define Cclv4Base 0x0d00  /*单通道因子+组合保护波及*/
#define CcMixBase 0x0e00  /*组合*/
#define CcBaseGet(cc) (0xff00 & (cc))
#define CcChkModify(old, new) \
do \
{ \
    if ((old) < CcBaseGet(new)) \
    { \
        old = new; \
    } \
}while(0)

/*受(非组合)托盘保护波及或者普通托盘保护,优先级最低*/
#define Cc0TrayTrig 0x0400  /*(非组合非托盘级因子)托盘级保护波及*/
#define Cc0SeriesTrig 0x0401  /*串联波及保护*/
#define Cc0BusySlotTmprLowLmt 0x0402  /*忙时库温下限*/
#define Cc0TmprBoxOffline   0x0403
#define Cc0MkNpExpr   0x0404
#define Cc0StepNpOverLmt   0x0405
#define Cc0TrayGas   0x0406
#define Cc0TrayCo   0x0407  /*co告警停流程但不参与消防*/
#define Cc0TrayFan   0x0408
#define Cc0SlotTmprBad   0x0409
#define Cc0NdbdDisc   0x040a  /*针床数据无效--PLC断线*/

/*(非因子)通道保护,且不涉全盘,优先级高于前者*/
#define Cc1LowCfg  0x0800   /*,下位机规格不匹配*/
#define Cc1CellTmprBad 0x0801  /*温度无效--温度盒断线*/
#define Cc1Reverse 0x0803  /*反接保护*/
#define Cc1QuietVolDown 0x0804  /*静置电压下降*/
#define Cc1QuietCurLeak 0x0805  /**/
#define Cc1StepVolUpLmt 0x0806  /**/
#define Cc1StepVolLowLmt 0x0807  /**/
#define Cc1StepChnTmprBigRise 0x0808  /**/
#define Cc1CccSetTimeVolUpLmt 0x0809  /**/
#define Cc1CccSetTimeVolLowLmt 0x080a  /**/
#define Cc1CcCurOfst 0x080b  /**/
#define Cc1CccVolRiseRateUpLmt 0x080c  /**/
#define Cc1CccVolRiseRateLowLmt 0x080d  /**/
#define Cc1CcdVolBigDown 0x080e  /**/
#define Cc1StepCapUpLmt 0x080f  /**/
#define Cc1StepCapLowLmt 0x0810  /**/
#define Cc1CcVolIntvlFluctUpLmt 0x0811  /**/
#define Cc1CcVolIntvlFluctLowLmt 0x0812  /**/
#define Cc1CccVolRiseCtnu 0x0813  /**/
#define Cc1CccVolDownCtnu 0x0814  /**/
#define Cc1CcdVolRiseAbnm 0x0815  /**/
#define Cc1CvcVolOfst 0x0816  /**/
#define Cc1CvcCurRiseAbnm 0x0817  /**/
#define Cc1CvcCurBigRise 0x0818  /**/
#define Cc1QuietVolChkBack 0x0819  /*电压回检即电流电芯双压差*/

/*(非因子)通道保护,但涉全盘,优先级高于前者*/
#define Cc2IdleVolRiseTtl 0x0a00  /*闲时累加电压*/
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

/*因子托盘保护,优先级高于前者*/
#define Cc3AllSlotTmprUpLmt 0x0c00  /*全时库位温度上限*/
#define Cc3AllCellTmprUpLmt 0x0c01  /*全时通道温度上限*/
#define Cc3AllSmoke 0x0c02  /*全时烟感*/
#define Cc3AllNpUpLmt 0x0c03  /*全时气压上限*/
#define Cc3BusyNpBigRise 0x0c04  /*忙时气压突升*/

/*因子通道保护+组合保护波及,优先级仅次于触发组合的通道*/
#define Cc4TrayMixTrag 0x0d00 /*组合保护波及*/
#define Cc4IdleVolUpFluctBig 0x0d01  /*闲时电压大上波动*/
#define Cc4IdleVolUpFluctSml 0x0d02  /*闲时电压小上波动*/
#define Cc4IdleTmprUpFluctBig 0x0d03  /*闲时温度大上波动*/
#define Cc4IdleTmprUpFluctSml 0x0d04  /*闲时温度小上波动*/
#define Cc4IdleCurLeak 0x0d05  /*闲时漏电流*/
#define Cc4QuietVolFluct 0x0d06  /**/
#define Cc4BusyChnTmprBigRise 0x0d07  /*忙时通道温度突升*/
#define Cc4CccVolDownAbnm 0x0d08  /**/
#define Cc4CccVolBigDown 0x0d09  /**/
#define Cc4FlowCrashStopVolUpLmt 0x0d0a  /**/
#define Cc4FlowCrashStopVolLowLmt 0x0d0b  /**/
#define Cc4FlowIdleVolIntvlRiseUpLmt 0x0d0c  /**/
#define Cc4FlowCcdcVolIntvlFluctLowLmt 0x0d0d  /**/

/*优先级最高的是触发组合的通道异常码:CcMixBase+表达式id*/

/*---------------通道原因码，结束-----------------*/

#endif



