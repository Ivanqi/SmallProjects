#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "compiler.h"
#include "rbtree.h"
#include "co.h"

static int efd;
static struct rb_root events_tree = RB_ROOT;
static struct epoll_event *events;
static int nr_events;
static int key_event;
static int key_coevent;

static void coevent_cleanup(void *data);
static void __init init_event() {
    nr_events = 1024;
    events = malloc(nr_events * sizeof(struct epoll_event));
    memset(events, 0, nr_events * sizeof(struct epoll_event));
    
    efd = epoll_create(nr_events);
    if (efd < 0) {
        //TODO
    }

    key_event = co_key_create(NULL);
    key_coevent = co_key_create(coevent_cleanup);
}

struct event_info {
    int fd;
    event_handler_t handle;
    void *data;
    int events;
    struct rb_node rb;
};

static int event_cmp(const struct event_info *e1, const struct event_info *e2) {
    return intcmp(e1->fd, e2->fd);
}

static struct event_info *lookup_event(int fd) {
    struct event_info key = { .fd = fd };
    return rb_search(&events_tree, &key, rb, event_cmp);
}

int register_event(int fd, event_handler_t h, void *data)
{
    int ret;
    struct epoll_event ev;
    struct event_info *ei;
    
    ei = malloc(sizeof(struct event_info));
    ei->fd = fd;
    ei->handle = h;
    ei->data = data;
    ei->events = EPOLLIN | EPOLLET;
    rb_init_node(&ei->rb);
    
    memset(&ev, 0, sizeof(ev));
    ev.events = ei->events;
    ev.data.ptr = ei;
    
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
    if (ret) {
        free(ei);
    } else
        rb_insert(&events_tree, ei, rb, event_cmp);
    return ret;
}

void unregister_event(int fd)
{
    struct event_info *ei;
    
    ei = lookup_event(fd);
    if (!ei) {
        return;
    }
    
    epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
    rb_erase(&ei->rb, &events_tree);
    free(ei);
}

int modify_event(int fd, unsigned int new_events)
{
    int ret;
    struct epoll_event ev;
    struct event_info *ei;
    
    if(likely(coid() != 0)) {
        ei = co_getspecific(key_event);
        if(unlikely(!ei || ei->fd != fd)) {
            ei = lookup_event(fd);
            co_setspecific(key_event, ei);
        }
    } else {
        ei = lookup_event(fd);
    }

    if (unlikely(!ei)) {
        return 1;
    }

    if(ei->events == new_events | EPOLLET) return 0;
    
    memset(&ev, 0, sizeof(ev));
    ev.events = ei->events = new_events | EPOLLET;
    ev.data.ptr = ei;

    ret = epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev);
    if (ret) {
        return 1;
    }

    return 0;
}

// timeout：单位是微妙
// return：是否继续循环，1：继续，0：不再继续
#define MAX_POLLING 128
int event_loop(int timeout) {
    int i, nr;
    static int polling = MAX_POLLING;
    
    if (RB_EMPTY_ROOT(&events_tree)) {
        // 无事件也不需要超时，返回0
        if(timeout < 0) return 0;
    } else {
        if (timeout < 0) {
            /*
             * 轮询和等待之间的转换
             * 在有大量的socket被监听，每个socket都有包需要处理时，epoll_wait
             * 大多数情况会直接返回，少数情况会把进程切换出去，然后其中一个
             * socket有包的时候，再把进程唤醒。
             * 进程切换再唤醒会存在较大延迟，会造成一部分包不能及时处理。
             * 直接的现象是：虽然有大量的包，但进程的cpu占用到不了100%
             *
             * 默认采用轮询方式(timeout = 0)，在经过MAX_POLLING次轮询，仍然没有
             * 任何一个socket有包时就切换到wait模式。
             * MAX_POLLING = 128：
             * 是测试到的一个经验值，不断的修改MAX_POLLING的值，通过perf测试进程
             * 切换次数，当切换次数不再明显减少时，即得到了最佳的值。
            **/
            if (polling != 0) {
                timeout = 0;
                polling--;
            }
        } else {
            polling = MAX_POLLING;
        }
    }

    nr = epoll_wait(efd, events, nr_events, timeout<0 ? -1 : timeout/1000);
    if (unlikely(nr < 0)) {
        if (errno == EINTR) {
            return 1;
        }
        exit(1);
    } else if (nr) {
        //复位轮询计数器
        polling = MAX_POLLING;
        for (i = 0; i < nr; i++) {
            struct event_info *ei;
            ei = (struct event_info *)events[i].data.ptr;
            ei->handle(ei->fd, events[i].events, ei->data);
        }
    }
    return 1;
}

struct coevent_info {
    void *co;
    int fd;
    coevent_handler_t handle;
    void *data;
    int events;
};

static void coevent_wakeup(int fd, int events, void *data) {
    struct coevent_info *coevent = data;
    coevent->events = events;
    __cowakeup(coevent->co);
}

// cokill，确保能销毁协程的coevent_info信息
static void coevent_cleanup(void *data) {
    struct coevent_info *coevent = data;
    
    unregister_event(coevent->fd);
    
    close(coevent->fd);
    
    free(coevent);
}

static void coevent_routine(void *data) {
    struct coevent_info *coevent = data;
    
    coevent->co = coself();
    
    register_event(coevent->fd, coevent_wakeup, coevent);
    
    //使用协程局部存储来存放coevent_info结构
    //在协程被cokill后，可以调用coevent_cleanup来销毁coevent_info信息
    co_setspecific(key_coevent, data);
    coevent->handle(coevent->fd, coevent->data);
    co_setspecific(key_coevent, NULL);
    
    coevent_cleanup(data);
}

int register_coevent(int fd, coevent_handler_t h, void *data) {
    struct coevent_info *coevent;
    
    coevent = malloc(sizeof(struct coevent_info));
    if(unlikely(coevent == NULL)) {
        return -1;
    }
    
    cocreate(DEFAULT_STACK, coevent_routine, coevent);
    coevent->fd = fd;
    coevent->handle = h;
    coevent->data = data;
    return 0;
}