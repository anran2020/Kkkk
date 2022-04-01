
#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include "basic.h"
#if TRAY_ENABLE
#include "define.h"
#include "enum.h"

/*trace输出分为流程和命令响应两种*/
typedef u8 u8eTrcLvl;
#define TrcLvlFlow  0x00  /*栈流程*/
#define TrcLvlShow  0x01  /*一般调试命令输出*/
#define TrcLvlDbg   0x02  /*较详细流程*/
#define TrcLvlTrc   0x03  /*缺省等级,一般性流程*/
#define TrcLvlCmd   0x04  /*重要调试命令输出*/
#define TrcLvlWarn  0x05  /*不影响业务的异常流程*/
#define TrcLvlErr   0x06  /*可能影响业务的异常流程*/
#define TrcLvlCri   0x07  /*临界非法值*/

#define TrcTimeLen 19
#define TrcTagLen 6
#define TrcPldLen 480
#define TrcLenMax (TrcTagLen+TrcTimeLen+TrcPldLen)
#define TrcBufSize (AlignStr(TrcLenMax))

#ifdef DebugVersion
#define GetLogTimeStr(mBuf) \
do { \
    struct tm *tm; \
    struct timeb tp; \
\
    ftime(&tp); \
    tm = localtime(&tp.time); \
    sprintf(mBuf, "%02d-%02d %02d:%02d:%02d.%03d ", tm->tm_mon+1, \
        tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tp.millitm); \
} while(0)
#endif

#define Trace(lvl, tag, fmt, ...) \
do { \
    if (lvl >= gLogLvl) \
    { \
        logTrc(tag, "[%s] "fmt, __FUNCTION__, ##__VA_ARGS__); \
    } \
} while(0)

/*简单调试命令的输出信息，不写日志*/
#define Show(fmt, ...) logDbg(TrcLvlShow, fmt, ##__VA_ARGS__)

/*相对重要调试命令的输出信息，可能写日志，取决于当前日志等级*/
#define Cmd(fmt, ...) logDbg(TrcLvlCmd, fmt, ##__VA_ARGS__)

#define Flow(fmt, ...) Trace(TrcLvlFlow, "Flow", fmt, ##__VA_ARGS__)
#define Dbg(fmt, ...) Trace(TrcLvlDbg, "Debug", fmt, ##__VA_ARGS__)
#define Trc(fmt, ...) Trace(TrcLvlTrc, "Trace", fmt, ##__VA_ARGS__)
#define Warn(fmt, ...) Trace(TrcLvlWarn, "Warn", fmt, ##__VA_ARGS__)
#define Err(fmt, ...) Trace(TrcLvlErr, "Error", fmt, ##__VA_ARGS__)


extern u8eTrcLvl gLogLvl;

#ifdef __cplusplus
extern "C"  {
#endif

void logTrc(s8 *tag, s8 *fmt, ...);
void logDbg(u8eTrcLvl trcLvl, s8 *fmt, ...);
void logInit(void);

#ifdef __cplusplus
}
#endif

#endif
#endif

