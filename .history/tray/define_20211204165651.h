
#ifndef _DEFINE_H_
#define _DEFINE_H_

#include "basic.h"

#define Align16(amt) (((amt)+1) & 0xfffe)
#define Align8(amt) (((amt)+3) & 0xfffc)
#define AlignStr(len) (((len)+4) & 0xfffc)


#define ParamAmt  8
#define StepIdNull 255
#define MsgSmplAmtMax 20

/*todo,这个需要改为配置*/
#define TmprValidMax 1300
#define TmprValidMin 50

#define Maxer(a, b) ((b)>(a) ? (b) : (a)) /*a >= b, 则a*/
#define Miner(a, b) ((b)<(a) ? (b) : (a)) /*a <= b, 则a*/

#define AbsDifVal(a, b) ((s32)(a) > (s32)(b) ? (s32)(a)-(s32)(b) : (s32)(b)-(s32)(a))
#define AbsVal(a) ((s32)(a) > 0 ? (s32)(a) : 0-(s32)(a))

#define BitSet(val, bit) do {(val) |= 1<<(bit);}while(0)
#define BitIsSet(val, bit) ((val) & 1<<(bit))
#define BitVal(val, bit) ((val)>>(bit) & 1)
#define BitClr(val, bit) do {(val) &= ~(1<<(bit));}while(0)
#define BitEqualSet(dst, bit, src) \
do \
{ \
    if (src) BitSet(dst, bit); \
    else BitClr(dst, bit); \
}while(0)

#define MaxFixtTmprSmplAmt 64

#define MaxUpdTransSize 1024

/* 目前合法token: A~Za~z0~9._/-: */
#define IsTokenChar(c)  ((c)>'a'-1&&(c)<'z'+1 || (c)>'0'-1&&(c)<'9'+1 \
    || (c)>'A'-1&&(c)<'Z'+1 || '.'==(c) || '_'==(c) || '/'==(c) || '-'==(c) \
    || ':'==(c))
#define IsSpacePrefix(c) (' '==(c) || '\t'==(c))
#define IsSpaceSuffix(c) (' '==(c) || '\t'==(c) || '\r'==(c) || '\n'==(c))

#define LineLenMax 255   /*单行字符串长度,并扩展到一般性*/


#endif


