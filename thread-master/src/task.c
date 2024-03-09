#include"task.h"
int  compare(int value,int i){//比对线程id和线程链表里存放的id  去处理   
    for(i=0;i<maxsize;i++){
        if(G.adjlist[i].receiver==value){
            return i;
        }
    }
    return -1;
}
void taskFunction(void *arg)//任务就是处理各个线程链表里的消息  这里我选择把每个线程链表里的消息打印出来
{
    int i;
    i = (int)arg;
    sleep(i); //通过i来区别每个线程
    printf("我是第%d个线程,我的线程ID = %lu\n", i + 1, i+1);//pthread_self()线程id是自己定的 第几个线程线程id就是几
    while(1) 
	{
        pthread_mutex_lock(&mutex);
        int k=0;
        int ret=compare(i+1,k);//把第几个线程和线程链表里的线程id进行比对
        if(ret==-1){
            printf("fatal\n");
        }else{  
                Arcnode*p;//定义一个新的指针去遍历每个线程链表     
                p=G.adjlist[ret].firstarc->nextarc;
                while(p!=NULL){
                    if(p->data[ret]!=HANDSHARK_MSG){
                        printf("%d:I has been received the msg is %s from %d and the msg is %x\n",G.adjlist[ret].receiver,p->data[ret],p->sender,p->msg_id);
                        //send(&G,G.adjlist[ret].firstarc->sender,HANDSHARK_MSG,G.adjlist[ret].receiver,G.adjlist[ret].firstarc->msg_id);//发送握手信息给对应的线程
                        //printf("%d:handshake send to %d successful senmsgid is %d\n",G.adjlist[ret].receiver,G.adjlist[ret].firstarc->sender,G.adjlist[ret].firstarc->msg_id);
                        connect=0;
                    }
                    if(p->data[ret]==HANDSHARK_MSG){
                        printf("%d:I has been received the %d reply and the msgid is %x\n",G.adjlist[ret].receiver,G.adjlist[ret].firstarc->sender,G.adjlist[ret].firstarc->msg_id);//发握手消息的时候把msgid也发过来了，为了方便判断是不是我上次发的msg而回的握手消息
                        connect=1;
                    }
                    p=p->nextarc;//遍历
                }
           }
        pthread_mutex_unlock(&mutex);
		sleep(2);
    }
    return; 
}
