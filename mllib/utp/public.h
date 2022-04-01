#ifndef  _PUBLIC_H_
#define  _PUBLIC_H_

#include "mlos.h"




/*
CRC校验
*/
mlu16 crc16_check(mlu8 *pbuf, mlu32 size);


/*
ms延时
*/
void delay_rough_ms(mlu32 n);

/*
us延时
*/
void delay_rough_us(mlu32 n);

/*
 * 函数名:GetOSRunTime
 * 参数:无
 * 返回值:
 * 功能:获取系统运行时间，单位ms
 */
mlu32 GetOSRunTime(void);

/*
 * 函数名:GetOSRunTimeDiff
 * 参数:_T1Time:记录的T1时间，单位ms
 * 返回值:返回当前时间和上一次记录的时间的差值
 * 功能:获取T1和T2时间的差值，单位ms
 */
mlu32 GetOSRunTimeDiff(mlu32 _T1Time);

#endif
