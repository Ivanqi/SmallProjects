#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

// 封装信号量的类
class sem 
{
    public:
        // 创建并初始化信号量
        sem() 
        {
            /**
             * int sem_init(sem_t* sem, int pshared, unsigned int value);
             * sem_init 函数用于初始化一个未命名的信号量
             *  pshared 参数指定信号量的类型。如果值为0，就表示这个信号量是当前进程的局部变量，否则该信号量就可以在多个进程之间共享
             *  value 参数指定信号量的初始值
             */
            if (sem_init(&m_sem, 0, 0) != 0) {
                // 构造函数没有返回值，可以通过抛出异常来报告错误
                throw std::exception();
            }
        }

        // 销毁信号量
        ~sem()
        {
            /**
             * int sem_destory(sem_t* sem);
             * sem_destory 函数用于销毁信号量，以释放其占用的内核资源
             */
            sem_destroy(&m_sem);
        }

        // 等待信号量
        bool wait()
        {
            /**
             * int sem_wait(sem_t* sem);
             * sem_wait 函数以原子操作的方式将信号量的值减1。如果信号量的值为0，则sem_wait将被阻塞，直到这个信号量具有非0的值
             * 函数成功时返回0，失败时返回-1
             */
            return sem_wait(&m_sem) == 0;
        }
        
        // 增加信号量
        bool post()
        {
            /**
             * int sem_post(sem_t* sem)
             * sem_post函数以原子操作的方式将信号量的值加1。当信号量的值大于0时候，其他正在调用sem_wait等待信号量的线程将被唤醒
             * 函数成功时返回0， 失败时返回-1
             */
            return sem_post(&m_sem) == 0;
        }
    private:
        sem_t m_sem;
};

// 封装互斥锁的类
class locker
{
    public:
        // 创建并初始化互斥锁
        locker()
        {
            if (pthread_mutex_init(&m_mutex, NULL) != 0)
            {
                throw std::exception();
            }
        }

        // 销毁互斥锁
        ~locker()
        {
            phtread_mutex_destory(&m_mutex);
        }

        // 获取互斥锁
        bool lock()
        {
            return pthread_mutex_lock(&m_mutex) == 0;
        }

        // 释放互斥锁
        bool unlock()
        {
            return pthread_mutex_unlock(&m_mutex) == 0;
        }

    private:
        pthread_mutex_t m_mutex;
};

// 封装条件变量的类
class cond
{
    public:
        // 创建并初始化条件变量
        cond()
        {
            if (pthread_mutex_init(&m_mutex, NULL) != 0) {
                throw std::exception();
            }

            if (pthread_cond_init(&m_cond, NULL) != 0) {
                // 构造函数中一旦出现问题，就应该立即释放已经成功分配的资源
                pthread_mutex_destory(&m_mutex);
                throw std::exception();
            }
        }

        // 销毁条件变量
        ~cond()
        {
            pthread_mutex_destory(&m_mutex);
            pthread_cond_destory(&m_cond);
        }

        // 等待条件变量
        bool wait()
        {
            int ret = 0;
            pthread_mutext_lock(&m_mutex);
            ret = pthread_cond_wait(&m_cond, &m_mutex);
            pthread_mutext_unlock(&m_mutex);
            return ret == 0;
        }

        // 唤醒等待条件变量的线程
        bool signal()
        {
            return pthread_cond_signal(&m_cond) == 0;
        }
    private:
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;
};

#endif