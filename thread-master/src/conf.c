#include"conf.h"
void send(graph *G,int receiver,char* sda,int sender,int msg_id){
    Arcnode *newnode;
    int i=0,j=0;
    //先去比对数组初始化的receiver 是否和发送对象一致，如果一致把消息放到这个id对应的链表里
    for(i=0;i<maxsize;i++){//采用头插的方法
        Arcnode *HEAD=G->adjlist[i].firstarc;
        if(G->adjlist[i].receiver==receiver){
            newnode=(Arcnode*)malloc(sizeof(Arcnode));
            newnode->data[i]=sda;
            newnode->sender=sender;
            newnode->msg_id=msg_id;
            newnode->nextarc=HEAD->nextarc;
            HEAD->nextarc=newnode;
            break;
        }
    }
    //如果没有一致的，就创建这个新的receiver对应的线程链表 
    if(i==maxsize){
        for(j=0;j<maxsize;j++){
            Arcnode *HEAD=G->adjlist[j].firstarc;
            if(G->adjlist[j].receiver==0){
                newnode=(Arcnode*)malloc(sizeof(Arcnode));
                newnode->data[j]=sda;
                G->adjlist[j].receiver=receiver;
                newnode->sender=sender;
                newnode->nextarc=HEAD->nextarc;
                HEAD->nextarc=newnode;
                break;
            }
        }
    }
}
//根据对应的msgid 和线程链表的id去删除(按id查找去删除)
void delete(graph*G,int index,int deid){
    Arcnode*p=G->adjlist[index-1].firstarc;
    Arcnode*deletenode;
    while(p!=NULL){
        if(p->nextarc->msg_id==deid){
            printf("the delete id is %x\n",deid);
            deletenode=p->nextarc;
            p->nextarc=p->nextarc->nextarc;
            free(deletenode);
            deletenode=NULL;
        }
        p=p->nextarc;
    }
}
//初始化线程id及msg比对数组
void init_all(graph*G){
    Arcnode *HEAD;
    int j=0;
    for(j=0;j<maxsize;j++){//初始化为带头的线程链表  线程id从1到100的 100个线程链表
        G->adjlist[j].receiver= j+1;
        G->adjlist[j].firstarc=NULL;
        HEAD=(Arcnode*)malloc(sizeof(Arcnode));
        HEAD->data[j]=NULL;
        HEAD->sender=0;
        HEAD->msg_id=0;
        HEAD->nextarc=G->adjlist[j].firstarc;
        G->adjlist[j].firstarc=HEAD;
    }
    pthread_cond_init(&recv,NULL);
	pthread_mutex_init(&mutex,NULL);
}
