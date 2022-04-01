
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "basic.h"
#include "log.h"
#include "func.h"
#include "entry.h"
#include "channel.h"

#ifdef DebugVersion
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/timeb.h>
#else
#include "host.h"
#include "mlos_log.h"
#endif


u8eTrcLvl gLogLvl = TrcLvlTrc;
s8 *gLogBuf;

void sendDbg(s8 *buf, u16 len)
{
    buf[len] = 0;
#ifdef DebugVersion
    printf("%s", buf);
#elif defined(DbgOnUpper)
    sendUpMsg(buf-sizeof(UpMsgHead), AlignStr(len), UpCmdId(UpMsgGrpDbg, DbgMsgLog));
#else
    //log_print(LOG_LVL_INFO, "%s", buf);  /*暂时不给log压力*/
#endif

    return;
}

/*流程跟踪日志*/
void logTrc(s8 *tag, s8 *fmt, ...)
{
    va_list args;
    s8 *buf;
    s8 *pld;
    int pldLen;

#ifdef DbgOnUpper
    buf = gLogBuf + sizeof(UpMsgHead);
#else
    buf = gLogBuf;
#endif

#ifdef DebugVersion
    GetLogTimeStr(buf);
#else
    sprintf(buf, "%02d-%02d %02d:%02d:%02d.%03d ", 0, 0, 0, 0, 0, 0);
#endif
    sprintf(buf+TrcTimeLen, "%-*.*s:", TrcTagLen-1, TrcTagLen-1, tag);
    pld = buf + TrcTimeLen+TrcTagLen;
    va_start(args, fmt);
    pldLen = vsnprintf(pld, TrcPldLen, fmt, args);
    va_end(args);
    if (pldLen <= 0)
    {
        return;
    }
    else if (pldLen >= TrcPldLen)
    {
        pld[TrcPldLen-1] = '\n';
        pldLen = TrcPldLen;
    }

    sendDbg(buf, (u16)pldLen);
    return;
}

/*调试命令结果输出*/
void logDbg(u8eTrcLvl trcLvl, s8 *fmt, ...)
{
    va_list args;
    s8 *buf;
    int pldLen;

#ifdef DbgOnUpper
    buf = gLogBuf + sizeof(UpMsgHead);
#else
    buf = gLogBuf;
#endif
    va_start(args, fmt);
    pldLen = vsnprintf(buf, TrcPldLen, fmt, args);
    va_end(args);
    if (pldLen <= 0)
    {
        return;
    }
    else if (pldLen >= TrcPldLen)
    {
        buf[TrcPldLen-1] = '\n';
        pldLen = TrcPldLen;
    }

    sendDbg(buf, (u16)pldLen);
    return;
}

void logInit()
{
    gLogLvl = TrcLvlTrc;
    gLogBuf = sysMemAlloc(2048);
}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译



