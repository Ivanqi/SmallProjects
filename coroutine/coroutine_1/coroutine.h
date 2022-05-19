#ifndef C_COROUTINE_H
#define C_COROUTINE_H

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

struct schedule;    // 前置声明协程调度器 struct schedule 类型

typedef void (*coroutine_func)(struct schedule*, void *uid);    // 声明一个函数指针类型

struct schedule *coroutine_open(void);  // 创建协程调度器
void coroutine_close(struct schedule*); // 关闭协程调度器

int coroutine_new(struct schedule*, coroutine_func, void *ud);  // 创建协程任务，将其加入调度器中
void coroutine_resume(struct schedule*, int id);                // 恢复协程号id的协程任务
int coroutine_status(struct schedule*, int id);                 // 根据协程任务ID返回协程当前状态
int coroutine_running(struct schedule*);                        // 返回调度器S中正在running的协程任务id
void coroutine_yield(struct schedule*);                         // 保存当前上下文后中断当前协程的执行

#endif