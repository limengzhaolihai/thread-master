
#ifndef MAIN_H
#define MAIN_H
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif
#define maxsize 100
#define HANDSHARK_MSG "1001"
pthread_mutex_t mutex;
pthread_cond_t recv;
//
//用于超时监测
int count;
int connect;
pthread_t t;//通知线程
pthread_t t5;//超时线程
pthread_t tid;//工作线程
int n;//线程数量

char c[100];//测试数组

