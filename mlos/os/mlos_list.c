/**
 ******************************************************************************
 * 文件:mlos_list.c
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
#include <stdio.h>
#include <stdlib.h>
#include "mlos_list.h"
#include "mlos_malloc.h"

//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------

// Doubly Linked List 双向链表
// Singly Linked List 单向链表

//SLU:Singly Linked unordered ,单向链接无序的
//SLO:Singly Linked ordered,单向链接有序的
//DLU:Doubly Linked unordered ,双向链接无序的
//DLO:Doubly Linked ordered,双向链接有序的

//链表节点
#pragma anon_unions
typedef struct ListNode{

	//链表都有指向下一个，节点
	struct ListNode *pnext;

	//单向和双向链表，的共享内存
	union{
		
		//DoubleLinked，双向链表，才有指向前一个
		struct ListNode *pprev;
		
		//Singly Linked,单向链表 ,排序关键字
		ListSortkey slSortkey;

	};

	////Doubly Linked，双向链表 链表排序关键字
	ListSortkey dlSortkey;

	
}ListItem,*ListItemPtr,*ListIterator;

//----------------------------------------------------------------------------
//variable
//----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// 函数名：list_remove，移除链表中的数据项
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  
//ListPtr list：
//			链表对象指针，传入需要操作的链表
//void *item：
//			链表项，需要移除的数据项
//
//功能描述：sl:Singly Linked      <List>
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
mlu8 list_remove(ListPtr list, void *item)
{

	ListItemPtr cur, prev,end;

	//	链表为空
	if(nullptr==list->head ) 
	{
		list->itemCount=0;
		list->tail=nullptr;
		return 0;
	}


	//遍历的起止条件获取
	if (list->linkMode==e_lst_linkSinglyCircularly||list->linkMode==e_lst_linkDoublyCircularly)
	{
		prev=list->tail;								//循环链表
		end=list->head;
	}
	else
	{
		prev = nullptr;									//单向链表
		end=nullptr;
	}

	//遍历查找节点，并删除
	cur = list->head;
	do
	{
		//找到需要删除的节点
		if(cur == item) 
		{
			if(cur == list->head&& cur==list->tail) 
			{
				//移除最后一个节点
				list->tail=nullptr;
				list->head = nullptr;
				if(list->linkMode>=e_lst_linkDoubly) cur->pprev=nullptr;
				list->itemCount=0;
			}
			else 
			{
				if(cur==list->head) list->head = cur->pnext;//移除表头
				if(cur==list->tail) list->tail=prev;//移除表尾
				
				if (prev)prev->pnext=cur->pnext;
				if(list->linkMode>=e_lst_linkDoubly)
				{
					if(cur->pnext) cur->pnext->pprev=cur->pprev;
					cur->pprev=nullptr;
				}
				list->itemCount--;
			}
			
			cur->pnext = nullptr;//断开链接
			return 1;
			
		}

		//记录前一个节点，同时移动到下一个节点
		prev = cur;
		cur = cur->pnext;
		
	}while(cur!=end);

	//链表找不到节点
	return 0;
	
}

//-----------------------------------------------------------------------------
// 函数名：list_dl_remove，双向链表移除数据项
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  
//ListPtr list：
//			链表对象指针，传入需要操作的链表
//void *item：
//			链表项，需要移除的数据项
//
//功能描述：sl:Singly Linked      <List>
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
mlu8 list_dl_remove(ListPtr list, void *item)
{

	ListItemPtr pRmvItem;

	//参数判断
	if(item==nullptr||list==nullptr||list->linkMode<e_lst_linkDoubly)
	{
		return 1;
	}
	
	//	链表为空
	if(nullptr==list->head ) 
	{
		list->itemCount=0;
		list->tail=nullptr;
		return 2;
	}
	
	pRmvItem=(ListItemPtr)item;

	if(pRmvItem==list->head&&pRmvItem==list->tail)
	{
		list->head=nullptr;
		list->tail=nullptr;
	}
	else if(pRmvItem==list->head)
	{
		list->head = pRmvItem->pnext;//移除表头
	}
	else if(pRmvItem==list->tail) 
	{
		list->tail=pRmvItem->pprev;//移除表尾
	}

	//断开连接
	if (pRmvItem->pprev)
	{
		pRmvItem->pprev->pnext=pRmvItem->pnext;
	}
	if (pRmvItem->pnext)
	{
		pRmvItem->pnext->pprev=pRmvItem->pprev;
	}

	list->itemCount--;
	pRmvItem->pnext=nullptr;
	pRmvItem->pprev=nullptr;

	return 0;
	
}

//-----------------------------------------------------------------------------
// 函数名：list_append
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  
//ListPtr list：
//			链表对象指针，传入需要操作的链表
//void *item：
//			链表项，需要插入的数据项
//
//功能描述：
//		链表插入数据
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void list_append(ListPtr list,void * item)
{
	mlu8 insertTail;
	mlu32 newKey,curKey;
	ListItemPtr newItem,end,cur,prev;


	//初始化节点
	newItem=(ListItemPtr)item;
	list->itemCount++;

	//插入头节点
	if(nullptr==list->head)
	{
		if(list->linkMode<e_lst_linkDoubly)
		{
			//单向链表，头节点的指向
			if(list->linkMode==e_lst_linkSinglyCircularly) {newItem->pnext=newItem;}
			else {newItem->pnext=nullptr;}
		}
		else
		{
			//双向链表，头节点的指向
			if(list->linkMode==e_lst_linkDoublyCircularly){newItem->pnext=newItem;newItem->pprev=newItem;}
			else{newItem->pnext=nullptr;newItem->pprev=nullptr;}
		}
		list->head=item;
		list->tail=item;
		list->itemCount=1;
		return ;
	}

	//遍历节点排序
	insertTail=1;
	if(e_lst_sortUnordered!=list->sortord)
	{
		//排序插入
		cur=(ListItemPtr)list->head;
		if (list->linkMode==e_lst_linkSinglyCircularly||list->linkMode==e_lst_linkDoublyCircularly)
		{
			end=cur; prev=(ListItemPtr)list->tail;
		}
		else{ end=nullptr; prev=nullptr;}

		//取当前插入节点关键字
		if(list->linkMode<e_lst_linkDoubly){newKey=newItem->slSortkey;}else{newKey=newItem->dlSortkey;}
		do
		{
			//取当前遍历节点关键字
			if(list->linkMode<e_lst_linkDoubly){curKey=cur->slSortkey;}else{curKey=cur->dlSortkey;}

			//比较关键字，查找插入的位置
			if ((list->sortord==e_lst_sortAscending&&newKey<=curKey)||//升序
				(list->sortord==e_lst_sortDescending&&newKey>=curKey))//降序
			{
				insertTail=0;
				break;
			}
				
			prev=cur;cur = cur->pnext;
				
		}while ( cur != end);
		
	}
	
	//插入位置判断
	if(insertTail==1)
	{
		//尾部插入
		prev=list->tail;
		cur=prev->pnext;
		list->tail=newItem;
	}
	else
	{
		if(cur==list->head) list->head=newItem;//插入
	}

	//建立链接
	if(prev!=nullptr)prev->pnext=newItem;
	newItem->pnext=cur;
	if(list->linkMode>=e_lst_linkDoubly)
	{
		newItem->pprev=prev;
		if(cur){cur->pprev=newItem;}
	}
		
}

//-----------------------------------------------------------------------------
// 函数名：list_clear
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  
//ListPtr list：
//			链表对象指针，传入需要操作的链表
//
//
//功能描述：
//	清空链表
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void list_clear(ListPtr list)
{
	if(list==nullptr) return;
	list->itemCount=0;
	list->head=nullptr;
	list->tail=nullptr;
}

//-----------------------------------------------------------------------------
// 函数名：list_key_contains
//-----------------------------------------------------------------------------
//
// 返回值 : 
// void*: 返回查到到的数据项，=nullptr标识没找到
// 参数   :  
//ListPtr list：
//			链表对象指针，传入需要操作的链表
//ListSortkey key:
//		数据项的关键字，需要查找的关键字
//功能描述：
//		查找，关键字为key的数据项，并返回
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void* list_key_contains(ListPtr list,ListSortkey key)
{
	ListIterator it, end;
	ListSortkey curKey;
	
	if(list==nullptr||list->head == nullptr)
	{
		return nullptr;
	}
	

	//循环链表判断
	if (list->linkMode==e_lst_linkSinglyCircularly||list->linkMode==e_lst_linkDoublyCircularly)
	{
		end=list->head;
	}
	else
	{
		end=nullptr;
	}

	it = list->head;
	do
	{
		if(list->linkMode<e_lst_linkDoubly) curKey=it->slSortkey;
		else curKey=it->dlSortkey;
		if (curKey==key)
		{
			return it;
		}
		it = it->pnext;
		
	}while(it!=end);

	return nullptr;


}

//-----------------------------------------------------------------------------
// 函数名：list_value_contains
//-----------------------------------------------------------------------------
//
// 返回值 : 
// void*: 返回查到到的数据项，=nullptr标识没找到
// 参数   :  
//ListPtr list：
//			链表对象指针，传入需要操作的链表
//void * value:
//		数据项的指针，需要查找的数据
//功能描述：
//		查找，链表中是否存在该数据项，并返回

//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void* list_value_contains(ListPtr list,void * value)
{
	ListIterator it, end;

	//	链表为空
	if(list->head == nullptr) 
	{
		return nullptr;
	}
	
	//循环链表判断
	if (list->linkMode==e_lst_linkSinglyCircularly||list->linkMode==e_lst_linkDoublyCircularly)
	{
		end=list->head;
	}
	else
	{
		end=nullptr;
	}
	
	it = list->head;
	do
	{
		if (value == it)
		{
			return it;
		}
		it = it->pnext;
		
	}while(it!=end);

	return nullptr;

}

//-----------------------------------------------------------------------------
// 函数名：list_init，初始化链表
//-----------------------------------------------------------------------------
//
// 返回值 : void
// 参数   :  void
//ListPtr list：
//			链表对象指针，传入需要操作的链表
//void * owner:
//			链表的拥有者，即该链表属于谁
//ListSortord sortord:
//			链表的排序方式，有无序、升序、降序三种排序方式
//ListLinkMode lnkmd:
//			链表的链接方式，有单向、单向循环、双向、双向循环，四种链接方式
//功能描述：
//		初始化链表
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
void list_init(ListPtr list,ListSortord sortord,ListLinkMode lnkmd)
{

	list->sortord=sortord;
	list->linkMode=lnkmd;
	list->itemCount=0;
	list->head=nullptr;
	list->tail=nullptr;
}

//-----------------------------------------------------------------------------
// 函数名：list_create，创建链表
//-----------------------------------------------------------------------------
//
// 返回值 : 
// ListPtr: 返回创建的链表对象，=nullptr标识创建失败
// 参数   :  
//MemoryType memtype：
// 			链表创建时，需要分配的内存类型
//void * creator：
//			链表的创建者，即该链表属于谁
//ListSortord sortord:
//			链表的排序方式，有无序、升序、降序三种排序方式
//ListLinkMode lnkmd:
//			链表的链接方式，有单向、单向循环、双向、双向循环，四种链接方式
//功能描述：
//
//
//修改履历：
//1.zzx ：  2014-12-01  ：创建函数
//
//-----------------------------------------------------------------------------
ListPtr list_create(MemoryType memtype,ListSortord sortord,ListLinkMode lnkmd)
{

	ListPtr list;
	list=mlos_malloc(memtype, sizeof(List));
	if(nullptr==list)
		return list;
		
	list->sortord=sortord;
	list->linkMode=lnkmd;
	list->itemCount=0;
	list->head=nullptr;
	list->tail=nullptr;

	return list;

}

//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------



