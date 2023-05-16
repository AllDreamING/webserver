#include "http_conn.h"

int http_conn::m_epollfd = -1;   //所有socket上的事件都被注册到一个epollfd上
int http_conn::m_user_count = 0;

void setnonblocking(int fd){    //设置文件描述符非阻塞
    int old_flag = fcntl(fd,F_GETFL);   //获取oldflag
    int new_flag = old_flag | O_NONBLOCK;   //增加nonblock
    fcntl(fd,F_SETFL,new_flag); //设置
}

void addfd(int epollfd,int fd,bool oneshot){    //添加文件描述符到epoll中
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP ;
    // event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    if(oneshot){
        event.events | EPOLLONESHOT ;   //重置oneshot
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event); //将fd添加到epollfd中
    setnonblocking(fd); //设置文件描述符非阻塞
}

void removefd(int epollfd,int fd){  //从epoll中删除文件描述符
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
    close(fd);
}

extern void modfd(int epollfd,int fd,int ev){   //修改文件描述符
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;  //重置oneshot
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

void http_conn::init(int sockfd,const sockaddr_in& addr){   //初始化新接收的连接
    m_sockfd = sockfd;
    m_address = addr;

    int reuse = 1;
    setsockopt(m_sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));//设置端口复用

    addfd(m_epollfd,m_sockfd,true); //添加到epoll中
    m_user_count++; //用户数++

    // init();
}

void http_conn::close_conn(){   //关闭连接
    if(m_sockfd!=-1){
        removefd(m_epollfd,m_sockfd);   //将fd从epollfd中移除
        m_sockfd = -1;  //
        m_user_count--; //用户数--
    }
}

bool http_conn::read(){ //一次性读完数据
    printf("一次性读完数据\n");
    return true;
}

bool http_conn::write(){    //一次性写完数据
    printf("一次性写完数据\n");
    return true;
}

void http_conn::process(){  //由线程池中的线程调用，这是处理http请求的入口函数
    //解析http请求
    printf("parse request , create response ! \n");
    //生成响应
}