/**
 ******************************************************************************
 * 文件:    
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:  
 *          
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                       2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "u_led.h"
#include"common_data.h"
#include "hal_data.h"
#include "mlos.h"
#include "ctp.h"
//----------------------------------------------------------------------------
//define 
//----------------------------------------------------------------------------
// led 操作 延时

//P403	RGB_RED		RGB红色灯控制引脚
//P404	RGB_GREEN	RGB绿色灯控制引脚
//P405	RGB_BLUE	RGB蓝色灯控制引脚



//----------------------------------------------------------------------------
// led 抽象数据类型定义
//----------------------------------------------------------------------------
typedef struct LEDDevice
{
    //操作
    mlu8 changeColorFlag;
    mlu8 twinkleSignal;

    mlu8 redTwinkle;
    mlu8 greenTwinkle;
    mlu8 orangeTwinkle;

    mlu8 changeColorRefreshTimes;

    //led 功能寄存器reg， 每个bit 代表一个功能
    mlu8 redBitReg;
    mlu8 greenBitReg;
//	u8 twinkleBitReg;

//led 灯闪烁定时器

	mlu32 twinkleFreq;
    LTimerPtr ptwinkletimer;

    // 对应通道的状态记录，不同状态，led 不同点亮
//	u8 showChnlSta[LED_DEFAULT_CNT];

    Taskptr pMytask;

} LED;

//----------------------------------------------------------------------------
//global varibale 
//----------------------------------------------------------------------------

LED led;






//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_twinkle_disable(mlu8 ledx)
{
    if (led.twinkleSignal & ((0x01 << ledx)))
    {
        led.twinkleSignal &= (~((0x01) << ledx));
        led.redTwinkle &= (~(0x01 << ledx));
        led.greenTwinkle &= (~(0x01 << ledx));
        led.orangeTwinkle &= (~(0x01 << ledx));
    }
}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_red_switch(SwitchState swsta)
{
	 R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_03, swsta);	
}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_green_switch(SwitchState swsta)
{
	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_04, swsta);
}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_blue_switch(SwitchState swsta)
{
	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_05, BSP_IO_LEVEL_LOW);	
}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_twinkle_red(mlu8 ledx)
{
    led.redBitReg |= (0x01 << ledx);
    led.greenBitReg &= (~(0x01 << ledx));

    led.twinkleSignal |= (0x01 << ledx);
    led.redTwinkle |= (0x01 << ledx);
    led.greenTwinkle &= (~(0x01 << ledx));
    led.orangeTwinkle &= (~(0x01 << ledx));
}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_twinkle_green(mlu8 ledx)
{
    led.greenBitReg |= (0x01 << ledx);
    led.redBitReg &= (~(0x01 << ledx));

    led.twinkleSignal |= (0x01 << ledx);
    led.redTwinkle &= (~(0x01 << ledx));
    led.greenTwinkle |= (0x01 << ledx);
    led.orangeTwinkle &= (~(0x01 << ledx));
}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_twinkle_orange(mlu8 ledx)
{
    led.greenBitReg |= (0x01 << ledx);
    led.redBitReg |= (0x01 << ledx);

    led.twinkleSignal |= (0x01 << ledx);
    led.redTwinkle &= (~(0x01 << ledx));
    led.greenTwinkle &= (~(0x01 << ledx));
    led.orangeTwinkle |= (0x01 << ledx);

}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_off(void)
{
    led.redBitReg = 0;
    led.greenBitReg = 0;
    led.twinkleSignal = 0;
    led.redTwinkle = 0;
    led.greenTwinkle = 0;
    led.orangeTwinkle = 0;
    led.changeColorFlag = _set;
    led.changeColorRefreshTimes = 0;
}



//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
Taskstate led_run(void *args)
{
    task_declaration(); //任务声明
	mlu8 i;

    task_begin(led.pMytask); //任务开始
    
    //测试
	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_03, BSP_IO_LEVEL_HIGH);
	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_04, BSP_IO_LEVEL_LOW);
	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_05, BSP_IO_LEVEL_LOW);
	task_ms_delay(led.pMytask, led.twinkleFreq, TASK_STA_NONBLOCKING_YIELD);

    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_03, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_04, BSP_IO_LEVEL_HIGH);
   	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_05, BSP_IO_LEVEL_LOW);
    task_ms_delay(led.pMytask, led.twinkleFreq, TASK_STA_NONBLOCKING_YIELD);

	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_03, BSP_IO_LEVEL_LOW);
	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_04, BSP_IO_LEVEL_LOW);
	R_IOPORT_PinWrite (&g_ioport_ctrl, BSP_IO_PORT_04_PIN_05, BSP_IO_LEVEL_HIGH);
	task_ms_delay(led.pMytask, led.twinkleFreq, TASK_STA_NONBLOCKING_YIELD);

	//log测试
	//static mlu32 testLogDat=0;
	//log_print(LOG_LVL_WRANING, "%s-%s-%d :ms time =%d!",__FILE__,__FUNCTION__,__LINE__,testLogDat);
	//testLogDat++;
	mlu8 txbuf[10];
    txbuf[1]=2;
    txbuf[1]=10;
    txbuf[1]=0;



	task_end(led.pMytask);	//任务结束

}

//---------------------------------------------------------------------------
// 名    称：
//---------------------------------------------------------------------------
//功    能：无描述
//
//入口参数：无
//
//出口参数：无
//
//修改记录：
// 1. by zzx  2015-07-29		 编写函数
//---------------------------------------------------------------------------
void led_init(mlu32 twinkleFreq)
{
    led.redBitReg = 0;
    led.greenBitReg = 0;
    led.twinkleSignal = 0;
    led.redTwinkle = 0;
    led.greenTwinkle = 0;
    led.orangeTwinkle = 0;
    led.changeColorFlag = _reset;
	led.twinkleFreq=twinkleFreq;

    //led 闪烁定时器初始化
    //申请定时器

    //初始化我的定时器
    led.ptwinkletimer = ltimer_create ();
    ltimer_load_start (led.ptwinkletimer, twinkleFreq);

    //创建任务
    led.pMytask = task_create (LED_TASK_PRIO, led_run, nullptr, ltimer_create (),"led");


}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------

