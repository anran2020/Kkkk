/**
 ******************************************************************************
 * 文件:mlos_list.h
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


#ifndef _MLOS_LIST_H_
#define _MLOS_LIST_H_

//---------------------------------------------------------
//inclludes
//---------------------------------------------------------

#include "mlos_dtd.h"
#include "mlos_malloc.h"

//---------------------------------------------------------
//链表节点数据项排序关键字，数据类型
//---------------------------------------------------------

typedef mlu32 ListSortkey;



//---------------------------------------------------------
//SLU:Singly Linked unordered ,单向链接无序的
//SLO:Singly Linked ordered,单向链接有序的
//DLU:Doubly Linked unordered ,双向链接无序的
//DLO:Doubly Linked ordered,双向链接有序的
//四种链表的元素项，基础属性结构体定义，
//链表item 继承基结构体
//---------------------------------------------------------

//单向链表，item
#define SLListItemBaseStruct(ItemType)  struct {				\
															\
		struct ItemType* pnext;/*指向下一个item*/			\
}

//双向链表，item
#define DLListItemBaseStruct(ItemType)  struct {\
				\
		struct ItemType* pnext;/*指向下一个item*/\
		struct ItemType* pprev;/*指向上一个item*/\
}



//---------------------------------------------------------
//链表排序方式
//---------------------------------------------------------
typedef enum{

	e_lst_sortUnordered=1,//无序的
	e_lst_sortAscending,//升序
	e_lst_sortDescending,//降序
		
}ListSortord;
	
//---------------------------------------------------------
//链表链接模式
//---------------------------------------------------------
typedef enum{

	e_lst_linkSingly=1,//单向链表
	e_lst_linkSinglyCircularly,//单向循环链表	
	e_lst_linkDoubly,//双向链表
	e_lst_linkDoublyCircularly,//双向循环链表
	
}ListLinkMode;

//---------------------------------------------------------
//链表
//---------------------------------------------------------
typedef struct{


	mlu8 linkMode;//链接模式
	mlu8 sortord;//排序方式
	mlu16 itemCount;//链表节点数

	void * head;//链表头
	void * tail;//链表尾

}List,*ListPtr;

//----------------------------------------------------------------------------
//extern variable
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//extern maroc
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//extern function
//----------------------------------------------------------------------------

ListPtr list_create(MemoryType memtype,ListSortord sortord,ListLinkMode lnkmd);
void list_init(ListPtr list,ListSortord sortord,ListLinkMode lnkmd);

void* list_value_contains(ListPtr list,void * value);
void* list_key_contains(ListPtr list,ListSortkey key);

void list_append(ListPtr list,void * item);
mlu8 list_remove(ListPtr list, void *item);
void list_clear(ListPtr list);

//双向链接链表，快速移除节点，接口
mlu8 list_dl_remove(ListPtr list, void *item);

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif // 

