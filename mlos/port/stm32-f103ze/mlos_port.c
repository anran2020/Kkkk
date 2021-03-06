/**
******************************************************************************
* 文件:
* 作者:	 zzx
* 版本:	 V0.01
* 日期:	   2014-12-01
* 内容简介:
*
******************************************************************************
*						文件历史
*
* 版本号-----  日期	-----	作者	  ------   说明
* v0.1 		2014-12-01	   zzx				   创建该文件
*
*
*
******************************************************************************
**/

//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#if 1

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_port.h"
#include "mlos_malloc.h"

//----------------------------------------------------------------------------
// define 
//----------------------------------------------------------------------------

#define SRAM1_U32_SIZE         (8*1024)//n*4k,

//----------------------------------------------------------------------------
// global variable 
//----------------------------------------------------------------------------

//mcu f103ze 内存资源声明
//sram
 mlu32 sram1[SRAM1_U32_SIZE];
MemoryBlock memblock1;


//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void port_clock_init(void)
{

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	// 开启TIMx_CLK,x[6,7] 
	RCC_APB1PeriphClockCmd(DELAY_TIMER_CLK|TASK_TIMING_TIMER_CLK, ENABLE); 
	
	// 累计 TIM_Period个后产生一个更新或者中断
	TIM_TimeBaseStructure.TIM_Period = 65535-1; 


	//TIMxCLK=SystemCoreClock/2=36MHz
	// 设定定时器频率为=TIMxCLK/(TIM_Prescaler+1)=1MHz
	TIM_TimeBaseStructure.TIM_Prescaler = 36-1;//us

	// 初始化定时器TIMx
	TIM_TimeBaseInit(DELAY_TIMER, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TASK_TIMING_TIMER, &TIM_TimeBaseStructure);

	// 清除定时器更新中断标志位
	TIM_ClearFlag(DELAY_TIMER, TIM_FLAG_Update);
	TIM_ClearFlag(TASK_TIMING_TIMER, TIM_FLAG_Update);

	// 开启定时器更新中断
	//TIM_ITConfig(BASIC_TIM,TIM_IT_Update,ENABLE);

	// 使能定时器
	TIM_Cmd(DELAY_TIMER, ENABLE);
	TIM_Cmd(TASK_TIMING_TIMER, ENABLE);

}

//-----------------------------------------------------------------------------
// 函数名：
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//
//
//
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void port_mem_init(void)
{
	//挂载内存，到mlos
	memblock1.mem=(mlu8*)sram1;
	memblock1.size=SRAM1_U32_SIZE*4;
	memblock1.type=e_mem_sram;
	mlos_mem_mount(&memblock1);
}

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif


