
#ifndef _BASIC_H_
#define _BASIC_H_
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
/*#define __LINUX_SYSTEM__*/

#ifdef __LINUX_SYSTEM__
#include <pthread.h>
#include <semaphore.h>
#else
#define _RA6_M4_
#endif

/*以下为临时调试宏开关,全部注释后才能编译版本*/
//#define DebugVersion

#ifdef DebugVersion
#else
//#define DbgOnUpper
#endif

//#define TimeWoLowStamp  /*等下位机加好时间戳/联机响应后删除该宏*/
//#define CanCtrlWoReTx /*启动命令的重传字段*/
#define TmpStepSave  /*等工步存储加好后删除该宏*/
#define NdbdStateWoChk  /*等有实物后删除,现在不检测针床有料和压接*/

/*调试宏开关结束*/

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
#ifdef __x86_64__
    typedef int s32;
    typedef unsigned int u32;
    typedef long s64;
    typedef unsigned long u64;
    #define VirtualMachineVersion
#else
    #ifdef _RA6_M4_
        typedef int s32;
        typedef unsigned int u32;
    #else
        typedef long s32;
        typedef unsigned long u32;
        typedef long long s64;
        typedef unsigned long long u64;
    #endif
#endif
typedef unsigned long point;

typedef unsigned char b8;   /*bool, 1 byte*/
#define True 1
#define False 0

#define WiReset 1
#define WoReset 0

#define ChnStart 1
#define ChnStop 0

typedef unsigned char Ret;
#define Ok 0
#define Nok 1

/*双链表相关*//*除工步和保护外，均用双链表*/
typedef struct chainDTag
{
    struct chainDTag *next;
    struct chainDTag *prev;
}ChainD,ListD;

#define ChainInitD(chn) \
do \
{ \
    (chn)->next = chn; \
    (chn)->prev = chn; \
}while(0)

/*插到list前面，等价于，作为以list为链表头的队列的尾巴*/
/*引用时参数内不可嵌套next/prev,例如ADD_CHAIN(a->next,b),危险*/
#define ChainInsertD(list, chn) \
do \
{ \
    (chn)->next = list; \
    (chn)->prev = (list)->prev; \
    (list)->prev->next = chn; \
    (list)->prev = chn; \
}while(0)

/*将chn从所在链表删除但不初始化，使用时需考虑安全性*/
/*如果不了解详细情况，应该用下面的宏*/
/*引用时参数不可嵌套next/prev,例如CHAIN_REMOVE(a->next),危险*/
#define ChainDeleteD(chn) \
do \
{ \
    (chn)->prev->next = (chn)->next; \
    (chn)->next->prev = (chn)->prev; \
}while(0)

/*将chn从所在链表删除并初始化*/
/*引用时参数不可嵌套next/prev,例如CHAIN_REMOVE_SAFE(a->next),危险*/
#define ChainDelSafeD(chn) \
do \
{ \
    ChainDeleteD(chn); \
    ChainInitD(chn); \
}while(0)

/*将chn从所在链表挪到新链表*/
/*引用时参数不可嵌套next/prev,例如CHAIN_REMOVE_SAFE(a->next),危险*/
#define ChainMoveD(list, chn) \
do \
{ \
    ChainDeleteD(chn); \
    ChainInsertD(list, chn); \
}while(0)

#define ChainInList(chn) ((chn)->next != (chn))
#define ListIsEmpty(list) ((list)->next == (list))
#define ListInitD ChainInitD

/*将list链表头更换为copy,即挪动链表*/
/*引用时参数不可嵌套next/prev,例如CHAIN_REPLACE(a->next,b),危险*/
#define ListMoveD(list, copy) \
do \
{ \
    if (ListIsEmpty(list)) \
    { \
        ChainInitD(copy); \
    } \
    else \
    { \
        (copy)->next = (list)->next; \
        (copy)->prev = (list)->prev; \
        (list)->prev->next = copy; \
        (list)->next->prev = copy; \
        ChainInitD(list); \
    } \
}while(0)

/*只遍历而不增删可以用这个*/
#define ListForEach(list, chn) \
    for (chn=(list)->next; chn!=(list); chn=(chn)->next)

/*涉及增删必须要用这个*/
#define ListForEachSafe(list, chn, tmp) \
    for (chn=(list)->next,tmp=(chn)->next; chn!=list; chn=tmp,tmp=(chn)->next)


/*单链表相关*//*单链表仅用于工步和保护，节约一丢丢内存*/
typedef struct chainSTag
{
    struct chainSTag *next;
}ChainS,ListS;

#define ChainInitS(chn) \
do \
{ \
    (chn)->next = chn; \
}while(0)

#define ListInitS ChainInitS

/*插到prevChn后面*/
/*引用时参数内不可嵌套next/prev,例如ADD_CHAIN(a->next,b),危险*/
#define ChainInsertS(prevChn, newChn) \
do \
{ \
    (newChn)->next = (prevChn)->next; \
    (prevChn)->next = newChn; \
}while(0)

/*将chn从所在链表删除但不初始化，使用时需考虑安全性*/
/*如果不了解详细情况，应该用下面的宏*/
/*引用时参数不可嵌套next/prev,例如CHAIN_REMOVE(a->next),危险*/
#define ChainDeleteS(prevChn, delChn) \
do \
{ \
    (prevChn)->next = (delChn)->next; \
}while(0)

/*将chn从所在链表删除并初始化*/
/*引用时参数不可嵌套next/prev,例如CHAIN_REMOVE_SAFE(a->next),危险*/
#define ChainDelSafeS(prevChn, delChn) \
do \
{ \
    ChainDeleteS(prevChn, delChn); \
    ChainInitS(delChn); \
}while(0)

#ifndef __LINUX_SYSTEM__

/*定义带互斥锁保护的队列*/
typedef struct queueTag
{
    ListD list;
    /*pthread_mutex_t mutex;*/
}Queue;

#define QueueInit(queue) \
do { \
    /*pthread_mutex_init(&(queue)->mutex, NULL);*/ \
    ListInitD(&(queue)->list); \
}while(0)

/*弹出队列头*/
#define QueueGet(queue, chain) \
do { \
    /*pthread_mutex_lock(&(queue)->mutex);*/ \
    chain = (queue)->list.next; \
    ChainDelSafeD(chain); \
    /*pthread_mutex_unlock(&(queue)->mutex);*/ \
}while(0)

/*塞到队列尾*/
#define QueuePut(queue, chain) \
do { \
    /*pthread_mutex_lock(&(queue)->mutex);*/ \
    ChainInsertD(&(queue)->list, chain); \
    /*pthread_mutex_unlock(&(queue)->mutex);*/ \
}while(0)

/*定义带同步信号量的异步消息队列*/
typedef struct msgQueueTag
{
    Queue queue;
    /*sem_t sem;*/
}MsgQueue;

#define MsgQueueInit(msgQ) \
do { \
    QueueInit(&(msgQ)->queue); \
    /*sem_init(&(msgQ)->sem, 0, 0);*/ \
} while(0)

/*不对称收发:单个发*/
#define MsgQueueSend(msgQ, chain)  \
do { \
    QueuePut(&(msgQ)->queue, chain); \
    /*sem_post(&(msgQ)->sem);*/ \
} while(0)

/*不对称收发:全部收*/
#define MsgQueueRecv(msgQ, mList) \
do { \
    /*sem_wait(&(msgQ)->sem);*/ \
    /*pthread_mutex_lock(&(msgQ)->queue.mutex);*/ \
    ListMoveD(&(msgQ)->queue.list, mList); \
    /*pthread_mutex_unlock(&(msgQ)->queue.mutex);*/ \
} while(0)

#else  /*__LINUX_SYSTEM__*/

#if 0   /*碍眼*/
/*定义带互斥锁保护的队列*/
typedef struct queueTag
{
    ListD list;
    pthread_mutex_t mutex;
}Queue;

#define QueueInit(queue) \
do { \
    pthread_mutex_init(&(queue)->mutex, NULL); \
    ListInitD(&(queue)->list); \
}while(0)

/*弹出队列头*/
#define QueueGet(queue, chain) \
do { \
    pthread_mutex_lock(&(queue)->mutex); \
    chain = (queue)->list.next; \
    ChainDeleteD(chain); \
    pthread_mutex_unlock(&(queue)->mutex); \
}while(0)

/*塞到队列尾*/
#define QueuePut(queue, chain) \
do { \
    pthread_mutex_lock(&(queue)->mutex); \
    ChainInsertD(&(queue)->list, chain); \
    pthread_mutex_unlock(&(queue)->mutex); \
}while(0)

/*定义带同步信号量的异步消息队列*/
typedef struct msgQueueTag
{
    Queue queue;
    sem_t sem;
}MsgQueue;

#define MsgQueueInit(msgQ) \
do { \
    QueueInit(&(msgQ)->queue); \
    sem_init(&(msgQ)->sem, 0, 0); \
} while(0)

/*不对称收发:单个发*/
#define MsgQueueSend(msgQ, chain)  \
do { \
    QueuePut(&(msgQ)->queue, chain); \
    sem_post(&(msgQ)->sem); \
} while(0)

/*不对称收发:全部收*/
#define MsgQueueRecv(msgQ, mList) \
do { \
    sem_wait(&(msgQ)->sem); \
    pthread_mutex_lock(&(msgQ)->queue.mutex); \
    ListMoveD(&(msgQ)->queue.list, mList); \
    pthread_mutex_unlock(&(msgQ)->queue.mutex); \
} while(0)
#endif
#endif /*__LINUX_SYSTEM__*/

#define Offset(type, mbr) ((point)((u8 *)&((type *)0)->mbr))
#define Container(type, mbr, addr) (type *)((u8 *)(addr) - Offset(type, mbr))
//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译
#endif



