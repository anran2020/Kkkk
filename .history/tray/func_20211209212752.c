//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"
#include "func.h"
#include "log.h"
#include "timer.h"

#ifdef DebugVersion
#else
#include "mlos_malloc.h"
#endif

/*
多项式: 不同标准对应多项式可能有差异，modbus为:x16+x15+x2+x0
poly: 多项式对应bit为1得到的hex，modbus为:0x8005
crc初始值：不同标准的初始值可能不同,modbus为0xffff
bit序：不同标准的要求可能不同，modbus要求为输入参数参与运算时低bit在前
输出：不同标准的要求可能不同(是否按位取反),modbus要求不取反
算法：各种标准一致，如下
1. 将crc赋值为初始值
2. 参数第一(随循环递增)字节异或到crc的高字节(即该字节左移8位后与crc异或)。
3. 判断crc最高位，0则crc左移一位，1则crc左移一位后再与poly异或。重复8次。
4. 重复2和3，直至处理完所有输入参数。
5. 根据标准，若需要则将crc按位取反(与0xffff异或)，结束
*/

#if 1
#if 0
/*倒换bit序，不超过32bit*/
void bitInvert(u32 *dst, u32 src, u8 bitLen)
{
    u8 cnt;

    for (cnt=0; cnt<bitLen; cnt++)
    {
        if (src & 1<<cnt)
        {
            *dst |= 1 << bitLen-1-cnt;
        }
        else
        {
            *dst &= ~(1 << bitLen-1-cnt);
        }
    }

    return;
}
/*下面所列三种算法，结果相同*/
/*以下是严格按照协议的版本*/
u16 crc16Modbus(u8 *buf, u16 len)
{
    u16 crc = 0xffff;  /*modbus要求为0xffff*/
    u16 poly = 0x8005;  /*x16+x15+x2+x0,对应bit置1后取低16bit*/
    u8 src;   /*modbus要求低bit在前,src用于换序*/
    u8 cnt;

    while(len--)
    {
        bitInvert((u32 *)&src, (u32)*buf++, 8); /*将参数调整为低bit在前*/
        crc ^= src<<8;  /*将src异或到crc的高字节*/
        for (cnt=0; cnt<8; cnt++)
        {
            if (crc & 0x8000)  /*判断crc最高位，为1则*/
            {
                crc <<= 1;  /*左移一位*/
                crc ^= poly;  /*再与poly异或*/
            }
            else  /*否则只左移一位*/
            {
                crc <<= 1;
            }
        }
    }
    bitInvert((u32 *)&crc, (u32)crc, 16);  /*还原为高bit在前*/
    return crc;
}
#else
/*下面是优化效率版本，逻辑也更简洁，不用调整bit序，并由此：
poly用倒序，异或到crc的低字节，判断最低位，将左移改为右移*/
u16 crc16Modbus(u8 *buf, u16 len)
{
    u16 crc = 0xffff;  /*modbus要求为0xffff*/
    u16 poly = 0xa001;  /*优化后，用0xa001,即0x8005的倒序*/
    u8 cnt;

    while(len--)
    {
        crc ^= *buf++;  /*优化后，将src异或到crc的低字节*/
        for (cnt=0; cnt<8; cnt++)
        {
            if (crc & 0x0001)  /*优化后判断crc最低位，为1则*/
            {
                crc >>= 1;  /*优化后，右移一位*/
                crc ^= poly;  /*再与poly异或*/
            }
            else  /*否则 优化后只右移一位*/
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}
#endif
#else
/*下面是用内存为代价继续优化效率的版本，不用嵌套循环*/
u16 crc16Modbus(u8 *buf, u16 len)
{
    static u8 crcLowTab[] = {
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
        0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
        0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
        0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
        0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
        0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
        0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
        0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
        0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
        0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
        0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
        0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
        0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
        0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
        0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
        0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
        0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
        0x41, 0x81, 0x80, 0x40
    };
    static u8 crcHighTab[] = {
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40
    };
    u8 *sentry;
    u8 crcLow;
    u8 crcHigh;
    u8 index;

    for (sentry=buf+len,crcHigh=crcLow=0xff; buf<sentry; buf++)
    {
        index = crcLow ^ *buf;
        crcLow = crcHigh ^ crcHighTab[index];
        crcHigh = crcLowTab[index];
    }

    return crcHigh<<8 | crcLow;
}
#endif

void outHex(u8 *ptr, u16 len)
{
	u16 line, left, pos;
	s8 str[128];
	u8 *sentry;

	for (line=len>>4; line; line--)
	{
		for (sentry=ptr+8,pos=0; ptr<sentry; ptr++,pos+=3)
		{
			sprintf(&str[pos], "%02x ", *ptr);
		}
		sprintf(&str[pos++], " ");
		for (sentry=ptr+8; ptr<sentry; ptr++,pos+=3)
		{
			sprintf(&str[pos], "%02x ", *ptr);
		}
		sprintf(&str[pos-1], "\r\n");
        Show("%s", str);
	}

	left = len & 0x0f;
	if (0 == left)
	{
		return;
	}

	pos = 0;
	if (left > 8)
	{
		for (sentry=ptr+8; ptr<sentry; ptr++,pos+=3)
		{
			sprintf(&str[pos], "%02x ", *ptr);
		}
		sprintf(&str[pos++], " ");
		left -= 8;
	}

	for (sentry=ptr+left; ptr<sentry; ptr++,pos+=3)
	{
		sprintf(&str[pos], "%02x ", *ptr);
	}
	
	sprintf(&str[pos-1], "\r\n");
    Show("%s", str);
	return;
}

/*只做以2对齐的4字节整数拷贝,其它勿用.box只做了2对齐,针对一下*/
/*len--字节长度*/
void mem2Copy(u16 *dst, u16 *src, u16 len)
{
    u16 *sentry;

    for (sentry=src+(len>>1); src<sentry; *dst++=*src++);
    return;
}

void *sysMemAlloc(u32 size)
{
    void *ptr;

    if (0 == size)
    {
        return NULL;
    }

#ifdef DebugVersion
    ptr = malloc(size);
#else
    ptr = mlos_malloc(e_mem_exsram, size);
#endif
    if (NULL != ptr)
    {
        memset(ptr, 0, size);
    }
    return ptr;
}

ListD gMemFamily;
Ret memMgrInit(ListD *familyList, MemDesc *desc, u16 num)
{
    u32 size;
    u8 *ptr;
    u8 *pos;
    MemFamily *family;
    MemBlk *blk;
    MemDesc *sentry;

    for (size=sizeof(MemFamily)*num,sentry=desc+num; desc<sentry; desc++)
    {
        size += (desc->size+sizeof(MemBlk)) * desc->count;
    }

    if (NULL == (ptr = (u8 *)sysMemAlloc(size)))
    {
        return Nok;
    }

    pos = ptr + sizeof(MemFamily)*num;
    for (family=(MemFamily *)ptr,desc=sentry-num; desc<sentry; desc++,family++)
    {
        family->size = desc->size;
        family->count = desc->count;
        QueueInit(&family->memList);
        ChainInitD(&family->chain);
        ChainInsertD(familyList, &family->chain);

        size = sizeof(MemBlk) + family->size;
        for (ptr=pos+family->count*size; pos<ptr; pos+=size)
        {
            blk = (MemBlk *)(pos);
            blk->family = &family->memList;
            ChainInitD(&blk->chain);
            ChainInsertD(&family->memList.list, &blk->chain);
        }
    }

    return Ok;
}

void *memAlloc(u32 size)
{
    ChainD *familyChn;
    ChainD *blkChn;
    MemFamily *family;
    MemBlk *blk;

    ListForEach(&gMemFamily, familyChn)
    {
        family = Container(MemFamily, chain, familyChn);
        if (family->size < size)
        {
            continue;
        }

        QueueGet(&family->memList, blkChn);
        if (blkChn != &family->memList.list)
        {
            blk = Container(MemBlk, chain, blkChn);
            return (void *)blk->buf;
        }
    }

    return NULL;
}

void memFree(void *ptr)
{
    MemBlk *blk;

    blk = Container(MemBlk, buf, ptr);
    QueuePut(blk->family, &blk->chain);

    return;
}

/*对字符串进行掐头去尾裁剪,返回指向裁剪后第一个有效字符的指针*/
/*str:源字符串*/
/*len:为零则取源串实际长度,否则以len为准,忽略源串实际长度*/
/*dst:若空,则原串上裁剪,否则原串不动,结果放入dst*/
/*return:指向裁剪过的串的起始*/
s8 *strStrip(s8 *str, u16 len, s8 *dst)
{
    s8 *tail;
    s8 *head;
    s8 *pos;

    if (0 == len)
    {
        len = strlen(str);
    }

    for (head=str; IsSpacePrefix(*head); head++);
    for (tail=str+len-1; tail>head && IsSpaceSuffix(*tail); tail--);
    tail++;
    if (NULL == dst)
    {
        if (head == str)
        {
            *tail = 0;
            return str;
        }
        dst = str;
    }

    for (pos=dst; head<tail; *pos++=*head++);
    *pos = 0;
    return dst;
}

void strTokRel(void *handle)
{
    StrTok *family;
    ChainD *chain;
    ChainD *temp;

    if (NULL == handle)
    {
        return;
    }

    family = (StrTok *)handle;
    ListForEachSafe(&family->list, chain, temp)
    {
        ChainDeleteD(chain);
        memFree(Container(StrTokUnit, chain, chain));
    }

    memFree(family);
    return;
}

/*分割str，且不改变str。返回操作句柄*/
/*ch--分割字符*/
void *strTokSetup(s8 *str, s8 ch)
{
    StrTok *family;
    StrTokUnit *unit;
    s8 *sentry;
    s8 *dst;

    family = (StrTok *)memAlloc(sizeof(StrTok));
    if (NULL == family)
    {
        return NULL;
    }
    ListInitD(&family->list);

    for (unit=NULL; 0!=*str; str++)
    {
        if (ch == *str)  /*分割符*/
        {
            if (NULL != unit)  /*得到一个完整token*/
            {
                *dst = 0; /*追加结束符*/
                unit = NULL; /*再次得到token字符时申请下一个unit*/
            }
        }
        else  /*非分隔符，也即有效token字符*/
        {
            if (NULL == unit) /*token首字符,先申请unit*/ 
            {
                unit = (StrTokUnit *)memAlloc(sizeof(StrTokUnit));
                if (NULL == unit)
                {
                    strTokRel(family);
                    return NULL;
                }

                dst = unit->buf;
                sentry = &unit->buf[LineLenMax];
                ChainInsertD(&family->list, &unit->chain);
            }

            if (dst < sentry)
            {
                *dst++ = *str;
            }
            else
            {
                strTokRel(family);
                return NULL;
            }
        }
    }

    if (ListIsEmpty(&family->list)) /*是否存在token*/
    {
        memFree(family);
        return NULL;
    }

    *dst = 0; /*最后一个token的结束符*/
    family->crnt = family->list.next;
    return family;
}

/**/
s8 *strTokGet(void *handle)
{
    StrTok *family;
    StrTokUnit *unit;

    if (NULL == handle)
    {
        return NULL;
    }

    family = (StrTok *)handle;
    if (family->crnt == &family->list)
    {
        return NULL;
    }

    unit = Container(StrTokUnit, chain, family->crnt);
    family->crnt = family->crnt->next;
    return unit->buf;
}

/*字符串转数字,不检查数值大小和宽度*/
Ret strToHex(s8 *str, u32 *hex)
{
    u32 val;

    if ('0'==*str && (('x'==str[1])||('X'==str[1])))
    {
        for (val=0,str+=2; ; str++)
        {
            if (*str>='0' && *str<='9')
            {
                val = (val<<4) + *str - '0';
            }
            else if (*str>='a' && *str<='f')
            {
                val = (val<<4) + *str - 'a';
            }
            else if (*str>='A' && *str<='F')
            {
                val = (val<<4) + *str - 'A';
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        for (val=0; *str>='0'&&*str<='9'; val=val*10+*str++-'0');
    }

    if (0 != *str)
    {
        return Nok;
    }

    *hex = val;
    return Ok;
}

/*数组解析*/
/*src:输入串,例如"1-5,8,9,11-13",严格由小至大*/
/*arrAmt:dst可存储元素个数*/
/*dst:输出数组,元素宽度u16*/
/*itemAmt:输出元素个数*/
Ret arrayParse(s8 *src, u8 *dst, u8 arrAmt, u8 *itemAmt)
{
    void *handle;
    void *handleSub;
    s8 *str;
    s8 *part;
    u32 idx;
    u32 min;
    u32 max;
    Ret ret;

    *itemAmt = idx = 0;
    if (NULL == (handle=strTokSetup(src, ',')))
    {
        return Ok;
    }

    ret = Nok;
    while (NULL != (part=strTokGet(handle)))
    {
        strStrip(part, 0, NULL);
        if (0 == part[0])
        {
            continue;
        }

        if (NULL == (handleSub=strTokSetup(part, '-')))
        {
            goto endHandler;
        }

        str = strTokGet(handleSub);
        strStrip(str, 0, NULL);
        if (Ok != strToHex(str, &min))
        {
            goto endHandler;
        }

        if (idx < arrAmt)
        {
            dst[idx++] = min;
        }
        else
        {
            goto endHandler;
        }

        str = strTokGet(handleSub);
        if (NULL != str)
        {
            strStrip(str, 0, NULL);
            if (Ok!=strToHex(str, &max) || max<=min)
            {
                goto endHandler;
            }

            for (; min<max; min++,idx++)
            {

                if (idx < arrAmt)
                {
                    dst[idx] = dst[idx-1] + 1;
                }
                else
                {
                    goto endHandler;
                }
            }

            if (NULL != strTokGet(handleSub))
            {
                goto endHandler;
            }
        }

        strTokRel(handleSub);
        handleSub = NULL;
    }

    if (idx > 1)
    {
        for (min=1; min<idx; min++)
        {
            if (dst[min] <= dst[min-1])
            {
                goto endHandler;
            }
        }
    }

    ret = Ok;
    *itemAmt = idx;

endHandler:
    if (NULL != handleSub)
    {
        strTokRel(handleSub);
    }
    strTokRel(handle);
    return ret;
}

/*栈限定：目前仅支持定长1字节的出入栈数据*/
/*以后视需要，可将出入栈数据扩展为可变长*/
static Stack *stackSetup(u16 size)
{
    Stack *stack;

    stack = (Stack *)memAlloc(sizeof(Stack));
    if (NULL != stack)
    {
        stack->buf = (u8 *)memAlloc(size);
        if (NULL == stack->buf)
        {
            memFree(stack);
            stack = NULL;
        }
        else
        {
            stack->top = 0;
            stack->size = size;
        }
    }

    return stack;
}

static void stackRel(Stack *stack)
{
    memFree(stack->buf);
    memFree(stack);
    return;
}

static Ret stackPush(Stack *stack, u8 element)
{
    if (stack->top < stack->size)
    {
        stack->buf[stack->top++] = element;
        return Ok;
    }

    return Nok;
}

/*空栈假,非空真且弹出栈顶*/
static b8 stackPop(Stack *stack, u8 *element)
{
    if (stack->top > 0)
    {
        *element = stack->buf[--stack->top];
        return True;
    }
    return False;
}


/*目前参与运算的数字均为保护因子id，可以看见的时间内均小于128*/
/*所以表达式转换为基于数字小于128的有限完整性*/
/*后缀表达式中,运算符加基址128,数字和运算符均1字节*/
/*infix: 输入中缀表达式数组*/
/*infixCri:中缀表达式结束*/
/*numCri:表达式中数字临界非法上限,用于保护时即因子上限*/
/*suffix: 输出后缀表达式数组,并非字符串*/
/*suffixCri:输入后缀表达式长度限制*/
/*suffixLen:输出后缀表达式的长度*/
Ret expInfix2Suffix(u8 *infix, u8 *infixCri, u8 numCri, s8 *suffix, s8 *suffixCri, u8 *suffixLen)
{
#define CharIsNum(ch) ((ch)>='0' && (ch)<='9')
    Stack *stack;
    s8 *dst;
    u8 num;
    u8 opr;
    u8 ret;

    if (NULL == (stack=stackSetup(256)))
    {
        return Nok;
    }

    for (dst=suffix,ret=Nok; infix<infixCri; infix++)
    {
        if (CharIsNum(*infix))
        {
            num = *infix - '0';
            if (CharIsNum(infix[1]))
            {
                num = num*10 + infix[1] - '0';
                if (CharIsNum(infix[2]) || num>=numCri)
                {
                    goto endHandler;
                }

                infix++;
            }

            if (dst < suffixCri)
            {
                *dst++ = num;
            }
            else
            {
                goto endHandler;
            }
        }
        else if ('(' == *infix)
        {
            if (Ok != stackPush(stack, *infix))
            {
                goto endHandler;
            }
        }
        else if (')' == *infix)
        {
            opr = 0;
            while (stackPop(stack, &opr))
            {
                if ('(' == opr)
                {
                    break;
                }
                else
                {
                    if (dst < suffixCri)
                    {
                        *dst++ = opr + 128;
                    }
                    else
                    {
                        goto endHandler;
                    }
                }
            }

            if ('(' != opr)
            {
                goto endHandler;
            }
        }
        else if ('|' == *infix)
        {
            while (stackPop(stack, &opr))
            {
                if ('(' == opr)
                {
                    if (Ok != stackPush(stack, opr))
                    {
                        goto endHandler;
                    }
                    break;
                }
                else
                {
                    if (dst < suffixCri)
                    {
                        *dst++ = opr + 128;
                    }
                    else
                    {
                        goto endHandler;
                    }
                }
            }

            if (Ok != stackPush(stack, *infix))
            {
                goto endHandler;
            }
        }
        else if ('&' == *infix)
        {
            while (stackPop(stack, &opr))
            {
                if ('(' == opr || '|' == opr)
                {
                    if (Ok != stackPush(stack, opr))
                    {
                        goto endHandler;
                    }
                    break;
                }
                else
                {
                    if (dst < suffixCri)
                    {
                        *dst++ = opr + 128;
                    }
                    else
                    {
                        goto endHandler;
                    }
                }
            }

            if (Ok != stackPush(stack, *infix))
            {
                goto endHandler;
            }
        }
        else
        {
            goto endHandler;
        }
    }

    while (stackPop(stack, &opr))
    {
        if ('&' == opr || '|' == opr)
        {
            if (dst < suffixCri)
            {
                *dst++ = opr + 128;
            }
            else
            {
                goto endHandler;
            }
        }
        else
        {
            goto endHandler;
        }
    }

    if (dst != suffix)  /*条件表达式不允许空*/
    {
        *suffixLen = dst - suffix;
        ret = Ok;
    }

endHandler:
    stackRel(stack);
    return ret;

#undef CharIsNum
}

/*根据数组计算后缀表达式结果*/
/*suffix:表达式,其中的操作数是数组下标*/
/*elemVal:输入数组*/
Ret expSuffixCalc(u8 *suffix, u8 suffixLen, u32 elemVal, u8 *result)
{
    Stack *stack;
    u8 *suffixCri;
    u8 left;
    u8 right;
    u8 opr;
    u8 ret;

    if (NULL == (stack=stackSetup(256)))
    {
        return Nok;
    }

    for (ret=Nok,suffixCri=suffix+suffixLen; suffix<suffixCri; suffix++)
    {

        if (*suffix > 128)
        {
            if (!stackPop(stack, &right) || !stackPop(stack, &left))
            {
                goto endHandler;
            }

            opr = *suffix - 128;
            if ('&' == opr)
            {
                left = left && right;
            }
            else
            {
                left = left || right;
            }

            if (Ok != stackPush(stack, left))
            {
                goto endHandler;
            }
        }
        else
        {
            if (Ok != stackPush(stack, BitVal(elemVal, *suffix)))
            {
                goto endHandler;
            }
        }
    }

    if (!stackPop(stack, result) || stackPop(stack, &left))
    {
        goto endHandler;
    }

    ret = Ok;
endHandler:
    stackRel(stack);
    return ret;
}

Ret funcInit(MemDesc *desc, u16 descNum)
{
    ChainInitD(&gMemFamily);
    if (NULL != desc)
    {
        if (Ok != memMgrInit(&gMemFamily, desc, descNum))
        {
            return Nok;
        }
    }

    return Ok;
}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译
