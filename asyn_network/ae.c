#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <string>
#include <time.h>
#include <errno.h>

#include "ae.h"

#include "ae_epoll.c"


aeEventLoop *aeCreateEventLoop(int setsize) {
    aeEventLoop *eventLoop;
    int i;

    // 创建事件状态结构
    if ((eventLoop = malloc(sizeof(*eventLoop))) == NULL) goto err;
    
    // 初始化文件事件结构和已就绪文件事件结构数组
    eventLoop->events = malloc(sizeof(aeFileEvent) * setsize);
    eventLoop->fired = malloc(sizeof(aeFiredEvent) * setsize);
    if (eventLoop->events == NULL || eventLoop->fired == NULL) goto err;

    // 设置数组大小
    eventLoop->setsize = setsize;
    // 初始化执行最近一次执行事件
    eventLoop->lastTime = time(NULL);
    
    // 初始化时间事件结构
    eventLoop->timeEventHead = NULL;
    eventLoop->timeEventNextId = 0;
    
    eventLoop->stop = 0;
    eventLoop->maxfd = -1;
    eventLoop->beforesleep = NULL;
    eventLoop->aftersleep = NULL;
    eventLoop->flags = 0;
    if (aeApiCreate(eventLoop) == -1) goto err;

    /* Events with mask == AE_NONE are not set. So let's initialize the
     * vector with it. */
    // 初始化监听事件
    for (i = 0; i < setsize; i++)
        eventLoop->events[i].mask = AE_NONE;

    // 返回事件循环
    return eventLoop;

err:
    if (eventLoop) {
        free(eventLoop->events);
        free(eventLoop->fired);
        free(eventLoop);
    }
    return NULL;
}

/* Return the current set size. */
// 返回当前文件槽大小
int aeGetSetSize(aeEventLoop *eventLoop) {
    return eventLoop->setsize;
}

/* Tells the next iteration/s of the event processing to set timeout of 0. */
// 通知事件处理的下一次迭代将超时设置为0
void aeSetDontWait(aeEventLoop *eventLoop, int noWait) {
    if (noWait) {
        eventLoop->flags |= AE_DONT_WAIT;
    } else {
        eventLoop->flags &= ~AE_DONT_WAIT;
    }
}

/** Resize the maximum set size of the event loop.
 * 
 * 调整事件槽的大小
 * 
 * If the requested set size is smaller than the current set size, but
 * there is already a file descriptor in use that is >= the requested
 * set size minus one, AE_ERR is returned and the operation is not
 * performed at all.
 * 
 * 如果尝试调整大小为setsize， 但是有 >= setsize的文件描述符存在
 * 那么返回AE——ERR, 不进行任何动作
 *
 * Otherwise AE_OK is returned and the operation is successful. 
 * 
 * 否则，执行大小调整操作，并返回AE_OK
 **/
int aeResizeSetSize(aeEventLoop *eventLoop, int setsize) {
    int i;

    if (setsize == eventLoop->setsize) return AE_OK;
    if (eventLoop->maxfd >= setsize) return AE_ERR;
    if (aeApiResize(eventLoop,setsize) == -1) return AE_ERR;

    eventLoop->events = realloc(eventLoop->events, sizeof(aeFileEvent) * setsize);
    eventLoop->fired = realloc(eventLoop->fired, sizeof(aeFiredEvent) * setsize);
    eventLoop->setsize = setsize;

    /* Make sure that if we created new slots, they are initialized with
     * an AE_NONE mask. */
    for (i = eventLoop->maxfd+1; i < setsize; i++) {
        eventLoop->events[i].mask = AE_NONE;
    }
    return AE_OK;
}

/**
 * 删除事件处理器
 */
void aeDeleteEventLoop(aeEventLoop *eventLoop) {
    aeApiFree(eventLoop);
    free(eventLoop->events);
    free(eventLoop->fired);
    free(eventLoop);
}

// 停止事件处理器
void aeStop(aeEventLoop *eventLoop) {
    eventLoop->stop = 1;
}

/**
 * 根据mask 参数的值，监听fd文件的状态
 * 当fd可用时，执行proc函数
 */
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileProc *proc, void *clientData)
{
    if (fd >= eventLoop->setsize) {
        errno = ERANGE;
        return AE_ERR;
    }
    
    // 取出文件事件结构
    aeFileEvent *fe = &eventLoop->events[fd];

    // 监听指定fd的指定事件
    if (aeApiAddEvent(eventLoop, fd, mask) == -1) {
        return AE_ERR;
    }

    // 设置文件事件类型，以及事件的处理器
    fe->mask |= mask;
    if (mask & AE_READABLE) fe->rfileProc = proc;
    if (mask & AE_WRITABLE) fe->wfileProc = proc;

    // 私有数据
    fe->clientData = clientData;

    // 如果有需要，更新事件处理器的最大 fd
    if (fd > eventLoop->maxfd) {
        eventLoop->maxfd = fd;
    }

    return AE_OK;
}

/**
 * 将 fd 从mask 指定的监听队列中删除
 */
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask) {

    if (fd >= eventLoop->setsize) return;

    // 取出文件事件结构
    aeFileEvent *fe = &eventLoop->events[fd];

    // 未设置舰艇的事件类型，直接返回
    if (fe->mask == AE_NONE) return;

    /* We want to always remove AE_BARRIER if set when AE_WRITABLE
     * is removed. */
    if (mask & AE_WRITABLE) mask |= AE_BARRIER;
    
    // 取消对给定 fd的给定事件的监视
    aeApiDelEvent(eventLoop, fd, mask);

    // 计算新掩码
    fe->mask = fe->mask & (~mask);
    if (fd == eventLoop->maxfd && fe->mask == AE_NONE) {
        /* Update the max fd */
        int j;

        for (j = eventLoop->maxfd-1; j >= 0; j--) {
            if (eventLoop->events[j].mask != AE_NONE) break;
        }
        eventLoop->maxfd = j;
    }
}

/**
 * 获得给定fd正在舰艇的事件类型
 */
int aeGetFileEvents(aeEventLoop *eventLoop, int fd) {
    if (fd >= eventLoop->setsize) return 0;
    aeFileEvent *fe = &eventLoop->events[fd];

    return fe->mask;
}

/**
 * 取出当前事件的秒和毫秒
 * 并分别将它们保存到 seconds 和 milliseconds 参数中
 */
static void aeGetTime(long *seconds, long *milliseconds) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *milliseconds = tv.tv_usec / 1000;
}

/**
 * 在当前时间上 milliseconds 毫秒
 * 并且将加上之后的秒数和毫秒数分别保存在 sec 和 ms指针中
 */
static void aeAddMillisecondsToNow(long long milliseconds, long *sec, long *ms) {
    long cur_sec, cur_ms, when_sec, when_ms;

    // 获取当前事件
    aeGetTime(&cur_sec, &cur_ms);
    when_sec = cur_sec + milliseconds / 1000;
    when_ms = cur_ms + milliseconds % 1000;

    /**
     * 进位:
     *  如果 when_ms 大于等于1000，那么将when_sec 增大一秒
     */
    if (when_ms >= 1000) {
        when_sec ++;
        when_ms -= 1000;
    }

    // 保存到指针中
    *sec = when_sec;
    *ms = when_ms;
}

/**
 * 创建时间事件
 */
long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds, aeTimeProc *proc, void *clientData, aeEventFinalizerProc *finalizerProc) {
    // 更新事件计数其
    long long id = eventLoop->timeEventNextId++;

    // 创建时间事件结构
    aeTimeEvent *te;

    te = malloc(sizeof(*te));
    if (te == NULL) return AE_ERR;

    // 设置ID
    te->id = id;

    // 设置处理事件的时间
    aeAddMillisecondsToNow(milliseconds,&te->when_sec,&te->when_ms);
    // 设置事件处理器
    te->timeProc = proc;
    te->finalizerProc = finalizerProc;
    // 设置私有数据
    te->clientData = clientData;
    te->prev = NULL;

    // 将新事件放入表头
    te->next = eventLoop->timeEventHead;
    if (te->next) {
        te->next->prev = te;
    }
    eventLoop->timeEventHead = te;

    return id;
}

/**
 * 删除给定id的时间事件
 */
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id) {
    // 遍历链表
    aeTimeEvent *te = eventLoop->timeEventHead;
    while(te) {
        // 发现目标事件，删除
        if (te->id == id) {
            te->id = AE_DELETED_EVENT_ID;
            return AE_OK;
        }
        te = te->next;
    }

    return AE_ERR; /* NO event with the specified ID found */
}

/* Search the first timer to fire.
 * This operation is useful to know how many time the select can be
 * put in sleep without to delay any event.
 * If there are no timers NULL is returned.
 *
 * Note that's O(N) since time events are unsorted.
 * Possible optimizations (not needed by Redis so far, but...):
 * 1) Insert the event in order, so that the nearest is just the head.
 *    Much better but still insertion or deletion of timers is O(N).
 * 2) Use a skiplist to have this operation as O(1) and insertion as O(log(N)).
 * 
 * 寻找目前时间最近的时间事件
 */
static aeTimeEvent *aeSearchNearestTimer(aeEventLoop *eventLoop)
{
    aeTimeEvent *te = eventLoop->timeEventHead;
    aeTimeEvent *nearest = NULL;

    while(te) {
        if (!nearest || te->when_sec < nearest->when_sec || (te->when_sec == nearest->when_sec && te->when_ms < nearest->when_ms)) {
            nearest = te;
        }
        te = te->next;
    }

    return nearest;
}

/* Process time events */
// 处理所有已到达的时间事件
static int processTimeEvents(aeEventLoop *eventLoop) {
    int processed = 0;
    aeTimeEvent *te;
    long long maxId;
    time_t now = time(NULL);

    /* If the system clock is moved to the future, and then set back to the
     * right value, time events may be delayed in a random way. Often this
     * means that scheduled operations will not be performed soon enough.
     *
     * Here we try to detect system clock skews, and force all the time
     * events to be processed ASAP when this happens: the idea is that
     * processing events earlier is less dangerous than delaying them
     * indefinitely, and practice suggests it is. */
    // 通过充值事件的运行时间
    // 防止因时间穿插(skew) 而造成的事件处理混乱
    if (now < eventLoop->lastTime) {
        te = eventLoop->timeEventHead;
        while(te) {
            te->when_sec = 0;
            te = te->next;
        }
    }
    
    // 更新最后一次处理时间时间的时间
    eventLoop->lastTime = now;

    // 遍历链表
    // 执行那些已经到达的事件
    te = eventLoop->timeEventHead;
    maxId = eventLoop->timeEventNextId - 1;

    while(te) {
        long now_sec, now_ms;
        long long id;

        /* Remove events scheduled for deletion. */
        if (te->id == AE_DELETED_EVENT_ID) {
            aeTimeEvent *next = te->next;
            if (te->prev) {
                te->prev->next = te->next;
            } else {
                eventLoop->timeEventHead = te->next;
            }

            if (te->next) {
                te->next->prev = te->prev;
            }

            if (te->finalizerProc) {
                te->finalizerProc(eventLoop, te->clientData);
            }

            free(te);
            te = next;
            continue;
        }

        /* Make sure we don't process time events created by time events in
         * this iteration. Note that this check is currently useless: we always
         * add new timers on the head, however if we change the implementation
         * detail, this check may be useful again: we keep it here for future
         * defense. */
        // 跳过无效事件
        if (te->id > maxId) {
            te = te->next;
            continue;
        }
        
        // 获取当前事件
        aeGetTime(&now_sec, &now_ms);
        // 如果当前事件等于或等于事件的执行时间，那么说明事件已经到达，执行这个事件
        if (now_sec > te->when_sec || (now_sec == te->when_sec && now_ms >= te->when_ms)) {
            int retval;

            id = te->id;
            // 执行事件处理器，并获取返回值
            retval = te->timeProc(eventLoop, id, te->clientData);
            processed++;

            // 记录是否有需要循环执行这个事件时间
            if (retval != AE_NOMORE) {
                // 是的，retval 毫秒之后继续执行这个时间事件
                aeAddMillisecondsToNow(retval,&te->when_sec,&te->when_ms);
            } else {
                te->id = AE_DELETED_EVENT_ID;
            }
        }
        te = te->next;
    }
    return processed;
}

/* Process every pending time event, then every pending file event
 * (that may be registered by time event callbacks just processed).
 * 
 * 处理所有已到达的时间事件，以及所有已就绪的文件事件
 * 
 * Without special flags the function sleeps until some file event
 * fires, or when the next time event occurs (if any).
 *
 * 如果不传入特殊flags 的话，那么函数睡眠直到文件事件就绪，或者下个时间事件到达(如果有的话)
 * 
 * If flags is 0, the function does nothing and returns.
 * 如果flags 为0，那么函数不作动作，直接返回
 * 
 * if flags has AE_ALL_EVENTS set, all the kind of events are processed.
 * 如果flags 包含 AE_ALL_EVENTS， 所有类型的事件都会被处理
 * 
 * if flags has AE_FILE_EVENTS set, file events are processed.
 * 如果flags 包含 AE_FILE_EVENTS ，那么处理文件事件
 * 
 * if flags has AE_TIME_EVENTS set, time events are processed.
 * 如果flags 包含AE_TIME_EVENTS ，那么吃力时间事件
 * 
 * if flags has AE_DONT_WAIT set the function returns ASAP until all
 * the events that's possible to process without to wait are processed.
 * if flags has AE_CALL_AFTER_SLEEP set, the aftersleep callback is called.
 * 如果 flags 包含 AE_DONT_WAIT
 * 那么函数在处理完所有不阻塞的事件之后，即刻返回
 *
 * The function returns the number of events processed. 
 * 函数的返回值为已处理事件的数量
 **/
int aeProcessEvents(aeEventLoop *eventLoop, int flags)
{
    int processed = 0, numevents;

    /* Nothing to do? return ASAP */
    if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS)) return 0;

    /* Note that we want call select() even if there are no
     * file events to process as long as we want to process time
     * events, in order to sleep until the next time event is ready
     * to fire. */
    if (eventLoop->maxfd != -1 || ((flags & AE_TIME_EVENTS) && !(flags & AE_DONT_WAIT))) {
        int j;
        aeTimeEvent *shortest = NULL;
        struct timeval tv, *tvp;

        // 获取最近的时间事件
        if (flags & AE_TIME_EVENTS && !(flags & AE_DONT_WAIT)) {
            shortest = aeSearchNearestTimer(eventLoop);
        }

        if (shortest) {
            /**
             * 如果时间事件存在的话
             * 那么根据最近可执行时间事件和现在时间的时间差来决定文件事件的阻塞事件
             */
            long now_sec, now_ms;

            /**
             * 计算距今最近的时间时间还要多久才能到达
             * 并将该时间距保存在tv结构中
             */
            aeGetTime(&now_sec, &now_ms);
            tvp = &tv;

            /* How many milliseconds we need to wait for the next
             * time event to fire? */
            long long ms = (shortest->when_sec - now_sec) * 1000 + shortest->when_ms - now_ms;

            if (ms > 0) {
                tvp->tv_sec = ms / 1000;
                tvp->tv_usec = (ms % 1000)*1000;
            } else {
                tvp->tv_sec = 0;
                tvp->tv_usec = 0;
            }
        } else {

            // 执行到这一步，说明没有时间事件
            // 那么根据AE_DONT_WAIT是否设置来决定是否阻塞，以及阻塞的时间长度


            /* If we have to check for events but need to return
             * ASAP because of AE_DONT_WAIT we need to set the timeout
             * to zero */
            if (flags & AE_DONT_WAIT) {
                // 设施文件时间不阻塞
                tv.tv_sec = tv.tv_usec = 0;
                tvp = &tv;
            } else {
                /* Otherwise we can block */
                // 文件事件可以阻塞直到有事件到达为止
                tvp = NULL; /* wait forever */
            }
        }

        if (eventLoop->flags & AE_DONT_WAIT) {
            tv.tv_sec = tv.tv_usec = 0;
            tvp = &tv;
        }

        /* Call the multiplexing API, will return only on timeout or when
         * some event fires. */
        // 处理文件事件，阻塞水岸由tvp决定
        numevents = aeApiPoll(eventLoop, tvp);

        /* After sleep callback. */
        if (eventLoop->aftersleep != NULL && flags & AE_CALL_AFTER_SLEEP) {
            eventLoop->aftersleep(eventLoop);
        }

        for (j = 0; j < numevents; j++) {
            // 从已就绪数组中获取事件
            aeFileEvent *fe = &eventLoop->events[eventLoop->fired[j].fd];
            int mask = eventLoop->fired[j].mask;
            int fd = eventLoop->fired[j].fd;
            int fired = 0; /* Number of events fired for current fd. */

            /* Normally we execute the readable event first, and the writable
             * event laster. This is useful as sometimes we may be able
             * to serve the reply of a query immediately after processing the
             * query.
             *
             * However if AE_BARRIER is set in the mask, our application is
             * asking us to do the reverse: never fire the writable event
             * after the readable. In such a case, we invert the calls.
             * This is useful when, for instance, we want to do things
             * in the beforeSleep() hook, like fsynching a file to disk,
             * before replying to a client. */
            int invert = fe->mask & AE_BARRIER;

            /* Note the "fe->mask & mask & ..." code: maybe an already
             * processed event removed an element that fired and we still
             * didn't processed, so we check if the event is still valid.
             *
             * Fire the readable event if the call sequence is not
             * inverted. */
            // 读事件
            if (!invert && fe->mask & mask & AE_READABLE) {
                fe->rfileProc(eventLoop, fd, fe->clientData, mask);
                fired++;
            }

            /* Fire the writable event. */
            // 写事件
            if (fe->mask & mask & AE_WRITABLE) {
                if (!fired || fe->wfileProc != fe->rfileProc) {
                    fe->wfileProc(eventLoop,fd,fe->clientData,mask);
                    fired++;
                }
            }

            /* If we have to invert the call, fire the readable event now
             * after the writable one. */
            if (invert && fe->mask & mask & AE_READABLE) {
                if (!fired || fe->wfileProc != fe->rfileProc) {
                    fe->rfileProc(eventLoop,fd,fe->clientData,mask);
                    fired++;
                }
            }

            processed++;
        }
    }
    /* Check time events */
    // 执行时间事件
    if (flags & AE_TIME_EVENTS) {
        processed += processTimeEvents(eventLoop);
    }

    return processed; /* return the number of processed file/time events */
}

/** Wait for milliseconds until the given file descriptor becomes
 * writable/readable/exception 
 * 
 * 在给定毫秒内等待，直到fd变成可写、可读或异常
 **/
int aeWait(int fd, int mask, long long milliseconds) {
    struct pollfd pfd;
    int retmask = 0, retval;

    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fd;
    if (mask & AE_READABLE) pfd.events |= POLLIN;
    if (mask & AE_WRITABLE) pfd.events |= POLLOUT;

    if ((retval = poll(&pfd, 1, milliseconds))== 1) {
        if (pfd.revents & POLLIN) retmask |= AE_READABLE;
        if (pfd.revents & POLLOUT) retmask |= AE_WRITABLE;
        if (pfd.revents & POLLERR) retmask |= AE_WRITABLE;
        if (pfd.revents & POLLHUP) retmask |= AE_WRITABLE;
        return retmask;
    } else {
        return retval;
    }
}

// 事件处理器的主循环
void aeMain(aeEventLoop *eventLoop) {
    eventLoop->stop = 0;
    while (!eventLoop->stop) {

        // 如果有需要在事件前执行的函数，那么运行它
        if (eventLoop->beforesleep != NULL) {
            eventLoop->beforesleep(eventLoop);
        }
        // 开始处理函数
        aeProcessEvents(eventLoop, AE_ALL_EVENTS|AE_CALL_AFTER_SLEEP);
    }
}

// 返回所有使用的多路复用库的名称
char *aeGetApiName(void) {
    return aeApiName();
}

/**
 * 设置处理事件前需要被执行的函数
 */
void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforesleep) {
    eventLoop->beforesleep = beforesleep;
}

/**
 * 设置处理事件后需要被执行的函数
 */
void aeSetAfterSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *aftersleep) {
    eventLoop->aftersleep = aftersleep;
}