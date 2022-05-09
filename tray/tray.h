

#ifndef __TRAY_H__
#define __TRAY_H__


#include "basic.h"
#if TRAY_ENABLE
#include "define.h"
#include "enum.h"
#include "type.h"
#include "cause.h"
#include "func.h"
#include "timer.h"
#include "log.h"
#include "entry.h"
#include "flow.h"



#ifdef __cplusplus
extern "C"  {
#endif

/*todo,,忘了咋加文件,以后这部分单独建立文件*/
/*目前来看,磁盘压力不小*/
/*减少头信息的写次数,采取记录和遍历结合的折中方案*/
/*折中后,暂定60秒写一次头信息,假设带4盘,那么平均15秒更新一次*/
typedef struct
{
    u32 pageBase;  /*托盘首页序号*/
    u32 timeStamp;
    u16 trayCellAmt;
    u16 smplSize;  /*单条采样字节数,不含额外信息*/
    u16 smplAmt;  /*存储条数*/
    u16 smplSeq;  /*当前时间戳的首条采样存储序号*/
    u32 writeCnt;  /*存储计数,磁盘不关心,运行维护*/
    b8 diskLooped;  /*磁盘写满*/
    u8 smplPageAmt;  /*单采样占据页数*/
    u8 trayBoxAmt;
    u8 rsvd[9];
}SmplDiskTrayHead;

/*前12字节字段格式固定*/
typedef struct
{
    u32 chkSum;  /*普通异或校验,自headLen开始校验*/
    u16 headLen;  /*自headLen(含)到结束的长度*/
    u8 version;
    u8 abTagCnt;  /*ab区交替写,用于识别新写的*/
    u32 timeStampBase;  /*秒*/
    u8 trayAmt;
    u8 smplAmtPerTs;
    u8 rsvd[18];
    SmplDiskTrayHead trayHead[0];
}SmplDiskHead;

#define SmplHeadDiskPage 131073  /*采样的头信息占据sd卡的页地址,ab区各1页共占2页*/
#define SmplDiskSizeMax (64*1024*1024)  /*采样信息(含头信息两页)的最大尺寸64M*/
#define DiskPageSize 512  /*每页大小,擦除的最小单位*/
#define PageIdToAddr(page) ((page) << 9)  /*page * 512*/
#define HeadLenWoChkSum(amt) (sizeof(SmplDiskHead)+sizeof(SmplDiskTrayHead)*(amt)-sizeof(u32))
#define SmplAmtPerTs 64  /*共用时间戳的采样条数*/

typedef struct
{
    u32 timeStamp;
    u32 chkSum;  /*暂时不用校验采样,预留*/
}SmplExtraInfo;

typedef struct
{
    SmplDiskHead *smplDiskHead;  /*存储*/
    u8 *buffer;   /*读写缓存*/
    u32 timeStampBase;
}SmplDiskMgr;

extern Ret smplDiskRead(u8 trayIdx, u8 smplAmt, u8 *smplData, u16 smplSeq);
extern Ret smplDiskWrite(u8 trayIdx, u8 smplAmt, u8 *smplData, u16 smplSeq);
extern Ret smplDiskInit(SmplDiskHead *headSet);
/*以上部分单独建立文件*/

extern void trayNpRatioSet(u8 trayIdx, u8eNpOprCode npOpr, u8 swVal, s16 npVal);
extern void trayNpSwProtDelay(Timer *timer);
extern void trayNpReachChk(Tray *tray);
extern void trayNpChnFree(Tray *tray, Channel *chn);
extern Ret trayNpChnAlloc(Tray *tray, Channel *chn);
extern Ret trayNpChnReAlloc(Tray *tray, Channel *chn);
extern void trayNpSwRstDelay(Timer *timer);
extern void traySmplSetAux(Tray *tray);
extern void trayStopByDisk(Tray *tray, u16eCauseCode causeCode);
extern void trayNpReset(u8 trayIdx);
extern void trayIgnoreNdbd(void);
extern void trayStopByProt(Tray *tray, u16eCauseCode causeCode);
extern void trayNdbdBreakChk(Timer *timer);
extern void trayMntnKeep(Tray *tray);
extern void trayMntnEnter(Tray *tray, u8eBoxWorkMode mode);
extern void trayMntnEnd(Tray *tray, u8eBoxWorkMode mode);
extern void trayMntnExpr(Timer *timer);
extern void trayNdbdBreak(u8 trayIdx);
extern void trayNdbdCtrl(u8 trayIdx, u8eNdbdCtrlType type, s16 oprVal);
extern b8 trayNdbdBeOnline(u8 trayIdx);

#ifdef __cplusplus
}
#endif

#endif
#endif
