#ifndef __AE_H__
#define __AE_H__

#include <time.h>

/**
 * 事件执行状态
 */
// 成功
#define AE_OK 0
// 出错
#define AE_ERR -1

/**
 * 文件事件状态
 */
// 未设置
#define AE_NONE 0
// 可读 
#define AE_READABLE 1
// 可写
#define AE_WRITABLE 2
/**
 * 对于WRITABLE，如果可读事件已在同一事件循环迭代中激发，则永远不要激活该事件
 * 当你希望在发送回复之前将内容持久化到磁盘，并且希望以组方式执行此操作时候，此功能非常有用
 */
#define AE_BARRIER 4

/**
 * 事件处理器执行 flags
 */
// 文件事件
#define AE_FILE_EVENTS 1
// 事件事件
#define AE_TIME_EVENTS 2
// 所有事件
#define AE_ALL_EVENTS (AE_FILE_EVENTS | AE_TIME_EVENTS)
// 不阻塞，也不进行等待
#define AE_DONT_WAIT 4
#define AE_CALL_AFTER_SLEEP 8

/**
 * 决定事件事件是否要持续执行flag
 */
#define AE_NOMORE -1
#define AE_DELETED_EVENT_ID -1

/**
 * Macors
 */
#define AE_NOTUSED(V) ((void) V)

/**
 * 事件处理器状态
 */
struct aeEventLoop;

/**
 * 事件接口
 */
typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

/**
 * 文件事件结构
 */
typedef struct aeFileEvent {
    /**
     * 坚挺四件类型编码
     * 值可以是 AE_READABLE 或 AE_WRITABLE
     * 或者 AE_READABLE | AE_WRIABLE
     */
    int mask;   

    // 读事件处理器
    aeFileProc *rfileProc;

    // 写事件处理器
    aeFileEvent *wfileProc;

    // 多路复用库的私有数据
    void *clientData;
} aeFileEvent;

/**
 * 时间事件结构
 */
typedef struct aeTimeEvent {
    // 时间事件标识符
    long long id;

    // 事件的到达事件
    long when_sec;  // 秒
    long when_ms;   // 毫秒

    // 事件处理函数
    aeTimeProc *timeProc;

    // 事件释放函数
    aeEventFinalizerProc *finalizerProc;

    // 多路复用库的私有数据
    void *clientData;

    // 指向下个时间事件结构，形成双向链表
    struct aeTimeEvent *prev;
    struct aeTimeEvent *next;
} aeTimeEvent;

/**
 * 已就绪事件
 */
typedef struct aeFiredEvent {
    // 已就绪文件描述符
    int fd;
    /**
     * 事件类型编码
     * 值可以是AE_RREADABLE 或 AE_WRITABLE
     */
    int mask;
} aeFiredEvent;

/**
 * 事件处理器的状态
 */
typedef struct aeEventLoop {
    // 目前已注册的最大描述符
    int maxfd;

    // 目前已经追踪的最大描述符
    int setsize;

    // 用于生成时间事件 id
    long long timeEventNextId;

    // 最后一次执行时间事件的事件
    time_t lastTime;

    // 已注册的文件事件
    aeFileEvent *events;

    // 已就绪的文件事件
    aeFiredEvent *fired;

    // 时间事件
    aeTimeEvent *timeEventHead;

    // 事件处理器的开关
    int stop;

    // 多路复用库的私有数据
    void *apidata;

    // 在处理事件前要执行的函数
    aeBeforeSleepProc *beforesleep;

    // 在处理事件后要执行的函数
    aeBeforeSleepProc *aftersleep;
} aeEventLoop;

// 原型
aeEventLoop *aeCreateEventLoop(int setsize);

void aeDeleteEventLoop(aeEventLoop *eventLoop);

void aeStop(aeEventLoop *eventLoop);

int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileProc *proc, void *clientData);

void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask);

int aeGetFileEvents(aeEventLoop *eventLoop, int fd);

long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds, aeTimeProc *proc, void *clientData, aeEventFinalizerProc *finalizerProc);
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id);

int aeProcessEvents(aeEventLoop *eventLoop, int flags);

int aeWait(int fd, int mask, long long milliseconds);

void aeMain(aeEventLoop *eventLoop);

char *aeGetApiName(void);

void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforesleep);

void aeSetAfterSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *aftersleep);

int aeGetSetSize(aeEventLoop *eventLoop);

int aeResizeSetSize(aeEventLoop *eventLoop, int setsize);

void aeSetDontWait(aeEventLoop *eventLoop, int noWait);

#endif