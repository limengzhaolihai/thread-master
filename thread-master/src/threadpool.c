#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>
#include "threadpool.h"


// 线程退出处理函数
void threadExit(ThreadPool *pool)
{
    for(int i = 0; i < pool->max_num; ++i) 
    {
        if(pthread_equal(pool->thread_ids[i], pthread_self()))      
        {
            pool->thread_ids[i] = 0;    // 该工作线程退出后将其原本id写为0, 表示后续可以创建新线程可用该位置
            printf("threadExit() called, tid: %ld exiting....\n", pthread_self()); 
            break; 
        }
    }
    pthread_exit(NULL); 
}


// 工作线程(本质:消费者线程)
void* workThread(void *arg)
{
    ThreadPool *pool = (ThreadPool*)arg;
    Task t; 
    while(1)
    {
        pthread_mutex_lock(&pool->mutex_pool); 
        while(!pool->queue_size && !pool->flag_shutdown) 
        {
            pthread_cond_wait(&pool->cond_not_empty, &pool->mutex_pool);    // 等待任务队列的条件变量

            // 若pool->exit_num 不为0, 说明是引导该线程自纱销毁
            if(pool->exit_num)  
            {
                --pool->exit_num; 
                if(pool->live_num > pool->min_num) 
                {
                    --pool->live_num; 
                    pthread_mutex_unlock(&pool->mutex_pool);    // 注意解锁 
                    threadExit(pool); 
                }
            }
        }

        if(pool->flag_shutdown) 
        {
            pthread_mutex_unlock(&pool->mutex_pool); 
            threadExit(pool); 
        }

        // 从任务队列中取出待执行的任务
        t.fun = pool->task_queue[pool->queue_front].fun; 
        t.arg = pool->task_queue[pool->queue_front].arg; 
        pool->queue_front = (pool->queue_front + 1) % pool->queue_capacity; 
        --pool->queue_size;        
        ++pool->busy_num;                             // 忙碌线程数 + 1
        pthread_cond_signal(&pool->cond_not_full);    // 通知生产者可以继续生产
        pthread_mutex_unlock(&pool->mutex_pool); 

        // 执行任务队列中取出的任务
        printf("thread %ld start working...\n", pthread_self()); 
        t.fun(t.arg);     
        printf("thread %ld end working...\n", pthread_self());
        
        pthread_rwlock_wrlock(&pool->rwlock_num); 
        --pool->busy_num; 
        pthread_rwlock_unlock(&pool->rwlock_num); 
    }
    pthread_exit(NULL);
}

// 管理者线程(管理线程的创建和销毁)
void* manageThread(void *arg)
{
    printf("manage_id : %ld\n", pthread_self()); 
    ThreadPool *pool = (ThreadPool*)arg;
    while(1)
    {
        sleep(3); 
        pthread_mutex_lock(&pool->mutex_pool); 
        if(pool->flag_shutdown) 
        {
            pthread_mutex_unlock(&pool->mutex_pool); 
            break;
        }
        pthread_mutex_unlock(&pool->mutex_pool); 
        printf("queuesize: %d,  pool->live_num: %d, pool->busy_num: %d\n", pool->queue_size, pool->live_num, pool->busy_num); 

        // 当前任务数大于工作线程时创建新线程  (每次在不超过线程池最大线程数的条件下多创建两个线程)
        // 其中 #define DELNUM 2 
        pthread_mutex_lock(&pool->mutex_pool); 
        if(pool->queue_size > pool->live_num && pool->live_num < pool->max_num)
        {
            int cnt = 0; 
            for(int i = 0; pool->live_num < pool->max_num && cnt < DELNUM; ++i)
            {
                if(!pool->thread_ids[i]) 
                {
                    printf("create new thread...............\n"); 
                    pthread_create(&pool->thread_ids[i], NULL, workThread, pool); 
                    pthread_detach(pool->thread_ids[i]); 
                    ++cnt; 
                    pool->live_num++; 
                }
            }
            pthread_mutex_unlock(&pool->mutex_pool); 
            continue; 
        } 
        pthread_mutex_unlock(&pool->mutex_pool); 
        
        // 当前忙碌线程太少销毁部分线程  (保证线程池中最小线程数的情况下, 每次销毁两个线程)
        pthread_mutex_lock(&pool->mutex_pool); 
        if(pool->busy_num * 2 < pool->live_num && pool->live_num > pool->min_num)
        {
            printf("del thread ..............\n"); 
            pool->exit_num = DELNUM; 
            pthread_mutex_unlock(&pool->mutex_pool); 
            for(int i = 0; i < DELNUM; ++i) 
                pthread_cond_signal(&pool->cond_not_empty); 
            continue; 
        }
        pthread_mutex_unlock(&pool->mutex_pool); 
    }
    pthread_exit(NULL); 
}

// 得到一个初始化的线程池
ThreadPool* getThreadPool(int min, int max, int capacity)
{
    if(min > max)
    {
        fprintf(stderr, "args error: arg1 > arg2\n"); 
        return NULL; 
    }

    ThreadPool *pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    do
    {
        if(!pool) break; 
        pool->task_queue = (Task*)malloc(sizeof(Task) * capacity); 
        if(!pool->task_queue) break;
        pool->queue_capacity = capacity;        
        pool->queue_size = 0;
        pool->queue_front = 0; 
        pool->queue_rear = 0; 
        pool->max_num = max; 
        pool->min_num = min; 
        pool->busy_num = 0; 
        pool->live_num = min; 
        pool->exit_num = 0; 
        if(pthread_mutex_init(&pool->mutex_pool, NULL) || 
           pthread_rwlock_init(&pool->rwlock_num, NULL) || 
           pthread_cond_init(&pool->cond_not_full, NULL) || 
           pthread_cond_init(&pool->cond_not_empty, NULL)) break; 

        pool->flag_shutdown = 0;
        
        // 先把上面的相关信息、锁、条件变量都初始化了才开始创建线程, 否则可能出现段错误
        int retvalue = 0; 
        retvalue = pthread_create(&pool->manage_id, NULL, manageThread, pool);
        if(retvalue) break;
        pool->thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * max);
        if(!pool->thread_ids) break;
        memset(pool->thread_ids, 0, sizeof(pthread_t) * max);

        // 创建工作线程 
        for(int i = 0; i < min; ++i) 
        {
            retvalue = pthread_create(pool->thread_ids + i, NULL, workThread, pool); 
            pthread_detach(pool->thread_ids[i]); 
            if(retvalue) break; 
        }
        if(retvalue) break;
        return pool; 
    }while(0);

    fprintf(stderr, "Init error\n");

    // 若上面出现初始化失败则释放所有资源
    if (pool) 
    {
        free(pool->thread_ids);
        free(pool->task_queue);
        pthread_mutex_destroy(&pool->mutex_pool); 
        pthread_rwlock_destroy(&pool->rwlock_num);
        pthread_cond_destroy(&pool->cond_not_full); 
        pthread_cond_destroy(&pool->cond_not_empty); 
        free(pool);
    }
    return NULL; 
}

// 往线程池添加任务(生产者)
void addTask(ThreadPool *pool, void (*fun)(void*), void *arg)  
{
    pthread_mutex_lock(&pool->mutex_pool); 
    while(pool->queue_size == pool->queue_capacity && !pool->flag_shutdown)  // 若任务队列已满则循环等待条件变量
        pthread_cond_wait(&pool->cond_not_full, &pool->mutex_pool);          

    if(pool->flag_shutdown) 
    {
        pthread_mutex_unlock(&pool->mutex_pool); 
        return; 
    }

    // 将提交的任务加入到任务队列中 
    pool->task_queue[pool->queue_rear].fun = fun; 
    pool->task_queue[pool->queue_rear].arg = arg; 
    ++pool->queue_size; 
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_capacity; 
    pthread_mutex_unlock(&pool->mutex_pool); 
    pthread_cond_signal(&pool->cond_not_empty);        // 通知阻塞在该条件变量等待任务的线程
}

// 销毁线程池 释放其资源
int delThreadPool(ThreadPool *pool) 
{
    if(!pool) return -1; 
    pool->flag_shutdown = 1;                           // 管理者线程退出
    pthread_join(pool->manage_id, NULL);               // 等待回收管理者线程 
    for(int i = 0; i < pool->live_num; ++i) pthread_cond_signal(&pool->cond_not_empty);  // 唤醒阻塞的工作线程引导退出

    free(pool->task_queue); 
    free(pool->thread_ids); 
    
    pthread_mutex_destroy(&pool->mutex_pool); 
    pthread_rwlock_destroy(&pool->rwlock_num);
    pthread_cond_destroy(&pool->cond_not_full); 
    pthread_cond_destroy(&pool->cond_not_empty); 
    free(pool); 
    return 0; 
}

// 获取线程池目前存活的线程个数
int getLiveNum(ThreadPool *pool)    
{
    pthread_rwlock_rdlock(&pool->rwlock_num); 
    int live_num = pool->live_num; 
    pthread_rwlock_unlock(&pool->rwlock_num); 
    return live_num; 
}

// 获取线程池目前忙碌的线程个数
int getBusyNum(ThreadPool *pool)    
{
    pthread_rwlock_rdlock(&pool->rwlock_num); 
    int busy_num = pool->busy_num; 
    pthread_rwlock_unlock(&pool->rwlock_num); 
    return busy_num; 
}
