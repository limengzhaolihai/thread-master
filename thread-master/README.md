# ThreadPool_C

#### 介绍
Linux下用C语言实现的线程池及其线程间通信




#### 安装教程

1.  直接make编译即可 



#### 使用说明

1.  src目录下存放源码 
2.  head目录下存放头文件
3.  obj目录存放编译后的.o文件

### 函数使用
1.  For instance :send(&G,2,c,1,0x0206);
    2是你要发送的线程ID  c是字符串消息的地址  1是你自己的线程ID  0x0206是msg的ID  &G不需要和改动
2.  For instance :delete(&G,1,0x106);
    1是你要删除消息所在的线程id 106是你要删除的消息msgid

