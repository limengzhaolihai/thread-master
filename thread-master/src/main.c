#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>
#include"threadpool.h"
#include"conf.h"
#include"task.h"
char c[100]="hello";//test
void sys_error(const char *str) 
{
    perror(str);
    exit(1);
}
int main(int argc, char *argv[])
{
    init_all(&G);
    printf("init is true\n");
    send(&G,1,c,2,0x106);//1就是接收者的id,2就是发送者的id  c就是消息内容  106就是消息id
    send(&G,2,c,3,0x107);
    send(&G,2,c,3,0x109);
    send(&G,3,c,2,0x206);
    delete(&G,1,0x106);//删除第1个线程链表里的106消息
    ThreadPool *pool = getThreadPool(2, 10, 100); //设置最小管理线程数和最大工作线程数 以及线程队列最大个数
    if(!pool) sys_error("getThreadPool error"); 
    for(int i = 0; i < 5; i++) 
        addTask(pool, taskFunction, (void*)i);//把任务添加到线程队列里去  i是区分第几个线程的标志
    sleep(30); 
    printf("pool->livenum = %d\n", getLiveNum(pool));
    delThreadPool(pool); 
    printf("end of main\n"); 
    pthread_exit(NULL);
}
