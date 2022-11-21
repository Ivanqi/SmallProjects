#include <sys/epoll.h>

typedef struct aeApiState {
    // epoll_event 实例描述
    int epfd;

    // 事件槽
    struct epoll_event *events;
} aeApiState;

/**
 * 创建
 */
static int aeApiCreate(aeEventLoop *eventLoop) {
    aeApiState *state = malloc(sizeof(aeApiState));


    if (!state) return -1;

    // 初始化事件槽空间
    state->events = malloc(sizeof(struct epoll_event) * eventLoop->setsize);
    if (!state->events) {
        zfree(state);
        return -1;
    }

    // 创建epoll实例
    state->epfd = epoll_create(1024); /* 1024 is just a hint for the kernel */
    if (state->epfd == -1) {
        free(state->events);
        free(state);
        return -1;
    }
    // 赋值给 eventloop
    eventLoop->apidata = state;
    return 0;
}

/**
 * 调整事件槽大小
 */
static int aeApiResize(aeEventLoop *eventLoop, int setsize) {
    aeApiState *state = eventLoop->apidata;

    state->events = realloc(state->events, sizeof(struct epoll_event) * setsize);
    return 0;
}

/**
 * 释放epoll 实例和事件槽
 */
static void aeApiFree(aeEventLoop *eventLoop) {
    aeApiState *state = eventLoop->apidata;

    close(state->epfd);
    free(state->events);
    free(state);
}

/**
 * 关联给定事件到 fd
 */
static int aeApiAddEvent(aeEventLoop *eventLoop, int fd, int mask) {
    aeApiState *state = eventLoop->apidata;
    struct epoll_event ee = {0}; 

    /* avoid valgrind warning */

    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. 
     * 
     * 如果 fd没有关联任何事件，那么这就是一个 ADD操作
     * 
     * 如果已经关联了某个/某些事件，那么这是一个MOD操作
     * */
    int op = eventLoop->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    //  注册事件到epoll
    ee.events = 0;

    mask |= eventLoop->events[fd].mask; /* Merge old events */

    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;

    ee.data.fd = fd;

    if (epoll_ctl(state->epfd, op, fd, &ee) == -1) return -1;

    return 0;
}

/**
 * 从 fd中删除给定事件
 */
static void aeApiDelEvent(aeEventLoop *eventLoop, int fd, int delmask) {
    aeApiState *state = eventLoop->apidata;
    struct epoll_event ee = {0}; /* avoid valgrind warning */

    int mask = eventLoop->events[fd].mask & (~delmask);

    ee.events = 0;

    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;

    ee.data.fd = fd;

    if (mask != AE_NONE) {
        epoll_ctl(state->epfd,EPOLL_CTL_MOD,fd,&ee);
    } else {
        /* Note, Kernel < 2.6.9 requires a non null event pointer even for
         * EPOLL_CTL_DEL. */
        epoll_ctl(state->epfd,EPOLL_CTL_DEL,fd,&ee);
    }
}

static int aeApiPoll(aeEventLoop *eventLoop, struct timeval *tvp) {

    aeApiState *state = eventLoop->apidata;
    int retval, numevents = 0;

    // 等待时间
    retval = epoll_wait(state->epfd,state->events,eventLoop->setsize, tvp ? (tvp->tv_sec * 1000 + tvp->tv_usec / 1000) : -1);

    // 有至少一个事件就绪?
    if (retval > 0) {
        int j;

        /**
         * 为已就绪事件设置相应的模式
         * 并加入到eventLoop的fired数组中
         */
        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct epoll_event *e = state->events + j;

            if (e->events & EPOLLIN) mask |= AE_READABLE;
            if (e->events & EPOLLOUT) mask |= AE_WRITABLE;
            if (e->events & EPOLLERR) mask |= AE_WRITABLE | AE_READABLE;
            if (e->events & EPOLLHUP) mask |= AE_WRITABLE | AE_READABLE;

            eventLoop->fired[j].fd = e->data.fd;
            eventLoop->fired[j].mask = mask;
        }
    }

    // 返回已就绪事件个数
    return numevents;
}

// 返回当前正在使用的poll 库的名字
static char *aeApiName(void) {
    return "epoll";
}
