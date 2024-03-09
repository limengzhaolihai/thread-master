#ifndef CONF
#define CONF
#include"var.h"
//注册的线程id 
typedef enum
{
    MODULE_MIN =0,
    MODULE_child1,
    MODULE_child2,
    MODULE_child3,
    MODULE_recv,
    MODULE_MAX
}MODULE_ID_E;

//注册的msgid
typedef enum
{
    MODULE_MIN1 = 106,
    MODULE_msg1id,
    MODULE_msg2id,
    MODULE_msg3id,
    MODULE_msg4id,
    MODULE_MAX1
}MODULE_ID_E1;

/*
    这个data的指针数组是存的字符串数据  存在对应线程id的下标里
    这个sender是发送者的线程id
    这个msg_id是消息的id
    这个receiver是发送对象的线程tid
*/
typedef struct Arcnode
{
	char *data[maxsize];
    int sender;
    int msg_id;
	struct Arcnode *nextarc;
    struct Arcnode *prearc;
}Arcnode;
typedef struct Vnode
{
	int receiver;
	Arcnode *firstarc;
}Vnode,AdjList[maxsize];
typedef struct graph
{
	AdjList adjlist;
}graph;

typedef struct msg_hdr {
    unsigned short msg_id;
    unsigned short msg_flag;
}head;//为了后续解析msg预留的（强转使用的)

//定义一个结构体全局变量
graph G;
void delete(graph*G,int index,int deid);
void send(graph *G,int reciver,char* sda,int sender,int msg_id);
void init_all(graph*G);
#endif