#ifndef __threadpool_H__
#define __threadpool_H__
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>

#define DELNUM 2 

typedef struct Task
{
    void (*fun)(void*); 
    void *arg; 
        
}Task;

typedef struct ThreadPool
{
    Task* task_queue;             // 任务队列(存放当前提交的任务)
    int queue_capacity;           // 队列最大容量 
    int queue_size;               // 当前任务个数 
    int queue_front;                
    int queue_rear; 
    
    pthread_t manage_id;          // 管理者线程ID
    pthread_t *thread_ids;         // 工作的线程ID数组(从任务队列中取出任务进行工作)
    int max_num;                  // 最大线程个数 
    int min_num;                  // 最小线程个数 
    int busy_num;                 // 正在忙碌的线程个数 
    int live_num;                 // 目前存活的线程个数 
    int exit_num;                 // 目前需要销毁的线程个数 

    pthread_mutex_t mutex_pool;   // 锁整个线程池的互斥锁 
    pthread_rwlock_t rwlock_num;  // 锁 xxx_num 变量的读写锁 
    pthread_cond_t cond_not_empty; // 任务队列是否为空
    pthread_cond_t cond_not_full;  // 任务队列是否为满 

    int flag_shutdown;             // 线程池是否被销毁  0 -- 运行状态    1 -- 销毁 
    
}ThreadPool;

ThreadPool* getThreadPool(int min, int max, int capacity); // 得到一个初始化的线程池

void addTask(ThreadPool *pool, void (*fun)(void*), void *arg); // 往线程池的任务队列添加任务 

int delThreadPool(ThreadPool *pool); // 销毁线程池

int getLiveNum(ThreadPool *pool);    // 获取线程池目前存活的线程个数

int getBusyNum(ThreadPool *pool);    // 获取线程池目前忙碌的线程个数

#endif
