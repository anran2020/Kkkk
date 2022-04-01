
#ifndef __FUNC_H__
#define __FUNC_H__
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------

#include <stdio.h>

#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"

typedef struct memFamilyTag
{
    ChainD chain;
    Queue memList;
    u16 size;
    u16 count;
}MemFamily;

typedef struct memBlockTag
{
    ChainD chain;
    Queue *family;
    u8 buf[0];
}MemBlk;

typedef struct memDescTag
{
    u16 size;
    u16 count;
}MemDesc;

typedef struct
{
    ChainD chain;
    s8 buf[AlignStr(LineLenMax)];
}StrTokUnit;

typedef struct
{
    ListD list;
    ChainD *crnt;
}StrTok;

typedef struct
{
    u8 *buf;
    u16 top;
    u16 size;
}Stack;


#ifdef __cplusplus
extern "C"  {
#endif

extern u16 crc16Modbus(u8 *buf, u16 len);
extern void outHex(u8 *ptr, u16 len);
extern void *sysMemAlloc(u32);
extern Ret funcInit(MemDesc *desc, u16 descNum);
extern void *memAlloc(u32 size);
extern void memFree(void *ptr);
extern void mem2Copy(u16 *dst, u16 *src, u16 amt);
extern s8 *strStrip(s8 *str, u16 len, s8 *dst);
extern Ret arrayParse(s8 *src, u8 *dst, u8 arrAmt, u8 *itemAmt);
extern Ret expInfix2Suffix(u8 *infix, u8 *infixCri, u8 numCri, s8 *suffix, s8 *suffixCri, u8 *suffixLen);
extern Ret expSuffixCalc(u8 *suffix, u8 suffixLen, u32 elemVal, u8 *result);
extern void *strTokSetup(s8 *str, s8 ch);
extern s8 *strTokGet(void *handle);
extern void strTokRel(void *handle);
extern Ret strToHex(s8 *str, u32 *hex);

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译
#endif

