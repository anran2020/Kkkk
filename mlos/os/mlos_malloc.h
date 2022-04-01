/**
 ******************************************************************************
 * 文件:os_malloc.h
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


#ifndef _MLOS_MALLOC_H_
#define _MLOS_MALLOC_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "mlos_dtd.h"


#if (1)

//---------------------------------------------------------
//mem //字节对齐
//---------------------------------------------------------

#define ARCH_BYTE_ALIGNMENT 			(4)


#if ARCH_BYTE_ALIGNMENT == 16
	#define BYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if ARCH_BYTE_ALIGNMENT == 8
	#define BYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if ARCH_BYTE_ALIGNMENT == 4
	#define BYTE_ALIGNMENT_MASK	( 0x0003 )
#endif

#if ARCH_BYTE_ALIGNMENT == 2
	#define BYTE_ALIGNMENT_MASK	( 0x0001 )
#endif

#if ARCH_BYTE_ALIGNMENT == 1
	#define BYTE_ALIGNMENT_MASK	( 0x0000 )
#endif

#ifndef BYTE_ALIGNMENT_MASK
	#error "Invalid MCU_BYTE_ALIGNMENT definition"
#endif

//---------------------------------------------------------
//mem type
//---------------------------------------------------------
typedef enum{

	e_mem_sram=1,
	e_mem_sdram,
	e_mem_exsram,//外部扩展sram，速度相对慢一点

}MemoryType;

//---------------------------------------------------------
//mem block
//---------------------------------------------------------
typedef struct MemBlockNode{

	struct MemBlockNode *pnext;
	mlu32 nextFreebyte;	
	mlu32 size;
	mlu8 *mem;
	MemoryType type;
	
}MemoryBlock,*MemBlockPtr;

//---------------------------------------------------------
//extern var
//---------------------------------------------------------



//---------------------------------------------------------
//export marco
//---------------------------------------------------------



//---------------------------------------------------------
//export funcation
//---------------------------------------------------------

void mlos_mem_init(void);

mlu8  mlos_mem_mount(MemBlockPtr mb);

void* mlos_malloc(MemoryType mt,mlu32 size);


//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif
#endif
