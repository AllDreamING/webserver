#pragma once
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include<sys/epoll.h>
#include<stdio.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<stdarg.h>
#include<sys/uio.h>
#include"locker.h"

class http_conn{
public:

    static int m_epollfd;   //所有socket上的事件都被注册到一个epollfd上
    static int m_user_count; //统计用户的数量
    http_conn(){}
    ~http_conn(){}
    void process(); //处理客户端的请求
    void init(int sockfd,const sockaddr_in& addr);  //初始化新接收的连接
    void close_conn();  //关闭连接
    bool read();    //非阻塞的读
    bool write();   //非阻塞的写

private:
    int m_sockfd;   //该链接的socket
    sockaddr_in m_address;  //客户端的socket地址
};

#endif